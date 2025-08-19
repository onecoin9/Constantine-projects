#pragma once

#include <afxmt.h>
class CHtmlLogWriter:public CFile
{
public:
	enum{
		LOGTYPE_NORMAL,
		LOGTYPE_REPORT,
	};
	CHtmlLogWriter();
	BOOL CreateLog(CString strLogFile,INT LogType=LOGTYPE_NORMAL);
	BOOL CloseLog();
	BOOL InsertLog(INT LogLevel,const char *fmt,...);

	BOOL ConfigDumpBegin();
	BOOL InsertConfigLog(CString strInfo);
	BOOL ConfigDumpEnd();
	BOOL WriteLog(CString&strLog);

	BOOL SktSimpleDumpBegin();
	BOOL InsertSktSimpleInfo(CString strInfo);
	BOOL SktSimpleDumpEnd();

private:
	CString GetLogTemplet();
	CString GetReportTemplet();

	void AppendEndTag();
	

	BOOL m_bTimeHead;
	CMutex m_Mutex;
	INT m_nEndPos;
};