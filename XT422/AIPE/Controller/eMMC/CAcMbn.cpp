
#include "CAcMbn.h"
#include <fstream>
static char Magic[4] = { 'A','C','B','N' };

CAcMbn::CAcMbn()
	:m_pEntries(NULL)
	, m_FileIdxAdd(0)
{

}

CAcMbn::~CAcMbn()
{
	if (m_pEntries) {
		delete[] m_pEntries;
	}
}

int CAcMbn::OpenFile(std::string strMbnFilePath, uint32_t nOpenFlags)
{
	int Ret = 0;
	bool Rtn = false;
	open(strMbnFilePath, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
	if (!is_open()) {
		m_ErrMsg = "Open File Failed:";
		m_ErrMsg += strMbnFilePath.c_str();
		Ret = -1; goto __end;
	}

__end:
	return Ret;
}

int CAcMbn::AddHeader(int FileNum)
{
	int Ret = 0;
	memset(&m_Header, 0, sizeof(tAcMbnHeader));
	memcpy(m_Header.Magic, Magic, 4);
	m_Header.FileNum = FileNum;
	m_Header.Version = ACMBN_VERSION;
	m_pEntries = new tAcMbnEntry[FileNum];
	if (m_pEntries == NULL) {
		m_ErrMsg = "Memory Alloc Entries Failed";
		Ret = -1; goto __end;
	}
	memset(m_pEntries, 0, sizeof(tAcMbnEntry) * FileNum);
	///Write Header and the Empty Entries First
	///The Entries Need to be modified later
	seekg(0, std::ios::beg);
	

	write((char*)&m_Header, sizeof(m_Header));
	write((char*)m_pEntries, sizeof(tAcMbnEntry) * FileNum);

__end:
	return Ret;
}

static void CalcByteSum(uint32_t& SumOrg, unsigned char* pData, int Size)
{
	int i;
	for (i = 0; i < Size; ++i) {
		SumOrg += pData[i];
	}
}

int CAcMbn::AddFileInfo(tAcMbnFileInfo* pFileInfo)
{
	int Ret = 0;
	int DataSize = 0;
	int i;
	tAcMbnEntry* pCurEntry = m_pEntries + m_FileIdxAdd;
	
	///When Write The File Information, It need to update the coordinate entry's information
	pCurEntry->DataOffset = (uint32_t)tellg();  ///Add File Info to the tail of the file
	pCurEntry->DataSum = 0;
	seekg(pCurEntry->DataOffset, std::ios::beg);
	write((char*)(&pFileInfo->Header), sizeof(tAcMbnFileInfoHeader));
	CalcByteSum(pCurEntry->DataSum, (unsigned char*)&pFileInfo->Header, sizeof(tAcMbnFileInfoHeader));
	DataSize += sizeof(tAcMbnFileInfoHeader);
	pFileInfo->Header.VFileNum = (int)pFileInfo->vVFiles.size();
	for (i = 0; i < (int)pFileInfo->vVFiles.size(); ++i) {
		tAcMbnVFile& MbnFile = pFileInfo->vVFiles[i];
		write((char*)&MbnFile, sizeof(tAcMbnVFile));
		CalcByteSum(pCurEntry->DataSum, (unsigned char*)&MbnFile, sizeof(tAcMbnVFile));
		DataSize += sizeof(tAcMbnVFile);
	}
	pCurEntry->DataSize = DataSize;

	m_FileIdxAdd++;
	return Ret;
}

int CAcMbn::CloseFile()
{
	int Ret = 0;
	////Update The Entries Information
	seekg(sizeof(m_Header), std::ios::beg);
	write((char*)m_pEntries, sizeof(tAcMbnEntry) * m_Header.FileNum);
	flush();
	close();
	return Ret;
}

int CAcMbn::GetErrMsg(std::string& ErrMsg)
{
	ErrMsg = m_ErrMsg;
	return 0;
}

int CAcMbn::ParserFile(std::string strMbnFilePath, std::vector<tAcMbnFileInfo*>& vFileInfo)
{
	int Ret = 0, i, j;

	uint32_t BytesRead;
	bool RtnCall;
	tAcMbnHeader MbnHeader;
	vFileInfo.clear();
	open(strMbnFilePath, std::ios::in | std::ios::out | std::ios::app);
	RtnCall = is_open();
	if (RtnCall == false) {
		m_ErrMsg = "Open File Failed:";
		m_ErrMsg += strMbnFilePath;
		Ret = -1; goto __end;
	}

	read((char*)&MbnHeader, sizeof(tAcMbnHeader));
	BytesRead = gcount();
	if (BytesRead != sizeof(tAcMbnHeader)) {
		m_ErrMsg = "Acmbn File Error: Read Header Failed";
		Ret = -1; goto __end;
	}

	if (memcmp(MbnHeader.Magic, Magic, 4) != 0) {
		m_ErrMsg = "Acmbn File Error: Read Header Failed";
		Ret = -1; goto __end;
	}

	for (i = 0; i < MbnHeader.FileNum; ++i) {
		uint32_t ByteSum = 0;
		tAcMbnEntry Entry;
		tAcMbnFileInfo* pMbnFileInfo = NULL;
		///切换到指定的Entry起始位置
		seekg(sizeof(tAcMbnHeader) + i * sizeof(tAcMbnEntry), std::ios::beg);
		read((char*)&Entry, sizeof(tAcMbnEntry));
		BytesRead = gcount();
		if (BytesRead != sizeof(tAcMbnEntry)) {
			char buf[200];
			memset(buf, 0, 200);
			sprintf(buf, "Acmbn File Error: Read Entry[%d] Failed", i);
			m_ErrMsg = buf;
			Ret = -1; goto __end;
		}
		///切换到FileInfo位置准备读取
		seekg(Entry.DataOffset, std::ios::beg);
		pMbnFileInfo = new tAcMbnFileInfo;
		if (pMbnFileInfo == NULL) {
			m_ErrMsg = "Acmbn File Error: Memory Alloc tAcMbnFileInfo Failed";
			Ret = -1; goto __end;
		}
		else {
			vFileInfo.push_back(pMbnFileInfo);
			///读取文件信息头部
			read((char*)&pMbnFileInfo->Header, sizeof(tAcMbnFileInfoHeader));
			BytesRead = gcount();
			CalcByteSum(ByteSum, (unsigned char*)&pMbnFileInfo->Header, sizeof(tAcMbnFileInfoHeader));
			if (BytesRead != sizeof(tAcMbnFileInfoHeader)) {
				char buf[200];
				memset(buf, 0, 200);
				sprintf(buf, "Acmbn File Error: Read FileInfo[%d] Failed", i);
				m_ErrMsg = buf;
				Ret = -1; goto __end;
			}
			for (j = 0; j < pMbnFileInfo->Header.VFileNum; ++j) {
				tAcMbnVFile VFile;
				///读取虚拟文件
				read((char*)&VFile, sizeof(tAcMbnVFile));
				BytesRead = gcount();
				CalcByteSum(ByteSum, (unsigned char*)&VFile, sizeof(tAcMbnVFile));
				if (BytesRead != sizeof(tAcMbnVFile)) {
					char buf[200];
					memset(buf, 0, 200);
					sprintf(buf, "Acmbn File Error: Read Vfile[%d] Failed", j);
					m_ErrMsg = buf;
					Ret = -1; goto __end;
				}
				else {
					pMbnFileInfo->vVFiles.push_back(VFile);
				}
			}

			///比对校验值，理论上FileInfo部分计算出来的校验值要和Entry中保存的一致
			if (Entry.DataSum != ByteSum) {
				char buf[200];
				memset(buf, 0, 200);
				sprintf(buf, "Acmbn File Error: FileInfo Checksum Compare Failed, ExpectSum:0x%X, RealSum:0x%X", Entry.DataSum, ByteSum);
				m_ErrMsg = buf;
				Ret = -1; goto __end;
			}
		}
	}

__end:
	if (Ret != 0) {
		///失败之后也要进行数据的释放
		DeleteAcMbnFileInfo(vFileInfo);
	}
	return Ret;
}

int CAcMbn::DeleteAcMbnFileInfo(std::vector<tAcMbnFileInfo*>& vFileInfo)
{
	int i;
	for (i = 0; i < vFileInfo.size(); ++i) {
		tAcMbnFileInfo* pFileInfo = vFileInfo[i];
		if (pFileInfo) {
			delete pFileInfo;
		}
	}
	vFileInfo.clear();
	return 0;
}



