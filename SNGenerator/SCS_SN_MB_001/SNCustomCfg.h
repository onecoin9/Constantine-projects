#pragma once

#include <vector>


//单独从INI配置文件中的
typedef struct tagSNCustomCfg
{
	CString strFactoryURL;
	CString strOSSURL;
	CString strSysURL;
	int nContentType;
	CString strHeaderKeyPara;

	UINT64 uSNAddr;
	UINT64 uSNLen;
	CString strAccount;
	CString strPassword;
	CString strMoLotNo;
	//CString strBatchNo;
	CString strToken;
	bool bIsCanConnect;
	
	void ReInit(){
		strFactoryURL.Empty();
		strOSSURL.Empty();
		strSysURL.Empty();
		uSNAddr = 0;
		uSNLen = 0;
		strAccount.Empty();
		strPassword.Empty();
		strMoLotNo.Empty();
		strToken.Empty();
		bIsCanConnect = false;
		nContentType = 0;
		strHeaderKeyPara.Empty();
	}

	tagSNCustomCfg(){
		ReInit();
	}
}tSNCustomCfg;

typedef struct tagRetBurnData{
	CString strKeyType;
	CString strHashCode;
	CString strKeyName;
	bool bIsFirstMac;
	CString strTagName;
	int nSnCodeNo;
	void RInit(){
		strKeyType.Empty();
		strHashCode.Empty();
		strKeyName.Empty();
		bIsFirstMac = false;
		strTagName.Empty();
		nSnCodeNo = 0;
	}
	tagRetBurnData(){
		RInit();
	}
}tRetBurnData;


typedef struct tagCacheMap{
	tRetBurnData itemInfo;
	std::vector<BYTE> vData;

}tCacheMap;

class CSNCustomCfg
{
public:
	CSNCustomCfg(void);
	~CSNCustomCfg(void);
	BOOL SerialInCfgData(CSerial& lSerial);
	BOOL SerialOutCfgData(CSerial& lSerial);
	tSNCustomCfg* GetCustomCfg(){return &m_Cfg;};
private:
	tSNCustomCfg m_Cfg;
};
