// DlgSNCustom.cpp : ÊµÏÖÎÄ¼þ
//

#include "stdafx.h"
#include "SCS_SN_CVTE_001.h"
#include "DlgSNCustom.h"

#include "../Com/ComTool.h"
#include "../Com/cJSON.h"
#include "Base64.h"
#include<cctype>
#include "../Com/ComFunc.h"
#include "sha1.h"
#include "aprog_common.h"
#include "../openssl/sha.h"

// CDlgSNCustom ¶Ô»°¿ò
//using namespace sha1;
#define  MAX_WAIT_TIME (60*20)
#define  Do_Test (0)

IMPLEMENT_DYNAMIC(CDlgSNCustom, CDialog)

CDlgSNCustom::CDlgSNCustom(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSNCustom::IDD, pParent)
	, m_pSnCfgPara(pSnCfgPara)
	,m_bQuit(FALSE)
	, m_strJsonPath(_T(""))
{
	/*CString strMsg;
	strMsg.Format(">>>>>>>>>>>>>�߳�=%d",  ::GetCurrentThreadId());
	AfxMessageBox(strMsg);*/

	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
	/*pCfg->strAccount.Empty();
	pCfg->strPassword.Empty();*/
	//AfxMessageBox("hhhhhhhhhh");
	m_nRtryCnt = 3;
	pCfg->strMoLotNo.Empty();
	pCfg->uSNAddr = 0;
	m_vRetOK.clear();
	m_wUpLoadRet.SetMsgHandler(CDlgSNCustom::UpLoadRetProc, this);
	m_wUpLoadRet.CreateThread();
	m_wUpLoadRet.PostMsg(MSG_START_WORK, 0, 0);

	InitializeCriticalSection(&m_CSUpload);
	InitializeCriticalSection(&m_CSReUse);
	m_vReUse.clear();
	m_ApplyKey = FALSE;

	CHAR TmpBuf[512];
	CString strIniFile;
	memset(TmpBuf,0,512);
	strIniFile.Format("%s\\sngen\\SCS_SN_CVTE_001.ini",GetCurrentPath());
	if (!PathFileExists(strIniFile)){
		//AfxMessageBox("�Ҳ���SCS_SN_CVTE_001.ini�����ļ�������!");
		//return ;
	}
	GetPrivateProfileString("Config", "Account", "",TmpBuf, MAX_PATH, strIniFile);
	m_tCfg.strAccount.Format("%s", TmpBuf);
	memset(TmpBuf,0,512);
	GetPrivateProfileString("Config", "PassWord", "",TmpBuf, MAX_PATH, strIniFile);
	m_tCfg.strPassword.Format("%s", TmpBuf);

	InitOnce();
}

CDlgSNCustom::~CDlgSNCustom()
{
	m_ApplyKey = FALSE;
	m_bQuit = TRUE;
	DeleteCriticalSection(&m_CSUpload);
	DeleteCriticalSection(&m_CSReUse);
	//AfxMessageBox("dddddddddddddeseeeee>>>>>>>>>>>>");
	//m_wUpLoadRet.DeleteThread();
}

INT WINAPI CDlgSNCustom::UpLoadRetProc(MSG msg, void *Para)
{
	INT Ret = 0;
	CDlgSNCustom *pDlg = (CDlgSNCustom*)Para;
	if (msg.message == MSG_START_WORK) {
		/*CString strMsg;
		strMsg.Format("xxxkaishi�߳�=%d",  ::GetCurrentThreadId());
		AfxMessageBox(strMsg);*/
		while (1) {
			Sleep(200);
			Ret = pDlg->DoWork();
			if (Ret != 0) {
				break;
			}
		}
	}
	/*CString strMsg;
	strMsg.Format("xxxxxx�����߳�=%d",  ::GetCurrentThreadId());
	AfxMessageBox(strMsg);*/

	return Ret;
}

int CDlgSNCustom::DoWork()
{
	int nRet =0;
	CString strRet;
	int nHttpRet = 0;
	//CHttpClient Client;
	CString strURL;
	CString strResponse;
	cJSON* pRoot = NULL;
	cJSON* pSuccess=NULL;
	cJSON* pResult=NULL;
	CString strErrMsg;
	CString strHeader;
	CString strBody;
	char* strBuildJson=NULL;
	cJSON* pItem=NULL;
	cJSON* RootObj= NULL;
	DWORD Idx = 0;
	std::vector<DWORD> vTempRetOK;
	vTempRetOK.clear();

	int nNeedRetry = -1;
	int nRetryCnt = 0;
	
	if (m_bQuit){
		nRet = -1;
		goto __end;
	}
	
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
	
	
	strHeader.Format("Content-Type: application/json;charset=UTF-8\r\n%s:%s", m_tCfg.strHeaderKeyPara, pCfg->strToken);

	if (m_vRetOK.size()== 0 || pCfg == NULL){	
		//m_FileLog.PrintLog(LOGLEVEL_ERR, "DoWork>>>>>>>>m_vRetOK.size()=%d>>>>>>>>>>", m_vRetOK.size());
		goto __end;
	}
	
	m_FileLog.PrintLog(LOGLEVEL_ERR, "DoWork>>>>>>>>start up CS>>>>>>>>>>");
	EnterCriticalSection(&m_CSUpload);
	if (m_vRetOK.size() > 1)
	{
		vTempRetOK.assign(m_vRetOK.begin()+1, m_vRetOK.end());
	}
	
	Idx= m_vRetOK[0];
	 m_vRetOK.clear();
	 if (vTempRetOK.size() > 0) {
		  m_vRetOK.assign(vTempRetOK.begin(), vTempRetOK.end());
	 }
	 LeaveCriticalSection(&m_CSUpload);
	m_FileLog.PrintLog(LOGLEVEL_ERR, "DoWork>>>>>>>>Leave CS>>>>>>>>>>");
	RootObj=cJSON_CreateObject();
	if(RootObj==NULL){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "TellResult>>>>>>>>cJSON_CreateObject Failed");
		goto __end;
	}
	cJSON_AddStringToObject(RootObj, "MoLotNo",(LPSTR)(LPCSTR)pCfg->strMoLotNo); 
	cJSON_AddStringToObject(RootObj, "BarCode",""); 
	m_FileLog.PrintLog(LOGLEVEL_ERR, ">>>>>>>>this is test code, idx=%d", Idx);
	cJSON_AddNumberToObject(RootObj, "BarcodeNo", m_SnCodeNoMap[Idx]/*m_vCurrRetBurnData[0].nSnCodeNo*/);  //�ͻ��з�˵���������ͬ�ģ�ȡ�������е�һ������
	
	pItem = cJSON_AddArrayToObject(RootObj, "KeyTypes");

	m_FileLog.PrintLog(LOGLEVEL_ERR, "<<<<<<<<<<t555555555555555, idx=%d====end", Idx);
	for (int i = 0 ; i </* m_vCurrRetBurnData.size()*/ m_AllRetBurnData[Idx].size(); i++){
		cJSON_AddStringToObject(pItem, "",  /*m_vCurrRetBurnData[i].*/m_AllRetBurnData[Idx][i].strKeyType);
	}
	m_FileLog.PrintLog(LOGLEVEL_ERR, "<<<<<<<<<<6666666ode, m_AllRetBurnData[Idx].size()=%d====end", m_AllRetBurnData[Idx].size());

	strBuildJson = cJSON_Print(RootObj);
	if (strBuildJson==NULL){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "TellResult cJSON_Print failed");
		goto __end;
	}
	strBody.Format("%s",strBuildJson);

	strURL.Format("https://%s/api/services/app/AtBind/BurnIsComplete", m_tCfg.strOSSURL );

	nNeedRetry = 0;
_Retry:
	strResponse.Empty();

	nHttpRet = m_UploadClient.HttpPost(strURL, strHeader, strBody, strResponse);
	m_FileLog.PrintLog(LOGLEVEL_ERR, "QuerySN strURL=%s, strBody=%s, HttpGet Ret=%d",strURL, strBody, nHttpRet );
	if (nHttpRet != 0){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "DoWork HttpPost Failed");
		if (nHttpRet==1){
		}else if (nHttpRet==2){
		}
		//httpÇëÇó³¬Ê±
		goto __end;
	}

	//////////////���Ƚ������ͷţ���Ϊ���ܽ���retry
	if (pRoot){
		cJSON_Delete(pRoot);
		pRoot = NULL;
	}
	///////////////////////////////

	pRoot=cJSON_Parse(strResponse.GetBuffer());
	if(pRoot==NULL){
		goto __end;
	}

	pSuccess = cJSON_GetObjectItem(pRoot,"Success");
	if(pSuccess == NULL){
		goto __end;
	}

	if ( cJSON_IsFalse(pSuccess)){
		strErrMsg.Format("%s",  cJSON_GetObjectItem(pRoot, "Error")->valuestring);
		goto __end ;
	}

	pResult = cJSON_GetObjectItem(pRoot,"Result");
	if (pResult == NULL){
		goto __end ;
	}

	nNeedRetry = 1;

__end:

	if (nNeedRetry ==0 ){
		if ( nRetryCnt < m_nRtryCnt){
			Sleep(100);
			nRetryCnt++;
			goto _Retry;
		}
	}
	
	if(strBuildJson){
		cJSON_free(strBuildJson);
	}
	if (RootObj){
		cJSON_Delete(RootObj);
	}
	if (pRoot){
		cJSON_Delete(pRoot);
	}

	return nRet;
}

BOOL CDlgSNCustom::InitCtrlsValue(CSNCustomCfg& SNCfg)
{
	tSNCustomCfg* pCfg=SNCfg.GetCustomCfg();
	CString strText;
	strText.Format("%I64X",pCfg->uSNAddr);
	GetDlgItem(IDC_SN_ADDR)->SetWindowText(strText);

	strText.Format("%s",/*pCfg->strAccount*/ m_tCfg.strAccount);
	GetDlgItem(IDC_ACCOUNT)->SetWindowText(strText);

	strText.Format("%s",/*pCfg->strPassword*/ m_tCfg.strPassword);
	GetDlgItem(IDC_PASSWORD)->SetWindowText(strText);

	strText.Format("%s",pCfg->strMoLotNo);
	GetDlgItem(IDC_BATCHNO)->SetWindowText(strText);

	UpdateData(FALSE);
	return TRUE;
}

