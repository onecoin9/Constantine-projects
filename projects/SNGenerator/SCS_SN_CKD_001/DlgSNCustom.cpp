// DlgSNCustom.cpp : 脢碌脧脰脦脛录镁
//

#include "stdafx.h"
#include "SCS_SN_CKD_001.h"
#include "DlgSNCustom.h"
#include "HttpClient.h"
#include "../Com/ComTool.h"
#include "../Com/cJSON.h"
#include "Base64.h"


// CDlgSNCustom 露脭禄掳驴貌

IMPLEMENT_DYNAMIC(CDlgSNCustom, CDialog)

CDlgSNCustom::CDlgSNCustom(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSNCustom::IDD, pParent)
	, m_pSnCfgPara(pSnCfgPara)
{
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
	/*pCfg->strAccount.Empty();
	pCfg->strPassword.Empty();*/
	pCfg->strMoLotNo.Empty();
	pCfg->uSNAddr = 0;

	CHAR TmpBuf[512];
	CString strIniFile;
	memset(TmpBuf,0,512);
	strIniFile.Format("%s\\sngen\\SCS_SN_CKD_001.ini",GetCurrentPath());
	if (!PathFileExists(strIniFile)){
		//AfxMessageBox("找不到SCS_SN_CKD_001.ini配置文件，请检查!");
		//return ;
	}
	GetPrivateProfileString("Config", "Account", "",TmpBuf, MAX_PATH, strIniFile);
	m_tCfg.strAccount.Format("%s", TmpBuf);
	memset(TmpBuf,0,512);
	GetPrivateProfileString("Config", "PassWord", "",TmpBuf, MAX_PATH, strIniFile);
	m_tCfg.strPassword.Format("%s", TmpBuf);


	if (true)
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
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		time_t startTime = time(NULL);

		int nRetryCnt = 0;
_Retry:

		time_t currTime = time(NULL);
		if ( (currTime -  startTime) >= 5000 ){
			m_FileLog.PrintLog(LOGLEVEL_ERR, "超时时间已到,请联系CVTE人员");
			goto __end;
		}

		if (true)
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
		}

		if (nHttpRet != 0){
			if (nHttpRet==1){
			}else if (nHttpRet==2){
			}
			//http脟毛脟贸鲁卢脢卤
			goto __end;
		}

		//////////////////事先进行下retry
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
			m_FileLog.PrintLog(LOGLEVEL_ERR, "mes返回的result数量为0");
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
					Sleep(10*1000); //每隔10s轮询下
					goto _Retry;
				}
			}
		}
		/////////////////////////////////////////////////////////////////////////////////

		Rtn = TRUE;

__end:

		if (pRoot != NULL){
			cJSON_Delete(pRoot);
			pRoot = NULL;
		}

	}
}

CDlgSNCustom::~CDlgSNCustom()
{
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
	if(lSerial.GetLength()!=0){///脭脵麓脦麓貌驴陋露脭禄掳驴貌碌脛脢卤潞貌戮脥禄谩麓芦脠毛拢卢碌芦脰庐脟掳禄谩脪貌脦陋脳脢脭麓卤禄脢脥路脜拢卢脣霉脪脭脨猫脪陋脰脴脨脗麓貌驴陋脦脛录镁
		Ret=m_SnCfg.SerialInCfgData(lSerial);
		if(Ret==TRUE){
			tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
		}
	}
	else{///脢脳麓脦录脫脭脴碌脛脢卤潞貌lSerial禄谩脙禄脫脨脰碌
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
		MessageBox("获取SN烧录的起始地址错误，请确认");
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
	strIniFile.Format("%s\\sngen\\SCS_SN_CKD_001.ini",GetCurrentPath());
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
	CHttpClient Client;
	CString strURL;
	CString strResponse;
	CString strErrMsg;
	cJSON* pRoot;
	cJSON* pSuccess;
	cJSON* pResult;
	CString strSuccess;
	CString strEnCodePwd;

	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	BYTE EnCode[4096] ={0};
	int nEnSize = /*pCfg->strPassword*/m_tCfg.strPassword.GetLength();
	int SizeConvert =base64_encode((uint8*)(LPCSTR)/*pCfg->strPassword*/m_tCfg.strPassword, nEnSize, EnCode);
	if (SizeConvert <= 0){
		goto __end; 
	}
	//ComTool::Hex2Str(ComTool::ENDIAN_LIT,EnCode, SizeConvert, strEnCodePwd);
	strEnCodePwd.Format("%s", EnCode);

	strURL.Format("https://%s/api/Account/CreateToken?Email=%s&Pwd=%s", m_tCfg.strSysURL, m_tCfg.strAccount/*pCfg->strAccount*/, strEnCodePwd);
	nHttpRet = Client.HttpGet(strURL, "", "", strResponse);
	m_FileLog.PrintLog(LOGLEVEL_ERR, "QuerySN strURL=%s, strResponse=%s, HttpGet Ret=%d",strURL, strResponse, nHttpRet );
	if (nHttpRet != 0){
		if (nHttpRet==1){
		}else if (nHttpRet==2){
		}
		//http脟毛脟贸鲁卢脢卤
		goto __end;
	}

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
	strToken.Format("%s", pResult->valuestring);

	nRet = TRUE;

__end:
	if (nRet != TRUE){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "GetTokenFromServer fail");
	}
	
	return nRet;
}

