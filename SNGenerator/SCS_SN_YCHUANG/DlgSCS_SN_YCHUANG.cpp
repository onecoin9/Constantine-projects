// DlgSCS_SN_YCHUANG.cpp : 实现文件
//

#include "stdafx.h"
#include "DlgSCS_SN_YCHUANG.h"

// CDlgSCS_SN_YCHUANG 对话框

IMPLEMENT_DYNAMIC(CDlgSCS_SN_YCHUANG, CDialog)

CDlgSCS_SN_YCHUANG::CDlgSCS_SN_YCHUANG(DRVSNCFGPARA *pSnCfgPara, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSCS_SN_YCHUANG::IDD, pParent)
	, m_pSnCfgPara(pSnCfgPara)
{
}

CDlgSCS_SN_YCHUANG::~CDlgSCS_SN_YCHUANG()
{
}

void CDlgSCS_SN_YCHUANG::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgSCS_SN_YCHUANG, CDialog)
END_MESSAGE_MAP()

// CDlgSCS_SN_YCHUANG 消息处理程序

BOOL CDlgSCS_SN_YCHUANG::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	InitCtrlsValue(m_SnYCHUANGCfg);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CDlgSCS_SN_YCHUANG::InitCtrlsValue()
{
	return InitCtrlsValue(m_SnYCHUANGCfg);
}

BOOL CDlgSCS_SN_YCHUANG::InitCtrlsValueWithoutWnd()
{
	return InitCtrlsValue(m_SnYCHUANGCfg);
}

BOOL CDlgSCS_SN_YCHUANG::GetCtrlsValue()
{
	// TODO: 在此添加控件值获取代码
	return TRUE;
}

INT CDlgSCS_SN_YCHUANG::QuerySN(DWORD Idx, BYTE* pData, INT* pSize)
{
	// TODO: 在此添加序列号查询代码
	return 0;
}

BOOL CDlgSCS_SN_YCHUANG::InitCtrlsValue(CSCS_SN_YCHUANGCfg& SNYCHUANGCfg)
{
	// TODO: 在此添加控件初始化代码
	return TRUE;
}

void CDlgSCS_SN_YCHUANG::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类
	GetCtrlsValue();
	CDialog::OnOK();
}
