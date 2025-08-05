#include "CmdHandler.h"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
#include "ACError.h"
#include "ICD.h"
#include "ACCmdPacket.h"
#include "AngkLogger.h"
#include "ACEventLogger.h"
#include "AngKTransmitSignals.h"
#include "AngKDeviceModel.h"
#include "AngKPathResolve.h"
#include "ACDeviceManager.h"
#include "Thread/ThreadPool.h"
#include <QDebug>
#include <QThread>
#include <stdexcept>
#include "CustomMessageHandler.h"
#include "../Controller/RemoteServer/JsonRpcServer.h"
extern Acro::Thread::ThreadPool g_ThreadPool;

CCmdHandler::CCmdHandler()
	:m_pAppModels(NULL)
	,m_pIACComm(NULL)
	,m_CmdExeSemaphore(CmdQueue_MaxSize)
	,m_CmdAckTimeoutms(CmdACK_TimeOutms)
	, bChangeState(false)
{

	int i = 0;
	//设置初始的资源使用个数
	for (i = 0; i < CmdPTACK_ProgrammerSemaMax; ++i) {
		m_PTACKSemaphore[i].release(CmdPTACK_SiteSemaMax);//重新设置资源个数
		m_PTACKSemaphore[i].acquire(CmdPTACK_SiteSemaMax);//让所有的资源不能被分配
	}
	m_UplinkCRC = 0;
	m_DownlinkCRC = 0;
	connect(this, &CCmdHandler::SendNotification,
		JsonRpcServer::Instance(), &JsonRpcServer::SendNotification,
		Qt::QueuedConnection);
	QVector<DeviceStu> vecDev = ACDeviceManager::instance().getAllDevInfo();
	for (auto& devInfo : vecDev) {
		QString strIpHop = QString::fromStdString(devInfo.strIP) + ":" + QString::number(devInfo.nHopNum);
		m_mapCmdExeSemaphore[strIpHop].release(CmdQueue_MaxSize);

		m_mapPTACKSemaphore[strIpHop].release(CmdPTACK_SiteSemaMax);//重新设置资源个数
		m_mapPTACKSemaphore[strIpHop].acquire(CmdPTACK_SiteSemaMax);//让所有的资源不能被分配

		//std::pair<QMutex, QWaitCondition> value1(QMutex(), QWaitCondition());
		m_mapMtxCondition[strIpHop] = std::make_unique<std::pair<std::mutex, std::condition_variable>>();
		//m_mapMtxCondition.emplace(strIpHop, std::make_pair(QMutex(), QWaitCondition()));
	}
}
void CCmdHandler::WaitWriteFinish(QString ipHop)
{
	std::unique_lock<std::mutex> uLock(m_mapMtxCondition[ipHop]->first);
	m_mapMtxCondition[ipHop]->second.wait_for(uLock, std::chrono::milliseconds(1000));
}
//
int32_t CCmdHandler::HandleHeartBeat(std::string recvDevIP, uint8_t * pPckData, int Size)
{
	int32_t Ret = 0;
	bool IsLastHop = false;
	tHeartbeatPacket* pHeartbeatPck = (tHeartbeatPacket*)pPckData;
	if (Size != sizeof(tHeartbeatPacket)) {
		Ret = ERR_NETCOMM_PckSize; goto __end;
	}
	IsLastHop = pHeartbeatPck->LastHopFlag == 1 ? true : false;
	//ALOG_DEBUG("Recv HeartBeat Packet from %s:%d.", "FP", "CU", recvDevIP.c_str(), pHeartbeatPck->HopNum);
	ACDeviceManager::instance().reStartDevOffLineTimer(recvDevIP.c_str(), pHeartbeatPck->HopNum);
	Ret = m_pAppModels->UpdateLinkStatus((quint32)pHeartbeatPck->HopNum, pHeartbeatPck->LinkStatus, IsLastHop);

__end:
	return Ret;
}

int32_t CCmdHandler::HandleDevInfoResp(std::string recvDevIP, uint8_t* pPckData, int Size)
{
	int32_t Ret = 0;
	tDevInfoRespPacket* pDevInfoResp = (tDevInfoRespPacket*)pPckData;
	if (Size != sizeof(tDevInfoRespPacket)) {
		Ret = ERR_NETCOMM_PckSize; goto __end;
	}

	ALOG_DEBUG("Recv DevInfoResp Packet from %s:%d.", "CU", "FP", recvDevIP.c_str(), pDevInfoResp->HopNum);
	Ret = m_pAppModels->SetDeviceInfoRaw((quint32)pDevInfoResp->HopNum, pDevInfoResp->DevInfoWord);

__end:
	return Ret;
}

