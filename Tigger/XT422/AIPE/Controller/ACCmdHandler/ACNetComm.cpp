#include "ACNetComm.h"
#include <QObject>
#include <QDebug>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include "AngkLogger.h"
#include "Thread/ThreadPool.h"
#pragma comment(lib,"ws2_32.lib")

extern Acro::Thread::ThreadPool g_ThreadPool;

CNetComm::CNetComm()
	:m_LocalSocket(INVALID_SOCKET)
	,m_pCmdHandler(NULL)
{
	SetSocketOpts(NULL);
	SetIPInfo(NULL, NULL);
}



void CNetComm::AttachICmdHandler(ICmdHandler* pCmdHandler)
{
	m_pCmdHandler = pCmdHandler;
}

int CNetComm::SetSocketOpts(tSocketOpts* pSocketOpts)
{
	tSocketOpts* pDestSocketOpts = &m_SocketOpts;
	if (pSocketOpts) {
		memcpy(&m_SocketOpts, pSocketOpts, sizeof(tSocketOpts));
	}
	else {
		pDestSocketOpts->RecvBufSize = 128 * 1024 * 1024;
		pDestSocketOpts->SendBufSize = 128 * 1024 * 1024;
		pDestSocketOpts->RecvTimeoutms = 5000;
		pDestSocketOpts->SendTimeoutms = 5000;
		pDestSocketOpts->bReuseaddr = true;
		pDestSocketOpts->ZeroCopy = 0;
	}
	return 0;
}

int32_t CNetComm::SendData(std::string devIP, uint8_t* pData, int32_t Size)
{
	int32_t Ret = 0;
	int32_t BytesSend = 0;
	if (m_LocalSocket == INVALID_SOCKET){
		Ret = ERR_NETCOMM_SocketInvalid;
		goto __end;
	}
	m_RemoteAddr.sin_addr.S_un.S_addr = inet_addr(devIP.data());
	BytesSend = sendto(m_LocalSocket,(char*)pData, Size, 0, reinterpret_cast<sockaddr*>(&m_RemoteAddr), sizeof(sockaddr));
	if (BytesSend <= 0) {
		Ret = ERR_NETCOMM_SendData;
		goto __end;
	}
	else if (BytesSend < 1000)
	{
		/*QByteArray buffer;
		buffer.resize(ret);
		memcpy(buffer.data(), _p, ret);
		emit appEvent_ptr.get()->debugInfoResponse(buffer.toHex());*/
	}

__end:
	return Ret;
}

void CNetComm::ThreadExit(int RetCode)
{
	ALOG_INFO("ThreadExit RetCode:%d", "CU", "--", RetCode);

}

int32_t CNetComm::SetIPInfo(tIPInfo*pRemote, tIPInfo*pLocal)
{
	tIPInfo* pDestRemote = &m_Remote;
	tIPInfo* pDestLocal = &m_Local;
	if (pRemote) {
		memcpy(pDestRemote, pRemote, sizeof(tIPInfo));
	}
	else {
		pDestRemote->Addr = "192.168.11.1";
		pDestRemote->Port = 8080;
	}
	if (pLocal) {
		memcpy(pDestLocal, pLocal, sizeof(tIPInfo));
	}
	else {
		pDestLocal->Addr = "192.168.11.2";
		pDestLocal->Port = 8081;
	}
	return 0;
}


int32_t CNetComm::CleanNetWork()
{
	closesocket(m_LocalSocket);
	//WSACleanup();
	m_LocalSocket = INVALID_SOCKET;
	return 0;
}

