#pragma once

#define ERRMSG_BUF_LEN	(256)
#define LOGMSG_MAXLEN	(10240)
#include <afxcmn.h>

typedef struct tagErrMSG{
	INT ErrNo;
	CHAR ErrMsg[ERRMSG_BUF_LEN];
}ERRMSG;
//
enum eLevel{
	/*LOGLEVEL_*/LOG,
	/*LOGLEVEL_*/WARNING,
	/*LOGLEVEL_*/ERR,
	/*LOGLEVEL_*/CRIST,
	//LOG_NOSHOW=0x10000000,	///直接输出到log文件中，不打印到屏幕上
};

class CLogMsg
{
public:
	CLogMsg(void):m_PrintEn(TRUE),m_PrintTimeEn(TRUE){};
	virtual ~CLogMsg(void){};
	virtual void PrintLog( INT LogLevel,const char *fmt,... );
	
	void SetPrintEn(BOOL En){m_PrintEn=En;}
	void SetPrintTimeEn(BOOL En){m_PrintTimeEn=En;}
protected:
	BOOL m_PrintEn;
	BOOL m_PrintTimeEn;
};

