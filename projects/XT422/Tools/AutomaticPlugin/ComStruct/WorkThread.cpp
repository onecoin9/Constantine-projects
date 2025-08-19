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
	DeleteThread();
}

bool CWorkThread::PostMsg(uint32_t Msg,WPARAM wParam,LPARAM lParam)
{
	if(m_dwThreadID){
		return PostThreadMessage(m_dwThreadID,Msg,wParam,lParam);
	}
	else
		return false;
}

int CWorkThread::SetMsgHandler(FnMsgHandler pMsgHandle,void *Para)
{
	m_Para=Para;
	m_pMsgHandle=pMsgHandle;
	return 0;
}


/**************
消息处理函数，处理成功返回0，否则返回非零
***************/
int CWorkThread::HandleMsg(MSG Msg)
{
	int Ret=0;
	if(m_pMsgHandle){
		Ret=m_pMsgHandle(Msg,m_Para);
		return Ret;
	}
	return -1;
}

int CWorkThread::CreateThread()
{
	HANDLE hThread=0;
	uint32_t threadid;
	int TryCnt=0;

	while(TryCnt<3){
		if(m_dwThreadID!=0){
			Sleep(400);
		}
		else{
			break;
		}
		TryCnt++;
	}
	if(m_dwThreadID!=0){
		m_strErrMsg = QString("Wait Thread Exit Failed, dwThreadID=0x%1").arg(QString::number(m_dwThreadID, 16));
		return false;
	}

	m_hEvent=CreateEvent(NULL,false,false,NULL);
	if(m_hEvent==NULL){
		m_strErrMsg = QString("Create Thread Event Failed");
		return false;
	}

	hThread = (HANDLE)_beginthreadex(NULL,8096,WorkThreadProc,this,0,&threadid);
	if(hThread==0){
		//CLogMsg::MessageBox(MB_OK,"CreateThread failed");
		m_strErrMsg = QString("Create Thread Failed");
		CloseHandle(m_hEvent);
		m_hEvent=NULL;
		return false;
	}
	m_dwThreadID = threadid;
	m_hThread=hThread;
	WaitForSingleObject(m_hEvent,INFINITE);
	CloseHandle(m_hEvent);
	return true;
}

bool CWorkThread::EnEvent()
{
	if(SetEvent(m_hEvent)==false){
		return false;
	}
	else{
		return true;
	}
}

uint32_t WINAPI CWorkThread::WorkThreadProc( void* args )
{
	int ret;
	MSG msg;
	CWorkThread* pWorkThread=(CWorkThread*)args;
	PeekMessage(&msg,NULL,WM_USER, WM_USER, PM_NOREMOVE);///用该函数强制系统创建消息队列
	if(pWorkThread->EnEvent()==false){
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

bool CWorkThread::DeleteThread()
{
	if(m_dwThreadID){
		bool Ret;
		//CLogMsg::PrintDebugString("Wait Thread ID=0x%X QUIT 0000000",m_dwThreadID);	
		Ret = PostMsg(WM_QUIT, 0, 0);
		if(Ret == false){
			ushort ErrNo = GetLastError();
			//CLogMsg::PrintDebugString("Thread post message WM_QUIT failed, ErrNo=0x%x", ErrNo);
		}
		else{
			//CLogMsg::PrintDebugString("Wait Thread ID=0x%X QUIT 111111",m_dwThreadID);	
			WaitForSingleObject(m_hThread,INFINITE);
			m_dwThreadID=0;
		}
		//CLogMsg::PrintDebugString("Wait Thread ID=0x%X QUIT 222222",m_dwThreadID);	

	}
	return true;
}