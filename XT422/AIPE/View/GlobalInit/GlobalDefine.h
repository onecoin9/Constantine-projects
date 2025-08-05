#pragma once
#include <string>
#include <QMetaType>

#define CHIPDATA_DATABASE "DB18U333.db"

#define OPERAT_ERASE	QObject::tr("Erase")
#define OPERAT_BLANK	QObject::tr("Blank")
#define OPERAT_PROGRAM	QObject::tr("Program")
#define OPERAT_VERIFY	QObject::tr("Verify")
#define OPERAT_SECURE	QObject::tr("Secure")
#define OPERAT_READ		QObject::tr("Read")

#define CFG_ENABLE_READ		0x00000001
#define CFG_ENABLE_PROGRAM	0x00000002
#define CFG_ENABLE_VERIFY	0x00000020
#define CFG_ENABLE_ERASE	0x00000040
#define CFG_ENABLE_BLANK	0x00000080
#define CFG_ENABLE_SECURE	0x00000100

#define SSD_WRITE_PARTITION_TABLE	(0x10000000)
#define SSD_WRITE_PARTITION_DATA	(0x14000000)
#define SSD_READ_MASTERCHIP_ANALYZE	(0x4200000000)

#define SAFEDEL(_ptr) do {if(_ptr)delete _ptr; _ptr=NULL;} while(0);

enum class SiteStatus
{
	Failed = -1,
	Normal = 0,
	Busy,
	Success
};

enum class AnimDirection
{
	Left = 0,
	Top,
	Right,
	Bottom
};

enum EthernetTableHeadName
{
	IPAddress = 0,
	HopIndex,
	SiteAlias,
	SiteSN,
	ChainID,
	Status,
	SiteType

};

enum ChipTableHeadName
{
	ChipManufact = 0,
	ChipName,
	BottomBoard,
	ChipAdapter,
	ChipPackage,
	ChipType,
	ChipStatus,
	AllChipInfo
};

enum FileTableHeadName
{
	FileName = 0,
	FileTag,
	FileSize,
	FilePath,
	FileFormat,
	FileAutoDetecteFormat,
	FileWordAddressEnable,
	FileRelcationEnable,
	FileBufferAddress,
	FileAddress,
	FileLoadLength,
	FileSwapType,
	FileChecksumType,
	FileChecksum
};

enum BufferCheckInsetionMode
{
	Disable = 0,
	Insetion_Check,
	Auto_Sensing,
};

enum BufferCheckType
{
	Byte = 0,
	Word,
	CRC16,
	CRC32
};

enum class ViewMode
{
	Light = 1,
	Dark
};

enum class WinActionType
{
	Log = 1,
	Project,
	Programmer,
	Site
};

enum class TranslateLanguage
{
	English = 1,
	Chinese,
	Janpanese
};

enum class OperationTagType
{
	None = -1,
	Erase = 1,
	Blank,
	Program,
	Verify,
	Secure,
	Read,
	Self,
	CheckSum,
};

enum class SerialConfigType
{
	Disable = 0,
	Incremental_Mode,
	From_File,
	SNC_Mode
};

enum class OperationComboBoxType
{
	Disabled = 0,
	Enabled  = 1,
	Blank_Check,
	Erase,
	EraseIfCheck,
	Hi_Verify,
	Low_Verify,
	Hi_Low_Verify
};

enum class progSettingMode
{
	Disabled = 0,
	Percent,
	Absolute
};

enum class optionSettingType
{
	Disabled = 0,
	MD5,
	SHA1
};

enum class ProgramOnPartition
{
	USER = 0,
	BOOT1 = 1,
	BOOT2 = 2,
	RPMB = 3,
	GPP1 = 4,
	GPP2 = 5,
	GPP3 = 6,
	GPP4 = 7,
};

enum class UserMode{
	Operator = 0x01,
	Engineer = 0x02,
	Developer = 0x03,
	Tester = 0x21, //只限模拟v9900使用
};
enum class AuthMode{
	DisableAuth = 0x00,
	LocalAuth = 0x01,
	ServerAuth = 0x02,
};


