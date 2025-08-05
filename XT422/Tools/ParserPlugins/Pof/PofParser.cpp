#include "PofParser.h"
#include <QFileInfo>




void PofParser::pof_init_crc()
{
	crc_register = 0xFFFF; /* start with all ones in shift 		reg */
}
void PofParser::pof_compute_crc(unsigned char in_byte)
{
	int bit, feedback;

	/* compute for each bit in in_byte */
	for (bit = 0; bit < CHAR_BIT; bit++)
	{
		feedback = (in_byte ^ crc_register) & 0x01; /* XOR low bits */
		crc_register >>= 1; /* shift the shift register */
		if (feedback)
			crc_register ^= CCITT_CRC; /* invert selected bits */
		in_byte >>= 1; /* get the next bit of in_byte */
	}
}
void PofParser::pof_calc_crc(unsigned char* pData, int size)
{
	int i;
	for (i = 0; i < size; ++i) {
		pof_compute_crc(pData[i]);
	}
}

unsigned int PofParser::pof_crc_value()
{
	return(~crc_register); /* CRC is complement of shift register */
}

PofParser::PofParser()
{
	memcpy(gMagicPof, "POF", 3);
	gMagicPof[4] = 0;
}

char* PofParser::getVersion() 
{
	return "1.0.0.0";
}

char* PofParser::getFormatName()
{
	return C_POF;
}

bool PofParser::ConfirmFormat(QString& filename)
{
	QFileInfo fileinfo(filename);
	if (fileinfo.suffix().compare("POF", Qt::CaseInsensitive) == 0)
	{
		return true;
	}
	else {
		return false;
	}

}
int PofParser::HandleStrTagInfo(struct PACKET_HEAD& PofPackHead, unsigned char* pTagData, char* StrTag)
{
	int Ret = 0;
	if (pTagData) {
		QString strMsg;
		strMsg = QString(StrTag) + (char*)pTagData;
		m_pOutput->Log(strMsg.toLocal8Bit().data());
	}
	else {
		Ret = -1;
	}

__end:
	return Ret;
}

int PofParser::HandleTerminator(struct PACKET_HEAD& PofPackHead, unsigned char* pTagData, unsigned short CRCCacl)
{
	int Ret = 0;
	QString strMsg;
	unsigned short CrcGet = 0;
	if (PofPackHead.length == sizeof(unsigned short)) {
		CrcGet = *(unsigned short*)pTagData;
		/// If this file_crc value is zero, the CRC check should be ignored.
		if (CrcGet != 0) {///如果文件中的CRC值为0表示不进行校验值比对
			if (CrcGet != CRCCacl) {
				char buf[100];
				sprintf(buf, "Crc Compare Error, CrcInFile:0x%X, CrcCalc:0x%X", CrcGet, CRCCacl);
				strMsg = buf;
				m_pOutput->Error(strMsg.toLocal8Bit().data());
				Ret = -1; goto __end;
			}
			else {
				strMsg = "Crc Compare Successfully";
				m_pOutput->Log(strMsg.toLocal8Bit().data());
			}
		}
	}

__end:
	return Ret;
}


int PofParser::TransferFile(QString& srcfn, QIODevice* dst)
{
	m_Dst = dst;
	int i;
	QString strMsg;
	struct POF_HEADER PofHeader;
	struct PACKET_HEAD PofPackHead;
	QFile PofFile(srcfn);
	pof_init_crc();
	//PofFile.Read()
	m_BufferDataOffset = 0;
	if (!PofFile.open(QIODevice::ReadOnly)) {
		ferror = C_Err_FileOpen;
		goto __end;
	}

	InitControl();
	FileSize = PofFile.size();
	//PofFile.read((char*)&PofHeader, sizeof(struct POF_HEADER));
	mynRead(&PofFile, sizeof(struct POF_HEADER));
	memcpy(&PofHeader, work, sizeof(struct POF_HEADER));
	//if (memcmp(PofHeader.file_type, gMagicPof, 4) != 0) {///比对Magic
	//	ferror = C_Err_FormatError; goto __end;
	//}

	pof_calc_crc((unsigned char*)&PofHeader, sizeof(struct POF_HEADER));
	BufferWrite(m_BufferDataOffset, (unsigned char*)&PofHeader, sizeof(struct POF_HEADER));
	m_BufferDataOffset += sizeof(struct POF_HEADER);

	for (i = 0; i < PofHeader.packet_count; ++i) {
		if (mynRead(&PofFile, sizeof(struct PACKET_HEAD)) != sizeof(struct PACKET_HEAD)) {
			ferror = C_Err_ReadError; goto __end;
		}
		/*if (PofFile.read((char*)&PofPackHead, sizeof(struct PACKET_HEAD)) != sizeof(struct PACKET_HEAD)) {
			Ret = -1; goto __end;
		}*/
		memcpy(&PofPackHead, work, sizeof(struct PACKET_HEAD));
		pof_calc_crc((unsigned char*)&PofPackHead, sizeof(struct PACKET_HEAD));
		BufferWrite(m_BufferDataOffset, (unsigned char*)&PofPackHead, sizeof(struct PACKET_HEAD));
		m_BufferDataOffset += sizeof(struct PACKET_HEAD);

		//pTmpData = new unsigned char[PofPackHead.length];
		//if (PofFile.read((char*)pTmpData, PofPackHead.length) != PofPackHead.length) {
		//	Ret = -1; goto __end;
		//}
		if (mynRead(&PofFile, PofPackHead.length) != PofPackHead.length) {
			ferror = C_Err_ReadError; goto __end; 
		}
		BufferWrite(m_BufferDataOffset, work, PofPackHead.length);

		m_BufferDataOffset += PofPackHead.length;

		if (PofPackHead.tag != POFTAG_Terminator) {///除了Terminator其他的Tag都是需要计入CRC计算范围，
			pof_calc_crc(work, PofPackHead.length);
		}
		//strMsg.Format("Handle Tag: %d",PofPackHead.tag);
		m_pOutput->Warning(strMsg.toLocal8Bit().data());

		switch (PofPackHead.tag) {
		case POFTAG_Creator_ID:
			if (HandleStrTagInfo(PofPackHead, work, "CreatorID"))
				ferror = C_Err_FormatError;
			break;
		case POFTAG_Device_Name:
			if (HandleStrTagInfo(PofPackHead, work, "DeviceName"))
				ferror = C_Err_FormatError;
			break;
		case POFTAG_Comment_Text:
			if(HandleStrTagInfo(PofPackHead, work, "CommentText"))
				ferror = C_Err_FormatError;
			break;
		case POFTAG_Terminator:
		{
			unsigned short CrcCalc = pof_crc_value();
			if(HandleTerminator(PofPackHead, work, CrcCalc))
				ferror = C_Err_FormatError;
			break;
		}
		default: ///其他TAG都不处理，后续驱动自己解析
			/*strMsg.Format("Not Support Tag: %d",PofPackHead.tag);
			m_pOutput->Log(strMsg);
			Ret=-1; goto __end;*/
			break;
		}
		
		if (ferror != 0) {
			strMsg = QString("Handle Tag[%1] Failed").arg(PofPackHead.tag);
			m_pOutput->Error(strMsg.toLocal8Bit().data());
			goto __end;
		}
		UpdateProgress(i / PofHeader.packet_count * 100);
	}
__end:

	return ferror;
}