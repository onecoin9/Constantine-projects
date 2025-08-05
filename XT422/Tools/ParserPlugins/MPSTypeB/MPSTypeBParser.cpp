#include "MPSTypeBParser.h"
#include "..\public\DrvDllCom\ComTool.h"

enum {
	START_COLUMN = 0,
	PAGE_COLUMN = 1,
	REG_COLUMN_HEX = 2,
	REG_COLUMN_DEC = 3,
	DESC_COLUMN = 4,
	DATA_COLUMN_HEX = 5,
	DATA_COLUMN_DEC = 6,
	TYPE_NUMBER = 7,
	MAX_COLUMN_NUMBER = 8,
};

#define DATA_WRITE_OFFSET		(0x100)
#define HEAD_WRITE_OFFSET		(0x0)
#define MAX_DATA_SIZE			(8)
#define PARSER_NAME_SIZE		(32)
#define FOUR_DIGI_C0DE			(0x0060)
#define CMD_TYPE				(0x00014234)
#define PARSER_NAME "MPSParserTypeB"
#define PARSER_VERSION "V000"
typedef struct HeadDataFmt {
	char  ParserName[PARSER_NAME_SIZE];
	char  Version[PARSER_NAME_SIZE];
	char  ProductID[PARSER_NAME_SIZE];
	unsigned int I2CAddress;
	unsigned int FourDigiCode;
	unsigned int MpsDataLen;				// MpsData 所有数据长度   截至end
	unsigned int MpsDataChk;				// MpsData 所有数据累加和，用于判断档案的完整性  截至end
	unsigned int ConfigNum;
	unsigned int CrcNum;

	void ReInit() {
		I2CAddress = 0x0;
		FourDigiCode = 0x0;
		MpsDataLen = 0x0;
		MpsDataChk = 0x0;
		memset(ParserName, 0, PARSER_NAME_SIZE);
		strncpy((char*)ParserName, PARSER_NAME, strlen(PARSER_NAME));
		memset(Version, 0, PARSER_NAME_SIZE);
		strncpy((char*)Version, PARSER_VERSION, strlen(PARSER_VERSION));
		memset(ProductID, 0, PARSER_NAME_SIZE);

		ConfigNum = 0;
		CrcNum = 0;
	};
	HeadDataFmt() {
		ReInit();
	};
}tHeadDataFmt;

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
		return false; // 确保字符串不为空
	}
	while (*str != '\0') {
		if (*str++ != '0') {
			return false; // 遇到非零字符，返回 false
		}
	}
	return true; // 字符串全部为零，返回 true
}
static unsigned int getAsciiIntValue(const QString& strValue) {
	unsigned int uint32Value = 0;
	int int02 = 0;
	int len = strValue.length();
	char* p = (char*)&uint32Value;
	for (int i = len - 1, j = 0; i >= 0; i--, j++) {
		char ch = strValue[i].cell();
		*(p + j) = ch;
	}
	return uint32Value;
}


MPSTypeBParser::MPSTypeBParser()
{
}

