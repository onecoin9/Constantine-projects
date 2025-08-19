#include "LogMsg.h"
#include "ErrTypes.h"
#include "ComFunc.h"
#include "shlwapi.h"
#include <time.h>
#include <QDateTime>
#include <QDir>
#include <QMessageBox>
#include <QDebug>
//#include "../RmtServer/IceServerApp.h"

//CHtmlLogWriter *CLogMsg::m_pLogFile=NULL;
//CBaseView *CLogMsg::m_pLogView=NULL;
//CBaseView *CLogMsg::m_pTempLogView=NULL;
std::mutex CLogMsg::m_csLogMsg;
bool CLogMsg::m_bTimeHead=true;
std::string CLogMsg::m_LogSavePath="";
std::string CLogMsg::m_strLogDirPath;
uint32_t CLogMsg::m_MaxLogSize=0;
uint32_t CLogMsg::m_MaxLogDay=0;
bool CLogMsg::m_bCloseApp=false;
std::map<std::string, void*> CLogMsg::m_SiteLogFileMap ;

static std::string getTime() {
	const char* time_fmt = "%Y-%m-%d %H:%M:%S";
	time_t t = time(nullptr);
	char time_str[64];
	strftime(time_str, sizeof(time_str), time_fmt, localtime(&t));

	return time_str;
}

CLogMsg::CLogMsg(void)
:m_bWriteToFile(true)
{
}

CLogMsg::~CLogMsg(void)
{
}

std::string CLogMsg::GetLogFullPathAtCurDir()
{
	QString LogPath = QString::fromStdString(CComFunc::GetFilePath(m_LogPath));
	QString ExePath = QString::fromStdString(CComFunc::GetCurrentPath());
	QString strFolderpath;
	QString LogName, LogFullPath;

	QDateTime Time = QDateTime::currentDateTime();
	LogName = Time.toString("yyyy-MM-dd");

	strFolderpath = QString("%1/log/%2").arg(ExePath, LogName);

	QDir dir(strFolderpath);
	if (!dir.exists()) {
		dir.mkpath(strFolderpath);
	}

	LogFullPath = QString("%1/log/%2/log[%3].html")
		.arg(ExePath, LogName)
		.arg(getTime().data());

	return LogFullPath.toStdString();
}

void CLogMsg::ChangeLog(const std::string &LogFullPathNew)
{
	//if(m_pLogFile){
	//	if(m_pLogFile->m_hFile!=CFile::hFileNull){
	//		m_pLogFile->Close();
	//	}
	//	delete m_pLogFile;
	//	m_pLogFile=NULL;
	//}
	//CHtmlLogWriter *pFile=new CHtmlLogWriter;
	//if(pFile->CreateLog(LogFullPathNew)==false){
	//	///即使打开失败也要将其他资源初始化
	//	PrintLog(LOGLEVEL_ERR,"Sorry,Open Log File Error, Log Path=%s",(LPCSTR)LogFullPathNew);
	//	return;
	//}
	//m_LogPath=LogFullPathNew;
	//m_pLogFile=pFile;
}

bool CLogMsg::CheckNeedChangeLog()
{
	//if(m_pLogFile && m_pLogFile->m_hFile!=CFile::hFileNull){
	//	if(m_MaxLogSize!=0){///有指定大小，则需要判断大小
	//		if(m_pLogFile->length()>m_MaxLogSize){///文件超过配置的最大长度，需要重新打开另外一个文件
	//			string LogFullPathNew=GetLogFullPathAtCurDir();///获取log文件名
	//			ChangeLog(LogFullPathNew);
	//		}
	//	}
	//}
	//return true;
	return true;
}

//void CLogMsg::WriteToLogFile(CHAR *pBuf,int Size)
//{
//	if(m_pLogFile && m_pLogFile->m_hFile!=CFile::hFileNull){
//		if(m_MaxLogSize!=0){///有指定大小，则需要判断大小
//			if(m_pLogFile->length()>m_MaxLogSize){///文件超过配置的最大长度，需要重新打开另外一个文件
//				string LogFullPathNew=GetLogFullPathAtCurDir();///获取log文件名
//				ChangeLog(LogFullPathNew);
//			}
//		}
//		m_pLogFile->Write(pBuf,Size);
//	}
//}

void CLogMsg::SetProgress(int CurV,int Total)
{
	//if(CurV==0 && Total==0){
	//	return ;
	//}
	//if(m_pLogView)
	//	m_pLogView->SetProgress(CurV,Total);

	//if(CRmtServer::IsServerRun()){
	//	CIceServerApp& GIceServerApp=GetGlobalServerApp();
	//	GIceServerApp.ClientSetProgress("ALL",CurV,Total);
	//}
}

