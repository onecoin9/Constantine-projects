#ifndef _STSOCKET_H_
#define _STSOCKET_H_
#include <string>
#include <WinSock2.h>
#include <mutex>

#define SIO_RCVALL            _WSAIOW(IOC_VENDOR,1)
#define SIO_RCVALL_MCAST      _WSAIOW(IOC_VENDOR,2)
#define SIO_RCVALL_IGMPMCAST _WSAIOW(IOC_VENDOR,3)
#define SIO_KEEPALIVE_VALS    _WSAIOW(IOC_VENDOR,4)
#define SIO_ABSORB_RTRALERT   _WSAIOW(IOC_VENDOR,5)
#define SIO_UCAST_IF          _WSAIOW(IOC_VENDOR,6)
#define SIO_LIMIT_BROADCASTS _WSAIOW(IOC_VENDOR,7)
#define SIO_INDEX_BIND        _WSAIOW(IOC_VENDOR,8)
#define SIO_INDEX_MCASTIF     _WSAIOW(IOC_VENDOR,9)
#define SIO_INDEX_ADD_MCAST   _WSAIOW(IOC_VENDOR,10)
#define SIO_INDEX_DEL_MCAST   _WSAIOW(IOC_VENDOR,11)

struct tcp_keepalive {
	u_long onoff;    // 是否开启 keepalive
	u_long keepalivetime; //// 多长时间（ ms ）没有数据就开始 send 心跳包
	u_long keepaliveinterval;  // 每隔多长时间（ ms ） send 一个心跳包，
};

class CSTSocket
{
public:
	CSTSocket();
	~CSTSocket();
	///初始化Socket环境，需要先被调用
	static bool InitSocket();
	///释放Socket环境，最后调用
	static bool UninitSocket();

	bool Create(int nSktType=SOCK_STREAM,int nSktProtocal=IPPROTO_TCP);
	bool Close();
	void SetLenByteSize(int byteLen) { m_LenByteSize = byteLen; }
	bool Bind(uint32_t nPort,std::string strIP);
	bool Listen(int BackLog);
	bool Accept(CSTSocket& rContSocket,SOCKADDR* lpSockAddr = NULL, int* lpSockAddrLen = NULL);

	bool Connect(std::string strIP,uint32_t nPort);
	
	bool SetSockOpt(int nOptionName, const void* lpOptionValue,
		int nOptionLen, int nLevel = SOL_SOCKET);
	bool SetSockOptKeepAlive(ULONG AliveTime,ULONG AliveInterval);

	/*****************
	接收数据，成功返回实际发送字节数
	返回0表示Socket关闭
	失败返回-1;
	*********************/
	int Receive(uint8_t*pData,int Size);
	int RawReceive(uint8_t*pData,int Size);

	/*****************
	发送数据，成功返回实际发送字节数
	失败返回-1;
	*********************/
	int Send(uint8_t*pData,int Size);
	std::string GetErrMsg();
	int GetLastError();

	void AttachHandle(SOCKET Sock){m_sock=Sock;}
	SOCKET FromHandle(){return m_sock;}
private:
	std::mutex m_SktMutex;
	bool Socket(int nSktType,int nSktProtocal);
	SOCKET m_sock;
	std::string m_strErrMsg;
	int m_LenByteSize;//默认是1
};
#endif