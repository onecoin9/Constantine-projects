#include "stdafx.h"
#include "WorkThread.h"

CWorkThread::CWorkThread(void)
{
	m_hEvent=NULL;
	m_dwThreadID=0;
	m_Para=NULL;
	m_pMsgHandle=NULL;
}

CWorkThread::~CWorkThread(void)
{
	/*CString strMsg;
	strMsg.Format("结束线程=%d",  ::GetCurrentThreadId());
	AfxMessageBox(strMsg);*/
	DeleteThread();
}

BOOL CWorkThread::PostMsg(UINT Msg,WPARAM wParam,LPARAM lParam)
{
	if(m_dwThreadID){
		return PostThreadMessage(m_dwThreadID,Msg,wParam,lParam);
	}
	else
		return FALSE;
}

INT CWorkThread::SetMsgHandler(FnMsgHandler pMsgHandle,void *Para)
{
	m_Para=Para;
	m_pMsgHandle=pMsgHandle;
	return 0;
}


/**************
消息处理函数，处理成功返回0，否则返回非零
***************/
INT CWorkThread::HandleMsg(MSG Msg)
{
	INT Ret=0;
	if(m_pMsgHandle){
		Ret=m_pMsgHandle(Msg,m_Para);
		return Ret;
	}
	return -1;
}

INT CWorkThread::CreateThread()
{
	HANDLE hThread=0;
	UINT threadid;
	if(m_dwThreadID!=0){
		//AfxMessageBox("Job Is Doing, Please Try Again Later!");
		return FALSE;
	}

	m_hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
	if(m_hEvent==NULL){
		return FALSE;
	}

	hThread = (HANDLE)_beginthreadex(NULL,8096,WorkThreadProc,this,0,&threadid);
	if(hThread==0){
		CloseHandle(m_hEvent);
		m_hEvent=NULL;
		return FALSE;
	}
	m_dwThreadID = threadid;
	m_hThread=hThread;
	WaitForSingleObject(m_hEvent,INFINITE);
	CloseHandle(m_hEvent);
	return TRUE;
}

BOOL CWorkThread::EnEvent()
{
	if(SetEvent(m_hEvent)==FALSE){
		return FALSE;
	}
	else{
		return TRUE;
	}
}

UINT WINAPI CWorkThread::WorkThreadProc( void* args )
{
	INT ret;
	MSG msg;
	CWorkThread* pWorkThread=(CWorkThread*)args;
	PeekMessage(&msg,NULL,WM_USER, WM_USER, PM_NOREMOVE);///用该函数强制系统创建消息队列
	if(pWorkThread->EnEvent()==FALSE){
		goto __end;
	}
	while(1){
		ret= ::GetMessage(&msg,NULL,0,0);
		if(ret==0 || ret==-1){///收到WM_QUIT消息返回0，出现错误则返回-1
			break;
		}
		ret=pWorkThread->HandleMsg(msg);
		if(ret!=0){
			break;
		}
	}
__end:
	pWorkThread->m_dwThreadID=0;
	_endthreadex(ret);
	return ret;
}

BOOL CWorkThread::DeleteThread()
{
	if(m_dwThreadID){
		BOOL Ret;
		Ret=PostMsg(WM_QUIT,0,0);
		if(Ret==FALSE){
			DWORD ErrNo=GetLastError();
			//PrintLog(LOGLEVEL_ERR,"Thread post message WM_QUIT failed, ErrNo=0x%x",ErrNo);
		}
		else{
			WaitForSingleObject(m_hThread,INFINITE);
			m_dwThreadID=0;
		}
	}

	if (m_pMsgHandle) {///需要释放
		//delete m_pMsgHandle;
		//m_pMsgHandle = NULL;
	}
	return TRUE;
}