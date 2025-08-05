#include "ActParser.h"
#include <QFileInfo>
#include "../public/common/Serialize.h"
#include "../public/maingui/ChkSum.h"
#include <QFile>


#ifdef WIN32
#pragma pack()
#endif
#undef PACK_ALIGN

static unsigned char Magic[4] = { 'A','C','A','T' };
enum {
	TAG_APP = 1,
	TAG_PATCH = 2,
	TAG_USERDATA = 3,
	TAG_CONFIG = 4,
	TAG_TOTAL,
};

ActParser::ActParser()
{
}


char* ActParser::getVersion() 
{
	return "1.0.0.0";
}
char* ActParser::getFormatName()
{
	return C_ACT;
}
bool ActParser::ConfirmFormat(QString& filename)
{
	if (QFileInfo(filename).suffix().compare("act", Qt::CaseInsensitive) == 0) {
		return true;
	}
	else {
		return false;
	}
}

int ActParser::ReadUserDataFromTag(QFile& File, tTag& Tag)
{
	int i;
	CSerial lSerial;
	CHKINFO ChkInfo;
	QString strMsg;
	memset(&ChkInfo, 0, sizeof(CHKINFO));
		
	File.seek(Tag.Offset);
	//File.Seek(Tag.Offset, CFile::begin);
	//if (File.Read(pData, Tag.Size) != Tag.Size) {
	//	Ret = -1; goto __end;
	//}
	if (mynRead(&File, Tag.Size) != Tag.Size) {
		ferror = C_Err_ReadError;
		goto __end;
	}
	Crc32CalcSubRoutine(&ChkInfo, work, Tag.Size);
	Crc32GetChkSum(&ChkInfo);
	if ((unsigned int)ChkInfo.chksum != Tag.CRC32) {
		strMsg = "Sorry, The data in the file is damaged, Please recreate the ACT File";
		m_pOutput->Error(strMsg.toLocal8Bit().data());
		ferror = C_Err_FormatError;
	}
	try {
		unsigned int TokenCnt;
		unsigned int BuffAddr, DataSize, BytesWrite;
		lSerial.SerialInBuff(work, Tag.Size);
		lSerial >> TokenCnt;
		for (i = 0; i < TokenCnt; ++i) {
			lSerial >> BuffAddr >> DataSize;
			lSerial.SerialOutBuff(work, DataSize);
			//BytesWrite = m_pDataBuffer->BufferWrite(BuffAddr, work, DataSize);
			m_Dst->seek(BuffAddr);
			BytesWrite = m_Dst->write((char*)work, DataSize);
			if (BytesWrite != DataSize) {
				char buf[200];
				sprintf(buf, "Buffer write error, Addr=0x%I64X, len=0x%X, really write bytes=0x%X", BuffAddr, DataSize, BytesWrite);
				
				m_pOutput->Error(strMsg.toLocal8Bit().data());
				ferror = C_Err_WriteError; goto __end;
			}
		}
	}
	catch (...) {
		ferror = C_Err_RecordSum;
	}


__end:
	return ferror;
}

int ActParser::ReadFileFromTag(QFile& File, tTag& Tag)
{
	int Ret = 0;
	unsigned int BuffAddr, DataSize, BytesWrite;
	CHKINFO ChkInfo;
	QString strMsg;
	memset(&ChkInfo, 0, sizeof(CHKINFO));
	File.seek(Tag.Offset);
	mynRead(&File, 4);
	memcpy(&BuffAddr, work, 4);
	Crc32CalcSubRoutine(&ChkInfo, (unsigned char*)&BuffAddr, 4);
	mynRead(&File, 4);
	memcpy(&DataSize, work, 4);
	Crc32CalcSubRoutine(&ChkInfo, (unsigned char*)&DataSize, 4);

	if (mynRead(&File, DataSize) != DataSize) {
		Ret = C_Err_ReadError; goto __end;
	}
	Crc32CalcSubRoutine(&ChkInfo, work, DataSize);
	Crc32GetChkSum(&ChkInfo);
	if ((unsigned int)ChkInfo.chksum != Tag.CRC32) {
		strMsg = "Sorry, The data in the file is damaged, Please recreate the ACT File";
		m_pOutput->Error(strMsg.toLocal8Bit().data());
		Ret = -1;
	}
	//BytesWrite = m_pDataBuffer->BufferWrite(BuffAddr, work, DataSize);
	m_Dst->seek(BuffAddr);
	BytesWrite = m_Dst->write((char*)work, DataSize);
	if (BytesWrite != DataSize) {
		//QString strErr;
		char buf[200];
		sprintf(buf, "Buffer write error, Addr=0x%I64X, len=0x%X, really write bytes=0x%X", BuffAddr, DataSize, BytesWrite);
		m_pOutput->Error(strMsg.toLocal8Bit().data());
		Ret = C_Err_WriteError; goto __end;
	}

__end:
	return Ret;
}


int ActParser::TransferFile(QString& srcfn, QIODevice* dst)
{
	int i;
	QString strMsg;
	QFile TmpFile(srcfn);
	QVector<tTag> vTags;
	m_Dst = dst;
	InitControl();
	FileSize = TmpFile.size();
	if (!TmpFile.open(QIODevice::ReadOnly)) {
		ferror = C_Err_FileOpen;
		goto __end;
	}

	if (mynRead(&TmpFile, sizeof(tHeader)) != sizeof(tHeader)) {
		ferror = C_Err_ReadError; goto __end;
	}
	if (memcmp(((tHeader*)work)->Magic, Magic, 4) != 0) {
		strMsg = "Sorry, The file is not an invalid ACT file";
		m_pOutput->Error(strMsg.toLocal8Bit().data());
		ferror = C_Err_FormatError; goto __end;
	}

	for (i = 0; i < ((tHeader*)work)->TagCnt; ++i) {
		tTag* TmpTag = (tTag*)work;
		if (mynRead(&TmpFile, sizeof(tTag)) != sizeof(tTag)) {
			ferror = C_Err_ReadError; goto __end;
		}
		if (TmpTag->TagID != 0) {
			vTags.push_back(*TmpTag);
		}
	}

	for (i = 0; i < vTags.count(); ++i) {
		if (vTags[i].TagID == TAG_APP || vTags[i].TagID == TAG_PATCH) {
			ferror = ReadFileFromTag(TmpFile, vTags[i]);
			if (ferror != 0) {
				goto __end;
			}
		}
		else if (vTags[i].TagID == TAG_USERDATA) {
			ferror = ReadUserDataFromTag(TmpFile, vTags[i]);
			if (ferror != 0) {
				goto __end;
			}
		}
		UpdateProgress(i / vTags.count() * 100);
	}

__end:
	TmpFile.close();
	return ferror;
}