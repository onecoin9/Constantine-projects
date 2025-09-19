#include "MpsParser.h"
#include "..\public\DrvDllCom\ComTool.h"

typedef struct tagChipInfo_Head_Type
{
	char ProductID[32];		// 芯片名
	char Version[32];		  	// 版本号
	unsigned int CrcDataLen;	    // 所有数据长度 (LSB)
	unsigned int CrcDataChk;	    // 所有数据累加和，用于判断档案的完整性 (LSB)
	unsigned int CrcNum;			// crc命令条数(LSB)
	void ReInit() {
		memset(ProductID, 0, 32);
		memset(Version, 0, 32);
		CrcDataLen = 0;
		CrcDataChk = 0;
		CrcNum = 0;
	}
	tagChipInfo_Head_Type() {
		ReInit();
	}
}ChipInfo_Head_Type;

MpsParser::MpsParser()
{
}

char* MpsParser::getVersion()
{
	return "1.0.0.0";
}
char* MpsParser::getFormatName()
{

	return C_MPS;
}
bool MpsParser::ConfirmFormat(QString& filename)
{
	bool ret = false;
	int LineNum = 0;
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

	QString strExt = QFileInfo(filename).suffix();
	if (strExt.compare("txt", Qt::CaseInsensitive) != 0) {
		ret = false; goto __end;
	}

	if (!File.open(QIODevice::ReadOnly)) {
		ret = false; goto __end;
	}

	while (!File.atEnd()) {
		lineNum++;
		strOneLine = File.readLine();
		if (strOneLine.isEmpty()) {
			continue;
		}

		strOneLine = strOneLine.replace("\r\n", ""); // 0x0d0a
		strOneLine = strOneLine.mid(strOneLine.indexOf(QRegExp("\\S")));

		if ((strOneLine.left(3)).compare("***") == 0) {
			continue;
		}

		if ((strOneLine.left(3)).compare("END", Qt::CaseInsensitive) == 0) {
			bEndFlag = true;
			break;
			continue;
		}

		strOneLine = strOneLine.replace(", ", "");//去除逗号空格，逗号TAP
		strOneLine = strOneLine.replace(",	", "");
		strOneLine = strOneLine.remove(',');
		//strOneLine.Replace(" ", ";");//空格键,  
		strOneLine = strOneLine.remove(0x20); //驱动要求只以0x09作为分隔符
		strOneLine = strOneLine.replace(0x09, ';');//为Tab键
		//strOneLine.Replace(0x20, ";"); //空格键

		strTempRowSrc = strOneLine;
		while (strTempRowSrc.indexOf(";;", 0) >= 0) {
			strTempRowSrc = strTempRowSrc.replace(";;", ";");
		}
		//修复MPS档案格式最后一列是09(多了一列的bug)
		int nLastPos = strTempRowSrc.lastIndexOf(';');
		if (nLastPos > 0) {
			if ((nLastPos + 1) == strTempRowSrc.length()) {
				strTempRowSrc = strTempRowSrc.remove(nLastPos, 1);
			}
		}
		strTempRowSrc = strTempRowSrc.replace(";", ",");
		strOneLine = strTempRowSrc;

		/*strOneLine.Replace(";;", ",");
		strOneLine.Replace(";", ",");*/
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

			if (index < 1) { //每列至少一个字符
				goto __end;
			}

			temp = strOneLine.left(index);//取每个分割段里的值

			if (nItemColCnt == 0 || nItemColCnt == 4) {  //第0，4列不需要解析
				if (nItemColCnt == 4) {
					int nReplace = temp.indexOf("MFR_ADDR_");
					temp = temp.replace("MFR_ADDR_", "");
					if (nReplace != -1) {  //MPtxt格式的MFR_ADDR_后面必须是接16进制的
						nEveryItemWithM_FR_ADDR++;
						//vector<unsigned char> vHexData;
						// temp.Replace("H", "");
						// temp.Replace("h", "");
						// if(ComTool::Str2Hex(temp, /*ComTool::ENDIAN_LIT*/ComTool::ENDIAN_BIG|ComTool::PREHEADZERO_NEED, vHexData)==TRUE){
						//	 goto __end;
						// }
						//goto __end;
					}
				}
				strOneLine = strOneLine.remove(0, index + 1);
				nItemColCnt++;
				vItemColList.push_back(temp);

				continue;
			}

			if (nItemColCnt == 1 || nItemColCnt == 3 || nItemColCnt == 6) {
				valueConv = temp.toInt();
			}

			if (nItemColCnt == 2 || nItemColCnt == 5) { //16进制转10进制
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

		nTotalCnt_WithM_FR_ADDR++;
		if (nEveryItemWithM_FR_ADDR == nTotalCnt_WithM_FR_ADDR) {
			goto __end;
		}

		if (nItemColCnt != 7) {//必须要有7列
			goto __end;
		}

		if (vItemColList[2].compare(vItemColList[3], Qt::CaseInsensitive) != 0) { //第2列和第3列必须相等
			goto __end;
		}

		if (vItemColList[5].compare(vItemColList[6], Qt::CaseInsensitive) != 0) { //第5列和第6列必须相等
			goto __end;
		}
	}

	ret = false;
	if (bEndFlag) {
		ret = true;
	}
	if (File.isOpen()) {
		File.close();
	}
	return ret;

__end:
	if (File.isOpen()) {
		File.close();
	}
	return false;
}
int MpsParser::TransferFile(QString& srcfn, QIODevice* dst)
{
	QString strMsg;
	int valueConv = 0;
	unsigned short sum = 0;
	QFile File(srcfn);
	QString strOneLine;
	QString strOneRow;
	QString temp;
	QVector<QString> vItemColList;
	QVector<unsigned char> vHexData;

	QString strPreFirstCol;
	ChipInfo_Head_Type ChipHead;
	QMap<QString, int> CRCAreaMap;
	CRCAreaMap.insert("CRC_CHECK_START", 0);
	CRCAreaMap.insert("CRC_CHECK_STOP", 0);
	bool bHasSupportCRC = false;
	unsigned long nAddrChipInfo = 0;
	for (int j = 0; j < m_vBufInfo.size(); ++j) {
		if (QString(m_vBufInfo[j].strBufName.c_str()).compare("ChipInfo", Qt::CaseInsensitive) == 0) {
			nAddrChipInfo = m_vBufInfo[j].llBufStart; //写入的地址。
			bHasSupportCRC = true;
		}
	}

	QString strTempRowSrc;
	int nItemColCnt = 0;

	int off = 0;
	int LineNum = 0;

	QString strConvTemp;
	int iValue;

	if (!File.open(QIODevice::ReadOnly)) {
		strMsg = QString("Open File Failed: %1").arg(srcfn);
		m_pOutput->Error(strMsg.toLocal8Bit().data());
		ferror = C_Err_FileOpen; goto __end;
	}

	int nLastPageIdx = -1; //上一个pagenum
	int nCurPageNum = -1; //当前的pagenum
	InitControl();
	FileSize = File.size();
	m_Dst = dst;
	while (!File.atEnd()) {
		myfgets(&File);
		strOneLine = (char*)work;
		LineNum++;
		if (LineNum >= 24) {
			int re = 0;
		}
		if (strOneLine.isEmpty()) {
			continue;
		}

		strOneRow = strOneLine;

		strOneLine = strOneLine.replace("\r\n", ""); // 0x0d0a
		strOneLine = strOneLine.remove(QRegExp("^\\s+"));

		if ((strOneLine.left(3)).compare("***", Qt::CaseInsensitive) == 0) {
			QString strFlag;
			strFlag = "Configuration file:";
			int nIdx = strOneLine.indexOf(strFlag, 3);
			if (nIdx > 0) {
				strOneLine = strOneLine.remove(0, nIdx + strFlag.length());
				ChipHead.CrcDataLen = ChipHead.CrcNum * sizeof(tDestDataFmt);

				memcpy(ChipHead.ProductID, strOneLine.toLocal8Bit().data(), strOneLine.length());
				memcpy(ChipHead.Version, "V000", 4);
				if (bHasSupportCRC) {
					BufferWrite(nAddrChipInfo, (unsigned char*)&ChipHead, sizeof(ChipInfo_Head_Type));
				}
			}
			continue;
		}

		if ((strOneLine.left(3)).compare("END", Qt::CaseInsensitive) == 0) {
			//////////////////////////////////////////////////////////
			bool bCRCEnd = false;
			while (!File.atEnd()) {

				myfgets(&File);
				strOneLine = (char*)work;
				//已和驱动确认Multi Index可以不解析
				if (strOneLine.isEmpty() || (strOneLine.left(3)).compare("***", Qt::CaseInsensitive) == 0 || strOneLine.indexOf("Multi Index") == 0) {
					continue;
				}
				tDestDataFmt EachDestRecord; //存放解析到的每行数据
				EachDestRecord.ReInit();
				EachDestRecord.Addr = 0x20;

				int nValue = 0;
				int nLastCol = 0;
				nItemColCnt = 0;
				QString strRowTemp;
				strOneLine = strOneLine.replace("\r\n", "");
				strRowTemp = strOneLine;
				strRowTemp = strRowTemp.remove(0x09);
				strOneLine = strOneLine.replace(0x09, ',');

				if (strRowTemp.isEmpty()) {
					continue;
				}

				if (strOneLine.compare("CRC_CHECK_START", Qt::CaseInsensitive) == 0 ||
					strOneLine.compare("CRC_CHECK_STOP", Qt::CaseInsensitive) == 0) {
					CRCAreaMap[strOneLine]++;

					if (strOneLine.compare("CRC_CHECK_STOP", Qt::CaseInsensitive) == 0) {
						bCRCEnd = true;
						break;
					}
					continue;
				}

				strOneLine += ",";
				while (!strOneLine.isEmpty()) {
					QString strItem;
					int nConv = 0;
					int nIdx = strOneLine.indexOf(",", 0);
					if (nIdx <= 0) {
						goto __end;
					}
					strItem = strOneLine.left(nIdx);
					strOneLine = strOneLine.remove(0, nIdx + 1);

					if (nItemColCnt == 0) {
						if (strPreFirstCol.compare(strItem, Qt::CaseInsensitive) != 0) {
							goto __end;
						}
					}

					if (nItemColCnt == 2) {
						if (strItem.compare("NA", Qt::CaseInsensitive) == 0) { //存放FF
							EachDestRecord.Cmd = 0xff;
						}
						else {
							unsigned char arr[2] = { 0 };
							QVector<unsigned char> vData;
							if (ComTool::Str2Hex(strItem, ComTool::ENDIAN_BIG, vData) == false) {
								goto __end;
							}
							if (vData.size() == 0 || vData.size() > 2) {
								goto __end;
							}
							std::copy(vData.begin(), vData.end(), arr);
							EachDestRecord.Cmd = arr[0];
						}
					}
					else if (nItemColCnt == 5) {
						sscanf_s(strItem.toLocal8Bit().data(), "%x", &nValue);
					}
					else if (nItemColCnt == 6) {
						unsigned char arr[2] = { 0 };
						QString strNewValue;
						QVector<unsigned char> vData;
						sscanf_s(strItem.toLocal8Bit().data(), "%d", &nLastCol);
						if (nValue != nLastCol) {

							goto __end;
						}
						strNewValue = QString::number(nLastCol, 16);
						if (ComTool::Str2Hex(strNewValue, ComTool::ENDIAN_BIG, vData) == false) {
							goto __end;
						}
						if (vData.size() == 0 || vData.size() > 2) {
							goto __end;
						}
						std::copy(vData.begin(), vData.end(), arr);
						memcpy(EachDestRecord.Data, arr, vData.size());
						EachDestRecord.NumCnt = vData.size();
						BufferWrite(nAddrChipInfo +
							0x100 + ChipHead.CrcNum * (sizeof(tDestDataFmt)),
							(unsigned char*)&EachDestRecord,
							sizeof(tDestDataFmt));

						ChipHead.CrcNum++;
						ChipHead.CrcDataChk += EachDestRecord.Addr;
						ChipHead.CrcDataChk += EachDestRecord.Cmd;
						ChipHead.CrcDataChk += EachDestRecord.NumCnt;
						ChipHead.CrcDataChk += EachDestRecord.Data[0];
						ChipHead.CrcDataChk += EachDestRecord.Data[1];
					}
					nItemColCnt++;
				}

				if (nItemColCnt != 7) {
					strMsg = "Must be 7 colum!";
					m_pOutput->Error(strMsg.toLocal8Bit().data());
					goto __end;
				}
			}

			if ((CRCAreaMap["CRC_CHECK_START"] == CRCAreaMap["CRC_CHECK_STOP"])
				&& (CRCAreaMap["CRC_CHECK_START"] == 0)) {
				break;
			}

			if (CRCAreaMap["CRC_CHECK_START"] != 1) {
				goto __end;
			}
			if (CRCAreaMap["CRC_CHECK_START"] != CRCAreaMap["CRC_CHECK_STOP"]) {
				goto __end;
			}

			if (!bHasSupportCRC) {
				goto __end;
			}

			if (bCRCEnd) {
				continue;
			}
			//////////////////////////////////////////////////////////
			break;
			//continue ;
		}

		//strOneLine.Replace(" ", ";");//空格键
		strOneLine = strOneLine.replace(", ", "");//去除逗号空格，逗号TAP
		strOneLine = strOneLine.replace(",	", "");
		strOneLine = strOneLine.remove(',');
		strOneLine = strOneLine.remove(0x20); //驱动要求只以0x09作为分隔符
		strOneLine = strOneLine.replace(0x09, ';');//为Tab键
		//strOneLine.Replace(0x20, ";"); //空格键

		strTempRowSrc = strOneLine;
		while (strTempRowSrc.indexOf(";;", 0) >= 0) {
			strTempRowSrc = strTempRowSrc.replace(";;", ";");
		}

		//修复MPS档案格式最后一列是09(多了一列的bug)
		int nLastPos = strTempRowSrc.lastIndexOf(';');
		if (nLastPos > 0) {
			if ((nLastPos + 1) == strTempRowSrc.length()) {
				strTempRowSrc = strTempRowSrc.remove(nLastPos, 1);
			}
		}

		strTempRowSrc = strTempRowSrc.replace(";", ",");
		strOneLine = strTempRowSrc;

		/*strOneLine.Replace(";;", ",");
		strOneLine.Replace(";", ",");*/
		strOneLine = strOneLine.replace("\r\n", ""); // 0x0d0a
		strOneLine += ",";

		nItemColCnt = 0;
		vItemColList.clear();

		if (strOneLine.length() == 0) {
			continue;
		}

		tDestDataFmt DestRecordOneItem; //存放解析到的每行数据
		DestRecordOneItem.Addr = 0x20;

		bool bHasPageFlg = false;  //这行中是否有page关键字
		bool bIsNeedAddPageRowData = false;
		tDestDataFmt PageRowData;
		bool bCurrRowIsSpecialReg = false;

		while (strOneLine.length() > 0) {
			int index = strOneLine.indexOf(",", 0);
			if (index < 0) {
				strMsg = "format error, please check";
				m_pOutput->Error(strMsg.toLocal8Bit().data());
				goto __end;
			}

			if (index < 1) {
				strMsg = "format error, please check";
				m_pOutput->Error(strMsg.toLocal8Bit().data());
				goto __end;
			}

			vHexData.clear();
			temp = strOneLine.left(index); //每个分隔段里的内容

			if (nItemColCnt == 0) {
				strPreFirstCol = temp;
			}

			if (nItemColCnt == 1) {
				nCurPageNum = temp.toInt();
			}

			if (nItemColCnt == 4) {
				if (temp.compare("PAGE", Qt::CaseInsensitive) == 0) {
					bHasPageFlg = true;
				}
			}

			if (nItemColCnt == 2) {
				vHexData.clear();
				if (ComTool::Str2Hex(temp, ComTool::ENDIAN_LIT, vHexData) == false) {
					ferror = C_Err_RecordSum;
					strMsg = "String to hex transfer failed, please check";
					m_pOutput->Error(strMsg.toLocal8Bit().data());
					goto __end;
				}

				if (vHexData.size() != 1) {
					ferror = C_Err_FormatError;
					strMsg = "page num is more than one byte, please check";
					m_pOutput->Error(strMsg.toLocal8Bit().data());
					goto __end;
				}
				DestRecordOneItem.Cmd = vHexData[0];

				if ((temp.compare("99") == 0) || (temp.compare("9A", Qt::CaseInsensitive) == 0) ||
					(temp.compare("9B", Qt::CaseInsensitive) == 0) || (temp.compare("9D", Qt::CaseInsensitive) == 0)) {
					bCurrRowIsSpecialReg = true;
				}
			}

			if (nItemColCnt == 5) {
				vHexData.clear();
				if (temp.compare("0") == 0) {
					temp = "0000";
				}
				if (temp.length() % 2 != 0) {
					QString strBuild;
					strBuild = "0";
					strBuild += temp;
					temp = strBuild;
				}
				if (ComTool::Str2Hex(temp, /*ComTool::ENDIAN_LIT*/ComTool::ENDIAN_BIG | ComTool::PREHEADZERO_NEED, vHexData) == false) {
					ferror = C_Err_FormatError;
					strMsg = "String to hex transfer failed, please check";
					m_pOutput->Error(strMsg.toLocal8Bit().data());
					goto __end;
				}

				DestRecordOneItem.NumCnt = 0x01;
				/*if (iValue > 0xFF){ //不用这个判断
					DestRecordOneItem.NumCnt = 0x02;
				}	*/
				DestRecordOneItem.NumCnt = vHexData.size();

				if (vHexData.size() > 1) {
					
					std::reverse(vHexData.begin(), vHexData.end());
				}

				if (vHexData.size() == 2) {
					std::copy(vHexData.begin(), vHexData.end(), DestRecordOneItem.Data);
				}
				//std::copy_backward(vHexData.begin(), vHexData.end(), DestRecordOneItem.Data);

				if (bCurrRowIsSpecialReg) {
					DestRecordOneItem.NumCnt = 0x82;

					if (vHexData.size() > 3) {
						ferror = C_Err_FormatError;
						strMsg = QString("LineNum = %1, there is more 6 character, please check").arg(LineNum);
						m_pOutput->Error(strMsg.toLocal8Bit().data());
						goto __end;
					}

					if ((vHexData.size() == 3) && (vHexData[0] != 0x02)) {
						ferror = C_Err_FormatError;
						strMsg = QString("LineNum = %1, it's should be '02', please check").arg(LineNum);
						m_pOutput->Error(strMsg.toLocal8Bit().data());
						goto __end;
					}

					if (vHexData.size() > 2) {
						std::copy(vHexData.begin() + 1, vHexData.end(), DestRecordOneItem.Data);
					}
					if (vHexData.size() == 1) {
						std::copy(vHexData.begin(), vHexData.end(), DestRecordOneItem.Data);
					}
				}
				else {
					if (vHexData.size() > 2) {
						ferror = C_Err_FormatError;
						strMsg = QString("LineNum = %1, there is more 4 character, only 4 characters are allow, please check").arg(LineNum);
						m_pOutput->Error(strMsg.toLocal8Bit().data());
						goto __end;
					}
					std::copy(vHexData.begin(), vHexData.end(), DestRecordOneItem.Data);
				}
			}

			//------------------add check format--------------------//
			if (nItemColCnt == 2 || nItemColCnt == 5) { //16进制转10进制
				strConvTemp = temp;
				iValue = -1;
				sscanf(strConvTemp.toLocal8Bit().data(), "%x", &iValue);
				temp = QString::number(iValue);
				if (iValue < 0) {
					goto __end;
				}
			}

			vItemColList.push_back(temp);
			strOneLine = strOneLine.remove(0, index + 1);
			nItemColCnt++;
		}

		if (nCurPageNum != nLastPageIdx) { //需要增加page组数据
			bIsNeedAddPageRowData = true;
			PageRowData.Addr = 0x20;
			PageRowData.Cmd = nCurPageNum;
			PageRowData.NumCnt = 1;
			PageRowData.Data[0] = 0x00;
			PageRowData.Data[1] = 0x00;
			nLastPageIdx = nCurPageNum;

			if (bHasPageFlg == false) {
				PageRowData.Cmd = 0x00;
				PageRowData.Data[0] = (unsigned char)(vItemColList[1].toInt());
			}
		}

		//-----------------more check----------------//
		if (nItemColCnt != 7) {
			ferror = C_Err_FormatError;
			strMsg = "less than 7 col, please check";
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			goto __end;
		}

		if (vItemColList[2].compare(vItemColList[3], Qt::CaseInsensitive) != 0) {
			ferror = C_Err_FormatError;
			strMsg = QString("Line[%1]: value not match with the format, please check ").arg(LineNum);
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			goto __end;
		}

		if (vItemColList[5].compare(vItemColList[6], Qt::CaseInsensitive) != 0) {
			strMsg = QString("Line[%1], value not match with the format, please check").arg(LineNum);
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			ferror = C_Err_FormatError;
			goto __end;
		}
		//----------------------------------------------//
		//赋值page num



		if (bIsNeedAddPageRowData && (bHasPageFlg == false)) { //page行的信息 ,当行中有page标识的时候,只选择record的，不然重复
			BufferWrite(off, (unsigned char*)&PageRowData, sizeof(tDestDataFmt));
			off += sizeof(tDestDataFmt);
		}

		BufferWrite(off, (unsigned char*)&DestRecordOneItem, sizeof(tDestDataFmt));
		off += sizeof(tDestDataFmt);
		UpdateProgress();

	}

__end:
	if (File.isOpen())
		File.close();
	return ferror;
}