/// <summary>
/// 命令发出后，设备接收到就需要发送命令接收响应包，也就是ACK，这个是ACK包的处理
/// </summary>
/// <param name="pPckData"></param>
/// <param name="Size"></param>
/// <returns></returns>
int32_t CCmdHandler::HandleCmdRecvResp(std::string recvDevIP, uint8_t* pPckData, int Size)
{
	int32_t Ret = 0;
	eSubCmdID SubCmdID = (eSubCmdID)0;
	tCmdRespPacket* pCmdRecvPacket = (tCmdRespPacket*)pPckData;
	std::string cmdName = Utils::AngKCommonTools::TranslateMessageCmdID(pCmdRecvPacket->CmdID);
	if (Size != sizeof(tCmdRespPacket)) {
		Ret = ERR_NETCOMM_PckSize; goto __end;
	}
	
	try {
		m_stdACKPromise.set_value();
	}
	catch (...) {
		// 使用std::current_exception 存储被抛出的异常
		m_stdACKPromise.set_exception(std::current_exception());
	}

	SubCmdID =(eSubCmdID)pCmdRecvPacket->CmdID;
	switch (SubCmdID) {
		case eSubCmdID::SubCmd_DataTrans_FIBER2SSD:
			break;
		case eSubCmdID::SubCmd_DataTrans_FIBER2SKT:
			break;
		case eSubCmdID::SubCmd_DataTrans_FIBER2DDR:
			break;
		case eSubCmdID::SubCmd_DataTrans_SSD2FIBER:
			break;
		case eSubCmdID::SubCmd_DataTrans_SSD2SKT:
			break;
		case eSubCmdID::SubCmd_DataTrans_SKT2FIBER:
			break;		
		case eSubCmdID::SubCmd_DataTrans_DDR2FIBER:
			// DDR2FIBER ACK Response - 静默运行以减少日志噪音
			// ALOG_INFO("Recv %s ACK Response from %s:%d.", "CU", "FP", cmdName.c_str(), recvDevIP.c_str(), pCmdRecvPacket->HopNum);
			break;
		case eSubCmdID::SubCmd_DataTrans_DDR2SSD:
			break;
		case eSubCmdID::SubCmd_DataTrans_SSD2DDR:
			break;
		case eSubCmdID::SubCmd_ReadCapacity_SSD:
			break;
		case eSubCmdID::SubCmd_ReadCapacity_DDR:
			break;
		case eSubCmdID::SubCmd_ReadCapacity_SKT:
			break;
		case eSubCmdID::SubCmd_Regist_Read:
			break;
		case eSubCmdID::SubCmd_Regist_Write:
			break;
		case eSubCmdID::SubCmd_ReadCRC32:
			break;
		default:
		{	
			Ret = ERR_CMDHAND_SubCmdNotSupport;
			ALOG_FATAL("Command Response Packet NotSupport.", "CU", "FP");
			goto __end;
		}
	}

__end:
	return Ret;
}

int32_t CCmdHandler::SetCapacity(uint8_t CmdID, tCmdCplPacketGetCapacity* pCmdCplPacketGetCapacity)
{
	int32_t Ret = 0;
	eCapacityType CapacityType = eCapacityType::CAPACITY_NONE;
	quint64  Capacity = 0;
	switch ((eSubCmdID)CmdID) {
		case eSubCmdID::SubCmd_ReadCapacity_SSD:
			CapacityType = eCapacityType::CAPACITY_SSD;
			break;
		case eSubCmdID::SubCmd_ReadCapacity_DDR:
			CapacityType = eCapacityType::CAPACITY_DDR;
			break;
		case eSubCmdID::SubCmd_ReadCapacity_SKT:
			CapacityType = eCapacityType::CAPACITY_SKT;
			break;
	}
	Capacity = (quint64)pCmdCplPacketGetCapacity->CapacityH << 32 | (quint64)pCmdCplPacketGetCapacity->CapacityL;

	Ret=m_pAppModels->SetCapacity(pCmdCplPacketGetCapacity->HopNum, CapacityType, Capacity);
	emit sigGetCapacityComplete(pCmdCplPacketGetCapacity->HopNum, (qint32)CapacityType, Capacity);
	return Ret;
}

int32_t CCmdHandler::ReceivedCRC32(std::string recvDevIP, int nHop, tCmdCplPacketGetCRC32* pCmdCplGetCRC32)
{

	int32_t Ret = 0;
	ALOG_DEBUG("Recv ReadCRC32 from %s:%d, CRC32Uplink:0x%08X,CRC32Downlink:0x%08X.", "CU", "FP", recvDevIP.c_str(),
					pCmdCplGetCRC32->HopNum, pCmdCplGetCRC32->CRC32_Uplink,pCmdCplGetCRC32->CRC32_Downlink);

	m_UplinkCRC = pCmdCplGetCRC32->CRC32_Uplink;
	m_DownlinkCRC = pCmdCplGetCRC32->CRC32_Downlink;
	
	try {
		m_stdCRCPromise.set_value();
	}
	catch (...) {
		// 使用std::current_exception 存储被抛出的异常
		m_stdCRCPromise.set_exception(std::current_exception());
	}
	return  Ret;
}

