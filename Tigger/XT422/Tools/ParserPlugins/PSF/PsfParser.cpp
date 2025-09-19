#include "PsfParser.h"
#include "../public/DrvDllCom/ComTool.h"

#define WRITE_BUFFER_CRC_START	(0x803FFC)
#define WRITE_BUFFER_TXT_START	(0x800000)
PsfParser::PsfParser()
{
}


char* PsfParser::getVersion() {
	return "1.0.0.0";
}
char* PsfParser::getFormatName()
{
	return C_PSF;
}
bool PsfParser::ConfirmFormat(QString& filename)
{
	bool Ret = true;

	if (!QFileInfo(filename).suffix().compare("psf", Qt::CaseInsensitive)) {
		Ret = true;
	}
	else {
		Ret = false;
	}
	return Ret;
}
int PsfParser::TransferFile(QString& srcfn, QIODevice* dst)
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
	bool startParseCRC = false;
	bool TxtWriteEnd = false;
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
			LineNum++;

			if (!TxtWriteEnd && strLine.indexOf("//", 0) == 0) {
				//comment
				int txtLength = strLine.length();
				int writeBytes = BufferWrite(WRITE_BUFFER_TXT_START + TxtWriteOffset, work, txtLength);
				TxtWriteOffset += writeBytes;
			}
			if (strLine.indexOf("//%RAIL_END") == 0) {
				TxtWriteEnd = true;
			}
			else if (strLine.indexOf("//%CRC", 0) == 0) {
				if (strLine.indexOf("//%CRC_END", 0) == 0) {
					startParseCRC = false;
				}
				else {
					startParseCRC = true;
				}
			}
			else if (startParseCRC && strLine.indexOf("//", 0) == 0) {

				//E1610C - 0xB4 - 599488CD
				int txtLength = strLine.length();
				int writeBytes = BufferWrite(WRITE_BUFFER_TXT_START + TxtWriteOffset, work, txtLength);
				TxtWriteOffset += writeBytes;

				QStringList array = strLine.split("-");
				if (array.count() != 3) {
					QString strErr;
					strErr = QString("Buffer write Data error,LineNum=%1 not correct crc line %2").arg(LineNum).arg(strLine);
					m_pOutput->Error(strMsg.toLocal8Bit().data());
					ferror = C_Err_FormatError; goto __end;
				}
				QString crcData = array[2];
				crcData = crcData.trimmed();

				QVector<unsigned char> vHexData;
				if (ComTool::Str2Hex(crcData, ComTool::ENDIAN_LIT, vHexData) == false) {
					QString strErr;
					strErr = "String to hex transfer failed, please check";
					m_pOutput->Error(strMsg.toLocal8Bit().data());
					ferror = C_Err_FormatError; goto __end;
				}
				int nSize = vHexData.size();
				for (int i = 0; i < nSize; i++) {
					BufferWrite(WRITE_BUFFER_CRC_START + i, (unsigned char*)&vHexData[i], 1);
				}


			}
			continue;
		}
		Ret = ParserOneLine(strLine, &LineData[0], &LineData[1]);
		BytesWrite = BufferWrite(Addr, (unsigned char*)LineData, 4);
		Addr += 4;
		if (BytesWrite != 4) {
			char buf[200];
			sprintf(buf, "Buffer write Data error,LineNum=%d Addr=0x%X, len=0x%X, really write bytes=0x%X", LineNum, Addr, 1, BytesWrite);
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			Ret = C_Err_WriteError; goto __end;
		}

		UpdateProgress();
		LineNum++;
	}

	if (ferror != 0) {
		strMsg = QString("Line %1 Format Error").arg(LineNum);
		m_pOutput->Error(strMsg.toLocal8Bit().data());
	}
__end:
	if (File.isOpen())
		File.close();
	return ferror;
}


int PsfParser::ParserOneLine(QString& strLine, unsigned short* pAddr, unsigned short* pData)
{
	int i;
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
		else {
			break;
		}
	}
	return 0;
}