#include "MPSHR12Parser.h"

typedef struct DestDataFmt {
	unsigned char Cmd;
	unsigned char Addr;
	unsigned char NumCnt;
	unsigned char Data[2];
	void ReInit() {
		Addr = 0x00;
		Cmd = 0x20;
		NumCnt = 0x00;
		memset(Data, 0, 2);
	};
	DestDataFmt() {
		ReInit();
	};
}tDestDataFmt;

#define DATAWRITE_OFFSET (4096)

MPSHR12Parser::MPSHR12Parser()
{
}

char* MPSHR12Parser::getVersion()
{
	return "1.0.0.0";
}
char* MPSHR12Parser::getFormatName()
{
	return C_MPSHR12;
}
bool MPSHR12Parser::ConfirmFormat(QString& filename)
{
	bool ret = false;
	int Rtn = 0, LineNum = 0;
	QFile File(filename);
	QString strOneLine;
	bool bEndFlag = false;
	QString temp;
	QVector<QString> vItemColList;
	int nItemColCnt = 0;
	int valueConv = 0;
	char* str;
	QString strConvTemp;
	int iValue = 0;
	int lineNum = 0;
	QString strHeadNote;
	QString strTempRowSrc;
	int nTotalCnt_WithM_FR_ADDR = 0;
	int 	nEveryItemWithM_FR_ADDR = 0;
	QString strMsg;
	QString strSign;
	strSign = "address	bits	value(HEX)	name	";
	bool bHasSymb = false;

	if (QFileInfo(filename).suffix().compare("txt", Qt::CaseInsensitive) != 0) {
		Rtn = false; goto __end;
	}

	if (File.open(QIODevice::ReadOnly) == false) {
		Rtn = false; goto __end;
	}

	while (!File.atEnd()) {
		strOneLine = File.readLine();
		if (strOneLine.isEmpty()) {
			continue;
		}

		lineNum++;

		strOneLine = strOneLine.replace("\r\n", ""); // 0x0d0a

		if ((strOneLine.left(3)).compare("***", Qt::CaseInsensitive) == 0) {
			continue;
		}

		if (lineNum == 1) {
			if (strOneLine.indexOf(strSign) >= 0) {
				bHasSymb = true;
			}
			continue;
			/*if (strOneLine.CompareNoCase(strSign) == 0){

			}*/
		}
		strOneLine = strOneLine.remove(QRegExp("^\\s+"));
		//strOneLine.Replace(" ", ";");//空格键,  
		//strOneLine.Remove(0x20); //驱动要求只以0x09作为分隔符
		strOneLine = strOneLine.replace(0x09, ';');//为Tab键
		//strOneLine.Replace(0x20, ";"); //空格键

		strTempRowSrc = strOneLine;
		//while(strTempRowSrc.Find(";;", 0) >= 0){
		//	strTempRowSrc.Replace(";;", ";");
		//}

		//strTempRowSrc.Replace(";", ",");
		strOneLine = strTempRowSrc;

		strOneLine = strOneLine.replace("\r\n", ""); // 0x0d0a
		//strOneLine+=",";

		nItemColCnt = 0;
		vItemColList.clear();

		if (strOneLine.length() == 0) {
			continue;
		}

		while (strOneLine.length() > 0) {
			int index = strOneLine.indexOf(";", 0);
			if (index < 0) {
				goto __end;
			}

			if (index < 1) { //每列至少一个字符
				goto __end;
			}

			temp = strOneLine.left(index);//取每个分割段里的值

			if (nItemColCnt == 1) {
				valueConv = temp.toInt();
			}
			else if (nItemColCnt == 0 || nItemColCnt == 2) { //16进制转10进制
				strConvTemp = temp;
				iValue = -1;
				//int iValue = (int)strtol((LPCTSTR)strConvTemp, &str, 16);
				sscanf(strConvTemp.toLocal8Bit().data(), "%x", &iValue);
				temp = QString::number(iValue);
				if (iValue < 0) {
					goto __end;
				}
			}

			strOneLine = strOneLine.remove(0, index + 1);
			nItemColCnt++;
			vItemColList.push_back(temp);
		}

		if (nItemColCnt != 4) {//必须要有4列
			goto __end;
		}
	}

	ret = true;
	if (!bHasSymb) {
		ret = false;
	}

__end:
	return ret;
}
int MPSHR12Parser::TransferFile(QString& srcfn, QIODevice* dst)
{
	m_Dst = dst;
	QString strMsg;
	int valueConv = 0;
	unsigned short sum = 0;
	QFile File(srcfn);
	QString strOneLine;
	QString strOneRow;
	QString temp;
	QVector<QString> vItemColList;
	QVector<unsigned char> vHexData;
	QString strFirstCol;
	int nFirstColFlag = -1;

	QString strTempRowSrc;
	int nItemColCnt = 0;

	int off = 5;
	int LineNum = 0;

	QString strConvTemp;
	int iValue;

	if (File.open(QIODevice::ReadOnly) == false) {
		strMsg = QString("Open File Failed: %1").arg(srcfn);
		m_pOutput->Error(strMsg.toLocal8Bit().data());
		ferror = C_Err_FileOpen; goto __end;
	}

	int nCurPageNum = -1; //当前的pagenum
	tDestDataFmt RecordOneItem; //
	RecordOneItem.Addr = 0x00;
	RecordOneItem.Cmd = 0x20;
	RecordOneItem.NumCnt = 1;

	BufferWrite(0, (unsigned char*)&RecordOneItem, sizeof(tDestDataFmt));
	InitControl();
	FileSize = File.size();

	while (!File.atEnd()) {
		myfgets(&File);
		strOneLine = (char*)work;
		LineNum++;
		strOneLine = strOneLine.replace("\r\n", ""); // 0x0d0a
		if (strOneLine.isEmpty()) {
			continue;
		}
		if (LineNum == 1) {
			continue;
		}
		strOneRow = strOneLine;
		strOneLine = strOneLine.remove(QRegExp("^\\s+"));
		strOneLine = strOneLine.replace(0x09, ';');//为Tab键

		strTempRowSrc = strOneLine;

		/*strTempRowSrc.Replace(";", ",");
		strOneLine.Format("%s", strTempRowSrc);*/

		strOneLine = strOneLine.replace("\r\n", ""); // 0x0d0a
		//strOneLine+=",";

		nItemColCnt = 0;
		vItemColList.clear();

		if (strOneLine.isEmpty()) {
			continue;
		}

		tDestDataFmt DestRecordOneItem; //存放解析到的每行数据
		DestRecordOneItem.Cmd = 0x20;

		while (strOneLine.length() > 0) {
			int index = strOneLine.indexOf(";", 0);
			if (index < 0) {
				ferror = C_Err_FormatError;
				strMsg = "format error, please check";
				m_pOutput->Error(strMsg.toLocal8Bit().data());
				goto __end;
			}

			if (index < 1) {
				ferror = C_Err_FormatError;
				strMsg = "format error, please check";
				m_pOutput->Error(strMsg.toLocal8Bit().data());
				goto __end;
			}

			vHexData.clear();
			temp = strOneLine.left(index); //每个分隔段里的内容

			if (nItemColCnt == 0 || nItemColCnt == 2) {
				//if (temp.GetLength()%2 !=0){
				//	CString strBuild;
				//	strBuild.Format("0");
				//	strBuild +=temp;
				//	temp.Format("%s", strBuild);
				//}

				//if(ComTool::Str2Hex(temp, /*ComTool::ENDIAN_LIT*/ComTool::ENDIAN_BIG|ComTool::PREHEADZERO_NEED, vHexData)==FALSE){
				//	ret = -1;
				//	strMsg.Format("%s", "String to hex transfer failed, please check");
				//	m_pOutput->Error(strMsg);
				//	goto __end;
				//}

				strConvTemp = temp;
				iValue = -1;
				sscanf(strConvTemp.toLocal8Bit().data(), "%x", &iValue);
				temp = QString::number(iValue);

				if (iValue < 0) {
					goto __end;
				}

				if (nItemColCnt == 0) { //16进制转10进制
					DestRecordOneItem.Addr = iValue;
				}
				else {
					if (iValue > 0xFFFF) {
						goto __end;
					}
					memcpy(DestRecordOneItem.Data, (unsigned char*)&iValue, 2);
				}
				/*if (vHexData.size() > 2){
					ret = -1;
					strMsg.Format("col[%d] there is more than two byte, please check", nItemColCnt+1);
					m_pOutput->Error(strMsg);
					goto __end;
				}

				std::copy(vHexData.begin(), vHexData.end(), DestRecordOneItem.Data);*/
			}
			else if (nItemColCnt == 1) {
				iValue = -1;
				iValue = temp.toInt();
				if (iValue <= 0 || iValue > 0xFF) {
					ferror = C_Err_RecordSum;
					strMsg = "There is some error, please check";
					m_pOutput->Error(strMsg.toLocal8Bit().data());
					goto __end;
				}

				DestRecordOneItem.NumCnt = (iValue % 8 == 0) ? (iValue / 8) : (iValue / 8 + 1);

				temp = QString::number(iValue);
			}

			vItemColList.push_back(temp);
			strOneLine = strOneLine.remove(0, index + 1);
			nItemColCnt++;
		}

		//-----------------more check----------------//
		if (nItemColCnt != 4) {

			ferror = C_Err_FormatError;
			strMsg = "less than 4 col, please check";
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			goto __end;
		}
		//----------------------------------------------//

		BufferWrite(off, (unsigned char*)&DestRecordOneItem, sizeof(tDestDataFmt));
		off += sizeof(tDestDataFmt);
	}


__end:
	return ferror;
}