/// <summary>
/// 命令在Device端处理完成之后，Device发送命令完成响应包，本函数是命令完成响应包到达时的处理函数
/// </summary>
/// <param name="pPckData"></param>
/// <param name="Size"></param>
/// <returns></returns>
int32_t CCmdHandler::HandleCmdCompleteResp(std::string recvDevIP, uint8_t* pPckData, int Size)
{
	int32_t Ret = 0;
	uCmdCplPacket* pCmdCplPacket = (uCmdCplPacket*)pPckData;
	quint64  Capacity = 0;
	uint8_t CmdID = 0;
	uint8_t HopNum = 0;
	uint32_t Status = 0;

	std::string CmdName;
	HopNum = pCmdCplPacket->PckData[1];
	QString strIPHop = QString::fromStdString(recvDevIP) + ":" + QString::number(HopNum);
	if (Size != sizeof(uCmdCplPacket)) {
		ALOG_FATAL("Recv Cmd Response Packet from %s, CompleteResp PacketSize Match Failed, Size:%d, PacketSizeNeed:%d.", "CU", "--", recvDevIP.c_str(), Size, sizeof(_uCmdCplPacket));
		Ret = ERR_NETCOMM_PckSize; goto __end;
	}

	ALOG_DEBUG("Releasing semaphore for %s, available before: %d", "CU", "--", strIPHop.toStdString().c_str(), m_mapCmdExeSemaphore[strIPHop].available());
	m_mapCmdExeSemaphore[strIPHop].release();///命令完成包回来后就需要释放
	ALOG_DEBUG("Semaphore released for %s, available after: %d", "CU", "--", strIPHop.toStdString().c_str(), m_mapCmdExeSemaphore[strIPHop].available());
	CmdID= pCmdCplPacket->PckData[3];
	Status = *(uint32_t*)(pCmdCplPacket->PckData + 4);
	CmdName = Utils::AngKCommonTools::TranslateMessageCmdID(CmdID);
	// Reduced verbosity: Command completion logging moved to DEBUG level
	ALOG_DEBUG("Recv %s Complete (Result:%d) from %s:%d.", "CU", "FP", CmdName.c_str(), Status, recvDevIP.c_str(), HopNum, CmdID);
	switch ((eSubCmdID)CmdID) {
		case eSubCmdID::SubCmd_DataTrans_FIBER2SSD:
			break;
		case eSubCmdID::SubCmd_DataTrans_FIBER2SKT:
			break;
		case eSubCmdID::SubCmd_DataTrans_FIBER2DDR:
			break;
		case eSubCmdID::SubCmd_DataTrans_SSD2FIBER:
			m_mapMtxCondition.find(strIPHop)->second->second.notify_one();
			break;
		case eSubCmdID::SubCmd_DataTrans_SSD2SKT:
			break;
		case eSubCmdID::SubCmd_DataTrans_SKT2FIBER:
			break;
		case eSubCmdID::SubCmd_DataTrans_DDR2FIBER:
			m_mapMtxCondition.find(strIPHop)->second->second.notify_one();
			break;
		case eSubCmdID::SubCmd_DataTrans_SSD2DDR:
			break;
		case eSubCmdID::SubCmd_DataTrans_DDR2SSD:
			break;
		case eSubCmdID::SubCmd_ReadCapacity_SSD:
		case eSubCmdID::SubCmd_ReadCapacity_DDR:
		case eSubCmdID::SubCmd_ReadCapacity_SKT:
			Ret=SetCapacity(CmdID, &pCmdCplPacket->GetCapacity);
			break;
		case eSubCmdID::SubCmd_Regist_Read:
			break;
		case eSubCmdID::SubCmd_Regist_Write:
			break;
		case eSubCmdID::SubCmd_ReadCRC32:
			Ret = ReceivedCRC32(recvDevIP, HopNum, &pCmdCplPacket->GetCRC32);
			break;
		default:
			Ret = ERR_CMDHAND_SubCmdNotSupport;
			ALOG_FATAL("Command Complete Packet NotSupport.", "CU", "FP");
			goto __end;
	}
__end:
	return Ret;
}
/*解析PTComplete包，这里可能会涉及到重新发起一个命令请求对SSD数据进行读取*/
/*
* 成功返回0，失败返回负数
*/
int32_t CCmdHandler::ParserPTComplete(std::string recvDevIP, tCmdPacketPT* pPacketPT)
{
	int32_t Ret = 0;
	tACCmdPack* pACCmdPack = (tACCmdPack *)pPacketPT->Data;
	QByteArray ACCmdPackBytes;
	ACCmdPackBytes.append((char*)pACCmdPack, ACCmdPack_GetTotalSize(pACCmdPack));
	emit sigPTPacketExecComplete(QString::fromStdString(recvDevIP), (qint32)pPacketPT->HopNum, (qint32)pPacketPT->PortID, ACCmdPackBytes);
	
	return Ret;
}

