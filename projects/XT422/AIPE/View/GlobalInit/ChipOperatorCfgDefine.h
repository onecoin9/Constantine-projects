#pragma once

#include <string>
#include <vector>

#define CFG_OP_READ                     0x00000001
#define CFG_OP_PROGRAM                  0x00000002
#define CFG_OP_BLOCK_PROGRAM            0x00000004
#define CFG_OP_ILLEGAL_BIT_CHECK        0x00000008
#define CFG_OP_FUNCTIONAL_TEST          0x00000010
#define CFG_OP_VERIFY                   0x00000020
#define CFG_OP_ERASE                    0x00000040
#define CFG_OP_BLACK_CHECK              0x00000080
#define CFG_OP_SECURE                   0x00000100
#define CFG_OP_INSECTION_CHECK          0x00000400
#define CFG_OP_ADDRESS_RELOCATE         0x00000800
#define CFG_OP_IDHECK					0x00001000
#define CFG_OP_EEPROM_AUTO_ID           0x00002000
#define CFG_OP_PIN_CONNTINUUM           0x00004000
#define CFG_OP_PROTECT                  0x00008000
#define CFG_OP_COMPARE                  0x00010000

#define CFG_OP_COPY_MASTER              0x00020000
#define CFG_OP_HLVCC_VERIFY             0x00040000
#define CFG_OP_LOOP_FUNCTION            0x00080000
#define CFG_OP_ISONLINE					0x00100000
#define CFG_OP_ISONTEST					0x00200000
#define CFG_OP_SUM                      0x00400000
#define CFG_OP_WORD_SUM                 0x00800000
#define CFG_OP_CRC16_SUM                0x01000000
#define CFG_OP_CRC32_SUM                0x02000000
#define CFG_OP_PIN_CHECK                0x04000000
#define CFG_OP_SN_ENABLE                0x08000000

#define CFG_OP_DATAWIDTH                0x30000000
#define CFG_OP_DATAW4BIT				0x00000000
#define CFG_OP_DATAW8BIT				0x10000000
#define CFG_OP_DATAW12BIT				0x20000000
#define CFG_OP_DATAW16BIT				0x30000000
#define CFG_BUFF_WORDADDR  				0x40000000
#define CFG_BUFF_BIGEND   				0x80000000

#define OP_ATTR_MENU					   0x00000001
#define OP_ATTR_TOOLBAR					   0x00000002
#define OP_ATTR_SUBOP				       0x00000004
#define OP_ATTR_NOTOOLBAR			       (OP_ATTR_MENU | OP_ATTR_SUBOP)
#define OP_ATTR_ALL					       (OP_ATTR_MENU | OP_ATTR_TOOLBAR | OP_ATTR_SUBOP)

enum ChipOperCfgSubCmdID
{
	UnEnable		= 0x000,	//结合UI界面使用，表示命令不启用
	CheckID			= 0x400,
	PinCheck		= 0x401,
	InsertionCheck	= 0x402,
	AutoSensing		= 0x403,

	DevicePowerOn	= 0x600,
	DevicePowerOff	= 0x601,
	PowerOn			= 0x602,
	PowerOff		= 0x603,

	SubProgram		= 0x800,
	SubErase		= 0x801,
	SubVerify		= 0x802,
	SubBlankCheck	= 0x803,
	SubSecure		= 0x804,
	SubIllegalCheck	= 0x805,
	SubRead			= 0x806,
	EraseIfBlankCheckFailed = 0x807,
	LowVerify		= 0x808,
	HighVerify		= 0x809,
	ChecksumCompare = 0x80A,
	SubReadChipUID	= 0x80B,
	High_Low_Verify = 0x80C
};

enum ChipOperCfgCmdID
{
	Program			= 0x1800,
	Erase			= 0x1801,
	Verify			= 0x1802,
	BlankCheck		= 0x1803,
	Secure			= 0x1804,
	IllegalCheck	= 0x1805,
	Read			= 0x1806,
	ReadChipUID		= 0x1807,
	Custom			= 0x1901,
};

typedef struct OperatorInfo {
	int					iOpId;
	int					iOpMask;
	int					iOpAttr;
	int					iCoinId;
	std::string			strOpName;
	std::vector<int>	vecOpList;
	uint32_t			uiPassCnt;
	uint32_t			uiFailCnt;
}OperatorInfo;

struct BaseOper
{
	bool bBlank;
	bool bBlockProg;
	bool bErase;
	bool bFunction;
	bool bIllegalBit;
	bool bProg;
	bool bRead;
	bool bSecure;
	bool bVerify;

	BaseOper()
	{
		clear();
	}

	void clear()
	{
		bBlank = false; bBlockProg = false; bErase = false; bFunction = false;
		bIllegalBit = false; bProg = false; bRead = false; bSecure = false;
		bVerify = false;
	}
};

struct BitsOper
{
	bool bBit12;
	bool bBit16;
	bool bBit4;
	bool bBit8;

	BitsOper()
	{
		clear();
	}

	void clear()
	{
		bBit12 = false; bBit16 = false; bBit4 = false; bBit8 = false;
	}
};

struct ChecksumOper
{
	bool bCRC16;
	bool bCRC32;
	bool bBytesum;
	bool bWordsum;

	ChecksumOper()
	{
		clear();
	}

	void clear()
	{
		bCRC16 = false; bCRC32 = false; bBytesum = false; bWordsum = false;
	}
};

struct FileLoadOper
{
	bool bBigEndian;
	bool bWordAddress;

	FileLoadOper()
	{
		clear();
	}

	void clear()
	{
		bBigEndian = false; bWordAddress = false;
	}
};

struct OtherOper
{
	bool bEEPROM;
	bool bIDCheck;
	bool bAddressRelocate;
	bool bCompare;
	bool bEmptyBuffer;
	bool bEnableSN;
	bool bInsection;
	bool bLoopFun;
	bool bMasterCopy;
	bool bOnline;
	bool bPin;
	bool bProtect;
	bool bunTest;
	bool bH_L_VccVerify;

	OtherOper()
	{
		clear();
	}

	void clear()
	{
		bEEPROM = false; bIDCheck = false; bAddressRelocate = false; bCompare = false;
		bEmptyBuffer = false; bEnableSN = false; bInsection = false; bLoopFun = false;
		bMasterCopy = false; bOnline = false; bPin = false; bProtect = false;
		bunTest = false; bH_L_VccVerify = false;
	}
};

struct StuOperatorJson
{
	BaseOper baseInfo;
	BitsOper bitsInfo;
	ChecksumOper checksumInfo;
	FileLoadOper fileLoadInfo;
	OtherOper otherInfo;

	void clear()
	{
		baseInfo.clear(); bitsInfo.clear(); checksumInfo.clear(); fileLoadInfo.clear(); otherInfo.clear();
	}
};

typedef std::vector<OperatorInfo> OpInfoList;