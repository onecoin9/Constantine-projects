#include "APPModels.h"
#include "AngkLogger.h"
#include "AngKPathResolve.h"
CAppModels::CAppModels()
{
	m_DeviceModelHash.clear();
	m_AppConfigModel.LoadConfig(Utils::AngKPathResolve::localGlobalSettingFile());
}

CAppModels::~CAppModels()
{
	tDeviceModelHashItr Itr = m_DeviceModelHash.begin();
	while (Itr != m_DeviceModelHash.end()) {
		delete Itr.value(); ///删除new对应的QByteArray
		Itr++;
	}
	m_DeviceModelHash.clear();
}

QString CAppModels::GetCapacityTypeName(eCapacityType Type)
{
	switch (Type) {
		case eCapacityType::CAPACITY_DDR:
			return QString("DDR");
		case eCapacityType::CAPACITY_SKT:
			return QString("SKT");
		case eCapacityType::CAPACITY_SSD:
			return QString("SSD");
		default:
			return QString("Unknown");
	}
}

int32_t CAppModels::InsertNewHop(quint32 HopNum)
{
	tDeviceModelHash* pDeviceModelHash = &m_DeviceModelHash;
	tDeviceModelHashItr iter;
	CACDeviceModel* pDeviceModel = NULL;
	iter = pDeviceModelHash->find(HopNum);
	if (iter != pDeviceModelHash->end()) {//已经存在了HOP
		pDeviceModel = iter.value();
	}
	else {///新的HOP
		pDeviceModel = new CACDeviceModel();
		pDeviceModel->resetProperties();
		pDeviceModel->setHopNum(HopNum);
		pDeviceModelHash->insert(HopNum, pDeviceModel);
	}
	return 0;
}

int32_t CAppModels::UpdateLinkStatus(quint32 HopNum, quint16 LinkStatus, bool IsLastHop)
{
	int32_t Ret = 0;
	tDeviceModelHash* pDeviceModelHash = &m_DeviceModelHash;
	CACDeviceModel* pDeviceModel = NULL;
	InsertNewHop(HopNum);
	pDeviceModel = pDeviceModelHash->find(HopNum).value();
	pDeviceModel->setLinkStatus(LinkStatus);
	pDeviceModel->setisLastHop(IsLastHop ? 1 : 0);
	emit sigLinkStatusChanged(HopNum, LinkStatus, IsLastHop);
	return Ret;
}

int32_t CAppModels::SetDeviceInfoRaw(quint32 HopNum, quint32 DevInfo[])
{
	int32_t Ret = 0;
	tDeviceModelHash* pDeviceModelHash = &m_DeviceModelHash;
	tDeviceModelHashItr iter;

	iter = pDeviceModelHash->find(HopNum);
	if (iter != pDeviceModelHash->end()) {//已经存在了HOP
		CACDeviceModel* pDeviceModel = iter.value();
		pDeviceModel->setDevInfoRaw((tDeviceInfoRaw*)DevInfo);
		emit sigDeviceInfoRawChanged(HopNum);
	}
	else {
		Ret = ERR_NETCOMM_HopNumError;
	}
	return Ret;
}


int32_t CAppModels::GetDeviceInfoRaw(quint32 HopNum, tDeviceInfoRaw& DeviceInfoRaw)
{
	int32_t Ret = 0;
	tDeviceModelHash* pDeviceModelHash = &m_DeviceModelHash;
	tDeviceModelHashItr iter;
	iter = pDeviceModelHash->find(HopNum);
	if (iter != pDeviceModelHash->end()) {//已经存在了HOP
		CACDeviceModel* pDeviceModel = iter.value();
		DeviceInfoRaw = pDeviceModel->getDevInfoRaw();
	}
	else {
		memset(&DeviceInfoRaw, 0, sizeof(tDeviceInfoRaw));
		Ret = ERR_NETCOMM_HopNumError;
	}
	return Ret;
}

