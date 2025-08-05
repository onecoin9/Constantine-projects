#include "AngKMessageHandler.h"
#include "AngkLogger.h"

//静态成员变量初始化
AngKMessageHandler* AngKMessageHandler::s_instance = nullptr;
std::once_flag AngKMessageHandler::once_flag;

int32_t AngKMessageHandler::Command_RemoteDoPTCmd(std::string devIP, uint32_t HopNum, uint32_t PortID, uint32_t CmdFlag, uint16_t CmdID, uint32_t SKTNum, uint16_t BPUID, QByteArray& CmdDataBytes)
{
	return m_pACmdHandler->RemoteDoPTCmd(devIP, HopNum, PortID, CmdFlag, CmdID, SKTNum, BPUID, CmdDataBytes);
}

int32_t AngKMessageHandler::Command_SendDevCallBackResult(std::string devIP, uint32_t HopNum, uint32_t PortID, int32_t ResultCode, QByteArray RespData)
{
	return m_pACmdHandler->SendDevCallBackResult(devIP, HopNum, PortID, ResultCode, RespData);
}

int32_t AngKMessageHandler::Command_SSD2DDR(std::string devIP, uint32_t HopNum, uint32_t PortID, uint64_t SrcSSDAddr, uint64_t DestDDRAddr, uint64_t DataSize)
{
	return m_pACmdHandler->SSD2DDR(devIP, HopNum, PortID, SrcSSDAddr, DestDDRAddr, DataSize);
}

int32_t AngKMessageHandler::Command_DDR2SSD(std::string devIP, uint32_t HopNum, uint32_t PortID, uint64_t SrcDDRAddr, uint64_t DestSSDAddr, uint64_t DataSize)
{
	return m_pACmdHandler->DDR2SSD(devIP, HopNum, PortID, SrcDDRAddr, DestSSDAddr, DataSize);
}

int32_t AngKMessageHandler::Command_GetCapacity(std::string devIP, QString Type, uint32_t HopNum, uint32_t PortID)
{
	return m_pACmdHandler->GetCapacity(devIP, Type, HopNum, PortID);
}

int32_t AngKMessageHandler::Command_ReadDataAndSaveToFile(std::string devIP, QString strFilePath, QString Type, uint32_t HopNum, uint32_t PortID, uint64_t SrcAddr, uint64_t Length, uint64_t fileOffset, tJsonParaHash& ResultHash)
{
	return m_pACmdHandler->ReadDataAndSaveToFile(devIP, strFilePath, Type, HopNum, PortID, SrcAddr, Length, fileOffset, ResultHash);
}

int32_t AngKMessageHandler::Command_WriteDataFromFile(std::string devIP, QString strFilePath, QString Type, uint32_t HopNum, uint32_t PortID, uint64_t DestAddr, tJsonParaHash& ResultHash)
{
	return m_pACmdHandler->WriteDataFromFile(devIP, strFilePath, Type, HopNum, PortID, DestAddr, ResultHash);
}

int32_t AngKMessageHandler::Command_StoreDataToSSDorDDR(std::string devIP, QString Type, uint32_t HopNum, uint32_t PortID, uint64_t SSDAddr, QByteArray& DataBytes, std::string strProgSN, std::string strDataType)
{
	return m_pACmdHandler->StoreDataToSSDorDDR(devIP, Type, HopNum, PortID, SSDAddr, DataBytes, strProgSN, strDataType);
}

int32_t AngKMessageHandler::Command_ReadDataFromSSD(std::string devIP, QString Type, uint32_t HopNum, uint32_t PortID, uint64_t SSDAddr, int32_t BytesToRead, QByteArray& DataBytes, std::string strProgSN, std::string strDataType)
{
	return m_pACmdHandler->ReadDataFromSSDorDDR(devIP, Type, HopNum, PortID, SSDAddr, BytesToRead, DataBytes, strProgSN, strDataType);
}

// 高性能版本：去掉CRC校验等耗时操作，尽可能减少拷贝
int32_t AngKMessageHandler::Command_ReadDataFromSSD2(std::string devIP, QString Type, uint32_t HopNum, uint32_t PortID, uint64_t SSDAddr, int32_t BytesToRead, QByteArray& DataBytes)
{
	return m_pACmdHandler->ReadDataFromSSDorDDR2(devIP, Type, HopNum, PortID, SSDAddr, BytesToRead, DataBytes);
}

void AngKMessageHandler::Command_Initialize(QWidget* obj)
{
	m_pACmdHandler->InitMainWidget(obj);
}

int32_t AngKMessageHandler::Command_LinkScan(std::string devIP, uint32_t HopNum)
{
	return m_pACmdHandler->LinkScan(devIP, HopNum);
}

int32_t AngKMessageHandler::Command_CloseProgramSocket()
{
	return m_pACmdHandler->CloseProgramSocket();
}
