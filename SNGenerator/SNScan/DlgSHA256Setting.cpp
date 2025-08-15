// DlgSNScan.cpp : 实现文件
//

#include "stdafx.h"
#include "SNScan.h"
#include "DlgSHA256Setting.h"

IMPLEMENT_DYNAMIC(CDlgSHA256Setting, CDialog)

CDlgSHA256Setting::CDlgSHA256Setting(const CString& cur_path, CWnd* pParent/*=NULL*/)
	: CDialog(CDlgSHA256Setting::IDD, pParent)
	, ini_path_(cur_path)
{
	ini_path_.Append("\\snrule\\SNRuleSHA256.ini");
}

CDlgSHA256Setting::~CDlgSHA256Setting()
{
}

void CDlgSHA256Setting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADDR_EDIT, addr_edit_);
	DDX_Control(pDX, IDC_TEXT_RADIO2, text_radio2_);
	DDX_Control(pDX, IDC_HEX_RADIO2, hex_radio2_);
	DDX_Control(pDX, IDC_DIS_RADIO, dis_radio_);
	DDX_Control(pDX, IDC_USE_RADIO, use_radio_);
	DDX_Control(pDX, IDC_TEXT_RADIO, text_radio_);
	DDX_Control(pDX, IDC_HEX_RADIO, hex_radio_);
	DDX_Control(pDX, IDC_KEY_EDIT, text_edit_);
}

BEGIN_MESSAGE_MAP(CDlgSHA256Setting, CDialog)
END_MESSAGE_MAP()



BOOL CDlgSHA256Setting::OnInitDialog() {
	CDialog::OnInitDialog();

	char tmp_buf[128];
	memset(tmp_buf, 0, 128);
	GetPrivateProfileString("Config", "Addr", "", tmp_buf, 128, ini_path_);
	addr_edit_.SetWindowText(tmp_buf);
	addr_.Format("%s", tmp_buf);

	GetPrivateProfileInt("Config", "UUIDType", 0, ini_path_) == 0 ? text_radio2_.SetCheck(TRUE) : hex_radio2_.SetCheck(TRUE);
	!!GetPrivateProfileInt("Config", "Algorithm", 1, ini_path_) ? use_radio_.SetCheck(TRUE) : dis_radio_.SetCheck(TRUE);
	GetPrivateProfileInt("Config", "AddKeyType", 0, ini_path_) == 0 ? text_radio_.SetCheck(TRUE) : hex_radio_.SetCheck(TRUE);

	memset(tmp_buf, 0, 128);
	GetPrivateProfileString("Config", "AddKeyContent", "", tmp_buf, 128, ini_path_);
	text_edit_.SetWindowText(tmp_buf);

	return FALSE;
}



void CDlgSHA256Setting::OnOK() {
	addr_edit_.GetWindowText(addr_);
	WritePrivateProfileString("Config", "Addr", addr_, ini_path_);

	CString str_temp;
	str_temp.Format("%d", hex_radio2_.GetCheck());
	WritePrivateProfileString("Config", "UUIDType", str_temp, ini_path_);

	str_temp.Format("%d", use_radio_.GetCheck());
	WritePrivateProfileString("Config", "Algorithm", str_temp, ini_path_);

	str_temp.Format("%d", hex_radio_.GetCheck());
	WritePrivateProfileString("Config", "AddKeyType", str_temp, ini_path_);

	text_edit_.GetWindowText(str_temp);
	WritePrivateProfileString("Config", "AddKeyContent", str_temp, ini_path_);

	CDialog::OnOK(); 
}



void CDlgSHA256Setting::OnCancel() {
	CDialog::OnCancel(); 
}