BOOL CDlgSNCustom::InitCtrls(CSerial& lSerial)
{
	BOOL Ret=TRUE;

	CString strErrMsg;
	if(lSerial.GetLength()!=0){///ÔÙ´Î´ò¿ª¶Ô»°¿òµÄÊ±ºò¾Í»á´«Èë£¬µ«Ö®Ç°»áÒòÎª×ÊÔ´±»ÊÍ·Å£¬ËùÒÔÐèÒªÖØÐÂ´ò¿ªÎÄ¼þ
		Ret=m_SnCfg.SerialInCfgData(lSerial);
		if(Ret==TRUE){
			tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
		}
	}
	else{///Ê×´Î¼ÓÔØµÄÊ±ºòlSerial»áÃ»ÓÐÖµ
	}
	Ret=InitCtrlsValue(m_SnCfg);
	return Ret;
}

BOOL CDlgSNCustom::SaveCtrls()
{
	UpdateData(TRUE);
	CString strText;
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	CString strTemp;
	GetDlgItemText(IDC_SN_ADDR, strTemp);
	if(sscanf(strTemp,"%I64X",&pCfg->uSNAddr)!=1){
		MessageBox("��ȡSN��¼����ʼ��ַ������ȷ��");
		return FALSE;
	}
	
	strTemp.Empty();
	GetDlgItemText(IDC_ACCOUNT, strTemp);
	//pCfg->strAccount.Format("%s",strTemp);
	m_tCfg.strAccount.Format("%s",strTemp);

	strTemp.Empty();
	GetDlgItemText(IDC_PASSWORD, strTemp);
	//pCfg->strPassword.Format("%s",strTemp);
	m_tCfg.strPassword.Format("%s",strTemp);

	CString strIniFile;
	strIniFile.Format("%s\\sngen\\SCS_SN_CVTE_001.ini",GetCurrentPath());
	WritePrivateProfileString("Config", "Account", (LPCTSTR)m_tCfg.strAccount, strIniFile);
	WritePrivateProfileString("Config", "PassWord", (LPCTSTR)m_tCfg.strPassword, strIniFile);

	strTemp.Empty();
	GetDlgItemText(IDC_BATCHNO, strTemp);
	pCfg->strMoLotNo.Format("%s",strTemp);

	return TRUE;
}

BOOL CDlgSNCustom::GetCtrls(CSerial&lSerial)
{
	/*BOOL Ret=TRUE;
	Ret=SaveCtrls();
	if(Ret!=TRUE){
		return Ret;
	}
	try{
		UINT uTest = 1;
		lSerial<<uTest;
	}
	catch(...){
		return FALSE;
	}
	return TRUE;*/
	BOOL Ret=TRUE;
	Ret=SaveCtrls();
	if(Ret!=TRUE){
		return Ret;
	}
	return m_SnCfg.SerialOutCfgData(lSerial);
	
}




void CDlgSNCustom::GetJsonFile(bool bRet, DWORD Idx){
	//m_strJson = "";
	//cJSON * Root = NULL;
	//cJSON * RootCommunicate = NULL;
	//CString strValue;
	//strValue.Format("%s", "NG");
	//if (bRet){
	//	strValue.Format("%s", "OK");
	//}
	//CTime Time;
	//tagReport item;
	//item.index = -1;
	//item.strMac = "";
	//item.strRet = "";
	//item.strStartTime ="";
	//item.nTime = 0;
	//int index = Idx -1;
	//
	//if (Idx <= m_vReport.size()){
	//	if (m_vReport[index].index == Idx){
	//		item.index = m_vReport[index].index;
	//		item.strMac.Format("%s", m_vReport[index].strMac);
	//		item.strRet.Format("%s", strValue);
	//		item.strStartTime.Format("%s", m_vReport[index].strStartTime) ;
	//		item.nTime = m_vReport[index].nTime;
	//	}	
	//}

	//Time=CTime::GetCurrentTime();
	//strValue.Format("%d-%02d-%02d %02d:%02d:%02d", 
	//	Time.GetYear(),Time.GetMonth(),Time.GetDay(),
	//	Time.GetHour(),Time.GetMinute(),Time.GetSecond());
	//Root=cJSON_CreateObject();
	//cJSON_AddStringToObject(Root, "_ZGDH",(LPSTR)(LPCSTR)m_tCfg.WorkOrderID); 
	//cJSON_AddStringToObject(Root, "_StartTime",(LPSTR)(LPCSTR)item.strStartTime); 
	//

	//RootCommunicate=cJSON_CreateObject();
	//cJSON_AddItemToObject(Root, "Communicate", RootCommunicate);
	//
	//time_t endTime = time(NULL);
	//time_t nCostTime = endTime - item.nTime;
	//strValue.Format("%llu", nCostTime);
	//cJSON_AddStringToObject(Root, "_TestTUsetime",(LPSTR)(LPCSTR)strValue); 	

	//cJSON *pArr=cJSON_AddArrayToObject(Root, "TestResultList");

	//cJSON *element = cJSON_CreateObject();
	//cJSON_AddStringToObject(element, "_MAC",(LPSTR)(LPCSTR)item.strMac);
	//cJSON_AddStringToObject(element, "Result",(LPSTR)(LPCSTR)item.strRet);
	//cJSON_AddStringToObject(element, "FwResult",(LPSTR)(LPCSTR)item.strRet); 

	//cJSON_AddItemToArray(pArr, element);
	//m_strJson = cJSON_Print(Root);		

	//m_FileLog.PrintLog(LOGLEVEL_ERR, "GetJsonFile =%s, Idx=%d", m_strJson, Idx);
}

void CDlgSNCustom::SaveFailReportFile(){
	//CString strFilePath;
	//CTime Time;
	//CString strCurPath=GetCurrentPath();
	//Time=CTime::GetCurrentTime();
	//strFilePath.Format("%s\\%s_%d%02d%02d_%02d%02d%02d.json",
	//	m_tCfg.CachePath, m_tCfg.MACSNAddr,
	//	Time.GetYear(),Time.GetMonth(),Time.GetDay(),
	//	Time.GetHour(),Time.GetMinute(),Time.GetSecond() );

	//CFile File;
	//if(File.Open(strFilePath,CFile::modeCreate|CFile::modeWrite,NULL)==TRUE){
	//	File.Write(m_strJson, m_strJson.GetLength());
	//	File.Close();	
	//}

}

INT CDlgSNCustom::GetTokenFromServer(CString& strToken)
{
	INT nRet = FALSE;
	int nHttpRet = 0;
	//CHttpClient Client;
	CString strURL;
	CString strResponse;
	CString strErrMsg;
	cJSON* pRoot = NULL;
	cJSON* pSuccess=NULL;
	cJSON* pResult=NULL;
	CString strSuccess;
	CString strEnCodePwd;

	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	int nRetryCnt = 0;
_Retry:

	BYTE EnCode[4096] ={0};

	int nEnSize = /*pCfg->strPassword*/m_tCfg.strPassword.GetLength();
	int SizeConvert =base64_encode((uint8*)(LPCSTR)/*pCfg->strPassword*/m_tCfg.strPassword, nEnSize, EnCode);
	if (SizeConvert <= 0){
		goto __end; 
	}
	//ComTool::Hex2Str(ComTool::ENDIAN_LIT,EnCode, SizeConvert, strEnCodePwd);
	strEnCodePwd.Format("%s", EnCode);

	//strEnCodePwd.Format("9A826C26D5F53E2BA0482043BC07024F"); //��ʱ����

	strURL.Format("https://%s/api/Account/CreateToken?Email=%s&Pwd=%s", m_tCfg.strSysURL, m_tCfg.strAccount/*pCfg->strAccount*/, strEnCodePwd);
	
	nHttpRet = Client.HttpGet(strURL, "", "", strResponse);
	m_FileLog.PrintLog(LOGLEVEL_ERR, "QuerySN strURL=%s,  HttpGet Ret=%d",strURL, nHttpRet );
	if (nHttpRet != 0){
		if (nHttpRet==1){
		}else if (nHttpRet==2){
		}
		goto __end;
	}
	//////////////////���Ƚ�����retry
	if (pRoot){
		cJSON_Delete(pRoot);
		pRoot = NULL;
	}
	//////////////////

	pRoot=cJSON_Parse(strResponse.GetBuffer());
	if(pRoot==NULL ){
		goto __end;
	}

	pSuccess = cJSON_GetObjectItem(pRoot,"Success");
	if(pSuccess == NULL){
		goto __end;
	}

	if (cJSON_IsFalse(pSuccess)){
		strErrMsg.Format("%s",  cJSON_GetObjectItem(pRoot, "Error")->valuestring);
		goto __end ;
	}

	pResult = cJSON_GetObjectItem(pRoot,"Result");
	if (pResult == NULL){
		goto __end ;
	}
	strToken.Format("%s", pResult->valuestring);

	nRet = TRUE;


__end:

	if (nRet != TRUE){
		if ( nRetryCnt < m_nRtryCnt){
			Sleep(100);
			nRetryCnt++;
			goto _Retry;
		}
		
		m_FileLog.PrintLog(LOGLEVEL_ERR, "GetTokenFromServer fail");
	}

	if (pRoot){
		cJSON_Delete(pRoot);
	}
	
	return nRet;
}

