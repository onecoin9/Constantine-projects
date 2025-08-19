#pragma once

#include <QObject>
#include <memory>
#include <thread>
#include <QDebug>
#include "safe_queue_base.h"
#include "thread_pool.h"
#include "ACCmdHandler.h"

#define THREAD_NUMS 5

//AngKMessageHandler 作为一个应用通信转发层发送至昂科通信协议层

class ACCmdHandler;
class AngKMessageHandler : public QObject
{
	Q_OBJECT

public:
	static AngKMessageHandler& instance()
	{
		if (!s_instance)
		{
			std::call_once(once_flag, []() { s_instance = new AngKMessageHandler(); });
		}
		return *s_instance;
	}
	//一个初始化启动函数，将创建成员变量在启动时直接启用
	void Command_Initialize(QWidget* widget = nullptr);

	//CACCmdHandler信号需要通过成员变量实现
	CACCmdHandler* GetACCmdHandler() { return m_pACmdHandler; }

	//请求远端执行执行的命令包
	int32_t Command_RemoteDoPTCmd(std::string devIP, uint32_t HopNum, uint32_t PortID, uint32_t CmdFlag, uint16_t CmdID, uint32_t SKTNum, uint16_t BPUID, QByteArray& CmdDataBytes);

	//当设备(AG06)请求PC执行某个功能之后，如果有需要返回给AG06响应数据或结果时，通过这个函数向下发送数据
	int32_t Command_SendDevCallBackResult(std::string devIP, uint32_t HopNum, uint32_t PortID, int32_t ResultCode, QByteArray RespData);

	//告知AG06将指定位置的SSD数据读取到DDR中，
	int32_t Command_SSD2DDR(std::string devIP, uint32_t HopNum, uint32_t PortID, uint64_t SrcSSDAddr, uint64_t DestDDRAddr, uint64_t DataSize);

	/// 告知AG06将指定位置的DDR数据存放到SSD指定的位置中，
	int32_t Command_DDR2SSD(std::string devIP, uint32_t HopNum, uint32_t PortID, uint64_t SrcDDRAddr, uint64_t DestSSDAddr, uint64_t DataSize);

	// 获取容量
	int32_t Command_GetCapacity(std::string devIP, QString Type, uint32_t HopNum, uint32_t PortID);

	// 读取数据SSD或DDR的数据并保存到指定的文件
	int32_t Command_ReadDataAndSaveToFile(std::string devIP, QString strFilePath, QString Type, uint32_t HopNum, uint32_t PortID, uint64_t SrcAddr, uint64_t Length, uint64_t fileOffset, tJsonParaHash& ResultHash);

	// 读取指定档案的数据将其写入到SSD或DDR中
	int32_t Command_WriteDataFromFile(std::string devIP, QString strFilePath, QString Type, uint32_t HopNum, uint32_t PortID, uint64_t DestAddr, tJsonParaHash& ResultHash);

	// 将数据存放到SSD中
	int32_t Command_StoreDataToSSDorDDR(std::string devIP, QString Type, uint32_t HopNum, uint32_t PortID, uint64_t SSDAddr, QByteArray& DataBytes, std::string strProgSN, std::string strDataType);
	int32_t Command_ReadDataFromSSD(std::string devIP, QString Type, uint32_t HopNum, uint32_t PortID, uint64_t SSDAddr, int32_t BytesToRead, QByteArray& DataBytes, std::string strProgSN, std::string strDataType);
	
	// 高性能版本：去掉CRC校验等耗时操作，尽可能减少拷贝
	int32_t Command_ReadDataFromSSD2(std::string devIP, QString Type, uint32_t HopNum, uint32_t PortID, uint64_t SSDAddr, int32_t BytesToRead, QByteArray& DataBytes);

	//获取AG06链路扫描包信息
	int32_t Command_LinkScan(std::string devIP, uint32_t HopNum);

	//程序关闭的时候关闭所有socket
	int32_t Command_CloseProgramSocket();
private:
	AngKMessageHandler() 
	: m_pACmdHandler(nullptr)
	{
		if(m_pACmdHandler == nullptr)
			m_pACmdHandler = new CACCmdHandler();
	};
	~AngKMessageHandler() 
	{
		if (m_pACmdHandler != nullptr)
		{
			m_pACmdHandler = nullptr;
			delete m_pACmdHandler;
		}
	};

private:
	CACCmdHandler* m_pACmdHandler;
	static AngKMessageHandler* s_instance;
	static std::once_flag once_flag;
	std::queue<CACCmdHandler*> m_pQueueHandler;
};