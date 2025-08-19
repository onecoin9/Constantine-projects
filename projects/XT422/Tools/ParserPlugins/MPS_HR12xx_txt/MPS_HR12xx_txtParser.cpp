#include "MPS_HR12xx_txtParser.h"
#include "..\public\DrvDllCom\ComTool.h"

#define DATAWRITE_OFFSET (4096)

enum {
	ADDR_COLUMN = 0,
	BITS_COLUMN = 1,
	VALUE_COLUMN = 2,
	NAME_COLUMN = 3,
	SAVETIME_COLUMN = 4,
	COLUMNS = 5,
};

typedef struct DestDataFmt {
	unsigned char Prefix;
	unsigned char Addr;
	unsigned char NumCnt;
	unsigned char Data[2];
	void ReInit() {
		Prefix = 0x20;
		Addr = 0x00;
		NumCnt = 0x00;
		memset(Data, 0, 2);
	};
	DestDataFmt() {
		ReInit();
	};
}tDestDataFmt;


MPS_HR12xx_txtParser::MPS_HR12xx_txtParser()
{
}

char* MPS_HR12xx_txtParser::getVersion()
{
	return "1.0.0.0";
}
char* MPS_HR12xx_txtParser::getFormatName()
{
	return C_MPSHR12TXT;
}
bool MPS_HR12xx_txtParser::ConfirmFormat(QString& filename)
{
	bool ret = false;
	int Rtn = 0, LineNum = 0;
	QFile File(filename);
	QString strOneLine;

	if (QFileInfo(filename).suffix().compare("txt", Qt::CaseInsensitive) != 0) {
		return false;
	}

	if (File.open(QIODevice::ReadOnly) == false) {
		return false;
	}

	while (!File.atEnd()) {
		strOneLine = File.readLine();
		LineNum++;
		if (strOneLine.isEmpty()) {
			return false;
		}
		strOneLine.remove("\r");
		strOneLine.remove("\n");
		QStringList destDataArray = strOneLine.split("\t");

		if (destDataArray.count() != COLUMNS) {
			return false;
		}

		if (destDataArray[ADDR_COLUMN].compare("Address", Qt::CaseInsensitive) == 0 &&
			destDataArray[BITS_COLUMN].compare("Bits", Qt::CaseInsensitive) == 0 &&
			destDataArray[VALUE_COLUMN].compare("Value(Hex)", Qt::CaseInsensitive) == 0 &&
			destDataArray[NAME_COLUMN].compare("Name", Qt::CaseInsensitive) == 0 &&
			destDataArray[SAVETIME_COLUMN].compare("SaveTime", Qt::CaseInsensitive) == 0)
		{
			ret = true;
			goto __end;
		}
		else {
			ret = false;
			goto __end;
		}
	}

__end:
	return ret;
}


int MPS_HR12xx_txtParser::TransferFile(QString& srcfn, QIODevice* dst)
{
	m_Dst = dst;
	int ret = -1;
	QString strMsg;
	QFile File(srcfn);
	QString strOneLine;
	QVector<unsigned char> vHexData;

	int off = 0;
	int LineNum = 0;

	QString strConvTemp;

	if (File.open(QIODevice::ReadOnly) == false) {
		strMsg = QString("Open File Failed: %s").arg(srcfn);
		m_pOutput->Error(strMsg.toLocal8Bit().data());
		ferror = C_Err_FileOpen; goto __end;
	}
	InitControl();
	FileSize = File.size();
	while (!File.atEnd()) {
		myfgets(&File);
		strOneLine = (char*)work;
		LineNum++;
		if (strOneLine.isEmpty()) {
			continue;
		}
		if (LineNum == 1) {
			tDestDataFmt PageRowData;//默认地址0 新增page
			PageRowData.Prefix = 0x20;
			PageRowData.Addr = 0x00;
			PageRowData.NumCnt = 0x01;
			PageRowData.Data[0] = 0x00;
			PageRowData.Data[1] = 0x00;
			BufferWrite(off, (unsigned char*)&PageRowData, sizeof(tDestDataFmt));
			off += sizeof(tDestDataFmt);
			continue;
		}

		strOneLine = strOneLine.replace(" ", "");
		strOneLine = strOneLine.replace("\r\n", ""); // 0x0d0a
		strOneLine = strOneLine.trimmed();

		int pos = 0, Columns = 0;
		QStringList colList = strOneLine.split("\t");
		for (auto it : colList) {
			Columns++;
			if (it.isEmpty() && Columns <= 3) {
				ferror = C_Err_FormatError;
				strMsg = QString("The First Three Columns Must Have Data.please check line:%1").arg(LineNum);
				m_pOutput->Error(strMsg.toLocal8Bit().data());
				goto __end;
			}
		}


		tDestDataFmt DestRecordOneItem; //存放解析到的每行数据
		DestRecordOneItem.Prefix = 0x20;

		QStringList destDataArray;
		destDataArray = strOneLine.split("\t");

		if (destDataArray[BITS_COLUMN].compare("8") != 0 && destDataArray[BITS_COLUMN].compare("16") != 0) {

			ferror = C_Err_RecordSum;
			strMsg = QString("The Bits Data Must Be 8 or 16.please check line:%1").arg(LineNum);
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			goto __end;
		}

		vHexData.clear();
		destDataArray[ADDR_COLUMN] = destDataArray[ADDR_COLUMN].replace("0x", "");
		if (ComTool::Str2Hex(destDataArray[ADDR_COLUMN], ComTool::ENDIAN_LIT, vHexData) == false) {
			ferror = C_Err_RecordSum;
			strMsg = QString("String to hex transfer failed, please check line:%1").arg(LineNum);
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			goto __end;
		}
		DestRecordOneItem.Addr = vHexData[0];


		if (destDataArray[BITS_COLUMN].compare("8") == 0) {
			DestRecordOneItem.NumCnt = 0x01;
		}
		else {
			DestRecordOneItem.NumCnt = 0x02;
		}

		vHexData.clear();
		if (ComTool::Str2Hex(destDataArray[VALUE_COLUMN], ComTool::ENDIAN_LIT, vHexData) == false) {
			ferror = C_Err_RecordSum;
			strMsg = QString("String to hex transfer failed, please check line:%1").arg(LineNum);
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			goto __end;
		}
		std::copy(vHexData.begin(), vHexData.end(), DestRecordOneItem.Data);

		BufferWrite(off, (unsigned char*)&DestRecordOneItem, sizeof(tDestDataFmt));
		off += sizeof(tDestDataFmt);
	}

	ret = 0;

__end:
	return ret;
}