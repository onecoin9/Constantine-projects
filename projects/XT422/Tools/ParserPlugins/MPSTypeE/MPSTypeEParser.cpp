#include "MPSTypeEParser.h"


#define DATAWRITE_OFFSET (256)

#ifdef WIN32
#pragma pack(1)
#define PACK_ALIGN
#else
#define PACK_ALIGN __attribute__((packed))
#endif

typedef struct EachDestData {
	unsigned char PageNum;
	unsigned char CmdCode;
	unsigned char BYTELen;
	unsigned short wValue;

	void ReInit() {
		PageNum = 0x20;
		CmdCode = 0x00;
		BYTELen = 0x00;
		wValue = 0x00;
	};
	EachDestData() {
		ReInit();
	};
}tEachDestData;

typedef struct TagItemRailData {
	QVector<tEachDestData> vData;
}tItemRailData;

MPSTypeEParser::MPSTypeEParser()
{
}

char* MPSTypeEParser::getVersion()
{
	return "1.0.0.0";
}
char* MPSTypeEParser::getFormatName()
{
	return C_MPSTYPEE;
}
bool MPSTypeEParser::ConfirmFormat(QString& filename)
{
	bool Rtn = false;
	int LineNum = 0;
	QFile File(filename);
	QString strOneLine;
	QString temp;
	QVector<QString> vItemColList;
	QMap<QString, int> vColSign;
	vColSign["Address"] = 0;
	vColSign["Command code"] = 0;
	vColSign["Command name"] = 0;
	vColSign["Byte"] = 0;
	vColSign["W/R"] = 0;
	vColSign["Rail"] = 0;

	int valueConv = 0;

	int iValue = 0;
	int lineNum = 0;

	int FormatFlag = 0;
	QString  strFourDigiCode;

	int nItemColIdx = 0;
	QString strConvTemp;


	if (QFileInfo(filename).suffix().compare("txt", Qt::CaseInsensitive) != 0) {
		return false;
	}

	if (File.open(QIODevice::ReadOnly) == false) {
		return false;
	}

	while (!File.atEnd()) {
		strOneLine = File.readLine();
		if (strOneLine.isEmpty()) {
			continue;
		}

		strOneLine = strOneLine.replace(", ", "");//去除逗号空格，逗号TAP
		strOneLine = strOneLine.replace(",	", "");
		strOneLine = strOneLine.remove(',');
		strOneLine = strOneLine.remove("\r");
		strOneLine = strOneLine.remove("\n");
		/*strOneLine.Remove(0x20);
		strOneLine.Remove(0x09);*/
		if (strOneLine.isEmpty()) {
			continue;
		}

		LineNum++;
		if (LineNum == 1) {
			QMap<QString, int>::iterator it;
			for (it = vColSign.begin(); it != vColSign.end(); it++) {
				if (strOneLine.indexOf(it.key()) >= 0) {
					it.value() = it.value() + 1;
				}
			}

			for (it = vColSign.begin(); it != vColSign.end(); it++) {
				if (it.value() == 0) {
					goto __end;
				}
			}
			continue;
		}

		strOneLine = strOneLine.replace(0x09, ';');//为Tab键
		strOneLine = strOneLine.replace(0x20, 0x3B); //空格键

		while (strOneLine.indexOf(";;", 0) >= 0) {
			strOneLine = strOneLine.replace(";;", ";");
		}
		strOneLine += ";";

		nItemColIdx = 0;
		int nByteLen = 0;
		vItemColList.clear();

		while (strOneLine.length() > 0) {
			int index = strOneLine.indexOf(";", 0);
			if (index < 0) {
				goto __end;
			}

			//if (index < 1) { //每列至少一个字符
			//	goto __end;
			//}

			temp = strOneLine.left(index);//取每个分割段里的值

			if (nItemColIdx == 3) {
				valueConv = temp.toInt();
				temp = QString::number(valueConv);
				nByteLen = valueConv;
			}
			else if (nItemColIdx >= 5) {
				temp = temp.replace("0x", "");
				temp = temp.replace("0X", "");
				if (!temp.isEmpty()) { //为空则跳过
					if (temp.length() != nByteLen * 2) {
						goto __end;
					}
				}
			}
			////////////
			strOneLine = strOneLine.remove(0, index + 1);
			nItemColIdx++;
			vItemColList.push_back(temp);
		}
	}

	Rtn = true;

__end:
	return Rtn;
}


