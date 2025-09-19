#include "ActelSTPPaser.h"
#include "../public/DrvDllCom/ComTool.h"
#include <stdio.h>
#include <QFileInfo>
extern "C" {
#include "JamStapl/jamport.h"
#include "JamStapl/jamexprt.h"
#include "JamStapl/jamdefs.h"
#include "JamStapl/jamexec.h"
#include "JamStapl/jamutil.h"
#include "JamStapl/jamexp.h"
#include "JamStapl/jamsym.h"
#include "JamStapl/jamstack.h"
#include "JamStapl/jamheap.h"
#include "JamStapl/jamarray.h"
#include "JamStapl/jamcomp.h"
}

#define VERSION "1.0.0.0"

typedef struct tParserInfoData {
	int nMagic;//MAGIC
	char strVersion[32];//解析器版本号
	void ReInit() {
		nMagic = 0x53415341;
		memset(strVersion, 0, sizeof(strVersion));
		strcpy(strVersion, VERSION);
	}
	tParserInfoData() {
		ReInit();
	};
}ParserInfoData;



#define PARSER_INFO_DATAWRITE_OFFSET	(0x0)
#define HEAD_INFO_DATAWRITE_OFFSET		(0x0001000)
#define NOTE_DATAWRITE_OFFSET			(0x000C000)
#define ACTION_DATAWRITE_OFFSET			(0x0010000)
#define BSRPATTERN_DATAWRITE_OFFSET		(0x0014000)
#define SAMPLEMASK_DATAWRITE_OFFSET		(0x0017000)
#define RLOCK_DATAWRITE_OFFSET			(0x0018000)
#define M1BUFF_DATAWRITE_OFFSET			(0x001A000)
#define M7BUFF_DATAWRITE_OFFSET			(0x001C000)
#define IDCODEVALUE_DATAWRITE_OFFSET	(0x001E000)
#define IDMASK_DATAWRITE_OFFSET			(0x001E040)
#define ACTUROWALGO_DATAWRITE_OFFSET	(0x001E080)
#define CHECKSUM_DATAWRITE_OFFSET		(0x001E100)
#define ACTUROWDESIGN_DATAWRITE_OFFSET	(0x001E180)
#define ACTUROWPROG_DATAWRITE_OFFSET	(0x001E200)
#define MDEVICE_DATAWRITE_OFFSET		(0x001E280)
#define ACTUROWPROGSW_DATAWRITE_OFFSET	(0x001E300)
#define ACTUROWMASK_DATAWRITE_OFFSET	(0x001E380)

#define DATASTREAM_DATAWRITE_OFFSET		(0xC000000)
#define FROMPLAIN_DATAWRITE_OFFSET		(0x10000000)
#define NVMPLAIN_DATAWRITE_OFFSET		(0x14000000)
#define SECURITY_DATAWRITE_OFFSET		(0x20000000)
#define STPFILE_DATAWRITE_OFFSET		(0x30000000)

int jam_6bit_char(int ch)
{
	int result = 0;

	if ((ch >= '0') && (ch <= '9')) result = (ch - '0');
	else if ((ch >= 'A') && (ch <= 'Z')) result = (ch + 10 - 'A');
	else if ((ch >= 'a') && (ch <= 'z')) result = (ch + 36 - 'a');
	else if (ch == '_') result = 62;
	else if (ch == '@') result = 63;
	else result = -1;	/* illegal character */

	return (result);
}


