#include "CmdHandler.h"
#include "ACError.h"
#include "ICD.h"
#include "ACCmdPacket.h"

#define _PrintLog(_Level,fmt,...) \
	if (m_pILog) {\
		m_pILog->PrintLog(_Level, fmt, __VA_ARGS__);\
	}

CCmdHandler::CCmdHandler()
	:m_pAppModels(NULL)
	,m_pIACComm(NULL)
	,m_CmdExeSemaphore(CmdQueue_MaxSize)
	,m_CmdAckTimeoutms(CmdACK_TimeOutms)
{
	int i = 0;
	m_MutexAck.lock();
	m_MutexCRC.lock();
	//m_MutexPTAck.lock();
	//设置初始的资源使用个数
	for (i = 0; i < CmdPTACK_ProgrammerSemaMax; ++i) {
		m_PTACKSemaphore[i].release(CmdPTACK_SiteSemaMax);//重新设置资源个数
		m_PTACKSemaphore[i].acquire(CmdPTACK_SiteSemaMax);//让所有的资源不能被分配
	}
	m_UplinkCRC = 0;
	m_DownlinkCRC = 0;
}
//
int32_t CCmdHandler::HandleHeartBeat(uint8_t * pPckData, int Size)
{
	int32_t Ret = 0;
	bool IsLastHop = false;
	tHeartbeatPacket* pHeartbeatPck = (tHeartbeatPacket*)pPckData;
	if (Size != sizeof(tHeartbeatPacket)) {
		Ret = ERR_NETCOMM_PckSize; goto __end;
	}
	IsLastHop = pHeartbeatPck->LastHopFlag == 1 ? true: false;
	Ret = m_pAppModels->UpdateLinkStatus((quint32)pHeartbeatPck->HopNum, pHeartbeatPck->LinkStatus,IsLastHop);

__end:
	return Ret;
}

