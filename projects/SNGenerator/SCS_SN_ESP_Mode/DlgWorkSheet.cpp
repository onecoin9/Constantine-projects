// CDlgWorkSheet.cpp : 实现文件
//

#include "stdafx.h"
#include "SCS_SN_ESP_Mode.h"
#include "DlgWorkSheet.h"

unsigned char wrap_key2[] = { 0xCF, 0x9B, 0x6B, 0x43, 0x68, 0xFD, 0xD5, 0x80 };

// CDlgWorkSheet 对话框

IMPLEMENT_DYNAMIC(CDlgWorkSheet, CDialog)

CDlgWorkSheet::CDlgWorkSheet(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgWorkSheet::IDD, pParent), flag_(false)
{
	
}

CDlgWorkSheet::~CDlgWorkSheet() {
}

BEGIN_MESSAGE_MAP(CDlgWorkSheet, CDialog)
END_MESSAGE_MAP()

void CDlgWorkSheet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}



BOOL CDlgWorkSheet::OnInitDialog() {
	return TRUE;
}



void CDlgWorkSheet::OnOK() {
	GetDlgItem(IDC_WO_EDIT)->GetWindowText(wsheet_.work_order);
	GetDlgItem(IDC_AN_EDIT)->GetWindowText(wsheet_.asset_num);
	GetDlgItem(IDC_EN_EDIT)->GetWindowText(wsheet_.emp_num);
	GetDlgItem(IDC_PN_EDIT)->GetWindowText(wsheet_.prog_name);
	GetDlgItem(IDC_PI_EDIT)->GetWindowText(wsheet_.prog_id);
	flag_ = true;
	CDialog::OnOK();
}



void CDlgWorkSheet::OnCancel() {
	wsheet_.work_order = "";
	wsheet_.asset_num = "";
	wsheet_.emp_num = "";
	wsheet_.prog_name = "";
	wsheet_.prog_id = "";
	flag_ = false;
	CDialog::OnCancel();
}