INT CDlgSNCustom::GetBurnDataFromServerByRepeat(int nSNCode)
{
	INT nRet = FALSE;
	int nHttpRet = 0;
	//CHttpClient Client;
	CString strURL;
	CString strResponse;
	CString strErrMsg;
	cJSON* pRoot=NULL;
	cJSON* pSuccess=NULL;
	cJSON* pResult=NULL;
	CString strSuccess;
	CString strHeader;
	CString strBody;
	CString strResultJson;
	cJSON* pItem=NULL;

	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	strHeader.Format("Content-Type: application/json;charset=UTF-8\r\n%s:%s", m_tCfg.strHeaderKeyPara, pCfg->strToken);

	strURL.Format("https://%s/api/services/app/AtBind/GetBurnData?MoLotNo=%s&SnCodeNo=%d",m_tCfg.strFactoryURL, pCfg->strMoLotNo, nSNCode);

	int nRetryCnt = 0;
_Retry:

	nHttpRet = Client.HttpGet(strURL, strHeader, strBody, strResponse);
	m_FileLog.PrintLog(LOGLEVEL_ERR, "this is GetBurnDataFromServerByRepeat,strURL=%s,  HttpGet Ret=%d", strURL, nHttpRet );

	if (nHttpRet != 0){
		if (nHttpRet==1){
		}else if (nHttpRet==2){
		}
		//httpÇëÇó³¬Ê±
		goto __end;
	}

	//////////////////���Ƚ�����retry
	if (pRoot){
		cJSON_Delete(pRoot);
		pRoot = NULL;
	}
	//////////////////

	pRoot=cJSON_Parse(strResponse.GetBuffer());
	if(pRoot==NULL){
		goto __end;
	}

	pSuccess = cJSON_GetObjectItem(pRoot,"Success");
	if(pSuccess == NULL){
		goto __end;
	}

	if (cJSON_IsFalse(pSuccess)){
		strErrMsg.Format("%s",  cJSON_GetObjectItem(pRoot, "Error")->valuestring);
		goto __end ;
	}

	pResult = cJSON_GetObjectItem(pRoot,"Result");
	if (pResult == NULL){
		goto __end ;
	}

	if (cJSON_GetArraySize(pResult) == 0){
		goto __end ;
	}

	nRet = TRUE;

__end:

	if (nRet != TRUE){
		if ( nRetryCnt < m_nRtryCnt){
			Sleep(100);
			nRetryCnt++;
			goto _Retry;
		}
	}


	if (pRoot){
		cJSON_Delete(pRoot);
		pRoot = NULL;
	}
	
	return nRet;

}

INT CDlgSNCustom::GetBurnDataFromServer(int nIdx, CString& strKeyName)
{
	INT nRet = FALSE;
	int nHttpRet = 0;
	//CHttpClient Client;
	CString strURL;
	CString strResponse;
	CString strErrMsg;
	cJSON* pRoot=NULL;
	cJSON* pSuccess=NULL;
	cJSON* pResult=NULL;
	CString strSuccess;
	CString strHeader;
	CString strBody;
	CString strResultJson;
	cJSON* pItem=NULL;

	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
	
	strHeader.Format("Content-Type: application/json;charset=UTF-8\r\n%s:%s", m_tCfg.strHeaderKeyPara, pCfg->strToken);
	
	strURL.Format("https://%s/api/services/app/AtBind/GetBurnData?MoLotNo=%s&SnCodeNo=%d",m_tCfg.strFactoryURL, pCfg->strMoLotNo, 0);

	int nRetryCnt = 0;
_Retry:

	nHttpRet = Client.HttpGet(strURL, strHeader, strBody, strResponse);
	m_FileLog.PrintLog(LOGLEVEL_ERR, "sktidx = %d, QuerySN strURL=%s,   HttpGet Ret=%d", nIdx, strURL, nHttpRet );
	if (nHttpRet != 0){
		if (nHttpRet==1){
		}else if (nHttpRet==2){
		}
		//httpÇëÇó³¬Ê±
		goto __end;
	}

	//////////////////���Ƚ�����retry
	if (pRoot){
		cJSON_Delete(pRoot);
		pRoot = NULL;
	}
	//////////////////

	pRoot=cJSON_Parse(strResponse.GetBuffer());
	if(pRoot==NULL){
		goto __end;
	}

	pSuccess = cJSON_GetObjectItem(pRoot,"Success");
	if(pSuccess == NULL){
		goto __end;
	}
	
	if (cJSON_IsFalse(pSuccess)){
		strErrMsg.Format("%s",  cJSON_GetObjectItem(pRoot, "Error")->valuestring);
		goto __end ;
	}

	pResult = cJSON_GetObjectItem(pRoot,"Result");
	if (pResult == NULL){
		goto __end ;
	}

	if (cJSON_GetArraySize(pResult) == 0){
		goto __end ;
	}
	
	if (m_AllRetBurnData.find(nIdx) != m_AllRetBurnData.end()){
		m_AllRetBurnData[nIdx].clear();
	}

	m_vCurrRetBurnData.clear();
	
	for (int i = 0; i < cJSON_GetArraySize(pResult) ; i++){
		tRetBurnData CurrRetBurnData;
		cJSON* pChild =cJSON_GetArrayItem(pResult, i);

		pItem = cJSON_GetObjectItem(pChild,"KeyType");
		if (pItem == NULL){
			goto __end ;
		}
		CurrRetBurnData.strKeyType.Format("%s", pItem->valuestring);

		pItem = cJSON_GetObjectItem(pChild,"HashCode");
		if (pItem == NULL){
			goto __end ;
		}
		CurrRetBurnData.strHashCode.Format("%s", pItem->valuestring);

		pItem = cJSON_GetObjectItem(pChild,"KeyName");
		if (pItem == NULL){
			goto __end ;
		}
		CurrRetBurnData.strKeyName.Format("%s", pItem->valuestring);
		//strKeyName.Format("%s", pItem->valuestring);

		pItem = cJSON_GetObjectItem(pChild,"IsFirstMac");
		if (pItem == NULL){
			goto __end ;
		}
		CurrRetBurnData.bIsFirstMac = cJSON_IsFalse(pItem);

		pItem = cJSON_GetObjectItem(pChild,"TagName");
		if (pItem == NULL){
			goto __end ;
		}
		CurrRetBurnData.strTagName.Format("%s", pItem->valuestring);


		pItem = cJSON_GetObjectItem(pChild,"SnCodeNo");
		if (pItem == NULL){
			goto __end ;
		}
		CurrRetBurnData.nSnCodeNo= pItem->valueint;

		m_SnCodeNoMap[nIdx] = CurrRetBurnData.nSnCodeNo;

		m_FileLog.PrintLog(LOGLEVEL_ERR, "push info, nIdx=%d, CurrRetBurnData.nSnCodeNo=%d", nIdx , CurrRetBurnData.nSnCodeNo);

		m_vCurrRetBurnData.push_back(CurrRetBurnData);
		m_AllRetBurnData[nIdx].push_back(CurrRetBurnData);
	}
	
	nRet = TRUE;

__end:

	if (nRet != TRUE){
		if ( nRetryCnt < m_nRtryCnt){
			Sleep(100);
			nRetryCnt++;
			goto _Retry;
		}
	}

	if (pRoot){
		cJSON_Delete(pRoot);
	}
	return nRet;
}

INT CDlgSNCustom::DownloadKeyFileFromServer(CString strKeyName, CString& strSN,  std::vector<BYTE>& vHex)
{
	INT nRet = FALSE;
	int nHttpRet = 0;
	//CHttpClient Client;
	CString strURL;
	CString strResponse;
	CString strErrMsg;
	cJSON* pRoot = NULL;
	cJSON* pSuccess=NULL;
	cJSON* pResult=NULL;
	CString strHeader;
	CString strBody;

	//std::vector<BYTE> vHex;
	int nRetryCnt = 0;
_Retry:

	vHex.clear();
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	if (strKeyName.IsEmpty()){
		goto __end;
	}

	strHeader.Format("Content-Type: application/json;charset=UTF-8\r\n%s:%s", m_tCfg.strHeaderKeyPara, pCfg->strToken);

	strURL.Format("http://%s/Attachments/DownloadKeyFile?KeyName=%s&ContentType=%d",m_tCfg.strOSSURL, strKeyName, m_tCfg.nContentType);

	nHttpRet = Client.HttpGetRaw(strURL, strHeader, strBody, strResponse, vHex);
	m_FileLog.PrintLog(LOGLEVEL_ERR, "QuerySN strURL=%s, HttpGet Ret=%d",strURL, nHttpRet );
	if (nHttpRet != 0){
		if (nHttpRet==1){
		}else if (nHttpRet==2){
		}
		//httpÇëÇó³¬Ê±
		goto __end;
	}
	m_FileLog.PrintLog(LOGLEVEL_ERR, "start build data,  ContentType= %d",m_tCfg.nContentType );
	if (m_tCfg.nContentType == 0){ //ԭʼ�ֽ����� �ɹ����������Զ������json��ʽ���ض���ԭʼ�������ĸ�ʽ��
		pRoot=cJSON_Parse(strResponse.GetBuffer());
		if (true){//ע�⣺�ɹ����������Զ������json��ʽ���ض���ԭʼ�������ĸ�ʽ��
			nRet = TRUE;
		}
		//if(pRoot==NULL || pRoot->type !=cJSON_Object ){ 	
		//	m_FileLog.PrintLog(LOGLEVEL_ERR, "is not json format" );
		//	//strSN.Format("%s", strResponse);
		//	nRet = TRUE;
		//}else{
		//	pSuccess = cJSON_GetObjectItem(pRoot,"Success");
		//	if(pSuccess == NULL){
		//		goto __end;
		//	}

		//	if ( cJSON_IsFalse(pSuccess) ){
		//		strErrMsg.Format("%s",  cJSON_GetObjectItem(pRoot, "Error")->valuestring);
		//		goto __end ;
		//	}
		//}
	}else{

		//////////////////���Ƚ�����retry
		if (pRoot){
			cJSON_Delete(pRoot);
			pRoot = NULL;
		}
		//////////////////

		pRoot=cJSON_Parse(strResponse.GetBuffer());
		if(pRoot==NULL){
			goto __end;
		}
		pSuccess = cJSON_GetObjectItem(pRoot,"Success");
		if(pSuccess == NULL){
			goto __end;
		}
		if ( cJSON_IsFalse(pSuccess) ){
			strErrMsg.Format("%s",  cJSON_GetObjectItem(pRoot, "Error")->valuestring);
			goto __end ;
		}
		pResult = cJSON_GetObjectItem(pRoot,"Result");
		if (pResult == NULL){
			goto __end ;
		}
		strSN.Format("%s", strResponse); //�ַ�������ʽ
		nRet = TRUE;
	}

__end:

	if (nRet!= TRUE){
		if ( nRetryCnt < m_nRtryCnt){
			Sleep(100);
			nRetryCnt++;
			goto _Retry;
		}
	}

	if (pRoot){
		cJSON_Delete(pRoot);
	}
	return nRet;
}

