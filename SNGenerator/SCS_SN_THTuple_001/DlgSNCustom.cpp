// DlgSNCustom.cpp : 实现文件
//

#include "stdafx.h"
#include "SCS_SN_THTuple_001.h"
#include "DlgSNCustom.h"
#include "HttpClient.h"
#include "../Com/ComTool.h"
#include "../Com/cJSON.h"
#include "../Com/ComFunc.h"


// CDlgSNCustom 对话框

IMPLEMENT_DYNAMIC(CDlgSNCustom, CDialog)

CDlgSNCustom::CDlgSNCustom(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSNCustom::IDD, pParent)
	, m_pSnCfgPara(pSnCfgPara)
{
	//tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
	//pCfg->DeviceIDAddr=0x1FF100;
	//pCfg->DeviceIDLen=17;
	//pCfg->SecretAddr=0x1FF000;
	//pCfg->SecretLen=64;
	//pCfg->strCSVFile="";
}

CDlgSNCustom::~CDlgSNCustom()
{
}

BOOL CDlgSNCustom::InitCtrlsValue(CSNCustomCfg& SNCfg)
{
	tSNCustomCfg* pCfg=SNCfg.GetCustomCfg();
	//CString strText;
	//strText.Format("%I64X",pCfg->DeviceIDAddr);
	//GetDlgItem(IDC_DEVICEID_ADDR)->SetWindowText(strText);
	//strText.Format("%d",pCfg->DeviceIDLen);
	//GetDlgItem(IDC_DEVICEID_LEN)->SetWindowText(strText);

	//strText.Format("%I64X",pCfg->SecretAddr);
	//GetDlgItem(IDC_SECRET_ADDR)->SetWindowText(strText);
	//strText.Format("%d",pCfg->SecretLen);
	//GetDlgItem(IDC_SECRET_LEN)->SetWindowText(strText);

	//m_strCSVFile=pCfg->strCSVFile;
	UpdateData(FALSE);
	return TRUE;
}

BOOL CDlgSNCustom::InitCtrls(CSerial& lSerial)
{
	BOOL Ret=TRUE;
	if (true){
		return TRUE;
	}
	CString strErrMsg;
	if(lSerial.GetLength()!=0){///再次打开对话框的时候就会传入，但之前会因为资源被释放，所以需要重新打开文件
		Ret=m_SnCfg.SerialInCfgData(lSerial);
		if(Ret==TRUE){
			tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
			/*m_CSVFile.CloseFile();
			if(m_CSVFile.OpenFile(pCfg->strCSVFile)!=0){
				strErrMsg=m_CSVFile.GetErrMsg();
				MessageBox(strErrMsg,"",MB_OK);
			}*/
		}
	}
	else{///首次加载的时候lSerial会没有值
	}
	//Ret=InitCtrlsValue(m_SnCfg);
	return Ret;
}

BOOL CDlgSNCustom::SaveCtrls()
{
	//UpdateData(TRUE);
	CString strText;
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
	/*GetDlgItem(IDC_DEVICEID_ADDR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->DeviceIDAddr)!=1){
		MessageBox("获取DeviceID起始地址错误,请确认");
		return FALSE;
	}*/

	return TRUE;
}

BOOL CDlgSNCustom::GetCtrls(CSerial&lSerial)
{
	BOOL Ret=TRUE;
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
	return TRUE;
	// return m_SnCfg.SerialOutCfgData(lSerial);
}

BOOL CDlgSNCustom::InitSocket()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD( 1, 1 );
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		return FALSE;
	}
	if ( LOBYTE( wsaData.wVersion ) != 1 ||
		HIBYTE( wsaData.wVersion ) != 1 ) {
			//WSACleanup( );
			return FALSE; 
	}
	return TRUE;
}