INT CDlgSNCustom::GetBurnDataFromServer(CString& strKeyName)
{
	INT nRet = FALSE;
	int nHttpRet = 0;
	CHttpClient Client;
	CString strURL;
	CString strResponse;
	CString strErrMsg;
	cJSON* pRoot;
	cJSON* pSuccess;
	cJSON* pResult;
	cJSON* pChild;
	CString strSuccess;
	CString strHeader;
	CString strBody;
	CString strResultJson;
	cJSON* pItem;

	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
	
	strHeader.Format("Content-Type: application/json;charset=UTF-8\r\n%s:%s", m_tCfg.strHeaderKeyPara, pCfg->strToken);
	
	strURL.Format("https://%s/api/services/app/AtBind/GetBurnData?MoLotNo=%s&SnCodeNo=%d",m_tCfg.strFactoryURL, pCfg->strMoLotNo, 0);
	nHttpRet = Client.HttpGet(strURL, strHeader, strBody, strResponse);
	m_FileLog.PrintLog(LOGLEVEL_ERR, "QuerySN strURL=%s, strResponse=%s, HttpGet Ret=%d",strURL, strResponse, nHttpRet );
	if (nHttpRet != 0){
		if (nHttpRet==1){
		}else if (nHttpRet==2){
		}
		//http脟毛脟贸鲁卢脢卤
		goto __end;
	}

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
	
	pChild =cJSON_GetArrayItem(pResult, 0);

	strResultJson.Format("%s", pChild->valuestring);
	
	pItem = cJSON_GetObjectItem(pChild,"KeyType");
	if (pItem == NULL){
		goto __end ;
	}
	m_tCurrRetBurnData.strKeyType.Format("%s", pItem->valuestring);

	pItem = cJSON_GetObjectItem(pChild,"HashCode");
	if (pItem == NULL){
		goto __end ;
	}
	m_tCurrRetBurnData.strHashCode.Format("%s", pItem->valuestring);

	pItem = cJSON_GetObjectItem(pChild,"KeyName");
	if (pItem == NULL){
		goto __end ;
	}
	m_tCurrRetBurnData.strKeyName.Format("%s", pItem->valuestring);
	strKeyName.Format("%s", pItem->valuestring);

	pItem = cJSON_GetObjectItem(pChild,"IsFirstMac");
	if (pItem == NULL){
		goto __end ;
	}
	m_tCurrRetBurnData.bIsFirstMac = cJSON_IsFalse(pItem);

	pItem = cJSON_GetObjectItem(pChild,"TagName");
	if (pItem == NULL){
		goto __end ;
	}
	m_tCurrRetBurnData.strTagName.Format("%s", pItem->valuestring);


	pItem = cJSON_GetObjectItem(pChild,"SnCodeNo");
	if (pItem == NULL){
		goto __end ;
	}
	m_tCurrRetBurnData.nSnCodeNo= pItem->valueint;

	nRet = TRUE;

__end:
	return nRet;
}