enum eMMCFileTableHeadName
{
	eFileIndex = 0,
	eFilePath,
	eFilePartitionName,
	eFileStartAddress,
	eFileSize,
	eFileSectorAlign,
	eFileCheckSum
};

enum EditValue
{
	DEC = 0,
	HEX = 1,
	STR = 2
};

enum EditEndian
{
	Little = 0,
	Big = 1
};

//档案基本信息
struct FileDataInfo
{
	std::string fileNameStr;	//文件名
	std::string fileTagStr;		//文件的Tag信息
	std::string fileSizeStr;	//文件大小
	std::string filePathStr;	//文件路径
	std::string fileFormatStr;	//文件类型
	std::string fileCheckStr;	//checksum值
	std::string fileAutoDetecteFormatStr;	//自动检测文件类型
	std::string fileWordAddressEnable;	//字地址是否启用
	std::string fileRelcationEnable;	//是否重定位
	std::string fileBufferAddressStr;	//缓冲地址
	std::string fileAddressStr;		//文件内部加载的开始偏移位置
	std::string fileLoadLengthStr;	//0表示加载全部文件
	int fileSwapType;
	std::string fileSwapTypeStr;	//Swap类型
	std::string fileChecksumType;	//文件Checksum类型
	std::string fileChecksumValue;	//文件Checksum值
	uint64_t fileBufAddrWriteMin;	//档案写入buffer的地址范围-起始地址
	uint64_t fileBufAddrWriteMax;	//档案写入buffer的地址范围-结束地址

	FileDataInfo() { clear(); }

	void clear()
	{
		fileNameStr = "";
		fileTagStr = "";
		fileSizeStr = "";
		filePathStr = "";
		fileFormatStr = "";
		fileCheckStr = "";
		fileAutoDetecteFormatStr = "";
		fileWordAddressEnable = "";
		fileRelcationEnable = "";
		fileBufferAddressStr = "";
		fileAddressStr = "";
		fileLoadLengthStr = "";
		fileSwapTypeStr = "";
		fileBufAddrWriteMin = 0;
		fileBufAddrWriteMax = 0;
		fileSwapType = 0;
	}
};

struct FileTagInfo
{
	std::string sTagName;
	std::string sFileType;
	std::string sReloadEn;
	std::string sReloadAddr;
};

struct eMMCFileInfo
{
	std::string sFilePath;
	std::string	sPartitionName;
	uint64_t nStartAddr;
	uint64_t nFileSize;
	bool bSectorAlign;
	uint64_t nCheckSum;

	eMMCFileInfo()
	{
		clear();
	}

	void clear()
	{
		sFilePath = "";
		sPartitionName = "Undefined";
		nStartAddr = 0;
		nFileSize = 0;
		bSectorAlign = false;
		nCheckSum = 0;
	}
};

//typedef struct tagChkInfo {
//	uint64_t chksum;	///program sum
//	uint64_t sumlen;	///used when calculate chksum	
//	void* PrivData;
//}CHKINFO;

//保存到table的档案本地记录一下，使用QtTable比较麻烦
struct eMMCFileRecord
{
	QString fileArea;	//eMMC烧录的区域
	int64_t StartAddr;	//烧录的起始地址
	int64_t fileSize;	//文件大小
};

struct SingleBPU
{
	int idx;
	int SktCnt;
	std::string AdapterID;
};

//BPU信息
struct BPUInfo
{
	std::string strBPUEn;
	std::vector<SingleBPU> vecInfos;
};

//固件版本头定义整体大小为256个字节
#define PartInfoHeaderSize (256)
typedef struct _tFirmwareHeader {
	uint8_t UID[16];
	uint8_t PartNum;
	uint8_t Version[3];
	uint32_t Date;
	uint8_t Reserved0[8];
	uint8_t OEMSign[64];
	uint8_t ProductSign[64];
	uint16_t PartInfoSize;
	uint8_t Reserved1[92];
	uint16_t CRC16Sum;
}tFirmwareHeader;

Q_DECLARE_METATYPE(FileDataInfo)
Q_DECLARE_METATYPE(eMMCFileInfo)