int CLogMsg::LogMessageBox(uint32_t nType,const char *fmt,...)
{
	int ret=0;
	int offset=0;
	char sprint_buf[LOG_MSG_MAXLEN]; 
	va_list args;
	ZeroMemory(sprint_buf,LOG_MSG_MAXLEN);
	va_start(args,fmt);  
	vsprintf(sprint_buf+offset,fmt,args); 
	va_end(args); /* 将argp置为NULL */
	if(strlen(sprint_buf)>LOG_MSG_MAXLEN-2){
		return ACERR_PARA;
	}
	return QMessageBox::information(nullptr, "Info", sprint_buf, QMessageBox::Ok);
}

#if 0
void CLogMsg::DumpBuf(int LogLevel,uchar*pData,int Size,int EachLine)
{
	int i=0;
	int j=0;
	int TotalSize,StartIdx=0;
	CHAR Buf[512];
	CHAR TmpBuf[8];
	CHAR StrTmpBuf[8];
	CHAR StrBuf[512];
	if(EachLine<=0 || EachLine>=16){
		EachLine=16;
	}
	TotalSize=(Size+EachLine-1)/EachLine*EachLine;///向上取整
	memset(Buf,0,512);
	memset(StrBuf,0,512);
	if(pData!=NULL && Size!=0){
		for(i=0;i<TotalSize;i++){
			if(i<Size){
				if(i!=0 && i%EachLine==0){
					PrintLog(LogLevel,"%s      %s",Buf,StrBuf);
					StartIdx+=EachLine;
					memset(Buf,0,512);
					memset(StrBuf,0,512);
				}
				sprintf(TmpBuf,"%02X ",pData[i]);

				if(pData[i]>=0x21 && pData[i]<=0x80){//可见字符
					sprintf(StrTmpBuf,"%c",pData[i]);
				}
				else{
					sprintf(StrTmpBuf,"%c",'.');
				}
			}
			else{
				sprintf(TmpBuf,"   ");///后面用空格补全
				sprintf(StrTmpBuf,"%c",' ');
			}
			strcat(Buf,TmpBuf);
			strcat(StrBuf,StrTmpBuf);
		}
		PrintLog(LogLevel,"%s      %s",Buf,StrBuf);
	}
}
#else
void CLogMsg::DumpBuf(int LogLevel,uchar* pData,int Size,int EachLine)
{
	//int i=0;
	//std::string strDataShow;
	//std::string strTmp;
	//if(EachLine<=0 || EachLine>=16){
	//	EachLine=16;
	//}
	//if(pData!=NULL && Size!=0){
	//	for(i=0;i<Size;i++){
	//		if(i!=0 && i%EachLine==0){///满16个数据打印输出
	//			PrintLog(LogLevel,"%s",strDataShow);
	//			strDataShow="";
	//		}
	//		strTmp.Format("%02X ",pData[i]);
	//		strDataShow +=strTmp;
	//	}
	//	PrintLog(LogLevel,"%s",strDataShow);
	//}
}
#endif 

void CLogMsg::ShowTimeEnable(bool ShowTime)
{
	m_bTimeHead=ShowTime;
}

bool CLogMsg::GetShowTimeEn()
{
	return m_bTimeHead;
}

bool CLogMsg::ResetLogView()
{
	//m_pLogView = m_pTempLogView;
	return true;
}
bool CLogMsg::DistachLogView()
{
	//m_pTempLogView = m_pLogView;
	//m_pLogView=NULL;
	m_bCloseApp=true;
	return true;
}
#define SPACE_HEAD (23)

