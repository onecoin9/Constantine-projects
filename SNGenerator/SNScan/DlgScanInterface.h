#pragma once

#include "SNScanCfg.h"
#include "DlgSNGroup.h"
// CDlgScanInterface �Ի���

class CDlgScanInterface : public CDialog
{
	DECLARE_DYNAMIC(CDlgScanInterface)

public:
	CDlgScanInterface(CSNScanCfg *pSNScanCfg,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgScanInterface();

// �Ի�������
	enum { IDD = IDD_DLGSCANINTERFACE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
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
