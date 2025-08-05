#include "HexParser.h"
#include "../public/DrvDllCom/ComTool.h"
#include <QMessageBox>

enum
{
	HEX_REC_DATA = 0,	// 数据
	HEX_REC_END = 1,	// 文件结束
	HEX_REC_SEGMENT = 2,	// 段地址
	HEX_REC_LINEAR = 4		// 线地址
};




HexParser::HexParser()
{
}

char* HexParser::getVersion() {
    return "1.0.0.0";
}
char* HexParser::getFormatName() {
    return C_HEX;
}
bool HexParser::ConfirmFormat(QString& filename) {
	QFile file(filename);
	char cLineBuf[1024];
	char pTempCache[1024] = { 0 };
	QString strOneline;
	int LineNum = 1;

	QFileInfo fileInfo(filename);
	QString strExt = fileInfo.suffix();
	//if(strExt.CompareNoCase("hex")!=0){///先判断后缀名
	//	return FALSE;
	//}
	if (strExt.compare("bin", Qt::CaseInsensitive) == 0) {
		return false;
	}

	if (!file.open(QIODevice::ReadOnly)) {
		return false;
	}
	for (int i = 0; i < 5; i++) {
		memset(cLineBuf, 0, 1024);
		if (file.read(cLineBuf, 1024) == 0) {
			file.close();
			if (i == 0)
				return false;
			return true;
		}

		/*if( i== 0){
			if ( (cLineBuf[0] == (char)0xFF) && ( cLineBuf[1] == (char)0xFE) ){
				memcpy(pTempCache, cLineBuf+2, 1022);
				memcpy(cLineBuf, pTempCache, 1024 );
			}
		}*/
		strOneline = cLineBuf;
		strOneline = strOneline.remove("\r\n");
		strOneline = strOneline.remove(0x09);///为Tab键
		strOneline = strOneline.remove(0x20); ///空格键
		if (strOneline.isEmpty()) {///为空白行
			i--; ///不计入5行范围
			continue;
		}



		if (cLineBuf[0] != ':') {
			file.close();
			return false;
		}
		else {

		}
	}

	file.close();
	return true;

}


//只提示一次有更多内容在结束标志后
int sBeNotifyWhenNotStandardFile = 0;

