#ifndef _ACMBN_H_
#define _ACMBN_H_
//Acroview MBN File for EMMC Partition Table
#include <iostream>  
#include <fstream>  
#include <vector>

#ifdef WIN32
#pragma pack(1)
#define PACK_ALIGN
#else
#define PACK_ALIGN __attribute__((packed))
#endif

#define ACMBN_VERSION (10)
typedef struct {
	uint8_t Magic[4];	//Magic Number
	int FileNum;	///The File Number Be Loaded
	uint16_t Version;	//Version 10 means 1.0
	uint8_t Reserved[6]; ///Reserved
}tAcMbnHeader;

typedef struct {
	uint32_t DataOffset;	///The Offset Of the FileInfo 
	uint32_t DataSize;	///The Size of the FileInfo
	uint32_t DataSum;	///The Bytesum of the FileInfo
	uint8_t Reserved[4]; ///Reserved
}tAcMbnEntry;

typedef struct {
	uint8_t FilePath[256];
	uint64_t FileSize;
	uint64_t FileChkSum;
	uint8_t FileMD5[16];
	uint8_t FileChecksumType;
	uint8_t PartitionIndex;
	uint8_t Reserved[2];
	uint32_t VFileNum;
	uint64_t FileLocationAddr;
}tAcMbnFileInfoHeader;

typedef struct {
	uint32_t VFileBlockStart;
	uint32_t VFileBlockNum;
	uint16_t CRC16;
	uint16_t Feature;
	uint8_t Reserved[4];
}tAcMbnVFile;

typedef struct {
	tAcMbnFileInfoHeader Header;
	std::vector<tAcMbnVFile> vVFiles;
}tAcMbnFileInfo;

#ifdef WIN32
#pragma pack()
#endif
#undef PACK_ALIGN

class CAcMbn :public std::fstream {
public:
	CAcMbn();
	virtual ~CAcMbn();
	///For Saving MBN To File
	int OpenFile(std::string strMbnFilePath, uint32_t nOpenFlags);
	int AddHeader(int FileNum);
	int AddFileInfo(tAcMbnFileInfo* pFileInfo);
	int CloseFile();
	int GetErrMsg(std::string& ErrMsg);

	///调用该函数解析给定的MBN档案，成功返回0，失败返回-1， vFileInfo会被填充，后续使用完成之后要调用
	///DeleteAcMbnFileInfo进行释放
	int ParserFile(std::string strMbnFilePath, std::vector<tAcMbnFileInfo*>& vFileInfo);
	int DeleteAcMbnFileInfo(std::vector<tAcMbnFileInfo*>& vFileInfo);
private:
	tAcMbnHeader m_Header;
	tAcMbnEntry* m_pEntries; ///Entries Array
	std::string m_ErrMsg;
	int m_FileIdxAdd;
};


#endif
