// DlgSNScan.cpp : 实现文件
//

#include "stdafx.h"
#include "SNScan.h"
#include "DlgSNScan.h"
#include "DlgSNGroup.h"
#include "DlgScanInterface.h"
#include "DlgSHA256Setting.h"
#include "SnRuleSLDlg.h"

// CDlgSNScan 对话框
enum{
	TAG_SNSTARTADDR,
	TAG_SNSIZE,
	TAG_SNNAME,
	TAG_SNTOTAL
};

IMPLEMENT_DYNAMIC(CDlgSNScan, CDialog)

CDlgSNScan::CDlgSNScan(DRVSNCFGPARA *pSNCfgPara,CWnd* pParent/*=NULL*/)
	: CDialog(CDlgSNScan::IDD, pParent)
	, m_pSNCfgPara(pSNCfgPara)
	, m_strRuleIni(_T(""))
	, m_bScannerEn(FALSE)
	, m_bUIDEn(FALSE)
	, m_bRuleDllTest(FALSE)
{

}

CDlgSNScan::~CDlgSNScan()
{
}

void CDlgSNScan::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_GroupCnt);
	DDX_Text(pDX, IDC_EDITRULEINI, m_strRuleIni);
	DDX_Control(pDX, IDC_LIST1, m_ListCtrlST);
	DDX_Check(pDX, IDC_CHKSCANNEREN, m_bScannerEn);
	DDX_Check(pDX, IDC_UIDEN, m_bUIDEn);
	DDX_Check(pDX, IDC_CHKTESTRULEDLL, m_bRuleDllTest);
	DDX_Control(pDX, IDC_CMBSNRULE, m_cmbSNRules);
}

BEGIN_MESSAGE_MAP(CDlgSNScan, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDlgSNScan::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_BTNINISEL, &CDlgSNScan::OnBnClickedBtninisel)
	ON_CBN_SELCHANGE(IDC_CMBSNRULE, &CDlgSNScan::OnCbnSelchangeCmbsnrule)
	ON_BN_CLICKED(IDC_CHKSCANNEREN, &CDlgSNScan::OnBnClickedChkscanneren)
	ON_BN_CLICKED(IDC_UIDEN, &CDlgSNScan::OnBnClickedUiden)
	ON_BN_CLICKED(IDC_CHKTESTRULEDLL, &CDlgSNScan::OnBnClickedChktestruledll)
END_MESSAGE_MAP()

BOOL CDlgSNScan::InitCtrlsValue(CSNScanCfg& SNScanCfg)
{
	INT i,GroupCnt;
	std::vector<CString>ItemData;
	CString strTmp;
	m_strRuleIni=SNScanCfg.m_strIniFile;
	GroupCnt=SNScanCfg.GetGroupNum();
	m_GroupCnt.SetCurSel(GroupCnt-1);
	m_ListCtrlST.DeleteAllItems();
	for(i=0;i<GroupCnt;++i){
		tSNScanGroup* pSnGroup=SNScanCfg.GetGroup(i);
		ItemData.clear();
		strTmp.Format("%I64X",pSnGroup->SNAddr);
		ItemData.push_back(strTmp);
		strTmp.Format("%d",pSnGroup->SNSize);
		ItemData.push_back(strTmp);
		ItemData.push_back(pSnGroup->SNName);
		m_ListCtrlST.AppendItem(ItemData);
	}

	///扫描Rules
	m_SNScanCfg.SearchSNRule();
	m_cmbSNRules.ResetContent();
	m_cmbSNRules.AddString("Disable");
	for(i=0;i<(INT)m_SNScanCfg.m_vSNRules.size();i++){
		m_cmbSNRules.AddString(m_SNScanCfg.m_vSNRules[i]);
	}

	if(m_SNScanCfg.m_strSNRule==""){
		m_cmbSNRules.SetCurSel(0);
	}
	else{
		if(m_cmbSNRules.SelectString(-1,m_SNScanCfg.m_strSNRule)==CB_ERR){
			CString strMsg;
			strMsg.Format("Can't find %s.dll under snrule folder, Please create it first",m_SNScanCfg.m_strSNRule);
			m_cmbSNRules.SetCurSel(0);
			AfxMessageBox(strMsg,MB_OK|MB_ICONERROR);
		}
	}

	//m_SNScanCfg.m_strSNRule = "SNRuleUUIDV4";
	//m_cmbSNRules.GetWindowText(m_SNScanCfg.m_strSNRule);
	m_SNScanCfg.OpenSNRuleDll(m_SNScanCfg.m_strSNRule);

	m_bUIDEn=m_SNScanCfg.m_bUIDEn;
	m_bScannerEn=m_SNScanCfg.m_bScannerEn;
	m_strRuleIni=m_SNScanCfg.m_strIniFile;
	//UpdateData(FALSE);
	return TRUE;
}