JAM_RETURN_TYPE jam_read_bool_compressed(char* station, long dimension, long* heap_record, long* outDataSize)
{
	int ch = 0;
	int bit = 0;
	int word = 0;
	int value = 0;
	long uncompressed_length = 0L;
	char* in = NULL;
	char* ch_data = NULL;
	long in_size = 0L;
	long out_size = 0L;
	long address = 0L;
	bool done = false;
	long* heap_data = heap_record;
	JAM_RETURN_TYPE status = JAMC_SUCCESS;
	/*
	*	We need two memory buffers:
	*
	*	in   (length of compressed bitstream)
	*	out  (length of uncompressed bitstream)
	*
	*	The "out" buffer is inside the heap record.  The "in" buffer
	*	resides in temporary storage above the last heap record.
	*/
	out_size = (dimension >> 3) +
		((dimension & 7) ? 1 : 0);
	*outDataSize = out_size;
	in = (char*)jam_get_temp_workspace(out_size + (out_size / 10) + 100);
	if (in == NULL)
	{
		status = JAMC_OUT_OF_MEMORY;
	}

	int ch_position = 0;
	while ((status == JAMC_SUCCESS) && (!done))
	{
		ch = *(station + ch_position);
		ch_position++;

		if (ch == JAMC_SEMICOLON_CHAR)
		{
			done = true;
		}
		else
		{
			value = jam_6bit_char(ch);

			if (value == -1)
			{
				status = JAMC_SYNTAX_ERROR;
			}
			else
			{
				for (bit = 0; bit < 6; ++bit)
				{
					if (value & (1 << (bit % 6)))
					{
						in[address >> 3] |= (1L << (address & 7));
					}
					else
					{
						in[address >> 3] &=
							~(unsigned int)(1 << (address & 7));
					}
					++address;
				}
			}
		}
	}

	if (done && (status == JAMC_SUCCESS))
	{
		/*
		*	Uncompress the data
		*/
		in_size = (address >> 3) + ((address & 7) ? 1 : 0);
		uncompressed_length = jam_uncompress(
			in, in_size, (char*)heap_data, out_size, jam_version);

		if (uncompressed_length != out_size)
		{
			status = JAMC_SYNTAX_ERROR;
		}
		else
		{
			/* convert data from bytes into 32-bit words */
			out_size = (dimension >> 5) +
				((dimension & 0x1f) ? 1 : 0);
			ch_data = (char*)heap_data;

			for (word = 0; word < out_size; ++word)
			{
				heap_data[word] =
					((((long)ch_data[(word * 4) + 3]) & 0xff) << 24L) |
					((((long)ch_data[(word * 4) + 2]) & 0xff) << 16L) |
					((((long)ch_data[(word * 4) + 1]) & 0xff) << 8L) |
					(((long)ch_data[word * 4]) & 0xff);
			}
		}
	}

	if (in != NULL) jam_free_temp_workspace(in);

	return (status);
}


ActelSTPPaser::ActelSTPPaser()
{
}



char* ActelSTPPaser::getVersion()
{
	return VERSION;
};
char* ActelSTPPaser::getFormatName()
{
	return C_ACTELSTP;
};

