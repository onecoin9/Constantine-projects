#include "PmfParse.h"
#include "..\public\common\Serialize.h"
#include "..\public\DrvDllCom\ComTool.h"
#define DATATYPE_NORMAL         (0)
#define DATATYPE_CONFIG_TABLE	(1)
#define DATATYPE_CONFIG_MULTI	(2)
#define DATATYPE_CRC			(3)

#define WRITE_BUFFER_CRC_START	(0x803FFC)
#define WRITE_BUFFER_TXT_START	(0x800000)

PmfParser::PmfParser()
{
	m_vTable.clear();
	OffsetTableInfo = 0x10000;
}


char* PmfParser::getVersion()
{
	return "1.0.0.0";
}
char* PmfParser::getFormatName()
{
	return C_PMF;
}
bool PmfParser::ConfirmFormat(QString& filename)
{
	if (QFileInfo(filename).suffix().compare("pmf", Qt::CaseInsensitive) == 0)
		return true;
	return false;
}

void PmfParser::ClearTable(QVector<tPMFTable>& vTable)
{
	int i;
	for (i = 0; i < vTable.size(); ++i) {
		if (vTable[i].pData != NULL) {
			delete[] vTable[i].pData;
		}
	}
	vTable.clear();
}

int PmfParser::GetTableCRC(unsigned char* pData, int Size, unsigned short& CRCSum)
{
	CRCSum = 0;
	for (int i = 0; i < Size; ++i) {
		CRCSum += pData[i];
	}
	return 0;
}

int PmfParser::SaveTableInfoToBuffer(tPMFTable* pTable)
{
	int Ret = 0;
	unsigned short CRCSum = 0;
	unsigned char Target = 0x55;
	CSerial lSerial;
	lSerial << Target << pTable->Key;
	lSerial << pTable->Length;
	if (pTable->Length > PMFDATA_STATIC_LEN) {
		lSerial.SerialInBuff(pTable->pData, pTable->Length);
	}
	else {
		lSerial.SerialInBuff(pTable->Data, pTable->Length);
	}
	GetTableCRC(lSerial.GetBuffer(), lSerial.GetLength(), CRCSum);
	lSerial << CRCSum;
	const unsigned char* tmpptr = lSerial.GetBuffer();
	int tmplen = lSerial.GetLength();
	BufferWrite(OffsetTableInfo, lSerial.GetBuffer(), lSerial.GetLength());
	OffsetTableInfo += lSerial.GetLength();
	return Ret;
}


tPMFTable* PmfParser::GetPMTTable(QString strLine, QVector<tPMFTable>& vTable)
{
	int i;

	for (i = 0; i < vTable.size(); ++i) {
		if (strLine.indexOf(vTable[i].strConfig, 0) > 0) {///找到了对应的Table记录
			return &vTable[i];
		}
	}
	return NULL;
}
int PmfParser::GetTableKey(tPMFTable* pTable)
{
	if (pTable->strTblID == "xv1Data") {
		pTable->Key = 0x9C;
	}
	else if (pTable->strTblID == "xv2Data") {
		pTable->Key = 0x9E;
	}
	else if (pTable->strTblID == "xv3Data") {
		pTable->Key = 0xA0;
	}
	else if (pTable->strTblID == "xv4Data") {
		pTable->Key = 0xA2;
	}
	else if (pTable->strTblID == "xv5Data") {
		pTable->Key = 0xA4;
	}
	else if (pTable->strTblID == "xv6Data") {
		pTable->Key = 0xA6;
	}
	else if (pTable->strTblID == "xv7Data") {
		pTable->Key = 0xA8;
	}
	else if (pTable->strTblID == "xv8Data") {
		pTable->Key = 0xAA;
	}
	else if (pTable->strTblID == "id1Data") {
		pTable->Key = 0xAB;
	}
	else if (pTable->strTblID == "id2Data") {
		pTable->Key = 0xAC;
	}
	else if (pTable->strTblID == "id3Data") {
		pTable->Key = 0xAD;
	}
	else if (pTable->strTblID == "id4Data") {
		pTable->Key = 0xAE;
	}
	else {
		return -1;
	}
	return 0;
}