void CLogMsg::CloseSiteLogFile(std::string strSiteName)
{
	//if (strSiteName.IsEmpty() ){
	//	return ;
	//}
	//
	//if (m_SiteLogFileMap[strSiteName] == NULL){
	//	m_SiteLogFileMap.erase(strSiteName);
	//	return ;
	//}

	//if ( ((CFile*)m_SiteLogFileMap[strSiteName])->m_hFile != CFile::hFileNull) {
	//	((CFile*)m_SiteLogFileMap[strSiteName])->Flush();
	//	((CFile*)m_SiteLogFileMap[strSiteName])->Close();
	//}

	//delete m_SiteLogFileMap[strSiteName];
	//m_SiteLogFileMap[strSiteName] = NULL;

	//m_SiteLogFileMap.erase(strSiteName);

}
void CLogMsg::CreateSiteLogFile(std::string strSiteName, std::string strAprName)
{
//	string strPath;
//	SYSTEMTIME st;
//	
//	if (strSiteName.IsEmpty() || strAprName.IsEmpty() ){
//		return ;
//	}
//
//	if (m_SiteLogFileMap[strSiteName] == NULL){ //首次创建文件
//		m_SiteLogFileMap[strSiteName] = new CFile;
//		
//		GetLocalTime(&st);
//		string LogPath=m_strLogDirPath;//CComFunc::GetFilePath(m_strLogDirPath);
//		strPath.Format("%s\\%s-SiteLog-%02d-%02d-%02d-%s.txt",LogPath/* CComFunc::GetFilePath(m_strLogDirPath)*/, strAprName,st.wHour,st.wMinute,st.wSecond, strSiteName);
//
//		if ( ((CFile*)m_SiteLogFileMap[strSiteName])->Open(strPath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone) == false) {
//		}
//	}
//	
//__end:
	return ;
}
void CLogMsg::Log2SiteFile( int LogLevel, std::string strSiteName, const char *fmt,... )
{
	//int ret=0;

	//if (strSiteName.IsEmpty()){
	//	return ;
	//}

	//if (m_SiteLogFileMap.find(strSiteName) == m_SiteLogFileMap.end() || m_SiteLogFileMap[strSiteName] == NULL){
	//	return ;
	//}

	//if ( ((CFile*)m_SiteLogFileMap[strSiteName])->m_hFile == CFile::hFileNull){
	//	return ;
	//}

	//int offset=0;
	//char sprint_buf[LOG_MSG_MAXLEN]; 
	//char LogLevelType; ///Normal
	//va_list args;
	//CTime Time;
	//SYSTEMTIME st;
	//GetLocalTime(&st);
	//ZeroMemory(sprint_buf,LOG_MSG_MAXLEN);


	//if(m_bTimeHead){
	//	switch(LogLevel){
	//		case LOGLEVEL_LOG:
	//			LogLevelType='N';///Normal
	//			break;
	//		case LOGLEVEL_ERR:
	//			LogLevelType='E';///Error
	//			break;
	//		case LOGLEVEL_WARNING:
	//			LogLevelType='W';///Warning
	//			break;
	//		default:
	//			LogLevelType='N';
	//			break;
	//	}
	//	sprintf(sprint_buf,"[%d/%02d/%02d %02d:%02d:%02d.%03d-%c]",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds,LogLevelType);
	//	offset=(int)strlen(sprint_buf);
	//}
	//else{
	//	offset+=SPACE_HEAD;
	//	memset(sprint_buf,0x20,SPACE_HEAD);///在前面添加23个空格，为了对齐操作
	//}
	//va_start(args,fmt);  
	//vsprintf(sprint_buf+offset,fmt,args); 
	//va_end(args); /* 将argp置为NULL */
	//if(strlen(sprint_buf)>LOG_MSG_MAXLEN-2){
	//	return;
	//}
	//strcat(sprint_buf, "\r\n");
	//offset=(int)strlen(sprint_buf);

	//((CFile*)m_SiteLogFileMap[strSiteName])->Write(sprint_buf, offset);
}


