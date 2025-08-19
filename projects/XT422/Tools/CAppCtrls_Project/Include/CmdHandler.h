#ifndef _CMDHANDLER_H_
#define _CMDHANDLER_H_

#include "IACComm.h"
#include "ICmdHandler.h"
#include "ICrcCheck.h"
#include "AppModels.h"
#include <QSemaphore>
#include <QMutex>
#include "ACCmdPacket.h"

#define CmdPTACK_SiteSemaMax		(8)			//每个Programmer有几个Site
#define CmdPTACK_ProgrammerSemaMax  (16)		//最大有几个Programmer
#define CmdQueue_MaxSize  (32)			//Device端最多能够处理的命令数
#define CmdACK_TimeOutms  (1000)		//100ms内需要有ACK

class CCmdHandler : public ICmdHandler
{
	Q_OBJECT
public:
	CCmdHandler();
	void AttachAppModels(CAppModels* pAppModels);
	void AttachIACComm(IACComm* pIACComm) {
		m_pIACComm = pIACComm;
	}
	int32_t HandlePacket(uint8_t* pPckData, int Size);

	int32_t SendInterruptACK(tInterruptPacket* pInterruptPck);
	int32_t SendLinkScanPacket(tLinkScanPacket* pLinkScanPacket);
	int32_t SendDevInfoGetPacket(tDevInfoGetPacket* pDevInfoGetPacket);

	int32_t SendCmdPacketDataTrans(tCmdPacketDataTrans* pCmdPacketDataTrans);
	int32_t SendCmdGetCapacity(tCmdPacketGetCapacity* pCmdPacketGetCapacity);
	int32_t SendCmdRegister(tCmdPacketRegister* pCmdPacketRegister);
	int32_t SendCmdGetCRC32(tCmdPacketGetCRC32* pCmdPacketGetCRC32, uint32_t& DownlinkCRC, uint32_t& UplinkCRC);

	//获取ACCmd包的最大数据传输量
	int32_t GetACCmdPacketPayloadMax();

	/// <summary>
	/// 发送透传包
	/// </summary>
	/// <param name="pPTPacket"></param>
	/// <returns></returns>
	int32_t SendCmdPTCmd(tCmdPacketPT* pPTPacket);

	/*
	* 发送通用命令
	*/
	int32_t SendCmdCommon(tCmdPacketCommon* pCmdPacketCommon);

	int32_t SendDownlinkData(tDataPacket* pDataPacket);


	bool IsCmdExeQueueEmpty();
	/// <summary>
	/// 等待命令队列可用数为AvailableCnt，可以指定超时时间
	/// </summary>
	/// <param name="Timeout">指定超时时间单位为毫秒，如果指定为负数，则永久等待</param>
	/// <returns></returns>
	bool WaitCmdExeQueueAvailable(int Timeout, int AvailableCnt = CmdQueue_MaxSize);
	bool ReleaseCmdExeQueueSemaphoreWhenFail(int TotalAvailableCnt = CmdQueue_MaxSize);
protected:

	int32_t HandleHeartBeat(uint8_t* pPckData, int Size);
	int32_t HandleDevInfoResp(uint8_t* pPckData, int Size);
	int32_t HandleCmdRecvResp(uint8_t* pPckData, int Size);
	int32_t HandleCmdCompleteResp(uint8_t* pPckData, int Size);
	int32_t HandleInterrupt(uint8_t* pPckData, int Size);
	int32_t HandleDataUplink(uint8_t* pPckData, int Size);
	int32_t HandlePT(uint8_t* pPckData, int Size); //处理透传包

	/// <summary>
	/// 得到容量响应包之后 设置容量
	/// </summary>
	/// <param name="CmdID"></param>
	/// <param name="pCmdCplPacketGetCapacity">命令响应包，包含容量信息</param>
	/// <returns></returns>
	int32_t SetCapacity(uint8_t CmdID, tCmdCplPacketGetCapacity* pCmdCplPacketGetCapacity);
	int32_t SendCmdPacket(uCmdPacket* pCmdPacket);

	/// <summary>
	/// 读取CRC32，CRC32命令完成包到达之后包含了CRC32的信息
	/// </summary>
	int32_t ReceivedCRC32(tCmdCplPacketGetCRC32* pCmdCplGetCRC32);


	
	/// <summary>
	/// 解析PTComplete包，这里可能会涉及到重新发起一个命令请求对SSD数据进行读取
	/// 成功返回0，失败返回负数
	/// </summary>
	int32_t ParserPTComplete(tCmdPacketPT* pPacketPT);
	/// <summary>
	/// 远端请求执行透传包命令，成功返回0，失败返回负数
	/// </summary>
	int32_t QueryDoPTCmd(tCmdPacketPT* pPacketPT);

	int32_t WaitPTACK(uint32_t HopNum, uint32_t PortID);
	int32_t ReleasePTACK(uint32_t HopNum, uint32_t PortID);
signals:
	//当Device的Interrupt到达之后，发送这个信号
	void sigDeviceEventArrive(uint32_t HopNum, uint32_t EventID);

	//这里使用QByteArray是因为数据从Device上传到PC之后，需要发送给其他模块处理，需要考虑到大容量数据间的传递效率
	//根据QT的ByteArray介绍：uses implicit sharing (copy-on-write) to reduce memory usage and avoid needless copying of data.	
	//采用的是隐式共享的方式，所以只要不写那么就不会产生拷贝
	void sigRecvDataUplink(const QByteArray PckData);

	//透传包执行完成,发出该信号，HopNum完成命令的节点号，PortID，对应的PortID，ACCmdCompletePack为结果包
	void sigPTPacketExecComplete(qint32 HopNum, qint32 PortID,QByteArray ACCmdCompletePack);

	//AG06请求执行透传包命令,发出该信号，HopNum完成命令的节点号，PortID，对应的PortID，ACCmdQueryPack为请求包
	void sigQueryDoPTCmd(qint32 HopNum, qint32 PortID, QByteArray ACCmdQueryPack);

	/// <summary>
	/// 当容量到达之后发送该信号
	/// </summary>
	void sigGetCapacityComplete(qint32 HopNum,qint32 CapacityType,quint64 Capacity);
private:
	CAppModels* m_pAppModels;
	IACComm* m_pIACComm;	///通信基础类，用来执行命令
	QSemaphore m_CmdExeSemaphore; //命令包的处理资源数，发送命令包的时候需要先申请，命令包完成后释放，FPGA端最大能并发处理的命令包数量为CmdQueue_MaxSize
									//如果命令包没有ACK这个时候可能也是丢包，则也需要释放
	QMutex m_MutexAck;		//命令包发送之后，需要等待回应，用这个互斥机制来确认命令包的ACK，需要有超时
	quint32 m_CmdAckTimeoutms; 
	QMutex m_MutexCRC;	//CRC32的互斥锁，需要等到CRC32的Complete包到达之后才能往下走，有超时
	//QMutex m_MutexPTAck;	///透传包发送之后，需要等待回应，用这个互斥机制来确认透传包的ACK，需要有超时
	QSemaphore m_PTACKSemaphore[CmdPTACK_ProgrammerSemaMax];
	volatile uint32_t m_UplinkCRC;
	volatile uint32_t m_DownlinkCRC;
};

#endif