INT CDlgSNCustom::DownloadKeyFileFromServer(CString strKeyName, CString& strSN,  std::vector<BYTE>& vHex)
{
	INT nRet = FALSE;
	int nHttpRet = 0;
	CHttpClient Client;
	CString strURL;
	CString strResponse;
	CString strErrMsg;
	cJSON* pRoot;
	cJSON* pSuccess;
	cJSON* pResult;
	CString strHeader;
	CString strBody;

	//std::vector<BYTE> vHex;
	vHex.clear();
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	if (strKeyName.IsEmpty()){
		goto __end;
	}

	strHeader.Format("Content-Type: application/json;charset=UTF-8\r\n%s:%s", m_tCfg.strHeaderKeyPara, pCfg->strToken);

	strURL.Format("http://%s/Attachments/DownloadKeyFile?KeyName=%s&ContentType=%d",m_tCfg.strOSSURL, strKeyName, m_tCfg.nContentType);
	nHttpRet = Client.HttpGetRaw(strURL, strHeader, strBody, strResponse, vHex);
	m_FileLog.PrintLog(LOGLEVEL_ERR, "QuerySN strURL=%s, strResponse=%s, HttpGet Ret=%d",strURL, strResponse, nHttpRet );
	if (nHttpRet != 0){
		if (nHttpRet==1){
		}else if (nHttpRet==2){
		}
		//http脟毛脟贸鲁卢脢卤
		goto __end;
	}
	m_FileLog.PrintLog(LOGLEVEL_ERR, "start build data,  ContentType= %d",m_tCfg.nContentType );
	if (m_tCfg.nContentType == 0){ //原始字节流
		pRoot=cJSON_Parse(strResponse.GetBuffer());
		if(pRoot==NULL){ //非json格式的数据则是返回直接数据流	
			m_FileLog.PrintLog(LOGLEVEL_ERR, "is not json format" );
			//strSN.Format("%s", strResponse);
			nRet = TRUE;
		}else{
			pSuccess = cJSON_GetObjectItem(pRoot,"Success");
			if(pSuccess == NULL){
				goto __end;
			}

			if ( cJSON_IsFalse(pSuccess) ){
				strErrMsg.Format("%s",  cJSON_GetObjectItem(pRoot, "Error")->valuestring);
				goto __end ;
			}
		}
	}else{
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
		strSN.Format("%s", strResponse); //字符串的形式
		nRet = TRUE;
	}

__end:
	return nRet;
}


INT CDlgSNCustom::QuerySN(UINT AdapIdx,DWORD Idx,BYTE*pData,INT*pSize)
{
	INT Ret=-1;
	CString strTime;
	CTime Time;
	CSerial lSerial;
	CString strFstCol,strSecCol;
	UINT GroupCnt= 1; //脰禄脫脨1脳茅
	CString strSN;
	CHttpClient Client;
	CString strURL;
	CString strTemp;
	CString strResponse;
	CString strKeyName;

	std::vector<BYTE>vHex;
	vHex.clear();

	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	if (!pCfg->bIsCanConnect){
		goto __end;
	}
	
	if (GetBurnDataFromServer(strKeyName) == FALSE){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "GetBurnDataFromServer fail");
		goto __end;
	}
	
	if (DownloadKeyFileFromServer(strKeyName, strSN, vHex) == FALSE){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "DownloadKeyFileFromServer fail");
		goto __end;
	}

	lSerial<<GroupCnt;
	for(int i = 0; i < GroupCnt; i++){
		lSerial<<pCfg->uSNAddr;
		if (m_tCfg.nContentType == 0){ //原始字节流
			lSerial<<vHex.size();
			for (int j = 0; j < vHex.size(); j++){
				lSerial<<vHex[j];
			}
		}else{ //字符串
			lSerial<<strSN.GetLength();
			lSerial.SerialInBuff((BYTE*)strSN.GetBuffer(),strSN.GetLength());
		}
	}
	
	memcpy(pData,lSerial.GetBuffer(),lSerial.GetLength());
	Ret = lSerial.GetLength();
	strSN.ReleaseBuffer();

__end:

	return Ret;
}

