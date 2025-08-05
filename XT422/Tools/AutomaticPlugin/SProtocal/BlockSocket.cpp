
#include "BlockSocket.h"
#include <string>

struct tcp_keepalive {
	u_long onoff;    // 是否开启 keepalive
	u_long keepalivetime; //// 多长时间（ ms ）没有数据就开始 send 心跳包
	u_long keepaliveinterval;  // 每隔多长时间（ ms ） send 一个心跳包，
};

CBlockSocket::CBlockSocket()
:m_sock(INVALID_SOCKET)
,m_strErrMsg("")
{

}

CBlockSocket::~CBlockSocket()
{
	Close();
}

bool CBlockSocket::InitSocket()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD( 1, 1 );
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		return false;
	}
	if ( LOBYTE( wsaData.wVersion ) != 1 ||
		HIBYTE( wsaData.wVersion ) != 1 ) {
			WSACleanup( );
			return false; 
	}
	return true;
}

bool CBlockSocket::UninitSocket()
{
	if(WSACleanup( )==0)
		return true;
	return false;
}



bool CBlockSocket::Socket(int nSktType,int nSktProtocal)
{
	ULONG ul;
	bool Ret=true;
	m_sock = socket(AF_INET,nSktType,nSktProtocal);
	if(m_sock!=INVALID_SOCKET){
		ul = 0;
		if(ioctlsocket(m_sock, FIONBIO, &ul)!=0){ //设置为阻塞模式
			Ret=false;
		}
	}
	return Ret;
}

bool CBlockSocket::Create(int nSktType,int nSktProtocal)
{
	bool Ret=true;
	Close();
	if(Socket(nSktType,nSktProtocal)==false){
		Ret=false;
	}
	return Ret;
}

bool CBlockSocket::Bind(UINT nPort,std::string strIP)
{
	bool Ret=true;
	struct sockaddr_in     servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(strIP.c_str());
	servaddr.sin_port = htons(nPort);

	if(bind(m_sock,(struct sockaddr*)&servaddr, sizeof(servaddr))!=0){
		char buf[100];
		memset(buf, 0, 100);
		sprintf(buf, "Bind Socket Fail, ErrCode=%d",GetLastError());
		m_strErrMsg = buf;
		Ret=false;
	}
	return Ret;
}

bool CBlockSocket::Listen(int BackLog)
{
	bool Ret=true;
	if(listen(m_sock,BackLog)!=0){
		char buf[100];
		memset(buf, 0, 100);
		sprintf(buf, "Listen Socket Fail, ErrCode=%d",GetLastError());
		m_strErrMsg = buf;
		Ret=false;
	}
	return Ret;
}

bool CBlockSocket::Accept(CBlockSocket& rContSocket,SOCKADDR* lpSockAddr, int* lpSockAddrLen)
{
	bool Ret=true;
	SOCKET skt=accept(m_sock,lpSockAddr,lpSockAddrLen);
	if(skt!=INVALID_SOCKET){
		rContSocket.AttachHandle(skt);
	}
	else{
		Ret=false;
	}
	return Ret;
}

bool CBlockSocket::SetSockOpt(int nOptionName, const void* lpOptionValue,
				int nOptionLen, int nLevel)
{
	bool Ret=true;
	if(m_sock!=INVALID_SOCKET){
		if(setsockopt(m_sock,nLevel,nOptionName,(const char*)lpOptionValue,nOptionLen)!=0){
			Ret=false;
		}
	}
	else{
		Ret=false;
	}

	return Ret;
}
///返回接收的字节数
int CBlockSocket::RawReceive(BYTE*pData,int Size)
{
	int Rtn=0;
	int  iRecvLen;
	int uiCompleted = 0;
	int uiPkgSize   = Size; //至少要接收5个字节

	std::lock_guard<std::mutex> lock(m_SktMutex);
	if (m_sock==INVALID_SOCKET ) {
		char buf[100];
		memset(buf, 0, 100);
		sprintf(buf, "NET ParamterError m_sock=0x%X",(UINT)m_sock);
		m_strErrMsg = buf;
		Rtn=-1; goto __end;
	}

	while (uiCompleted < uiPkgSize) {
		iRecvLen = recv(m_sock, (char*)pData+uiCompleted, uiPkgSize-uiCompleted, 0);
		if (iRecvLen == 0) {///Socket被关闭
			m_strErrMsg = "Receive Fail, Socket Close";		
			Rtn=0; goto __end;
		}
		else if(iRecvLen<0){
			char buf[100];
			memset(buf, 0, 100);
			sprintf(buf, "Receive Fail, ErrCode=%d",WSAGetLastError());
			m_strErrMsg = buf;
			Rtn=-1; goto __end;
		}
		uiCompleted += iRecvLen;
	}
	Rtn=uiCompleted;

__end:
	return Rtn;
}