int PmfParser::GetTableCfg(QString& strLine, QVector<tPMFTable>& vTable)
{
	int Length = strLine.length();
	int i, Phase = 0;
	char ch;
	tPMFTable TblCfg;
	for (i = 0; i < Length; ++i) {
		ch = strLine[i].cell();
		if (ch == 0x0D || ch == 0x0A) {
			break;
		}
		if (ch == '/') {
			continue;
		}
		else if (ch == ' ' && TblCfg.strConfig == "") {///将Config部分之前的空格忽略
			continue;
		}
		else if (ch == '-') {///解析到这个则认为后面就是Config
			Phase = 1;
		}
		else {
			if (Phase == 0) {
				TblCfg.strTblID.append(ch);
			}
			else if (Phase == 1) {
				TblCfg.strConfig.append(ch);
			}
		}
	}
	if (TblCfg.strTblID != "") {
		vTable.push_back(TblCfg);
	}
	return 0;
}


int PmfParser::ParserOneLine(QString& strLine, unsigned short* pAddr, unsigned short* pData)
{
	int Ret = 0, i;
	char ch;
	bool bAddrGet = false, bDataGet = false;
	int Length = strLine.length();
	*pAddr = 0;
	for (i = 0; i < Length; ++i) {///开始提取地址
		ch = strLine[i].cell();
		if (isdigit(ch)) {
			*pAddr = (*pAddr) * 16 + ch - '0';
		}
		else if (isupper(ch)) {
			*pAddr = (*pAddr) * 16 + ch - 'A' + 10;
		}
		else if (islower(ch)) {
			*pAddr = (*pAddr) * 16 + ch - 'a' + 10;
		}
		else {
			break;
		}
	}

	for (; i < Length; ++i) {
		ch = strLine[i].cell();
		if (ch == '0' || ch == '1') {///忽略地址和数据之间的空格或者其他分割符号
			break;
		}
		else {
			continue;
		}
	}
	*pData = 0;
	for (; i < Length; ++i) {///开始提取数据
		ch = strLine[i].cell();
		if (ch == '0' || ch == '1') {
			*pData = (*pData) * 2 + ch - '0';
		}
		else if (ch == 'x' || ch == 'X') {
			*pData = (*pData) * 2;
		}
		else {
			break;
		}
	}
	return Ret;
}



int PmfParser::WriteDataToPMFTable(tPMFTable* pTable, unsigned short* pAddr, unsigned short* pData)
{
	int Ret = 0;
	unsigned char* pTmpData = NULL, * pNewData = NULL;
	pTable->Length += 4;

	if (pTable->Length <= PMFDATA_STATIC_LEN) {
		pTmpData = pTable->Data;
	}
	else {
		pTable->MaxSize += PMFDATA_STATIC_LEN;
		if (pTable->pData == NULL) {
			pTable->pData = new unsigned char[pTable->MaxSize];
			if (pTable->pData) {//将数据拷贝过来
				memcpy(pTable->pData, pTable->Data, PMFDATA_STATIC_LEN);
			}
			else {
				Ret = -1; goto __end;
			}
		}
		else {
			pNewData = new unsigned char[pTable->MaxSize];
			if (pNewData) {//将数据拷贝过来
				memcpy(pNewData, pTable->pData, pTable->MaxSize - PMFDATA_STATIC_LEN);
				delete[] pTable->pData; ///释放原来的内存
				pTable->pData = pNewData;///重新赋值
			}
			else {
				Ret = -1; goto __end;
			}
		}
		pTmpData = pTable->pData;
	}
	*(unsigned short*)(pTmpData + pTable->Offset) = *pAddr;
	pTable->Offset += 2;
	*(unsigned short*)(pTmpData + pTable->Offset) = *pData;
	pTable->Offset += 2;

__end:
	return Ret;
}

