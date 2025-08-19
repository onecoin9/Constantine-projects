#pragma once
#include <QObject>
#include <iostream>

#define CRC_LEN			(16)
#define CRCTYPE_CRC16	(1)

typedef struct tageMMCCRC EMMCCRC;
typedef int(*FnCalcCRC)(EMMCCRC* pCRC, uint8_t* Buf, int Size);

struct tageMMCCRC
{
	uint16_t CRCType;
	uint8_t CRCValue[CRC_LEN];
	FnCalcCRC CalcCRC;
};

typedef struct tagHugeBlkInfo {
	uint32_t dwBlkStart;				//起始block，对于字节或是sector模式都一样，以512字节为一个block。
	uint32_t dwBlkNum;					//多少个block
	uint32_t bIsVirgin;					//Hugeblock是否是virgin值，1为Virgin值，0不是
	uint8_t wVVirgin;					//virgin值
	EMMCCRC eMMCCrc;
}HUGEBLKINFO;

typedef struct tageMMCVFile {
	unsigned int dwBlkStart;				//起始block，对于字节或是sector模式都一样，以512字节为一个block。16进制
	unsigned int dwBlkNum;					//多少个block
	uint16_t Feature;					//虚拟文件的特征值
	uint16_t CRCType;					//为1表示CRC16
	uint8_t CRCValue[16];				//CRC值总共16个字节
	uint32_t PartIndex;			//对应档案要被放置的分区索引，0表示acxml档案所有分区，1表示USER，2表示BOOT1...
	std::string  PartName;		//对应档案要被放置的分区索引名称
	struct tageMMCVFile* pNext;
}eMMCVFILE;		///虚拟文件信息

typedef struct tagMMAKEFILE
{
	std::string strFileName;	//烧录文件
	int64_t	lastModifyTime;		//烧录文件最后修改时间，用于写入SSD前快速校验
	unsigned int  PartIndex;	//烧录在哪个区域
	std::string strPartName;	//烧录在哪个区域的文件名
	bool IsSectorAlign;			//是否需要Sector对齐操作
	int64_t StartAddr;			//烧录的起始地址
	int64_t Size;				//文件大小
	int64_t EndAddr;			///Sector对齐
	uint64_t CheckSum;		//存放CRC校验值，目前是CRC16
	unsigned int VFileCnt;		///总共有多少个虚拟文件
	unsigned int TotalBlk;		///总共多少个block需要处理
	eMMCVFILE* pVFileHead;		///虚拟文件头部
}MMAKEFILE, tImgMakeFile;

typedef void (*FuChksumCalc)(CHKINFO* pChkInfo, uint8_t* buf, uint32_t size);

typedef struct tagMakeCRC32Info {
	CHKINFO CRC32ChkInfo;
}tMakeCRC32Info;

typedef struct _teMMCVFile {
	std::string EntryName;	//数据段的虚拟名称
	uint32_t EntryIndex;	
	uint64_t FileBlkPos;	//数据在原始文件内的偏移位置
	uint64_t ChipBlkPos;	//数据在写入芯片内的偏移位置
	uint32_t BlockNum;      //数据的长度
	uint32_t vFileIdx;		//对应哪个bin文件
	uint16_t Feature;       //特征值，用于快速进行数据比对，一般取前面1K数据做CRC16
	uint16_t CRCType;		//1表示CRC16,2表示CRC32
	uint16_t CRCValue;		//实际CRC的值
	uint32_t PartIndex;			//对应档案要被放置的分区索引，0表示acxml档案所有分区，1表示USER，2表示BOOT1...
	std::string  PartName;		//对应档案要被放置的分区索引名称
}teMMCVFile;

typedef struct _tSeptBineMMCFileMap {
	std::string strFileName;	//对应档案的路径
	int64_t	lastModifyTime;		//烧录文件最后修改时间，用于写入SSD前快速校验
	uint64_t ChipBlkOrgStart;	//数据要被放置在芯片的起始位置
	uint64_t FileSize;			//文件大小
	uint32_t PartIndex;			//对应档案要被放置的分区索引，1表示USER，2表示BOOT1...
	std::string  PartName;		//对应档案要被放置的分区索引名称
	uint32_t CheckSum;			//存放CRC校验值，目前是CRC16
	uint32_t EntryCnt;			//Entry的数量
	uint32_t vFileIdx;			//对应哪个解析的VFile
	bool bSector;
	std::vector<teMMCVFile> vFiles;
}tSeptBineMMCFileMap;