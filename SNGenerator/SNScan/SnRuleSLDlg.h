#pragma once
#include "afxwin.h"

// SnRuleSLDlg �Ի���

class SnRuleSLDlg : public CDialog
{
	DECLARE_DYNAMIC(SnRuleSLDlg)

public:
	SnRuleSLDlg(const CString& ini_path_,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~SnRuleSLDlg();


// �Ի�������
	enum { IDD = IDD_SNRULE_SL_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()

public:
	CString ini_path;
	CString franchised_id;
	CString serv_url;
	CString write_addr;
	CString write_addr2;

	INT addr_len;
	INT addr_len2;

};