char* MPSTypeBParser::getVersion()
{
	return "1.0.0.0";
}
char* MPSTypeBParser::getFormatName()
{
	return C_MPSTYPEB;
}
bool MPSTypeBParser::ConfirmFormat(QString& filename)
{
	bool ret = false;
	int Rtn = 0, LineNum = 0;
	QFile File(filename);
	QString strOneLine;
	QString temp;
	QVector<QString> vItemColList;
	int nItemColCnt = 0;
	int valueConv = 0;
	QString strConvTemp;
	int iValue = 0;
	int lineNum = 0;
	QString strHeadNote;
	QString strCommandType;
	QString strTempRowSrc;
	int nTotalCnt_WithM_FR_ADDR = 0;
	int 	nEveryItemWithM_FR_ADDR = 0;
	QString strMsg;

	QMap<QString, int> crcZoneMap;
	crcZoneMap.insert("CRC_CHECK_START", -1);
	crcZoneMap.insert("CRC_CHECK_STOP", -1);

	if (QFileInfo(filename).suffix().compare("txt", Qt::CaseInsensitive) != 0) {
		return false;
	}

	if (!File.open(QIODevice::ReadOnly)) {
		ferror = C_Err_FileOpen;
		goto __end;
	}

	int dataLine = 0;
	bool hasDigiCode = false, hasI2CAddr = false, hasProductID = false;
	bool hasEnd = false, hasCommandType = false;

	while (!File.atEnd()) {
		strOneLine = File.readLine();
		bool bCurRowIsCRC = false;
		lineNum++;
		if (strOneLine.isEmpty()) {
			continue;
		}

		strOneLine = strOneLine.replace("\r\n", ""); // 0x0d0a
		strOneLine = strOneLine.mid(strOneLine.indexOf(QRegExp("\\S")));
		if (strOneLine.length() == 0) {
			continue;
		}

		if (strOneLine.indexOf("****") == 0) {
			break;
		}

		//MP2891GQN.txt档案，已和驱动确认可以不解析
		if (strOneLine.indexOf("Multi Index") == 0) {
			continue;
		}

		QMap<QString, int>::iterator it = crcZoneMap.begin();
		for (; it != crcZoneMap.end(); it++) {
			QString strName;
			strName = it.key();
			if (strOneLine.left(15).compare(strName, Qt::CaseInsensitive) == 0) {
				crcZoneMap[strName]++;
				bCurRowIsCRC = true;
				break;
			}
			else if (strOneLine.left(14).compare(strName, Qt::CaseInsensitive) == 0) {
				crcZoneMap[strName]++;
				bCurRowIsCRC = true;
				break;
			}
		}
		if (bCurRowIsCRC) {
			continue;
		}


		if ((strOneLine.left(3)).compare("END", Qt::CaseInsensitive) == 0) {
			hasEnd = true;
			continue;
			//break;///还是判断Product ID，I2C address，4-dige Code
		}

		if (strOneLine.indexOf("4-dig") >= 0) {///
			hasDigiCode = true;
			continue;
		}

		if ((strOneLine.left(11)).compare("I2C Address", Qt::CaseInsensitive) == 0) {
			hasI2CAddr = true;
			continue;
		}

		if ((strOneLine.left(10)).compare("Product ID", Qt::CaseInsensitive) == 0) {
			hasProductID = true;
			continue;
		}
		strCommandType = strOneLine.mid(strOneLine.length() - 2, 2);
		if (strCommandType.compare("B1", Qt::CaseInsensitive) == 0 || strCommandType.compare("B2", Qt::CaseInsensitive) == 0 || strCommandType.compare("B3", Qt::CaseInsensitive) == 0) {
			hasCommandType = true;
		}
		QStringList destDataArray;
		destDataArray = strOneLine.split("\t");
		if (destDataArray.count() != MAX_COLUMN_NUMBER && destDataArray.count() != TYPE_NUMBER) {
			ret = false; goto __end;
		}
		bool ok = false;
		int tmpInt = 0;

		tmpInt = destDataArray[REG_COLUMN_DEC].toInt(&ok, 16);
		if (!ok) {
			ret = false; goto __end;
		}

		tmpInt = destDataArray[DATA_COLUMN_DEC].toInt(&ok, 16);
		if (!ok) {
			ret = false; goto __end;
		}

		long typeHex;
		if (destDataArray.count() == TYPE_NUMBER)
		{
			typeHex = destDataArray[DATA_COLUMN_DEC].toLong(NULL, 16);
		}
		else {
			typeHex = destDataArray[TYPE_NUMBER].toLong(NULL, 16);
		}
		if (typeHex == 0) {

			const char* pBuffer = destDataArray[REG_COLUMN_HEX].toLocal8Bit().data();
			//返回0,不一定是错误,也可能全是0
			if (!IsStrAllZeros(pBuffer)) {
				ret = false; goto __end;
			}
		}

		typeHex = typeHex & 0x0F;

		if (crcZoneMap["CRC_CHECK_START"] == crcZoneMap["CRC_CHECK_STOP"]) {
			if (destDataArray[DATA_COLUMN_HEX].length() != typeHex * 2) {
				ret = false; goto __end;
			}
		}

		dataLine++;
	}

	bool hasProductI2CDig = hasI2CAddr | hasProductID;
	if (hasEnd && hasCommandType) {
		ret = true; goto __end;
	}
	else if (hasEnd && hasProductI2CDig) {
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
int MPSTypeBParser::TransferFile(QString& srcfn, QIODevice* dst)
{

	bool StopUpdate = false;
	QString strMsg;
	int valueConv = 0;
	unsigned short sum = 0;
	QFile File(srcfn);
	QString strOneLine;
	QString temp;
	QVector<QString> vItemColList;
	QVector<unsigned char> vHexData;

	QMap<QString, int> crcZoneMap;
	crcZoneMap.insert("CRC_CHECK_START", -1);
	crcZoneMap.insert("CRC_CHECK_STOP", -1);
	unsigned long CRC_CHECK_STARTAddr = 0;
	unsigned int uCrcOff = 0;

	for (int j = 0; j < m_vBufInfo.size(); ++j) {
		if (QString(m_vBufInfo[j].strBufName.c_str()).compare("CRC_CHECK_START", Qt::CaseInsensitive) == 0) {
			CRC_CHECK_STARTAddr = m_vBufInfo[j].llBufStart;
		}
	}

	QString strTempRowSrc;
	int nItemColCnt = 0;

	int off = DATA_WRITE_OFFSET;
	int oneItemDataLen = 0;
	int oneItemDataIdx = 0;
	int LineNum = 0;

	QString strConvTemp;
	long typeHex;
	unsigned int regAddrHex;
	tHeadDataFmt HeadDataFmt; //存放解析到的头
	QStringList destDataArray;

	if (File.open(QIODevice::ReadOnly) == false) {
		strMsg = QString("Open File Failed: %1").arg(srcfn);
		m_pOutput->Error(strMsg.toLocal8Bit().data());
		ferror = C_Err_FileOpen; goto __end;
	}

	HeadDataFmt.ConfigNum = 0;
	HeadDataFmt.CrcNum = 0;

	InitControl();
	FileSize = File.size();
	m_Dst = dst;

	while (!File.atEnd()) {
		myfgets(&File);
		strOneLine = (char*)work;
		bool bCurRowIsCRC = false;
		bool bCurRowIsCRCData = false;
		LineNum++;
		if (strOneLine.isEmpty()) {
			continue;
		}

		strOneLine = strOneLine.replace("\r\n", ""); // 0x0d0a
		strOneLine = strOneLine.trimmed();

		if (strOneLine.length() == 0) {
			continue;
		}
		if ((strOneLine.left(3)).compare("***", Qt::CaseInsensitive) == 0) {
			continue;
		}

		QMap<QString, int>::iterator it = crcZoneMap.begin();
		for (; it != crcZoneMap.end(); it++) {
			QString strName;
			strName = it.key();
			if (strOneLine.left(15).compare(strName, Qt::CaseInsensitive) == 0) {
				crcZoneMap[strName]++;
				bCurRowIsCRC = true;
				break;
			}
			else if (strOneLine.left(14).compare(strName, Qt::CaseInsensitive) == 0) {
				crcZoneMap[strName]++;
				bCurRowIsCRC = true;
				break;
			}
		}
		if (bCurRowIsCRC) {
			continue;
		}

		if ((strOneLine.left(3)).compare("END", Qt::CaseInsensitive) == 0) {
			StopUpdate = true;
			continue;
		}
		//if ((strOneLine.Left(11)).CompareNoCase("4-digi Code") ==0 || (strOneLine.Left(14)).CompareNoCase("4-digital Code") ==0){
		if (strOneLine.indexOf("4-dig") >= 0) {
			destDataArray = strOneLine.split("\t");
			if (destDataArray.count() != 3) {
				ferror = C_Err_FormatError;
				strMsg = QString("format error %1 LineNum:%2").arg(strOneLine).arg(LineNum);
				m_pOutput->Error(strMsg.toLocal8Bit().data());
				goto __end;
			}
			HeadDataFmt.FourDigiCode = destDataArray[2].toInt();
			//HeadDataFmt.FourDigiCode = FOUR_DIGI_C0DE;//后续指定为0x0060
			break;
		}


		if ((strOneLine.left(11)).compare("I2C Address", Qt::CaseInsensitive) == 0) {
			destDataArray = strOneLine.split("\t");
			if (destDataArray.count() != 3) {
				ferror = C_Err_FormatError;
				strMsg = QString("format error %1 LineNum:%2").arg(strOneLine).arg(LineNum);
				m_pOutput->Error(strMsg.toLocal8Bit().data());
				goto __end;
			}
			HeadDataFmt.I2CAddress = destDataArray[2].toInt();
			continue;
		}

		if ((strOneLine.left(10)).compare("Product ID", Qt::CaseInsensitive) == 0) {
			destDataArray = strOneLine.split("\t");
			if (destDataArray.count() != 2) {
				ferror = C_Err_FormatError;
				strMsg = QString("format error %1 LineNum:%2").arg(strOneLine).arg(LineNum);
				m_pOutput->Error(strMsg.toLocal8Bit().data());
				goto __end;
			}
			strcpy((char*)HeadDataFmt.ProductID, destDataArray[1].toLocal8Bit().data());
			continue;
		}

		tNDestDataFmt DestRecordOneItem; //存放解析到的每行数据
		DestRecordOneItem.ReInit();


		destDataArray = strOneLine.split("\t");
		if (destDataArray.count() != MAX_COLUMN_NUMBER) {
			if (destDataArray.count() != TYPE_NUMBER)
			{
				ferror = C_Err_FormatError;
				strMsg = QString("%1 %2.LineNum:%3").arg("data column must be ").arg(MAX_COLUMN_NUMBER).arg(LineNum);
				m_pOutput->Error(strMsg.toLocal8Bit().data());
				goto __end;
			}

		}


		/*
		* JWL：
			下面这两个判断原型是 if (hex2Int(destDataArray[DATA_COLUMN_HEX])!= atoi(destDataArray[DATA_COLUMN_DEC]))
			atoi只能转换十进制，这里是判断字符是否为16进制，还是判断字符是否为0-9

		*/
		bool ok = false;
		int tmpInt = destDataArray[REG_COLUMN_DEC].toInt(&ok, 16);
		if (!ok) {

			ferror = C_Err_FormatError;
			strMsg = QString("%1 LineNum:%2").arg("value not match with the format, please check").arg(LineNum);
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			goto __end;
		}

		tmpInt = destDataArray[DATA_COLUMN_DEC].toInt(&ok, 16);
		if (!ok) {
			strMsg = QString("%1 LineNum:%2").arg("value not match with the format, please check").arg(LineNum);
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			ferror = C_Err_FormatError;
			goto __end;
		}

		//1. Page
		if (!std::all_of(destDataArray[PAGE_COLUMN].begin(), destDataArray[PAGE_COLUMN].end(), [](QChar ch) { return ch.isDigit(); })) {

			ferror = C_Err_FormatError;
			strMsg = QString("%s line: %d.").arg("page column must be digit").arg(LineNum);
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			goto __end;
		}
		DestRecordOneItem.PageNum = destDataArray[PAGE_COLUMN].toInt();

		//2. reg addr
		regAddrHex = (unsigned int)destDataArray[REG_COLUMN_HEX].toLong(NULL, 16);
		if (regAddrHex == 0) {
			const char* pBuffer = destDataArray[REG_COLUMN_HEX].toLocal8Bit().data();
			//返回0,不一定是错误,也可能全是0
			if (IsStrAllZeros(pBuffer)) {
				strMsg = QString("%1 Max Column: %2. LineNum:%3").arg("Reg number column is all zero. ").arg(MAX_COLUMN_NUMBER).arg(LineNum);
				m_pOutput->Log(strMsg.toLocal8Bit().data());
			}
			else {
				ferror = C_Err_FormatError;
				strMsg = QString("%1 %2. LineNum:%3").arg("reg number column transfer failure ").arg(MAX_COLUMN_NUMBER).arg(LineNum);
				m_pOutput->Error(strMsg.toLocal8Bit().data());
				goto __end;
			}
		}
		DestRecordOneItem.RegAddr = regAddrHex;


		//3. type number
		if (destDataArray.count() == TYPE_NUMBER) {//crc为7列，type指定为34 42 01 00，小端存放
			DestRecordOneItem.CmdType = CMD_TYPE;
		}
		if (destDataArray.count() == MAX_COLUMN_NUMBER && destDataArray[TYPE_NUMBER].length() > 4) {

			ferror = C_Err_FormatError;
			strMsg = QString("%1 line: %2.").arg("type number column is too long ").arg(LineNum);
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			goto __end;
		}
		else if (destDataArray.count() == MAX_COLUMN_NUMBER && destDataArray[TYPE_NUMBER].length() < 4)
		{
			DestRecordOneItem.CmdType = getAsciiIntValue(destDataArray[TYPE_NUMBER]);
		}


		//4. len
		if (destDataArray.count() == TYPE_NUMBER)
		{
			typeHex = destDataArray[DATA_COLUMN_DEC].toLong(NULL, 16);
		}
		else if (destDataArray.count() == MAX_COLUMN_NUMBER)
		{
			typeHex = destDataArray[TYPE_NUMBER].toLong(NULL, 16);
		}
		//typeHex = strtol(destDataArray[TYPE_NUMBER].GetBuffer(), NULL, 16);
		if (typeHex == 0) {
			ferror = C_Err_RecordSum;
			strMsg = QString("%1 %2. LineNum :%3").arg("type number column transfer failure ").arg(MAX_COLUMN_NUMBER).arg(LineNum);
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			goto __end;
		}

		typeHex = typeHex & 0x0F;

		if (crcZoneMap["CRC_CHECK_START"] == crcZoneMap["CRC_CHECK_STOP"]) {
			if (destDataArray[DATA_COLUMN_HEX].length() != typeHex * 2) {
				ferror = C_Err_FormatError;
				strMsg = QString("%1 line: %2 column:%3.").arg("type number mismatch data column ").arg(LineNum).arg(MAX_COLUMN_NUMBER);
				m_pOutput->Error(strMsg.toLocal8Bit().data());
				goto __end;
			}
			HeadDataFmt.ConfigNum++;
			bCurRowIsCRCData = true;
		}
		else {
			HeadDataFmt.CrcNum++;
		}

		DestRecordOneItem.Len = typeHex;
		if (destDataArray.count() == TYPE_NUMBER) {
			DestRecordOneItem.Len = 4;//crc为7列，len指定为4
		}

		//5. DATA_COLUMN_HEX
		vHexData.clear();
		if (destDataArray[DATA_COLUMN_HEX].length() % 2 != 0) {//value列为奇数补0
			destDataArray[DATA_COLUMN_HEX] = "0" + destDataArray[DATA_COLUMN_HEX];
		}
		if (ComTool::Str2Hex(destDataArray[DATA_COLUMN_HEX], /*ComTool::ENDIAN_LIT*/ComTool::ENDIAN_LIT | ComTool::PREHEADZERO_NEED, vHexData) == false) {
			ferror = C_Err_FormatError;
			strMsg = QString("String to hex transfer failed, please check line:%1 %2").arg(LineNum).arg(destDataArray[DATA_COLUMN_HEX]);
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			goto __end;
		}
		if (vHexData.size() > MAX_DATA_SIZE) {
			ferror = C_Err_FormatError;
			strMsg = QString("data column hex length is too long, please check line:%1 %2").arg(LineNum).arg(destDataArray[DATA_COLUMN_HEX]);
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			goto __end;
		}
		std::copy(vHexData.begin(), vHexData.end(), DestRecordOneItem.Data);

		if (bCurRowIsCRCData) {
			BufferWrite(off, (unsigned char*)&DestRecordOneItem.PageNum, 4);
			off += 4;
			BufferWrite(off, (unsigned char*)&DestRecordOneItem.RegAddr, 4);
			off += 4;
			BufferWrite(off, (unsigned char*)&DestRecordOneItem.CmdType, 4);
			off += 4;
			BufferWrite(off, (unsigned char*)&DestRecordOneItem.Len, 4);
			off += 4;
			BufferWrite(off, (unsigned char*)&DestRecordOneItem.Data, DestRecordOneItem.Len);
			off += DestRecordOneItem.Len;
		}
		else {
			//m_pDataBuffer->BufWrite(CRC_CHECK_STARTAddr + uCrcOff , (unsigned char*)&DestRecordOneItem, 16);
			BufferWrite(CRC_CHECK_STARTAddr + uCrcOff, (unsigned char*)&DestRecordOneItem.PageNum, 4);
			uCrcOff += 4;
			BufferWrite(CRC_CHECK_STARTAddr + uCrcOff, (unsigned char*)&DestRecordOneItem.RegAddr, 4);
			uCrcOff += 4;
			BufferWrite(CRC_CHECK_STARTAddr + uCrcOff, (unsigned char*)&DestRecordOneItem.CmdType, 4);
			uCrcOff += 4;
			BufferWrite(CRC_CHECK_STARTAddr + uCrcOff, (unsigned char*)&DestRecordOneItem.Len, 4);
			uCrcOff += 4;
			BufferWrite(CRC_CHECK_STARTAddr + uCrcOff, (unsigned char*)&DestRecordOneItem.Data, DestRecordOneItem.Len);
			uCrcOff += (DestRecordOneItem.Len);
		}


		//update total
		if (!StopUpdate) {///长度计算截至END,不计算CRC	
			oneItemDataLen = (4 * 4 + DestRecordOneItem.Len);
			HeadDataFmt.MpsDataLen += oneItemDataLen;
			for (oneItemDataIdx = 0; oneItemDataIdx < oneItemDataLen; oneItemDataIdx++) {
				HeadDataFmt.MpsDataChk += *((unsigned char*)&DestRecordOneItem + oneItemDataIdx);
			}
		}

		UpdateProgress();
	}

	if (crcZoneMap["CRC_CHECK_START"] != crcZoneMap["CRC_CHECK_STOP"]) {
		goto __end;
	}

	if (crcZoneMap["CRC_CHECK_START"] >= 0) {
		QString strVer;
		strVer = "V001";

		memcpy(HeadDataFmt.Version, strVer.toLocal8Bit().data(), strVer.length());
	}

	BufferWrite(HEAD_WRITE_OFFSET, (unsigned char*)&HeadDataFmt, PARSER_NAME_SIZE * 3 + 4 * 4 + 4 * 2);


__end:
	if (File.isOpen())
		File.close();
	return ferror;
}
