#include "MicParser.h"
#include "..\public\DrvDllCom\ComTool.h"



#define PHASE_CNFG (1)
#define PHASE_MASK (2)
#define PHASE_IMAGE (3)
#define PHASE_END (4)

#define IMAGEBASE_ADDR (0x1000)


MicParser::MicParser()
{

	m_CNFGOffset = 0x03A0;
	m_ImgStrOffset = IMAGEBASE_ADDR;
	m_ImgDataOffset = m_ImgStrOffset + 0x400;
	m_MaskOffset = 0x0400;
	m_NoteOffset = 0;
}

char* MicParser::getVersion()
{
	return "1.0.0.0";
}
char* MicParser::getFormatName()
{
	return C_MIC;
}

bool MicParser::ConfirmFormat(QString& filename)
{
	return QFileInfo(filename).suffix().compare("mic", Qt::CaseInsensitive) == 0;
}

int MicParser::WriteStrLine(QString& strLine, unsigned int& Offset)
{
	BufferWrite(Offset, (unsigned char*)strLine.toLocal8Bit().data(), strLine.length());
	Offset += strLine.length();
	return 0;
}


int MicParser::WriteDataLine(QString& strLine, unsigned int& BufferOffset)
{
	int Ret = 0;
	QString strTmp = strLine;
	strTmp = strTmp.remove(' ');
	strTmp = strTmp.remove(0x0D);
	strTmp = strTmp.remove(0x0A);
	unsigned char DataGet[256];
	Ret = Str2Hex((unsigned char*)strTmp.toLocal8Bit().data(), DataGet, 256);
	if (Ret >= 0) {///返回实际转换的字节数
		if (BufferWrite(BufferOffset, DataGet, Ret) != Ret) {
			Ret = -1; goto __end;
		}
		BufferOffset += Ret;
	}

__end:
	return Ret;
}

int MicParser::TransferFile(QString& srcfn, QIODevice* dst)
{
	m_Dst = dst;
	int Ret = 0;
	int LineNum = 1;
	int Offset = 0;
	QString strMsg, strLine;
	QFile File(srcfn);
	unsigned int Addr = 0;
	unsigned int BytesWrite = 0;
	int Phase = 0, ImageCnt = 0;

	if (!File.open(QIODevice::ReadOnly)) {
		ferror = C_Err_FileOpen;
		goto __end;
	}
	InitControl();
	FileSize = File.size();
	while (!File.atEnd()) {
		myfgets(&File);
		strLine = (char*)work;

#ifdef _WIN32
		strLine += "\r\n";
#endif // _WIN32

		if (strLine[0] == 0x0D || strLine[0] == '/' || strLine[0] == 0x0A) {
			if (strLine[0] == '/') {//注释行
				WriteStrLine(strLine, m_NoteOffset);
			}
			LineNum++;
			continue;
		}
		if (ishex(strLine[0].cell()) && !strLine.contains("Config File")) {///为数据行
			if (Phase == PHASE_CNFG) {
				if (WriteDataLine(strLine, m_CNFGOffset) <= 0) {
					strMsg = QString("Parser CNFG Data At Line%1 Failed").arg(LineNum);
					m_pOutput->Error(strMsg.toLocal8Bit().data());
					ferror = C_Err_WriteError; goto __end;
				}
			}
			else if (Phase == PHASE_MASK) {
				if (WriteDataLine(strLine, m_MaskOffset) <= 0) {
					char buf[100];
					sprintf(buf, "Parser MASK Data At Line%d Failed", LineNum);
					m_pOutput->Error(buf);
					ferror = C_Err_WriteError; goto __end;
				}
			}
			else if (Phase == PHASE_IMAGE) {
				Ret = WriteDataLine(strLine, m_ImgDataOffset);
				if (Ret <= 0) {
					if (Ret == -1) {///如果解析出错并且存在非法字符则按照字符串处理
						WriteStrLine(strLine, m_ImgStrOffset);
					}
					else {
						char buf[100];
						sprintf(buf, "Parser Image Data At Line%d Failed", LineNum);
						m_pOutput->Error(buf);
						ferror = C_Err_WriteError; goto __end;
					}
				}
			}
			else if (Phase == 0) {///Header中的非注释行，在非注释行前面加上//编程注释行
				strLine.insert(0, "//");
				WriteStrLine(strLine, m_NoteOffset);
			}
		}
		else {
			if (strLine.indexOf("[CNFG]", 0) == 0) {
				Phase = PHASE_CNFG;
			}
			else if (strLine.indexOf("[Mask]", 0) == 0) {
				Phase = PHASE_MASK;
			}
			else if (strLine.indexOf("[Image", 0) == 0) {
				m_ImgStrOffset = IMAGEBASE_ADDR + ImageCnt * 0x1000;
				if (ImageCnt > 0 && m_ImgDataOffset > m_ImgStrOffset) {
					char buf[100];
					sprintf(buf, "Image Index[%d] Data Is Out Of Range,Please Check, ImageDataOffset=0x%X", ImageCnt - 1, m_ImgDataOffset);
					m_pOutput->Error(buf);
					ferror = C_Err_FormatError; goto __end;
				}
				m_ImgDataOffset = m_ImgStrOffset + 0x400;
				ImageCnt++;
				WriteStrLine(strLine, m_ImgStrOffset);
				Phase = PHASE_IMAGE;
			}
			else if (strLine.indexOf("[End]", 0) == 0) {
				Phase = PHASE_END;
			}
			else {
				if (Phase == PHASE_IMAGE) {///属于Image的注释部分
					WriteStrLine(strLine, m_ImgStrOffset);
				}
			}
			if (Phase == 0) {///Header中的非注释行，在非注释行前面加上//编程注释行
				strLine.insert(0, "//");
				WriteStrLine(strLine, m_NoteOffset);
			}
		}
		LineNum++;
		UpdateProgress();
	}

	if (ferror != 0) {
		char buf[100];
		sprintf(buf, "Line %1 Format Error", LineNum);
		m_pOutput->Error(buf);
	}
__end:

	m_CNFGOffset = 0x03A0;
	m_ImgStrOffset = IMAGEBASE_ADDR;
	m_ImgDataOffset = m_ImgStrOffset + 0x400;
	m_MaskOffset = 0x0400;
	m_NoteOffset = 0;
	if (File.isOpen())
		File.close();
	return ferror;

}