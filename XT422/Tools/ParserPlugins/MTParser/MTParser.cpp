#include "MTParser.h"
#include "..\public\DrvDllCom\ComTool.h"
#include <QMessageBox>

enum {
	REG_COLUMN_DEC = 0,
	DATA_COLUMN_DEC = 1,
	PAGE_COLUMN = 2,
	TYPE_NUMBER = 3,
};

#define MAX_COLUMN_NUMBER		(4)
#define DATA_WRITE_OFFSET		(0x0)
#define MAX_DATA_SIZE			(8)
#define PARSER_NAME_SIZE		(32)
#define END_FLAG				(0xFFFFFFFF)
#define PARSER_NAME "MTParser"
#define PARSER_VERSION "V000"

typedef struct TagNewDestDataFmt {
	unsigned int PageNum;
	unsigned int RegAddr;
	unsigned int CmdType;
	unsigned int Len;
	unsigned char Data[MAX_DATA_SIZE];
	void ReInit() {
		RegAddr = 0x0;
		PageNum = 0x0;
		CmdType = 0x0;
		Len = 0x00;
		memset(Data, 0, MAX_DATA_SIZE);
	};
	TagNewDestDataFmt() {
		ReInit();
	};
}tNDestDataFmt;


inline bool IsStrAllZeros(const char* str) {
	if (str == NULL) {
		return false; 
	}
	while (*str != '\0') {
		if (*str++ != '0') {
			return false; 
		}
	}
	return true; 
}

QString decimalStringToHex(const QString& decimalString) {
	bool ok;
	// 将十进制字符串转换为整数
	qint64 value = decimalString.toLongLong(&ok);
	if (!ok) {
		// 转换失败处理
		return QString();
	}
	// 将整数转换为十六进制字符串
	return QString::number(value, 16);
}


MTParser::MTParser()
{
}

