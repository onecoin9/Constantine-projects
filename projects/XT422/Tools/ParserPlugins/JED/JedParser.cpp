#include "JedParser.h"
#include "../public/JedParser/JedPlayerAPI.h"

#define SPC_OFFSET__SECURE 0x10

JedParser::JedParser()
{
}

char* JedParser::getVersion()
{
	return "1.0.0.0";
}
char* JedParser::getFormatName()
{
	return C_JED;
}
bool JedParser::ConfirmFormat(QString& filename)
{
	if (QFileInfo(filename).suffix().compare("jed", Qt::CaseInsensitive))
		return false;
	return true;
}
int JedParser::TransferFile(QString& srcfn, QIODevice* dst)
{
	m_Dst = dst;
	int DataStartAddr = 0;
	ADR BytesWrite;
	char StrTmp[128] = { 0 };
	QString strMsg;

	int nRealSizeOfFuse = 0;
	int BytesToWrite = 0;
	tJedPlayer* pJedPlayer = NULL;
	//tIReporter* pReporter = &AppReporter;
	tFileHandle* pFileHandle = NULL;
	pJedPlayer = JedPlayer_GetPlayer(NULL);
	HANDLE hFile = INVALID_HANDLE_VALUE;
	if (pJedPlayer) {
		pFileHandle = GetFileHandle();
		if (pFileHandle == NULL) {
			ferror = C_Err_RecordSum; goto __end;
		}
		JedPlayer_AttachFileHandle(pJedPlayer, pFileHandle);
		//edPlayer_AttachReporter(pJedPlayer, &AppReporter);

		hFile = CreateFile(
			(const wchar_t*)srcfn.utf16(),              // 文件名称
			GENERIC_READ | GENERIC_WRITE, // 访问模式
			0,                            // 共享模式
			nullptr,                      // 安全属性
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,        // 文件属性
			nullptr                       // 模板文件句柄
		);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			ferror = C_Err_FileOpen;
			goto __end;
		}

		int Ret = 0;
		Ret = pJedPlayer->Opts.CheckHandleFile(pJedPlayer, hFile);
		if (Ret == 1) {///校验正确
			Ret = pJedPlayer->Opts.ParserHandleFile(pJedPlayer, hFile);
			if (Ret == 0) {
				if (pJedPlayer->Opts.CompareFuseListChecksum(pJedPlayer) != 1) {
					ferror = C_Err_RecordSum; goto __end;
				}
			}
			if (Ret == 0) {///解析成功，进行Buffer的写入
				int i;
				int NextLocation = 0;//m_pDataBuffer->BufferGetCurPartition();
				unsigned int ByteSum = 0;
				tFuseData* pFeatureRowBit = NULL;
				tFuseData* pUserCode = NULL;
				tFuseDataMap* pFuseDataMap = pJedPlayer->Opts.GetFuseDataMap(pJedPlayer);
				if (pFuseDataMap) {
					////按bit写入
					UINT i, Offset = 0;
					BytesToWrite = (pFuseDataMap->DataBitSize + 7) / 8;

					nRealSizeOfFuse = pFuseDataMap->DataBitSize;

					//m_pSpcBuf[SPC_OFFSET__SECURE] = pFuseDataMap->nGValue;
					//m_pSpcBuf[1] = pFuseDataMap->nGValue > 0 ? 1 : 0; //驱动端所有的IC设置第1个字节为Securety
					//pJedPlayer->Opts.GetRealSize(pJedPlayer, &nRealSizeOfFuse);				

					for (i = 0; i < BytesToWrite; ++i) {
						ByteSum += pFuseDataMap->pData[i];
					}
					char buf[100];
					sprintf(buf, "Write FuseData BitLength:0x%X", pFuseDataMap->DataBitSize);
					m_pOutput->Log(buf);
					for (i = 0; i < pFuseDataMap->DataBitSize; ++i) {

						unsigned char vBit = (pFuseDataMap->pData[i / 8] >> (i % 8)) & 0x01;
						BytesWrite = BufferWrite(NextLocation + i, &vBit, 1);
						if (BytesWrite != 1) {
							char buf[100] = "Write FuseData To Buffer Failed";
							m_pOutput->Error(buf);
							ferror = C_Err_WriteError;
							break;
						}


					}
					if (i == pFuseDataMap->DataBitSize) {
						char buf[100];
						sprintf(buf, "FuseData Location=0x%08X, TotalBit=%d, Checksum=0x%08X--Byte",
							NextLocation, pFuseDataMap->DataBitSize, ByteSum);
						m_pOutput->Log(buf);
					}
					NextLocation += pFuseDataMap->DataBitSize;
				}
				else {
					ferror = C_Err_RecordSum; goto __end;
				}

				pFeatureRowBit = pJedPlayer->Opts.GetFeatureRowBit(pJedPlayer);
				if (pFeatureRowBit && pFeatureRowBit->DataBitLen) {
					////按bit写入 
					for (i = 0; i < pFeatureRowBit->DataBitLen; ++i) {
						unsigned char vBit = (pFeatureRowBit->pFuseData[i / 8] >> (i % 8)) & 0x01;
						BytesWrite = BufferWrite(NextLocation + i, &vBit, 1);
						if (BytesWrite != 1) {
							char buf[100] ="Write FeatureRowBit To Buffer Failed";
							m_pOutput->Error(buf);
							ferror = C_Err_WriteError;
							break;
						}
					}
					if (i == pFeatureRowBit->DataBitLen) {
						char buf[100];
						sprintf(buf, "Feature Row/Bits Location=0x%X, TotalBit=%d ",
							NextLocation, pFeatureRowBit->DataBitLen);
						m_pOutput->Log(buf);
					}
					NextLocation += pFeatureRowBit->DataBitLen;
				}


				pUserCode = pJedPlayer->Opts.GetUserCode(pJedPlayer);
				if (pUserCode && pUserCode->DataBitLen) {
					////按bit写入 
					if (pJedPlayer->UserCodeWriteMode == 0) {
						for (i = 0; i < pUserCode->DataBitLen; ++i) {
							unsigned char vBit = (pUserCode->pFuseData[i / 8] >> (i % 8)) & 0x01;
							BytesWrite = BufferWrite(NextLocation + i, &vBit, 1);
							if (BytesWrite != 1) {
								char buf[100];
								sprintf(buf, "Write FeatureRowBit To Buffer Failed");
								m_pOutput->Error(buf);
								ferror = C_Err_WriteError;
								break;
							}
						}
					}
					else {
						for (i = 0; i < pUserCode->DataBitLen; ++i) {
							unsigned char vBit = (pUserCode->pFuseData[i / 8] >> (7 - (i % 8))) & 0x01;
							BytesWrite = BufferWrite(NextLocation + i, &vBit, 1);
							if (BytesWrite != 1) {

								char buf[100];
								sprintf(buf, "Write FeatureRowBit To Buffer Failed");
								m_pOutput->Error(buf);
								ferror = C_Err_WriteError;
								break;
							}
						}
					}

					if (i == pUserCode->DataBitLen) {
						char buf[100];
						sprintf(buf, "UserCode Location=0x%X, TotalBit=%d ",
							NextLocation, pUserCode->DataBitLen);
						m_pOutput->Log(buf);
					}
					NextLocation += pUserCode->DataBitLen;
				}
			}
		}
		else {
			ferror = C_Err_RecordSum;
		}
	}

__end:
	if (INVALID_HANDLE_VALUE != hFile)
		CloseHandle(hFile);
	m_pOutput->Log("================Parser Completed=================");
	if (ferror == 0) {
		m_pOutput->Log("Pass");
	}
	else {
		pJedPlayer->Opts.GetErrMsg(pJedPlayer, StrTmp, 128);
		m_pOutput->Log(StrTmp);
	}

	if (pFileHandle) {
		PutFileHandle(pFileHandle);
	}
	if (pJedPlayer) {
		JedPlayer_PutPlayer(pJedPlayer);
	}
	return ferror;
}