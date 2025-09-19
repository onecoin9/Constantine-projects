#include "stdafx.h"
#include "LogFile.h"


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
	if (m_LogFile.Open(m_strLogFile, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate | CFile::shareDenyNone) == FALSE) {
		AfxMessageBox("����־�ļ�ʧ��", MB_OK | MB_ICONERROR);
	}
	m_LogFile.SeekToEnd();
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
	if (m_LogFile.m_hFile == CFile::hFileNull)
		return;

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
	vsprintf(sprint_buf + offset, fmt, args);
	va_end(args); /* ��argp��ΪNULL */
	if (strlen(sprint_buf) > LOGMSG_MAXLEN - 2) {
		return;
	}
	strcat(sprint_buf, "\r\n");

	LenStr = (INT)strlen(sprint_buf) + 1;
	pData = new BYTE[LenStr];
	if (pData) {
		memcpy(pData, sprint_buf, LenStr);
		m_LogFile.Write(pData, LenStr);
		delete[] pData;
	}
}