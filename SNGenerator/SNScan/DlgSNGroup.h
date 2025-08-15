#pragma once

#include "SNScanCfg.h"
#include "myedit.h"
// CDlgSNGroup �Ի���

class CDlgSNGroup : public CDialog
{
	DECLARE_DYNAMIC(CDlgSNGroup)

public:
	CDlgSNGroup(CSNScanCfg *pSNScanCfg,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgSNGroup();

// �Ի�������
	enum { IDD = IDD_DLGSCANGROUP };
	void SetSNGroup(tSNScanGroup *pSNScanGroup){m_pSNScanGroup=pSNScanGroup;};
	void SNGetFocus();
	BOOL IsFocusOn();
	INT GetSNScanned();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

	CSNScanCfg *m_pSNScanCfg;
	tSNScanGroup *m_pSNScanGroup;
	
	CString m_strSNScanned;

	int SetFakedChipID(CString strChipID);
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
	CMyEdit m_EditGroupSN;
public:
	UINT m_SNIdx;
public:
	CString m_strSNAddr;
public:
	DWORD m_SNSize;
public:
	CString m_SNName;
};
