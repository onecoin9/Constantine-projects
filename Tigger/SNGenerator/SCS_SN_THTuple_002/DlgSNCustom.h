#pragma once

#include "../Com/Serial.h"
#include "../../MultiAprog/ComStruct/DrvSNCfg.h"
#include "../Com/ComTool.h"
#include "SNCustomCfg.h"
#include "LogFile.h"
#include "DllHelp.h"
// CDlgSNCustom �Ի���
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
	CDlgSNCustom(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgSNCustom();

	BOOL InitCtrls(CSerial& lSerial);
	BOOL GetCtrls(CSerial&lSerial);
	INT QuerySN(UINT AdapIdx,DWORD Idx,BYTE*pData,INT*pSize);
	INT TellResult(DWORD Idx,INT IsPass);
	INT PreLoad();
	CString GetCurrentPath( void );
	INT PostProgramRetToServer(CString strRet, DWORD Idx);
	BOOL InitSocket();
	BOOL ConnectAndSend(CString strSendData);
	void GetJsonFile(bool bRet, DWORD Idx);
	void SaveFailReportFile();
	INT CreatLog();

	// �Ի�������
	enum { IDD = IDD_DLGSNCUSTOM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	BOOL InitCtrlsValue(CSNCustomCfg& SNCfg);
	BOOL SaveCtrls();
	void initOnce();

public:
	// �̶����ֵ�SN�ֽ���
	CSNCustomCfg m_SnCfg;
	DRVSNCFGPARA *m_pSnCfgPara;

	tagSNCustomCfg m_tCfg;
	tagReport* m_tagReport;
	std::vector<tagReport> m_vReport;

public:
	CString m_strIniFilePath;
	SOCKET m_sock;
	CString m_strJson;

	CLogFile m_FileLog;
	CDllHelp m_DllHelp;
public:
	afx_msg void OnBnClickedBtnselini();
	afx_msg void OnEnKillfocusEdit();
};
