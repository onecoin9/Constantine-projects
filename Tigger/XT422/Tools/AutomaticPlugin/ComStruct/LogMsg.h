#pragma once

#define ERRMSG_BUF_LEN	(256)
#define LOG_MSG_MAXLEN	(2048)
//#include "BaseView.h"
#include "ErrTypes.h"
//#include "../HtmlLogWriter/CHtmlLogWriter.h"
#include <mutex>
#include <map>
#include <QMetaType>

typedef struct tagErrMSG{
	int ErrNo;
	char ErrMsg[ERRMSG_BUF_LEN];
}ERRMSG;

enum eLogLevel{
	LOGLEVEL_LOG,
	LOGLEVEL_WARNING,
	LOGLEVEL_ERR,
	LOGLEVEL_CRIST,
	LOGLEVEL_SUCCESS,
	LOG_NOSHOW=0x10000000,	///直接输出到log文件中，不打印到屏幕上
};

// typedef struct tagSiteLogFile{
// 	string strFilePath;
// 	CFile file;
// };

class CLogMsg
{
public:
	CLogMsg(void);
	virtual ~CLogMsg(void);
	virtual void CLogMsg::SetProgress(int CurV, int Total);
	virtual void CLogMsg::PrintLog(int LogLevel, const char* fmt, ...);
	virtual int CLogMsg::LogMessageBox(uint32_t nType, const char* fmt, ...);///nType=MB_OK etc
	virtual void CLogMsg::DumpBuf(int LogLevel, uchar* pData, int Size, int EachLine); ///打印Buffer内容，pData指向Buffer数据，Size指示Buffer大小，EachLine每行多少个
	virtual void ShowTimeEnable(bool ShowTime);
	virtual bool GetShowTimeEn();
	virtual void ShowErrMsg(QString ErrMsgHead, ushort ErrNum);
	virtual void WriteFileEnable(bool En);

	void Log2SiteFile(int LogLevel, std::string strSiteName, const char* fmt, ...);
	void CreateSiteLogFile(std::string strSiteName, std::string strAprName);
	void CloseSiteLogFile(std::string strSiteName);

	////Extended Print For Html/////
	bool ConfigDumpBegin();
	bool InsertConfigLog(std::string strLog);
	bool ConfigDumpEnd();
	//CHtmlLogWriter* GetHtmlWriter(){return m_pLogFile;}

	bool SktSimpleDumpBegin();
	bool InsertSktSimpleInfo(std::string strLog );
	bool SktSimpleDumpEnd();
	
	///////

	static void PrintDebugString(const char *fmt,... );

	static int SetLogConfig();
	static std::string GetLogSavePath(){return m_LogSavePath;};
	static uint32_t GetMaxLogSize(){return m_MaxLogSize;};
	static uint32_t GetMaxLogDay(){return m_MaxLogDay;};
	static int DeleteLogOverdue();
	static int CreateLogDir(const std::string& DirName, std::string& LogPath);
	
	void CloseLog();
	//int  RedirLog( string strFile, CBaseView *pLogView);
	static bool DistachLogView();
	static bool ResetLogView();
	std::string GetLogPath();
	void ChangeLog(const std::string &LogFullPathNew);
protected:
	/*void CLogMsg::WriteToLogFile(CHAR *pBuf,int Size);*/
	bool CheckNeedChangeLog();

	std::string GetLogFullPathAtCurDir();
	
public:
	//static CHtmlLogWriter* m_pLogFile;
	//static CBaseView *m_pLogView;
	//static CBaseView *m_pTempLogView;
	static std::mutex m_csLogMsg;
	static bool m_bTimeHead;
	static std::string m_LogSavePath;
	static uint32_t m_MaxLogSize;
	static uint32_t m_MaxLogDay;
	static bool m_bCloseApp;
	static std::string m_strLogDirPath;
	std::string m_LogPath;
	bool m_bWriteToFile;
	static std::map<std::string, void*> m_SiteLogFileMap;
	
	//static CHtmlLogWriter* m_pHtmlLogFile;
};
