// DlgScanInterface.cpp : 实现文件
//

#include "stdafx.h"
#include "SNScan.h"
#include "../../MultiAprog/ComStruct/DrvSNCfg.h"
#include "DlgScanInterface.h"


// CDlgScanInterface 对话框

IMPLEMENT_DYNAMIC(CDlgScanInterface, CDialog)

CDlgScanInterface::CDlgScanInterface(CSNScanCfg *pSNScanCfg,CWnd* pParent /*=NULL*/)
	: CDialog(CDlgScanInterface::IDD, pParent)
	, m_pSNScanCfg(pSNScanCfg)
	, m_strSiteSN(_T(""))
	, m_AdpIdx(_T(""))
	, m_strChipID(_T(""))
{
	m_vDlgSNGroup.clear();
}

CDlgScanInterface::~CDlgScanInterface()
{
	ReleaseSNGroupDlgs();
}

void CDlgScanInterface::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//DDX_Text(pDX, IDC_EDITSITENAME, m_strSiteSN);
	//DDX_Text(pDX, IDC_EDITADPIDX, m_AdpIdx);
	DDX_Text(pDX, IDC_EDITCHIPID, m_strChipID);
}


BEGIN_MESSAGE_MAP(CDlgScanInterface, CDialog)
	ON_BN_CLICKED(IDOK, &CDlgScanInterface::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgScanInterface 消息处理程序

BOOL CDlgScanInterface::CreatSNGroupDlgs()
{
	BOOL Ret=TRUE;
	if(m_pSNScanCfg){
		INT i;
		CRect RectPos,DlgRect,RectBtn;
		CRect GroupWndRect,RectFix;
		GetWindowRect(DlgRect);
		GetDlgItem(IDC_GROUPPOS)->GetWindowRect(RectPos);
		ScreenToClient(RectPos);
		for(i=0;i<m_pSNScanCfg->GetGroupNum();i++){
			tSNScanGroup *pSNScanGroup=m_pSNScanCfg->GetGroup(i);
			CDlgSNGroup *pDlgSNGroup=new CDlgSNGroup(m_pSNScanCfg,this);
			if(!pDlgSNGroup){
				AfxMessageBox("Memory Alloc CDlgSNGroup failed");
				Ret=FALSE; goto __end;
			}
			pDlgSNGroup->SetSNGroup(pSNScanGroup);
			pDlgSNGroup->Create(CDlgSNGroup::IDD,this);
			pDlgSNGroup->GetWindowRect(GroupWndRect);
			RectFix.left = RectPos.left;
			RectFix.top = RectPos.top+i*GroupWndRect.Height();
			RectFix.right=RectFix.left + GroupWndRect.Width();
			RectFix.bottom=RectFix.top+GroupWndRect.Height();
			pDlgSNGroup->MoveWindow(RectFix);
			pDlgSNGroup->ShowWindow(SW_SHOW);
			if(i==0){
				pDlgSNGroup->SNGetFocus();
			}
			m_vDlgSNGroup.push_back(pDlgSNGroup);
		}
		///调整OK按钮位置
		GetDlgItem(IDOK)->GetWindowRect(RectBtn);
		ScreenToClient(RectBtn);
		RectPos.top= RectFix.bottom+8;
		RectPos.bottom= RectPos.top+RectBtn.Height();
		RectPos.left=RectBtn.left;
		RectPos.right=RectBtn.right;
		GetDlgItem(IDOK)->MoveWindow(RectPos);
		
		///调整对话框大小
		DlgRect.bottom =DlgRect.top+RectPos.bottom+30;
		SetWindowPos(NULL,DlgRect.left,DlgRect.top,DlgRect.Width(),DlgRect.Height(),0);
	}

__end:
	return Ret;
}

BOOL CDlgScanInterface::ReleaseSNGroupDlgs()
{
	INT i;
	for(i=0;i<(INT)m_vDlgSNGroup.size();i++){
		if(m_vDlgSNGroup[i]){
			delete m_vDlgSNGroup[i];
		}
	}
	m_vDlgSNGroup.clear();
	return TRUE;
}

BOOL CDlgScanInterface::InitCtrls()
{
	BOOL Ret=TRUE;
	if(m_pSNScanCfg){
		DRVSNCFGPARA *pSNCfgPara=(DRVSNCFGPARA*)m_pSNScanCfg->m_pDrvSNCfgPara;
		if(pSNCfgPara){
			m_strSiteSN.Format("%s",pSNCfgPara->pSiteSN);
			m_AdpIdx.Format("%d",pSNCfgPara->AdapIdx+1);
			GetDlgItem(IDC_EDITSITENAME)->SetWindowText(m_strSiteSN);
			GetDlgItem(IDC_EDITADPIDX)->SetWindowText(m_AdpIdx);
		}
		else{
			AfxMessageBox("Parameter Error: SNCfgPara=NULL, Please check first");
			Ret = FALSE;
		}

		if(m_pSNScanCfg->m_bUIDEn&&m_pSNScanCfg->m_bRuleDllTest){
			GetDlgItem(IDC_EDITCHIPID)->EnableWindow(TRUE);
		}
		else{
			GetDlgItem(IDC_EDITCHIPID)->EnableWindow(FALSE);
		}
	}
	return Ret;	
}

BOOL CDlgScanInterface::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	InitCtrls();
	CreatSNGroupDlgs();
	return FALSE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CDlgScanInterface::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	INT i,GroupCnt,FetchCnt=0;
	INT bAllSNFetched=0,bChipIDFetched=0;
	BOOL LastGroupSet=FALSE;
	GroupCnt=(INT)m_vDlgSNGroup.size();
	//需要保证如果有Scanner使能设置时，所有组的SN都要被获得，否则不能使能
	if(m_pSNScanCfg->m_bScannerEn){
		for(i=0;i<GroupCnt;i++){
			CDlgSNGroup* pDlgSNGroup=m_vDlgSNGroup[i];
			if(pDlgSNGroup){
				if(pDlgSNGroup->IsFocusOn()){
					TRACE("Focus ON Group%d, Move To Next\n",i+1);
					if(i!=GroupCnt-1){
						m_vDlgSNGroup[i+1]->SNGetFocus();
						return;
					}
					else{
						TRACE("Last Group \n",i+1);
						LastGroupSet=TRUE;
					}
				}
				pDlgSNGroup->GetSNScanned();
			}
		}

		for(i=0;i<GroupCnt;i++){
			tSNScanGroup *pSNScanGroup=m_pSNScanCfg->GetGroup(i);
			if(pSNScanGroup){
				if(pSNScanGroup->SNScaned!=""){
					FetchCnt++;
				}
			}
		}
		if(FetchCnt==GroupCnt){///全部SN填写完成
			bAllSNFetched=TRUE;
		}
	}
	else{
		bAllSNFetched=TRUE;
	}
	//需要保证如果有ChipID设置时，ChipID一定要被赋值，否则不能退出
	if(m_pSNScanCfg->m_bUIDEn&&m_pSNScanCfg->m_bRuleDllTest){
		GetDlgItem(IDC_EDITCHIPID)->GetWindowText(m_pSNScanCfg->m_vSNGroup.ChipID);
		if(m_pSNScanCfg->m_vSNGroup.ChipID!=""){
			bChipIDFetched=TRUE;
		}
	}
	else{
		bChipIDFetched=TRUE;
	}
	
	if(bChipIDFetched && bAllSNFetched){
		OnOK();
	}
}
