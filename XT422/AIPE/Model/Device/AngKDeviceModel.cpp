#include "AngKDeviceModel.h"
#include "AngkLogger.h"
#include <QMutexLocker>

//静态成员变量初始化
AngKDeviceModel* AngKDeviceModel::s_instance = nullptr;
std::once_flag AngKDeviceModel::once_flag;
std::map<std::string, DeviceStu> AngKDeviceModel::m_mapScanDeviceData;//保存所有扫描的设备 key为 [IP:Hop] value为设备信息
std::map<std::string, DeviceStu> AngKDeviceModel::m_mapConnetDeviceData;//保存所有连接的设备
static QMutex m_mutex;
// 定义并初始化 static 成员变量
bool AngKDeviceModel::autoAddToConnect = false; // 默认值为 false
void AngKDeviceModel::AddScanDevIP(std::string strIPHop)
{
	QMutexLocker locker(&m_mutex);
	DeviceStu newDevStu;
	auto it = m_mapScanDeviceData.find(strIPHop);
	if (it != m_mapScanDeviceData.end()) {
		newDevStu.nChainID = it->second.nChainID;
	}
	else {
		newDevStu.nChainID = m_mapScanDeviceData.size() + 1;
	}

	m_mapScanDeviceData[strIPHop] = newDevStu;
}

bool AngKDeviceModel::FindScanDev(std::string strIPHop)
{
	QMutexLocker locker(&m_mutex);
	auto it = m_mapScanDeviceData.find(strIPHop);
	if (it != m_mapScanDeviceData.end()) {
		return true;
	}

	return false;
}
bool AngKDeviceModel::SetScanDevInfo(std::string strIPHop, DeviceStu devInfo)
{
	bool bRet = FindScanDev(strIPHop);
	QMutexLocker locker(&m_mutex);
	if (!bRet) {
		goto _end;
	}

	m_mapScanDeviceData[strIPHop].strIP = devInfo.strIP;
	m_mapScanDeviceData[strIPHop].strPort = devInfo.strPort;
	m_mapScanDeviceData[strIPHop].strMac = devInfo.strMac;
	m_mapScanDeviceData[strIPHop].strSiteAlias = devInfo.strSiteAlias;
	m_mapScanDeviceData[strIPHop].strFirmwareVersion = devInfo.strFirmwareVersion;
	m_mapScanDeviceData[strIPHop].strFirmwareVersionDate = devInfo.strFirmwareVersionDate;
	m_mapScanDeviceData[strIPHop].strMUAPPVersion = devInfo.strMUAPPVersion;
	m_mapScanDeviceData[strIPHop].strMUAPPVersionDate = devInfo.strMUAPPVersionDate;
	m_mapScanDeviceData[strIPHop].strFPGAVersion = devInfo.strFPGAVersion;
	m_mapScanDeviceData[strIPHop].strFPGALocation = devInfo.strFPGALocation;
	m_mapScanDeviceData[strIPHop].strDPSFwVersion = devInfo.strDPSFwVersion;
	m_mapScanDeviceData[strIPHop].strDPSFPGAVersion = devInfo.strDPSFPGAVersion;
	m_mapScanDeviceData[strIPHop].strMULocation = devInfo.strMULocation;
	m_mapScanDeviceData[strIPHop].nHopNum = devInfo.nHopNum;
	m_mapScanDeviceData[strIPHop].nLinkNum = devInfo.nLinkNum;
	m_mapScanDeviceData[strIPHop].getLastHop = devInfo.getLastHop;
	m_mapScanDeviceData[strIPHop].vecLinkDev = devInfo.vecLinkDev;
	m_mapScanDeviceData[strIPHop].recvPT = devInfo.recvPT;

	m_mapScanDeviceData[strIPHop].tMainBoardInfo.strHardwareUID = devInfo.tMainBoardInfo.strHardwareUID;
	m_mapScanDeviceData[strIPHop].tMainBoardInfo.strHardwareSN = devInfo.tMainBoardInfo.strHardwareSN;
	m_mapScanDeviceData[strIPHop].tMainBoardInfo.strHardwareVersion = devInfo.tMainBoardInfo.strHardwareVersion;
	m_mapScanDeviceData[strIPHop].tMainBoardInfo.strHardwareOEM = devInfo.tMainBoardInfo.strHardwareOEM;
	// 根据标志决定是否将设备添加到连接设备列表
    if (autoAddToConnect) {
        m_mapConnetDeviceData[strIPHop] = m_mapScanDeviceData[strIPHop];
        ALOG_INFO("Device %s automatically added to connection list.", "CU", "--", strIPHop.c_str());
    }

_end:
	return bRet;
}

void AngKDeviceModel::GetScanDevMap(std::map<std::string, DeviceStu>& _devMap)
{
	QMutexLocker locker(&m_mutex);
	_devMap = m_mapScanDeviceData;
}

void AngKDeviceModel::ClearScanDev()
{
	QMutexLocker locker(&m_mutex);
	m_mapScanDeviceData.clear();
}

void AngKDeviceModel::AddConnetDevIP(std::string strIPHop)
{
	QMutexLocker locker(&m_mutex);
	DeviceStu newDevStu;
	m_mapConnetDeviceData[strIPHop] = newDevStu;
}

