#pragma once

#include "SNScanCfg.h"
#include "DlgSNGroup.h"
// CDlgScanInterface 对话框

class CDlgScanInterface : public CDialog
{
	DECLARE_DYNAMIC(CDlgScanInterface)

public:
	CDlgScanInterface(CSNScanCfg *pSNScanCfg,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgScanInterface();

// 对话框数据
	enum { IDD = IDD_DLGSCANINTERFACE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	BOOL InitCtrls();
	DECLARE_MESSAGE_MAP()


	BOOL CreatSNGroupDlgs();
	BOOL ReleaseSNGroupDlgs();
private:
	CSNScanCfg *m_pSNScanCfg;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	CString m_strSiteSN;
	CString m_AdpIdx;
	std::vector<CDlgSNGroup*> m_vDlgSNGroup; 
	CString m_strChipID;
};