void CLogMsg::PrintLog( int LogLevel,const char *fmt,... )
{
	int ret=0;
	int offset=0;
	char sprint_buf[LOG_MSG_MAXLEN]; 
	char LogLevelType; ///Normal
	va_list args;
	QTime Time;
	SYSTEMTIME st;
	GetLocalTime(&st);
	//Time=CTime::GetCurrentTime();
	ZeroMemory(sprint_buf,LOG_MSG_MAXLEN);
	
	if(m_bTimeHead){
		switch(LogLevel){
			case LOGLEVEL_LOG:
				LogLevelType='N';///Normal
				break;
			case LOGLEVEL_ERR:
				LogLevelType='E';///Error
				break;
			case LOGLEVEL_WARNING:
				LogLevelType='W';///Warning
				break;
			default:
				LogLevelType='N';
				break;
		}
		//sprintf(sprint_buf,"[%d/%02d/%02d %02d:%02d:%02d-%c]",Time.GetYear(),Time.GetMonth(),Time.GetDay(),
		//	Time.GetHour(),Time.GetMinute(),Time.GetSecond(),LogLevelType);
		sprintf(sprint_buf,"[%d/%02d/%02d %02d:%02d:%02d.%03d-%c]",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds,LogLevelType);
		offset=(int)strlen(sprint_buf);
	}
	else{
		offset+=SPACE_HEAD;
		memset(sprint_buf,0x20,SPACE_HEAD);///在前面添加23个空格，为了对齐操作
	}
	va_start(args,fmt);  
	vsprintf(sprint_buf+offset,fmt,args); 
	va_end(args); /* 将argp置为NULL */
	if(strlen(sprint_buf)>LOG_MSG_MAXLEN-2){
		return;
	}

	if(m_bWriteToFile){
		CheckNeedChangeLog();
		//if(m_pLogFile){
		//	m_pLogFile->InsertLog(LogLevel,sprint_buf+offset);///不需要将日期传入
		//}
	}
	
	strcat(sprint_buf,"\r\n");
	m_csLogMsg.lock();
	if(!(LogLevel&LOG_NOSHOW)){
		//if(m_pLogView){
		//	//m_pLogView->ShowLog(LogLevel,"%s",sprint_buf);
		//	m_pLogView->ShowLog(LogLevel,sprint_buf);
		//}
	
		//if(CRmtServer::IsServerRun()==true && m_bCloseApp==false){
		//	///客户端请求关闭服务器之后就不再推送日志，否则ClientOutput函数会占用长时间等待才会退出
		//	CIceServerApp& GServerApp=GetGlobalServerApp();
		//	GServerApp.ClientOutput("ALL",LogLevel,"%s",sprint_buf);
		//}
	
	}
	m_csLogMsg.unlock();
}

void CLogMsg::CloseLog()
{
	//if(m_pLogFile && m_pLogFile->m_hFile!=CFile::hFileNull){
	//	m_pLogFile->Close();
	//	delete m_pLogFile;
	//	m_pLogFile=NULL;
	//}
	//if(m_pLogView){
	//	m_pLogView=NULL;
	//}

	//if (m_pTempLogView){
	//	m_pTempLogView = NULL;
	//}

	//DeleteCriticalSection(&m_csLogMsg);
}

//int  CLogMsg::RedirLog( string strFile, CBaseView *pLogView)
//{
//	int Ret=ACERR_OK;
//	if(strFile.length()){
//		CHtmlLogWriter *pFile=new CHtmlLogWriter;
//		CloseLog();
//		if(pFile->CreateLog(strFile)==false){
//			m_pLogView=pLogView;
//			InitializeCriticalSection(&m_csLogMsg);
//			delete pFile;
//			PrintLog(LOGLEVEL_ERR,"Sorry,Open Log File Error, Log Path=%s",(LPCSTR)strFile);
//			return ACERR_OPENLOG;
//		}
//		m_LogPath=strFile;
//		m_pLogFile=pFile;
//		
//	}
//	else{
//		CloseLog();
//	}
//	m_pLogView=pLogView;
//	InitializeCriticalSection(&m_csLogMsg);
//	return Ret;
//}

int CLogMsg::SetLogConfig()
{
	int Ret=ACERR_OK;
	//CHAR TmpBuf[MAX_PATH];
	//string ExePath=CComFunc::GetCurrentPath();
	//string strIniFile = ExePath+"/MultiCfg.ini";
	//memset(TmpBuf,0,MAX_PATH);
	//GetPrivateProfileString("Log", "LogPath", ExePath+"/log",TmpBuf, MAX_PATH, strIniFile);
	//if(TmpBuf[0]==0){
	//	return ACERR_PARA;
	//}
	//m_LogSavePath.Format("%s",TmpBuf);
	//m_MaxLogSize = GetPrivateProfileint("Log", "LogSize",1, strIniFile);
	//if(m_MaxLogSize!=0){
	//	m_MaxLogSize *=1024*1024; ///以M字节为单位
	//}
	//m_MaxLogDay = GetPrivateProfileint("Log", "LogDay", 0, strIniFile);
	return Ret;
}


///删除过期的Log
int CLogMsg::DeleteLogOverdue()
{
	int Ret = ACERR_OK;

	QDateTime currentTime = QDateTime::currentDateTime();
	QString ExePath = QString::fromStdString(CComFunc::GetCurrentPath());
	QString LogDirPath;
	QString LogSavePath = QString::fromStdString(CLogMsg::GetLogSavePath());

	if (CLogMsg::GetMaxLogDay() == 0) {
		return Ret;
	}

	if (LogSavePath.at(0) == '.') {
		LogDirPath = QString("%1/%2").arg(ExePath, LogSavePath);
	}
	else {
		LogDirPath = LogSavePath;
	}

	QDir dir(LogDirPath);
	if (!dir.exists()) {
		return ACERR_FAIL;
	}

	QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
	foreach(const QString & fileName, files) {
		QString filePath = dir.filePath(fileName);
		QFileInfo fileInfo(filePath);
		QDateTime creationTime = fileInfo.created();
		qint64 daysDiff = creationTime.daysTo(currentTime);
		if (daysDiff > CLogMsg::GetMaxLogDay()) {
			if (!QFile::remove(filePath)) {
				qWarning() << "Failed to delete file:" << filePath;
			}
		}
	}
	return Ret;
}