int32_t CCmdHandler::HandleDevInfoResp(uint8_t* pPckData, int Size)
{
	int32_t Ret = 0;
	tDevInfoRespPacket* pDevInfoResp = (tDevInfoRespPacket*)pPckData;
	if (Size != sizeof(tDevInfoRespPacket)) {
		Ret = ERR_NETCOMM_PckSize; goto __end;
	}
	
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
int32_t CCmdHandler::HandleCmdRecvResp(uint8_t* pPckData, int Size)
{
	int32_t Ret = 0;
	eSubCmdID SubCmdID = (eSubCmdID)0;
	tCmdRespPacket* pCmdRecvPacket = (tCmdRespPacket*)pPckData;
	if (Size != sizeof(tCmdRespPacket)) {
		Ret = ERR_NETCOMM_PckSize; goto __end;
	}
	m_MutexAck.unlock(); ///释放接收包
	_PrintLog(LOGLEVEL_D, "Command Response Packet HopNum:%d, CMDID:0x%02X\r\n", pCmdRecvPacket->HopNum, pCmdRecvPacket->CmdID);
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
			_PrintLog(LOGLEVEL_E, "Command Response Packet NotSupport\r\n");
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

int32_t CCmdHandler::ReceivedCRC32(tCmdCplPacketGetCRC32* pCmdCplGetCRC32)
{
	int32_t Ret = 0;
	_PrintLog(LOGLEVEL_D, "ReceivedCRC32 HopNum:0x%02X, CRC32Uplink:0x%08X,CRC32Downlink:0x%08X\r\n",
					pCmdCplGetCRC32->HopNum, pCmdCplGetCRC32->CRC32_Uplink,pCmdCplGetCRC32->CRC32_Downlink);

	m_UplinkCRC = pCmdCplGetCRC32->CRC32_Uplink;
	m_DownlinkCRC = pCmdCplGetCRC32->CRC32_Downlink;
	m_MutexCRC.unlock();
	return  Ret;
}

/// <summary>
/// 命令在Device端处理完成之后，Device发送命令完成响应包，本函数是命令完成响应包到达时的处理函数
/// </summary>
/// <param name="pPckData"></param>
/// <param name="Size"></param>
/// <returns></returns>
int32_t CCmdHandler::HandleCmdCompleteResp(uint8_t* pPckData, int Size)
{
	int32_t Ret = 0;
	uCmdCplPacket* pCmdCplPacket = (uCmdCplPacket*)pPckData;
	quint64  Capacity = 0;
	uint8_t CmdID = 0;
	uint8_t HopNum = 0;
	uint32_t Status = 0;
	if (Size != sizeof(uCmdCplPacket)) {
		_PrintLog(LOGLEVEL_E, "CompleteResp PacketSize Match Failed, Size:%d, PacketSizeNeed:%d\r\n", Size, sizeof(_uCmdCplPacket));
		Ret = ERR_NETCOMM_PckSize; goto __end;
	}
	m_CmdExeSemaphore.release();///命令完成包回来后就需要释放
	HopNum = pCmdCplPacket->PckData[1];
	CmdID= pCmdCplPacket->PckData[3];
	Status = *(uint32_t*)(pCmdCplPacket->PckData + 4);
	_PrintLog(LOGLEVEL_D, "Command Complete Packet HopNum:0x%02X, CMDID:0x%02X, Status:0x%08X\r\n", HopNum, CmdID, Status);
	switch ((eSubCmdID)CmdID) {
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
			Ret = ReceivedCRC32(&pCmdCplPacket->GetCRC32);
			break;
		default:
			Ret = ERR_CMDHAND_SubCmdNotSupport;
			_PrintLog(LOGLEVEL_E, "Command Complete Packet NotSupport\r\n");
			goto __end;
	}
__end:
	return Ret;
}
/*解析PTComplete包，这里可能会涉及到重新发起一个命令请求对SSD数据进行读取*/
/*
* 成功返回0，失败返回负数
*/
int32_t CCmdHandler::ParserPTComplete(tCmdPacketPT* pPacketPT)
{
	int32_t Ret = 0;
	tACCmdPack* pACCmdPack = (tACCmdPack *)pPacketPT->Data;
	QByteArray ACCmdPackBytes;
	ACCmdPackBytes.append((char*)pACCmdPack, ACCmdPack_GetTotalSize(pACCmdPack));
	//_PrintLog(LOGLEVEL_D, "ParserPTComplete ACCmdPackTotalSize:%d\r\n", ACCmdPack_GetTotalSize(pACCmdPack));
	emit sigPTPacketExecComplete((qint32)pPacketPT->HopNum, (qint32)pPacketPT->PortID, ACCmdPackBytes);
	return Ret;
}

int32_t CCmdHandler::QueryDoPTCmd(tCmdPacketPT* pPacketPT)
{
	int32_t Ret = 0;
	tACCmdPack* pACCmdPack = (tACCmdPack*)pPacketPT->Data;
	QByteArray ACCmdQueryPackBytes;
	ACCmdQueryPackBytes.append((char*)pACCmdPack, ACCmdPack_GetTotalSize(pACCmdPack));
	emit sigQueryDoPTCmd((qint32)pPacketPT->HopNum, (qint32)pPacketPT->PortID, ACCmdQueryPackBytes);
	return Ret;
}

int32_t CCmdHandler::ReleasePTACK(uint32_t HopNum,uint32_t PortID)
{
	int32_t Ret = 0;
	if (HopNum >= CmdPTACK_ProgrammerSemaMax) {
		Ret = ERR_CMDHAND_HopNum; goto __end;
	}
	m_PTACKSemaphore[HopNum].release(1);
__end:
	return Ret;
}

/*处理一个透传包*/
int32_t CCmdHandler::HandlePT(uint8_t* pPckData, int Size)
{	
	int32_t Ret = 0;
	tCmdPacketPT* pPacketPT = (tCmdPacketPT*)pPckData;
	if (Size != sizeof(tCmdPacketPT)) {
		Ret = ERR_NETCOMM_PckSize; goto __end;
	}
	//m_pILog->PrintBuf((char*)"=========PTPacketFrom AG06=========",(char*)pPckData, 64);
	switch (pPacketPT->CMD_ID) {
		case CMDID_PTPACK_ACK:  //是一个远端返回的ACK包，不需要进行额外处理
			_PrintLog(LOGLEVEL_D, "Receive PT ACK!\r\n");
			Ret = ReleasePTACK(pPacketPT->HopNum, pPacketPT->PortID);///释放透传包的ACK
			break;
		case CMDID_PTPACK_COMPLETE:
			_PrintLog(LOGLEVEL_D, "Receive PT Complete!\r\n");
			Ret=ParserPTComplete(pPacketPT);
			break;
		case CMDID_PTPACK_QUERYDOCMD:
			_PrintLog(LOGLEVEL_D, "Remote PT QueryDoCmd\r\n");
			Ret = QueryDoPTCmd(pPacketPT);
			break;
		default:
			_PrintLog(LOGLEVEL_E, "PTPacket CmdID Not Support CmdID:0x%02X\r\n", pPacketPT->CMD_ID);
			Ret = ERR_NETCOMM_PTPACKTYPE;
			break;
	}

__end:
	return Ret;
}

int32_t CCmdHandler::HandleInterrupt(uint8_t* pPckData, int Size)
{
	int32_t Ret = 0;
	bool bPackLost = 1;
	quint16 PreSeqNum = 0;
	tInterruptPacket* pInterruptPck = (tInterruptPacket*)pPckData;

	if (Size != sizeof(tInterruptPacket)) {
		Ret = ERR_NETCOMM_PckSize; goto __end;
	}

	//给Device发送响应包
	Ret = SendInterruptACK(pInterruptPck);

	emit sigDeviceEventArrive(pInterruptPck->HopNum, pInterruptPck->INTID);

__end:
	return Ret;
}

int32_t CCmdHandler::HandleDataUplink(uint8_t* pPckData, int Size)
{
	int32_t Ret = 0;
	tDataPacket* pDataPacket = (tDataPacket*)pPckData;
	QByteArray DataArray;
	DataArray.append((char*)pPckData, Size);
	//_PrintLog(LOGLEVEL_D, "GetDataUpLink, Size:%d\r\n", Size);
	emit sigRecvDataUplink(DataArray);
	return Ret;
}


void CCmdHandler::AttachAppModels(CAppModels* pAppModels)
{
	m_pAppModels = pAppModels;
	m_CmdAckTimeoutms=m_pAppModels->m_AppConfigModel.NetCommCmdAckTimeoutms();
}

int32_t CCmdHandler::HandlePacket(uint8_t* pPckData, int Size)
{
	int32_t Ret = 0;
	uint8_t MsgID = pPckData[0];
	_PrintLog(LOGLEVEL_D,"GetOnePacket MsgId:0x%02X, PckSize:%d\r\n", MsgID, Size);
	switch (MsgID) {
		case ICDMsgID_Heartbeat:
			Ret = HandleHeartBeat(pPckData, Size);
			break;
		case ICDMsgID_DevInfoResp:
			Ret = HandleDevInfoResp(pPckData, Size);
			break;
		case ICDMsgID_CmdRecvResp:
			Ret= HandleCmdRecvResp(pPckData, Size);
			break;
		case ICDMsgID_CmdCompleteResp:
			Ret = HandleCmdCompleteResp(pPckData, Size);
			break;
		case ICDMsgID_Interrupt:
			Ret = HandleInterrupt(pPckData, Size);
			break;
		case ICDMsgID_DataUplink:
			Ret = HandleDataUplink(pPckData, Size);
			break;
		case ICDMsgID_PT:
			Ret = HandlePT(pPckData, Size);
			break;
		default:
			Ret = ERR_CMDHAND_CmdNotSupport;
			break;

	}
	if (Ret != 0) {
		_PrintLog(LOGLEVEL_E, "ExecMsgID:0x%02X Failed, ErrCode:%d\r\n", MsgID, Ret);
	}
	return Ret;
}

int32_t CCmdHandler::SendInterruptACK(tInterruptPacket* pInterruptPck)
{
	int32_t Ret = 0;
	Ret = m_pIACComm->SendData((uint8_t*)pInterruptPck, sizeof(tInterruptPacket));
	return Ret;
}

int32_t CCmdHandler::SendLinkScanPacket(tLinkScanPacket* pLinkScanPacket)
{
	int32_t Ret = 0;
	Ret = m_pIACComm->SendData((uint8_t*)pLinkScanPacket, sizeof(tLinkScanPacket));
	return Ret;
}


int32_t CCmdHandler::SendDevInfoGetPacket(tDevInfoGetPacket* pDevInfoGetPacket)
{
	int32_t Ret = 0;
	Ret = m_pIACComm->SendData((uint8_t*)pDevInfoGetPacket, sizeof(tDevInfoGetPacket));
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
bool CCmdHandler::ReleaseCmdExeQueueSemaphoreWhenFail(int TotalAvailableCnt)
{
	int CurCnt=m_CmdExeSemaphore.available();
	if (CurCnt != TotalAvailableCnt) {
		_PrintLog(LOGLEVEL_N, "====Release %d CmdExeSemaphore\r\n", TotalAvailableCnt - CurCnt);
		m_CmdExeSemaphore.release(TotalAvailableCnt - CurCnt);
	}
	return true;
}


bool CCmdHandler::WaitCmdExeQueueAvailable(int Timeout,int AvailableCnt)
{
	bool Ret = false;
	Ret = m_CmdExeSemaphore.tryAcquire(AvailableCnt, Timeout);
	if (Ret == true) {//如果能够得到，那么就立即释放
		m_CmdExeSemaphore.release(AvailableCnt);
	}
	return Ret;
}

int32_t CCmdHandler::SendCmdPacket(uCmdPacket* pCmdPacket)
{
	int32_t Ret = 0;
	bool bLockGet = false;
	m_CmdExeSemaphore.acquire(); //需要请求资源
	_PrintLog(LOGLEVEL_D,"SendCmdPacket,MsgId:0x%X, CmdID:0x%02X\r\n", pCmdPacket->Common.MsgID, pCmdPacket->Common.CmdID);
	Ret = m_pIACComm->SendData((uint8_t*)pCmdPacket, sizeof(uCmdPacket));
	_PrintLog(LOGLEVEL_D, "CNetComm::SendData Ret = %d\r\n", Ret);
	if (Ret != 0) {
		goto __end;
	}
	bLockGet = m_MutexAck.tryLock(m_CmdAckTimeoutms);
	if (bLockGet == false) {
		_PrintLog(LOGLEVEL_E, "Wait ACK Timeout[%d]ms\r\n", m_CmdAckTimeoutms);
		Ret = ERR_NETCOMM_CmdACKTimeout; goto __end;
	}
__end:
	return Ret;
}

int32_t CCmdHandler::SendCmdCommon(tCmdPacketCommon* pCmdPacketCommon)
{
	int32_t Ret = 0;
	Ret = SendCmdPacket((uCmdPacket*)pCmdPacketCommon);
	return Ret;
}
int32_t CCmdHandler::GetACCmdPacketPayloadMax()
{
	int32_t PayloadMax = PTPACKET_PAYLOADLEN - sizeof(tACCmdPack);
	return PayloadMax;
}


int32_t CCmdHandler::WaitPTACK(uint32_t HopNum,uint32_t PortID)
{
	int32_t i,Ret=0;
	int32_t SemaWaitCnt = 0;
	bool bLockGet = false;
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

	bLockGet=m_PTACKSemaphore[HopNum].tryAcquire(SemaWaitCnt, m_CmdAckTimeoutms);
	if (bLockGet == false) {
		_PrintLog(LOGLEVEL_E, "[E] Wait PT ACK Timeout:%d ms\r\n", m_CmdAckTimeoutms);
		Ret = ERR_NETCOMM_CmdACKTimeout; goto __end;
	}
	_PrintLog(LOGLEVEL_N, "PTACKSemaphore avaliable:%d\r\n", m_PTACKSemaphore[HopNum].available());
__end:
	return Ret;

}

int32_t CCmdHandler::SendCmdPTCmd(tCmdPacketPT* pPTPacket)
{
	int32_t Ret = 0;
	bool bLockGet = false;
	Ret = m_pIACComm->SendData((uint8_t*)pPTPacket, sizeof(tCmdPacketPT));
	if (Ret != 0) {
		goto __end;
	}
#if 0
	bLockGet = m_MutexPTAck.tryLock(m_CmdAckTimeoutms);
	if (bLockGet == false) {
		_PrintLog(LOGLEVEL_E, "[E] Wait PT ACK Timeout:%d ms\r\n", m_CmdAckTimeoutms);
		Ret = ERR_NETCOMM_CmdACKTimeout; goto __end;
	}
#else
	Ret = WaitPTACK(pPTPacket->HopNum, pPTPacket->PortID);
#endif
	//_PrintLog(LOGLEVEL_D, "PTCmdACK Received,m_CmdACKTimeoutms:%d ms\r\n", m_CmdAckTimeoutms);
__end:
	return Ret;
}

int32_t CCmdHandler::SendCmdPacketDataTrans(tCmdPacketDataTrans* pCmdPacketDataTrans)
{
	int32_t Ret = 0;
	Ret=SendCmdPacket((uCmdPacket*)pCmdPacketDataTrans);
	return Ret;
}

int32_t CCmdHandler::SendCmdGetCapacity(tCmdPacketGetCapacity* pCmdPacketGetCapacity)
{
	int32_t Ret = 0;
	Ret = SendCmdPacket((uCmdPacket*)pCmdPacketGetCapacity);
	return Ret;
}

int32_t CCmdHandler::SendCmdRegister(tCmdPacketRegister* pCmdPacketRegister)
{
	int32_t Ret = 0;
	Ret = SendCmdPacket((uCmdPacket*)pCmdPacketRegister);
	return Ret;
}

int32_t CCmdHandler::SendCmdGetCRC32(tCmdPacketGetCRC32* pCmdPacketGetCRC32,uint32_t& DownlinkCRC,uint32_t& UplinkCRC)
{
	int32_t Ret = 0;
	bool bLockGet = false;
	Ret = SendCmdPacket((uCmdPacket*)pCmdPacketGetCRC32);
	if (Ret == 0) {
		bLockGet = m_MutexCRC.tryLock(1000);
		if (bLockGet == false) {
			Ret = ERR_NETCOMM_CmdCRCGetTimeout; goto __end;
		}
		else {
			DownlinkCRC = m_DownlinkCRC;
			UplinkCRC = m_UplinkCRC;
		}
	}
__end:
	return Ret;
}

int32_t CCmdHandler::SendDownlinkData(tDataPacket* pDataPacket)
{
	int32_t Ret = 0;
	Ret= m_pIACComm->SendData((uint8_t*)pDataPacket, sizeof(tDataPacket));
	return int32_t();
}

