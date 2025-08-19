
////////////////////////////////// HttpClient.cpp
#include "StdAfx.h"
#include "HttpClient.h"
#include "../Com/ComTool.h"
 
#define  BUFFER_SIZE       1024
 
#define  NORMAL_CONNECT             INTERNET_FLAG_KEEP_CONNECTION
#define  SECURE_CONNECT                NORMAL_CONNECT | INTERNET_FLAG_SECURE
#define  NORMAL_REQUEST             INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE 
#define  SECURE_REQUEST             NORMAL_REQUEST | INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID

#define PrintLog(_Level,fmt,...) \
	if(m_pLogMsg){\
	m_pLogMsg->PrintLog(_Level,fmt,__VA_ARGS__);\
	}
 
CHttpClient::CHttpClient(LPCTSTR strAgent)
{
    m_pSession = new CInternetSession(strAgent);
    m_pConnection = NULL;
    m_pFile = NULL;
}
 
 
CHttpClient::~CHttpClient(void)
{
    Clear();
    if(NULL != m_pSession)
    {
        m_pSession->Close();
#ifdef _DEBUG
#else
		delete m_pSession;
#endif
       
        m_pSession = NULL;
    }
}
 
void CHttpClient::Clear()
{
    if(NULL != m_pFile)
    {
        m_pFile->Close();
        delete m_pFile;
        m_pFile = NULL;
    }
 
    if(NULL != m_pConnection)
    {
        m_pConnection->Close();
        delete m_pConnection;
        m_pConnection = NULL;
    }
}

#ifdef USEHTTP_UTF8IN
/****
返回0表示成功，返回1表示失败，返回2表示超时
***********/
int CHttpClient::ExecuteRequest(LPCTSTR strMethod, LPCTSTR strUrl,LPCTSTR strHeader, LPCTSTR strPostData, CString &strResponse)
{
    CString strServer;
    CString strObject;
    DWORD dwServiceType;
    INTERNET_PORT nPort;
    strResponse = "";
	DWORD UTF8DataSize=4096,DataSizeUsed;
	INT Ret=SUCCESS;
	BYTE *pPostDataUTF8=NULL;
	pPostDataUTF8=new BYTE[UTF8DataSize];
	if(pPostDataUTF8==NULL){
		Ret=FAILURE; goto __end;
	}
 
    AfxParseURL(strUrl, dwServiceType, strServer, strObject, nPort);
 
    if(AFX_INET_SERVICE_HTTP != dwServiceType && AFX_INET_SERVICE_HTTPS != dwServiceType)
    {
       Ret=FAILURE; goto __end;
    }
 
    try
    {
        m_pConnection = m_pSession->GetHttpConnection(strServer,
            dwServiceType == AFX_INET_SERVICE_HTTP ? NORMAL_CONNECT : SECURE_CONNECT,
            nPort);
        m_pFile = m_pConnection->OpenRequest(strMethod, strObject, 
            NULL, 1, NULL, NULL, 
            (dwServiceType == AFX_INET_SERVICE_HTTP ? NORMAL_REQUEST : SECURE_REQUEST));
 
		if(strHeader==""){
			m_pFile->AddRequestHeaders("Accept: *,*/*");
			m_pFile->AddRequestHeaders("Accept-Language: zh-cn");
			m_pFile->AddRequestHeaders("Content-Type: application/x-www-form-urlencoded");
			m_pFile->AddRequestHeaders("Accept-Encoding: gzip, deflate");
		}
		else{
			m_pFile->AddRequestHeaders(strHeader);
		}

		if(ComTool::MByteToUtf8(strPostData,(LPSTR)pPostDataUTF8,UTF8DataSize,DataSizeUsed)!=1){
			Ret=FAILURE; goto __end;
		}
	
		if(m_pFile->SendRequest(NULL, 0, (LPVOID)(LPCTSTR)pPostDataUTF8,DataSizeUsed)==FALSE){
			Ret=FAILURE; goto __end;
		}
 
        char szChars[BUFFER_SIZE + 1] = {0};
        string strRawResponse = "";
        UINT nReaded = 0;
        while ((nReaded = m_pFile->Read((void*)szChars, BUFFER_SIZE)) > 0)
        {
            szChars[nReaded] = '\0';
            strRawResponse += szChars;
            memset(szChars, 0, BUFFER_SIZE + 1);
        }
 
        int unicodeLen = MultiByteToWideChar(CP_UTF8, 0, strRawResponse.c_str(), -1, NULL, 0);
        WCHAR *pUnicode = new WCHAR[unicodeLen + 1];
        memset(pUnicode,0,(unicodeLen+1)*sizeof(wchar_t));
 
        MultiByteToWideChar(CP_UTF8,0,strRawResponse.c_str(),-1, pUnicode,unicodeLen);
        CString cs(pUnicode);
        delete []pUnicode; 
        pUnicode = NULL;
 
        strResponse = cs;
 
        Clear();
    }
    catch (CInternetException* e)
    {
        Clear();
        DWORD dwErrorCode = e->m_dwError;
        e->Delete();
 
        DWORD dwError = GetLastError();
 
        //PRINT_LOG("dwError = %d", dwError, 0);
 
        if (ERROR_INTERNET_TIMEOUT == dwErrorCode)
        {
            Ret=OUTTIME; goto __end;
        }
        else
        {
            Ret=FAILURE; goto __end;
        }
    }

__end:
	if(pPostDataUTF8){
		delete[] pPostDataUTF8;
	}
    return Ret;
}

