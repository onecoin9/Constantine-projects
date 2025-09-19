#include "ACDeviceModel.h"


void CACDeviceModel::setDevInfoRaw(tDeviceInfoRaw* pDevInfoRaw)
{
	memcpy(&m_DevInfoRaw, pDevInfoRaw, sizeof(tDeviceInfoRaw));
}

tDeviceInfoRaw CACDeviceModel::getDevInfoRaw()
{
	return m_DevInfoRaw;
}

void CACDeviceModel::SetDevStuInfo(DeviceStu paraDev)
{
	m_DevStu = paraDev;
}

DeviceStu CACDeviceModel::GetDevStuInfo()
{
	return m_DevStu;
}

int32_t CACDeviceModel::enqueueDeviceEvent(tDeviceEvent)
{
	return int32_t();
}

int32_t CACDeviceModel::dequeueDeviceEvent(tDeviceEvent& DeviceEvent)
{
	return int32_t();
}