bool CDlgSNCustom::CheckSize()
{
	bool bRtn =true;
	//if (m_vCurrRetBurnData.size() != m_tBurnALVFromFile.vArrGroup.size() || m_vCurrRetBurnData.size() == 0 ||
	//	m_tBurnALVFromFile.vArrGroup.size() == 0){
	//	bRtn = false;
	//}
	
	return bRtn;
}

INT CDlgSNCustom::QuerySN(UINT AdapIdx,DWORD Idx,BYTE*pData,INT*pSize)
{
	INT Ret=-1;
	CString strTime;
	CTime Time;
	CSerial lSerial;
	CString strFstCol,strSecCol;
	UINT GroupCnt= 0; //Ö»ÓÐ1×é
	CString strSN;

	CString strURL;
	CString strTemp;
	CString strResponse;
	CString strKeyName;
	INT TmpBufSize=1024*1024;
	UCHAR *pTransHex=NULL;

	std::vector<BYTE>vHex;
	vHex.clear();

	std::vector<tReUseMap> vTempReUse;
	vTempReUse.clear();
	bool bUseByRepeat = false;
	DWORD RepeatIdx = 0;

	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	if (!pCfg->bIsCanConnect){
		goto __end;
	}

	if (m_ApplyKey != TRUE)
	{
		//goto __end;
	}

	pTransHex=new BYTE[TmpBufSize];
	if(pTransHex==NULL){

		goto __end;
	}
	memset(pTransHex,0,TmpBufSize);
	
	EnterCriticalSection(&m_CSReUse);
	if (m_vReUse.size() > 0){ //�����õ�������
		RepeatIdx = m_vReUse[0].IdxReuse;
		vTempReUse.assign(m_vReUse.begin()+1, m_vReUse.end());
		m_FileLog.PrintLog(LOGLEVEL_ERR, "Enter GetBurnDataFromServerByRepeat, idx = %d, sncode=%d", m_vReUse[0].IdxReuse, m_vReUse[0].nSNCodeNo );

		if (GetBurnDataFromServerByRepeat(m_vReUse[0].nSNCodeNo) == FALSE){
			m_FileLog.PrintLog(LOGLEVEL_ERR, "GetBurnDataFromServerByRepeat fail");
			LeaveCriticalSection(&m_CSReUse);
			goto __end;
		}else{
			bUseByRepeat = true;
			m_vReUse.clear();
			if (vTempReUse.size() > 0){
				m_vReUse.assign(vTempReUse.begin(), vTempReUse.end());
			}
		}
	}
	LeaveCriticalSection(&m_CSReUse);
	
	if (bUseByRepeat == false){
		if (GetBurnDataFromServer(Idx, strKeyName) == FALSE){
			m_FileLog.PrintLog(LOGLEVEL_ERR, "GetBurnDataFromServer fail");
			goto __end;
		}
	}
	
	/*if (!CheckSize()){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "��̨���ص����ݴ�С�������ļ��еĴ�С��һ������" );	
		goto __end;
	}*/

	m_FileLog.PrintLog(LOGLEVEL_ERR, "enter >>>>>>>>  GetBurnDataFromServer>>>>>>>>");
	
	GroupCnt = m_tBurnALVFromFile.vArrGroup.size();
	lSerial<<GroupCnt;

	for (int i = 0; i < GroupCnt; i ++){
		std::vector<tRetBurnData> vMatchObj;
		CString strTemp;
		CString strCurDataType;
		CString strKeyTypeFromFile;
		strCurDataType.Format("%s", m_tBurnALVFromFile.vArrGroup[i].strDataType);
		strKeyTypeFromFile.Format("%s", m_tBurnALVFromFile.vArrGroup[i].strKeyType); //δ������������·���
		//strKeyName.Format("%s", m_vCurrRetBurnData[i].strKeyName);
		int nSNSize = m_tBurnALVFromFile.vArrGroup[i].SNSize ;
		UINT64 SNAddr = m_tBurnALVFromFile.vArrGroup[i].SNAddr;

		//m_FileLog.PrintLog(LOGLEVEL_ERR, "part 01>>>>>>>>>>>>>>>>");

		strKeyName.Empty();
		if (bUseByRepeat){
			//ʹ���ظ���
			for (int j = 0; j < m_AllRetBurnData[RepeatIdx].size(); j++){
				if (m_AllRetBurnData[RepeatIdx][j].strKeyType.CompareNoCase(strKeyTypeFromFile) == 0 ){
					strKeyName.Format("%s", m_AllRetBurnData[RepeatIdx][j].strKeyName);
					vMatchObj.push_back(m_AllRetBurnData[RepeatIdx][j]);
				}
			}
			//
		}else{
			for (int j = 0; j < m_vCurrRetBurnData.size(); j++){
				if (m_vCurrRetBurnData[j].strKeyType.CompareNoCase(strKeyTypeFromFile) == 0 ){
					strKeyName.Format("%s", m_vCurrRetBurnData[j].strKeyName);
					vMatchObj.push_back(m_vCurrRetBurnData[j]);
					/*MatchObj = m_vCurrRetBurnData[j];
					break;*/
				}
			}
		}
		
		//m_FileLog.PrintLog(LOGLEVEL_ERR, "part 02>>>>>>>>>>>>>>>>");

		if (strKeyName.IsEmpty()){
			m_FileLog.PrintLog(LOGLEVEL_ERR, "service�ӿڷ��ص�result��δ�ҵ�keyType��Ӧ��\"%s\",����ȷ��", strKeyTypeFromFile);
			goto __end;
		}

		if (strCurDataType.CompareNoCase("DATA") == 0){
			lSerial<<SNAddr;
			
			if (vMatchObj.size() <= 0){//Ϊdata�Ĵ�ģʽ��web service�Ǳ��뷵����Ϣ��
				m_FileLog.PrintLog(LOGLEVEL_ERR, "service�ӿڷ��ص�result��δ�ҵ�keyType��Ӧ��\"%s\",����ȷ��", strKeyTypeFromFile);
				goto __end;
			}
			bool bIsMacType = false;
			std::vector<std::vector<BYTE>> vList;
			//std::vector<tRetBurnData> vAllList;
			std::vector<tCacheMap> vCacheAllMap;
			//m_FileLog.PrintLog(LOGLEVEL_ERR, "part 03>>>>>>>>>>>>>>>>");

			vCacheAllMap.clear();
			CString strSum = "";
			strSN.Empty();
			for (int n = 0; n < vMatchObj.size(); n++){
				tCacheMap cell;
				tRetBurnData curItem = vMatchObj[n];
				std::vector<BYTE> vHex;
				if (curItem.strTagName.CompareNoCase("MAC") ==0){
					bIsMacType = true;
					if(ComTool::Str2Hex(curItem.strKeyName, ComTool::ENDIAN_BIG, vHex)==FALSE){
						m_FileLog.PrintLog(LOGLEVEL_ERR, "server���ص��ַ���:%s�к��зǷ��ַ�", curItem.strKeyType);
						goto __end;
					}
					//
					if (curItem.bIsFirstMac){
						strSN.Format("%s", curItem.strKeyName);
						continue;
					}
					vList.push_back(vHex);

					 cell.vData.assign(vHex.begin(), vHex.end());
					 cell.itemInfo = curItem;
					vCacheAllMap.push_back(cell);
					
				}
			}
			//m_FileLog.PrintLog(LOGLEVEL_ERR, "part 04>>>>>>>>>>>>>>>>");
			if (bIsMacType){
				if ( (vCacheAllMap.size() == 0 && strSN.IsEmpty())/* || (strSN.IsEmpty() )*/ ){
					m_FileLog.PrintLog(LOGLEVEL_ERR, "δ�ҵ�tagName=MAC����Ϣ������server���ص��ֶ���Ϣ");
					goto __end;
				}

				std::vector<BYTE> tmp;
				int nCntSize =vCacheAllMap.size() -1 ;
				

				//m_FileLog.PrintLog(LOGLEVEL_ERR, "part 05>>>>>>>>>>>>>>>>");
				
				for (int j = 0; j <nCntSize ; j++ ){
					for (int k = 0; k < nCntSize - j; k++){

						if (vCacheAllMap[k].vData.size() >vCacheAllMap[k +1].vData.size()  ){//��ķ��ұ�
							tmp.clear();
							tmp.assign(vCacheAllMap[k].vData.begin(), vCacheAllMap[k].vData.end());

							vCacheAllMap[k].vData.clear();
							vCacheAllMap[k].vData.assign(vCacheAllMap[k+1].vData.begin(), vCacheAllMap[k+1].vData.end());

							vCacheAllMap[k+1].vData.clear();
							vCacheAllMap[k+1].vData.assign(tmp.begin(), tmp.end());
						}
						else if (vCacheAllMap[k].vData.size() < vCacheAllMap[k +1].vData.size())//
						{
						}
						else{ //����һ�µ�����£�ֻ��ÿ�ֽڽ��бȽ�
							for (int m = 0; m < vCacheAllMap[k].vData.size(); m++){
								if (vCacheAllMap[k].vData[m] > vCacheAllMap[k+1].vData[m]){
									/////
									tmp.clear();
									tmp.assign(vCacheAllMap[k].vData.begin(), vCacheAllMap[k].vData.end());

									vCacheAllMap[k].vData.clear();
									vCacheAllMap[k].vData.assign(vCacheAllMap[k+1].vData.begin(), vCacheAllMap[k+1].vData.end());

									vCacheAllMap[k+1].vData.clear();
									vCacheAllMap[k+1].vData.assign(tmp.begin(), tmp.end());
									/////
									break;
								}	
							}
						}
					}
				}
				//m_FileLog.PrintLog(LOGLEVEL_ERR, "part 06>>>>>>>>>>>>>>>>");

				for (int n = 0; n < vCacheAllMap.size(); n++){
					strSN+= vCacheAllMap[n].itemInfo.strKeyName ;

					///////////////tagNameΪMac��ʱ��Ҳ����Ҫ����Hash����///////////////
					/*CString strInput;
					strInput.Format("%s", vCacheAllMap[n].itemInfo.strKeyName);
					if (!CheckHash(strInput.GetBuffer(), strInput.GetLength(), vCacheAllMap[n].itemInfo.strHashCode)){
						m_FileLog.PrintLog(LOGLEVEL_ERR, "hash����ʧ��");
						goto __end;
					}*/
					/////////////////////////////////////
				}
				//m_FileLog.PrintLog(LOGLEVEL_ERR, "part 07>>>>>>>>>>>>>>>>");
				int nTansHexLen = strSN.GetLength()/2;
				int nConvertSize =Str2Hex((UCHAR*)(LPCSTR)strSN, pTransHex,  nTansHexLen);
				if (nConvertSize<=0){
					m_FileLog.PrintLog(LOGLEVEL_ERR, "Illegal char is found, string = %s, please check it out", strTemp );
					 goto __end;
				}
				
				if ( nTansHexLen/*strSN.GetLength()*/ >nSNSize ){
					m_FileLog.PrintLog(LOGLEVEL_ERR, "���ȳ�����json����������ô�С�����õĳ��ȴ�С=%d, �ַ���:%s", nSNSize, strSN);
					goto __end;
				}

				lSerial<<nTansHexLen/*strSN.GetLength()*/;
				lSerial.SerialInBuff((UCHAR*)(LPCSTR)pTransHex, nTansHexLen/*(BYTE*)strSN.GetBuffer(),strSN.GetLength()*/);
				//m_FileLog.PrintLog(LOGLEVEL_ERR, "part 08>>>>>>>>>>>>>>>>");

			}else{
				vHex.clear();
				if (DownloadKeyFileFromServer(strKeyName, strSN, vHex) == FALSE){
					m_FileLog.PrintLog(LOGLEVEL_ERR, "DownloadKeyFileFromServer fail");
					goto __end;
				}
				//m_FileLog.PrintLog(LOGLEVEL_ERR, "part 09>>>>>>>>>>>>>>>>");
				if (m_tCfg.nContentType == 0){ //ԭʼ�ֽ���	
					if(true){
						int nFileLen = vHex.size();
						if (nFileLen == 0){
							AfxMessageBox("��ȡ���ĳ���Ϊ0");
							goto __end;
						}
						BYTE* pSave = new BYTE[nFileLen];
						memset(pSave, 0 , nFileLen);
						std::copy(vHex.begin(), vHex.end(), pSave);

						CFile File;
						CString strBin;
						strBin =GetCurrentPath();
						strBin += "\\Test.data";
						if(File.Open(strBin,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite|CFile::shareDenyNone,NULL)==FALSE){
							AfxMessageBox("�����ļ�ʧ��");
							goto __end;
						}
						File.Write(pSave, nFileLen);
						File.Close();
						SAFEDELARRAY(pSave);
					}
					//m_FileLog.PrintLog(LOGLEVEL_ERR, "part 10>>>>>>>>>>>>>>>>");
					///////////
					if (vHex.size() != nSNSize){
						m_FileLog.PrintLog(LOGLEVEL_ERR, "�ӿڷ��ص�SN���ȴ�С������json�����ļ��е�Լ����ʵ�ʷ��صĳ�����=%d, json�ļ������õĳ�����=%d", vHex.size(), nSNSize);
						goto __end;
					}

					////////////����Hash�Ƚ�/////////////////
					int nInSize = vHex.size();
					char* pInData = new char[nInSize];
					if (pInData == NULL){
						goto __end;
					}
					memset(pInData, 0, nInSize);
					//vHex[0]=0x05;
					std::copy(vHex.begin(), vHex.end(), pInData);
					
					//�ͻ�Ҫ�����ñ����ļ��ķ�ʽ��
					//if(true){
					//	CString strCONTENT;
					//	//ComTool::Hex2Str(ComTool::ENDIAN_BIG,(BYTE*)pInData, nInSize, strCONTENT);

					//	CFile File;
					//	CString strBin;
					//	strBin =GetCurrentPath();
					//	strBin += "\\Test.bin";
					//	if(File.Open(strBin,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite|CFile::shareDenyNone,NULL)==FALSE){
					//		//PrintLog(LOGLEVEL_ERR, "Creat or Open SNBin Failed!");
					//		goto __end;
					//	}
					//	File.Write(pInData, nInSize);
					//	File.Close();
					//}
					//m_FileLog.PrintLog(LOGLEVEL_ERR, "part 11>>>>>>>>>>>>>>>>");
					if (!CheckHash(pInData, nInSize, vMatchObj[0].strHashCode)){
						SAFEDELARRAY(pInData);
						m_FileLog.PrintLog(LOGLEVEL_ERR, "hash����ʧ��");
						goto __end;
					}
					SAFEDELARRAY(pInData);
					/*CString strCalHash;
					char pOutHash[4096]={0};
					int nInSize = vHex.size();
					char* pInData = new char[nInSize];
					if (pInData == NULL){
						goto __end;
					}
					memset(pInData, 0, nInSize);
					std::copy(vHex.begin(), vHex.end(), pInData);
					
					if (!sha1.SHA_GO(pInData, pOutHash)){
						SAFEDELARRAY(pInData);
						m_FileLog.PrintLog(LOGLEVEL_ERR, "ִ�л�ȡhashcode����ʧ��");
						goto __end;
					}
					SAFEDELARRAY(pInData);
					strCalHash.Format("%s", pOutHash);
					if (strCalHash.CompareNoCase(vMatchObj[0].strHashCode) != 0){
						m_FileLog.PrintLog(LOGLEVEL_ERR, "hashcode����ʧ��");
						goto __end;
					}*/
					///////////////////////////////////////////////////////////////
					
					lSerial<<vHex.size();
					for (int j = 0; j < vHex.size(); j++){
						lSerial<<vHex[j];
					}
				}
				else{ //�ַ���
					if ( strSN.GetLength() != nSNSize ){
						m_FileLog.PrintLog(LOGLEVEL_ERR, "�ӿڷ��ص�SN���ȴ�С������json�����ļ��е��趨, �ַ���=%s, josn�����õĳ�����=%d", strSN, nSNSize);
						goto __end;
					}
					//m_FileLog.PrintLog(LOGLEVEL_ERR, "part 12>>>>>>>>>>>>>>>>");
					///////////////hashУ��///////////////
					CString strInput;
					strInput.Format("%s", strSN);
					if (!CheckHash(strInput.GetBuffer(), strInput.GetLength(), vMatchObj[0].strHashCode)){
						m_FileLog.PrintLog(LOGLEVEL_ERR, "hash����ʧ��");
						goto __end;
					}
					/////////////////////////////////////
		//m_FileLog.PrintLog(LOGLEVEL_ERR, "part 13>>>>>>>>>>>>>>>>");
					lSerial<<strSN.GetLength();
					lSerial.SerialInBuff((BYTE*)strSN.GetBuffer(),strSN.GetLength());
				}
			}
		}else if (strCurDataType.CompareNoCase("NAME") == 0){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "part 14>>>>>>>>>>>>>>>>");
			strTemp.Empty();
			CString strExt= CComFunc::GetFileExt(strKeyName);
			if (!strExt.IsEmpty()){
				CString strSign;
				strSign.Format(".%s",strExt );
				strExt.Format("%s", strSign);
			}
			strKeyName.Replace(strExt, "");
			int nIdx = strKeyName.GetLength();
			while(nIdx >0){
				char curr = strKeyName.GetAt(nIdx -1);
				if (isdigit(curr)){
					nIdx--;
					continue;
				}
				strTemp.Format("%s", strKeyName.Right(strKeyName.GetLength() - nIdx));
				break;
			}

			if (strTemp.IsEmpty() || strTemp.GetLength() > nSNSize){
				m_FileLog.PrintLog(LOGLEVEL_ERR, "��ȡ���ĳ��Ȳ�����json�����ļ��е�Լ��");
				goto __end;
			}
			strSN.Format("%s", strTemp);
m_FileLog.PrintLog(LOGLEVEL_ERR, "part15>>>>>>>>>>>>>>>>");
			//////////////////////////ͨ���ļ�����ȡ�ķ�ʽ��û��ԭʼ�ֽ����ĸ���////////////
			lSerial<<SNAddr;
			if (m_tCfg.nContentType == 0 && false){ //ԭʼ�ֽ���				
				if (vHex.size() != nSNSize){
					goto __end;
				}

				lSerial<<vHex.size();
				for (int j = 0; j < vHex.size(); j++){
					lSerial<<vHex[j];
				}
			}
			else{ //�ַ���
				int nGap = 0;
				if ( strSN.GetLength() >nSNSize ){
					goto __end;
				}else if ( strSN.GetLength() < nSNSize) //����С�ڹ涨�������Ϊ0xff
				{
					nGap = nSNSize - strSN.GetLength();
				}

				///////////////Name��ʱ����ҪhashУ��///////////////
				/*CString strInput;
				strInput.Format("%s", strSN);
				if (!CheckHash(strInput.GetBuffer(), strInput.GetLength(), vMatchObj[0].strHashCode)){
					m_FileLog.PrintLog(LOGLEVEL_ERR, "hash����ʧ��");
					goto __end;
				}*/
				/////////////////////////////////////
				lSerial<<nSNSize/*strSN.GetLength()*/; //���ò�ff
				lSerial.SerialInBuff((BYTE*)strSN.GetBuffer(),strSN.GetLength());
				m_FileLog.PrintLog(LOGLEVEL_ERR, "part 16>>>>>>>>>>>>>>>>");

				for (int nAdd = 0; nAdd < nGap; nAdd++ ){
					BYTE tmp = 0xff;
					lSerial<<tmp;
				}
			}
		}
	}

	m_FileLog.PrintLog(LOGLEVEL_ERR, "enter >>>>>>>>  last>>>>>>>>");

	//////////////////////////////////////////////////
	memcpy(pData,lSerial.GetBuffer(),lSerial.GetLength());
	Ret = lSerial.GetLength();
	strSN.ReleaseBuffer();
	/////////////////////////////////////////////////

