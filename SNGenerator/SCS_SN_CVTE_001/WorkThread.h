#pragma once

//#include "LogMsg.h"
#include <process.h>
#include <afxcmn.h>
#include <windef.h>

//typedef unsigned int  WINAPI (*FnWorkThreadProc)(void* args);
//typedef INT WINAPI (*FnMsgHandler)(MSG msg,void *Para);

typedef  unsigned int (WINAPI* FnWorkThreadProc)(void* args);
typedef INT (WINAPI* FnMsgHandler)(MSG msg,void *Para);

class CWorkThread/*:public CLogMsg*/
{
public:
	CWorkThread(void);
	INT CreateThread();
	INT CWorkThread::SetMsgHandler(FnMsgHandler pMsgHandle,void *Para);
	BOOL CWorkThread::PostMsg(UINT Msg,WPARAM wParam,LPARAM lParam);
	BOOL DeleteThread();

	static UINT WINAPI  WorkThreadProc(void* args);
public:
	virtual ~CWorkThread(void);

protected:
	BOOL CWorkThread::EnEvent();
	INT CWorkThread::HandleMsg(MSG Msg);

private:
	HANDLE m_hEvent;
	UINT m_dwThreadID;
	HANDLE m_hThread;
	void *m_Para;
	FnMsgHandler m_pMsgHandle;
};
