#pragma once
#include <vector>
#include "../Com/Serial.h"

typedef struct tagSNScanGroup{
	UINT GroupIdx;
	UINT64 SNAddr;	///SN��ַ
	UINT SNSize;		///SN�Ĵ�С
	CString SNName;	///SN�ı�ע����
	CString SNScaned;	///ɨ��õ������к�
	tagSNScanGroup(){ReInit();}
	void ReInit(){
		SNAddr=0;
		SNSize=0;
		SNName="";
		GroupIdx=1;
	};
}tSNScanGroup;

////���溯��ָ���˵����ο����̨SN�滮�ĵ�
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
	///һ��оƬֻ��һ��ChipID������
	std::vector<tSNScanGroup> vSNCached;////����ɨ��õ���SN
	UINT AdpIdx;
	CString strSiteSN;
	CString ChipID;	///оƬ��ID
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
	///��ѯ������SNֵ
	INT QuerySN(DWORD Idx,BYTE*pData,INT*pSize);

	BOOL SNUUIDTellResult(DWORD Idx,INT IsPass);/////////////////////////////////////////UUID

	CString m_strIniFile;
	CString m_strSNRule; ///SNRule DLL����
	INT m_bSNGroupNum;
	BOOL m_bScannerEn;
	BOOL m_bUIDEn;
	BOOL m_bRuleDllTest; ///�Ƿ����Dll����
	void * m_pDrvSNCfgPara;	///ʵ��ָ��DRVSNCFGPARA�ṹ��
	tSNCache m_vSNGroup;
	//std::vector<tSNScanGroup> m_vSNGroup;
	std::vector<CString>m_vSNRules;

	std::vector<tSNCache> m_SNCached;///���浱ǰɨ��õ������к�

	tSNRuleOpt m_SNRuleOpt;
	HINSTANCE m_SNRullhLib; ///Rule�Ķ�̬����
};