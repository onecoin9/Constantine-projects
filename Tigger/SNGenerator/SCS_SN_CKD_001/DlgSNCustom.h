#pragma once

#include "../Com/Serial.h"
#include "../../MultiAprog/ComStruct/DrvSNCfg.h"
#include "../Com/ComTool.h"
#include "SNCustomCfg.h"
#include "LogFile.h"

// CDlgSNCustom 对话框
typedef struct tagReport{
	int index;
	CString strStartTime;
	CString strMac;
	CString strRet;
	INT64  nTime;
};

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
	INT GetBurnDataFromServer(CString& strKeyName);
	INT DownloadKeyFileFromServer(CString strKeyName, CString& strSN, std::vector<BYTE>& vHex);

	// 对话框数据
	enum { IDD = IDD_DLGSNCUSTOM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	//virtual BOOL OnInitDialog();
	BOOL InitCtrlsValue(CSNCustomCfg& SNCfg);
	BOOL SaveCtrls();
	void initOnce();

public:
	// 固定部分的SN字节数
	CSNCustomCfg m_SnCfg;
	DRVSNCFGPARA *m_pSnCfgPara;

	tagSNCustomCfg m_tCfg;
	tagReport* m_tagReport;
	std::vector<tagReport> m_vReport;

	tRetBurnData m_tCurrRetBurnData;

public:
	CString m_strIniFilePath;
	SOCKET m_sock;
	CString m_strJson;

	CLogFile m_FileLog;
public:
	afx_msg void OnEnKillfocusEdit();
};
