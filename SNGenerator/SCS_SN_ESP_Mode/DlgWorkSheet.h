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

// CDlgWorkSheet �Ի���

class CDlgWorkSheet : public CDialog
{
	DECLARE_DYNAMIC(CDlgWorkSheet)

public:
	CDlgWorkSheet(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgWorkSheet();

	bool GetFlag() const { return flag_; }
	WorkSheet GetWorkSheetParam() const { return wsheet_; }

// �Ի�������
	enum { IDD = IDD_WORKSHEET_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()

private:
	bool flag_;
	WorkSheet wsheet_;
};
