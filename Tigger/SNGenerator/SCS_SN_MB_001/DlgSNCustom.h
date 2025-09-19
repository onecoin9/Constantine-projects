#pragma once

#include "../Com/Serial.h"
#include "../../MultiAprog/ComStruct/DrvSNCfg.h"
#include "../Com/ComTool.h"
#include "SNCustomCfg.h"
#include "LogFile.h"
#include <map>
#include "HttpClient.h"
#include "WorkThread.h"

#define  MSG_START_WORK (WM_USER+0x600)

// CDlgSNCustom 对话框
typedef struct tagReport{
	int index;
	CString strStartTime;
	CString strMac;
	CString strRet;
	INT64  nTime;
};

typedef struct tagUnitInfo{
	CString strDataType;
	int nGroupIdx;
	CString strKeyType;
	UINT64 SNAddr;
	UINT SNSize;
	void ReInit(){
		strDataType.Empty();
		nGroupIdx = 0;
		strKeyType.Empty();
		SNAddr = 0;
		SNSize = 0;
	}
	struct tagUnitInfo(){
		ReInit();
	}
}tUnitInfo;

typedef struct tagBurnALV{
	CString strChipType;
	 int nGroup;
	std::vector<tUnitInfo> vArrGroup;
	void ReInit(){
		strChipType.Empty();
		nGroup = 0;
		vArrGroup.clear();
	}
	struct tagBurnALV(){
		ReInit();
	}
}tBurnALV;

typedef struct tagResponsePara{
	CString strKeyType;
	CString strKeyName;
	CString strTagName;
	CString strHashCode;
	CString strIsFirstMac;
	CString strWareId;
	int nSnCodeNo;
	void ReInit(){
		strKeyType.Empty();
		strKeyName.Empty();
		strTagName.Empty();
		strHashCode.Empty();
		strIsFirstMac.Empty();
		strWareId.Empty();
		nSnCodeNo = 0;
	}
	struct tagResponsePara(){
		ReInit();
	}
}tResponsePara;

typedef struct {
	DWORD IdxReuse;
	int nSNCodeNo;
}tReUseMap;

class CDlgSNCustom : public CDialog
{
	DECLARE_DYNAMIC(CDlgSNCustom)

public:
	CDlgSNCustom(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgSNCustom();

	BOOL InitCtrls(CSerial& lSerial);
	BOOL GetCtrls(CSerial&lSerial);
	INT QuerySN(UINT AdapIdx,DWORD Idx,BYTE*pData,INT*pSize);
	INT TellResult(DWORD Idx,INT IsPass);
	INT PreLoad();
	CString GetCurrentPath( void );
	
	void GetJsonFile(bool bRet, DWORD Idx);
	void SaveFailReportFile();
	INT CreatLog();
	
	INT GetTokenFromServer(CString& strToken);
	INT CheckIsCanConnectToServer();
	INT GetBurnDataFromServer(int nIdx, CString& strKeyName);
	INT DownloadKeyFileFromServer(CString strKeyName, CString& strSN, std::vector<BYTE>& vHex);

	INT GetBurnDataFromServerByRepeat(int nSNCode);

	// 对话框数据
	enum { IDD = IDD_DLGSNCUSTOM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	//virtual BOOL OnInitDialog();
	BOOL InitCtrlsValue(CSNCustomCfg& SNCfg);
	BOOL SaveCtrls();
	void InitOnce();
	BOOL Parser_Json();
	
	bool CheckSize();
	bool CheckHash(char* pInputData, int nInSize, CString strDestHash);
public:
	// 固定部分的SN字节数
	CSNCustomCfg m_SnCfg;
	DRVSNCFGPARA *m_pSnCfgPara;
	int m_nRtryCnt;

	tagSNCustomCfg m_tCfg;
	tagReport* m_tagReport;
	std::vector<tagReport> m_vReport;

	tRetBurnData m_tCurrRetBurnData;
	
	tBurnALV m_tBurnALVFromFile;
	std::vector<tRetBurnData> m_vCurrRetBurnData;
	////////////////////////
	CRITICAL_SECTION m_CSReUse;
	CRITICAL_SECTION m_CSUpload;
	std::map<int, std::vector<tRetBurnData>> m_AllRetBurnData;
	 std::vector<DWORD> m_vRetOK;
	 std::vector</*DWORD*/tReUseMap> m_vReUse;
	CWorkThread m_wUpLoadRet;
	volatile BOOL m_bQuit;
	static INT WINAPI UpLoadRetProc(MSG msg, void *Para);
	int DoWork();
	///////////////////////
	std::map<CString, int> m_DataTypeMap;
	std::map<int, int> m_SnCodeNoMap;
	CHttpClient Client;
	CHttpClient m_UploadClient;
public:
	CString m_strIniFilePath;
	SOCKET m_sock;
	CString m_strJson;

	CLogFile m_FileLog;
	CString m_strJsonPath;
	volatile BOOL m_ApplyKey;
public:
	afx_msg void OnEnKillfocusEdit();
	afx_msg void OnBnClickedBtnSelJsonPath();
	
	afx_msg void OnBnClickedBtnApplyKey();
	BOOL GetDataFromShareMem(DWORD& nData);
	void WriteShareMem(DWORD nData, bool bIsInit);
	BOOL CreateBindTask(CString strMoLotNo);
	BOOL GetKeyByMo(CString strMoLotNo);
};
