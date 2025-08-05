#ifndef _ACCMDHANDLER_H_
#define _ACCMDHANDLER_H_

#include "ACTypes.h"
#include "ILog.h"
#include <QObject>
#include "CmdHandler.h"
#include "ACCmdPacket.h"
#include <QHash>
#include <QJsonValue>

typedef QHash<QString, QJsonValue> tJsonParaHash;
#define SSD_PCMUDataExchangeSize 		(0x10000000UL)    //256M
#define SSD_PC2MUDataExchangeOffset		(0x4100000000UL)  //256G+4G位置
#define SSD_MU2PCDataExchangeOffset		(SSD_PC2MUDataExchangeOffset+SSD_PCMUDataExchangeSize)  //256G+4G+256M位置

//ACCmdHandler为应用命令协议层，CCmdHandler为基础协议命令层
//当最近一次通过RemoteDoCmd发送的命令请求被执行完成之后，会调用这个回调函数告知命令执行结果，如果有返回信息则RespDataBytes内有值
typedef int32_t (*FnCBKCmdComplete)(void*PrivData,uint32_t HopNum, uint32_t PortID,int32_t ResultCode, QByteArray& RespDataBytes);

typedef int32_t (*FnCBKQueryDoCmd)(void* PrivData, uint32_t HopNum, uint32_t PortID,uint16_t CmdID,QByteArray& CmdDataBytes);
class CACCmdHandler: public QObject
{
	Q_OBJECT
public:
	CACCmdHandler();
	void AttachILog(ILog* pLog);
	void AttachCmdHandler(CCmdHandler *pCmdHandler);
	void AttachAppModels(CAppModels* pAppModels);
	void RegistCbkCmdComplete(void*PrivData, FnCBKCmdComplete fnCbkComplete);
	void RegistCbkQueryDoCmd(void* PrivData, FnCBKQueryDoCmd fnCbkQueryDoCmd);
	/// <summary>
	/// 请求远端执行执行的命令包
	/// </summary>
	/// <param name="HopNum">命令请求节点号</param>
	/// <param name="PortID">命令请求端口号</param>
	/// <param name="CmdFlag">命令标识，CmdFlag_Notify等</param>
	/// <param name="CmdID">命令码</param>
	/// <param name="pCmdData">命令数据</param>
	/// <param name="CmdDataSize">命令数据大小，字节为单位</param>
	/// <returns>发送成功返回0，错误返回负数，如果该命令有结果需要返回，则通过onPTPacketExecComplete槽函数</returns>
	int32_t RemoteDoPTCmd(uint32_t HopNum, uint32_t PortID, uint32_t CmdFlag, uint16_t CmdID, uint32_t SktEn, uint16_t BPUID, QByteArray& CmdDataBytes);

	/// <summary>
	/// 当设备(AG06)请求PC执行某个功能之后，如果有需要返回给AG06响应数据或结果时，通过这个函数向下发送数据
	/// </summary>
	/// <param name="HopNum">命令请求节点号</param>
	/// <param name="PortID">命令请求端口号</param>
	/// <param name="ResultCode">请求执行的命令结果</param>
	/// <param name="RespData">请求执行的命令需要的响应数据</param>
	/// <returns>0表示发送成功，负数表示发送失败</returns>
	int32_t SendDevCallBackResult(uint32_t HopNum, uint32_t PortID, int32_t ResultCode, QByteArray RespData);


	/// <summary>
	/// 告知AG06将指定位置的SSD数据读取到DDR中，
	/// </summary>
	/// <param name="HopNum">请求发送的节点号，也就是AG06位置</param>
	/// <param name="PortID">对应的端口</param>
	/// <param name="SrcSSDAddr">指定SDD的读取位置，要为4K的整数倍</param>
	/// <param name="DestDDRAddr">请求存在数据的DDR位置，也有约束，</param>
	/// <param name="DataSize">数据长度</param>
	/// <returns>成功返回0，失败返回负数</returns>
	int32_t SSD2DDR(uint32_t HopNum, uint32_t PortID, uint64_t SrcSSDAddr, uint64_t DestDDRAddr, uint64_t DataSize);

	/// <summary>
	/// 告知AG06将指定位置的DDR数据存放到SSD指定的位置中，
	/// </summary>
	/// <param name="HopNum">请求发送的节点号，也就是AG06位置</param>
	/// <param name="PortID">对应的端口</param>
	/// <param name="DestSSDAddr">指定SDD的写入位置，要为4K的整数倍</param>
	/// <param name="SrcDDRAddr">请求读取的DDR位置，也有约束，</param>
	/// <param name="DataSize">数据长度</param>
	/// <returns>成功返回0，失败返回负数</returns>
	int32_t DDR2SSD(uint32_t HopNum, uint32_t PortID, uint64_t SrcDDRAddr, uint64_t DestSSDAddr, uint64_t DataSize);