int32_t CCmdHandler::QueryDoPTCmd(std::string recvDevIP, tCmdPacketPT* pPacketPT)
{
	int32_t Ret = 0;
	tACCmdPack* pACCmdPack = (tACCmdPack*)pPacketPT->Data;
	QByteArray ACCmdQueryPackBytes;
	ACCmdQueryPackBytes.append((char*)pACCmdPack, ACCmdPack_GetTotalSize(pACCmdPack));
	emit sigQueryDoPTCmd(QString::fromStdString(recvDevIP), (qint32)pPacketPT->HopNum, (qint32)pPacketPT->PortID, ACCmdQueryPackBytes);
	return Ret;
}
int32_t CCmdHandler::CustomPT(std::string recvDevIP, tCmdPacketPT* pPacketPT) {
	//int32_t Ret = 0;
	//tACCmdPack* pACCmdPack = (tACCmdPack*)pPacketPT->Data;
	//QByteArray ACCmdQueryPackBytes;
	//ACCmdQueryPackBytes.append((char*)pACCmdPack, ACCmdPack_GetTotalSize(pACCmdPack));
	////to do 
	//QByteArray CmdDataBytes;
	//CmdDataBytes.append((char*)pACCmdPack->CmdData, pACCmdPack->CmdDataSize);

 //   // [1/3] 数据追踪日志
 //   ALOG_INFO("[1/3] CCmdHandler::CustomPT - devIP: %s, BPUID: %u", "CU", "--", 
 //             recvDevIP.c_str(), (uint16_t)pACCmdPack->BPUID);
 //   ALOG_INFO("[1/3] CCmdHandler::CustomPT - Hex: %s", "CU", "--", 
 //             CmdDataBytes.toHex().constData());
 //   ALOG_INFO("[1/3] CCmdHandler::CustomPT - Str: %s", "CU", "--", 
 //             QString(CmdDataBytes).toStdString().c_str());

	//if ((eSubCmdID)pACCmdPack->CmdID == eSubCmdID::SubCmd_MU_DoCustom) {
	//	CustomMessageHandler::instance()->OnRecvDoCustom(QString::fromStdString(recvDevIP), (uint16_t)pACCmdPack->BPUID, CmdDataBytes);
	//	
	//}
	//return Ret;
	return 0;
}
int32_t CCmdHandler::ReleasePTACK(std::string devIP, uint32_t HopNum, uint32_t PortID, std::string cmdName)
{
	int32_t Ret = 0;
	std::string strIPHop = devIP + ":" + QString::number(HopNum).toStdString();
	if (HopNum >= CmdPTACK_ProgrammerSemaMax) {
		Ret = ERR_CMDHAND_HopNum; goto __end;
	}
	//m_PTACKSemaphore[HopNum].release(1);
	m_mapPTACKSemaphore[QString::fromStdString(strIPHop)].release(1);
__end:
	if (Ret == 0) {
		std::map<std::string, DeviceStu> insertDev;
		AngKDeviceModel::instance().GetConnetDevMap(insertDev);
		
		nlohmann::json PTJson;
		PTJson["ProgSN"] = insertDev[strIPHop].tMainBoardInfo.strHardwareSN;
		PTJson["DataType"] = cmdName;
		PTJson["CMD"] = "Ack";
		PTJson["RetCode"] = Ret;

		EventLogger->SendEvent(EventBuilder->GetPTTransfer(PTJson));
	}

	return Ret;
}

/*处理一个透传包*/
int32_t CCmdHandler::HandlePT(std::string recvDevIP, uint8_t* pPckData, int Size)
{	
	int32_t Ret = 0;
	std::string cmdName;
	tCmdPacketPT* pPacketPT = (tCmdPacketPT*)pPckData;
	tACCmdPack* pACCmdPack = (tACCmdPack*)pPacketPT->Data;
	
	if (Size != sizeof(tCmdPacketPT)) {
		Ret = ERR_NETCOMM_PckSize; goto __end;
	}

	switch (pPacketPT->MsgSubID) {
			case CMDID_PTPACK_ACK:  //是一个远端返回的ACK包，不需要进行额外处理
				cmdName = Utils::AngKCommonTools::TranslateMessageCmdID(pACCmdPack->CmdID);
				ALOG_INFO("Recv %s ACK from %s:%d.", "MU", "CU", cmdName.c_str(), recvDevIP.c_str(), pPacketPT->HopNum);
				Ret = ReleasePTACK(recvDevIP, pPacketPT->HopNum, pPacketPT->PortID, cmdName);///释放透传包的ACK
				break;
			case CMDID_PTPACK_COMPLETE:
				Ret=ParserPTComplete(recvDevIP, pPacketPT);
				break;
			case CMDID_PTPACK_QUERYDOCMD: {
				Ret = QueryDoPTCmd(recvDevIP, pPacketPT);
				break; 
			case CMDID_PTPACK_CUSTOM: {
				break;
				//Ret = CustomPT(recvDevIP, pPacketPT);
			}
		}
		default:
			ALOG_FATAL("Recv PTPacket MsgSubID:0x%02X Not Support.", "MU", "CU", pPacketPT->MsgSubID);
			Ret = ERR_NETCOMM_PTPACKTYPE;
			break;
	}

__end:
	return Ret;
}

