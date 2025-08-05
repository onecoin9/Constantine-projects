#include "MPSTypeCParser.h"
#include "..\public\DrvDllCom\ComTool.h"

typedef struct
{
	char ParserName[32];       // 解析器名称，填入“MPSParserTypeC”
	char Version[32];		  	  //  解析器版本号“V000”
	char ProductID[32];         // “MPSPMIC”
	unsigned int I2CAddress;	      //  0x30
	unsigned int FourDigiCode;     //  0x0000
	unsigned int MpsDataLen;	      //  MpsData 所有数据长度，
	unsigned int MpsDataChk;	      //  MpsData 所有数据累加和，用于判断档案的完整性
} tFileHead_Type;

typedef struct
{
	unsigned int PageNum;    // 0
	unsigned int RegAdr;      // 第2列数据
	unsigned int CmdType;    // 第5列数据，用4个字节表示 
	unsigned int Len;         // 第4列数据
	char* Value;		//  第6列数据的长度, HEX表示，小端存放
} MpsData_Type;

#define DATAWRITE_OFFSET (0x100)

MPSTypeCParser::MPSTypeCParser()
{
}

char* MPSTypeCParser::getVersion()
{
	return "1.0.0.0";
}
char* MPSTypeCParser::getFormatName()
{
	return C_MPSTYPEC;
}
bool MPSTypeCParser::ConfirmFormat(QString& filename)
{

	bool ret = false;
	int Rtn = 0;
	QFile File(filename);
	QString strOneLine;

	bool bIdentifyFlag = false;
	QString strTemp;
	QVector<QString> vItemColList;
	int nItemColCnt = 0;
	int nValueConv = 0;

	/*CString strConvTemp;
	int iValue = 0;*/
	int nLineNum = 0;
	QString strHeadNote;
	QString strTempRowSrc;
	bool bIsExistFlag = false;
	int nFirstColValuePre = -1;
	//char prePart[3] = { 0xEF, 0xBB, 0xBF };


	
	if (QFileInfo(filename).suffix().compare("txt", Qt::CaseInsensitive) != 0) {
		Rtn = -1; goto __end;
	}

	if (File.open(QIODevice::ReadOnly) == false) {
		Rtn = -1; goto __end;
	}

	while (!File.atEnd()) {
		int nValueLen6Col = 0;
		int nCntChar4Col = 0;
		int nSamePart = 0;
		strOneLine = QString::fromUtf8(File.readLine());
		nLineNum++;
		if (strOneLine.isEmpty()) {
			continue;
		}

		strOneLine = strOneLine.replace("\r\n", ""); // 0x0d0a
		//strOneLine.TrimLeft();
		strHeadNote = strOneLine.left(3);
		if (strHeadNote.compare("***", Qt::CaseInsensitive) == 0) {
			continue;
		}

		//for (int i = 0; i < 3; i++) {
		//	if (strHeadNote[i] == prePart[i]) {
		//		nSamePart++;
		//	}
		//}

		//if (nSamePart == 3) {
		//	strOneLine = strOneLine.remove(0, 3);
			if (strOneLine.indexOf("Device Address", 0) >= 0) {
				if (strOneLine.indexOf("W/R", 0) >= 0) {
					bIdentifyFlag = true;
					continue;
				}
				else {
					Rtn = -1; goto __end;
				}
			}
		//}

		strOneLine = strOneLine.replace(" ", "");//空格键
		//strOneLine.Replace(0x20, ";"); //空格键

		strTempRowSrc = strOneLine;
		//while(strTempRowSrc.Find(";;", 0) >= 0){
		//	strTempRowSrc.Replace(";;", ";"); //连续多个空格只保留一个空格
		//}
		//strTempRowSrc.Replace(";", ",");//将空格转换成，
		strTempRowSrc = strTempRowSrc.replace(0x09, ',');//将Tab键转成；

		/*while(strTempRowSrc.Find(",;", 0) >=0){
			strTempRowSrc.Replace(",;", ",");
		}*/
		strOneLine = strTempRowSrc;

		strOneLine = strOneLine.replace("\r\n", ""); // 0x0d0a
		strOneLine += ",";

		nItemColCnt = 0;
		vItemColList.clear();

		if (strOneLine.length() == 0) {
			continue;
		}

		while (strOneLine.length() > 0) {
			int index = strOneLine.indexOf(",", 0);
			if (index < 0) {
				goto __end;
			}

			if (index < 1) { //每列至少一个
				goto __end;
			}

			strTemp = strOneLine.left(index);

			if (nItemColCnt == 0) {
				sscanf(strTemp.toLocal8Bit().data(), "%x", &nValueConv);
				if (nValueConv < 0 || nValueConv > 0x7F) {
					goto __end;
				}

				if (nFirstColValuePre == -1) {
					nFirstColValuePre = nValueConv;
				}
				else {
					if (nFirstColValuePre != nValueConv) {
						goto __end;
					}
				}
			}
			else if (nItemColCnt == 3) {
				nValueConv = strTemp.toInt();
				nCntChar4Col = nValueConv;
			}
			else if (nItemColCnt == 5) {
				sscanf(strTemp.toLocal8Bit().data(), "%x", &nValueConv);
				QString strOriginal;
				strOriginal = strTemp;
				strOriginal = strOriginal.replace("0x", "");
				strOriginal = strOriginal.replace("0X", "");
				nValueLen6Col = strOriginal.length();
				if (nValueLen6Col % 2 != 0) {
					goto __end;
				}
			}

			strOneLine = strOneLine.remove(0, index + 1);
			nItemColCnt++;
			vItemColList.push_back(strTemp);
		}

		if (nItemColCnt != 6) {
			goto __end;
		}

		if (nCntChar4Col * 2 != nValueLen6Col) {
			goto __end;
		}
	}

	ret = true;
	if (!bIdentifyFlag) {
		ret = false;
	}
	if (File.isOpen())
		File.close();
	return ret;

__end:
	if (File.isOpen())
		File.close();
	return false;
}
int MPSTypeCParser::TransferFile(QString& srcfn, QIODevice* dst)
{
	int Rtn = -1;
	QString strMsg;
	int valueConv = 0;
	unsigned short sum = 0;
	QFile File(srcfn);
	QString strOneLine;
	QString temp;
	QVector<QString> vItemColList;
	QVector<unsigned char> vHexData;
	QVector<unsigned char> vData;

	bool bIdentifyFlag = false;
	QString strTemp;

	int nItemColCnt = 0;
	int nValueConv = 0;

	QString strConvTemp;
	int iValue = 0;

	int nLineNum = 0;
	QString strHeadNote;
	QString strTempRowSrc;
	bool bIsExistFlag = false;
	int nFirstColValuePre = -1;
	char prePart[3] = { 0xEF, 0xBB, 0xBF };

	unsigned int uSumMpsDataLen = 0;
	unsigned char pItemRowData[128] = { 0 };
	char pValueData[64] = { 0 };
	MpsData_Type pMpsData_Type;

	tFileHead_Type FileHead_Type;
	QString strMagic;

	memset(FileHead_Type.ParserName, 0, 32);
	memset(FileHead_Type.Version, 0, 32);
	memset(FileHead_Type.ProductID, 0, 32);
	FileHead_Type.FourDigiCode = 0;
	FileHead_Type.MpsDataChk = 0;
	FileHead_Type.MpsDataLen = 0;

	strMagic = "MPSParserTypeC";
	memcpy(FileHead_Type.ParserName, strMagic.toLocal8Bit().data(), strMagic.length());

	strMagic = "MPSPMIC";
	memcpy(FileHead_Type.ProductID, strMagic.toLocal8Bit().data(), strMagic.length());

	if (File.open(QIODevice::ReadOnly) == false) {
		ferror = C_Err_FileOpen;
		strMsg = QString("Open File Failed: %1").arg(srcfn);
		m_pOutput->Error(strMsg.toLocal8Bit().data());
		goto __end;
	}
	InitControl();
	FileSize = File.size();
	m_Dst = dst;
	while (!File.atEnd()) {
		int nValueLen6Col = 0;
		int nCntChar4Col = 0;
		int nSamePart = 0;

		myfgets(&File);
		strOneLine = (char*)work;

		nLineNum++;
		if (strOneLine.isEmpty()) {
			continue;
		}

		strOneLine = strOneLine.replace("\r\n", ""); // 0x0d0a
		strHeadNote = strOneLine.left(3);
		if (strHeadNote.compare("***", Qt::CaseInsensitive) == 0) {
			continue;
		}

		//for (int i = 0; i < 3; i++) {
		//	if (strHeadNote[i] == prePart[i]) {
		//		nSamePart++;
		//	}
		//}

		//if (nSamePart == 3) {
		//	strOneLine = strOneLine.remove(0, 3);
			if (strOneLine.indexOf("Device Address", 0) >= 0) {
				if (strOneLine.indexOf("W/R", 0) >= 0) {
					bIdentifyFlag = true;
					continue;
				}
				else {
					Rtn = -1; goto __end;
				}
			}
		//}

		strOneLine = strOneLine.replace(" ", "");//空格键
		strTempRowSrc = strOneLine;
		strTempRowSrc = strTempRowSrc.replace(0x09, ',');//将Tab键转成；
		strOneLine = strTempRowSrc;

		strOneLine = strOneLine.replace("\r\n", ""); // 0x0d0a
		strOneLine += ",";

		nItemColCnt = 0;
		vItemColList.clear();

		if (strOneLine.length() == 0) {
			continue;
		}

		pMpsData_Type.PageNum = 0;

		while (strOneLine.length() > 0) {
			int index = strOneLine.indexOf(",", 0);
			if (index < 0) {
				goto __end;
			}

			if (index < 1) { //每列至少一个
				goto __end;
			}

			strTemp = strOneLine.left(index);

			if (nItemColCnt == 0) {
				sscanf(strTemp.toLocal8Bit().data(), "%x", &nValueConv);
				if (nValueConv < 0 || nValueConv > 0x7F) {
					goto __end;
				}

				if (nFirstColValuePre == -1) {
					nFirstColValuePre = nValueConv;
					FileHead_Type.I2CAddress = nValueConv;
				}
				else {
					if (nFirstColValuePre != nValueConv) {
						goto __end;
					}
				}
			}
			else if (nItemColCnt == 1) {
				sscanf(strTemp.toLocal8Bit().data(), "%x", &nValueConv);
				pMpsData_Type.RegAdr = nValueConv;
			}
			else if (nItemColCnt == 3) {
				nValueConv = strTemp.toInt();
				nCntChar4Col = nValueConv;
				pMpsData_Type.Len = nValueConv;
			}
			else if (nItemColCnt == 4) {
				pMpsData_Type.CmdType = 1;
				if (strTemp.compare("WR", Qt::CaseInsensitive) == 0) {
				}
				else if (strTemp.compare("W", Qt::CaseInsensitive) == 0)
				{
					pMpsData_Type.CmdType = 2;
				}
				else if (strTemp.compare("R", Qt::CaseInsensitive) == 0)
				{
					pMpsData_Type.CmdType = 3;
				}
				else {
					goto __end;
				}
			}
			else if (nItemColCnt == 5) {
				sscanf(strTemp.toLocal8Bit().data(), "%x", &nValueConv);
				QString strOriginal;
				strOriginal = strTemp;
				strOriginal = strOriginal.replace("0x", "");
				strOriginal = strOriginal.replace("0X", "");
				nValueLen6Col = strOriginal.length();
				if (nValueLen6Col % 2 != 0) {
					goto __end;
				}

				////////////////////////////////////////////////////////////////////////
				vHexData.clear();
				if (ComTool::Str2Hex(strOriginal, ComTool::ENDIAN_BIG | ComTool::PREHEADZERO_NEED/*ComTool::ENDIAN_LIT*/, vHexData) == false) {
					strMsg = "String to hex transfer failed, please check";
					m_pOutput->Error(strMsg.toLocal8Bit().data());
					Rtn = -1;
					goto __end;
				}

				if (vHexData.size() <= 0) {
					Rtn = -1;
					goto __end;
				}
			}

			strOneLine = strOneLine.remove(0, index + 1);
			nItemColCnt++;
			vItemColList.push_back(strTemp);
		}

		if (nItemColCnt != 6) {
			goto __end;
		}

		if (nCntChar4Col * 2 != nValueLen6Col) {
			goto __end;
		}

		//////////////////////////////////////////////////////
		vData.clear();
		memset(pItemRowData, 0, 128);
		memcpy(pItemRowData, &pMpsData_Type, 16);
		for (int n = vHexData.size() - 1; n >= 0; n--) {
			vData.push_back(vHexData[n]);
		}
		std::copy(vData.begin(), vData.end(), pItemRowData + 16);

		int nPerRecordDataLen = 16 + vHexData.size();

		BufferWrite(DATAWRITE_OFFSET + uSumMpsDataLen, pItemRowData, nPerRecordDataLen);

		uSumMpsDataLen += nPerRecordDataLen;

		for (int n = 0; n < nPerRecordDataLen; n++) {
			FileHead_Type.MpsDataChk += pItemRowData[n];
		}
		UpdateProgress();
	}

	FileHead_Type.MpsDataLen = uSumMpsDataLen;

	BufferWrite(0, (unsigned char*)&FileHead_Type, sizeof(tFileHead_Type));

	if (bIdentifyFlag) {
		Rtn = 0;
	}

__end:
	if (File.isOpen())
		File.close();
	return Rtn;
}