// CDlgSNScan 消息处理程序
BOOL CDlgSNScan::InitCtrls( CSerial& lSerial )
{
	BOOL Ret=TRUE;
	if(lSerial.GetLength()!=0){
		Ret=m_SNScanCfg.SerialInCfgData(lSerial);
	}
	else{
	}
	m_SNScanCfg.m_pDrvSNCfgPara=(void*)m_pSNCfgPara;
	Ret=InitCtrlsValue(m_SNScanCfg);
	return Ret;
}

BOOL CDlgSNScan::GetCtrls( CSerial&lSerial )
{
	return m_SNScanCfg.SerialOutCfgData(lSerial);
}

extern CSNScanApp theApp;

INT CDlgSNScan::SetUID(DWORD Idx,BYTE*pUID,INT Size)
{
	INT Ret=0;
	if(m_bUIDEn==TRUE){///只有在扫描枪使能的时候才需要去Pop
		Ret=m_SNScanCfg.SetUID(Idx,pUID,Size);
	}
	return Ret;
}


INT CDlgSNScan::FetchSN( DWORD Idx,BYTE*pData,INT*pSize )
{
	INT Ret=0,RetDlg;
	if(m_bScannerEn==TRUE || (m_bRuleDllTest==TRUE&&m_bUIDEn==TRUE)){////只有使用扫描枪或者在测试模式下的UID使能才能弹出框
		HINSTANCE save_hInstance = AfxGetResourceHandle();
		AfxSetResourceHandle(theApp.m_hInstance);
		CDlgScanInterface DlgScanInterface(&m_SNScanCfg,CWnd::GetDesktopWindow());
		RetDlg=DlgScanInterface.DoModal();
		if(RetDlg==IDOK){
			m_SNScanCfg.PushCache();
		}
		else{
			Ret=-2;
		}
		AfxSetResourceHandle(save_hInstance);
		
	}
	return Ret;
}

INT CDlgSNScan::FetchSNForDB( DWORD Idx,BYTE*pData,INT*pSize )
{
	INT Ret=0,RetDlg;
	if(m_bScannerEn==TRUE || m_bUIDEn==TRUE){////只有使用扫描枪或者UID使能才能弹出框
		if(m_SNScanCfg.m_strSNRule.CompareNoCase("SNRuleUUIDV4") == 0){
			m_SNScanCfg.m_vSNGroup.ChipID = "C2F50812040";
			m_SNScanCfg.m_bUIDEn = TRUE;
			m_SNScanCfg.PushCache();
		}else {
			HINSTANCE save_hInstance = AfxGetResourceHandle();
			AfxSetResourceHandle(theApp.m_hInstance);
			CDlgScanInterface DlgScanInterface(&m_SNScanCfg,CWnd::GetDesktopWindow());			
			RetDlg=DlgScanInterface.DoModal();
			if(RetDlg==IDOK){
				m_SNScanCfg.PushCache();
			}
			else{
				Ret=-2;
			}
			AfxSetResourceHandle(save_hInstance);
		}
	}
	return Ret;
}

INT CDlgSNScan::QuerySN( DWORD Idx,BYTE*pData,INT*pSize )
{
	INT Ret=0;
	HINSTANCE save_hInstance = AfxGetResourceHandle();
	AfxSetResourceHandle(theApp.m_hInstance);
	if(m_SNScanCfg.PopCache()!=0){
		Ret=-1; goto __end;
	}
	Ret=m_SNScanCfg.QuerySN(Idx,pData,pSize);
__end:
	if(Ret==-1){///如果找不到，那么Cache需要全部清除，后面再次进行扫描
		m_SNScanCfg.ClearCache();
	}
	AfxSetResourceHandle(save_hInstance);
	return Ret;
}


