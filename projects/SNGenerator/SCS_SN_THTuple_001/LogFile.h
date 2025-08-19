#pragma once

#include "logmsg.h"
class CLogFile:public CLogMsg
{
public:
	CLogFile();
	~CLogFile();

	void SetLogFile(CString strLogFile);
	void CloseLogFile();
	void PrintLog(INT LogLevel, const char *fmt, ...);

public:
	CString m_strLogFile;
	CFile m_LogFile;
};

