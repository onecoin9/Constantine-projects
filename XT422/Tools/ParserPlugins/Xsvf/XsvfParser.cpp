#include "XsvfParser.h"
#include <QString>
XsvfParser::XsvfParser()
{
}


char* XsvfParser::getVersion()
{
	return "1.0.0.0";
}
char* XsvfParser::getFormatName()
{
	return C_XSVF;
}
bool XsvfParser::ConfirmFormat(QString& filename)
{
	bool Ret = true;
	QString FileExt = QFileInfo(filename).suffix();
	
	if (FileExt.compare("xsvf") == 0) {
		Ret = true;
	}
	else {
		Ret = false;
	}
	return Ret;
}
int XsvfParser::TransferFile(QString& srcfn, QIODevice* dst)
{
	m_Dst = dst;
	int Ret = 0;
	OUTPUTFILEINFO FileInfo;
	//	UINT Length;
	unsigned int FileOffset = 0;
	unsigned long BytesRead = 0;
	QFile file(srcfn);
	if (!file.open(QIODevice::ReadOnly)) {
		emit ErrorRpt(ferror = C_Err_FileOpen, C_ErrMsg_FileOpen);
		return ferror;
	}
	InitControl();
	FileSize = file.size();
	FileInfo.FileType = OUTPUTFILE_XSVF;
	FileInfo.MAGIC[0] = 'A';
	FileInfo.MAGIC[1] = 'C';
	FileInfo.MAGIC[2] = 'A';
	FileInfo.MAGIC[3] = 'P';
	FileInfo.Offset = 0x20;
	do {
		BytesRead = mynRead(&file, C_BufSize);
		if (BytesRead <= 0) {
			emit ErrorRpt(ferror = C_Err_ReadError, C_ErrMsg_FileOpen);
			break;
		}
		else {
			BufferWrite(FileInfo.Offset + FileOffset, work, BytesRead);
			FileOffset += BytesRead;
		}
		UpdateProgress();
	} while (!file.atEnd());
	FileInfo.Length = FileOffset;
	BufferWrite(0, (unsigned char*)&FileInfo, sizeof(OUTPUTFILEINFO));

__end:
	return Ret;
}