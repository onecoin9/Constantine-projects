#pragma once


// CDlgSNScan �Ի���

#include "afxwin.h"



class CDlgSHA256Setting : public CDialog
{
	DECLARE_DYNAMIC(CDlgSHA256Setting)

public:
	CDlgSHA256Setting(const CString& cur_path, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgSHA256Setting();

	CString GetAddress() const {
		return addr_;
	}

// �Ի�������
	enum { IDD = IDD_SHA256_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()

private:
	CEdit addr_edit_;
	CButton text_radio2_;
	CButton hex_radio2_;
	CButton dis_radio_;
	CButton use_radio_;
	CButton text_radio_;
	CButton hex_radio_;
	CEdit text_edit_;

	CString ini_path_;
	CString addr_;
};