bool ActelSTPPaser::ConfirmFormat(QString& filename)
{
	bool ret = false;
	int Rtn = 0, LineNum = 0;
	QFile File(filename);
	QString strOneLine;
	int nItemColCnt = 0;
	int valueConv = 0;
	QString strConvTemp;
	int iValue = 0;
	int lineNum = 0;
	QString strMsg;
	bool hasCreateor = false;
	bool hasCapture = false;
	bool hasDevice = false;
	bool hasPackage = false;
	bool hasVersion = false;
	bool hasVendor = false;

	QString strExt = QFileInfo(filename).suffix();
	if (strExt.compare("stp", Qt::CaseInsensitive) != 0) {
		Rtn = -1;
		goto __end;
	}

	if (!File.open(QIODevice::ReadOnly)) {
		Rtn = -1;
		goto __end;
	}

	while (!File.atEnd()) {
		strOneLine = File.readLine();
		lineNum++;
		if (strOneLine.isEmpty()) {
			continue;
		}

		if (lineNum > 100) {
			Rtn = -1; goto __end;
		}

		strOneLine = strOneLine.replace("\r\n", ""); // 0x0d0a
		strOneLine = strOneLine.remove(QRegExp("^\\s+"));

		if (strOneLine.contains("NOTE \"CREATOR\" \"Designer Version")) {
			hasCreateor = true;
			continue;
		}
		else if (strOneLine.contains("NOTE \"CAPTURE\"")) {
			hasCapture = true;
			continue;
		}
		else if (strOneLine.contains("NOTE \"DEVICE\"")) {
			hasDevice = true;
			continue;
		}
		else if (strOneLine.contains("NOTE \"PACKAGE\"")) {
			hasPackage = true;
			continue;
		}
		else if (strOneLine.contains("NOTE \"STAPL_VERSION\" \"JESD71\"")) {
			hasVersion = true;
			continue;
		}
		else if (strOneLine.contains("NOTE \"VENDOR\"")) {
			hasVendor = true;
			break;
		}
	}
	if (hasCapture && hasCreateor && hasDevice && hasPackage && hasVersion && hasVendor) {
		ret = true;
	}
__end:
	if (File.isOpen())
		File.close();
	return ret;
};
int ActelSTPPaser::TransferFile(QString& srcfn, QIODevice* dst)
{
	m_Dst = dst;
	QString strMsg;
	int valueConv = 0;
	unsigned short sum = 0;
	QFile File(srcfn);
	InitControl();
	FileSize = File.size();
	QString strOneLine;
	QString strOneRow;

	bool bDoingParseAction = false;
	bool bDoingParseRlock = false;
	bool bDoingParseDataStream = false;
	bool bDoingParseBsrpattern = false;
	bool bDoingParseSampleMask = false;
	bool bDoingParseM1Buff = false;
	bool bDoingParseM7Buff = false;
	bool bDoingParseIDCodeBuff = false;
	bool bDoingParseIDMaskBuff = false;
	bool bDoingParseActUrowAlgo = false;
	bool bDoingParseActUrowDesign = false;
	bool bDoingParseActUrowProg = false;
	bool bDoingParseActUrowProgramSw = false;
	bool bDoingParseUrowMask = false;

	QString strLastParseBsrpattern = "";
	QString strLastParseSampleMask = "";
	QString strLastParseM1Buff = "";
	QString strLastParseM7Buff = "";
	QString strLastParseIDCodeBuff = "";
	QString strLastParseIDMaskBuff = "";
	QString strLastParseRlock = "";
	QString strLastParseActUrowAlgo = "";
	QString strLastParseActUrowDesign = "";
	QString strLastParseActUrowProg = "";
	QString strLastParseActUrowProgramSw = "";
	QString strLastParseUrowMask = "";

	QString strTempRowSrc;
	int nItemColCnt = 0;
	char* pDataStream = NULL;
	long* pDataOutStream = NULL;
	int nDataStreamLen = 0;
	char* fileBuffer = NULL;

	int nNoteWriteOffset = 0;
	int nActionWriteOffset = 0;
	int nRlockWriteOffset = 0;
	int nRsrPatternWriteOffset = 0;
	int nDataStreamWriteOffset = 0;
	int LineNum = 0;
	int nFileLength = 0;
	HeadData curHeadData;
	ParserInfoData curParserInfoData;

	QString strConvTemp;
	std::vector<unsigned char> vRegisterData;
	struct stat sbuf;

	/*if (m_pDataBuffer != NULL){
		std::vector<tDllBufInfoExt> vSimpleBufInfo;
		ret = ((CMultiDataBuffer *)m_pDataBuffer)->GetBufSimpleInfoExt(vSimpleBufInfo);
		if(ret!=0){
			goto __end;
		}

		int nHasBufferName = 1;
		for(int i=0;i<vSimpleBufInfo.size();++i){
			if(vSimpleBufInfo[i].strBufName=="Paser Info(4K)"){
				nHasBufferName++;
			}
			if(vSimpleBufInfo[i].strBufName=="STP Head(44K)"){
				nHasBufferName++;
			}
			if(vSimpleBufInfo[i].strBufName=="NOTE(16K)"){
				nHasBufferName++;
			}
		}
		if (nHasBufferName != 3){
			strMsg.Format("此解析器不匹配当时的buffer，请手动选择解析器");
			m_pOutput->Error(strMsg);
			ret = false;
			goto __end;
		}
	}*/

	if (!File.open(QIODevice::ReadOnly)) {
		strMsg = QString("Open File Failed: %1").arg(srcfn);
		m_pOutput->Error(strMsg.toLocal8Bit().data());
		ferror = C_Err_FileOpen; goto __end;
	}

	while (!File.atEnd()) {
		LineNum++;
		myfgets(&File);
		strOneLine = (char*)work;
		if (strOneLine.isEmpty()) {
			continue;
		}

		//去掉\r\n
		strOneRow = strOneLine;
		strOneLine = strOneLine.replace("\r\n", ""); // 0x0d0a
		strOneLine = strOneLine.remove(QRegExp("^\\s+"));

		if (!strOneRow.contains("NOTE ")) {
			if (strOneRow.contains(";")) {
				curHeadData.nNoteCnt++;
			}
			unsigned int writeAddr = NOTE_DATAWRITE_OFFSET + nNoteWriteOffset;
			unsigned int writeLen = strOneRow.length();
			BufferWrite(writeAddr, (unsigned char*)strOneRow.toLocal8Bit().data(), writeLen);
			nNoteWriteOffset += writeLen;
		}
		else if (strOneRow.contains("INTEGER CHECKSUM") && strOneRow.indexOf(";") > 0) {
			unsigned int leftposition = strOneRow.indexOf("=") + 1;
			unsigned int rightposition = strOneRow.indexOf(";");
			QString stationment = strOneRow.mid(leftposition, rightposition - leftposition);
			stationment = stationment.trimmed();
			int nCheckSum = stationment.toInt();
			curHeadData.nChecksumCnt = sizeof(nCheckSum);
			BufferWrite(CHECKSUM_DATAWRITE_OFFSET, (unsigned char*)&nCheckSum, sizeof(nCheckSum));
		}
		else if (strOneRow.indexOf("INTEGER BM7DEVICE") >= 0 && strOneRow.indexOf(";") > 0) {
			unsigned int leftposition = strOneRow.indexOf("=") + 1;
			unsigned int rightposition = strOneRow.indexOf(";");
			QString stationment = strOneRow.mid(leftposition, rightposition - leftposition);
			stationment = stationment.trimmed();
			int nValue = stationment.toInt();
			BufferWrite(MDEVICE_DATAWRITE_OFFSET, (unsigned char*)&nValue, 4);
			curHeadData.nMDeviceBufferLen += 4;
		}
		else if (strOneRow.indexOf("INTEGER BM1DEVICE") >= 0 && strOneRow.indexOf(";") > 0) {
			unsigned int leftposition = strOneRow.indexOf("=") + 1;
			unsigned int rightposition = strOneRow.indexOf(";");
			QString stationment = strOneRow.mid(leftposition, rightposition - leftposition);
			stationment = stationment.trimmed();
			int nValue = stationment.toInt();
			BufferWrite(MDEVICE_DATAWRITE_OFFSET + 4, (unsigned char*)&nValue, 4);
			curHeadData.nMDeviceBufferLen += 4;
		}
		else if (strOneRow.indexOf("ACTION ") == 0 || bDoingParseAction) {
			//遇到;结束语句，退出ParseAction
			if (strOneRow.indexOf(";") < 0) {
				bDoingParseAction = true;
			}
			else {
				curHeadData.nActionCnt++;
				bDoingParseAction = false;
			}
			int pos = 0;
			while (pos < strOneRow.length() && strOneRow.at(pos).isSpace()) {
				++pos;
			}
			strOneRow = strOneRow.mid(pos);

			unsigned int writeAddr = ACTION_DATAWRITE_OFFSET + nActionWriteOffset;
			unsigned int writeLen = strOneRow.length();
			BufferWrite(writeAddr, (unsigned char*)strOneRow.toStdString().c_str(), writeLen);
			
			nActionWriteOffset += writeLen;
		}
		else if (strOneRow.indexOf("BOOLEAN RLOCK") > 0 || bDoingParseRlock) {
			if (strOneRow.indexOf(";") < 0) {
				bDoingParseRlock = true;
			}
			else {
				bDoingParseRlock = false;
			}

			int nArrayLen = 0;
			int nWriteBufferLen = 0;
			if (ParseStationment("BOOLEAN RLOCK[", strLastParseRlock, strOneRow, RLOCK_DATAWRITE_OFFSET,
				bDoingParseRlock, &nArrayLen, &nWriteBufferLen) < 0) {
				ferror = C_Err_FormatError;
				goto __end;
			}
			curHeadData.nRlockUnCompressLen += nArrayLen;
			curHeadData.nRlockBufferLen += nWriteBufferLen;

		}
		else if (strOneRow.indexOf("BOOLEAN BSRPATTERN[") > 0 || bDoingParseBsrpattern) {
			if (strOneRow.indexOf(";") < 0) {
				bDoingParseBsrpattern = true;
			}
			else {
				bDoingParseBsrpattern = false;
			}


			int nArrayLen = 0;
			int nWriteBufferLen = 0;
			if (ParseStationment("BOOLEAN BSRPATTERN[", strLastParseBsrpattern, strOneRow, BSRPATTERN_DATAWRITE_OFFSET,
				bDoingParseBsrpattern, &nArrayLen, &nWriteBufferLen) < 0) {
				ferror = C_Err_FormatError;
				goto __end;
			}
			curHeadData.nBsrPatternUnCompressLen += nArrayLen;
			curHeadData.nBsrPatternBufferLen += nWriteBufferLen;

		}
		else if (strOneRow.indexOf("BOOLEAN SAMPLEMASK[") > 0 || bDoingParseSampleMask) {
			if (strOneRow.indexOf(";") < 0) {
				bDoingParseSampleMask = true;
			}
			else {
				bDoingParseSampleMask = false;
			}

			int nArrayLen = 0;
			int nWriteBufferLen = 0;
			if (ParseStationment("BOOLEAN SAMPLEMASK[", strLastParseSampleMask, strOneRow, SAMPLEMASK_DATAWRITE_OFFSET,
				bDoingParseSampleMask, &nArrayLen, &nWriteBufferLen) < 0) {
				ferror = C_Err_FormatError;
				goto __end;
			}
			curHeadData.nSampleMaskUnCompressLen += nArrayLen;
			curHeadData.nSampleMaskBufferLen += nWriteBufferLen;

		}
		else if (strOneRow.indexOf("BOOLEAN M1BUFF[") > 0 || bDoingParseM1Buff) {
			if (strOneRow.indexOf(";") < 0) {
				bDoingParseM1Buff = true;
			}
			else {
				bDoingParseM1Buff = false;
			}

			int nArrayLen = 0;
			int nWriteBufferLen = 0;
			if (ParseStationment("BOOLEAN M1BUFF[", strLastParseM1Buff, strOneRow, M1BUFF_DATAWRITE_OFFSET,
				bDoingParseM1Buff, &nArrayLen, &nWriteBufferLen) < 0) {
				ferror = C_Err_FormatError;
				goto __end;
			}
			curHeadData.nM1UnCompressLen += nArrayLen;
			curHeadData.nM1BufferLen += nWriteBufferLen;

		}
		else if (strOneRow.indexOf("BOOLEAN M7BUFF[") > 0 || bDoingParseM7Buff) {
			if (strOneRow.indexOf(";") < 0) {
				bDoingParseM7Buff = true;
			}
			else {
				bDoingParseM7Buff = false;
			}

			int nArrayLen = 0;
			int nWriteBufferLen = 0;
			if (ParseStationment("BOOLEAN M7BUFF[", strLastParseM7Buff, strOneRow, M7BUFF_DATAWRITE_OFFSET,
				bDoingParseM7Buff, &nArrayLen, &nWriteBufferLen) < 0) {
				ferror = C_Err_FormatError;
				goto __end;
			}
			curHeadData.nM7UnCompressLen += nArrayLen;
			curHeadData.nM7BufferLen += nWriteBufferLen;

		}
		else if (strOneRow.indexOf("BOOLEAN IDCODEVALUE[") > 0 || bDoingParseIDCodeBuff) {
			if (strOneRow.indexOf(";") < 0) {
				bDoingParseIDCodeBuff = true;
			}
			else {
				bDoingParseIDCodeBuff = false;
			}

			int nArrayLen = 0;
			int nWriteBufferLen = 0;
			if (ParseStationment("BOOLEAN IDCODEVALUE[", strLastParseIDCodeBuff, strOneRow, IDCODEVALUE_DATAWRITE_OFFSET,
				bDoingParseIDCodeBuff, &nArrayLen, &nWriteBufferLen) < 0) {
				ferror = C_Err_FormatError;
				goto __end;
			}
			curHeadData.nIDCodeUnCompressLen += nArrayLen;
			curHeadData.nIDCodeBufferLen += nWriteBufferLen;

		}
		else if (strOneRow.indexOf("BOOLEAN IDMASK[") > 0 || bDoingParseIDMaskBuff) {
			if (strOneRow.indexOf(";") < 0) {
				bDoingParseIDMaskBuff = true;
			}
			else {
				bDoingParseIDMaskBuff = false;
			}

			int nArrayLen = 0;
			int nWriteBufferLen = 0;
			if (ParseStationment("BOOLEAN IDMASK[", strLastParseIDMaskBuff, strOneRow, IDMASK_DATAWRITE_OFFSET,
				bDoingParseIDMaskBuff, &nArrayLen, &nWriteBufferLen) < 0) {
				ferror = C_Err_FormatError;
				goto __end;
			}
			curHeadData.nIDMaskUnCompressLen += nArrayLen;
			curHeadData.nIDMaskBufferLen += nWriteBufferLen;

		}
		else if (strOneRow.indexOf("BOOLEAN ACT_UROW_ALGO_VERSION[") > 0 || bDoingParseActUrowAlgo) {
			if (strOneRow.indexOf(";") < 0) {
				bDoingParseActUrowAlgo = true;
			}
			else {
				bDoingParseActUrowAlgo = false;
			}

			int nArrayLen = 0;
			int nWriteBufferLen = 0;
			if (ParseStationment("BOOLEAN ACT_UROW_ALGO_VERSION[", strLastParseActUrowAlgo, strOneRow, ACTUROWALGO_DATAWRITE_OFFSET,
				bDoingParseActUrowAlgo, &nArrayLen, &nWriteBufferLen) < 0) {
				ferror = C_Err_FormatError;
				goto __end;
			}
			curHeadData.nActUrowAlgoUnCompressLen += nArrayLen;
			curHeadData.nActUrowAlgoBufferLen += nWriteBufferLen;

		}
		else if (strOneRow.indexOf("BOOLEAN ACT_UROW_DESIGN_NAME[") > 0 || bDoingParseActUrowDesign) {
			if (strOneRow.indexOf(";") < 0) {
				bDoingParseActUrowDesign = true;
			}
			else {
				bDoingParseActUrowDesign = false;
			}

			int nArrayLen = 0;
			int nWriteBufferLen = 0;
			if (ParseStationment("BOOLEAN ACT_UROW_DESIGN_NAME[", strLastParseActUrowDesign, strOneRow, ACTUROWDESIGN_DATAWRITE_OFFSET,
				bDoingParseActUrowDesign, &nArrayLen, &nWriteBufferLen) < 0) {
				ferror = C_Err_FormatError;
				goto __end;
			}
			curHeadData.nActUrowDesignUnCompressLen += nArrayLen;
			curHeadData.nActUrowDesignBufferLen += nWriteBufferLen;

		}
		else if (strOneRow.indexOf("BOOLEAN ACT_UROW_PROG_METHOD[") > 0 || bDoingParseActUrowProg) {
			if (strOneRow.indexOf(";") < 0) {
				bDoingParseActUrowProg = true;
			}
			else {
				bDoingParseActUrowProg = false;
			}

			int nArrayLen = 0;
			int nWriteBufferLen = 0;
			if (ParseStationment("BOOLEAN ACT_UROW_PROG_METHOD[", strLastParseActUrowProg, strOneRow, ACTUROWPROG_DATAWRITE_OFFSET,
				bDoingParseActUrowProg, &nArrayLen, &nWriteBufferLen) < 0) {
				ferror = C_Err_FormatError;
				goto __end;
			}
			curHeadData.nActUrowProgUnCompressLen += nArrayLen;
			curHeadData.nActUrowProgBufferLen += nWriteBufferLen;

		}
		else if (strOneRow.indexOf("BOOLEAN ACT_UROW_PROGRAM_SW[") > 0 || bDoingParseActUrowProgramSw) {
			if (strOneRow.indexOf(";") < 0) {
				bDoingParseActUrowProgramSw = true;
			}
			else {
				bDoingParseActUrowProgramSw = false;
			}

			int nArrayLen = 0;
			int nWriteBufferLen = 0;
			if (ParseStationment("BOOLEAN ACT_UROW_PROGRAM_SW[", strLastParseActUrowProgramSw, strOneRow, ACTUROWPROGSW_DATAWRITE_OFFSET,
				bDoingParseActUrowProgramSw, &nArrayLen, &nWriteBufferLen) < 0) {
				ferror = C_Err_FormatError;
				goto __end;
			}
			curHeadData.nActUrowProgramSwUnCompressLen += nArrayLen;
			curHeadData.nActUrowProgramSwBufferLen += nWriteBufferLen;

		}
		else if (strOneRow.indexOf("BOOLEAN UROW_MASK[") > 0 || bDoingParseUrowMask) {
			if (strOneRow.indexOf(";") < 0) {
				bDoingParseUrowMask = true;
			}
			else {
				bDoingParseUrowMask = false;
			}

			int nArrayLen = 0;
			int nWriteBufferLen = 0;
			if (ParseStationment("BOOLEAN UROW_MASK[", strLastParseUrowMask, strOneRow, ACTUROWMASK_DATAWRITE_OFFSET,
				bDoingParseUrowMask, &nArrayLen, &nWriteBufferLen) < 0) {
				ferror = C_Err_FormatError;
				goto __end;
			}
			curHeadData.nUrowMaskUnCompressLen += nArrayLen;
			curHeadData.nUrowMaskBufferLen += nWriteBufferLen;

		}
		else if (strOneRow.indexOf("BOOLEAN DATASTREAM[") >= 0 || bDoingParseDataStream) {
			if (strOneRow.indexOf(";") < 0) {
				bDoingParseDataStream = true;
			}
			else {
				bDoingParseDataStream = false;
			}
			unsigned int position = 0;
			if (strOneRow.indexOf("BOOLEAN DATASTREAM[") >= 0) {
				unsigned int leftposition = strOneRow.indexOf("[") + 1;
				unsigned int rightposition = strOneRow.indexOf("]");
				nDataStreamLen = strOneRow.mid(leftposition, rightposition - leftposition).toInt();
				if (nDataStreamLen > 0) {
					pDataStream = new char[nDataStreamLen];
					pDataOutStream = new long[nDataStreamLen];
					curHeadData.nDataStreamCompressLen = nDataStreamLen;
				}
				position = strOneRow.indexOf("@") + 1;
			}
			else {
				position = 0;
			}

			QString newString = strOneRow.mid(position);
			newString = newString.replace(" ", "");
			int pos = 0;
			while (pos < newString.length() && newString.at(pos).isSpace()) {
				++pos;
			}
			newString = newString.mid(pos);
			unsigned int writeLen = newString.length();

			memcpy(pDataStream + nDataStreamWriteOffset, (unsigned char*)newString.toStdString().c_str(), writeLen);

			nDataStreamWriteOffset += (writeLen);
#if 0
			char* statement_buffer = "O00008Cn63PbPMRWpGBDgj6RV60;";//abcdefabcdefghijkldefabc
			int len = 189;
			long outbufferSize = 0;
			long outbuffer[1024];
			ret = jam_read_bool_compressed(statement_buffer, len, outbuffer, &outbufferSize);

#else
			if (!bDoingParseDataStream) {
				long outbufferSize = 0;
				int ret = jam_read_bool_compressed(pDataStream, nDataStreamLen, pDataOutStream, &outbufferSize);
				if (ret == 0) {
					curHeadData.nDataStreamUnCompressLen = outbufferSize;
					char buf[100] = {0};
					sprintf(buf, "read_bool_compressed: %ld", outbufferSize);
					strMsg = buf;
					m_pOutput->Log(strMsg.toLocal8Bit().data());
					unsigned int writeAddr = DATASTREAM_DATAWRITE_OFFSET;
					BufferWrite(writeAddr, (unsigned char*)pDataOutStream, outbufferSize * sizeof(long));
				}
			}
#endif


		}
		UpdateProgress();
	}

	File.close();
	/* get length of file */

	nFileLength = File.size();
	if (File.open(QIODevice::ReadWrite))
	{
		strMsg = QString("Error: can't open file \"%1\"\n").arg(srcfn);
		ferror = C_Err_FileOpen;
		goto __end;
	}
	curHeadData.nStpFileBufferLen = nFileLength;
	fileBuffer = (char*)new char[((size_t)nFileLength)];
	if (fileBuffer == NULL)
	{
		strMsg = QString("Error: can't allocate memory (%1 Kbytes)").arg((int)(nFileLength / 1024L));
		ferror = C_Err_MemAllocErr;
		goto __end;
	}
	if (File.read(fileBuffer, nFileLength) != (size_t)nFileLength) {
		strMsg = "Error reading file";
		File.close();
		ferror = C_Err_ReadError;
		goto __end;
	}
	BufferWrite(STPFILE_DATAWRITE_OFFSET, (unsigned char*)fileBuffer, nFileLength);

	

	BufferWrite(HEAD_INFO_DATAWRITE_OFFSET, (unsigned char*)&curHeadData, sizeof(HeadData));

	
	BufferWrite(PARSER_INFO_DATAWRITE_OFFSET, (unsigned char*)&curParserInfoData, sizeof(ParserInfoData));

	

__end:
	if (File.isOpen())
		File.close();

	if (ferror != 0) {
		strMsg = QString("Line[%1] Parser Failed").arg(LineNum);
		m_pOutput->Error(strMsg.toLocal8Bit().data());
	}
	if (pDataStream != NULL) {
		delete pDataStream;
		pDataStream = NULL;
	}
	if (pDataOutStream != NULL) {
		delete pDataOutStream;
		pDataOutStream = NULL;
	}
	if (fileBuffer != NULL) {
		delete fileBuffer;
		fileBuffer = NULL;
	}
	return ferror;
};