int PmfParser::CheckTableDataReady(QVector<tPMFTable>& vTable)
{
	int i;
	QString strMsg;
	for (i = 0; i < vTable.size(); ++i) {
		if (vTable[i].strConfig != "" && vTable[i].Length == 0) {//表中有相应的Config配置,但又找不到CONFIG_MULTI部分
			strMsg = strMsg + "Can't Find Config Multi Associated With %s" +vTable[i].strConfig;
			m_pOutput->Warning(strMsg.toLocal8Bit().data());
		}
	}
	return 0;
}

int PmfParser::TransferFile(QString& srcfn, QIODevice* dst)
{
	m_Dst = dst;
	int Ret = 0;
	int LineNum = 1;
	int Offset = 0;
	QString strMsg, strLine;
	QFile File(srcfn);
	unsigned int Addr = 0;
	unsigned int BytesWrite = 0;
	unsigned short LineData[2];
	int TxtWriteOffset = 0;
	bool TxtWriteEnd = false;
	DataType = DATATYPE_NORMAL;
	ClearTable(m_vTable);
	tPMFTable* pPMFTable = NULL;

	if (!File.open(QIODevice::ReadOnly)) {
		ferror = C_Err_FileOpen;
		goto __end;
	}

	InitControl();
	FileSize = File.size();

	while (!File.atEnd()) {
		myfgets(&File);
		strLine = (char*)work;
		if (strLine[0] == 0x0D || strLine[0] == '/' || strLine[0] == 0x0A) {
			if (strLine.indexOf("//%CONFIG_TABLE", 0) == 0) {///找到Table表
				if (strLine.indexOf("//%CONFIG_TABLE_END", 0) == 0) {
					DataType = DATATYPE_NORMAL;
				}
				else {
					DataType = DATATYPE_CONFIG_TABLE;
				}
			}
			else if (strLine.indexOf("//%CONFIG_MULTI", 0) == 0) {///找到Multi
				if (strLine.indexOf("//%CONFIG_MULTI_END", 0) == 0) {
					if (DataType = DATATYPE_CONFIG_MULTI) {
						if (pPMFTable) {
							SaveTableInfoToBuffer(pPMFTable);
						}
					}
					DataType = DATATYPE_NORMAL;
				}
				else {
					DataType = DATATYPE_CONFIG_MULTI;
					pPMFTable = GetPMTTable(strLine, m_vTable);
					if (pPMFTable == NULL) {///数据有，但是表中没有相应的引用，直接报Warning
						strMsg = QString("Can't Find Config In The Table At Line %1").arg(LineNum);
						m_pOutput->Warning(strMsg.toLocal8Bit().data());
					}
					else {
						Ret = GetTableKey(pPMFTable);
						if (Ret != 0) {
							strMsg = QString("Translate Table Key Failed At Line %1").arg(LineNum);
							m_pOutput->Error(strMsg.toLocal8Bit().data());
							ferror = C_Err_FormatError; goto __end;
						}
					}
				}
			}
			else if (strLine.indexOf("//%CRC", 0) == 0) {///find CRC
				if (strLine.indexOf("//%CRC_END", 0) == 0) {
					DataType = DATATYPE_NORMAL;
				}
				else {
					DataType = DATATYPE_CRC;
				}
				int txtLength = strLine.length();
				int writeBytes =BufferWrite(WRITE_BUFFER_TXT_START + TxtWriteOffset, work, txtLength);
				TxtWriteOffset += writeBytes;
			}
			else if (strLine.indexOf("//%RAIL_END") == 0 && !TxtWriteEnd) {
				int txtLength = strLine.length();
				int writeBytes = BufferWrite(WRITE_BUFFER_TXT_START + TxtWriteOffset, work, txtLength);
				TxtWriteOffset += writeBytes;
				TxtWriteEnd = true;
			}
			else {
				if (DataType == DATATYPE_CONFIG_TABLE) {
					Ret = GetTableCfg(strLine, m_vTable);
					if (Ret != 0) {
						strMsg = QString("Get Table Config Failed At Line %1").arg(LineNum);
						m_pOutput->Error(strMsg.toLocal8Bit().data());
						ferror = C_Err_FormatError; goto __end;
					}
				}
				else if (DataType == DATATYPE_CRC) {
					//CRC need write
					QString txtString = strLine;
					int txtLength = txtString.length();
					int writeBytes = BufferWrite(WRITE_BUFFER_TXT_START + TxtWriteOffset, work, txtLength);
					TxtWriteOffset += writeBytes;
					//E1610C - 0xB4 - 599488CD
					QStringList array = strLine.split("-");
					if (array.size() != 3) {
						QString strErr;
						strErr = QString("Buffer write Data error,LineNum=%1 not correct crc line %2").arg(LineNum).arg(Addr);
						m_pOutput->Log(strErr.toLocal8Bit().data());
						ferror = C_Err_FormatError; goto __end;
					}
					QString crcData = array[2];
					crcData = crcData.trimmed();

					QVector<unsigned char> vHexData;
					if (ComTool::Str2Hex(crcData, ComTool::ENDIAN_LIT, vHexData) == false) {
						m_pOutput->Log("String to hex transfer failed, please check");
						ferror = C_Err_FormatError; goto __end;
					}
					int nSize = vHexData.size();
					for (int i = 0; i < nSize; i++) {
						BufferWrite(WRITE_BUFFER_CRC_START + i, (unsigned char*)&vHexData[i], 1);
					}

					DataType = DATATYPE_NORMAL;
				}
				else {
					if (!TxtWriteEnd) {
						//comment
						int txtLength = strLine.length();
						int writeBytes = BufferWrite(WRITE_BUFFER_TXT_START + TxtWriteOffset, work, txtLength);
						TxtWriteOffset += writeBytes;
					}
				}
			}
			LineNum++;
			continue;
		}

		if (DataType == DATATYPE_CONFIG_MULTI) {
			if (pPMFTable) {
				Ret = ParserOneLine(strLine, &LineData[0], &LineData[1]);
				if (Ret == 0) {
					Ret = WriteDataToPMFTable(pPMFTable, &LineData[0], &LineData[1]);
					if (Ret != 0) {
						strMsg = QString("Save Config Multi At Line %1 Failed").arg(LineNum);
						m_pOutput->Error(strMsg.toLocal8Bit().data());
						ferror = C_Err_WriteError; goto __end;
					}
				}
				else {
					strMsg = QString("Parser Config Multi At Line %d Failed").arg(LineNum);
					m_pOutput->Error(strMsg.toLocal8Bit().data());
					ferror = C_Err_WriteError; goto __end;
				}
			}
		}
		else {
			Ret = ParserOneLine(strLine, &LineData[0], &LineData[1]);
			BytesWrite = BufferWrite(Addr, (unsigned char*)LineData, 4);
			Addr += 4;
			if (BytesWrite != 4) {
				char buf[200];
				sprintf(buf, "Buffer write Data error,LineNum=%d Addr=0x%X, len=0x%X, really write bytes=0x%X", LineNum, Addr, 1, BytesWrite);
				m_pOutput->Log(buf);
				ferror = C_Err_WriteError; goto __end;
			}
		}
		LineNum++;
		UpdateProgress();
	}

	CheckTableDataReady(m_vTable);
__end:
	OffsetTableInfo = 0x10000;
	if (ferror != 0) {
		char buf[200];
		sprintf(buf, "Line %1 Format Error", LineNum);
		m_pOutput->Error(buf);
	}
	if (File.isOpen())
		File.close();
	return ferror;

}