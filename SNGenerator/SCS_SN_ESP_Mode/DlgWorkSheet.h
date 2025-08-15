#pragma once

#include "stdint.h"
#include "afxwin.h"

struct WorkSheet{
	CString work_order;
	CString asset_num;
	CString emp_num;
	CString prog_name;
	CString prog_id;
};

// CDlgWorkSheet 对话框

class CDlgWorkSheet : public CDialog
{
	DECLARE_DYNAMIC(CDlgWorkSheet)

public:
	CDlgWorkSheet(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgWorkSheet();

	bool GetFlag() const { return flag_; }
	WorkSheet GetWorkSheetParam() const { return wsheet_; }

// 对话框数据
	enum { IDD = IDD_WORKSHEET_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()

private:
	bool flag_;
	WorkSheet wsheet_;
};