inline QString RevertCString(QString in) {
	int nLen = in.length();
	QString out = "";
	if (nLen % 2 != 0) {
		in = "0" + in;
	}
	nLen = in.length();
	for (int i = nLen - 2; i >= 0; i = i - 2) {
		out += in.mid(i, 2);

	}
	return out;
}


int ActelSTPPaser::ParseStationment(QString strFormat, QString& lastStationment, QString strOneRow, ADR writeAddr, bool bDoingParse, int* nArrayLen, int* writeBufferLen)
{
	unsigned char pDataRecord[2048] = { 0 };
	int ret = -1;
	QVector<unsigned char> vRegisterData;
	QString strMsg = "";
	QString strStationment = "";
	unsigned int position = 0;
	if (strOneRow.indexOf(strFormat) > 0) {
		position = strOneRow.indexOf("$") + 1;
		unsigned int leftposition = strOneRow.indexOf("[") + 1;
		unsigned int rightposition = strOneRow.indexOf("]");
		int nLen = strOneRow.mid(leftposition, rightposition - leftposition).toInt();
		*nArrayLen = nLen;
	}
	else {
		position = 0;
	}


	QString newString = strOneRow.mid(position);
	int pos = 0;
    while (pos < newString.length() && newString.at(pos).isSpace()) {
        ++pos;
    }
	newString = newString.mid(pos);
	newString = newString.replace(";", "");
	lastStationment += newString;

	//遇到结束标志
	if (!bDoingParse) {
		lastStationment = RevertCString(lastStationment);
		vRegisterData.clear();
		if (ComTool::Str2Hex(lastStationment, ComTool::ENDIAN_BIG | ComTool::PREHEADZERO_NEED, vRegisterData) == false) {
			ret = -1; goto __end;
		}
		unsigned int writeLen = vRegisterData.size();
		std::copy(vRegisterData.begin(), vRegisterData.end(), pDataRecord);
		BufferWrite(writeAddr, pDataRecord, writeLen);
		*writeBufferLen = writeLen;
	}
	ret = 0;
__end:
	return ret;
}