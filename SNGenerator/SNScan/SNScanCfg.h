#pragma once
#include <vector>
#include "../Com/Serial.h"

typedef struct tagSNScanGroup{
	UINT GroupIdx;
	UINT64 SNAddr;	///SN地址
	UINT SNSize;		///SN的大小
	CString SNName;	///SN的备注名称
	CString SNScaned;	///扫描得到的序列号
	tagSNScanGroup(){ReInit();}
	void ReInit(){
		SNAddr=0;
		SNSize=0;
		SNName="";
		GroupIdx=1;
	};
}tSNScanGroup;

////下面函数指针的说明请参看多机台SN规划文档
typedef INT (*FuncInitSNRule)(UINT *pHandle);
typedef INT (*FuncFreeSNRule)(UINT Handle);
typedef INT (*FuncSetSNCfg)(UINT Handle,char *strIniFile);
typedef INT (*FuncGetSNValue)(UINT Handle,INT ProgramIdx, BYTE*pSNName, BYTE*pSN, INT SNSize, BYTE*pSNValue,INT SNValueSize);
typedef INT (*FuncGetErrMsg)(UINT Handle,BYTE *pErrMsg, INT Size);
typedef INT (*FuncSetSNResult)(UINT Handle,INT ProgramIdx,INT ISPass);//////////////////////////////////////////////

typedef struct tagSNRuleOpt
{
	FuncInitSNRule pInitSNRule;
	FuncFreeSNRule pFreeSNRule;
	FuncSetSNCfg pSetSNCfg;
	FuncGetSNValue pGetSNValue;
	FuncGetErrMsg pGetErrMsg;
	FuncSetSNResult pSetSNResult;//////////////////////////////////////////
	UINT hRule;
}tSNRuleOpt;

typedef struct tagSNCache{
	///一个芯片只有一个ChipID，但是
	std::vector<tSNScanGroup> vSNCached;////缓存扫描得到的SN
	UINT AdpIdx;
	CString strSiteSN;
	CString ChipID;	///芯片的ID
	void ReInit(){
		vSNCached.clear();
		AdpIdx=0;
		strSiteSN="";
	};
	tagSNCache(){ReInit();}
}tSNCache;

class CSNScanCfg{

public:
	CSNScanCfg();
	~CSNScanCfg();
	void ReInit();
	BOOL SerialInCfgData(CSerial& lSerial);
	BOOL SerialOutCfgData(CSerial& lSerial);
	BOOL AppendGroup(INT nNum);
	BOOL RemoveGroup(INT nNum);
	INT GetGroupNum(){return m_bSNGroupNum;};
	CString GetIniFile(){return m_strIniFile;}
	tSNScanGroup* GetGroup(INT idx);
	INT SearchSNRule();
	BOOL OpenSNRuleDll( CString strSNRule );
	BOOL CloseSNRuleDll();

	BOOL InitSNRuleDll(CString& strErrmsg);
	
	INT SetUID(DWORD Idx,BYTE*pUID,INT Size);
	INT PushCache();
	INT PopCache();
	INT ClearCache();
	///查询真正的SN值
	INT QuerySN(DWORD Idx,BYTE*pData,INT*pSize);

	BOOL SNUUIDTellResult(DWORD Idx,INT IsPass);/////////////////////////////////////////UUID

	CString m_strIniFile;
	CString m_strSNRule; ///SNRule DLL名称
	INT m_bSNGroupNum;
	BOOL m_bScannerEn;
	BOOL m_bUIDEn;
	BOOL m_bRuleDllTest; ///是否进行Dll测试
	void * m_pDrvSNCfgPara;	///实际指向DRVSNCFGPARA结构体
	tSNCache m_vSNGroup;
	//std::vector<tSNScanGroup> m_vSNGroup;
	std::vector<CString>m_vSNRules;

	std::vector<tSNCache> m_SNCached;///缓存当前扫描得到的序列号

	tSNRuleOpt m_SNRuleOpt;
	HINSTANCE m_SNRullhLib; ///Rule的动态库句柄
};