__end:
	if(pTransHex){
		delete[] pTransHex;
	}
	m_FileLog.PrintLog(LOGLEVEL_ERR, "enter >>>>>>>>  over>>>>>>>>");

	return Ret;
}

bool CDlgSNCustom::CheckHash(char* pInputData, int nInSize, CString strDestHash)
{
	bool bRtn = false;

	CString strCalHash;
	unsigned char pOutHash[4096]={0};
	sha1::Sha1 sha1Ins;

	if (pInputData == NULL || strDestHash.IsEmpty() || nInSize <= 0){
		goto __end;
	}
	
	/*if (!sha1Ins.update(pInputData, nInSize)){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "ִ�л�ȡhashcode����updateʧ��");
		goto __end;
	}
	sha1Ins.getDigestString(pOutHash);*/
	SHA1((BYTE*)pInputData, nInSize, pOutHash);
	//if (/*sha1.SHA_GO(pInputData, pOutHash, nInSize)*/){
	//	//SAFEDELARRAY(pInData);
	//	m_FileLog.PrintLog(LOGLEVEL_ERR, "ִ�л�ȡhashcode����ʧ��");
	//	goto __end;
	//}
	//SAFEDELARRAY(pInData);
	for (int i = 0; i < SHA_DIGEST_LENGTH; i++){
		CString strTmp;
		strTmp.Format("%02X",pOutHash[i] );
		strCalHash += strTmp;
	}
	//strCalHash.Format("%s", pOutHash);
	if (strCalHash.CompareNoCase(strDestHash) != 0){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "hashcode����ʧ��, ���������ֵ=%s, ����=%s", strCalHash , strDestHash);
		goto __end;
	}
	bRtn = true;