char* MTParser::getVersion()
{
	return "1.0.0.0";
}
char* MTParser::getFormatName()
{
	return C_MTPARSER;
}
bool MTParser::ConfirmFormat(QString& filename)
{
	bool ret = false;
	int Rtn = 0, LineNum = 0;
	QFile File(filename);
	QString strOneLine;
	int lineNum = 0;
	QString strMsg;


	if (QFileInfo(filename).suffix().compare("txt", Qt::CaseInsensitive) != 0) {
		goto __end;
	}

	if (!File.open(QIODevice::ReadOnly)) {
		ferror = C_Err_FileOpen;
		goto __end;
	}

	int dataLine = 0;
	bool hasEnd = false, hasHeader = false;

	while (!File.atEnd()) {
		strOneLine = File.readLine();
		lineNum++;
		if (strOneLine.isEmpty()) {
			continue;
		}

		strOneLine = strOneLine.replace("\r\n", ""); // 0x0d0a
		strOneLine = strOneLine.mid(strOneLine.indexOf(QRegExp("\\S")));
		if (strOneLine.length() == 0) {
			continue;
		}

		if ((strOneLine.left(8)).compare("Register", Qt::CaseInsensitive) == 0) {
			hasHeader = true;
			continue;
		}

		if ((strOneLine.left(3)).compare("END", Qt::CaseInsensitive) == 0) {
			hasEnd = true;
			break;
		}

		QStringList destDataArray;
		destDataArray = strOneLine.split("\t");
		if (destDataArray.count() != MAX_COLUMN_NUMBER) {
			goto __end;
		}

		dataLine++;
	}

	if (hasEnd && hasHeader) {
		ret = true; goto __end;
	}
	else {
		ret = false; goto __end;
	}

__end:
	if (File.isOpen())
		File.close();
	return ret;
}
int MTParser::TransferFile(QString& srcfn, QIODevice* dst)
{
	int ferror = 0;
	bool StopUpdate = false;
	QString strMsg;
	int valueConv = 0;
	unsigned short sum = 0;
	QFile File(srcfn);
	QString strOneLine;
	QString temp;
	QVector<QString> vItemColList;
	QVector<unsigned char> vHexData;

	unsigned long CRC_CHECK_STARTAddr = 0;
	unsigned int uCrcOff = 0;

	QString strTempRowSrc;
	int nItemColCnt = 0;

	int off = DATA_WRITE_OFFSET;
	int oneItemDataLen = 0;
	int oneItemDataIdx = 0;
	int LineNum = 0;

	QString strConvTemp;
	unsigned int regAddrHex;
	unsigned int TypeHex;
	QStringList destDataArray;

	QMessageBox msgBox;
	

	if (File.open(QIODevice::ReadOnly) == false) {
		strMsg = QString("Open File Failed: %1").arg(srcfn);
		m_pOutput->Error(strMsg.toLocal8Bit().data());
		ferror = C_Err_FileOpen; goto __end;
	}


	InitControl();
	FileSize = File.size();
	m_Dst = dst;


	while (!File.atEnd()) {
	
		myfgets(&File);
		strOneLine = (char*)work;
		LineNum++;
		if (strOneLine.isEmpty()) {
			continue;
		}

		strOneLine = strOneLine.replace("\r\n", ""); // 0x0d0a
		strOneLine = strOneLine.trimmed();

		if (strOneLine.length() == 0) {
			continue;
		}

		if ((strOneLine.left(3)).compare("END", Qt::CaseInsensitive) == 0) {
			StopUpdate = true;
			//以0xFFFFFFFF为结束标记
			unsigned int EndFlag = END_FLAG;
			BufferWrite(off, (unsigned char*)&EndFlag, 4);
			break;
		}

		if (LineNum == 1){
			continue;
		}

		tNDestDataFmt DestRecordOneItem; 
		DestRecordOneItem.ReInit();

		destDataArray = strOneLine.split("\t");
		if (destDataArray.count() != MAX_COLUMN_NUMBER) {
			ferror = C_Err_FormatError;
			strMsg = QString("%1 %2.LineNum:%3").arg("data column must be ").arg(MAX_COLUMN_NUMBER).arg(LineNum);
			msgBox.setText(strMsg);
			msgBox.exec();
			//m_pOutput->Error(strMsg.toLocal8Bit().data());
			goto __end;
		}

		bool ok = false;

		//1. Page

		if (!std::all_of(destDataArray[PAGE_COLUMN].begin(), destDataArray[PAGE_COLUMN].end(), [](QChar ch) {
			return ch >= QChar('0') && ch <= QChar('9'); })) {
			ferror = C_Err_FormatError;
			strMsg = QString("Page column must be digits (0-9) at line %1.").arg(LineNum);
			msgBox.setText(strMsg);
			msgBox.exec();
			//m_pOutput->Error(strMsg.toLocal8Bit().data());
			goto __end;
		}
		DestRecordOneItem.PageNum = destDataArray[PAGE_COLUMN].toInt();

		//2. reg addr
		regAddrHex = (unsigned int)destDataArray[REG_COLUMN_DEC].toLong(NULL, 10);
		if (regAddrHex == 0) {
			const char* pBuffer = destDataArray[REG_COLUMN_DEC].toLocal8Bit().data();
			if (IsStrAllZeros(pBuffer)) {
				strMsg = QString("%1 Max Column: %2. LineNum:%3").arg("Reg number column is all zero. ").arg(MAX_COLUMN_NUMBER).arg(LineNum);
				//m_pOutput->Log(strMsg.toLocal8Bit().data());
			}
			else {
				ferror = C_Err_FormatError;
				strMsg = QString("%1 %2. LineNum:%3").arg("reg number column transfer failure ").arg(MAX_COLUMN_NUMBER).arg(LineNum);
				msgBox.setText(strMsg);
				msgBox.exec();
				//m_pOutput->Error(strMsg.toLocal8Bit().data());
				goto __end;
			}
		}
		DestRecordOneItem.RegAddr = regAddrHex;

		//3. type number
		DestRecordOneItem.CmdType = (unsigned int)destDataArray[TYPE_NUMBER].toLong(NULL, 16);

		//4. DATA_COLUMN_HEX
		vHexData.clear();
		QString hexData = decimalStringToHex(destDataArray[DATA_COLUMN_DEC]);
		if (ComTool::Str2Hex(hexData, ComTool::ENDIAN_LIT, vHexData) == false) {
			ferror = C_Err_FormatError;
			strMsg = QString("String to hex transfer failed, please check line:%1 %2").arg(LineNum).arg(hexData);
			msgBox.setText(strMsg);
			msgBox.exec();
			//m_pOutput->Error(strMsg.toLocal8Bit().data());
			goto __end;
		}
		if (vHexData.size() > MAX_DATA_SIZE) {
			ferror = C_Err_FormatError;
			strMsg = QString("data column hex length is too long, please check line:%1 %2").arg(LineNum).arg(hexData);
			msgBox.setText(strMsg);
			msgBox.exec();
			//m_pOutput->Error(strMsg.toLocal8Bit().data());
			goto __end;
		}
		std::copy(vHexData.begin(), vHexData.end(), DestRecordOneItem.Data);

		//5. LEN
		DestRecordOneItem.Len = vHexData.size();

		BufferWrite(off, (unsigned char*)&DestRecordOneItem.PageNum, 4);
		off += 4;
		BufferWrite(off, (unsigned char*)&DestRecordOneItem.RegAddr, 4);
		off += 4;
		BufferWrite(off, (unsigned char*)&DestRecordOneItem.CmdType, 4);
		off += 4;
		BufferWrite(off, (unsigned char*)&DestRecordOneItem.Len, 4);
		off += 4;
		BufferWrite(off, DestRecordOneItem.Data, DestRecordOneItem.Len);
		off += DestRecordOneItem.Len;

		UpdateProgress();

	}


__end:
	if (File.isOpen())
		File.close();
	return ferror;
}
