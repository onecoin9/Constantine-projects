#include "stdafx.h"
#include "LogFile.h"
#include "shlwapi.h"


CLogFile::CLogFile()
{
}


CLogFile::~CLogFile()
{
	CloseLogFile();
}

void CLogFile::SetLogFile(CString strLogFile)
{
	CloseLogFile();

	m_strLogFile = strLogFile;
	if (m_LogFile.Open(m_strLogFile, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone) == FALSE) {
		AfxMessageBox("打开日志文件失败", MB_OK | MB_ICONERROR);
	}
}

void CLogFile::CloseLogFile()
{
	if (m_LogFile.m_hFile != CFile::hFileNull) {
		m_LogFile.Flush();
		m_LogFile.Close();
	}
	/*FreeLogMsgs();*/
}

void CLogFile::PrintLog(INT LogLevel, const char *fmt, ...)
{
	INT LenStr;
	BYTE*pData = NULL;
	BYTE LogLevelType = 0;
	int ret = 0;
	int offset = 0;
	char sprint_buf[LOGMSG_MAXLEN];
	va_list args;
	memset(sprint_buf, 0, LOGMSG_MAXLEN);

	CTime Time;
	Time = CTime::GetCurrentTime();

	switch (LogLevel) {
	case /*LOGLEVEL_*/LOG:
		LogLevelType = 'N';///Normal
		break;
	case /*LOGLEVEL_*/ERR:
		LogLevelType = 'E';///Error
		break;
	case /*LOGLEVEL_*/WARNING:
		LogLevelType = 'W';///Warning
		break;
	default:
		LogLevelType = 'N';
		break;
	}

	SYSTEMTIME st;
	GetLocalTime(&st);
	
	sprintf(sprint_buf, "[%d/%02d/%02d %02d:%02d:%02d:%03d-%c]", st.wYear, st.wMonth, st.wDay, st.wHour, 
		st.wMinute, st.wSecond, st.wMilliseconds, LogLevelType);
	//if (m_PrintTimeEn) {
		/*sprintf(sprint_buf, "[%d/%02d/%02d %02d:%02d:%02d-%c]", Time.GetYear(), Time.GetMonth(), Time.GetDay(),
			Time.GetHour(), Time.GetMinute(), Time.GetSecond(), LogLevelType);*/

		offset = (INT)strlen(sprint_buf);
	//}
	va_start(args, fmt);
	_vsnprintf(sprint_buf + offset, LOGMSG_MAXLEN -offset, fmt, args);
	va_end(args); /* 将argp置为NULL */
	if (strlen(sprint_buf) > LOGMSG_MAXLEN - 2) {
		return;
	}
	strcat(sprint_buf, "\r\n");

	LenStr = (INT)strlen(sprint_buf) + 1;
	pData = new BYTE[LenStr];
	if (pData) {
		memcpy(pData, sprint_buf, LenStr);
		if (m_LogFile.m_hFile != CFile::hFileNull) {///写入文件
			m_LogFile.Write(pData, LenStr);
			//m_LogFile.Flush();
		}
	}
}