int32_t CNetComm::SetupNetWork()
{
	int32_t Ret=0;
	int32_t SktErr = 0;
	tSocketOpts* pDestSocketOpts = &m_SocketOpts;
	////初始化网络环境
	//WSADATA wsa;
	//if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0){
	//	ALOG_FATAL("WSAStartup failed.", "CU", "--");
	//	Ret = ERR_NETCOMM_WSAStartup;
	//	goto __end;
	//}

	//建立一个UDP的socket
	m_LocalSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_LocalSocket == SOCKET_ERROR){
		ALOG_FATAL("Create UDP Socket failed.", "CU", "--");
		Ret = ERR_NETCOMM_CreateSocket;
		goto __end;
	}
	//绑定地址信息
	m_LocalAddr.sin_family = AF_INET;
	m_LocalAddr.sin_port = htons(m_Local.Port);
	m_LocalAddr.sin_addr.S_un.S_addr = inet_addr(m_Local.Addr.toStdString().data());


	m_RemoteAddr.sin_family = AF_INET;
	m_RemoteAddr.sin_port = htons(m_Remote.Port);
	//m_RemoteAddr.sin_addr.S_un.S_addr = inet_addr(m_Remote.Addr.toStdString().data());

	ALOG_DEBUG("Establishing local udp socket connection %s:%d.", "CU", "FP", m_Local.Addr.toStdString().data(), m_Local.Port);
	SktErr= bind(m_LocalSocket, (struct sockaddr*)&m_LocalAddr, sizeof(struct sockaddr));
	if (SktErr != 0) {
		ALOG_FATAL("Bind UDP Socket Error.", "CU", "--");
		Ret = ERR_NETCOMM_BindSocket;
		goto __end;
	}
	/*在send()的时候，返回的是实际发送出去的字节(同步)或发送到socket缓冲区的字节
	(异步);系统默认的状态发送和接收一次为8688字节(约为8.5K)；在实际的过程中发送数据
	和接收数据量比较大，可以设置socket缓冲区，而避免了send(),recv()不断的循环收发：*/
	SktErr =setsockopt(m_LocalSocket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&pDestSocketOpts->RecvBufSize), sizeof(int32_t));
	if (SktErr != 0) { 
		ALOG_FATAL("SetSockOpt SO_RCVBUF Error.", "CU", "--");
		Ret = ERR_NETCOMM_SetSktOpts;
		goto __end;
	}
	SktErr = setsockopt(m_LocalSocket, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char*>(&pDestSocketOpts->SendBufSize), sizeof(int32_t));
	if (SktErr != 0) {
		ALOG_FATAL("SetSockOpt SO_SNDBUF Error.", "CU", "--");
		Ret = ERR_NETCOMM_SetSktOpts;
		goto __end;
	}
	//closesocket（一般不会立即关闭而经历TIME_WAIT的过程）后想继续重用该socket：
	SktErr = setsockopt(m_LocalSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&pDestSocketOpts->bReuseaddr, sizeof(bool));
	if (SktErr != 0) {
		ALOG_FATAL("SetSockOpt SO_REUSEADDR Error.", "CU", "--");
		Ret = ERR_NETCOMM_SetSktOpts;
		goto __end;
	}
	//在send(), recv()过程中有时由于网络状况等原因，发收不能预期进行, 而设置收发时限：
	SktErr = setsockopt(m_LocalSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&pDestSocketOpts->SendTimeoutms, sizeof(int32_t)); //发送时限
	if (SktErr != 0) {
		ALOG_FATAL("SetSockOpt SO_SNDTIMEO Error.", "CU", "--");
		Ret = ERR_NETCOMM_SetSktOpts;
		goto __end;
	}

	SktErr = setsockopt(m_LocalSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&pDestSocketOpts->RecvTimeoutms, sizeof(int32_t)); //接收时限
	if (SktErr != 0) {
		ALOG_FATAL("SetSockOpt SO_RCVTIMEO Error.", "CU", "--");
		Ret = ERR_NETCOMM_SetSktOpts;
		goto __end;
	}
	
	//如果在发送数据的时，希望不经历由系统缓冲区到socket缓冲区的拷贝而影响程序的性能：
	//SktErr = setsockopt(m_Socket, SOL_SOCKET, SO_SNDBUF, (const char*)&pDestSocketOpts->ZeroCopy, sizeof(int32_t));
	//同上在recv()完成上述功能(默认情况是将socket缓冲区的内容拷贝到系统缓冲区)：
	//SktErr = setsockopt(m_Socket, SOL_SOCKET, SO_RCVBUF, (const char*)&pDestSocketOpts->ZeroCopy, sizeof(int32_t));
__end:
	if (SktErr) {
		ALOG_FATAL("SocketErrorCode:%d.", "CU", "--", WSAGetLastError());
	}
	else {
		ALOG_DEBUG("Establishing local socket connection successfully.", "CU", "--");
	}

	return Ret;
}

int32_t CNetComm::StartComm()
{
	int32_t Ret = 0;
	Ret = SetupNetWork();
	//将工作函数和QThread的信号相关联
	QObject::connect(&m_WorkThread, &QThread::started, this, &CNetComm::ThreadHandler);

	//将信号和QThread的exit关联
	QObject::connect(this, &CNetComm::sigWorkFinished,this, &CNetComm::ThreadExit, Qt::DirectConnection);
	QObject::connect(this, &CNetComm::sigWorkFinished, &m_WorkThread, &QThread::exit,Qt::DirectConnection);//采用直接方式避免线程主题被先释放

	this->moveToThread(&m_WorkThread);

	m_bStopThead = false;
	m_WorkThread.start();
	//m_NetThread.moveToThread()
	return Ret;
}

int32_t CNetComm::StopComm()
{
	int32_t Ret = 0;
	int32_t Timeoutms = 3000;
	m_bStopThead = true;
	m_WorkThread.wait(Timeoutms);

	CleanNetWork();
	return Ret;
}

int32_t CNetComm::HandleMsg(std::string recvDevIP, uint8_t* PckData, int32_t PckSize)
{
	int32_t Ret = 0;
	if (m_pCmdHandler) {
		Ret=m_pCmdHandler->HandlePacket(recvDevIP, PckData, PckSize);
	}

	return Ret;
}

void CNetComm::ThreadHandler()
{
	int32_t Ret = 0;
	int32_t CurIdx=0;
	int32_t TimeRuns = 2;
	int test = 0;
	int32_t nSize = sizeof(sockaddr_in);
	uint8_t* TmpDataBuf = (uint8_t *)SysMalloc(MaxNetDataSize);
	if (TmpDataBuf == NULL) {
		Ret = ERR_MemoryAlloc;
		goto __end;
	}
	while (!m_bStopThead){
		SOCKADDR clntAddr;  //客户端地址信息
		sockaddr_in addrclnt;
		memset(TmpDataBuf, 0, MaxNetDataSize);

		//UDP协议不需要处理粘包问题，整包不会给你分割
		int32_t BytesRead = recvfrom(m_LocalSocket, (char*)TmpDataBuf, MaxNetDataSize, 0, (SOCKADDR*)&addrclnt, &nSize);
		if (BytesRead>=0){
			char clientIP[256];
			inet_ntop(AF_INET, &addrclnt.sin_addr, clientIP, 256);
			std::string recvDevIP = clientIP;
			int32_t Ret = 0;
			uint8_t MsgID = TmpDataBuf[0];
			HandleMsg(recvDevIP, TmpDataBuf, BytesRead);
		}
		else{
			//错误码对应信息 https://learn.microsoft.com/zh-cn/windows/win32/winsock/windows-sockets-error-codes-2
			//_PrintLog(LOGLEVEL_E, "Recv Data Error:%d", WSAGetLastError());
		}
	}

__end:
	SysSafeFree(TmpDataBuf);

	emit sigWorkFinished(Ret);
}