int MPSTypeEParser::TransferFile(QString& srcfn, QIODevice* dst)
{
	m_Dst = dst;
	QString strMsg;
	int valueConv = 0;
	unsigned short sum = 0;
	QFile File(srcfn);
	QString strOneLine;
	QString temp;
	QVector<QString> vItemColList;
	QVector<unsigned char> vHexData;
	QVector<unsigned char> vData;
	QMap<QString, int> vColSign;
	vColSign["Address"] = 0;
	vColSign["Command code"] = 0;
	vColSign["Command name"] = 0;
	vColSign["Byte"] = 0;
	vColSign["W/R"] = 0;
	vColSign["Rail"] = 0;

	tEachDestData  itemData;
	QMap<int, QString> TitleMap;
	QMap<QString, tItemRailData> vAllBuffData; //数据
	QMap<QString, tItemRailData>::iterator itBuff;
	unsigned int AllBuffDataOffset = 0;

	int nAddrChipInfo = 0;

	int off = 0;
	int FormatFlag = 0;
	QString  strFourDigiCode;

	int nItemColIdx = 0;
	int LineNum = 0;
	QString strConvTemp;
	int FirstRowNeedInserPage = -1;
	int nBegain3RowHasPage = -3;
	int nRomExist = 0;
	int nPageIdx = 0;
	int nRailIdx = 0;
	//int nReadDataStartOffset = 0;

	if (File.open(QIODevice::ReadOnly) == false) {
		strMsg = QString("Open File Failed: %1").arg(srcfn);
		m_pOutput->Error(strMsg.toLocal8Bit().data());
		ferror = C_Err_FileOpen; goto __end;
	}

	int nLastPageIdx = -1; //上一个pagenum
	int nCurPageNum = -1; //当前的pagenum

	unsigned char pItemRowData[128] = { 0 };
	unsigned int uSumMpsDataLen = 0;

	InitControl();
	FileSize = File.size();

	while (!File.atEnd()) {
		myfgets(&File);
		strOneLine = (char*)work;
		if (strOneLine.isEmpty()) {
			continue;
		}

		strOneLine = strOneLine.replace(", ", "");
		strOneLine = strOneLine.replace(",	", "");
		strOneLine = strOneLine.remove(',');
		strOneLine = strOneLine.remove(0x0d0a);
		/*strOneLine.Remove(0x20);
		strOneLine.Remove(0x09);*/
		if (strOneLine.isEmpty()) {
			continue;
		}

		LineNum++;

		if (LineNum == 1) {
			QMap<QString, int>::iterator it;
			int i = 0;

			strOneLine = strOneLine.replace(0x09, 0x3B);
			if (strOneLine.isEmpty()) {
				goto __end;
			}
			strOneLine += ";";

			while (!strOneLine.isEmpty()) {
				QString strCur;
				int nIdx = strOneLine.indexOf(";", 0);
				if (nIdx < 0) {
					goto __end;
				}
				strCur = strOneLine.left(nIdx);
				strOneLine = strOneLine.remove(0, nIdx + 1);
				//
				for (it = vColSign.begin(); it != vColSign.end(); it++) {
					if (strCur.indexOf(it.key()) >= 0) {
						it.value()++;
						TitleMap[i] = strCur;
						i++;

						if (strCur.indexOf("Rail") >= 0) {
							tItemRailData data;
							vAllBuffData.insert(strCur, data);
						}
						break;
					}
				}
				//
			}

			it = vColSign.begin();
			for (it = vColSign.begin(); it != vColSign.end(); it++) {
				if (it.value() == 0) {
					goto __end;
				}
			}

			continue;
		}

		/*if (nReadDataStartOffset == 0){
			nReadDataStartOffset = File.GetPosition();
		}*/

		strOneLine = strOneLine.replace(0x09, 0x3B);//为Tab键
		strOneLine = strOneLine.replace(" ", "");//去除空格
		if (strOneLine.right(1) != ";") {//2955档案行末没有tap键
			strOneLine += ";";
		}
		//strOneLine.Replace(0x20, 0x3B); //空格键

		/*while(strOneLine.Find(";;", 0) >= 0){
			strOneLine.Replace(";;", ";");
		}
		strOneLine+=";";*/

		nItemColIdx = 0;
		int nByteLen = 0;
		vItemColList.clear();
		//////////////////////////////////////

		while (strOneLine.length() > 0) {
			int index = strOneLine.indexOf(";", 0);
			if (index < 0) {
				goto __end;
			}

			temp = strOneLine.left(index);//取每个分割段里的值
			///////////////////////////////////////////////////////
			if (nItemColIdx == 1) {
				strConvTemp = temp;
				int iValue = -1;

				sscanf(strConvTemp.toLocal8Bit().data(), "%x", &iValue);
				if (iValue < 0) {
					goto __end;
				}
				itemData.CmdCode = (unsigned char)iValue;
			}
			else if (nItemColIdx == 2) {
				if (nBegain3RowHasPage < 0) {
					if (temp.compare("page", Qt::CaseInsensitive) == 0) {
						nBegain3RowHasPage = 1; //有
						//break;
					}
					if (nBegain3RowHasPage < 0) {
						nBegain3RowHasPage++;
					}

				}
				else if (nBegain3RowHasPage == 0) {
					nBegain3RowHasPage = 2; //無
					//break;
				}

			}
			else if (nItemColIdx == 3) {
				valueConv = temp.toInt();
				temp = QString::number(valueConv);
				nByteLen = valueConv;
				itemData.BYTELen = (unsigned char)nByteLen;
			}
			else if (nItemColIdx == 4) {
				if (temp.compare("ROM", Qt::CaseInsensitive) == 0) {
					nRomExist = 1;//R/W列有"ROM",添加一个page29
					tItemRailData data;

					vAllBuffData.insert("Rail29.Value", data);
				}
			}
			else if (nItemColIdx >= 5) {
				//if (nBegain3RowHasPage < 0){ //先确定有无page
				//	continue;
				//}

				temp = temp.replace("0x", "");
				temp = temp.replace("0X", "");
				if (temp.isEmpty() || (TitleMap[nItemColIdx].indexOf("Rail") < 0)) { //为空则跳过 // 非Rail则跳过

				}
				else {
					QString strKey;
					strKey = TitleMap[nItemColIdx];

					if (temp.length() != nByteLen * 2) {
						goto __end;
					}

					strConvTemp = temp;
					int iValue = -1;
					sscanf(strConvTemp.toLocal8Bit().data(), "%x", &iValue);
					if (iValue < 0) {
						goto __end;
					}
					itemData.wValue = (unsigned short)iValue;

					if (nRomExist) {
						vAllBuffData["Rail29.Value"].vData.push_back(itemData);
					}
					else {
						vAllBuffData[strKey].vData.push_back(itemData);
					}

				}
			}

			strOneLine = strOneLine.remove(0, index + 1);
			nItemColIdx++;
			vItemColList.push_back(temp);
			///////////////////////////////////////////////////////
		}
		/*if (nBegain3RowHasPage == 2 || nBegain3RowHasPage == 1){
			nBegain3RowHasPage = 3;
			File.Seek(nReadDataStartOffset, CFile::begin);
			LineNum = 2;
		}*/

		UpdateProgress();
	}
	//写入buffer数据
	//////////////////////////////////////////////////////	
	for (itBuff = vAllBuffData.begin(); itBuff != vAllBuffData.end(); itBuff++) {
		char buf[100];
		sprintf(buf, "%s Start Address: 0x%x", itBuff.key(), AllBuffDataOffset);
		m_pOutput->Log(buf);

		if (itBuff.key().compare("Rail29.Value", Qt::CaseInsensitive) == 0) {
			tEachDestData ItemData;
			ItemData.PageNum = 0x20;
			ItemData.CmdCode = 0x00;
			ItemData.BYTELen = 0x01;
			ItemData.wValue = 0x1D;

			strMsg = "W/R Column Is ROM,Add page29";
			m_pOutput->Log(strMsg.toLocal8Bit().data());

			BufferWrite(AllBuffDataOffset, (unsigned char*)&ItemData, sizeof(tEachDestData));
			AllBuffDataOffset += sizeof(tEachDestData);
		}
		if (nBegain3RowHasPage == 2 && itBuff.key().compare("Rail29.Value", Qt::CaseInsensitive) != 0) {
			tEachDestData ItemData;
			ItemData.PageNum = 0x20;
			ItemData.CmdCode = 0x00;
			ItemData.BYTELen = 0x01;
			ItemData.wValue = (unsigned short)nRailIdx;

			strMsg = "There is no 'page'";
			m_pOutput->Log(strMsg.toLocal8Bit().data());

			BufferWrite(AllBuffDataOffset, (unsigned char*)&ItemData, sizeof(tEachDestData));
			AllBuffDataOffset += sizeof(tEachDestData);
		}

		for (int i = 0; i < itBuff.value().vData.size(); i++) {
			tEachDestData ItemData = itBuff.value().vData[i];
			BufferWrite(AllBuffDataOffset, (unsigned char*)&ItemData, sizeof(tEachDestData));
			AllBuffDataOffset += sizeof(tEachDestData);
		}
		nRailIdx++;
		//
	}


__end:
	if (File.isOpen())
		File.close();

	return ferror;
}