#else

/****
返回0表示成功，返回1表示失败，返回2表示超时
***********/
int CHttpClient::ExecuteRequest(LPCTSTR strMethod, LPCTSTR strUrl,LPCTSTR strHeader, LPCTSTR strPostData, CString &strResponse)
{
    CString strServer;
    CString strObject;
    DWORD dwServiceType;
    INTERNET_PORT nPort;
    strResponse = "";
	INT Ret=SUCCESS;
	DWORD dwFlags;

	DWORD  dwHttpConnectMax = 20;

	CString strHttpStatusResponse;


    AfxParseURL(strUrl, dwServiceType, strServer, strObject, nPort);
 
    if(AFX_INET_SERVICE_HTTP != dwServiceType && AFX_INET_SERVICE_HTTPS != dwServiceType)
    {
		PrintLog(0, "ExecuteRequest, run AfxParseURL fail");
       Ret=FAILURE; goto __end;
    }
 
    try
    {
		m_pSession->SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 15);
		m_pSession->SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, 15);
		m_pSession->SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 15);

		/*m_pSession->SetOption(INTERNET_OPTION_MAX_CONNS_PER_SERVER,&dwHttpConnectMax, sizeof(DWORD));
		m_pSession->SetOption(INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER,&dwHttpConnectMax, sizeof(DWORD));*/
		///////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////

        m_pConnection = m_pSession->GetHttpConnection(strServer,
            dwServiceType == AFX_INET_SERVICE_HTTP ? NORMAL_CONNECT : SECURE_CONNECT,
            nPort);

		InternetSetOption(m_pSession, INTERNET_OPTION_RESET_URLCACHE_SESSION, NULL, 0);

		//DWORD dwData = 0;
		//DWORD dwData2 = 0;
		//DWORD dwSize = sizeof(dwData);

		//InternetQueryOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_SERVER, &dwData, &dwSize);
		//InternetQueryOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER, &dwData2, &dwSize);
		//PrintLog(0, "ExecuteRequest,Before Set1.0, cnt=%lu,  cnt2=%lu",dwData, dwData2 );
		//
		//////////////////////////进行设置、、、、、、、、、、、、、、、、、
		//InternetSetOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_SERVER,&dwHttpConnectMax, sizeof(DWORD));
		//InternetSetOption(NULL,INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER,&dwHttpConnectMax,sizeof(DWORD));
		//////////////////////////、、、、、、、、、、、、、、、、、、、、

		//dwData = 0;
		//dwData2 = 0;
		//InternetQueryOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_SERVER, &dwData, &dwSize);
		//InternetQueryOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER, &dwData2, &dwSize);
		//PrintLog(0, "ExecuteRequest, Aftern Set1.0, cnt=%lu,  cnt2=%lu",dwData, dwData2 );
		if (m_pConnection == NULL){
			PrintLog(0, "ExecuteRequest, m_pConnection is Null");
			Ret=FAILURE; goto __end;
		}


        m_pFile = m_pConnection->OpenRequest(strMethod, strObject, 
			NULL, 1, NULL, NULL, INTERNET_FLAG_SECURE |
			INTERNET_FLAG_EXISTING_CONNECT |
			INTERNET_FLAG_RELOAD |
			INTERNET_FLAG_NO_CACHE_WRITE |
			INTERNET_FLAG_IGNORE_CERT_DATE_INVALID |
			INTERNET_FLAG_IGNORE_CERT_CN_INVALID
            /*(dwServiceType == AFX_INET_SERVICE_HTTP ? NORMAL_REQUEST|INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_IGNORE_CERT_CN_INVALID : SECURE_REQUEST|INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_IGNORE_CERT_CN_INVALID)*/);

		
		if (m_pFile == NULL){
			PrintLog(0, "ExecuteRequest, m_pFile is Null");
			Ret=FAILURE; goto __end;
		}
		
		
		m_pFile->QueryOption(INTERNET_OPTION_SECURITY_FLAGS, dwFlags);
		dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
		//set web server option
		m_pFile->SetOption(INTERNET_OPTION_SECURITY_FLAGS, dwFlags);

 
		if(strHeader==""){
			m_pFile->AddRequestHeaders("Accept: *,*/*");
			m_pFile->AddRequestHeaders("Accept-Language: zh-cn");
			m_pFile->AddRequestHeaders("Content-Type: application/x-www-form-urlencoded");
			m_pFile->AddRequestHeaders("Accept-Encoding: gzip, deflate");
		}
		else{
			m_pFile->AddRequestHeaders(strHeader);
		}

		if(m_pFile->SendRequest(NULL, 0,(LPVOID)(LPCTSTR)strPostData, strPostData == NULL ? 0 : _tcslen(strPostData))==FALSE){
			PrintLog(0, "ExecuteRequest, run SendRequest fail");
			Ret=FAILURE; goto __end;
		}
		///////////////////////////////
		//m_pFile->EndRequest();
		DWORD dwHttpCode = 0;
		if ( m_pFile->QueryInfoStatusCode(dwHttpCode) == FALSE){
			PrintLog(0, "ExecuteRequest, run QueryInfoStatusCode fail, dwHttpCode=%lu", dwHttpCode);
		}

		// 出错的原因
		if ((dwHttpCode < 200) || (dwHttpCode >= 300)){
			//m_pFile->QueryInfo(HTTP_QUERY_STATUS_TEXT, strHttpStatusResponse);
			PrintLog(0, "ExecuteRequest, http response error , dwHttpCode=%d,  %s", dwHttpCode, strHttpStatusResponse);
			if ( dwHttpCode == 0){
				PrintLog(0, "ExecuteRequest, error code , %d", GetLastError());
			}
		}
		///////////////////////////////
		PrintLog(0, "ExecuteRequest, run start read");
 
        char szChars[BUFFER_SIZE + 1] = {0};
        string strRawResponse = "";
        UINT nReaded = 0;

        while ((nReaded = m_pFile->Read((void*)szChars, BUFFER_SIZE)) > 0)
        {
            szChars[nReaded] = '\0';
            strRawResponse += szChars;
            memset(szChars, 0, BUFFER_SIZE + 1);
        }

        int unicodeLen = MultiByteToWideChar(CP_UTF8, 0, strRawResponse.c_str(), -1, NULL, 0);
        WCHAR *pUnicode = new WCHAR[unicodeLen + 1];
		if (pUnicode == NULL)
		{
			PrintLog(0, "ExecuteRequest, run new failed");
		}
        memset(pUnicode,0,(unicodeLen+1)*sizeof(wchar_t));
 
        MultiByteToWideChar(CP_UTF8,0,strRawResponse.c_str(),-1, pUnicode,unicodeLen);
        CString cs(pUnicode);
        delete []pUnicode; 
        pUnicode = NULL;
 
        strResponse = cs;

		PrintLog(0, "ExecuteRequest, run end read");
 
        Clear();
		PrintLog(0, "ExecuteRequest,free end");
    }
    catch (CInternetException* e)
    {
        Clear();
        DWORD dwErrorCode = e->m_dwError;
        e->Delete();
 
        DWORD dwError = GetLastError();
 
        PrintLog(0, "dwError = %d", dwErrorCode);
 
        if (ERROR_INTERNET_TIMEOUT == dwErrorCode)
        {
			PrintLog(0, "ERROR_INTERNET_TIMEOUT");
            Ret=OUTTIME; goto __end;
        }
        else
        {
			PrintLog(0, "执行时发生错误，抛出异常");
            Ret=FAILURE; goto __end;
        }
    }

