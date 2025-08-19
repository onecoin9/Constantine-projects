#pragma once

#include <QObject>
#include <memory>
#include <mutex>
#include "DeviceModel.h"

class AngKDeviceModel : public QObject
{
	Q_OBJECT

public:
	static AngKDeviceModel& instance()
	{
		if (!s_instance)
		{
			autoAddToConnect = false;
			std::call_once(once_flag, []() { s_instance = new AngKDeviceModel(); });
		}
		return *s_instance;
	}

	// 标志：是否自动将扫描设备添加到连接设备
	static bool autoAddToConnect ; // 默认关闭

	//扫描接口
	static void AddScanDevIP(std::string strIP);

	static bool FindScanDev(std::string strIP);

	static bool SetScanDevInfo(std::string strIP, DeviceStu devInfo);

	static void GetScanDevMap(std::map<std::string, DeviceStu>& _devMap);

	static bool FindScanDevByIPHop(std::string strIPHop, DeviceStu& devInfo);

	static void ClearScanDev();

	static void AddScanDevChain(std::string strIPHop);
	//连接接口
	static void AddConnetDevIP(std::string strIPHop);

	static bool FindConnetDev(std::string strIP);

	static bool FindConnetDevByIPHop(std::string strIPHop);

	static bool SetConnetDevInfo(std::string strIPHop, DeviceStu devInfo);

	static void GetConnetDevMap(std::map<std::string, DeviceStu>& _devMap);

	static int GetConnetDevMapSize();

	static void ClearConnetDev(std::string strIPHop);

	static DeviceStu GetConnetDev(std::string strIPHop);
private:

	AngKDeviceModel() = default;
	~AngKDeviceModel() = default;

	static AngKDeviceModel* s_instance;
	static std::once_flag once_flag;
	static std::map<std::string, DeviceStu> m_mapScanDeviceData;//保存所有扫描的设备
	static std::map<std::string, DeviceStu> m_mapConnetDeviceData;//保存所有连接的设备
	
};
