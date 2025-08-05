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
	u_long onoff;    // �Ƿ��� keepalive
	u_long keepalivetime; //// �೤ʱ�䣨 ms ��û�����ݾͿ�ʼ send ������
	u_long keepaliveinterval;  // ÿ���೤ʱ�䣨 ms �� send һ����������
};

class CSTSocket
{
public:
	CSTSocket();
	~CSTSocket();
	///��ʼ��Socket��������Ҫ�ȱ�����
	static bool InitSocket();
	///�ͷ�Socket������������
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
	�������ݣ��ɹ�����ʵ�ʷ����ֽ���
	����0��ʾSocket�ر�
	ʧ�ܷ���-1;
	*********************/
	int Receive(uint8_t*pData,int Size);
	int RawReceive(uint8_t*pData,int Size);

	/*****************
	�������ݣ��ɹ�����ʵ�ʷ����ֽ���
	ʧ�ܷ���-1;
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
	int m_LenByteSize;//Ĭ����1
};
#endif