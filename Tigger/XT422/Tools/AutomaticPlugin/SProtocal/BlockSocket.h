#ifndef _BLOCK_SOCKET_H_
#define _BLOCK_SOCKET_H_

#include <WinSock2.h>
#include <mutex>

class CBlockSocket
{
public:
	CBlockSocket();
	~CBlockSocket();
	///初始化Socket环境，需要先被调用
	static bool InitSocket();
	///释放Socket环境，最后调用
	static bool UninitSocket();

	bool Create(int nSktType=SOCK_STREAM,int nSktProtocal=IPPROTO_TCP);
	bool Close();
	
	bool Bind(UINT nPort,std::string strIP);
	bool Listen(int BackLog);
	bool Accept(CBlockSocket& rContSocket,SOCKADDR* lpSockAddr = NULL, int* lpSockAddrLen = NULL);

	bool Connect(std::string strIP,UINT nPort);
	
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
};
#endif