int32_t CCmdHandler::HandleInterrupt(std::string recvDevIP, uint8_t* pPckData, int Size)
{
	int32_t Ret = 0;
	bool bPackLost = 1;
	quint16 PreSeqNum = 0;
	tInterruptPacket* pInterruptPck = (tInterruptPacket*)pPckData;

	ALOG_DEBUG("Recv DevInterrupt Packet from %s:%d.", "CU", "FP", recvDevIP.c_str(), pInterruptPck->HopNum);
	if (Size != sizeof(tInterruptPacket)) {
		Ret = ERR_NETCOMM_PckSize; goto __end;
	}

	//给Device发送响应包
	Ret = SendInterruptACK(recvDevIP, pInterruptPck);
	ALOG_DEBUG("Send DevInterrupt Packet to %s:%d.", "CU", "FP", recvDevIP.c_str(), pInterruptPck->HopNum);

	emit sigDeviceEventArrive(pInterruptPck->HopNum, pInterruptPck->INTID);

__end:
	return Ret;
}

int32_t CCmdHandler::HandleDataUplink(std::string recvDevIP, uint8_t* pPckData, int Size)
{
	int32_t Ret = 0;
	tDataPacket* pDataPacket = (tDataPacket*)pPckData;
	QByteArray DataArray;
	DataArray.append((char*)pPckData, Size);
	ALOG_DEBUG("Recv DataUplink Packet from %s:%d.", "CU", "FP", recvDevIP.c_str(), pDataPacket->HopNum);
	//_PrintLog(LOGLEVEL_D, "GetDataUpLink, Size:%d", Size);
	emit sigRecvDataUplink(DataArray);
	return Ret;
}


void CCmdHandler::AttachAppModels(CAppModels* pAppModels)
{
	m_pAppModels = pAppModels;
	m_CmdAckTimeoutms=m_pAppModels->m_AppConfigModel.NetCommCmdAckTimeoutms();

	connect(m_pAppModels, &CAppModels::sigLinkStatusChanged, &AngKTransmitSignals::GetInstance(), &AngKTransmitSignals::sigLinkStatusChanged);
}

int32_t CCmdHandler::HandlePacket(std::string recvDevIP, uint8_t* pPckData, int Size)
{
	int32_t Ret = 0;
	uint8_t MsgID = pPckData[0];
	//uint8_t HEAD1 = pPckData[0];
	//uint8_t HEAD2 = pPckData[1];
	if (pPckData[0]==0x19 && pPckData[0+16] == 0xAA && pPckData[1+16] == 0x55) {
		uint16_t dataLength = (static_cast<uint16_t>(pPckData[6+16]) << 8) | pPckData[5+16];
		QByteArray CmdDataBytes(reinterpret_cast<const char*>(pPckData + 16 + 8), dataLength);
		CustomMessageHandler::instance()->ProcessLocalMessage(recvDevIP.c_str(), pPckData[2+16], pPckData[4+16], CmdDataBytes);
		return 0;
	}
	switch (MsgID) {
		case ICDMsgID_Heartbeat:
			Ret = HandleHeartBeat(recvDevIP, pPckData, Size);
			break;
		case ICDMsgID_DevInfoResp:
			Ret = HandleDevInfoResp(recvDevIP, pPckData, Size);
			break;
		case ICDMsgID_CmdRecvResp:
			Ret= HandleCmdRecvResp(recvDevIP, pPckData, Size);
			break;
		case ICDMsgID_CmdCompleteResp:
			Ret = HandleCmdCompleteResp(recvDevIP, pPckData, Size);
			break;
		case ICDMsgID_Interrupt:
			Ret = HandleInterrupt(recvDevIP, pPckData, Size);
			break;
		case ICDMsgID_DataUplink:
			if (!bChangeState) {
				Ret = HandleDataUplink(recvDevIP, pPckData, Size);
			}
			else {
				std::string strTmpDataBuf((char*)pPckData, Size);
				std::tuple<std::string, std::string, int32_t> tup = std::make_tuple(std::move(recvDevIP), std::move(strTmpDataBuf), Size);
				std::shared_ptr<std::tuple<std::string, std::string, int32_t>> pTup = std::make_shared<std::tuple<std::string, std::string, int32_t>>(std::move(tup));
				g_ThreadPool.PushTask([this, pTup]() {HandleDataUplink(std::get<0>(*pTup), (uint8_t*)std::get<1>(*pTup).c_str(), std::get<2>(*pTup)); });
			}
			break;
		case ICDMsgID_PT:
			{
				tCmdPacketPT* pPacketPT = (tCmdPacketPT*)pPckData;
				tACCmdPack* pACCmdPack = (tACCmdPack*)pPacketPT->Data;

				// 优先处理 DoCustom 命令，因为它最常见且需要快速响应
				if ((eSubCmdID)pACCmdPack->CmdID == eSubCmdID::SubCmd_MU_DoCustom)
				{
					if (pPacketPT->MsgSubID != CMDID_PTPACK_ACK) {
						QByteArray CmdDataBytes;
						CmdDataBytes.append((char*)pACCmdPack->CmdData, pACCmdPack->CmdDataSize);
						nlohmann::json DocustomResult;
						DocustomResult["result"] = true;
						DocustomResult["ip"] = recvDevIP;
						DocustomResult["bpuid"] = (uint16_t)pACCmdPack->BPUID;
						DocustomResult["data"] = CmdDataBytes;
						if (JsonRpcServer::Instance()->RUNING_STATUS)emit SendNotification("setDocustomResult", DocustomResult);
					}
					//CustomMessageHandler::instance()->OnRecvDoCustom(QString::fromStdString(recvDevIP), (uint16_t)pACCmdPack->BPUID, CmdDataBytes);
				}
				else if (pPacketPT->MsgSubID == CMDID_PTPACK_CUSTOM)
				{
					// 其他自定义命令，仍然通过 CustomPT 处理
					//CustomPT(recvDevIP, pPacketPT);
				}
				else {
					// 其他透传子类型（非CUSTOM），交由 HandlePT 异步处理
					std::string strTmpDataBuf((char*)pPckData, Size);
					std::tuple<std::string, std::string, int32_t> tup = std::make_tuple(std::move(recvDevIP), std::move(strTmpDataBuf), Size);
					std::shared_ptr<std::tuple<std::string, std::string, int32_t>> pTup = std::make_shared<std::tuple<std::string, std::string, int32_t>>(std::move(tup));
					g_ThreadPool.PushTask([this, pTup]() {HandlePT(std::get<0>(*pTup), (uint8_t*)std::get<1>(*pTup).c_str(), std::get<2>(*pTup)); });
				}
				break;
			}
			//Ret = HandlePT(recvDevIP, pPckData, Size);	
		default:
			Ret = ERR_CMDHAND_CmdNotSupport;
			break;
	}
	if (Ret != 0) {
		ALOG_FATAL("Recv ExecMsgID:0x%02X from %s Failed, ErrCode:%d.", "CU", "FP", MsgID, recvDevIP.c_str(), Ret);
	}
	return Ret;
}

