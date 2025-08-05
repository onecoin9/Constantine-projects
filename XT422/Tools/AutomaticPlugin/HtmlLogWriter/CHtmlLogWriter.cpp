#include "stdafx.h"
#include "CHtmlLogWriter.h"
#include "../ComStruct/ComFunc.h"

#define STR_ENDTAG "<hr/>\r\n</div>\r\n</body>\r\n</html>"

#define LOG_MSG_MAXLEN (2048)
enum eLogLevel{
	LOGLEVEL_LOG,
	LOGLEVEL_WARNING,
	LOGLEVEL_ERR,
	LOGLEVEL_CRIST,
	LOGLEVEL_SUCCESS,
	LOG_NOSHOW=0x10000000,	///直接输出到log文件中，不打印到屏幕上
};

CString CHtmlLogWriter::GetReportTemplet()
{
	CString strExe=CComFunc::GetCurrentPath();
	CString strTemplet;
	strTemplet.Format("%s\\data\\ReportTemplet.html",strExe);
	return strTemplet;
}
CString CHtmlLogWriter::GetLogTemplet()
{
	CString strExe=CComFunc::GetCurrentPath();
	CString strTemplet;
	strTemplet.Format("%s\\data\\LogTemplet.html",strExe);
	return strTemplet;
}

CHtmlLogWriter::CHtmlLogWriter()
:m_bTimeHead(TRUE)
,m_nEndPos(0)
{

}

BOOL CHtmlLogWriter::WriteLog(CString&strLog)
{
	INT len=strLog.GetLength();
	m_Mutex.Lock();
	Seek(-m_nEndPos,CFile::end);
	Write(strLog,strLog.GetLength());
	AppendEndTag();
	Flush();
	m_Mutex.Unlock();
	return TRUE;
}

BOOL CHtmlLogWriter::InsertLog(INT LogLevel,const char *fmt,...)
{	
	BOOL Ret=TRUE;
	int offset=0;
	char sprint_buf[LOG_MSG_MAXLEN]; 
	CString LogStatus;
	CString LogShort="N";
	va_list args;
	CString strLine,strLog;
	ZeroMemory(sprint_buf,LOG_MSG_MAXLEN);
	switch(LogLevel){
		case LOGLEVEL_LOG:
			LogStatus="lognormal";///Normal
			LogShort="N";
			break;
		case LOGLEVEL_ERR:
			LogStatus="logerror";///Error
			LogShort="E";
			break;
		case LOGLEVEL_WARNING:
			LogStatus="logwarning";///Warning
			LogShort="W";
			break;
		default:
			LogStatus="lognormal";///Normal
			break;
	}
	if(m_bTimeHead){
		CString strTime;
		CTime Time;
		Time=CTime::GetCurrentTime();
		strTime.Format("<p class='logtime'> [%d/%02d/%02d %02d:%02d:%02d-%s] &nbsp;</p>",Time.GetYear(),Time.GetMonth(),Time.GetDay(),
			Time.GetHour(),Time.GetMinute(),Time.GetSecond(),LogShort);
		strLine+=strTime;
	}
	va_start(args,fmt);  
	vsprintf(sprint_buf+offset,fmt,args); 
	va_end(args); /* 将argp置为NULL */
	if(strlen(sprint_buf)>LOG_MSG_MAXLEN-2){
		return FALSE;
	}
	CString strOrg;
	strOrg.Format("%s",sprint_buf);
	strOrg.Replace(" ","&nbsp");
	strLog.Format("<p class = '%s'>%s</p><br/>\r\n",LogStatus,strOrg);
	strLine +=strLog;
	return WriteLog(strLine);
}

void CHtmlLogWriter::AppendEndTag()
{
	Write(STR_ENDTAG,(UINT)strlen(STR_ENDTAG));
}

