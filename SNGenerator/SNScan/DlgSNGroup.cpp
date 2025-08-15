// DlgSNGroup.cpp : 实现文件
//

#include "stdafx.h"
#include "SNScan.h"
#include "DlgSNGroup.h"


// CDlgSNGroup 对话框

IMPLEMENT_DYNAMIC(CDlgSNGroup, CDialog)

CDlgSNGroup::CDlgSNGroup(CSNScanCfg *pSNScanCfg,CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSNGroup::IDD, pParent)
	,m_pSNScanCfg(pSNScanCfg)
	,m_pSNScanGroup(NULL)
	, m_SNIdx(0)
	, m_strSNAddr(_T(""))
	, m_SNSize(0)
	, m_SNName(_T(""))
{

}

CDlgSNGroup::~CDlgSNGroup()
{
}

void CDlgSNGroup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDITGROUPNAME, m_EditGroupSN);
	DDX_Text(pDX, IDC_GROUPIDX, m_SNIdx);
	DDX_Text(pDX, IDC_SNADDR, m_strSNAddr);
	DDX_Text(pDX, IDC_SNSIZE, m_SNSize);
	DDX_Text(pDX, IDC_SNNAME, m_SNName);
}


BEGIN_MESSAGE_MAP(CDlgSNGroup, CDialog)
END_MESSAGE_MAP()


// CDlgSNGroup 消息处理程序

BOOL CDlgSNGroup::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	if(m_pSNScanGroup){
		if(m_pSNScanGroup->SNName!=""){
			m_SNName=m_pSNScanGroup->SNName;
			m_EditGroupSN.SetName(m_pSNScanGroup->SNName);
		}
		else{
			CString strSNGroup;
			strSNGroup.Format("SN%d",m_pSNScanGroup->GroupIdx);
			m_SNName=strSNGroup;
			m_EditGroupSN.SetName(strSNGroup);
		}
		m_SNIdx=m_pSNScanGroup->GroupIdx;
		m_SNSize=m_pSNScanGroup->SNSize;
		m_strSNAddr.Format("0x%X",m_pSNScanGroup->SNAddr);
	
		GetDlgItem(IDC_EDITGROUPNAME)->EnableWindow(m_pSNScanCfg->m_bScannerEn);
		UpdateData(FALSE);
	}
	return FALSE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

////获取扫描到的二维码
INT CDlgSNGroup::GetSNScanned()
{
	INT Ret=0;
	GetDlgItem(IDC_EDITGROUPNAME)->GetWindowText(m_pSNScanGroup->SNScaned);
	return Ret;
}

void CDlgSNGroup::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类
	CWnd *pWnd=GetParent();
	::PostMessage(pWnd->m_hWnd,WM_COMMAND,MAKEWPARAM(IDOK, BN_CLICKED),0);
}

void CDlgSNGroup::SNGetFocus()
{
	m_EditGroupSN.SetFocus();
}

BOOL CDlgSNGroup::IsFocusOn()
{
	if(m_EditGroupSN.IsFocus()){
		return TRUE;
	}
	else{
		return FALSE;
	}
}