__end:
	
    return Ret;
}
#endif 
 
int CHttpClient::HttpGet(LPCTSTR strUrl,LPCTSTR strHeader, LPCTSTR strPostData, CString &strResponse)
{
    return ExecuteRequest("GET", strUrl,strHeader, strPostData, strResponse);
}
 
int CHttpClient::HttpPost(LPCTSTR strUrl,LPCTSTR strHeader, LPCTSTR strPostData, CString &strResponse)
{
    return ExecuteRequest("POST", strUrl,strHeader, strPostData, strResponse);
}

int CHttpClient::HttpGetRaw(LPCTSTR strUrl,LPCTSTR strHeader, LPCTSTR strPostData, CString &strResponse, std::vector<BYTE>& vOut)
{
	CString strServer;
    CString strObject;
    DWORD dwServiceType;
    INTERNET_PORT nPort;
	CString strMethod;
	strMethod.Format("GET");
	vOut.clear();
	INT Ret=SUCCESS;

	DWORD  dwHttpConnectMax = 20;

	CString strQueryHeader;
	CString strContent_Length;
	int nPos = 0;
	DWORD dwStatusCode = 0;
	ULONGLONG RawLen = 0;
	CString strHttpStatusResponse;
 
    AfxParseURL(strUrl, dwServiceType, strServer, strObject, nPort);
 
    if(AFX_INET_SERVICE_HTTP != dwServiceType && AFX_INET_SERVICE_HTTPS != dwServiceType)
    {
		PrintLog(0, "HttpGetRaw, Run AfxParseURL fail");
       Ret=FAILURE; goto __end;
    }
 
    try
    {
		m_pSession->SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 15);
		m_pSession->SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, 15);
		m_pSession->SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 15);
		/*m_pSession->SetOption(INTERNET_OPTION_MAX_CONNS_PER_SERVER,&dwHttpConnectMax, sizeof(DWORD));
		m_pSession->SetOption(INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER,&dwHttpConnectMax, sizeof(DWORD));*/
		///////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////

        m_pConnection = m_pSession->GetHttpConnection(strServer,
            dwServiceType == AFX_INET_SERVICE_HTTP ? NORMAL_CONNECT : SECURE_CONNECT,
            nPort);

		if (m_pConnection == NULL){
			PrintLog(0, "raw, m_pConnection is Null");
			Ret=FAILURE; goto __end;
		}

		InternetSetOption(m_pSession, INTERNET_OPTION_RESET_URLCACHE_SESSION, NULL, 0);

		///////////////////////////////////
		//DWORD dwData = 0;
		//DWORD dwData2 = 0;
		//DWORD dwSize = sizeof(dwData);

		//InternetQueryOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_SERVER, &dwData, &dwSize);
		//InternetQueryOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER, &dwData2, &dwSize);
		//PrintLog(0, "HttpGetRaw,Before Set1.0, cnt=%lu,  cnt2=%lu",dwData, dwData2 );

		//////////////////////////////////进行设置/////////////
		//InternetSetOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_SERVER,&dwHttpConnectMax, sizeof(DWORD));
		//InternetSetOption(NULL,INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER,&dwHttpConnectMax,sizeof(DWORD));
		///////////////////////////////////////////////////////
		//
		//dwData = 0;
		//dwData2 = 0;
		//InternetQueryOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_SERVER, &dwData, &dwSize);
		//InternetQueryOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER, &dwData2, &dwSize);
		//PrintLog(0, "HttpGetRaw,Aftern Set1.0, cnt=%lu,  cnt2=%lu",dwData, dwData2 );
		
		


        m_pFile = m_pConnection->OpenRequest(strMethod, strObject, 
            NULL, 1, NULL, NULL, 
            (dwServiceType == AFX_INET_SERVICE_HTTP ? NORMAL_REQUEST : SECURE_REQUEST));

		if (m_pFile == NULL){
			PrintLog(0, "raw, m_pFile is Null");
			Ret=FAILURE; goto __end;
		}
 
		if(strHeader==""){
			m_pFile->AddRequestHeaders("Accept: *,*/*");
			m_pFile->AddRequestHeaders("Accept-Language: zh-cn");
			m_pFile->AddRequestHeaders("Content-Type: application/x-www-form-urlencoded");
			m_pFile->AddRequestHeaders("Accept-Encoding: gzip, deflate");
		}
		else{
			m_pFile->AddRequestHeaders(strHeader);
		}
		
		if(m_pFile->SendRequest(NULL, 0,(LPVOID)(LPCTSTR)strPostData, strPostData == NULL ? 0 : _tcslen(strPostData))==FALSE){
			PrintLog(0, "HttpGetRaw, Run SendRequest fail");
			Ret=FAILURE; goto __end;
		}
 
        char szChars[BUFFER_SIZE + 1] = {0};
        string strRawResponse = "";
        UINT nReaded = 0;

		/*m_pFile->QueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF, strQueryHeader);
		nPos = strQueryHeader.Find("Content-Length:");
		for (int j = nPos +15; j < strQueryHeader.GetLength(); j++){
			if (strQueryHeader.GetAt(j) !='\n'){
				strContent_Length += strQueryHeader.GetAt(j);
				strContent_Length.Trim();
			} else{
				break;
			}
		}*/
		/*m_pFile->QueryInfoStatusCode(dwStatusCode);
		if (dwStatusCode == HTTP_STATUS_OK){
			RawLen = _ttoi(strContent_Length);
		}else{
			PrintLog(0, "HttpGetRaw, return dwHttpCode =%lu",  dwStatusCode);
		}
		if ((dwStatusCode < 200) || (dwStatusCode >= 300)){
			m_pFile->QueryInfo(HTTP_QUERY_STATUS_TEXT, strHttpStatusResponse);
			PrintLog(0, "HttpGetRaw, response error , %s", strHttpStatusResponse);
			if ( dwStatusCode == 0){
				PrintLog(0, "HttpGetRaw, error code , %d", GetLastError());
			}
		}*/
		 PrintLog(0, "Raw start read");

        while ((nReaded = m_pFile->Read((void*)szChars, BUFFER_SIZE)) > 0)
        {
            szChars[nReaded] = '\0';
            strRawResponse += szChars;
			for (int i = 0; i < nReaded; i ++){
				vOut.push_back(szChars[i]);
			}
            memset(szChars, 0, BUFFER_SIZE + 1);
        }

        int unicodeLen = MultiByteToWideChar(CP_UTF8, 0, strRawResponse.c_str(), -1, NULL, 0);
        WCHAR *pUnicode = new WCHAR[unicodeLen + 1];
		if (pUnicode == NULL)
		{
			PrintLog(0, "分配失败");
		}
        memset(pUnicode,0,(unicodeLen+1)*sizeof(wchar_t));
 
        MultiByteToWideChar(CP_UTF8,0,strRawResponse.c_str(),-1, pUnicode,unicodeLen);
        CString cs(pUnicode);
        delete []pUnicode; 
        pUnicode = NULL;

		strResponse = cs;

		PrintLog(0, "Raw end read");
 
        Clear();
    }
    catch (CInternetException* e)
    {
        Clear();
        DWORD dwErrorCode = e->m_dwError;
        e->Delete();
 
        DWORD dwError = GetLastError();
 
        PrintLog(0, "Raw dwError = %d", dwError);
 
        if (ERROR_INTERNET_TIMEOUT == dwErrorCode)
        {
			PrintLog(0, " raw ERROR_INTERNET_TIMEOUT error" );
			
            Ret=OUTTIME; goto __end;
        }
        else
        {
			PrintLog(0, "raw 异常抛出" );
			
            Ret=FAILURE; goto __end;
        }
    }

__end:
	
    return Ret;
}