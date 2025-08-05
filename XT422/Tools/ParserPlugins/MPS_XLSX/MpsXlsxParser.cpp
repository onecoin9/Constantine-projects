#include "MpsXlsxParser.h"
#include "..\public\DrvDllCom\ComTool.h"
#include "..\public\xlsxlib\libxl.h"
#include "..\public\DrvDllCom\ComTool.h"
using namespace libxl;

MpsXlsxParser::MpsXlsxParser()
{
}



char* MpsXlsxParser::getVersion()
{
	return "1.0.0.0";
}
char* MpsXlsxParser::getFormatName()
{
	return C_MPSXLSX;
}
bool MpsXlsxParser::ConfirmFormat(QString& filename)
{
	bool Ret = false;

	int Rtn = 0, LineNum = 0;

	int nVauleRow = 0;
	int nTotalRow = 0;
	int nTotalCol = 0;

	if (QFileInfo(filename).suffix().compare("xlsx", Qt::CaseInsensitive) != 0) {
		Rtn = false; goto __end;
	}

	Book* book = xlCreateXMLBook();
	if (book == NULL) {
		Ret = false; goto __end;
	}

	if (!book->load(filename.toStdWString().c_str())) {
		Ret = false; goto __end;
	}

	Sheet* sheet = book->getSheet(0);
	if (sheet == NULL) {
		Ret = false; goto __end;
	}

	nTotalRow = sheet->lastRow();
	nTotalCol = sheet->lastCol();
	if (nTotalCol != 6)
	{
		Ret = false; goto __end;
	}
	while (nTotalRow > 0) {
		nTotalRow--;
		tagItemData itemData;
		
		QString strTemp;
		int nValue = 0;

		QString pTemp = QString::fromWCharArray(sheet->readStr(LineNum, 0));
		if (pTemp == NULL) {
			Ret = -1; goto __end;
		}
		sscanf(pTemp.toLocal8Bit().data(), "%x", &nValue);
		itemData.Device_Addr = (unsigned char)nValue;

		pTemp = QString::fromWCharArray(sheet->readStr(LineNum, 1));
		if (pTemp == NULL) {
			Ret = -1; goto __end;
		}
		sscanf(pTemp.toLocal8Bit().data(), "%x", &nValue);
		itemData.Command_Code = (unsigned char)nValue;

		pTemp = QString::fromWCharArray(sheet->readStr(LineNum, 3));
		if (pTemp == NULL) {
			Ret = -1; goto __end;
		}
		sscanf(pTemp.toLocal8Bit().data(), "%d", &nValue);
		itemData.Len = (unsigned char)nValue;

		pTemp = QString::fromWCharArray(sheet->readStr(LineNum, 5));
		if (pTemp == NULL) {
			Ret = -1; goto __end;
		}
		sscanf(pTemp.toLocal8Bit().data(), "%x", &nValue);
		itemData.Register_Value = (unsigned short)nValue;

		nVauleRow++;
	}
	if (nVauleRow >= 2) {
		Ret = true;
	}

__end:

	return Ret;
}
int MpsXlsxParser::TransferFile(QString& srcfn, QIODevice* dst)
{
	m_Dst = dst;
	int TransRtn = 0;
	int LineNum = 1;
	QString strErrMsg;
	int nVauleRow = 0;
	int nTotalRow = 0;
	unsigned int uOffSet = 0;

	if (QFileInfo(srcfn).suffix().compare("xlsx", Qt::CaseInsensitive) != 0) {
		ferror = C_Err_FormatError; goto __end;
	}

	Book* book = xlCreateXMLBook();
	if (book == NULL) {
		ferror = C_Err_MemAllocErr; goto __end;
	}
	wchar_t* tmparr = new wchar_t[srcfn.length() + 1];
	srcfn.toWCharArray(tmparr);
	if (!book->load(tmparr)) {
		ferror = C_Err_MemAllocErr; goto __end;
	}

	Sheet* sheet = book->getSheet(0);
	if (sheet == NULL) {
		ferror = C_Err_MemAllocErr; goto __end;
	}
	nTotalRow = sheet->lastRow();
	int rowCount = nTotalRow;
	InitControl();
	while (nTotalRow > 1) {
		nTotalRow--;
		tagItemData itemData;
		QString strTemp;
		int nValue = 0;

		QString pTemp = QString::fromWCharArray(sheet->readStr(LineNum, 0));
		if (pTemp == NULL) {
			ferror = C_Err_ReadError; goto __end;
		}
		TransRtn = sscanf(pTemp.toLocal8Bit().data(), "%x", &nValue);
		if (TransRtn != 1) {
			strErrMsg = QString("Line[%1] Get Device_Addr Failed").arg(LineNum);
			m_pOutput->Error(strErrMsg.toLocal8Bit().data());
			ferror = C_Err_RecordSum; goto __end;
		}
		itemData.Device_Addr = (unsigned char)nValue;

		pTemp = QString::fromWCharArray(sheet->readStr(LineNum, 1));
		if (pTemp == NULL) {
			ferror = C_Err_ReadError; goto __end;
		}
		TransRtn = sscanf(pTemp.toLocal8Bit().data(), "%x", &nValue);
		if (TransRtn != 1) {
			strErrMsg = QString("Line[%1] Get Command_Code Failed").arg(LineNum);
			m_pOutput->Error(strErrMsg.toLocal8Bit().data());
			ferror = C_Err_RecordSum; goto __end;
		}
		itemData.Command_Code = (unsigned char)nValue;

		pTemp = QString::fromWCharArray(sheet->readStr(LineNum, 3));
		if (pTemp == NULL) {
			ferror = C_Err_ReadError; goto __end;
		}
		TransRtn = sscanf(pTemp.toLocal8Bit().data(), "%d", &nValue);
		if (TransRtn != 1) {
			strErrMsg = QString("Line[%1] Get Len Failed").arg(LineNum);
			m_pOutput->Error(strErrMsg.toLocal8Bit().data());
			ferror = C_Err_RecordSum; goto __end;
		}
		itemData.Len = (unsigned char)nValue;

		pTemp = QString::fromWCharArray(sheet->readStr(LineNum, 5));
		if (pTemp == NULL) {
			ferror = C_Err_ReadError; goto __end;
		}
		TransRtn = sscanf(pTemp.toLocal8Bit().data(), "%x", &nValue);
		if (TransRtn != 1) {
			strErrMsg = QString("Line[%1] Get Register_Value Failed").arg(LineNum);
			m_pOutput->Error(strErrMsg.toLocal8Bit().data());
			ferror = C_Err_RecordSum; goto __end;
		}
		itemData.Register_Value = (unsigned short)nValue;
		nVauleRow++;

		BufferWrite(uOffSet, (unsigned char*)&itemData, sizeof(tagItemData));
		uOffSet += sizeof(tagItemData);
		LineNum++;
		UpdateProgress((rowCount - nTotalRow) / rowCount * 100);
	}

__end:
	if (tmparr)
		free(tmparr);
	return ferror;
}