int32_t CCmdHandler::SendInterruptACK(std::string devIP, tInterruptPacket* pInterruptPck)
{
	int32_t Ret = 0;
	Ret = m_pIACComm->SendData(devIP, (uint8_t*)pInterruptPck, sizeof(tInterruptPacket));
	return Ret;
}

int32_t CCmdHandler::SendLinkScanPacket(std::string devIP, tLinkScanPacket* pLinkScanPacket)
{
	int32_t Ret = 0;
	Ret = m_pIACComm->SendData(devIP, (uint8_t*)pLinkScanPacket, sizeof(tLinkScanPacket));
	return Ret;
}


int32_t CCmdHandler::SendDevInfoGetPacket(std::string devIP, tDevInfoGetPacket* pDevInfoGetPacket)
{
	int32_t Ret = 0;
	Ret = m_pIACComm->SendData(devIP, (uint8_t*)pDevInfoGetPacket, sizeof(tDevInfoGetPacket));
	return Ret;
}

/// <summary>
/// 确定命令队列是否为空
/// </summary>
/// <returns>返回true表示为空，返回false表示错误</returns>
bool CCmdHandler::IsCmdExeQueueEmpty()
{
	if (m_CmdExeSemaphore.available() == CmdQueue_MaxSize) {
		return true;
	}
	else {
		return false;
	}
}

/*
* 当发送或接收数据的时候出现错误，为了能让下次发送命令队列都可以使用，将所有的互斥量归位为可使用
*/
bool CCmdHandler::ReleaseCmdExeQueueSemaphoreWhenFail(std::string recvIp, int nHop, int TotalAvailableCnt)
{
	QString strIPHop = QString::fromStdString(recvIp) + ":" + QString::number(nHop);
	int CurCnt = m_mapCmdExeSemaphore[strIPHop].available();
	if (CurCnt != TotalAvailableCnt) {
		ALOG_DEBUG("Release %d CmdExeSemaphore", "CU", "--", TotalAvailableCnt - CurCnt);
		m_mapCmdExeSemaphore[strIPHop].release(TotalAvailableCnt - CurCnt);
	}
	return true;
}


bool CCmdHandler::WaitCmdExeQueueAvailable(std::string recvIp, int nHop, int Timeout,int AvailableCnt)
{
	bool Ret = false;
	QString strIPHop = QString::fromStdString(recvIp) + ":" + QString::number(nHop);
	Ret = m_mapCmdExeSemaphore[strIPHop].tryAcquire(AvailableCnt, Timeout);
	if (Ret == true) {//如果能够得到，那么就立即释放
		m_mapCmdExeSemaphore[strIPHop].release(AvailableCnt);
	}
	return Ret;
}