int32_t CAppModels::SetCapacity(quint32 HopNum, eCapacityType CapacityType, quint64 Capacity)
{
	int32_t Ret = 0;
	tDeviceModelHash* pDeviceModelHash = &m_DeviceModelHash;
	tDeviceModelHashItr iter;
	InsertNewHop(HopNum);
	iter = pDeviceModelHash->find(HopNum);
	if (iter != pDeviceModelHash->end()) {//已经存在了HOP
		CACDeviceModel* pDeviceModel = iter.value();
		switch (CapacityType) {
			case eCapacityType::CAPACITY_SSD:
				pDeviceModel->setSSDCapacity(Capacity);
				break;
			case eCapacityType::CAPACITY_DDR:
				pDeviceModel->setDDRCapacity(Capacity);
				break;
			case eCapacityType::CAPACITY_SKT:
				pDeviceModel->setSKTCapacity(Capacity);
				break;
			default:
				Ret = ERR_MODEL_CapacityType;
				break;
		}
	}
	else {
		Ret = ERR_NETCOMM_HopNumError;
	}
	if (Ret == 0) {
		emit sigCapacityReceived(HopNum, (qint32)CapacityType);
	}
	return Ret;
}

int32_t CAppModels::GetCapacity(quint32 HopNum, eCapacityType CapacityType,quint64& Capacity)
{
	int32_t Ret = 0;
	tDeviceModelHash* pDeviceModelHash = &m_DeviceModelHash;
	tDeviceModelHashItr iter;
	iter = pDeviceModelHash->find(HopNum);
	if (iter != pDeviceModelHash->end()) {//已经存在了HOP
		CACDeviceModel* pDeviceModel = iter.value();
		switch (CapacityType) {
			case eCapacityType::CAPACITY_SSD:
				Capacity=pDeviceModel->SSDCapacity();
				break;
			case eCapacityType::CAPACITY_DDR:
				Capacity = pDeviceModel->DDRCapacity();
				break;
			case eCapacityType::CAPACITY_SKT:
				Capacity = pDeviceModel->SKTCapacity();
				break;
			default:
				Ret = ERR_NETCOMM_HopNumError;
				break;
		}
	}
	return Ret;
}


int32_t CAppModels::IncreaseSeqNum(quint32 HopNum, quint16& PreSeqNum)
{
	int32_t Ret = 0;
	tDeviceModelHash* pDeviceModelHash = &m_DeviceModelHash;
	tDeviceModelHashItr iter;
	iter = pDeviceModelHash->find(HopNum);
	if (iter != pDeviceModelHash->end()) {//已经存在了HOP
		CACDeviceModel* pDeviceModel = iter.value();
		PreSeqNum =pDeviceModel->SeqNum();
		//SeqNum在整个包中只占用了5个Bit，最大值到31，如果再增加则需要回到0
		if (PreSeqNum >= 31) {
			pDeviceModel->setSeqNum(0);
		}
		else {
			pDeviceModel->setSeqNum(PreSeqNum+1);
		}
	}
	else {
		Ret = ERR_NETCOMM_HopNumError;
	}
	return Ret;
}

int32_t CAppModels::IncreasePackTotal(quint32 HopNum,bool bPackLost)
{
	int32_t Ret = 0;
	tDeviceModelHash* pDeviceModelHash = &m_DeviceModelHash;
	tDeviceModelHashItr iter;
	iter = pDeviceModelHash->find(HopNum);
	if (iter != pDeviceModelHash->end()) {//已经存在了HOP
		CACDeviceModel* pDeviceModel = iter.value();
		pDeviceModel->setPacketTotal(pDeviceModel->PacketTotal() + 1);
		if (bPackLost) {
			pDeviceModel->setPacketLost(pDeviceModel->PacketLost() + 1);
		}
	}
	else {
		Ret = ERR_NETCOMM_HopNumError;
	}
	return Ret;
}