__end:
	return bRtn;
}

INT CDlgSNCustom::TellResult(DWORD Idx,INT IsPass)
{
	m_FileLog.PrintLog(LOGLEVEL_ERR, "TellResult Idx=%u", Idx);
	m_FileLog.PrintLog(LOGLEVEL_ERR, "TellResult Idx=%d, ret=%d", Idx, IsPass);
	INT Ret=0;
	CString strRet;
	strRet.Format("%d", IsPass);

	int nHttpRet = 0;
	//CHttpClient Client;
	CString strURL;
	CString strResponse;
	cJSON* pRoot = NULL;
	cJSON* pSuccess=NULL;
	cJSON* pResult=NULL;
	CString strErrMsg;
	CString strHeader;
	CString strBody;
	char* strBuildJson=NULL;
	cJSON* pItem=NULL;
	cJSON* RootObj= NULL;

	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	if (IsPass == 0){ //ʧ�ܲ��ϴ����ͻ�Ҫ��
		EnterCriticalSection(&m_CSReUse);
		tReUseMap tItem;
		tItem.IdxReuse = Idx;
		tItem.nSNCodeNo = m_SnCodeNoMap[Idx];
		m_vReUse.push_back(tItem);
		LeaveCriticalSection(&m_CSReUse);
		goto __end;
	}

	//if(true){ //ceshi
	//	goto __end;
	//}

	EnterCriticalSection(&m_CSUpload);
	m_vRetOK.push_back(Idx);
	LeaveCriticalSection(&m_CSUpload);
	if(true){
		goto __end;
	}

	//strHeader.Format("Content-Type: application/json;charset=UTF-8\r\n%s:%s", m_tCfg.strHeaderKeyPara, pCfg->strToken);

	//RootObj=cJSON_CreateObject();
	//if(RootObj==NULL){
	//	m_FileLog.PrintLog(LOGLEVEL_ERR, "TellResult>>>>>>>>cJSON_CreateObject Failed");
	//	goto __end;
	//}
	//cJSON_AddStringToObject(RootObj, "MoLotNo",(LPSTR)(LPCSTR)pCfg->strMoLotNo); 
	//cJSON_AddStringToObject(RootObj, "BarCode",""); 
	////m_FileLog.PrintLog(LOGLEVEL_ERR, ">>>>>>>>this is test code, idx=%d", Idx);
	//cJSON_AddNumberToObject(RootObj, "BarcodeNo", m_SnCodeNoMap[Idx]/*m_vCurrRetBurnData[0].nSnCodeNo*/);  //�ͻ��з�˵���������ͬ�ģ�ȡ�������е�һ������
	//m_FileLog.PrintLog(LOGLEVEL_ERR, "<<<<<<<<<<this is test code, idx=%d====end", Idx);
	//pItem = cJSON_AddArrayToObject(RootObj, "KeyTypes");

	//
	//for (int i = 0 ; i </* m_vCurrRetBurnData.size()*/ m_AllRetBurnData[Idx].size(); i++){
	//	cJSON_AddStringToObject(pItem, "",  /*m_vCurrRetBurnData[i].*/m_AllRetBurnData[Idx][i].strKeyType);
	//}

	//strBuildJson = cJSON_Print(RootObj);
	//if (strBuildJson==NULL){
	//		m_FileLog.PrintLog(LOGLEVEL_ERR, "TellResult cJSON_Print failed");
	//		goto __end;
	//}
	//strBody.Format("%s",strBuildJson);

	//strURL.Format("https://%s/api/services/app/AtBind/BurnIsComplete", m_tCfg.strOSSURL );

	//nHttpRet = Client.HttpPost(strURL, strHeader, strBody, strResponse);
	//m_FileLog.PrintLog(LOGLEVEL_ERR, "QuerySN strURL=%s, strBody=%s, HttpGet Ret=%d",strURL, strBody, nHttpRet );
	//if (nHttpRet != 0){
	//	m_FileLog.PrintLog(LOGLEVEL_ERR, "TellResult HttpPost Failed");
	//	if (nHttpRet==1){
	//	}else if (nHttpRet==2){
	//	}
	//	//httpÇëÇó³¬Ê±
	//	goto __end;
	//}

	//pRoot=cJSON_Parse(strResponse.GetBuffer());
	//if(pRoot==NULL){
	//	goto __end;
	//}

	//pSuccess = cJSON_GetObjectItem(pRoot,"Success");
	//if(pSuccess == NULL){
	//	goto __end;
	//}

	//if ( cJSON_IsFalse(pSuccess)){
	//	strErrMsg.Format("%s",  cJSON_GetObjectItem(pRoot, "Error")->valuestring);
	//	goto __end ;
	//}

	//pResult = cJSON_GetObjectItem(pRoot,"Result");
	//if (pResult == NULL){
	//	goto __end ;
	//}

__end:
	
	if(strBuildJson){
		cJSON_free(strBuildJson);
	}
	if (RootObj){
		cJSON_Delete(RootObj);
	}
	if (pRoot){
		cJSON_Delete(pRoot);
	}
	
	return Ret;
}

INT CDlgSNCustom::PreLoad()
{
	InitOnce();
	INT Ret=0;
	return Ret;
}

