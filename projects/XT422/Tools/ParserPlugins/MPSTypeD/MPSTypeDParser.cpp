#include "MPSTypeDParser.h"
#include "..\public\DrvDllCom\ComTool.h"
#define DATAWRITE_OFFSET (256)
typedef struct tChipHead_Type
{
	char ParserName[32];
	char Version[32];
	char ProductID[32];
	unsigned int I2CAddress;
	unsigned int FourDigiCode;
	unsigned int MpsDataLen;
	unsigned int MpsDataChk;

	void Reint() {
		memset(ParserName, 0, 32);
		memset(Version, 0, 32);
		memset(ProductID, 0, 32);
		I2CAddress = 0;
		FourDigiCode = 0;
		MpsDataLen = 0;;
		MpsDataChk = 0;;
	}
	tChipHead_Type() {
		Reint();
	}
}ChipHead_Type;

typedef struct DestDataEach {
	unsigned int PageNum;
	unsigned int RegAdr;
	unsigned int CmdType;
	unsigned int Len;
	char* value;

	void ReInit() {
		PageNum = 0x00;
		RegAdr = 0x00;
		CmdType = 0x00;
		Len = 0x00;
		value = NULL;
	};
	DestDataEach() {
		ReInit();
	};
}tDestDataEach;

MPSTypeDParser::MPSTypeDParser()
{
}

char* MPSTypeDParser::getVersion()
{
	return "1.0.0.0";
}
char* MPSTypeDParser::getFormatName()
{
	return C_MPSTYPED;
}
bool MPSTypeDParser::ConfirmFormat(QString& filename)
{
	bool ret = false;
	int Rtn = 0, LineNum = 0;
	QFile File(filename);
	QString strOneLine;
	QString temp;
	QVector<QString> vItemColList;

	int valueConv = 0;

	int iValue = 0;
	int lineNum = 0;

	int FormatFlag = 0;
	QString  strFourDigiCode;

	int nItemColCnt = 0;
	QString strConvTemp;


	if (QFileInfo(filename).suffix().compare("txt", Qt::CaseInsensitive) != 0) {
		return false;
	}

	if (File.open(QIODevice::ReadOnly) == false) {
		ferror = C_Err_FileOpen;
		goto __end;
	}

	//std::vector<tDllBufInfo> vBufInfo;
	/*tGlbSetting GlbSetting;
	GetGlbSetting(&GlbSetting);*/

	/*for(int j=0;j<m_vBufInfo.size();++j){
		if (m_vBufInfo[j].strBufName.CompareNoCase("TypeD") == 0){
			FormatFlag = 1;
		}
	}*/

	/*if (FormatFlag == 0){
		ret=-1; goto __end;
	}*/

	while (!File.atEnd()) {
		strOneLine = File.readLine();
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
		if ((strOneLine.left(3)).compare("END", Qt::CaseInsensitive) == 0) {
			FormatFlag = 2;
			break;
		}
		strOneLine = strOneLine.remove(0x20); //驱动要求只以0x09作为分隔符
		strOneLine = strOneLine.replace(0x09, ',');//为Tab键
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

			if (index < 1) { //每列至少一个字符
				goto __end;
			}

			temp = strOneLine.left(index);//取每个分割段里的值
			if (nItemColCnt == 0) {
				if (temp.isEmpty()) {
					goto __end;
				}

				if (strFourDigiCode.isEmpty()) {
					strFourDigiCode = temp;
				}
				else {
					if (strFourDigiCode.compare(temp, Qt::CaseInsensitive) != 0) {
						goto __end;
					}
				}
			}

			if (nItemColCnt == 1 || nItemColCnt == 2 || nItemColCnt == 4) {
				strOneLine = strOneLine.remove(0, index + 1);
				nItemColCnt++;
				vItemColList.push_back(temp);

				continue;
			}

			if (nItemColCnt == 6) {
				valueConv = temp.toInt();
				temp = QString::number(valueConv);
			}

			if (nItemColCnt == 3 || nItemColCnt == 5) { //16进制转10进制
				strConvTemp = temp;
				int iValue = -1;

				sscanf(strConvTemp.toLocal8Bit().data(), "%x", &iValue);
				if (iValue < 0) {
					goto __end;
				}

				temp = QString::number(iValue);
			}

			strOneLine = strOneLine.remove(0, index + 1);
			nItemColCnt++;
			vItemColList.push_back(temp);
		}

		if (nItemColCnt != 7) {//必须要有7列
			goto __end;
		}

		if (vItemColList[5].compare(vItemColList[6], Qt::CaseInsensitive) != 0) { //第5列和第6列必须相等
			goto __end;
		}
	}

	ret = false;
	if (FormatFlag == 2) {
		ret = true;
	}
