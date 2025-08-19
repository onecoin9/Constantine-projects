// DlgSNStep.cpp : 实现文件
//

#include "stdafx.h"
#include "DlgSNStep.h"
#include <vector>


// CDlgSNStep 对话框

IMPLEMENT_DYNAMIC(CDlgSNStep, CDialog)

CDlgSNStep::CDlgSNStep(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSNStep::IDD, pParent),
	m_pSnCfgPara(pSnCfgPara)
{

}

CDlgSNStep::~CDlgSNStep()
{
	DestroyWindow();
}

void CDlgSNStep::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgSNStep, CDialog)
	ON_EN_KILLFOCUS(IDC_ADDR_EDIT, &CDlgSNStep::OnEnKillfocusItemEdit)
	ON_EN_KILLFOCUS(IDC_BYTE_EDIT, &CDlgSNStep::OnEnKillfocusItemEdit)
END_MESSAGE_MAP()


// CDlgSNStep 消息处理程序

BOOL CDlgSNStep::InitCtrls(CSerial& lSerial) {
	m_SnStepCfg.SerialInCfgData(lSerial);
	GetDlgItem(IDC_ADDR_EDIT)->SetWindowText(Integer2Str(m_SnStepCfg.StartAddress(), 16));
	GetDlgItem(IDC_BYTE_EDIT)->SetWindowText(Integer2Str(m_SnStepCfg.ByteSize()));
	return TRUE;
}



BOOL CDlgSNStep::GetCtrls(CSerial& lSerial) {
	/*CString str;
	GetDlgItem(IDC_ADDR_EDIT)->GetWindowText(str);
	m_SnStepCfg.SetStartAddress(atoi(str));
	GetDlgItem(IDC_BYTE_EDIT)->GetWindowText(str);
	m_SnStepCfg.SetByteSize(atoi(str));*/
	m_SnStepCfg.SerialOutCfgData(lSerial);
	return TRUE;
}



INT CDlgSNStep::QuerySN(DWORD Idx, BYTE* pData, INT* pSize) {
	UINT32 grp_cnt = m_SnStepCfg.ByteSize();
	CSerial lSerial;
	lSerial << grp_cnt;

	UINT64 start_addr = m_SnStepCfg.StartAddress() * 2;
	for (size_t i = 0; i < grp_cnt; ++i) {
		UINT64 addr = start_addr + i * 2;
		UINT sn_len = 1;
		lSerial << addr << sn_len;

		BYTE sn[1];
		sn[0] = Idx >> (i * 8);
		lSerial.SerialInBuff(sn, sn_len);
	}

	if(lSerial.GetLength() > *pSize){
		*pSize = lSerial.GetLength();
		return -2;
	}

	memcpy(pData, lSerial.GetBuffer(), lSerial.GetLength());
	return lSerial.GetLength();
}



BOOL CDlgSNStep::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}



void CDlgSNStep::OnEnKillfocusItemEdit()
{
	CString str;
	GetDlgItem(IDC_ADDR_EDIT)->GetWindowText(str);
	m_SnStepCfg.SetStartAddress(strtol(str, NULL, 16));
	GetDlgItem(IDC_BYTE_EDIT)->GetWindowText(str);
	m_SnStepCfg.SetByteSize(atoi(str));
}



CString CDlgSNStep::Integer2Str(UINT16 value, INT base) {
	CString str;
	str.Format((base == 16 ? "0x%08X" : "%d"), value);
	return str;
}