int HexParser::TransferFile(QString& srcfn, QIODevice* dst) {
	m_Dst = dst;
	m_PreDataTblAddr = -1;
	m_StartDataTblAdrr = -1;
	m_FlashTableOff = 0;
	//m_bExistFlashTable = false;

	int LineNum = 0, i;
	unsigned nHasRecEndCnt = 0;
	QFile File(srcfn);

	if (!File.open(QIODevice::ReadOnly)) {
		ferror = C_Err_FileOpen;
		return ferror;
	}


	QString strOneline;
	unsigned long DataAddr;
	int HexBytes;
	//tDataALVInfo64 DataALVInfo;
	unsigned char DataBytes, RecordType, * pTmpBuf = NULL, * pDataStart = NULL;
	QString strMsg;
	unsigned long SegmentAddr = 0, LinarAddr = 0, RealDataAddr = 0;
	unsigned long BeginAddr, EndAddr, BytesWrite;
	BeginAddr = (unsigned long)-1;
	EndAddr = 0;
	unsigned char LastRecordType = -1;
	pTmpBuf = new unsigned char[BUFFER_RW_SIZE];
	if (!pTmpBuf) {
		ferror = C_Err_MemAllocErr; goto __end;
	}

	InitControl();
	FileSize = File.size();

	unsigned long nStartErrorAddr = 0;
	unsigned long nCurrErrorAddr = 0;
	int nErrorLen = 0;
	int nStep = 0;
	int nPreErrorRow = 0;
	int nBeginErrorRow = 0;
	bool hasOverlap = false;
	bool bHasPre = false;

	unsigned long FlashTableStartAddr = 0;
	unsigned long FlashTableEndAddr = 0;
	unsigned long FlashBuffSize = 0;

	bool bExistFlashTable = false;
	for (int j = 0; j < m_vBufInfo.size(); ++j) {
		if (QString("Flash table").compare(m_vBufInfo[j].strBufName.c_str(), Qt::CaseInsensitive) == 0) {
			FlashTableStartAddr = m_vBufInfo[j].llBufStart;
			FlashTableEndAddr = m_vBufInfo[j].llBufEnd;
			bExistFlashTable = true;
			break;
		}
	}

	//m_bExistFlashTable = bExistFlashTable;
	m_FlashTableStartAddr = FlashTableStartAddr;

	while (!File.atEnd()) {
		myfgets(&File);
		strOneline = (char*)work;
		if (strOneline.length() == 0) {
			break;
		}
		LineNum++;
		strOneline = strOneline.remove("\r");
		strOneline = strOneline.remove("\n");
		strOneline = strOneline.remove(0x09);///为Tab键
		strOneline = strOneline.remove(0x20); ///空格键

		//
		//strOneline.Trim(0x0A);
		/*strOneline.Trim(0x0D);
		strOneline.Remove(0x0D);		*/

		if (strOneline.isEmpty()) {///为空白行
			continue;
		}

		/*if (LineNum == 1){
			char b0 = (char)strOneline.GetAt(0);
			char b1 = (char)strOneline.GetAt(1);
			if ( (b0== (char)0xFF) && (b1 == (char)0xFE) ){
				strOneline.Delete(0, 2);
				bHasPre = true;
			}
		}

		if (bHasPre){
			strOneline.Remove(0x00);
		}*/

		HexBytes = Str2Hex((const unsigned char*)strOneline.right(strOneline.length() - 1).toLocal8Bit().data(), pTmpBuf, strOneline.length() - 1);
		

		if (HexBytes < 0) {
			strMsg = QString("Line[%1] Translate string to hex data failed, there must be some invalid chars ").arg(LineNum);
			ferror = C_Err_FormatError; goto __end;
		} ///转化错误
		if (HexBytes == 0) { continue; } ///为空行

		if (CheckSumCompare(pTmpBuf, HexBytes) != 0) {///比对校验值
			ferror = C_Err_FormatError; goto __end;
		}

		///提取各个成员数据
		DataBytes = pTmpBuf[0];

		if (HexBytes != (DataBytes + 5))
		{
			strMsg = QString("Line[%1] The length of recode don't match!").arg(LineNum);
			m_pOutput->Warning(strMsg.toLocal8Bit().data());
			ferror = C_Err_FormatError; goto __end;
		}

		DataAddr = (unsigned long)pTmpBuf[1] << 8 | (unsigned long)pTmpBuf[2];
		RecordType = pTmpBuf[3];
		pDataStart = pTmpBuf + 4;
		///注意，线性地址和段地址可以叠加
		switch (RecordType) {
		case HEX_REC_DATA:  ///data record
			break;
		case HEX_REC_END: //end-of-file
			nHasRecEndCnt++;
			break;
		case HEX_REC_SEGMENT: //extended segment address record
			if (LastRecordType == HEX_REC_LINEAR) {
				SegmentAddr = ((unsigned long)pDataStart[0] << 8 | (unsigned long)pDataStart[1]);
			}
			else {
				SegmentAddr = ((unsigned long)pDataStart[0] << 8 | (unsigned long)pDataStart[1]) << 4;
			}
			LastRecordType = HEX_REC_SEGMENT;

			break;
		case HEX_REC_LINEAR: //extended linear address record
			LinarAddr = ((unsigned long)pDataStart[0] << 8 | (unsigned long)pDataStart[1]) << 16;
			SegmentAddr = 0;  ///新的一个线性地址开始，需要将Segment地址清除掉
			LastRecordType = HEX_REC_LINEAR;
			break;
		case 5: //start linear address record (MDK-ARM only)
		case 3:
		default:
			strMsg = QString("Line[%1] Ignore RecordType[%2]").arg(LineNum, RecordType);
			break;
		}

		if (!sBeNotifyWhenNotStandardFile && nHasRecEndCnt == 1 && RecordType != HEX_REC_END) {
			sBeNotifyWhenNotStandardFile = 1;
			//某个tenx档案要求不解析Rec-End后内容，给用户自己选择
			QString tips = "";
			tips = QString("%1\n%2\n%3\n%4").arg("More content found after Rec-End flag, parse content after Rec-End?",
				"Choose Yes: Content After first Rec-End is useful.",
				"Choose No: Content After first Rec-End is not useful.",
				"Choose Cancel: Exit file parser and choose new file.");
			int boxResult = QMessageBox::question(nullptr, "Question", tips,
				QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

			if (boxResult == QMessageBox::Yes) {
				m_pOutput->Warning("More content found after Rec-End flag, User choose parse more..");
			}
			else if (boxResult == QMessageBox::No) {
				m_pOutput->Warning("More content found after Rec-End flag, User choose not parse more.");
				ferror = 0;
				break;
			}
			else if (boxResult == QMessageBox::Cancel) {
				m_pOutput->Error("More content found after Rec-End flag, User choose stop parse.");
				ferror = C_Err_UserAbort;
				goto __ParserEnd;
			}
		}

		if (RecordType == HEX_REC_DATA) {///处理数据Record
			RealDataAddr = LinarAddr + SegmentAddr + DataAddr;

			//RealDataAddr = RealDataAddr + nStartBuffAddr; ///Buffer地址重定向

			//if (bWord) { RealDataAddr *= 2; }  ///如果指定是字节地址，则需要将获取到的地址*2
			//if (bSwap) { SwapByWord(pDataStart, DataBytes); } ///处理Word大小端

			if (RealDataAddr >= EndAddr) { EndAddr = RealDataAddr + DataBytes; }  ///记录最高地址范围
			if (RealDataAddr < BeginAddr) { BeginAddr = RealDataAddr; } ///记录最低地址范围

			if (DataBytes == 0) {
				/*if (m_pIFileALVInfo) {
					DataALVInfo.Address = RealDataAddr;
					DataALVInfo.Length = DataBytes;

					if (m_pIFileALVInfo->WriteALV(DataALVInfo) == FALSE) {
						strMsg.Format("Save ALV Information To FileAVLInfo Buffer Failed");
						m_pOutput->Log(LOG_ERROR, strMsg);
						Ret = -1; goto __end;
					}
				}*/
				continue;
			}
			BytesWrite = BufferWrite(RealDataAddr, pDataStart, DataBytes);
			/*if ((ADR)RealDataAddr > m_pDataBuffer->GetMaxFlashSize()) {
				QString strErrMsg;
				strErrMsg.Format("Data Write Out of Buffer Range,DataWriteAddress:0x%I64X, RealBuff_MaxAddress:0x%I64X", RealDataAddr, m_pDataBuffer->GetMaxFlashSize() - 1);
				m_pOutput->Error(strErrMsg);
				Ret = -1; goto __end;
			}*/

			if (BytesWrite != DataBytes) {
				/*strMsg.Format("Line[%d] Buffer write error, Addr:0x%I64X, Len:0x%X, RealBytesWrite:0x%X",LineNum,RealDataAddr,DataBytes,BytesWrite);
				m_pOutput->Log(LOGLEVEL_WARNING,strMsg);*/
				//return E_FILE_TOO_LONG;

				if (nStartErrorAddr == 0) {
					nStartErrorAddr = RealDataAddr;
					nErrorLen += DataBytes;
					nBeginErrorRow == 0 ? nBeginErrorRow = LineNum : nBeginErrorRow = nBeginErrorRow;
				}
				else {
					if (RealDataAddr != nStartErrorAddr + nErrorLen && (!(RealDataAddr < nStartErrorAddr + nErrorLen)))
					{
						QString strOverlap = "";
						if (hasOverlap) {
							strOverlap = " There Is Overlapped In Buffer, ";
						}

						QString strLineInfo;
						int nEndErrorRow = 0;
						nPreErrorRow == 0 ? nEndErrorRow = LineNum : nEndErrorRow = nPreErrorRow;
						if (nStep) {
							strLineInfo = QString("Line[%d]").arg(nBeginErrorRow);
						}
						else {
							strLineInfo = QString("Line[%d - %d]").arg(nBeginErrorRow, nEndErrorRow);
						}


						char buf[250];
						memset(buf, 0, 250);

						
						sprintf(buf, "%s Buffer write error,%s Addr:0x%I64X, Len:0x%X, RealBytesWrite:0x%X", strLineInfo.toStdString().c_str(), strOverlap.toStdString().c_str(), nStartErrorAddr, nErrorLen, 0);
						//nErrorLen = 0;
						//nStartErrorAddr = 0;
						nStartErrorAddr = RealDataAddr;
						nErrorLen = DataBytes;

						nStep = 0;
						nBeginErrorRow = LineNum;
						m_pOutput->Warning(strMsg.toLocal8Bit().data());

						hasOverlap = false;
					}
					else {
						if (RealDataAddr < nStartErrorAddr + nErrorLen) {
							hasOverlap = true;
						}
						nErrorLen += DataBytes;
						++nStep;
					}
				}

				nPreErrorRow = LineNum;
				nCurrErrorAddr = RealDataAddr;
			}

			//////////////////////////////////////////////////////////
			if (bExistFlashTable) {
				tFlashTableInfo  FlashTableInfo;
				FlashTableInfo.Address = RealDataAddr;
				FlashTableInfo.Len = DataBytes;

				if (DataBytes > 0) {
					int nWriteCnt = (0x7F + DataBytes) / MAX_FLASHTABLE_SIZE;

					for (int k = 0; k < nWriteCnt; k++) {
						FlashTableInfo.Address = RealDataAddr + k * 0x80;

						FlashTableInfo.Len = MAX_FLASHTABLE_SIZE;
						if (nWriteCnt == k + 1) {//
							FlashTableInfo.Len = DataBytes % MAX_FLASHTABLE_SIZE;
						}
						int nRealLen = FlashTableInfo.Len;
						///////////////////////////////////////////////////////////////////////////////////////////////
						if (m_StartDataTblAdrr == -1) {//首次
							m_StartDataTblAdrr = FlashTableInfo.Address;
							m_SumLen = FlashTableInfo.Len;
						}
						else {
							unsigned long CurAddr = FlashTableInfo.Address;

							if (CurAddr == m_SumLen + m_StartDataTblAdrr) { //是连续的
								m_SumLen += FlashTableInfo.Len;

								if (m_SumLen >= MAX_FLASHTABLE_SIZE) {//最大长度为0x80;
									int nLeftLen = m_SumLen - MAX_FLASHTABLE_SIZE /*-1*/;

									FlashTableInfo.Address = m_StartDataTblAdrr;
									FlashTableInfo.Len = MAX_FLASHTABLE_SIZE;
									/////////////////////////////////////////////////////////////////////////////////////////////////
									m_PreDataTblAddr = FlashTableInfo.Address;

									/////////////////////////////////////////////////////////////////////////////////////////////////
									if (CheckFlashTableInfo(FlashTableStartAddr + m_FlashTableOff, FlashTableInfo) != true) {
										ferror = C_Err_FormatError;
										goto __end;
									}
									/////////////////////////////////////////////////////////////////////////////////////////////////

									BufferWrite(FlashTableStartAddr + m_FlashTableOff, (unsigned char*)(&FlashTableInfo), sizeof(tFlashTableInfo));
									m_FlashTableOff += sizeof(tFlashTableInfo);
									////////////////////////////////////////////////////////////////////////////////////////////////

									m_StartDataTblAdrr += (MAX_FLASHTABLE_SIZE/*+1*/); //更新起始地址
									m_SumLen = 0; //长度重置为0

									if (nLeftLen > 0) {
										m_SumLen += nLeftLen;
									}
								}
							}
							else {//非连续
							   /*tDataTbl tb;
							   tb.DataAddr = m_StartDataTblAdrr;
							   tb.DataLen = m_SumLen;*/
								FlashTableInfo.Address = m_StartDataTblAdrr;
								FlashTableInfo.Len = m_SumLen;

								if (m_SumLen == 0) { //新 地址刚好上次已经写过就不需要再写
									m_SumLen = nRealLen /*FlashTableInfo.Len*/;
									m_StartDataTblAdrr = CurAddr;
								}
								else {
									/////////////////////////////////////////////////////////////////////////////////////////////////
									m_PreDataTblAddr = FlashTableInfo.Address;

									////////////////////////////////////////////////////////////////////////////////////////////////
									if (CheckFlashTableInfo(FlashTableStartAddr + m_FlashTableOff, FlashTableInfo) != true) {
										ferror = C_Err_FormatError;
										goto __end;
									}
									/////////////////////////////////////////////////////////////////////////////////////////////////

									BufferWrite(FlashTableStartAddr + m_FlashTableOff, (unsigned char*)(&FlashTableInfo), sizeof(tFlashTableInfo));
									m_FlashTableOff += sizeof(tFlashTableInfo);
									////////////////////////////////////////////////////////////////////////////////////////////////

									m_StartDataTblAdrr = CurAddr; //不连续的重新赋值
									m_SumLen = nRealLen /*DataTbl.DataLen*/; //赋值为当前的地址和长度
								}
							}
						}
						///////////////////////////////////////////////////////////////////////////////////////////////

							/*m_pDataBuffer->BufWrite(FlashTableStartAddr + FlashTableOff, (BYTE*)(&FlashTableInfo),  sizeof(tFlashTableInfo));
							FlashTableOff += sizeof(tFlashTableInfo);*/
					}
				}
				else {
					////////////////////////////////////////////////////////////////////////////////////////////////
					if (CheckFlashTableInfo(FlashTableStartAddr + m_FlashTableOff, FlashTableInfo) != true) {
						ferror = C_Err_FormatError;
						goto __end;
					}
					////////////////////////////////////////////////////////////////////////////////////////////////
					BufferWrite(FlashTableStartAddr + m_FlashTableOff, (unsigned char*)(&FlashTableInfo), sizeof(tFlashTableInfo));
					m_FlashTableOff += sizeof(tFlashTableInfo);
				}
			}
			//////////////////////////////////////////////////////////

			//if (m_pIFileALVInfo) {
			//	DataALVInfo.Address = RealDataAddr;
			//	DataALVInfo.Length = DataBytes;

			//	if (m_pIFileALVInfo->WriteALV(DataALVInfo) == FALSE) {
			//		strMsg.Format("Save ALV Information To FileAVLInfo Buffer Failed");
			//		m_pOutput->Log(LOG_ERROR, strMsg);
			//		Ret = -1; goto __end;
			//	}
			//}
		}
	
		UpdateProgress();
	}

	////最后 一次写入
	if (bExistFlashTable) {
		tFlashTableInfo  FlashTableInfo;
		FlashTableInfo.Address = m_StartDataTblAdrr;
		FlashTableInfo.Len = m_SumLen;

		if ((m_PreDataTblAddr != m_StartDataTblAdrr) && m_SumLen > 0) { //不一致则证明需要写最后一次
			BufferWrite(m_FlashTableStartAddr + m_FlashTableOff, (unsigned char*)(&FlashTableInfo), sizeof(tFlashTableInfo));

			if (CheckFlashTableInfo(m_FlashTableStartAddr + m_FlashTableOff, FlashTableInfo) != true) {
				ferror = C_Err_FormatError;
				goto __end;
			}
		}
	}


__ParserEnd:
	if (nBeginErrorRow != 0) {
		QString strLineInfo;
		strLineInfo = QString("Line[%1]").arg(nBeginErrorRow);

		unsigned long nErrorAddr = nCurrErrorAddr;

		if (nStep != 0) {
			strLineInfo = QString("Line[%1] - Line[%2] ").arg(nPreErrorRow - nStep, nPreErrorRow);
			nErrorAddr = nStartErrorAddr;
		}

		QString strOverlap = "";
		if (hasOverlap) {
			strOverlap = "There Is Overlapped In Buffer,";
		}
		char buf[250];
		sprintf(buf, "%s Buffer write error,%s Addr:0x%I64X, Len:0x%X, RealBytesWrite:0x%X", strLineInfo.toStdString().c_str(), strOverlap.toStdString().c_str(), nErrorAddr, nErrorLen, 0);
		strMsg = buf;
		m_pOutput->Warning(strMsg.toLocal8Bit().data());

		nBeginErrorRow = 0;
	}
	char buf[250];
	sprintf(buf,"Range of Address [0x%I64X-0x%I64X] Byte", BeginAddr, EndAddr);
	strMsg = buf;
	/*m_pOutput->Log(LOG_OUTPUT, strMsg);
	m_pOutput->SetProgPos(100);*/

__end:
	if (nBeginErrorRow != 0) {
		QString strLineInfo;
		strLineInfo = QString("Line[%1]").arg(nBeginErrorRow);

		unsigned long nErrorAddr = nCurrErrorAddr;

		if (nStep != 0) {
			strLineInfo = QString("Line[%1] - Line[%2] ").arg(nPreErrorRow - nStep, nPreErrorRow);
			nErrorAddr = nStartErrorAddr;
		}

		QString strOverlap = "";
		if (hasOverlap) {
			strOverlap = "There Is Overlapped In Buffer,";
		}
		char buf[250];
		sprintf(buf, "%s Buffer write error,%s Addr:0x%I64X, Len:0x%X, RealBytesWrite:0x%X", strLineInfo.toStdString().c_str(), strOverlap.toStdString().c_str(), nErrorAddr, nErrorLen, 0);
		strMsg = buf;
		m_pOutput->Warning(strMsg.toLocal8Bit().data());
	}

	if (ferror != 0) {
		strMsg = QString("Line[%1] Parser Failed").arg(LineNum);
		m_pOutput->Error(strMsg.toLocal8Bit().data());
	}
	if (pTmpBuf) {
		delete[] pTmpBuf;
	}
	if (File.isOpen())
		File.close();
	return ferror;
}

///確定校验值，最后一个字节是Sum值，返回0表示成功，返回-1表示失败
int HexParser::CheckSumCompare(unsigned char* pData, int Size)
{
	int i = 0;
	unsigned char Sum = 0;
	for (i = 0; i < Size - 1; ++i) {
		Sum += pData[i];
	}
	if (((Sum + pData[Size - 1]) & 0xFF) == 0) {
		return 0;
	}
	else {
		QString strErr;
		strErr = QString("Checksum Compare Error, ChksumGet:0x%1X, ChksumCalc:0x%2X").arg(QString::number(pData[Size - 1], 16), QString::number(0x100 - Sum));
		
		return -1;
	}
}