BOOL CHtmlLogWriter::ConfigDumpBegin()
{
	CString strTime,strLog;
	CTime Time;
	Time=CTime::GetCurrentTime();
	strTime.Format("[%d/%02d/%02d %02d:%02d:%02d]",Time.GetYear(),Time.GetMonth(),Time.GetDay(),
		Time.GetHour(),Time.GetMinute(),Time.GetSecond());
	char strBegin[]="\
<div id = \"configinfo\">\r\n\
<p class='logtime'>%s &nbsp;</p><p class = 'SectionHeader button'><span>+</span>Chip Config</p><br/>\r\n\
<div style = \"display:none; \">\r\n\
<table><tr><td style='font:12pt Consolas; font-weight:bold;'><center>Chip Config</center></td></tr></table>\r\n\
<tr><td><hr/></td></tr>\r\n";
	strLog.Format(strBegin,strTime);
	return WriteLog(strLog);
}

BOOL CHtmlLogWriter::SktSimpleDumpBegin()
{
	CString strTime,strLog;
	CTime Time;
	Time=CTime::GetCurrentTime();
	strTime.Format("[%d/%02d/%02d %02d:%02d:%02d]",Time.GetYear(),Time.GetMonth(),Time.GetDay(),
		Time.GetHour(),Time.GetMinute(),Time.GetSecond());
	char strBegin[]="\
					<div id = \"AdapterInformation\">\r\n\
					<p class='logtime'>%s &nbsp;</p><p class = 'SectionHeader button'><span>+</span>Adapter Information</p><br/>\r\n\
					<div style = \"display:none; \">\r\n\
					<table><tr><td style='font:12pt Consolas; font-weight:bold;'><center>Adapter Information</center></td></tr></table>\r\n\
					<tr><td><hr/></td></tr>\r\n";
	strLog.Format(strBegin,strTime);
	return WriteLog(strLog);
}

BOOL CHtmlLogWriter::InsertSktSimpleInfo(CString strInfo)
{	
	return InsertConfigLog(strInfo);
}

BOOL CHtmlLogWriter::SktSimpleDumpEnd()
{
	CString strLine="\
					<tr><td><hr/></td></tr>\r\n\
					</div>\r\n\
					</div>\r\n";
	return WriteLog(strLine);
}

BOOL CHtmlLogWriter::InsertConfigLog(CString strInfo)
{
	CString strLine;
	strInfo.Replace(" ","&nbsp");
	strInfo.Replace("\r\n","&nbsp");
	strLine.Format("<p class='lognormal'>%s</p><br/>\r\n",strInfo);
	return WriteLog(strLine);
}

BOOL CHtmlLogWriter::ConfigDumpEnd()
{
	CString strLine="\
<tr><td><hr/></td></tr>\r\n\
</div>\r\n\
</div>\r\n";
	return WriteLog(strLine);
}


BOOL CHtmlLogWriter::CreateLog(CString strLogFile,INT LogType)
{
	BOOL Ret=TRUE;
	BYTE *pByte=NULL;
	m_Mutex.Lock();
	//Ret=Open(strLogFile,CFile::modeWrite|CFile::modeCreate|CFile::shareDenyWrite,NULL);
	Ret=Open(strLogFile,CFile::modeWrite|CFile::modeCreate|CFile::shareDenyNone,NULL);
	if(Ret){
		CString strTemplet;
		if(LogType==LOGTYPE_NORMAL){
			strTemplet=GetLogTemplet();
		}
		else{
			strTemplet=GetReportTemplet();
		}
		CFile TmpFile;
		if(TmpFile.Open(strTemplet,CFile::modeRead,NULL)){
			pByte =new BYTE[(UINT)TmpFile.GetLength()];
			if(pByte){
				TmpFile.Read(pByte,(UINT)TmpFile.GetLength());
				Write(pByte,(UINT)TmpFile.GetLength());
			}
			else{
				Ret=FALSE;
			}
			TmpFile.Close();
		}
		else{
			Ret=FALSE;
		}
	}

	if(pByte){
		delete[] pByte;
	}
	m_nEndPos=(INT)strlen(STR_ENDTAG);
	m_Mutex.Unlock();
	return Ret;
}

