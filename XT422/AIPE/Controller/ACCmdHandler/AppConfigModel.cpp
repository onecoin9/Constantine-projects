#include "AppConfigModel.h"

int32_t CAppConfigModel::LoadConfig(QString strConfig)
{
	int32_t Ret = 0;
	m_strConfigPath = strConfig;
	QSettings AppSetting(strConfig, QSettings::IniFormat);
	if (AppSetting.status() != QSettings::NoError) {
		Ret = ERR_CONFIG_LoadIni;
		goto __end;
	}

	m_RemoteIP= AppSetting.value("DeviceComm/RemoteIP","0.0.0.0").toString();
	m_RemotePort = AppSetting.value("DeviceComm/RemotePort",0).toInt();
	m_LocalIP = AppSetting.value("DeviceComm/LocalIP","0.0.0.0").toString();
	m_LocalPort = AppSetting.value("DeviceComm/LocalPort",0).toInt();
	m_NetCommCmdAckTimeoutms = AppSetting.value("DeviceComm/CmdAckTimeoutms", -1).toInt();
	m_CmdQueueAvailableTimeoutms = AppSetting.value("DeviceComm/CmdQueueAvailableTimeoutms", -1).toInt();
	m_SoftCRC32En = AppSetting.value("DeviceComm/SoftCRC32En",1).toInt();

	m_LogLevel = AppSetting.value("Log/LogLevel", 1).toInt();
	m_DataFilePath = AppSetting.value("DataFile/Path", "").toString();

__end:
	return Ret;
}
int32_t CAppConfigModel::SaveConfig(QString strConfig)
{
	int32_t Ret = 0;
	QSettings* pAppSetting = NULL; 
	if (strConfig!="") {
		pAppSetting = new QSettings(strConfig, QSettings::IniFormat);
	}
	else {
		pAppSetting = new QSettings(m_strConfigPath, QSettings::IniFormat);
	}

	if (pAppSetting->status() != QSettings::NoError) {
		Ret = ERR_CONFIG_LoadIni;
		goto __end;
	}

	pAppSetting->setValue("DeviceComm/RemoteIP", m_RemoteIP);
	pAppSetting->setValue("DeviceComm/RemotePort", m_RemotePort);
	pAppSetting->setValue("DeviceComm/LocalIP", m_LocalIP);
	pAppSetting->setValue("DeviceComm/LocalPort", m_LocalPort);
	pAppSetting->sync(); ///需要同步才能写入
__end:
	if (pAppSetting) {
		delete pAppSetting;
	}
	return Ret;
}