LRESULT CDlgSNScan::SetText(INT nRow,INT nColumn,CString&strText)
{
	if(strText.IsEmpty() && nColumn!=TAG_SNNAME){
		CString strErrMsg;
		strErrMsg.Format("SN Error: %s can't be empty",vHeader[nColumn]);
		MessageBox(strErrMsg);
		return -1;
	}
	tSNScanGroup* pSnGroup=m_SNScanCfg.GetGroup(nRow);
	if(pSnGroup==NULL){
		return 0;
	}
	switch(nColumn){
		case TAG_SNSIZE:
			sscanf(strText,"%d",&pSnGroup->SNSize);
			break;
		case TAG_SNSTARTADDR:
			sscanf(strText,"%I64X",&pSnGroup->SNAddr);
			break;
		case TAG_SNNAME:
			pSnGroup->SNName=strText;
			break;
		default:
			break;
	}
	return 0;
}

CString CDlgSNScan::GetTipText(INT nRow,INT nColumn)
{
	switch(nColumn){
		case TAG_SNSIZE:
			return CString("1<=Size");
			break;
		case TAG_SNSTARTADDR:
		case TAG_SNNAME:
		default:
			return CString("");
			break;
	}
}

UINT CDlgSNScan::GetTextInfo(INT subCmd,INT nRow,INT nColumn)
{
	tSNScanGroup* pSnGroup=m_SNScanCfg.GetGroup(nRow);
	if(pSnGroup!=NULL){
		if(subCmd==CMD_GETTEXTLIMIT){
			if(nColumn==TAG_SNNAME){///名称最长输入64个字节
				return 64;
			}
		}
		else if(subCmd==CMD_GETTEXTTYPE){
			///其他情况交给SetInputTypeAsText负责
		}
	}
	return (UINT)-1;
}


LRESULT CALLBACK DlgSNSetText(void *Para,INT nRow,INT nColumn,CString&strText)
{
	CDlgSNScan*pDlgSN=(CDlgSNScan*)Para;
	if(pDlgSN){
		return pDlgSN->SetText(nRow,nColumn,strText);
	}
	return 0;
}

CString CALLBACK DlgSNGetTipText(void *Para,INT nRow,INT nColumn)
{
	CDlgSNScan*pDlgSN=(CDlgSNScan*)Para;
	if(pDlgSN){
		return pDlgSN->GetTipText(nRow,nColumn);
	}
	else{
		return CString("");
	}
}

UINT CALLBACK DlgSNGetTextInfo(void *Para,INT subCmd,INT nRow,INT nColumn)
{
	CDlgSNScan*pDlgSN=(CDlgSNScan*)Para;
	if(pDlgSN){
		return pDlgSN->GetTextInfo(subCmd,nRow,nColumn);
	}
	return (UINT)-1;
}



BOOL CDlgSNScan::InitCtrlList()
{
	DWORD Style=m_ListCtrlST.GetStyle();
	Style |=LVS_EX_GRIDLINES;
	m_ListCtrlST.SetExtendedStyle(Style);
	m_ListCtrlST.RegistSetTextCallBack(DlgSNSetText,this);///注册数据改变回调函数
	m_ListCtrlST.RegistGetTipTextCallBack(DlgSNGetTipText,this);///注册提示回调函数
	m_ListCtrlST.RegistGetTextInfoCallBack(DlgSNGetTextInfo,this);///注册文本编辑框输入允许最大字符
	///设置有方格线

	m_ListCtrlST.SetHeight(0);///根据字体自己调整

	vHeader.clear();
	vHeader.push_back("Start Address(h)");
	vHeader.push_back("SN Size");
	vHeader.push_back("SN Name");
	m_ListCtrlST.InitColumnHeader(vHeader);

	m_ListCtrlST.SetInputTypeAsText(TAG_SNSIZE,CListEditST::DATATYPE_NUM,4);
	m_ListCtrlST.SetInputTypeAsText(TAG_SNSTARTADDR,CListEditST::DATATYPE_HEX,8);
	m_ListCtrlST.SetInputTypeAsText(TAG_SNNAME,CListEditST::DATATYPE_STR,(UINT)-1);

	return TRUE;
}

