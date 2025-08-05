#pragma once

//#include "LogMsg.h"
#include <windows.h>
#include <process.h>
//#include <windef.h>
#include <QString>
//typedef unsigned int  WINAPI (*FnWorkThreadProc)(void* args);
//typedef int WINAPI (*FnMsgHandler)(MSG msg,void *Para);


typedef  unsigned int (__stdcall* FnWorkThreadProc)(void* args);
typedef int (__stdcall* FnMsgHandler)(MSG msg,void *Para);

class CWorkThread/*:public CLogMsg*/
{
public:
	CWorkThread(void);
	int CreateThread();
	int CWorkThread::SetMsgHandler(FnMsgHandler pMsgHandle,void *Para);
	bool CWorkThread::PostMsg(uint32_t Msg,WPARAM wParam,LPARAM lParam);
	bool DeleteThread();
	uint32_t GetThreadID(){return m_dwThreadID;}
	static uint32_t WINAPI  WorkThreadProc(void* args);
	QString GetErrMsg(){return m_strErrMsg;};
public:
	virtual ~CWorkThread(void);

protected:
	bool CWorkThread::EnEvent();
	int CWorkThread::HandleMsg(MSG Msg);

private:
	HANDLE m_hEvent;
	uint32_t m_dwThreadID;
	HANDLE m_hThread;
	void *m_Para;
	FnMsgHandler m_pMsgHandle;
	QString m_strErrMsg;
};