__end:
	if (File.isOpen())
		File.close();
	return ret;
}
int MPSTypeDParser::TransferFile(QString& srcfn, QIODevice* dst)
{
	QString strMsg;
	int valueConv = 0;
	unsigned short sum = 0;
	QFile File(srcfn);
	QString strOneLine;
	QString temp;
	QVector<QString> vItemColList;
	QVector<unsigned char> vHexData;
	QVector<unsigned char> vData;

	int nAddrChipInfo = 0;

	ChipHead_Type headData;
	QString strMagic;
	strMagic = "MPSParserTypeD";
	memcpy(headData.ParserName, strMagic.toLocal8Bit().data(), strMagic.length());

	strMagic = "V000";
	memcpy(headData.Version, strMagic.toLocal8Bit().data(), strMagic.length());

	strMagic = "MPQ7920";
	memcpy(headData.ProductID, strMagic.toLocal8Bit().data(), strMagic.length());

	int off = 0;
	int FormatFlag = 0;
	QString  strFourDigiCode;

	int nItemColCnt = 0;
	int LineNum = 0;
	QString strConvTemp;

	if (File.open(QIODevice::ReadOnly) == false) {
		strMsg = QString("Open File Failed: %s").arg(srcfn);
		m_pOutput->Error(strMsg.toLocal8Bit().data());
		ferror = C_Err_FileOpen; goto __end;
	}

	int nLastPageIdx = -1; //上一个pagenum
	int nCurPageNum = -1; //当前的pagenum

	unsigned char pItemRowData[128] = { 0 };
	unsigned int uSumMpsDataLen = 0;

	/*for(int j=0;j<m_vBufInfo.size();++j){
		if (m_vBufInfo[j].strBufName.CompareNoCase("TypeD") == 0){
			FormatFlag = 1;
		}
	}*/

	FormatFlag = 1;
	/*if (FormatFlag == 0){
		ret=-1; goto __end;
	}*/
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
		if ((strOneLine.left(3)).compare("***", Qt::CaseInsensitive) == 0) {
			continue;
		}
		if ((strOneLine.left(3)).compare("END", Qt::CaseInsensitive) == 0) {
			FormatFlag = 2;
			break;
		}
		strOneLine = strOneLine.remove(0x20); //驱动要求只以0x09作为分隔符
		strOneLine = strOneLine.replace(0x09, ',');//为Tab键
		strOneLine += ",";

		nItemColCnt = 0;
		vItemColList.clear();

		if (strOneLine.length() == 0) {
			continue;
		}

		tDestDataEach  itemData;
		while (strOneLine.length() > 0) {
			int index = strOneLine.indexOf(",", 0);
			if (index < 0) {
				goto __end;
			}

			if (index < 1) { //每列至少一个字符
				goto __end;
			}

			temp = strOneLine.left(index);//取每个分割段里的值
			if (nItemColCnt == 0) {
				if (temp.isEmpty()) {
					goto __end;
				}

				if (strFourDigiCode.isEmpty()) {
					strFourDigiCode = temp;
				}
				else {
					if (strFourDigiCode.compare(temp, Qt::CaseInsensitive) != 0) {
						goto __end;
					}
				}
			}

			if (nItemColCnt == 1 || nItemColCnt == 2 || nItemColCnt == 4) {
			}
			else if (nItemColCnt == 6) {
				valueConv = temp.toInt();
				temp = QString::number(valueConv);
			}
			else if (nItemColCnt == 3 || nItemColCnt == 5) { //16进制转10进制
				strConvTemp = temp;
				int iValue = -1;

				sscanf(strConvTemp.toLocal8Bit().data(), "%x", &iValue);
				if (iValue < 0) {
					goto __end;
				}

				temp = QString::number(iValue);

				if (nItemColCnt == 3) {
					itemData.RegAdr = iValue;
				}

				if (nItemColCnt == 5) {
					QString strOriginal;
					strOriginal = strConvTemp;
					strOriginal = strOriginal.replace("0x", "");
					strOriginal = strOriginal.replace("0X", "");

					if (strOriginal.length() % 2 != 0) {
						goto __end;
					}
					////////////////////////////////////////////////////////////////////////
					vHexData.clear();
					if (ComTool::Str2Hex(strOriginal, ComTool::ENDIAN_BIG | ComTool::PREHEADZERO_NEED/*ComTool::ENDIAN_LIT*/, vHexData) == false) {
						strMsg = "String to hex transfer failed, please check";
						m_pOutput->Error(strMsg.toLocal8Bit().data());
						ferror = C_Err_RecordSum;
						goto __end;
					}

					if (vHexData.size() <= 0) {
						ferror = C_Err_RecordSum;
						goto __end;
					}
				}
			}

			strOneLine = strOneLine.remove(0, index + 1);
			nItemColCnt++;
			vItemColList.push_back(temp);
		}
		//每行解析完成判断
		if (nItemColCnt != 7) {//必须要有7列
			goto __end;
		}

		if (vItemColList[5].compare(vItemColList[6], Qt::CaseInsensitive) != 0) { //第5列和第6列必须相等
			goto __end;
		}

		itemData.Len = 0x01;
		//写入buffer数据
		//////////////////////////////////////////////////////
		vData.clear();
		memset(pItemRowData, 0, 128);
		memcpy(pItemRowData, &itemData, 16);
		for (int n = vHexData.size() - 1; n >= 0; n--) {
			vData.push_back(vHexData[n]);
		}
		std::copy(vData.begin(), vData.end(), pItemRowData + 16);

		int nPerRecordDataLen = 16 + vHexData.size();

		BufferWrite(DATAWRITE_OFFSET + uSumMpsDataLen, pItemRowData, nPerRecordDataLen);

		uSumMpsDataLen += nPerRecordDataLen;

		for (int n = 0; n < nPerRecordDataLen; n++) {
			headData.MpsDataChk += pItemRowData[n];
		}
	}

	headData.MpsDataLen = uSumMpsDataLen;

	BufferWrite(0, (unsigned char*)&headData, sizeof(ChipHead_Type));

	ferror = C_Err_RecordSum;
	if (FormatFlag == 2) {
		ferror = 0;
	}

__end:
	if (File.isOpen())
		File.close();
	return ferror;
}