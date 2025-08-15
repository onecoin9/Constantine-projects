// SnRuleSLDlg.cpp : 实现文件

#include "stdafx.h"
#include "SNScan.h"
#include "SnRuleSLDlg.h"

#define BUFFER_SIZE 256

// SnRuleSLDlg 对话框

IMPLEMENT_DYNAMIC(SnRuleSLDlg, CDialog)

SnRuleSLDlg::SnRuleSLDlg(const CString& ini_path_ ,CWnd* pParent /*=NULL*/)
	: CDialog(SnRuleSLDlg::IDD, pParent),ini_path(ini_path_), franchised_id(_T("")), serv_url(_T("")), write_addr(_T("")), write_addr2(_T(""))
{
	ini_path.Append("\\snrule\\SNRuleSL001.ini");
}

SnRuleSLDlg::~SnRuleSLDlg()
{
}

void SnRuleSLDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FRANCHIS_ID, franchised_id);
	DDX_Text(pDX, IDC_EDIT_HTTP_ADDR, serv_url);
	DDX_Text(pDX, IDC_EDIT_WR_ADDR_ONE, write_addr);
	DDX_Text(pDX, IDC_EDIT_WR_ADDR_T, write_addr2);
}


BEGIN_MESSAGE_MAP(SnRuleSLDlg, CDialog)
END_MESSAGE_MAP()


// SnRuleSLDlg 消息处理程序

BOOL SnRuleSLDlg::OnInitDialog() {
	CDialog::OnInitDialog();

	char tmp_buf[BUFFER_SIZE];

	memset(tmp_buf, 0, BUFFER_SIZE);
	GetPrivateProfileString("Config", "franchiseId", "", tmp_buf, BUFFER_SIZE, ini_path);
	franchised_id.Format("%s",tmp_buf);

	memset(tmp_buf, 0, BUFFER_SIZE);
	GetPrivateProfileString("Config", "MESAddr", "", tmp_buf, BUFFER_SIZE, ini_path);
	serv_url.Format("%s",tmp_buf);

	memset(tmp_buf, 0, BUFFER_SIZE);
	GetPrivateProfileString("Config", "Addr1", "", tmp_buf, BUFFER_SIZE, ini_path);
	write_addr.Format("%s",tmp_buf);

	memset(tmp_buf, 0, BUFFER_SIZE);
	GetPrivateProfileString("Config", "Addr2", "", tmp_buf, BUFFER_SIZE, ini_path);
	write_addr2.Format("%s",tmp_buf);

	addr_len = GetPrivateProfileInt("Config", "Len1", 8, ini_path);
	addr_len2 = GetPrivateProfileInt("Config", "Len2", 128, ini_path);

	UpdateData(FALSE);
	return TRUE;
}



void SnRuleSLDlg::OnOK() {
	WritePrivateProfileString("Config", "franchiseId", franchised_id, ini_path);
	WritePrivateProfileString("Config", "MESAddr", serv_url, ini_path);
	WritePrivateProfileString("Config", "Addr1", write_addr, ini_path);
	WritePrivateProfileString("Config", "Addr2", write_addr2, ini_path);

	CDialog::OnOK();
}



void SnRuleSLDlg::OnCancel() {
	CDialog::OnCancel();
}
