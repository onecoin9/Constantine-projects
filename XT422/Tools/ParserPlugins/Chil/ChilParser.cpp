#include "ChilParser.h"
#include <QFileInfo>

#define NOTE_OFFSET (0x8000)
#define CRC8_OFFSET	(0x7FFF) ///CRC8所在位置
ChilParser::ChilParser()
{
}

char* ChilParser::getVersion() {
	return "1.0.0.0";
}
char* ChilParser::getFormatName() {
	return C_CHIL;
}


bool ChilParser::ConfirmFormat(QString& filename) {
	bool Rtn = false;
	QFile File(filename);
	int Ret = 0;
	QString strLine;
	int LineMatch = 0;
	unsigned int Addr, Data, Mask;
	if (!File.open(QIODevice::ReadOnly)) {
		return false;
	}

	QString strExt = QFileInfo(filename).suffix();


	//jwl 这里原来是 == ，改成了 != ，chil档案后缀到底是不是bin
	if (strExt.compare("txt", Qt::CaseInsensitive) != 0) {///先判断后缀名
		return false;
	}

	while (!File.atEnd()) {
		QString strLine = File.readLine();

		if (strLine[0] == 0x0D || strLine[0] == '/' || strLine[0] == 0x0A) {
			continue;
		}
		Ret = ParserOneLine(strLine, Addr, Data, Mask);
		if (Ret == false) {
			Rtn = false; goto __end;
		}
		else {
			if (LineMatch >= 5) {
				break;
			}
			LineMatch++;
		}
	}
__end:
	if (LineMatch >= 5) {
		Rtn = true;
	}
	if (File.isOpen())
		File.close();
	return Rtn;
}


bool ChilParser::ParserOneLine(QString& strLine, unsigned int& Addr, unsigned int& Data, unsigned int& Mask) {
	bool Ret = true;
	int i, j, Length = strLine.length();
	char ch;
	Addr = 0;
	char Tmp[9];
	memset(Tmp, 0, 9);
	j = 0;
	for (i = 0; i < Length; ++i) {///开始提取地址
		ch = strLine[i].cell();
		if (ishex(ch)) {
			if (j >= 9) {
				Ret = false; goto __end;
			}
			Tmp[j++] = ch;
		}
		else {
			break;
		}
	}
	if (sscanf(Tmp, "%X", &Addr) != 1) {
		Ret = false; goto __end;
	}
	for (; i < Length; ++i) {///忽略地址与数据间的空格
		ch = strLine[i].cell();
		if (ishex(ch)) {
			break;
		}
	}

	j = 0;
	memset(Tmp, 0, 9);
	for (; i < Length; ++i) {///开始提取数据
		ch = strLine[i].cell();
		if (ishex(ch)) {
			if (j >= 9) {
				Ret = false; goto __end;
			}
			Tmp[j++] = ch;
		}
		else {
			break;
		}
	}
	if (sscanf(Tmp, "%X", &Data) != 1) {
		Ret = false; goto __end;
	}
	for (; i < Length; ++i) {///忽略Data与Mask间的空格
		ch = strLine[i].cell();
		if (ishex(ch)) {
			break;
		}
	}

	j = 0;
	memset(Tmp, 0, 9);
	for (; i < Length; ++i) {///开始提取Mask
		ch = strLine[i].cell();
		if (ishex(ch)) {
			if (j >= 9) {
				Ret = false; goto __end;
			}
			Tmp[j++] = ch;
		}
		else {
			break;
		}
	}
	if (sscanf(Tmp, "%X", &Mask) != 1) {
		Ret = false; goto __end;
	}

__end:
	return Ret;
}

int ChilParser::TransferFile(QString& srcfn, QIODevice* dst)
{
	m_Dst = dst;
	int Ret = 0;
	int LineNum = 1;
	int Offset = 0;
	QString strMsg;
	QString strLine;
	QFile File(srcfn);
	unsigned int Addr, Data, Mask;
	unsigned int BytesWrite = 0;
	unsigned int NoteOffset = NOTE_OFFSET;
	QString strErr;
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
			if (strLine[0] == '/') {///注释行需要写到固定的位置
				if (strLine.contains("CRC8")) {///找到CRC8
					Ret = SetCRC8(strLine);
					if (Ret != 0) {
						ferror = C_Err_FormatError;
						strErr = "SetCRC8 Failed";
						m_pOutput->Error(strMsg.toLocal8Bit().data());
						goto __end;
					}
				}
				WriteStrLine(strLine, NoteOffset);
			}
			LineNum++;
			continue;
		}
		Ret = ParserOneLine(strLine, Addr, Data, Mask);
		if (Ret == false) {
			ferror = C_Err_FormatError;
			break;
		}
		BytesWrite = BufferWrite(Addr, (unsigned char*)&Data, 1);

		if (BytesWrite != 1) {
			char buf[200];
			sprintf(buf, "Buffer write Data error,LineNum=%d Addr=0x%X, len=0x%X, really write bytes=0x%X", LineNum, Addr, 1, BytesWrite);
			strErr = buf;
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			ferror = C_Err_WriteError;
			goto __end;
		}
		BytesWrite = BufferWrite(Addr + 0x10000, (unsigned char*)&Mask, 1);////将Mask值写到0x10000开始的位置+Addr

		if (BytesWrite != 1) {
			char buf[200];
			sprintf(buf, "Buffer write Mask error,LineNum=%d Addr=0x%X, len=0x%X, really write bytes=0x%X", LineNum, Addr + 0x10000, 1, BytesWrite);
			strErr = buf;
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			ferror = C_Err_WriteError; goto __end;
		}
		LineNum++;
		UpdateProgress();
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


int ChilParser::WriteStrLine(QString& strLine, unsigned int& Offset)
{
	BufferWrite(Offset, (uchar*)strLine.toStdString().c_str(), strLine.length());

	Offset += strLine.length();
	return 0;
}

int ChilParser::SetCRC8(QString& strLine)
{
	int Pos = strLine.indexOf("0x", 0);
	int Ret = 0;
	if (Pos > 0) {
		unsigned int CRC;
		bool ok;
		CRC = strLine.mid(Pos).toInt(&ok, 16);
		if (ok) {
			unsigned char CRC8 = (uchar)CRC;
			BufferWrite(CRC8_OFFSET, (uchar*)&CRC8, 1);
		}
		else {
			Ret = -1;
		}
	}
	else {
		Ret = -1;
	}
	return Ret;
}