CString CDlgSNCustom::GetCurrentPath( void )
{
	TCHAR szFilePath[MAX_PATH + 1]; 
	TCHAR *pPos=NULL;
	CString str_url;
	GetModuleFileName(NULL, szFilePath, MAX_PATH); 
	pPos=_tcsrchr(szFilePath, _T('\\'));
	if(pPos!=NULL){
		pPos[0] = 0;//É¾³ýÎÄ¼þÃû£¬Ö»»ñµÃÂ·¾¶
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

INT CDlgSNCustom::CreatLog()
{
	CString strLogPath;
	CTime Time;
	CString strCurPath= GetCurrentPath();
	Time=CTime::GetCurrentTime();
	strLogPath.Format("%s\\sngen\\%d%02d%02d_%02d%02d%02d.txt", strCurPath,Time.GetYear(),Time.GetMonth(),Time.GetDay(),
		Time.GetHour(),Time.GetMinute(),Time.GetSecond());

	m_FileLog.SetLogFile(strLogPath);
	Client.SetLogMsg(&m_FileLog);
	m_UploadClient.SetLogMsg(&m_FileLog);
	return 0;
}

void CDlgSNCustom::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//DDX_Text(pDX, IDC_CSVFILE, m_strIniFilePath);
	DDX_Text(pDX, IDC_EDIT_JSONPATH, m_strJsonPath);
}


BEGIN_MESSAGE_MAP(CDlgSNCustom, CDialog)
	//ON_BN_CLICKED(IDC_BTNSELCSV, &CDlgSNCustom::OnBnClickedBtnselini)
	ON_EN_KILLFOCUS(IDC_SN_ADDR , &CDlgSNCustom::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_ACCOUNT, &CDlgSNCustom::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_PASSWORD , &CDlgSNCustom::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_BATCHNO, &CDlgSNCustom::OnEnKillfocusEdit)
	ON_BN_CLICKED(IDC_BTN_SEL_JSON_PATH, &CDlgSNCustom::OnBnClickedBtnSelJsonPath)
	ON_BN_CLICKED(IDC_BTN_APPLY_KEY, &CDlgSNCustom::OnBnClickedBtnApplyKey)
END_MESSAGE_MAP()


// CDlgSNCustom ÏûÏ¢´¦Àí³ÌÐò


INT CDlgSNCustom::CheckIsCanConnectToServer()
{
	INT nRet = FALSE;
	INT nHttpRet = 0;
	CString strErrMsg;
	//CHttpClient Client;
	CString strURL;
	CString strTemp;
	CString strResponse;
	
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	if (pCfg->strToken.IsEmpty()){
		goto __end;
	}

	strURL.Format("https://%s/OSS/TmpFixs/IsOssCanConnect", m_tCfg.strOSSURL);

	nHttpRet = Client.HttpGet(strURL, "", "", strResponse);
	m_FileLog.PrintLog(LOGLEVEL_ERR, "QuerySN strURL=%s,  HttpGet Ret=%d",strURL, nHttpRet );
	if (nHttpRet != 0){
		if (nHttpRet==1){
		}else if (nHttpRet==2){
		}
		//httpÇëÇó³¬Ê±
		goto __end;
	}
	
	pCfg->bIsCanConnect = false;
	if (strResponse.CompareNoCase("1") == 0){
		pCfg->bIsCanConnect = true;
	}

	nRet = TRUE;

__end:
	return nRet;

}
void CDlgSNCustom::InitOnce(){
	//AfxMessageBox("ggggg");
	m_vReport.clear();
	//m_wUpLoadRet.PostMsg(MSG_START_WORK, 0, 0);
	CreatLog();
	m_SnCodeNoMap.clear();

	CHAR TmpBuf[512];
	CString strIniFile;
	memset(TmpBuf,0,512);
	strIniFile.Format("%s\\sngen\\SCS_SN_CVTE_001.ini",GetCurrentPath());
	if (!PathFileExists(strIniFile)){
		AfxMessageBox("�Ҳ���SCS_SN_CVTE_001.ini�����ļ�������!");
		return ;
	}
	
	memset(TmpBuf,0,512);
	GetPrivateProfileString("Config", "FactoryURL", "",TmpBuf, MAX_PATH, strIniFile);
	m_tCfg.strFactoryURL.Format("%s", TmpBuf);
	
	memset(TmpBuf,0,512);
	GetPrivateProfileString("Config", "OSSURL", "",TmpBuf, MAX_PATH, strIniFile);
	m_tCfg.strOSSURL.Format("%s", TmpBuf);
	
	memset(TmpBuf,0,512);
	GetPrivateProfileString("Config", "SysURL", "",TmpBuf, MAX_PATH, strIniFile);
	m_tCfg.strSysURL.Format("%s", TmpBuf);

	m_tCfg.nContentType = GetPrivateProfileInt("Config", "ContentType ",0, strIniFile);
	if (m_tCfg.nContentType == 1) {
		AfxMessageBox("ĿǰContentTypeֻ���ֽ����ģ��ַ���������ϵ�з���Աȷ��");
		return ;
	}
	
	memset(TmpBuf,0,512);
	GetPrivateProfileString("Config", "HeaderKeyPara", "",TmpBuf, MAX_PATH, strIniFile);
	m_tCfg.strHeaderKeyPara.Format("%s", TmpBuf);
	

	GetPrivateProfileString("Config", "Account", "",TmpBuf, MAX_PATH, strIniFile);
	m_tCfg.strAccount.Format("%s", TmpBuf);

	GetPrivateProfileString("Config", "PassWord", "",TmpBuf, MAX_PATH, strIniFile);
	m_tCfg.strPassword.Format("%s", TmpBuf);

	/*GetPrivateProfileString("Config", "BatchNo","",TmpBuf, MAX_PATH, strIniFile);
	m_tCfg.strBatchNo.Format("%s", TmpBuf);

	CString strTemp;
	GetPrivateProfileString("Config", "SNAddr", "0",TmpBuf, MAX_PATH, strIniFile);
	strTemp.Format("%s", TmpBuf);
	sscanf(strTemp, "%I64X", &m_tCfg.uSNAddr);*/

	m_FileLog.PrintLog(LOGLEVEL_ERR, "FactoryURL=%s, OSSURL=%s, SysURL=%s", 
		m_tCfg.strFactoryURL, m_tCfg.strOSSURL, m_tCfg.strSysURL);
	
	CString strToken;
	tSNCustomCfg* pCfg = m_SnCfg.GetCustomCfg() ;	
	if (GetTokenFromServer(strToken) == TRUE){
		pCfg->strToken.Format("%s", strToken);
		CheckIsCanConnectToServer();
	}
}
//BOOL CDlgSNCustom::OnInitDialog()
//{
//	CDialog::OnInitDialog();
//	
//	
//	// TODO:  ÔÚ´ËÌí¼Ó¶îÍâµÄ³õÊ¼»¯
//	return FALSE;
//	//return TRUE;  // return TRUE unless you set the focus to a control
//	// Òì³£: OCX ÊôÐÔÒ³Ó¦·µ»Ø FALSE
//}



void CDlgSNCustom::OnEnKillfocusEdit()
{
	// TODO: ÔÚ´ËÌí¼Ó¿Ø¼þÍ¨Öª´¦Àí³ÌÐò´úÂë
	SaveCtrls();
}

void CDlgSNCustom::OnBnClickedBtnSelJsonPath()
{
	CFileDialog Dlg(TRUE,NULL,NULL,OFN_PATHMUSTEXIST,"Json File(*.json)|*.json||",this);
	if(Dlg.DoModal()==IDOK){
		m_strJsonPath=Dlg.GetPathName();
		if (Parser_Json() == FALSE){
			AfxMessageBox("����json�ļ�ʧ�ܣ�������ļ��Ƿ����json��ʽ");
		}
	}
	else{
		m_strJsonPath="";
	}



	UpdateData(FALSE);
}


BOOL CDlgSNCustom::Parser_Json()
{
	BOOL nRtn = FALSE;
	cJSON* pRoot = NULL;
	cJSON* pNode;
	cJSON* pItem;
	CFile file;
	BYTE* pData =NULL;
	UINT uFileLen = 0;
	CString strTemp;
	if (file.Open(m_strJsonPath, CFile::modeRead|CFile::shareDenyNone, NULL) == FALSE){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "��json�ļ�ʧ��" );
		goto __end;
	}

	uFileLen = file.GetLength();
	pData = new BYTE[uFileLen];
	if (pData == NULL){
		goto __end;
	}
	memset(pData, 0, uFileLen);
	file.Read(pData, uFileLen);

	pRoot = cJSON_Parse((LPCSTR)pData);
	if (pRoot == NULL){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "����json����ʧ��");
		goto __end;
	}

	m_tBurnALVFromFile.vArrGroup.clear();
	m_DataTypeMap.clear();

	m_tBurnALVFromFile.nGroup = cJSON_GetObjectItem(pRoot, "Group")->valueint;
	m_tBurnALVFromFile.strChipType = cJSON_GetObjectItem(pRoot, "ChipType")->valuestring; 
	pNode = cJSON_GetObjectItem(pRoot, "SNInfo");
	for (int i = 0; i < cJSON_GetArraySize(pNode); i++){
		std::map<CString, int>::iterator it;
		tUnitInfo UnitInfo;
		pItem = cJSON_GetArrayItem(pNode, i);
		if (pItem == NULL){
			goto __end;
		}

		UnitInfo.nGroupIdx = cJSON_GetObjectItem(pItem, "GroupIdx")->valueint;
		UnitInfo.strDataType = cJSON_GetObjectItem(pItem, "DataType")->valuestring;
		if (UnitInfo.strDataType.IsEmpty()){
			AfxMessageBox("DataType�ֶ�ֵ����Ϊ�գ��������ȷ��json�����ļ�");
			goto __end ;
		}

		UnitInfo.strKeyType = cJSON_GetObjectItem(pItem, "KeyType")->valuestring;
		if (UnitInfo.strKeyType.IsEmpty()){
			AfxMessageBox("keyType�ֶ�ֵ����Ϊ�գ��������ȷ��json�����ļ�");
			goto __end ;
		}

		strTemp.Format("%s",  cJSON_GetObjectItem(pItem, "SNAddr")->valuestring);
		strTemp.Replace("0x", "");
		strTemp.Replace("0X", "");
		sscanf(strTemp, "%I64X", &UnitInfo.SNAddr );
		
		strTemp.Format("%s",  cJSON_GetObjectItem(pItem, "SNSize")->valuestring);
		strTemp.Replace("0x", "");
		strTemp.Replace("0X", "");
		sscanf(strTemp, "%u", &UnitInfo.SNSize );
		
		m_tBurnALVFromFile.vArrGroup.push_back(UnitInfo);

		it = m_DataTypeMap.find(UnitInfo.strKeyType) ;
		if (it == m_DataTypeMap.end()){
			m_DataTypeMap[UnitInfo.strKeyType]++;
		}

	}
	
	nRtn = TRUE;
__end:

	if (pRoot){
		cJSON_Delete(pRoot);
	}

	SAFEDELARRAY(pData);

	file.Close();
	return nRtn;
}


BOOL CDlgSNCustom::GetDataFromShareMem(DWORD& nData)
{
	CString strMsg;

	BOOL Ret = FALSE;
	int nSize = MAX_PATH;
	int pid = _getpid();
	CString strName;
	strName.Format("AC_MULTI_SMForBurnSN");

	HANDLE hMap = OpenFileMapping(FILE_MAP_WRITE, FALSE, strName);
	if (hMap == NULL) {
		DWORD dwError = GetLastError();

		strMsg.Format("open file_map  fail %d", dwError);
		/*AfxMessageBox(strMsg);*/
		return Ret;
	}

	LPSTR pMapView = (LPSTR)MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, nSize);
	if (pMapView == NULL) {
		/*AfxMessageBox("��дӳ������ʧ��");*/
		CloseHandle(hMap); 
		return Ret;
	}
	
	memcpy(&nData, pMapView, sizeof(DWORD));

	UnmapViewOfFile(pMapView);  
	CloseHandle(hMap);  

	Ret = TRUE;
	return Ret;
}

void CDlgSNCustom::WriteShareMem(DWORD nData, bool bIsInit)
{
	int nSize = MAX_PATH;
	int pid = _getpid();
	CString strName;
	strName.Format("AC_MULTI_SMForBurnSN");

	HANDLE hMap = OpenFileMapping(FILE_MAP_WRITE, FALSE, strName);
	if (hMap == NULL) {
		//AfxMessageBox("open file_map  fail");
		return;
	}

	LPSTR pMapView = (LPSTR)MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, nSize);
	if (pMapView == NULL) {
		//AfxMessageBox("��дӳ������ʧ��");
		CloseHandle(hMap);  
		return;
	}

	if (bIsInit)
	{
		memset(pMapView, 0, nSize); //�����е�������յ�
	}
	else {
		memcpy(pMapView, &nData, sizeof(DWORD));
	}

	UnmapViewOfFile(pMapView);  
	CloseHandle(hMap);  
}