int CBlockSocket::Receive( BYTE*pData,int Size )
{
	int Rtn=0;
	int  iRecvLen;
	int uiCompleted = 0;
	int uiPkgSize   = 8; //至少要接收8个字节

	std::lock_guard<std::mutex> lock(m_SktMutex);
	if (m_sock==INVALID_SOCKET || Size < 8) {
		char buf[100];
		memset(buf, 0, 100);
		sprintf(buf, "NET ParamterError m_sock=0x%X,bytes=%d",(UINT)m_sock,Size);
		m_strErrMsg = buf;
		Rtn=-1; goto __end;
	}

	while (uiCompleted < uiPkgSize) {
		iRecvLen = recv(m_sock, (char*)pData+uiCompleted, uiPkgSize-uiCompleted, 0);
		if (iRecvLen == 0) {///Socket被关闭
			m_strErrMsg = "Receive Fail, Socket Close";		
			Rtn=0; goto __end;
		}
		else if(iRecvLen<0){
			char buf[100];
			memset(buf, 0, 100);
			sprintf(buf, "Receive Fail, ErrCode=%d",WSAGetLastError());
			m_strErrMsg = buf;
			Rtn=-1; goto __end;
		}
		uiCompleted += iRecvLen;
		if (uiCompleted >= 8) {
			uiPkgSize =8+(*(UINT*)(pData + 4)); //协议中从第4个字节表示pData长度
			char buf[100];
			memset(buf, 0, 100);
			sprintf(buf, "Receive, uiPkgSize=%d", uiPkgSize);
			m_strErrMsg = buf;
			if (uiPkgSize > Size) {	//包长度不能超过缓冲区长度
				char buf[100];
				memset(buf, 0, 100);
				sprintf(buf, "NET GetPack Error Size=%d,but the buffer size Max is=%d",uiPkgSize,Size);
				m_strErrMsg = buf;
				Rtn=-1; goto __end;
			}
		}
	}
	Rtn=uiCompleted;

__end:
	return Rtn;
}
#define SIO_KEEPALIVE_VALS    _WSAIOW(IOC_VENDOR,4)
//设置心跳包，AliveTime为ms，多少ms没数据就发送心跳包
//AliveInterval 两个心跳包发送的时间间隔
bool CBlockSocket::SetSockOptKeepAlive(ULONG AliveTime,ULONG AliveInterval)
{
	bool Ret=true;
	tcp_keepalive live,liveout; 
	live.keepaliveinterval=AliveInterval; ///每隔2s发送一个心跳包     
	live.keepalivetime=AliveTime; //30s没有就发送心跳包  
	live.onoff=true;
	bool bKeepAlive = true;
	int iRet = setsockopt(m_sock,SOL_SOCKET,SO_KEEPALIVE,(char *)&bKeepAlive,sizeof(bKeepAlive));
	if(iRet == 0){
		DWORD dw;      
		if(WSAIoctl(m_sock,SIO_KEEPALIVE_VALS, &live,sizeof(live),&liveout,sizeof(liveout),&dw,NULL,NULL)== SOCKET_ERROR){   
			Ret=false;
		}
	}
	else{
		Ret=false;
	}
	return Ret;
}

bool CBlockSocket::Connect(std::string strIP,UINT nPort)
{
	bool Ret=true;

	if(m_sock==INVALID_SOCKET){
		Ret=Create();
		if(Ret==false){
			return Ret;
		}
	}
	struct sockaddr_in serv_addr;

	//以服务器地址填充结构serv_addr
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(strIP.c_str());
	serv_addr.sin_port = htons(nPort);
	int error = -1;
	int len = sizeof(int);
	//timeval tm;
	//fd_set set;
	unsigned long ul = 1;
	//ioctlsocket(sockfd, FIONBIO, &ul); //设置为非阻塞模式
	bool ret = false;
	if( connect(m_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
		Ret=false;
	}
	return Ret;
}

bool CBlockSocket::Close()
{
	if (m_sock!=INVALID_SOCKET) {
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}
	return true;
}

int CBlockSocket::Send(BYTE *pData,int Size)
{
	int Rtn=0;
	int  iSendLen;
	int uiCompleted = 0;;
	//m_SktMutex.Lock();
	if (m_sock==INVALID_SOCKET) {
		char buf[100];
		memset(buf, 0, 100);
		sprintf(buf, "NET ParamterError m_sock=0x%X",(UINT)m_sock);
		m_strErrMsg = buf;
		Rtn=-1; goto __end;
	}
	
	while (uiCompleted < Size) {
		iSendLen = send(m_sock, (char*)pData+uiCompleted, Size-uiCompleted, 0);
		if (iSendLen<0) {
			char buf[100];
			memset(buf, 0, 100);
			sprintf(buf, "Send Data Fail, ErrCode=%d",WSAGetLastError());
			m_strErrMsg = buf;
			Rtn=-1; goto __end;
		}
		uiCompleted += iSendLen;
	}
	Rtn=uiCompleted;

__end:
	//m_SktMutex.Unlock();
	return Rtn;
}


std::string CBlockSocket::GetErrMsg()
{
	return m_strErrMsg;
}

int CBlockSocket::GetLastError()
{
	return WSAGetLastError();
}