INT CDlgSNCustom::TellResult(DWORD Idx,INT IsPass)
{
	m_FileLog.PrintLog(LOGLEVEL_ERR, "TellResult Idx=%u", Idx);
	m_FileLog.PrintLog(LOGLEVEL_ERR, "TellResult Idx=%d, ret=%d", Idx, IsPass);
	INT Ret=0;
	CString strRet;
	strRet.Format("%d", IsPass);

	int nHttpRet = 0;
	CHttpClient Client;
	CString strURL;
	CString strResponse;
	cJSON* pRoot;
	cJSON* pSuccess;
	cJSON* pResult;
	CString strErrMsg;
	CString strHeader;
	CString strBody;
	CString strBuildJson;
	cJSON* pItem;

	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	if (IsPass == 0){ //失败不上传，客户要求
		goto __end;
	}

	strHeader.Format("Content-Type: application/json;charset=UTF-8\r\n%s:%s", m_tCfg.strHeaderKeyPara, pCfg->strToken);

	cJSON* RootObj=cJSON_CreateObject();
	cJSON_AddStringToObject(RootObj, "MoLotNo",(LPSTR)(LPCSTR)pCfg->strMoLotNo); 
	cJSON_AddStringToObject(RootObj, "BarCode",""); 
	cJSON_AddNumberToObject(RootObj, "BarcodeNo",m_tCurrRetBurnData.nSnCodeNo); 
	pItem = cJSON_AddArrayToObject(RootObj, "KeyTypes");
	cJSON_AddStringToObject(pItem, "", m_tCurrRetBurnData.strKeyType);
	strBuildJson = cJSON_Print(RootObj);
	strBody.Format("%s",strBuildJson);

	strURL.Format("https://%s/api/services/app/AtBind/SnKeysIsComplete", m_tCfg.strOSSURL );

	nHttpRet = Client.HttpPost(strURL, strHeader, strBody, strResponse);
	m_FileLog.PrintLog(LOGLEVEL_ERR, "QuerySN strURL=%s, strResponse=%s, HttpGet Ret=%d",strURL, strResponse, nHttpRet );
	if (nHttpRet != 0){
		if (nHttpRet==1){
		}else if (nHttpRet==2){
		}
		//http脟毛脟贸鲁卢脢卤
		goto __end;
	}

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

__end:
	if (RootObj){
		cJSON_Delete(RootObj);
	}
	
	return Ret;
}

INT CDlgSNCustom::PreLoad()
{
	initOnce();
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
		pPos[0] = 0;//脡戮鲁媒脦脛录镁脙没拢卢脰禄禄帽碌脙脗路戮露
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
	return 0;
}

void CDlgSNCustom::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	
	//DDX_Text(pDX, IDC_CSVFILE, m_strIniFilePath);
}


BEGIN_MESSAGE_MAP(CDlgSNCustom, CDialog)
	//ON_BN_CLICKED(IDC_BTNSELCSV, &CDlgSNCustom::OnBnClickedBtnselini)
	ON_EN_KILLFOCUS(IDC_SN_ADDR , &CDlgSNCustom::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_ACCOUNT, &CDlgSNCustom::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_PASSWORD , &CDlgSNCustom::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_BATCHNO, &CDlgSNCustom::OnEnKillfocusEdit)
END_MESSAGE_MAP()


// CDlgSNCustom 脧没脧垄麓娄脌铆鲁脤脨貌


INT CDlgSNCustom::CheckIsCanConnectToServer()
{
	INT nRet = FALSE;
	INT nHttpRet = 0;
	CString strErrMsg;
	CHttpClient Client;
	CString strURL;
	CString strTemp;
	CString strResponse;
	cJSON* pRoot;
	cJSON* pSuccess;
	cJSON* pResult;
	
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	if (pCfg->strToken.IsEmpty()){
		goto __end;
	}

	strURL.Format("https://%s/OSS/TmpFixs/IsOssCanConnect", m_tCfg.strOSSURL);

	nHttpRet = Client.HttpGet(strURL, "", "", strResponse);
	m_FileLog.PrintLog(LOGLEVEL_ERR, "QuerySN strURL=%s, strResponse=%s, HttpGet Ret=%d",strURL, strResponse, nHttpRet );
	if (nHttpRet != 0){
		if (nHttpRet==1){
		}else if (nHttpRet==2){
		}
		//http脟毛脟贸鲁卢脢卤
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
void CDlgSNCustom::initOnce(){
	m_vReport.clear();

	CreatLog();

	CHAR TmpBuf[512];
	CString strIniFile;
	memset(TmpBuf,0,512);
	strIniFile.Format("%s\\sngen\\SCS_SN_CKD_001.ini",GetCurrentPath());
	if (!PathFileExists(strIniFile)){
		AfxMessageBox("找不到SCS_SN_CKD_001.ini配置文件，请检查!");
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
		AfxMessageBox("目前ContentType只有字节流的，字符串的请联系研发人员确认");
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
//	// TODO:  脭脷麓脣脤铆录脫露卯脥芒碌脛鲁玫脢录禄炉
//	return FALSE;
//	//return TRUE;  // return TRUE unless you set the focus to a control
//	// 脪矛鲁拢: OCX 脢么脨脭脪鲁脫娄路碌禄脴 FALSE
//}



void CDlgSNCustom::OnEnKillfocusEdit()
{
	// TODO: 脭脷麓脣脤铆录脫驴脴录镁脥篓脰陋麓娄脌铆鲁脤脨貌麓煤脗毛
	SaveCtrls();
}