BOOL CDlgSNScan::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_GroupCnt.AddString("1");
	m_GroupCnt.AddString("2");
	m_GroupCnt.AddString("3");
	m_GroupCnt.AddString("4");
	m_GroupCnt.SetCurSel(1);

	InitCtrlList();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CDlgSNScan::OnCbnSelchangeCombo1()
{
	// TODO: 在此添加控件通知处理程序代码
	INT CurGroup=m_GroupCnt.GetCurSel()+1;
	INT GroupCnt=m_SNScanCfg.GetGroupNum();
	if(CurGroup==GroupCnt)
		return;

	if(CurGroup>GroupCnt){
		m_SNScanCfg.AppendGroup(CurGroup-GroupCnt);
	}
	else{
		m_SNScanCfg.RemoveGroup(GroupCnt-CurGroup);
	}
	InitCtrlsValue(m_SNScanCfg);
}

void CDlgSNScan::OnBnClickedBtninisel()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strFilePath;
	CFileDialog Dlg(TRUE,NULL,NULL,OFN_PATHMUSTEXIST,"SNRule Ini File(*.ini)|*.ini||",this);
	if(Dlg.DoModal()==IDOK){
		strFilePath=Dlg.GetPathName();
	}
	m_strRuleIni=strFilePath;
	m_SNScanCfg.m_strIniFile=m_strRuleIni;
	if(m_strRuleIni!=""){
		CString strErrmsg;
		if(m_SNScanCfg.InitSNRuleDll(strErrmsg)==FALSE){
			AfxMessageBox(strErrmsg,MB_OK|MB_ICONERROR);
		}
	}
	UpdateData(FALSE);
}

