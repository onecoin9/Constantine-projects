#pragma once

#include <vector>
#include "../Com/Serial.h"

#define SNMODE_ASCII	(0)
#define SNMODE_BIN		(1)
#define SNSTYLE_BCD		(0)
#define SNSTYLE_HEX		(1)
#define SNMSB_LSB		(0)
#define SNMSB_MSB		(1)

typedef struct tagSNGroup
{
	UINT64	llSNStartAddr;	///SN的起始地址
	UINT	dwSNLen;			///SN的长度
	UINT	dwSNMode;			///SN模式 0—ASCII自增，1—BIN自增
	UINT	dwSNStyle;		///SN类型 0—BCD码自增，1—HEX自增
	UINT	dwSNMSB;			///SN大小端模式 0—小端格式，1—大端格式
	CString strSNStartValue;	///起始值字符串
	UINT	dwStep;			///步进值
	struct tagSNGroup(){
		ReInit();
	};
	void ReInit(){
		llSNStartAddr=0;
		dwSNLen=1;
		dwSNMode=SNMODE_BIN;
		dwSNStyle=SNSTYLE_HEX;
		dwSNMSB=SNMSB_MSB;
		strSNStartValue="00";
		dwStep=1;
	};
}SNGROUP;

class CSN_EverCreation_001Cfg
{
public:
	CSN_EverCreation_001Cfg(){ReInit();};
	~CSN_EverCreation_001Cfg(){bSNGroupNum=0;m_vSNGroup.clear();};
	void ReInit();
	BOOL SerialInCfgData(CSerial& lSerial);
	BOOL SerialOutCfgData(CSerial& lSerial);
	BOOL AppendGroup(INT nNum);
	BOOL RemoveGroup(INT nNum);
	INT GetGroupNum(){return bSNGroupNum;};
	SNGROUP* GetGroup(INT idx);
	BOOL CheckDataValid(CString& strErrMsg);
private:
	BYTE bSNGroupNum;
	std::vector<SNGROUP>m_vSNGroup;
};




#ifdef WIN32
#pragma pack(1)
#define PACK_ALIGN
#else
#define PACK_ALIGN __attribute__((packed))
#endif

#ifdef WIN32
#pragma pack()
#endif
#undef PACK_ALIGN