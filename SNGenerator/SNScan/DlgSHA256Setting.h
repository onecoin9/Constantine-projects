#pragma once


// CDlgSNScan 对话框

#include "afxwin.h"



class CDlgSHA256Setting : public CDialog
{
	DECLARE_DYNAMIC(CDlgSHA256Setting)

public:
	CDlgSHA256Setting(const CString& cur_path, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgSHA256Setting();

	CString GetAddress() const {
		return addr_;
	}

// 对话框数据
	enum { IDD = IDD_SHA256_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
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