BOOL CDlgSNCustom::ConnectAndSend(CString strSendData)
{
	BOOL Ret=FALSE;
	CString strIP;

	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock==INVALID_SOCKET){
		return FALSE;
	}

	strIP.Format("%s", m_tCfg.UploadIPAddr);
	UINT nPort = m_tCfg.UploadIPPort;
	int TryCnt = 2;
	struct sockaddr_in serv_addr;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(strIP);
	serv_addr.sin_port = htons(nPort);

	unsigned long ul = 1;
	while(TryCnt>0){
		if( connect(m_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != -1){
			Ret = TRUE;
			break;
		}

		Sleep(300);
		TryCnt--;
	}
	
	m_FileLog.PrintLog(LOGLEVEL_ERR, "SEND strSendData=%s, Ret=%d", strSendData, Ret);
	if (Ret == TRUE){
		send(m_sock,/*(LPCTSTR)*/ (LPSTR)(LPCSTR)strSendData, strSendData.GetLength(), 0);
	}

	closesocket(m_sock);

	return Ret;
}


INT CDlgSNCustom::PostProgramRetToServer(CString strRet, DWORD Idx){
	INT Ret=0;
	bool bPass = false;
	if (strRet.CompareNoCase("1") == 0){
		bPass = true;
	}

	GetJsonFile( bPass, Idx);

	if (InitSocket() == FALSE){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "InitSocket failed");
		goto __end;
	}
	
	if (ConnectAndSend(m_strJson) == TRUE){
		Ret = 1;
	}
	
__end:

	if (Ret != 1){//保存json文件
		SaveFailReportFile();
	}

	return Ret;
}

void CDlgSNCustom::GetJsonFile(bool bRet, DWORD Idx){
	m_strJson = "";
	cJSON * Root = NULL;
	cJSON * RootCommunicate = NULL;
	CString strValue;
	strValue.Format("%s", "NG");
	if (bRet){
		strValue.Format("%s", "OK");
	}
	CTime Time;
	tagReport item;
	item.index = -1;
	item.strMac = "";
	item.strRet = "";
	item.strStartTime ="";
	item.nTime = 0;
	int index = Idx -1;
	
	if (Idx <= m_vReport.size()){
		if (m_vReport[index].index == Idx){
			item.index = m_vReport[index].index;
			item.strMac.Format("%s", m_vReport[index].strMac);
			item.strRet.Format("%s", strValue);
			item.strStartTime.Format("%s", m_vReport[index].strStartTime) ;
			item.nTime = m_vReport[index].nTime;
		}	
	}

	Time=CTime::GetCurrentTime();
	strValue.Format("%d-%02d-%02d %02d:%02d:%02d", 
		Time.GetYear(),Time.GetMonth(),Time.GetDay(),
		Time.GetHour(),Time.GetMinute(),Time.GetSecond());
	Root=cJSON_CreateObject();
	cJSON_AddStringToObject(Root, "_ZGDH",(LPSTR)(LPCSTR)m_tCfg.WorkOrderID); 
	cJSON_AddStringToObject(Root, "_StartTime",(LPSTR)(LPCSTR)item.strStartTime); 
	//cJSON_AddStringToObject(Root, "EndTime",(LPSTR)(LPCSTR)strValue); 	

	cJSON_AddStringToObject(Root, "_MATNR",(LPSTR)(LPCSTR)m_tCfg._MATNR); 
	cJSON_AddStringToObject(Root, "_TYPE1",(LPSTR)(LPCSTR)m_tCfg._TYPE1); 
	cJSON_AddStringToObject(Root, "_DOTYPE",(LPSTR)(LPCSTR)m_tCfg._DOTYPE); 	
	cJSON_AddStringToObject(Root, "_USERID",(LPSTR)(LPCSTR)m_tCfg._USERID); 
	cJSON_AddStringToObject(Root, "_FILEVERSION",(LPSTR)(LPCSTR)m_tCfg._FILEVERSION); 
	cJSON_AddStringToObject(Root, "_RemoteUpdate",(LPSTR)(LPCSTR)m_tCfg._RemoteUpdate); 	
	cJSON_AddStringToObject(Root, "Datecode",(LPSTR)(LPCSTR)m_tCfg.Datecode); 
	cJSON_AddStringToObject(Root, "KehuName",(LPSTR)(LPCSTR)m_tCfg.KehuName); 
	cJSON_AddStringToObject(Root, "ModelID",(LPSTR)(LPCSTR)m_tCfg.ModelID); 	
	cJSON_AddStringToObject(Root, "Version",(LPSTR)(LPCSTR)m_tCfg.Version); 
	cJSON_AddStringToObject(Root, "FwId",(LPSTR)(LPCSTR)m_tCfg.FwId); 
	cJSON_AddStringToObject(Root, "DeviceInfo",(LPSTR)(LPCSTR)m_tCfg.DeviceInfo);
	cJSON_AddStringToObject(Root, "Token",(LPSTR)(LPCSTR)m_tCfg.Token); 
	cJSON_AddStringToObject(Root, "KeyType",(LPSTR)(LPCSTR)m_tCfg.KeyType); 	
	cJSON_AddStringToObject(Root, "TestLightcount",(LPSTR)(LPCSTR)m_tCfg.TestLightcount); 
	cJSON_AddStringToObject(Root, "_MultiLightIndex",(LPSTR)(LPCSTR)m_tCfg._MultiLightIndex); 
	cJSON_AddStringToObject(Root, "_TestLightIndex",(LPSTR)(LPCSTR)m_tCfg._TestLightIndex); 
	cJSON_AddStringToObject(Root, "PC_MAC",(LPSTR)(LPCSTR)m_tCfg.PC_MAC); 	

	RootCommunicate=cJSON_CreateObject();
	cJSON_AddItemToObject(Root, "Communicate", RootCommunicate);
	
	time_t endTime = time(NULL);
	time_t nCostTime = endTime - item.nTime;
	strValue.Format("%llu", nCostTime);
	cJSON_AddStringToObject(Root, "_TestTUsetime",(LPSTR)(LPCSTR)strValue); 	

	cJSON *pArr=cJSON_AddArrayToObject(Root, "TestResultList");

	cJSON *element = cJSON_CreateObject();
	cJSON_AddStringToObject(element, "_MAC",(LPSTR)(LPCSTR)item.strMac);
	cJSON_AddStringToObject(element, "Result",(LPSTR)(LPCSTR)item.strRet);
	cJSON_AddStringToObject(element, "FwResult",(LPSTR)(LPCSTR)item.strRet); 

	cJSON_AddItemToArray(pArr, element);
	m_strJson = cJSON_Print(Root);		

	m_FileLog.PrintLog(LOGLEVEL_ERR, "GetJsonFile =%s, Idx=%d", m_strJson, Idx);
}