// DDR2FIBER操作专用的同步等待函数，确保数据传输完全完成
bool CCmdHandler::WaitDDR2FiberTransferComplete(std::string recvIp, int nHop, int Timeout)
{
	// 第一次等待：确保命令队列为空
	bool firstWait = WaitCmdExeQueueAvailable(recvIp, nHop, Timeout);
	if (!firstWait) {
		ALOG_DEBUG("DDR2FIBER first WaitCmdExeQueueAvailable failed for %s:%d", "CU", "--", recvIp.c_str(), nHop);
		return false;
	}
	
	// DDR操作特殊等待：给硬件额外时间稳定
	QThread::msleep(30);
	
	// 第二次等待：确保DDR到FIBER的数据传输完全完成
	bool secondWait = WaitCmdExeQueueAvailable(recvIp, nHop, Timeout);
	if (!secondWait) {
		ALOG_DEBUG("DDR2FIBER second WaitCmdExeQueueAvailable failed for %s:%d", "CU", "--", recvIp.c_str(), nHop);
		return false;
	}
	
	// 最终等待：确保所有缓冲和状态同步
	QThread::msleep(20);
	
	ALOG_DEBUG("DDR2FIBER transfer synchronization complete for %s:%d", "CU", "--", recvIp.c_str(), nHop);
	return true;
}

int32_t CCmdHandler::SendCmdPacket(std::string devIP, int nHop, uCmdPacket* pCmdPacket)
{
	int32_t Ret = 0;
	bool bLockGet = false;
	m_stdACKPromise.swap(std::promise<void>());
	QString strIPHop = QString::fromStdString(devIP) + ":" + QString::number(nHop);
	m_mapCmdExeSemaphore[strIPHop].acquire(); //需要请求资源
	Ret = m_pIACComm->SendData(devIP, (uint8_t*)pCmdPacket, sizeof(uCmdPacket));
	std::string cmdName = Utils::AngKCommonTools::TranslateMessageCmdID(pCmdPacket->Common.CmdID);
	if (Ret != 0) {
		goto __end;
	}

	try {
		auto btmpe = m_stdACKPromise.get_future().wait_for(std::chrono::milliseconds(m_CmdAckTimeoutms));
		if (btmpe == std::future_status::timeout) {
			ALOG_FATAL("Wait %s SendCmdPack ACK Timeout[%d]ms from %s:%d.", "MU", "CU", cmdName.c_str(), m_CmdAckTimeoutms, devIP.c_str(), pCmdPacket->Common.HopNum);
			Ret = ERR_NETCOMM_CmdACKTimeout; goto __end;
		}
	}
	catch (...) {
		// 使用std::current_exception 存储被抛出的异常
		ALOG_DEBUG("Wait %s ACK package to be out-of-order from %s:%d.", "MU", "CU", cmdName.c_str(), devIP.c_str(), nHop);
	}

__end:
	return Ret;
}

int32_t CCmdHandler::SendCmdCommon(std::string devIP, tCmdPacketCommon* pCmdPacketCommon)
{
	int32_t Ret = 0;
	Ret = SendCmdPacket(devIP, pCmdPacketCommon->HopNum, (uCmdPacket*)pCmdPacketCommon);
	return Ret;
}
int32_t CCmdHandler::GetACCmdPacketPayloadMax()
{
	int32_t PayloadMax = PTPACKET_PAYLOADLEN - sizeof(tACCmdPack);
	return PayloadMax;
}

int32_t CCmdHandler::WaitPTACK(std::string devIP, uint32_t HopNum, uint32_t PortID, std::string cmdName)
{
	int32_t i, Ret = 0;
	int32_t SemaWaitCnt = 0;
	bool bLockGet = false;
	QString strIPHop = QString::fromStdString(devIP) + ":" + QString::number(HopNum);
	for (i = 0; i < CmdPTACK_SiteSemaMax; i++) {
		if ((PortID >> (i * 2)) & 0x03) { //查看有几个BPU被发送
			SemaWaitCnt++;
		}
	}
	if (SemaWaitCnt == 0) {//发送给MU，还是需要等待1个
		SemaWaitCnt = 1;
	}

	if (HopNum >= CmdPTACK_ProgrammerSemaMax) {
		Ret = ERR_CMDHAND_HopNum; goto __end;
	}

	if((int)m_CmdAckTimeoutms > -1)
		bLockGet = m_mapPTACKSemaphore[strIPHop].tryAcquire(SemaWaitCnt, m_CmdAckTimeoutms);
	

	if (bLockGet == false) {
		ALOG_FATAL("Wait %s PTACK Timeout[%d]ms from %s:%d.", "MU", "CU", cmdName.c_str(), m_CmdAckTimeoutms, devIP.c_str(), HopNum);
		Ret = ERR_NETCOMM_CmdACKTimeout; goto __end;
	}
	//ALOG_DEBUG("PTACKSemaphore avaliable:%d.", "CU", "--", m_PTACKSemaphore[HopNum].available());
__end:
	return Ret;

}

int32_t CCmdHandler::SendCmdPTCmd(std::string devIP, std::string cmdName, tCmdPacketPT* pPTPacket)
{
	int32_t Ret = 0;
	bool bLockGet = false;
	m_MutexThreadPT.lock();
	Ret = m_pIACComm->SendData(devIP, (uint8_t*)pPTPacket, sizeof(tCmdPacketPT));
	m_MutexThreadPT.unlock();
	if (Ret != 0) {
		goto __end;
	}
#if 0
	bLockGet = m_MutexPTAck.tryLock(m_CmdAckTimeoutms);
	if (bLockGet == false) {
		_PrintLog(LOGLEVEL_E, "[E] Wait PT ACK Timeout:%d ms", m_CmdAckTimeoutms);
		Ret = ERR_NETCOMM_CmdACKTimeout; goto __end;
	}
#else
	Ret = WaitPTACK(devIP, pPTPacket->HopNum, pPTPacket->PortID, cmdName);
#endif
	//_PrintLog(LOGLEVEL_D, "PTCmdACK Received,m_CmdACKTimeoutms:%d ms", m_CmdAckTimeoutms);
__end:
	return Ret;
}

