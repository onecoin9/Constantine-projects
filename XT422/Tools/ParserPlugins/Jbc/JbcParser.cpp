#include "JbcParser.h"

JbcParser::JbcParser()
{
}


char* JbcParser::getVersion()
{
	return "1.0.0.0";
}
char* JbcParser::getFormatName()
{
	return C_JBC;
}

bool JbcParser::ConfirmFormat(QString& filename) {

	bool Ret = true;
	QFile file(filename);
	unsigned char Magic[4];

	if (!file.open(QIODevice::ReadOnly)) {
		return false;
	}

	if (file.read((char*)Magic, 4) < 4) {
		Ret = false;
	}
	else {
		///JBC文件的头四个字节为0x4A 0x41 0x4D 0x01
		if ((Magic[0] == 0x4A && Magic[1] == 0x41 && Magic[2] == 0x4D) && (Magic[3] == 0x00 || Magic[3] == 0x01)) {
			Ret = true;
		}
		else {
			Ret = false;
		}
	}

	file.close();

	return Ret;
}
int JbcParser::TransferFile(QString& srcfn, QIODevice* dst)
{
	m_Dst = dst;
	JBCFILEINFO FileInfo;
	//UINT Length;
	unsigned int FileOffset = 0;
	unsigned long BytesRead = 0;
	QFile file(srcfn);
	FileInfo.FileType = 1;
	FileInfo.MAGIC[0] = 'A';
	FileInfo.MAGIC[1] = 'C';
	FileInfo.MAGIC[2] = 'A';
	FileInfo.MAGIC[3] = 'P';
	FileInfo.Offset = 0x20;

	if (!file.open(QIODevice::ReadOnly)) {
		emit ErrorRpt(ferror = C_Err_FileOpen, C_ErrMsg_FileOpen);
		return ferror;
	}
	InitControl();
	FileSize = file.size();
	do {
		BytesRead = mynRead(&file, C_BufSize);
		if (BytesRead < 0) {
			ferror = C_Err_ReadError;
			emit ErrorRpt(ferror = C_Err_ReadError, C_ErrMsg_FileOpen);
			break;
		}

		BufferWrite(FileInfo.Offset + FileOffset, work, BytesRead);
		FileOffset += BytesRead;
	} while (!file.atEnd());
	FileInfo.Length = FileOffset;
	BufferWrite(0, (unsigned char*)&FileInfo, sizeof(JBCFILEINFO));

__end:
	return ferror;
}