void CDlgSNCustom::OnBnClickedBtnApplyKey()
{
	UpdateData(TRUE);
	CString strRet;

	/*DWORD dwTotal = 0;
	GetDataFromShareMem(dwTotal);
	strRet.Format("���%d", dwTotal);
	AfxMessageBox(strRet);*/

	if (Do_Test == 1)
	{
		GetKeyByMo("1234");
		return ;
	}

	GetDlgItem(IDC_BTN_APPLY_KEY)->EnableWindow(FALSE);

	CString strTemp;
	GetDlgItemText(IDC_BATCHNO, strTemp);

	if ( strTemp.IsEmpty() || m_strJsonPath.IsEmpty() || (PathFileExists(m_strJsonPath) != TRUE)  )
	{
		AfxMessageBox("������д��������Ϣ");
		goto __end ;
	}

	if (CreateBindTask(strTemp) != TRUE)
	{
		strRet.Format("����ʧ�ܣ�������");
		AfxMessageBox(strRet);
		GetDlgItem(IDC_STATIC_RESULT)->SetWindowText(strRet);
		goto __end ;
	}
	
	strRet.Format("���ڽ�����...�����ĵȴ�");
	GetDlgItem(IDC_STATIC_RESULT)->SetWindowText(strRet);
	Sleep(1000);
	
	if (GetKeyByMo(strTemp) != TRUE){
		strRet.Format("����ʧ�ܣ�������");
		AfxMessageBox(strRet);
		GetDlgItem(IDC_STATIC_RESULT)->SetWindowText(strRet);
	}else{
		m_ApplyKey = TRUE;
		strRet.Format("����ɹ�");
		AfxMessageBox(strRet);
		GetDlgItem(IDC_STATIC_RESULT)->SetWindowText(strRet);
	}

__end:

	GetDlgItem(IDC_BTN_APPLY_KEY)->EnableWindow(TRUE);
	return ;
}


BOOL CDlgSNCustom::CreateBindTask(CString strMoLotNo)
{
	BOOL Rtn = FALSE;
	int nHttpRet = 0;
	//CHttpClient Client;
	CString strURL;
	CString strResponse;
	CString strErrMsg;
	cJSON* pRoot=NULL;
	cJSON* pSuccess=NULL;
	cJSON* pResult=NULL;
	CString strSuccess;
	CString strHeader;
	CString strBody;
	CString strResultJson;
	cJSON* pItem=NULL;
	cJSON* RootObj = NULL;
	char* pBuildJson = NULL;

	DWORD dwTotal = 0;
	GetDataFromShareMem(dwTotal);
	if (dwTotal == 0)
	{
		m_FileLog.PrintLog(LOGLEVEL_ERR, "CreateBindTask,	TotalNum is invaild, is cannot be less than 0");
		goto __end;
	}

	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	strHeader.Format("Content-Type: application/json;charset=UTF-8\r\n%s:%s", m_tCfg.strHeaderKeyPara, pCfg->strToken);

	strURL.Format("https://%s/api/services/app/Tasks/CreateBindTask",m_tCfg.strFactoryURL);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	RootObj=cJSON_CreateObject();
	if(RootObj==NULL){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "CreateBindTask,	cJSON_CreateObject Failed");
		goto __end;
	}
	cJSON_AddStringToObject(RootObj, "MoLotNo",(LPSTR)(LPCSTR)strMoLotNo/*pCfg->strMoLotNo*/); 
	cJSON_AddNumberToObject(RootObj, "origin",2); 
	cJSON_AddNumberToObject(RootObj, "beginNo",1); 
	cJSON_AddNumberToObject(RootObj, "endNo",dwTotal /*m_pSnCfgPara->dwTotalNum*/); 

	pBuildJson = cJSON_Print(RootObj);
	if (pBuildJson==NULL){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "CreateBindTask, cJSON_Print failed");
		goto __end;
	}
	strBody.Format("%s",pBuildJson);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int nRetryCnt = 0;
_Retry:

	nHttpRet = Client.HttpPost(strURL, strHeader, strBody, strResponse);
	m_FileLog.PrintLog(LOGLEVEL_ERR, "this is CreateBindTask,strURL=%s,  HttpGet Ret:%d, Response:%s", strURL, nHttpRet,strResponse );

	if (nHttpRet != 0){
		if (nHttpRet==1){
		}else if (nHttpRet==2){
		}
		//httpÇëÇó³¬Ê±
		goto __end;
	}

	//////////////////���Ƚ�����retry
	if (pRoot){
		cJSON_Delete(pRoot);
		pRoot = NULL;
	}
	//////////////////

	pRoot=cJSON_Parse(strResponse.GetBuffer());
	strResponse.ReleaseBuffer();
	if(pRoot==NULL){
		goto __end;
	}

	pSuccess = cJSON_GetObjectItem(pRoot,"success");
	if(pSuccess == NULL){
		goto __end;
	}

	if (cJSON_IsFalse(pSuccess)){
		strErrMsg.Format("%s",  cJSON_GetObjectItem(pRoot, "error")->valuestring);
		goto __end ;
	}

	/*pResult = cJSON_GetObjectItem(pRoot,"result");
	if (pResult == NULL){
		goto __end ;
	}*/


	Rtn = TRUE;

__end:

	if (Rtn != TRUE&& false){
		if ( nRetryCnt < m_nRtryCnt){
			Sleep(100);
			nRetryCnt++;
			goto _Retry;
		}
	}

	if (pRoot != NULL){
		cJSON_Delete(pRoot);
		pRoot = NULL;
	}

	if (RootObj != NULL)
	{
		cJSON_Delete(RootObj);
		RootObj = NULL;
	}

	return Rtn;
}

BOOL CDlgSNCustom::GetKeyByMo(CString strMoLotNo)
{
	BOOL Rtn = FALSE;
	int nHttpRet = 0;
	//CHttpClient Client;
	CString strURL;
	CString strResponse;
	CString strErrMsg;
	cJSON* pRoot=NULL;
	cJSON* pSuccess=NULL;
	cJSON* pResult=NULL;
	CString strSuccess;
	CString strHeader;
	CString strBody;
	CString strResultJson;
	cJSON* pItem=NULL;

	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	strHeader.Format("Content-Type: application/json;charset=UTF-8\r\n%s:%s", m_tCfg.strHeaderKeyPara, pCfg->strToken);

	strURL.Format("https://%s/api/services/app/AtBind/GetKeyByMo?MoLotNo=%s",m_tCfg.strFactoryURL, (LPSTR)(LPCSTR)strMoLotNo/*pCfg->strMoLotNo*/);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	time_t startTime = time(NULL);
	
	bool bIsTimeOut = false;
	int nRetryCnt = 0;
_Retry:

	time_t currTime = time(NULL);
	if ( (currTime -  startTime) >= MAX_WAIT_TIME ){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "��ʱʱ���ѵ�,����ϵCVTE��Ա");
		bIsTimeOut = true;
		goto __end;
	}
	
	if (Do_Test == 1)
	{
		BYTE pFileData[1024] = {0};
		CFile File;
		CString strFilePath;
		strFilePath.Format("C:\\Users\\SF_Engineer2\\Desktop\\372\\1.json");

		if(File.Open(strFilePath,CFile::modeRead,NULL)==TRUE){
			File.Read(pFileData,File.GetLength());
			File.Close();	
		}
		strResponse.Format("%s", pFileData);
	}else{
		nHttpRet = Client.HttpGet(strURL, strHeader, strBody, strResponse);
	}
	m_FileLog.PrintLog(LOGLEVEL_ERR, "this is GetKeyByMo,strURL=%s,  HttpGet Ret:%d, Response:%s", strURL, nHttpRet,strResponse );

	if (nHttpRet != 0){
		if (nHttpRet==1){
		}else if (nHttpRet==2){
		}
		//httpÇëÇó³¬Ê±
		goto __end;
	}

	//////////////////���Ƚ�����retry
	if (pRoot != NULL){
		cJSON_Delete(pRoot);
		pRoot = NULL;
	}
	//////////////////

	pRoot=cJSON_Parse(strResponse.GetBuffer());
	strResponse.ReleaseBuffer();
	if(pRoot==NULL){
		goto __end;
	}

	pSuccess = cJSON_GetObjectItem(pRoot,"success");
	if(pSuccess == NULL){
		goto __end;
	}

	if (cJSON_IsFalse(pSuccess)){
		strErrMsg.Format("%s",  cJSON_GetObjectItem(pRoot, "error")->valuestring);
		goto __end ;
	}

	pResult = cJSON_GetObjectItem(pRoot,"result");
	if (pResult == NULL){
		goto __end ;
	}

	/////////////////////////////////////////////////////////////////////////////////
	if (cJSON_GetArraySize(pResult) == 0){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "mes���ص�result����Ϊ0");
		goto __end ;
	}

	for (int i = 0; i < cJSON_GetArraySize(pResult) ; i++){

		cJSON* pChild =cJSON_GetArrayItem(pResult, i);

		pItem = cJSON_GetObjectItem(pChild,"items");
		if (pItem == NULL){
			goto __end ;
		}

		for (int j = 0; j < cJSON_GetArraySize(pItem); j++){ //

			cJSON* pNode =cJSON_GetArrayItem(pItem, j);
			if (pNode == NULL){
				goto __end ;
			}
			
			cJSON* pleftNum = cJSON_GetObjectItem(pNode,"leftNum");
			if (pleftNum == NULL){
				goto __end ;
			}

			int nCurr = pleftNum->valueint;
			if ( nCurr <   m_pSnCfgPara->dwTotalNum){
				Sleep(10*1000); //ÿ��10s��ѯ��
				goto _Retry;
			}
		}
	}
	/////////////////////////////////////////////////////////////////////////////////

	Rtn = TRUE;

__end:

	if (Rtn != TRUE){ //�ͻ��������ܺ���״����Ҫѭ���ȴ���ֱ��20���ӳ�ʱʱ�䵽��
		if ( !bIsTimeOut){
			Sleep(10*1000);
			nRetryCnt++;
			goto _Retry;
		}
	}

	if (pRoot != NULL){
		cJSON_Delete(pRoot);
		pRoot = NULL;
	}
	return Rtn;
}
