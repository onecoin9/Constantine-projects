// DlgLogin.cpp : 实现文件
//



#include "stdafx.h"
#include "Mes.h"
#include "DlgLogin.h"
#include "afxdialogex.h"
#include "cJSON.h"
#include "MD5.hpp"
#include "HttpClient.h"
#include "MesInterface.h"
//#include "SmtFtpServiceSoapBinding.nsmap"

// CDlgLogin 对话框

IMPLEMENT_DYNAMIC(CDlgLogin, CDialogEx)

CDlgLogin::CDlgLogin(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGLOGIN, pParent)
	, m_bShowPasswd(FALSE)
	, m_strBurnSoftware("APS")
{

}

CDlgLogin::~CDlgLogin()
{
}

void CDlgLogin::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHKSHOWPASSWD, m_bShowPasswd);
	DDX_CBString(pDX, IDC_CMB_BURN_SOFTWARE, m_strBurnSoftware);
	DDX_Text(pDX, IDC_EDITOPERATOR, m_strOperatorID);
	DDX_Text(pDX, IDC_EDITPASSWD, m_strAdminPasswd);
}


BEGIN_MESSAGE_MAP(CDlgLogin, CDialogEx)
	ON_BN_CLICKED(IDC_CHKSHOWPASSWD, &CDlgLogin::OnBnClickedChkshowpasswd)
	ON_BN_CLICKED(IDOK, &CDlgLogin::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgLogin 消息处理程序


BOOL CDlgLogin::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetBackgroundColor(RGB(147,220,255));
	m_pOperatorData->m_bAuthCancel = FALSE;
	m_strOperatorID = m_pOperatorData->m_strOperator;

	CString strSettingJsonPath;
	strSettingJsonPath.Format("%s\\Setting.json", GetCurrentPath());
	m_Setting.SetJsonPath(strSettingJsonPath);
	if (m_Setting.Load()) {
		m_strOperatorID  = m_Setting.strOperator;
	}
	
	// Initialize BurnSoftware ComboBox
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_CMB_BURN_SOFTWARE);
	if (pComboBox) {
		pComboBox->AddString("APS");
		pComboBox->AddString("MultiAprog");
		pComboBox->SetCurSel(0); // Default to APS
	}

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgLogin::OnBnClickedChkshowpasswd()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	CEdit *pEdit1;
	pEdit1 = (CEdit*)(GetDlgItem(IDC_EDITPASSWD));
	if (m_bShowPasswd == TRUE) {
		pEdit1->SetPasswordChar(NULL);
	}
	else {
		pEdit1->SetPasswordChar('*');
	}
	pEdit1->SetWindowText(m_strAdminPasswd);
}

std::string CDlgLogin::WideCharToMultiByte(const wchar_t* pwszMultiByte, UINT uCodePage /* = CP_ACP */)
{
	try
	{
		if (NULL == pwszMultiByte)
		{
			throw - 1;
		}

		int iMultiBytes = ::WideCharToMultiByte(uCodePage, 0, pwszMultiByte, -1, NULL, 0, NULL, FALSE);

		if (iMultiBytes == 0)
		{
			throw - 1;
		}

		char* pszMultiByte = new char[iMultiBytes + 1];

		pszMultiByte[iMultiBytes] = 0;
		if (!pszMultiByte)
		{
			throw - 1;
		}

		::WideCharToMultiByte(uCodePage, 0, pwszMultiByte, -1, pszMultiByte, iMultiBytes, NULL, FALSE);

		std::string strMultiChar = pszMultiByte;
		delete[] pszMultiByte;
		pszMultiByte = NULL;

		return strMultiChar.c_str();
	}
	catch (...)
	{
		return "";
	}
}