void CDlgSNScan::OnCbnSelchangeCmbsnrule()
{
	m_cmbSNRules.GetWindowText(m_SNScanCfg.m_strSNRule);
	if(m_SNScanCfg.m_strSNRule.CompareNoCase("SNRuleUUIDV4") == 0)
	{		//将地址配置为默认 0X03FFF000 ，长度 ： 36位，名称为 ：UUIDV4
		m_ListCtrlST.DeleteAllItems();
		
		tSNScanGroup* pSnGroup=m_SNScanCfg.GetGroup(0);
		std::vector<CString>UUIDItem;
		CString strTmp;
		UINT AddrLength;
		CHAR TmpBuf[MAX_PATH];
		m_strRuleIni.Format("%s\\snrule\\SNRuleUUIDV4.ini",GetCurrentPath());
		m_SNScanCfg.m_strIniFile=m_strRuleIni;

		memset(TmpBuf,0,MAX_PATH);
		GetPrivateProfileString("Config", "UUIDAddr","",TmpBuf, MAX_PATH, m_strRuleIni);
		strTmp.Format("%s",TmpBuf);
		sscanf(strTmp,"%I64X",&pSnGroup->SNAddr);
		UUIDItem.push_back(strTmp);

		AddrLength = GetPrivateProfileInt("Config", "UUIDLeng", 0, m_strRuleIni);
		strTmp.Format("%d",AddrLength);
		sscanf(strTmp,"%d",&pSnGroup->SNSize);
		UUIDItem.push_back(strTmp);

		memset(TmpBuf,0,MAX_PATH);
		GetPrivateProfileString("Config", "UUIDName","",TmpBuf, MAX_PATH, m_strRuleIni);
		strTmp.Format("%s",TmpBuf);
		pSnGroup->SNName = strTmp;
		UUIDItem.push_back(strTmp);
		
		m_ListCtrlST.AppendItem(UUIDItem);

		m_bScannerEn = FALSE;
		m_SNScanCfg.m_bScannerEn=m_bScannerEn;

		m_bUIDEn = TRUE;
		m_SNScanCfg.m_bUIDEn=m_bUIDEn;
		if(m_bUIDEn)
		{
			m_pSNCfgPara->OptFlag |=OPTFLAG_UIDEN;
		}
		else
		{
			m_pSNCfgPara->OptFlag &=~OPTFLAG_UIDEN;
		}

		UpdateData(FALSE);
	}
	else if (m_SNScanCfg.m_strSNRule.CompareNoCase("SNRuleSHA256") == 0) {
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		CDlgSHA256Setting dlg(GetCurrentPath());
		dlg.DoModal();

		m_ListCtrlST.DeleteAllItems();
		CString addr = dlg.GetAddress();
		for (INT i = 0; i < m_SNScanCfg.GetGroupNum(); ++i) {
			tSNScanGroup* pSnGroup = m_SNScanCfg.GetGroup(i);
			sscanf(addr, "%I64X", &pSnGroup->SNAddr);
			pSnGroup->SNSize = 32;
			pSnGroup->SNName = "";

			std::vector<CString> item;
			item.push_back(addr);
			item.push_back("32");
			item.push_back("");
			m_ListCtrlST.AppendItem(item);
		}

		m_strRuleIni.Format("%s\\snrule\\SNRuleSHA256.ini", GetCurrentPath());
		m_SNScanCfg.m_strIniFile = m_strRuleIni;

		m_bScannerEn = FALSE;
		m_SNScanCfg.m_bScannerEn = m_bScannerEn;

		m_bUIDEn = TRUE;
		m_SNScanCfg.m_bUIDEn = m_bUIDEn;
		m_pSNCfgPara->OptFlag |= OPTFLAG_UIDEN;

		UpdateData(FALSE);
	}else if (m_SNScanCfg.m_strSNRule.CompareNoCase("SNRuleExe") == 0)
	{
		m_ListCtrlST.DeleteAllItems();
		CString strBinPath;
		strBinPath.Format("%s\\snrule\\generate.bin",GetCurrentPath());
		if ((PathFileExists(strBinPath))) {
			DeleteFile(strBinPath);
		}

		tSNScanGroup* pSnGroup=m_SNScanCfg.GetGroup(0);
		std::vector<CString>ExeItem;
		CString strTmp;
		UINT AddrLength;
		CHAR TmpBuf[MAX_PATH];
		m_strRuleIni.Format("%s\\snrule\\SNRuleExe.ini",GetCurrentPath());
		m_SNScanCfg.m_strIniFile=m_strRuleIni;

		memset(TmpBuf,0,MAX_PATH);
		GetPrivateProfileString("Config", "Addr","",TmpBuf, MAX_PATH, m_strRuleIni);
		strTmp.Format("%s",TmpBuf);
		sscanf(strTmp,"%I64X",&pSnGroup->SNAddr);
		ExeItem.push_back(strTmp);

		AddrLength = GetPrivateProfileInt("Config", "Len", 0, m_strRuleIni);
		strTmp.Format("%d",AddrLength);
		sscanf(strTmp,"%d",&pSnGroup->SNSize);
		ExeItem.push_back(strTmp);

		memset(TmpBuf,0,MAX_PATH);
		GetPrivateProfileString("Config", "Name","",TmpBuf, MAX_PATH, m_strRuleIni);
		strTmp.Format("%s",TmpBuf);
		pSnGroup->SNName = strTmp;
		ExeItem.push_back(strTmp);

		m_ListCtrlST.AppendItem(ExeItem);

		m_bScannerEn = FALSE;
		m_SNScanCfg.m_bScannerEn=m_bScannerEn;

		m_bUIDEn = TRUE;
		m_SNScanCfg.m_bUIDEn=m_bUIDEn;
		if(m_bUIDEn)
		{
			m_pSNCfgPara->OptFlag |=OPTFLAG_UIDEN;
		}
		else
		{
			m_pSNCfgPara->OptFlag &=~OPTFLAG_UIDEN;
		}

		UpdateData(FALSE);
	}		
	else if (m_SNScanCfg.m_strSNRule.CompareNoCase("SNRuleSL001") == 0) {
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		SnRuleSLDlg dlg(GetCurrentPath());
		dlg.DoModal();

		m_ListCtrlST.DeleteAllItems();
		for (INT i = 0; i < m_SNScanCfg.GetGroupNum(); ++i) {
			CString temp;
			std::vector<CString> item;
			tSNScanGroup* pSnGroup = m_SNScanCfg.GetGroup(i);
			if( i == 0){
				temp = dlg.write_addr;
				temp.Replace("0x","");
				sscanf(temp, "%I64X", &pSnGroup->SNAddr);
				pSnGroup->SNSize = dlg.addr_len;
				//pSnGroup->SNName = "SN_SL001";
				pSnGroup->SNName.Format("SN_SL%s",temp);
							
				item.push_back(dlg.write_addr);
				item.push_back("8");
				item.push_back(pSnGroup->SNName);
				m_ListCtrlST.AppendItem(item);
			}
			else if( i == 1){
				temp = dlg.write_addr2;
				temp.Replace("0x","");
				sscanf(temp, "%I64X", &pSnGroup->SNAddr);
				pSnGroup->SNSize = dlg.addr_len2;
				//pSnGroup->SNName = "SN_SL002";
				pSnGroup->SNName.Format("SN_SL%s",temp);

				item.push_back(dlg.write_addr2);
				item.push_back("128");
				item.push_back(pSnGroup->SNName);
				m_ListCtrlST.AppendItem(item);
			}

		}
		m_strRuleIni.Format("%s\\snrule\\SNRuleSL001.ini", GetCurrentPath());
		m_SNScanCfg.m_strIniFile = m_strRuleIni;
		m_bScannerEn = FALSE;
		m_SNScanCfg.m_bScannerEn=m_bScannerEn;
		m_bUIDEn = TRUE;
		m_SNScanCfg.m_bUIDEn=m_bUIDEn;
		m_pSNCfgPara->OptFlag |= OPTFLAG_UIDEN;

		UpdateData(FALSE);
	}

	m_SNScanCfg.CloseSNRuleDll();
	if(m_SNScanCfg.OpenSNRuleDll(m_SNScanCfg.m_strSNRule) == FALSE){
		m_cmbSNRules.SetCurSel(0);
	}
}