std::string CLogMsg::GetLogPath()
{
	return m_LogPath;
}

void CLogMsg::PrintDebugString( const char *fmt,... )
{
	int ret = 0;
	int offset = 0;
	char sprint_buf[LOG_MSG_MAXLEN];
	va_list args;
	va_start(args, fmt);

	QDateTime currentTime = QDateTime::currentDateTime();
	QString timeString = currentTime.toString("[MultiAporg-MSG:hh:mm:ss.zzz]");

	memset(sprint_buf, 0, LOG_MSG_MAXLEN);
	sprintf(sprint_buf, "%s", qPrintable(timeString));
	offset = (int)strlen(sprint_buf);
	vsprintf(sprint_buf + offset, fmt, args);
	va_end(args);

	if (strlen(sprint_buf) > LOG_MSG_MAXLEN - 2) {
		return;
	}

	qDebug() << sprint_buf;
}

void CLogMsg::ShowErrMsg(QString ErrMsgHead, ushort ErrNum)
{
	LPTSTR lpMessageBuffer = NULL;
	//FormatMessage 将GetLastError函数得到的错误信息（这个错误信息是数字代号）转化成字符串信息的函数。
	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,NULL,ErrNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR)&lpMessageBuffer,0,NULL )){
		PrintLog(LOGLEVEL_ERR, "%s failure: (0x%08X) %s\n", ErrMsgHead, ErrNum, (LPTSTR)lpMessageBuffer);
	}
	else{
		PrintLog(LOGLEVEL_ERR,"%s failure: (0x%08X)",ErrMsgHead,ErrNum);
	}
	if (lpMessageBuffer) 
		LocalFree( lpMessageBuffer ); // Free system buffer
}

void CLogMsg::WriteFileEnable( bool En )
{
	m_bWriteToFile=En;
}

bool CLogMsg::ConfigDumpBegin()
{
	//if(m_pLogFile){
	//	return m_pLogFile->ConfigDumpBegin();
	//}
	return false;
}

bool CLogMsg::InsertConfigLog(std::string strLog )
{
	//if(m_pLogFile){
	//	return m_pLogFile->InsertConfigLog(strLog);
	//}
	return false;
}

bool CLogMsg::ConfigDumpEnd()
{
	//if(m_pLogFile){
	//	return m_pLogFile->ConfigDumpEnd();
	//}
	return false;
}

bool CLogMsg::SktSimpleDumpBegin()
{
	//if(m_pLogFile){
	//	return m_pLogFile->SktSimpleDumpBegin();
	//}
	return false;
}

bool CLogMsg::InsertSktSimpleInfo(std::string strLog )
{
	//if(m_pLogFile){
	//	return m_pLogFile->InsertSktSimpleInfo(strLog);
	//}
	return false;
}

bool CLogMsg::SktSimpleDumpEnd()
{
	//if(m_pLogFile){
	//	return m_pLogFile->SktSimpleDumpEnd();
	//}
	return false;
}

int CLogMsg::CreateLogDir( const std::string& DirName, std::string &LogPath )
{
	int Ret=ACERR_OK;
	QString ExePath = QDir::currentPath();
	QString LogDirPath;
	QString LogSavePath = QString::fromStdString(CLogMsg::GetLogSavePath());

	if (LogSavePath.at(0) == '.') {
		LogDirPath = QString("%1/%2/%3").arg(ExePath, LogSavePath, QString::fromStdString(DirName));
	}
	else {
		LogDirPath = QString("%1/%2").arg(LogSavePath, QString::fromStdString(DirName));
	}

	QDir dir;
	if (!dir.mkpath(LogDirPath)) {
		if (GetLastError() == ERROR_PATH_NOT_FOUND) {
			Ret = ACERR_CREATELOGDIR;
		}
	}

	LogPath = LogDirPath.toStdString();
	m_strLogDirPath = LogPath;
	
	return Ret;
}
