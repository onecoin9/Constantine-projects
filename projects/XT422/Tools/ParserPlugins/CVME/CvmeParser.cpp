#include "CvmeParser.h"
#include <QFileInfo>
CvmeParser::CvmeParser()
{
}



char* CvmeParser::getVersion()
{
	return "1.0.0.0";
}
char* CvmeParser::getFormatName()
{
	return C_CVME;
}
bool CvmeParser::ConfirmFormat(QString& filename)
{

	bool Ret = true;
	QString FileExt = QFileInfo(filename).suffix();
	if (FileExt.compare("vme", Qt::CaseInsensitive) == 0 || FileExt.compare("tde", Qt::CaseInsensitive) == 0) {
		Ret = true;
	}
	else {
		Ret = false;
	}
	return Ret;
}


#define TMPBUF_SIZE (32*1024)
int CvmeParser::TransferFile(QString& srcfn, QIODevice* dst)
{
	m_Dst = dst;
	OUTPUTFILEINFO FileInfo;
	//	UINT Length;
	unsigned int FileOffset = 0;
	unsigned long BytesRead = 0;
	QFile file(srcfn);

	FileInfo.FileType = OUTPUTFILE_VME;
	FileInfo.MAGIC[0] = 'A';
	FileInfo.MAGIC[1] = 'C';
	FileInfo.MAGIC[2] = 'A';
	FileInfo.MAGIC[3] = 'P';
	FileInfo.Offset = 0x20;

	if (!file.open(QIODevice::ReadOnly)) {
		ferror = C_Err_FileOpen;
		goto __end;
	}
	do {
		BytesRead = mynRead(&file, TMPBUF_SIZE);
		if (BytesRead <= 0) {
			ferror = C_Err_ReadError;
			break;
		}
		else {
			if (BytesRead != 0) {
				BufferWrite(FileInfo.Offset + FileOffset, work, BytesRead);
				FileOffset += BytesRead;
			}
		}
		UpdateProgress();
	} while (!file.atEnd());
	FileInfo.Length = FileOffset;
	BufferWrite(0, (unsigned char*)&FileInfo, sizeof(OUTPUTFILEINFO));
__end:
	return ferror;
}