// DlgSNCustom.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SNCustom_FC001.h"
#include "DlgSNCustom.h"


// CDlgSNCustom �Ի���

IMPLEMENT_DYNAMIC(CDlgSNCustom, CDialog)

CDlgSNCustom::CDlgSNCustom(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSNCustom::IDD, pParent)
	, m_SNFixSize(0)
	, m_pSnCfgPara(pSnCfgPara)
	, m_SNStartAddr(0)
{

}

CDlgSNCustom::~CDlgSNCustom()
{
}

BOOL CDlgSNCustom::InitCtrlsValue(CSNCustomCfg& SNCfg)
{
	tSNCustomCfg* pCfg=SNCfg.GetCustomCfg();
	CString strText;
	m_strSNFix=pCfg->strSNFix;
	m_SNFixSize=pCfg->SNFixSize;
	m_SNStartAddr=pCfg->SNStartAddr;
	strText.Format("%I64X",m_SNStartAddr);
	GetDlgItem(IDC_EDIT3)->SetWindowText(strText);
	UpdateData(FALSE);
	return TRUE;
}

BOOL CDlgSNCustom::InitCtrls(CSerial& lSerial)
{
	BOOL Ret=TRUE;
	if(lSerial.GetLength()!=0){
		Ret=m_SnCfg.SerialInCfgData(lSerial);
	}
	else{
	}
	Ret=InitCtrlsValue(m_SnCfg);
	return Ret;
}

BOOL CDlgSNCustom::GetCtrls(CSerial&lSerial)
{
	UpdateData(TRUE);
	CString strText;
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
	GetDlgItem(IDC_EDIT3)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&m_SNStartAddr)!=1){
		MessageBox("��ȡSN��ʼ��ַ����,��ȷ��");
		return FALSE;
	}
	pCfg->strSNFix=m_strSNFix;
	pCfg->SNFixSize=m_SNFixSize;
	pCfg->SNStartAddr=m_SNStartAddr;
	if(m_SNFixSize!=(m_strSNFix.GetLength()+1)/2){
		MessageBox("�������,�̶����кŲ��ֵ��ֽ�����Ҫ����������ֽ���");
		return FALSE;
	}
	return m_SnCfg.SerialOutCfgData(lSerial);
}

INT CDlgSNCustom::QuerySN(UINT AdapIdx,DWORD Idx,BYTE*pData,INT*pSize)
{
	INT Ret=0;
	std::vector<BYTE> vFixData,vTimeData;
	CString Curtime;
	CTime Time;
	CSerial lSerial;
	UINT GroupCnt=1;
	BYTE *pSNData=NULL;
	INT SNSize=m_SNFixSize+7+1;
	INT SNTotalSize=SNSize+16;

	if(*pSize<SNTotalSize){
		*pSize=SNTotalSize;
		Ret=-2; goto __end;
	}

	pSNData=new BYTE[SNSize];
	if(!pSNData){
		Ret=-1; goto __end;
	}

	Time=CTime::GetCurrentTime();
	Curtime.Format("%04d%02d%02d%02d%02d%02d",Time.GetYear(),Time.GetMonth(),Time.GetDay(),
		Time.GetHour(),Time.GetMinute(),Time.GetSecond());
	ComTool::Str2HexNoTruncate(m_strSNFix,ComTool::ENDIAN_BIG,vFixData);
	if(vFixData.size()!=m_SNFixSize){
		Ret=-1; goto __end;
	}

	ComTool::Str2HexNoTruncate(Curtime,ComTool::ENDIAN_BIG,vTimeData);
	if(vTimeData.size()!=7){
		Ret=-1; goto __end;
	}

	copy(vFixData.begin(),vFixData.end(),pSNData);
	copy(vTimeData.begin(),vTimeData.end(),pSNData+m_SNFixSize);
	*(pSNData+SNSize-1)=(BYTE)AdapIdx+1;
	
	lSerial<<GroupCnt;
	lSerial<<m_SNStartAddr;
	lSerial<<SNSize;
	lSerial.SerialInBuff(pSNData,SNSize);

	
	memcpy(pData,lSerial.GetBuffer(),lSerial.GetLength());
	Ret=SNTotalSize;
__end:
	if(pSNData){
		delete[] pSNData;
	}
	return Ret;
}

INT CDlgSNCustom::TellResult(DWORD Idx,INT IsPass)
{
	INT Ret=0;
	return Ret;
}

INT CDlgSNCustom::PreLoad()
{
	INT Ret=0;
	return Ret;
}

void CDlgSNCustom::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_SNFixSize);
	DDX_Text(pDX, IDC_EDIT2, m_strSNFix);
}


BEGIN_MESSAGE_MAP(CDlgSNCustom, CDialog)
END_MESSAGE_MAP()


// CDlgSNCustom ��Ϣ�������

BOOL CDlgSNCustom::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
#if 0
	CString strText;
	strText.Format("%d",m_SNFixSize);
	GetDlgItem(IDC_EDIT1)->SetWindowText(strText);
	GetDlgItem(IDC_EDIT2)->SetWindowText(m_strSNFix);
#endif 
	GetDlgItem(IDC_EDIT2)->SetFocus();
	return FALSE;
	//return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}
