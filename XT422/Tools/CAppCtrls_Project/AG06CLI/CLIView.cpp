#include "CLIView.h"

#define _PrintLog(_Level,fmt,...) \
	if (m_pILog) {\
		m_pILog->PrintLog(_Level, fmt, __VA_ARGS__);\
	}

void CCLIView::OnDeviceModelLinkStatusChanged(quint32 HopNum, quint16 LinkStatus,quint32 isLastHop)
{
	_PrintLog(LOGLEVEL_N, "StatusChanged, HopNum:%d,isLastHop:%d, LinkStatus:0x%04X\r\n", HopNum, isLastHop, LinkStatus);
}


void CCLIView::OnDeviceInfoRawChanged(quint32 HopNum)
{
	int32_t Ret = 0;
	QString strTmp;
	tDeviceInfoRaw DeviceInfoRaw;
	Ret =m_pAppModels->GetDeviceInfoRaw(HopNum, DeviceInfoRaw);
	m_pILog->TimeHeadEn(false);
	strTmp = QString::asprintf("AC%02d_%04d%02d%02d%04d",
		DeviceInfoRaw.DevType, DeviceInfoRaw.Year+2020, DeviceInfoRaw.Month, DeviceInfoRaw.Day, DeviceInfoRaw.Serial);
	_PrintLog(LOGLEVEL_N, "========DeviceInfoRawChanged======HopNum:%d======\r\n", HopNum);
	_PrintLog(LOGLEVEL_N, "Serial: %s\r\n", strTmp.toStdString().c_str());
	_PrintLog(LOGLEVEL_N, "=================================================\r\n");
	m_pILog->TimeHeadEn(true);
	
}

void CCLIView::OnCapacityReceived(quint32 HopNum, qint32 CapacityType)
{
	int32_t Ret = 0;
	QString strType=CAppModels::GetCapacityTypeName((eCapacityType)CapacityType);
	quint64 Capacity = 0;
	m_pAppModels->GetCapacity(HopNum, (eCapacityType)CapacityType, Capacity);
	_PrintLog(LOGLEVEL_N, "HopNum:%d--Capacity[%s]: 0x%I64X\r\n", HopNum, strType.toStdString().c_str(), Capacity);
}

void CCLIView::AttachILog(ILog* pILog)
{
	m_pILog = pILog;
}

void CCLIView::AttachAppModel(CAppModels* pAppModels)
{
	m_pAppModels = pAppModels;
	//这个地方的Connect方式可能后续需要修改
	Qt::ConnectionType ConnetType = Qt::AutoConnection;//Qt::DirectConnection;
	connect(m_pAppModels, SIGNAL(sigLinkStatusChanged(quint32, quint16, quint32)),this, SLOT(OnDeviceModelLinkStatusChanged(quint32, quint16, quint32)), ConnetType);
	connect(m_pAppModels, SIGNAL(sigDeviceInfoRawChanged(quint32)), this, SLOT(OnDeviceInfoRawChanged(quint32)), ConnetType);
	connect(m_pAppModels, &CAppModels::sigCapacityReceived, this,&CCLIView::OnCapacityReceived, ConnetType);
}
