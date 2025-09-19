#include "DatParser.h"
#include <QFileInfo>
#include <QMessageBox>
DatParser::DatParser()
{
}



char* DatParser::getVersion() 
{
	return "1.0.0.0";
}
char* DatParser::getFormatName() 
{
	return C_DAT;
}





///判断是否真是dat文件
///返回-1出错，0表示不是dat文件，1表示是dat文件
bool DatParser::CheckIsDat(const QString& strFilePath)
{
	int Ret = 1, i = 0, BytesRead;
	bool IsChecked = false;
	QFile File(strFilePath);
	QString rData;
	if (File.open(QIODevice::ReadOnly)) {
		Ret = -1; goto __end;
	}
	rData = File.read(51);

	if (rData.length() != 51) {
		Ret = -1; goto __end;
	}
	for (i = 0; i < 16; i++) {
		if (rData[i * 3 + 2] != ';') {
			Ret = 0;
			IsChecked = true;
		}
	}
__end:
	if (File.isOpen()) {
		File.close();
	}
	return Ret;
}





bool DatParser::ConfirmFormat(QString& filename) 
{
	bool Ret = true;
	QString strMsg;
	QString FileExt = QFileInfo(filename).suffix();
	if (FileExt.compare("dat", Qt::CaseInsensitive) == 0) {
		if (CheckIsDat(filename) == 1) {
			strMsg ="the file will be parsed as .dat format. which the data format is \"data1;data2;data3;….;datan;\", Do you want to continue?";
			int boxResult = QMessageBox::question(nullptr, "Question", strMsg,
				QMessageBox::Yes | QMessageBox::No);

			if (boxResult == QMessageBox::Yes)
				Ret = true;
			else
				Ret = false;
		}
		else {
			Ret = false;
		}
	}
	else {
		Ret = false;
	}
	return Ret;
}


#define LINEBUF_SIZE (50)
int DatParser::TransferFile(QString& srcfn, QIODevice* dst) 
{
	m_Dst = dst;
	int LineNum = 0, BytesBufWrite;
	unsigned long SizeGet = 0, BytesRead;
	QString strMsg;
	long nbuf_addr = 0;
	unsigned char pDataWrite[16];
	QFile file(srcfn);
	if (file.open(!QIODevice::ReadOnly)) {
		ferror = C_Err_FileOpen;
		return ferror;
	}

	InitControl();
	FileSize = file.size();
	QString rData;

	while (!file.atEnd()) {
		memset(pDataWrite, 0, 16);
		LineNum++;
		mynRead(&file, LINEBUF_SIZE);
		rData = (char*)work;
		if (rData.length() == 0) {
			break;
		}
		if (ParserOneLine((const unsigned char*)rData.toStdString().c_str(), BytesRead, pDataWrite, (int*)&SizeGet, LineNum) == -1) {
			ferror = C_Err_FormatError;
			break;
		}
		//BytesBufWrite = m_pDataBuffer->BufferWrite(nbuf_addr, pDataWrite, SizeGet);
		m_Dst->seek(nbuf_addr);
		BytesBufWrite = m_Dst->write((const char*)pDataWrite, SizeGet);
		if (BytesBufWrite != SizeGet) {
			QString strErr;
			char buf[200];
			sprintf(buf, "Buffer write error, Addr=0x%I64X, len=0x%X, really write bytes=0x%X, At Line[%d]", nbuf_addr, SizeGet, BytesBufWrite, LineNum);
			strErr = buf;
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			ferror = C_Err_WriteError;
			goto __end;
		}
		nbuf_addr += SizeGet;
		UpdateProgress();
	}
__end:
	return ferror;
}

#define isdigital(a)	(((a)<='9' && (a)>='0')?1:0)
#define isalpha_hex(a)	((((a)>='A' && (a)<='F')||((a)>='a'&&(a)<='f'))?1:0)

static int _puthigh(unsigned char* place, unsigned char data)
{
	int ret = 0;
	if (isdigital(data)) {///get high 4 bits
		*place |= (data - '0') << 4;
	}
	else if (isalpha_hex(data)) {
		if (data >= 'a') {
			*place |= (data - 'a' + 0x0A) << 4;
		}
		else {
			*place |= (data - 'A' + 0x0A) << 4;
		}
	}
	else {
		ret = -1;
	}
	return ret;
}

static int _putlow(unsigned char* place, unsigned char data)
{
	int ret = 0;
	if (isdigital(data)) {///get low 4 bits
		*place |= data - '0';
	}
	else if (isalpha_hex(data)) {
		if (data >= 'a') {
			*place |= data - 'a' + 0x0A;
		}
		else {
			*place |= data - 'A' + 0x0A;
		}
	}
	else {
		ret = -1;
	}
	return ret;
}


int DatParser::ParserOneLine(const unsigned char* pData, int Size, unsigned char* pDataGet, int* SizeGet, int LineNum) 
{
	int i, Ret = 0, Bytes = 0;
	QString strMsg;
	int Column;
	if (pData[Size - 2] != 0x0D && pData[Size - 1] != 0x0A) {
		Column = Size - 1;
		strMsg = QString("Sorry, Line[%1], Column[%2] had invalid char").arg(LineNum, Column);
		m_pOutput->Error(strMsg.toLocal8Bit().data());
		Ret = -1; goto __end;
	}
	Bytes = (Size - 2) / 3;
	if ((Size - 2) % 3 != 0) {
		strMsg = QString("Sorry, Line[%1] has %2 chars, it must had %d chars").arg(LineNum, Bytes * +2);
		m_pOutput->Error(strMsg.toLocal8Bit().data());
		Ret = -1; goto __end;
	}

	for (i = 0; i < Bytes; i++) {
		if (pData[i * 3 + 2] != ';') {
			Column = i * 3 + 3;
			strMsg = QString("Sorry, Line[%d], Column[%d] had invalid char").arg(LineNum, Column);
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			Ret = -1; goto __end;
		}
		if (_puthigh(pDataGet + i, pData[i * 3]) == -1) {
			Column = i * 3 + 1;
			strMsg = QString("Sorry, Line[%d], Column[%d] had invalid char").arg(LineNum, Column);
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			Ret = -1; goto __end;
		}
		if (_putlow(pDataGet + i, pData[i * 3 + 1]) == -1) {
			Column = i * 3 + 2;
			strMsg = QString("Sorry, Line[%d], Column[%d] had invalid char").arg(LineNum, Column);
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			Ret = -1; goto __end;
		}
	}

	*SizeGet = Bytes;

__end:
	return Ret;
}