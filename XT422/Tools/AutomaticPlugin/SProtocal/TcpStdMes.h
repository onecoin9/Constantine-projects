#pragma once

#include "STSocket.h"
#include "PckMsg.h"
#include "../ComStruct/WorkThread.h"
#include "ComDelegate.h"

#define PCKDATA_BUFLEN			(4096)
#define CURRENT_VERSION			(0x30)
#define PFLAG_AS				(0x4153)		///AutoApp To A
#define PFLAG_SA				(0x5341)

#define DPU_TELLAUTOSITERDY			(0x63)			///告知自动化站点初始化情况
#define PDU_QUERYSITEEN				(0x64)			///请求自动化告知站点使能 
#define PDU_TELLSITEEN				(0x64|0x80)		///自动化告知站点使能
#define PDU_PUTCHIPS				(0x66)			///请求自动化开始放置芯片
#define PDU_TELLCHIPSEN				(0x66|0x80)		///自动化告知芯片放置情况
#define PDU_TELLREUSLTS				(0x67)			///告知自动化烧录结果
#define PDU_TELLICSTATUS			(0x68)			///告诉自动化IC的状态
#define PDU_QUERYICSTATUS			(0x68|0x80)		///自动请求确认IC状态，是否在座子里
	
#define EC_OK		(0x00)			///正常
#define EC_FAIL		(0x01)			///出现错误
#define EC_PDUERR	(0x02)			///PDU不支持
#define EC_CRC		(0x03)			///CRC错误


typedef struct tagSktOpt{
	uint32_t Port;
	std::string strIPAddr;
	CSTSocket Socket;
}tSktOpt;

class CTcpStdMes
{
public:
	CTcpStdMes(void);
	~CTcpStdMes(void);
	/************************
	初始化StdMes的TCP通讯环境
	成功返回0 ， m_strErrMsg填充错误码
	失败返回-1
	*************************/
	int Init();

	void CloseClientSocket();
	int InitClientSocket();


	/***************************
	释放StdMes的TCP通信环境
	*****************************/
	int Uninit();

	///启动服务器端口接收数据包任务，不需要外部
	int DoJob();

	/********************
	客户端向服务器发送一个包消息
	-1表示失败,
	0表示成功
	********************/
	int ClientSendPck(uint16_t PFlag,uint8_t Pdu, int PLen,uint8_t *pData);
	/********************
	客户端接收一个包消息
	-1表示失败,m_strErrMsg填充错误消息
	0表示Socket终止
	1表示获取包消息成功
	********************/
	int ClientRecvPck(CPackMsg&PckMsg);
	/********************
	客户端接收Ack包，
	-1表示失败
	0表示Socket终止
	1表示获取包消息成功,ErrCode填充错误码
	********************/
	int ClientGetAck(uint8_t&ErrCode, CPackMsg* pckMsg = nullptr);

	bool IsReady(){return m_bTcpStdMesRdy;}

	int ServerSendAck(CPackMsg&PckMsg, uint8_t ErrCode);
	int ServerSendAck(uint8_t Pdu, int PLen, uint8_t *pData);

	std::string GetErrMsg(){return m_strErrMsg;}
	CEventSource m_EventSiteEn;
	CEventSource m_EventChipsEn;
	CEventSource m_EventQueryICStatus;
	CEventSource m_EventDumpPck;
	CEventSource m_EventDumpMsg;
	CEventSource m_EventHandlePck;
	CEventSource m_EventHandleProgramResultBack;
	int UseNewVesion();
	int IsAutoSupportNewProtocal();
private:
	int RecvPck(CSTSocket&Socket,CPackMsg&PckMsg);
	int SendPck(CSTSocket&Socket,uint16_t PFlag,uint8_t Pdu, int PLen,uint8_t *pData);
	int HandlePck(CSTSocket &Socket,uint8_t*pData,int Size);
	int SendPckAck(CSTSocket &Socket,CPackMsg&PckMsg,uint8_t ErrCode);
	int InitServerSocket();

private:
	bool m_bTcpStdMesRdy;
	CSTSocket m_SktAccept;
	tSktOpt m_SktClient;
	tSktOpt m_SktServer;
	std::string m_strErrMsg;
	CWorkThread m_wtServer;
	//CSocket m_SktServerWt; /// WorkThread中的Skt
	volatile bool m_bExit; ///是否退出操作
	int m_TryCntMax;		////尝试的最大次数，默认为3
	int m_TryDelayTime;	///两次尝试之间的延时 ms为单位，默认为200ms
};