int32_t CCmdHandler::SendCmdPacketDataTrans(std::string devIP, tCmdPacketDataTrans* pCmdPacketDataTrans)
{
	int32_t Ret = 0;
	Ret=SendCmdPacket(devIP, pCmdPacketDataTrans->HopNum, (uCmdPacket*)pCmdPacketDataTrans);
	return Ret;
}

int32_t CCmdHandler::SendCmdGetCapacity(std::string devIP, tCmdPacketGetCapacity* pCmdPacketGetCapacity)
{
	int32_t Ret = 0;
	Ret = SendCmdPacket(devIP, pCmdPacketGetCapacity->HopNum, (uCmdPacket*)pCmdPacketGetCapacity);
	return Ret;
}

int32_t CCmdHandler::SendCmdRegister(std::string devIP, tCmdPacketRegister* pCmdPacketRegister)
{
	int32_t Ret = 0;
	Ret = SendCmdPacket(devIP, pCmdPacketRegister->HopNum, (uCmdPacket*)pCmdPacketRegister);
	return Ret;
}

int32_t CCmdHandler::SendCmdGetCRC32(std::string devIP, tCmdPacketGetCRC32* pCmdPacketGetCRC32,uint32_t& DownlinkCRC,uint32_t& UplinkCRC)
{
	int32_t Ret = 0;
	bool bLockGet = false;
	int retryCount = 0;
	const int maxRetries = 3; // DDR2FIBER操作的最大重试次数
	std::string cmdName = Utils::AngKCommonTools::TranslateMessageCmdID(pCmdPacketGetCRC32->CmdID);
	
	// DDR2FIBER操作需要重试机制以确保CRC32请求成功
	bool isDDR2FIBER = (pCmdPacketGetCRC32->PortID != 0); // 简化判断，实际可以根据具体协议优化
	
	while (retryCount <= maxRetries) {
		m_stdCRCPromise.swap(std::promise<void>());
		
		// DDR2FIBER操作在重试前需要额外等待
		if (isDDR2FIBER && retryCount > 0) {
			ALOG_DEBUG("DDR2FIBER CRC32 request retry %d, waiting for hardware stabilization.", "MU", "CU", retryCount);
			QThread::msleep(100 * retryCount); // 递增等待时间
		}
		
		Ret = SendCmdPacket(devIP, pCmdPacketGetCRC32->HopNum, (uCmdPacket*)pCmdPacketGetCRC32);
		
		if (Ret == 0) {
			try {
				auto btmpe = m_stdCRCPromise.get_future().wait_for(std::chrono::milliseconds(m_CmdAckTimeoutms));
				
				if (btmpe == std::future_status::timeout) {
					if (retryCount < maxRetries) {
						ALOG_DEBUG("Wait %s CRC32 timeout, retry %d/%d from %s:%d.", "MU", "CU", cmdName.c_str(), retryCount + 1, maxRetries, devIP.c_str(), pCmdPacketGetCRC32->HopNum);
						retryCount++;
						continue;
					}
					Ret = ERR_NETCOMM_CmdCRCGetTimeout; 
					break;
				}
				else {
					DownlinkCRC = m_DownlinkCRC;
					UplinkCRC = m_UplinkCRC;
					if (retryCount > 0) {
						ALOG_DEBUG("CRC32 request succeeded after %d retries from %s:%d.", "MU", "CU", retryCount, devIP.c_str(), pCmdPacketGetCRC32->HopNum);
					}
					break; // 成功获取CRC32，退出重试循环
				}
			}
			catch (...) {
				// 使用std::current_exception 存储被抛出的异常
				ALOG_DEBUG("Wait %s CRC32 package to be out-of-order from %s:%d.", "MU", "CU", cmdName.c_str(), devIP.c_str(), pCmdPacketGetCRC32->HopNum);
				if (retryCount < maxRetries) {
					retryCount++;
					continue;
				}
				break;
			}
		} else {
			if (retryCount < maxRetries) {
				ALOG_DEBUG("Send %s CRC32 command failed, retry %d/%d from %s:%d.", "MU", "CU", cmdName.c_str(), retryCount + 1, maxRetries, devIP.c_str(), pCmdPacketGetCRC32->HopNum);
				retryCount++;
				continue;
			}
			break;
		}
	}

	return Ret;
}

int32_t CCmdHandler::SendDownlinkData(std::string devIP, tDataPacket* pDataPacket)
{
	int32_t Ret = 0;
	Ret= m_pIACComm->SendData(devIP, (uint8_t*)pDataPacket, sizeof(tDataPacket));
	return int32_t();
}