void CDlgSNScan::OnBnClickedChkscanneren()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_SNScanCfg.m_bScannerEn=m_bScannerEn;
}

void CDlgSNScan::OnBnClickedUiden()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_SNScanCfg.m_bUIDEn=m_bUIDEn;
	if(m_bUIDEn)
		m_pSNCfgPara->OptFlag |=OPTFLAG_UIDEN;
	else
		m_pSNCfgPara->OptFlag &=~OPTFLAG_UIDEN;
}

void CDlgSNScan::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialog::OnOK();
}

void CDlgSNScan::OnBnClickedChktestruledll()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_SNScanCfg.m_bRuleDllTest=m_bRuleDllTest;
}

///////////////////////////////////////////////
INT CDlgSNScan::SNUUIDResult(DWORD Idx,INT IsPass)
{
	INT Ret=0,RetDlg;
	///////////////////////////////////////////
	if(IsPass <= 0)
	{
		////打印说明烧录结果失败
		CString strErrMsg;
		strErrMsg.Format("SN Error: No Program Pass");
		//MessageBox(strErrMsg);
		Ret=-1;
	}

	m_SNScanCfg.SNUUIDTellResult(Idx,IsPass);
	
	return Ret;
}

CString CDlgSNScan::GetCurrentPath( void )
{
	TCHAR szFilePath[MAX_PATH + 1]; 
	TCHAR *pPos=NULL;
	CString str_url;
	GetModuleFileName(theApp.m_hInstance, szFilePath, MAX_PATH); 
	pPos=_tcsrchr(szFilePath, _T('\\'));
	if(pPos!=NULL){
		pPos[0] = 0;//删除文件名，只获得路径
		str_url=CString(szFilePath);
	}
	else{
		pPos=_tcsrchr(szFilePath, _T('/'));
		if(pPos==NULL){
			str_url="";
		}
		else{
			str_url=CString(szFilePath);
		}
	}	
	return str_url;
}