void CDlgSNCustom::SaveFailReportFile(){
	CString strFilePath;
	CTime Time;
	CString strCurPath=GetCurrentPath();
	Time=CTime::GetCurrentTime();
	strFilePath.Format("%s\\%s_%d%02d%02d_%02d%02d%02d.json",
		m_tCfg.CachePath, m_tCfg.MACSNAddr,
		Time.GetYear(),Time.GetMonth(),Time.GetDay(),
		Time.GetHour(),Time.GetMinute(),Time.GetSecond() );

	CFile File;
	if(File.Open(strFilePath,CFile::modeCreate|CFile::modeWrite,NULL)==TRUE){
		File.Write(m_strJson, m_strJson.GetLength());
		File.Close();	
	}

}

INT CDlgSNCustom::QuerySN(UINT AdapIdx,DWORD Idx,BYTE*pData,INT*pSize)
{
	INT Ret=0;
	CSerial lSerial;
	UINT GroupCnt= 1; //只有1组
	CString strSN;
	CString strCurPath;
	
	char path[1024] = {0};
	int nTempLen = 1024;
	UCHAR* reg = new UCHAR[nTempLen];
	if (reg == NULL ){
		goto __end; 
	}
	memset(reg, 0, nTempLen);
	char TupleOut[1024] = {};
	int len = 512;

	strCurPath=CComFunc::GetCurrentPath();
	strCurPath +="\\lib\\Device_TH.ini";
	memcpy(path, strCurPath, strCurPath.GetLength());
	
	if (m_DllHelp.m_bLoadDll == true){
		m_DllHelp.GetTuple_Zigbee(path, reg, TupleOut, len);

		if (*reg != 1){
			AfxMessageBox("从绿米ERP服务器获取元祖数据失败");
			goto __end; 
		}

		
		strSN.Format("%s", TupleOut); //获取到的SN号

		//for test
		m_FileLog.PrintLog(LOGLEVEL_ERR, "%s", strSN);
		//strSN.Format("%s", "04CF8CDF3C737625");


		lSerial<<GroupCnt;
		for(int i = 0; i < GroupCnt; i++){
			lSerial<<m_tCfg.MACSNAddr;
			lSerial<<strSN.GetLength()/*m_tCfg.MACSNLen*/; //工厂回复是基于SN长度来的。

			lSerial.SerialInBuff((BYTE*)strSN.GetBuffer(),strSN.GetLength());
		}

		memcpy(pData, lSerial.GetBuffer(), lSerial.GetLength());
		Ret = lSerial.GetLength();
		strSN.ReleaseBuffer();
	}

	/*
	if (strSN.GetLength() != m_tCfg.MACSNLen){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "length is error");
	}*/

__end:

	if (reg){
		delete [] reg;
		reg = NULL;
	}

	return Ret;
}

