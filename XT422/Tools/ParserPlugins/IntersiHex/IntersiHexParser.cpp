#include "IntersiHexParser.h"
#include "../public/DrvDllCom/ComTool.h"
IntersiHexParser::IntersiHexParser()
{
}

char* IntersiHexParser::getVersion()
{
	return "1.0.0.0";
}
char* IntersiHexParser::getFormatName()
{
	return C_INTERSIHEX;
}
bool IntersiHexParser::ConfirmFormat(QString& filename)
{

	bool Rtn = false;
	QFile File(filename);
	int Ret = 0;
	QString strLine;
	int LineMatch = 0, LineNum = 0;
	if (!File.open(QIODevice::ReadOnly)) {
		return false;
	}
	while (!File.atEnd()) {
		strLine = File.readLine();
		if (strLine[0] == '4' && strLine[1] == '9') {
			LineMatch++;
		}
		LineNum++;
		if (LineNum > 5) {
			break;
		}
	}
	if (LineMatch >= 4) {
		Rtn = true;
	}

	if (File.isOpen()) {
		File.close();
	}
	return Rtn;
}
int IntersiHexParser::TransferFile(QString& srcfn, QIODevice* dst)
{
	m_Dst = dst;
	int LineNum = 1;
	int Offset = 0, BytesGet, BytesTotal = 0, BufSize = 256;
	QString strMsg, strLine;
	QFile File(srcfn);
	unsigned int BytesWrite = 0;
	//unsigned long nStartBuffAddr = m_pDataBuffer->BufferGetCurPartition();
	ADR Addr = 0x10;
	unsigned char* pData = NULL;
	pData = new unsigned char[BufSize];
	if (!pData) {
		ferror = C_Err_MemAllocErr; goto __end;
	}
	if (!File.open(QIODevice::ReadOnly)) {
		ferror = C_Err_FileOpen;
		goto __end;
	}
	InitControl();
	FileSize = File.size();

	while (!File.atEnd()) {
		myfgets(&File);
		strLine = (char*)work;
		strLine = strLine.replace("\r", "\0");
		strLine = strLine.replace("\n", "\0");
		
		BytesGet = Str2Hex(work, pData, BufSize);
		if (BytesGet == -1) {
			QString strErr;
			strErr = QString("LineNum=%1 Has Invalid Characters").arg(LineNum);
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			ferror = C_Err_FormatError; goto __end;
		}
		if (BytesGet == 0) {///忽略空行
			continue;
		}
		BytesWrite = BufferWrite(Addr, pData, BytesGet);
		if (BytesWrite != BytesGet) {
			char buf[200];
			sprintf(buf, "Buffer write Data error,LineNum=%d Addr=0x%X, len=0x%X, really write bytes=0x%X", LineNum, Addr, 1, BytesWrite);
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			ferror = C_Err_WriteError; goto __end;
		}
		LineNum++;
		Addr += BytesWrite;
		BytesTotal += BytesWrite;
	}
	///在头部插入4字节的数据大小和总行数
	LineNum -= 1;
	//m_pDataBuffer->BufferWrite(0 + nStartBuffAddr, (unsigned char*)&BytesTotal, 4);
	//m_pDataBuffer->BufferWrite(4 + nStartBuffAddr, (unsigned char*)&LineNum, 4);
	BufferWrite(0, (unsigned char*)&BytesTotal, 4);
	BufferWrite(4, (unsigned char*)&LineNum, 4);

__end:
	if (!pData) {
		delete pData;
	}
	if (File.isOpen())
		File.close();
	return ferror;
}