bool AngKDeviceModel::FindConnetDev(std::string strIP)
{
	QMutexLocker locker(&m_mutex);
	for (auto iter : m_mapConnetDeviceData)
	{
		if (iter.second.strIP == strIP)
			return true;
	}

	return false;
}


bool AngKDeviceModel::FindConnetDevByIPHop(std::string strIPHop)
{
	QMutexLocker locker(&m_mutex);
	auto it = m_mapConnetDeviceData.find(strIPHop);
	if (it != m_mapConnetDeviceData.end()) {
		return true;
	}

	return false;
}

bool AngKDeviceModel::SetConnetDevInfo(std::string strIPHop, DeviceStu devInfo)
{
	bool bRet = FindConnetDevByIPHop(strIPHop);
	QMutexLocker locker(&m_mutex);
	if (!bRet) {
		goto _end;
	}

	m_mapConnetDeviceData[strIPHop].strIP = devInfo.strIP;
	m_mapConnetDeviceData[strIPHop].nChainID = devInfo.nChainID;
	m_mapConnetDeviceData[strIPHop].strPort = devInfo.strPort;
	m_mapConnetDeviceData[strIPHop].strMac = devInfo.strMac;
	m_mapConnetDeviceData[strIPHop].strSiteAlias = devInfo.strSiteAlias;
	m_mapConnetDeviceData[strIPHop].strFirmwareVersion = devInfo.strFirmwareVersion;
	m_mapConnetDeviceData[strIPHop].strFirmwareVersionDate = devInfo.strFirmwareVersionDate;
	m_mapConnetDeviceData[strIPHop].strMUAPPVersion = devInfo.strMUAPPVersion;
	m_mapConnetDeviceData[strIPHop].strMUAPPVersionDate = devInfo.strMUAPPVersionDate;
	m_mapConnetDeviceData[strIPHop].strFPGAVersion = devInfo.strFPGAVersion;
	m_mapConnetDeviceData[strIPHop].strFPGALocation = devInfo.strFPGALocation;
	m_mapConnetDeviceData[strIPHop].strDPSFwVersion = devInfo.strDPSFwVersion;
	m_mapConnetDeviceData[strIPHop].strDPSFPGAVersion = devInfo.strDPSFPGAVersion;
	m_mapConnetDeviceData[strIPHop].strMULocation = devInfo.strMULocation;
	m_mapConnetDeviceData[strIPHop].nHopNum = devInfo.nHopNum;
	m_mapConnetDeviceData[strIPHop].nLinkNum = devInfo.nLinkNum;
	m_mapConnetDeviceData[strIPHop].getLastHop = devInfo.getLastHop;
	m_mapConnetDeviceData[strIPHop].vecLinkDev = devInfo.vecLinkDev;

	m_mapConnetDeviceData[strIPHop].tMainBoardInfo.strHardwareUID = devInfo.tMainBoardInfo.strHardwareUID;
	m_mapConnetDeviceData[strIPHop].tMainBoardInfo.strHardwareSN = devInfo.tMainBoardInfo.strHardwareSN;
	m_mapConnetDeviceData[strIPHop].tMainBoardInfo.strHardwareVersion = devInfo.tMainBoardInfo.strHardwareVersion;
	m_mapConnetDeviceData[strIPHop].tMainBoardInfo.strHardwareOEM = devInfo.tMainBoardInfo.strHardwareOEM;

_end:
	return bRet;
}

void AngKDeviceModel::GetConnetDevMap(std::map<std::string, DeviceStu>& _devMap)
{
	QMutexLocker locker(&m_mutex);
	_devMap = m_mapConnetDeviceData;
}

int AngKDeviceModel::GetConnetDevMapSize()
{
	return m_mapConnetDeviceData.size();
}

void AngKDeviceModel::ClearConnetDev(std::string strIPHop)
{
	QMutexLocker locker(&m_mutex);
	m_mapConnetDeviceData.erase(strIPHop);
}

void AngKDeviceModel::AddScanDevChain(std::string strIPHop)
{
	QMutexLocker locker(&m_mutex);
	DeviceStu newDevStu;
	QStringList IPHopList = QString::fromStdString(strIPHop).split(":");
	int findChain = -1;
	if (IPHopList.size() > 0) {
		for (auto scanDev : m_mapScanDeviceData)
		{
			if (scanDev.first == IPHopList[0].toStdString()) {
				findChain = scanDev.second.nChainID;
				newDevStu.strIP = scanDev.second.strIP;
				newDevStu.strPort = scanDev.second.strPort;
				newDevStu.nHopNum = IPHopList[1].toInt();
			}
		}
	}

	if (findChain < 0)
		newDevStu.nChainID = m_mapScanDeviceData.size() + 1;
	else
		newDevStu.nChainID = findChain;

	m_mapScanDeviceData[strIPHop] = newDevStu;
}

DeviceStu AngKDeviceModel::GetConnetDev(std::string strIPHop)
{
	bool bRet = FindConnetDevByIPHop(strIPHop);
	if (!bRet) {
		return DeviceStu();
	}

	return m_mapConnetDeviceData[strIPHop];
}

bool AngKDeviceModel::FindScanDevByIPHop(std::string strIPHop, DeviceStu& devInfo)
{
	QMutexLocker locker(&m_mutex);
	auto it = m_mapScanDeviceData.find(strIPHop);
	if (it != m_mapScanDeviceData.end()) {
		devInfo = m_mapScanDeviceData[strIPHop];
		return true;
	}

	return false;
}