INT CDlgSNCustom::TellResult(DWORD Idx,INT IsPass)
{
	m_FileLog.PrintLog(LOGLEVEL_ERR, "TellResult Idx=%u", Idx);
	m_FileLog.PrintLog(LOGLEVEL_ERR, "TellResult Idx=%d, ret=%d", Idx, IsPass);
	INT Ret=0;
	CString strRet;
	strRet.Format("%d", IsPass);
	//PostProgramRetToServer(strRet, Idx);
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
	
	DDX_Text(pDX, IDC_CSVFILE, m_strIniFilePath);
}


BEGIN_MESSAGE_MAP(CDlgSNCustom, CDialog)
	ON_BN_CLICKED(IDC_BTNSELCSV, &CDlgSNCustom::OnBnClickedBtnselini)
	/*ON_EN_KILLFOCUS(IDC_DEVICEID_ADDR, &CDlgSNCustom::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_DEVICEID_LEN, &CDlgSNCustom::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_SECRET_ADDR, &CDlgSNCustom::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_SECRET_LEN, &CDlgSNCustom::OnEnKillfocusEdit)*/
END_MESSAGE_MAP()


// CDlgSNCustom 消息处理程序

void CDlgSNCustom::initOnce(){
	CreatLog();

	CHAR TmpBuf[512];
	CString strIniFile;
	memset(TmpBuf,0,512);
	strIniFile.Format("%s\\sngen\\SCS_SN_THTuple_001.ini",GetCurrentPath());
	if (!PathFileExists(strIniFile)){
		AfxMessageBox("找不到SCS_SN_THTuple_001.ini配置文件，请检查!");
		return ;
	}

	CString strTemp;
	GetPrivateProfileString("Config", "SNAddr", "",TmpBuf, MAX_PATH, strIniFile);
	strTemp.Format("%s", TmpBuf);
	sscanf(strTemp, "%I64X", &m_tCfg.MACSNAddr);

	m_tCfg.MACSNLen = GetPrivateProfileInt("Config", "SNLen ",16, strIniFile);

	BOOL bRet  = m_DllHelp.AttachDll();

	if (bRet == TRUE){
		if (m_DllHelp.m_bLoadDll == false){
			m_FileLog.PrintLog(LOGLEVEL_ERR, "Tuple_TuoHeng.dll调用RegisterID接口失败 "); ////注册失败
		}
	}

	
}
BOOL CDlgSNCustom::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	
	// TODO:  在此添加额外的初始化
	return FALSE;
	//return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CDlgSNCustom::OnBnClickedBtnselini()
{
	CFileDialog dlgFile(TRUE, NULL, NULL, OFN_OVERWRITEPROMPT, "ini File(*.ini)|*.ini||");
	if (dlgFile.DoModal() != IDOK){
		return;
	}
	m_strIniFilePath=dlgFile.GetPathName();
	
	UpdateData(FALSE);
}

void CDlgSNCustom::OnEnKillfocusEdit()
{
	// TODO: 在此添加控件通知处理程序代码
	SaveCtrls();
}
