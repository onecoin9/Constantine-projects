
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
        delete m_pSession;
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
 
    AfxParseURL(strUrl, dwServiceType, strServer, strObject, nPort);
 
    if(AFX_INET_SERVICE_HTTP != dwServiceType && AFX_INET_SERVICE_HTTPS != dwServiceType)
    {
       Ret=FAILURE; goto __end;
    }
 
    try
    {
		m_pSession->SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 15);
		m_pSession->SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, 15);
		m_pSession->SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 15);
		///////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////

        m_pConnection = m_pSession->GetHttpConnection(strServer,
            dwServiceType == AFX_INET_SERVICE_HTTP ? NORMAL_CONNECT : SECURE_CONNECT,
            nPort);

		InternetSetOption(m_pSession, INTERNET_OPTION_RESET_URLCACHE_SESSION, NULL, 0);


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

		if(m_pFile->SendRequest(NULL, 0,(LPVOID)(LPCTSTR)strPostData, strPostData == NULL ? 0 : _tcslen(strPostData))==FALSE){
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