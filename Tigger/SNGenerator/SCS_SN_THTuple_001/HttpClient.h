#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

//#define USEHTTP_UTF8IN
#include "LogMsg.h"
//#include "LogFile.h"
//#include "ILog.h"

#include <afxinet.h>
#include <string>
using namespace std;

#define  IE_AGENT  _T("Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 2.0.50727)")

// 操作成功
#define SUCCESS        0
// 操作失败
#define FAILURE        1
// 操作超时 www.it165.net
#define OUTTIME        2

class CHttpClient
{
public:
	CHttpClient(LPCTSTR strAgent = IE_AGENT);
	virtual ~CHttpClient(void);

	int HttpGet(LPCTSTR strUrl,LPCTSTR strHeader, LPCTSTR strPostData, CString &strResponse);
	int HttpPost(LPCTSTR strUrl,LPCTSTR strHeader, LPCTSTR strPostData, CString &strResponse);

	void SetLogMsg(CLogMsg*pLogMsg) { m_pLogMsg = pLogMsg; }
	CLogMsg*m_pLogMsg;

private:
	int ExecuteRequest(LPCTSTR strMethod, LPCTSTR strUrl,LPCTSTR strHeader, LPCTSTR strPostData, CString &strResponse);
	void Clear();

private:
	CInternetSession *m_pSession;
	CHttpConnection *m_pConnection;
	CHttpFile *m_pFile;
};

#endif //