	/// <summary>
	/// 获取容量
	/// </summary>
	/// <param name="Type">类型字符串，可以为SSD，DDR或SKT表示读取SSD或DDR的容量</param>
	/// <param name="HopNum">请求发送的节点号，也就是AG06位置</param>
	/// <param name="PortID">填充为0</param>
	/// <returns>成功返回0，失败返回负数，真正返回容量大小是通过sigGetCapacityComplete信号发送</returns>
	int32_t GetCapacity(QString Type,uint32_t HopNum,uint32_t PortID);

	/// <summary>
	/// 读取数据SSD或DDR的数据并保存到指定的文件
	/// </summary>
	/// <param name="strFilePath">保存的档案位置</param>
	/// <param name="Type">类型字符串，可以为SSD2FIBER,DDR2FIBER</param>
	/// <param name="HopNum">请求发送的节点号，也就是AG06位置</param>
	/// <param name="PortID">填充为0</param>
	/// <param name="SrcAddr">请求读取的地址位置，字节为单位</param>
	/// <param name="Length">请求读取的长度，字节为单位</param>
	/// <param name="ResultHash">反馈的结果，放在这个Hash里</param>
	/// <returns>成功返回0，失败返回负数</returns>
	int32_t ReadDataAndSaveToFile(QString strFilePath, QString Type, uint32_t HopNum, uint32_t PortID, uint64_t SrcAddr, uint64_t Length, tJsonParaHash& ResultHash);

	/// <summary>
	/// 读取指定档案的数据将其写入到SSD或DDR中
	/// </summary>
	/// <param name="strFilePath">需要读取的档案路径</param>
	/// <param name="Type">类型字符串，可以为FIBER2SSD,FIBER2DDR</param>
	/// <param name="HopNum">请求发送的节点号，也就是AG06位置</param>
	/// <param name="PortID">填充为0</param>
	/// <param name="DestAddr">希望写入的地址位置，字节为单位，SSD需要4K对齐，Length也要是4K的整数倍,这个长度大小由文件大小决定</param>
	/// <param name="ResultHash">反馈的结果，放在这个Hash里</param>
	/// <returns>成功返回0，失败返回负数</returns>
	int32_t WriteDataFromFile(QString strFilePath, QString Type, uint32_t HopNum, uint32_t PortID, uint64_t DestAddr, tJsonParaHash& ResultHash);

	/// <summary>
	/// 将数据存放到SSD中
	/// </summary>
	/// <param name="HopNum">请求发送的节点号，也就是AG06位置</param>
	/// <param name="PortID">对应的端口</param>
	/// <param name="SSDAddr">数据被写入到SSD的哪个位置</param>
	/// <param name="pCmdData">请求存放的数据</param>
	/// <param name="CmdDataSize">数据长度</param>
	/// <returns>成功返回0，失败返回负数</returns>
	int32_t StoreDataToSSD(uint32_t HopNum, uint32_t PortID, uint64_t SSDAddr, QByteArray& DataBytes);
	int32_t ReadDataFromSSD(uint32_t HopNum, uint32_t PortID, uint64_t SSDAddr, int32_t BytesToRead, QByteArray& DataBytes);

	int32_t LinkScan(uint32_t HopNum);
public slots:
	//透传包执行完成,发出该信号，HopNum完成命令的节点号，PortID，对应的PortID，ACCmdCompletePack为结果包
	void onPTPacketExecComplete(qint32 HopNum, qint32 PortID, QByteArray ACCmdCompletePack);
	//AG06请求执行透传包命令,发出该信号，HopNum完成命令的节点号，PortID，对应的PortID，ACCmdQueryPack为请求包
	void onQueryDoPTCmd(qint32 HopNum, qint32 PortID, QByteArray ACCmdQueryPack);

signals:
	//当AG06完成RemoteDoCmd命令之后，如果没有通过RegistCbkCmdComplete注册回调函数，则会发送sigCmdComplete信号
	void sigRemoteCmdComplete(uint32_t HopNum, uint32_t PortID, int32_t ResultCode, QByteArray RespDataBytes);
	//当AG06有请求PC的命令到达之后，如果没有通过RegistCbkQueryDoCmd注册回调函数，则会发送sigRemoteQueryDoCmd信号
	void sigRemoteQueryDoCmd(uint32_t HopNum, uint32_t PortID,uint32_t CmdFlag, uint16_t CmdID, QByteArray CmdDataBytes);
protected:

private:
	ILog* m_pILog;
	CCmdHandler* m_pCmdHandler;
	CAppModels* m_pAppModels;
	void* m_PrivDataCbkComplete;
	FnCBKCmdComplete m_fnCbkComplete;
	void* m_PrivDataCbkQueryDoCmd;
	FnCBKQueryDoCmd m_fnCbkQueryDoCmd;
};


#endif 