void CDlgLogin::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	
	// Save BurnSoftware selection to global variable
	// m_strBurnSoftware now contains the selected value ("APS" or "MultiAprog")

	CString strHeader = _T("");
	CString strBody = _T("");

	int nMesInterfaceRet = 0;
	CHttpClient Client;
	CString strURL = _T("");
	CString strResponse = _T("");
	CString strErrMsg = _T("");

	m_Setting.strOperator  = m_strOperatorID;
	m_Setting.Save();

	unsigned char decrypt[16];

	MD5_CTX md5;

	MD5Init(&md5);
	MD5Update(&md5, (unsigned char *)m_strAdminPasswd.GetBuffer(), (int)m_strAdminPasswd.GetLength());//只是个中间步骤
	m_strAdminPasswd.ReleaseBuffer();
	MD5Final(&md5, decrypt);//32位
	CString md5Password = _T("");
	for (INT i = 0; i<16; i++) {
		CString tmp = _T("");
		tmp.Format("%02x", decrypt[i]);
		md5Password += tmp;
	}

	BOOL Ret = FALSE;
	m_pOperatorData->m_bCheckAuthPass = FALSE;

	CMesInterface &MesInterface = CMesInterface::getInstance();
	nMesInterfaceRet = MesInterface.GetAuthFromServer(m_Setting.strWorkStationID, m_strOperatorID, md5Password, strResponse);
	if (nMesInterfaceRet != 0) {
		goto __end;
	}

	//1||Authority:eng      1||Authority:op		0||UID
	INT Result = atoi(strResponse.Mid(0, 1));
	if (Result == 0) {
		goto __end;
	}

	m_pOperatorData->m_bCheckAuthPass = TRUE;
	m_pOperatorData->m_strOperator = m_strOperatorID;
	Ret = TRUE;

__end:
	if (Ret == TRUE) {
		CDialogEx::OnOK();
	}
	else {
		CString errMsg = _T("");
		errMsg.Format("operator ID[%s]Login failed，return reuslt ：%s", m_strOperatorID, strResponse.Mid(3));
		AfxMessageBox(errMsg);

		//CDialogEx::OnOK();
	}
}


void CDlgLogin::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类
	m_pOperatorData->m_bAuthCancel = TRUE;
	CDialogEx::OnCancel();
}

INT CDlgLogin::GetTokenFromServer(CString& strToken)
{
	INT nRet = FALSE;
	int nHttpRet = 0;
	CHttpClient Client;
	CString strURL;
	CString strResponse;
	CString strErrMsg;

	cJSON* pRootParser;
	cJSON* pSuccess;
	cJSON* pResData;

	CString strHeader;
	CString strBody;
	CString strBuildJson;

	strURL.Format("%s/Authentication/Authentication/GetTokenRS", m_Setting.strWebServiceInterface);
	strHeader.Format("Content-Type: application/json;charset=UTF-8\r\n");//

	cJSON* RootBuild = cJSON_CreateObject();
	cJSON_AddStringToObject(RootBuild, "serviceproviderid", (LPSTR)(LPCSTR)m_Setting.strServiceproviderid);
	cJSON_AddStringToObject(RootBuild, "account", (LPSTR)(LPCSTR)m_Setting.strAccount);
	cJSON_AddStringToObject(RootBuild, "pwd", (LPSTR)(LPCSTR)m_Setting.strPwd);
	strBuildJson = cJSON_Print(RootBuild);
	strBody.Format("%s", strBuildJson);

	nHttpRet = Client.HttpPost(strURL, strHeader, strBody, strResponse);
	m_pILog->PrintLog(LOGLEVEL_ERR, "GetTokenFromServer strURL=%s\r\n", strURL);
	m_pILog->PrintLog(LOGLEVEL_ERR, "GetTokenFromServer HttpPost Ret=%d strResponse=%s\r\n", nHttpRet, strResponse);
	if (nHttpRet != 0) {
		if (nHttpRet == 1) {
		}
		else if (nHttpRet == 2) {
		}
		goto __end;
	}

	pRootParser = cJSON_Parse(strResponse.GetBuffer());
	if (pRootParser == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "Mes返回的不符合Json数据格式 \r\n");
		goto __end;
	}

	pSuccess = cJSON_GetObjectItem(pRootParser, "isSuccess");
	if (pSuccess == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "解析Mes返回的isSuccess字段错误，请确认Mes维护的字段信息, \r\n");
		goto __end;
	}

	if (cJSON_IsFalse(pSuccess)) {
		strErrMsg.Format("%s", cJSON_GetObjectItem(pRootParser, "errMsg")->valuestring);
		m_pILog->PrintLog(LOGLEVEL_ERR, "Mes返回获取Token失败，错误原因为:%s \r\n", strErrMsg);
		goto __end;
	}

	pResData = cJSON_GetObjectItem(pRootParser, "data");
	if (pResData == NULL) {
		m_pILog->PrintLog(LOGLEVEL_ERR, "解析Mes返回的data字段错误，请确认Mes维护的字段信息 \r\n");
		goto __end;
	}

	strToken.Format("%s", pResData->valuestring);
	//m_strToken.Format("%s", strToken);

	nRet = TRUE;

__end:

	if (RootBuild) {
		cJSON_Delete(RootBuild);
	}
	return nRet;
}