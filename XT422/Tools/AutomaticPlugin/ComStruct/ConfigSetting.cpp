
#include "ConfigSetting.h"
#include "./ComTool.h"
#include "../SProtocal/PckMsg.h"

CConfigSetting::CConfigSetting(void)
{
}

CConfigSetting::~CConfigSetting(void)
{
}

int CConfigSetting::SaveSetting()
{
	int Ret=0;
	std::string strTmp;
	std::string ExePath= ComTool::GetCurrentPath();
	std::string strIniFile = ExePath+"/Config.ini";

	//strTmp = m_strACServerFolder;
	//WritePrivateProfileStringA("Config", "AprogPath", strTmp.c_str(), strIniFile.c_str());

	strTmp = m_strIPAddress;
	WritePrivateProfileStringA("Config", "IPAddr", strTmp.c_str(), strIniFile.c_str());
	
	char buf[100];
	memset(buf, 0, 100);
	sprintf(buf, "%u", m_uPort);
	strTmp = buf;
	WritePrivateProfileStringA("Config", "IPPort", strTmp.c_str(), strIniFile.c_str());

	//////////////
	strTmp = m_SitesGroup;
	WritePrivateProfileStringA("Config", "SitesGroup", strTmp.c_str(), strIniFile.c_str());

	memset(buf, 0, 100);
	sprintf(buf, "%u", m_StopWhenFailCnt);
	strTmp = buf;
	WritePrivateProfileStringA("Config", "StopWhenFailCnt", strTmp.c_str(), strIniFile.c_str());

	memset(buf, 0, 100);
	sprintf(buf, "%d", m_WaitCylinderUp);
	strTmp = buf;
	WritePrivateProfileStringA("Config", "WaitcylinderUp", strTmp.c_str(), strIniFile.c_str());

	memset(buf, 0, 100);
	sprintf(buf, "%d", m_bNeedSwapSKT1234_5678);
	strTmp = buf;
	WritePrivateProfileStringA("Config", "SwapSKT1234_5678", strTmp.c_str(), strIniFile.c_str());

	memset(buf, 0, 100);
	sprintf(buf, "%d", m_MaxSiteNum);
	strTmp = buf;
	WritePrivateProfileStringA("Config", "MAXSiteNum", strTmp.c_str(), strIniFile.c_str());

	memset(buf, 0, 100);
	sprintf(buf, "%lld", m_nItemSiteEn);
	strTmp = buf;
	WritePrivateProfileStringA("Config", "ItemSiteEn", strTmp.c_str(), strIniFile.c_str());

	strTmp = m_strComMap;
	WritePrivateProfileStringA("Config", "ComMap", strTmp.c_str(), strIniFile.c_str());

	strTmp = m_strSKTMap;
	WritePrivateProfileStringA("Config", "SKTMap", strTmp.c_str(), strIniFile.c_str());

	strTmp = m_strCallExePath;
	WritePrivateProfileStringA("Config", "CallExePath", strTmp.c_str(), strIniFile.c_str());

	strTmp = m_strCallExeConfigFile;
	WritePrivateProfileStringA("Config", "CallExeConfigFile", strTmp.c_str(), strIniFile.c_str());

	strTmp = m_strCallExeLogFolder;
	WritePrivateProfileStringA("Config", "CallExeLogFolder", strTmp.c_str(), strIniFile.c_str());

	memset(buf, 0, 100);
	sprintf(buf, "%d", m_nCallExeEn);
	strTmp = buf;
	WritePrivateProfileStringA("Config", "CallExeEn", strTmp.c_str(), strIniFile.c_str());

	strTmp = m_strChipName;
	WritePrivateProfileStringA("Config", "ChipName", strTmp.c_str(), strIniFile.c_str());

	strTmp = m_strAutoType;
	WritePrivateProfileStringA("Config", "AutoType", strTmp.c_str(), strIniFile.c_str());

	memset(buf, 0, 100);
	sprintf(buf, "%d", m_nSaveSelPath);
	strTmp = buf;
	WritePrivateProfileStringA("Config", "SaveSelPath", strTmp.c_str(), strIniFile.c_str());

	strTmp = m_strSelContentPath;
	WritePrivateProfileStringA("Config", "SelContentPath", strTmp.c_str(), strIniFile.c_str());

	strTmp = m_strSelProgramPath;
	WritePrivateProfileStringA("Config", "SelProgramPath", strTmp.c_str(), strIniFile.c_str());

	strTmp = m_strSelAutotskPath;
	WritePrivateProfileStringA("Config", "SelAutotskPath", strTmp.c_str(), strIniFile.c_str());

	return 0;
}

int CConfigSetting::LoadSetting()
{
	int Ret=0;
	std::string strDefault;
	std::string ExePath=ComTool::GetCurrentPath();
	std::string strIniFile = ExePath+"\\Config.ini";
	std::string strFCRange;
	CHAR TmpBuf[MAX_PATH];

	
	m_bCheckCRC = GetPrivateProfileIntA("Config", "CheckCRC", 1, strIniFile.c_str());

	bConfigFileCheckCRC = m_bCheckCRC;

	//memset(TmpBuf, 0, MAX_PATH);
	//GetPrivateProfileStringA("Config", "AprogPath", "", TmpBuf, MAX_PATH, strIniFile.c_str());
	//m_strACServerFolder = TmpBuf;

	memset(TmpBuf,0,MAX_PATH);
	GetPrivateProfileStringA("Config", "IPAddr","",TmpBuf, MAX_PATH, strIniFile.c_str());
	m_strIPAddress = TmpBuf;

	m_uPort =GetPrivateProfileIntA("Config","IPPort",1000,strIniFile.c_str());

	memset(TmpBuf, 0, MAX_PATH);
	GetPrivateProfileStringA("Config", "SitesAutoMap", "", TmpBuf, MAX_PATH, strIniFile.c_str());
	m_strSiteAutoMap = TmpBuf;

	memset(TmpBuf, 0, MAX_PATH);
	GetPrivateProfileStringA("Config", "ComMap", "", TmpBuf, MAX_PATH, strIniFile.c_str());
	m_strComMap = TmpBuf;

	memset(TmpBuf, 0, MAX_PATH);
	GetPrivateProfileStringA("Config", "SKTMap", "", TmpBuf, MAX_PATH, strIniFile.c_str());
	m_strSKTMap = TmpBuf;

	m_StopWhenFailCnt = GetPrivateProfileIntA("Config", "StopWhenFailCnt", 0, strIniFile.c_str());
	if (m_StopWhenFailCnt == 0) {
		m_StopWhenFailCnt = 0xFFFFFFF0; ///设定一个很大的值，表示不使能
	}
	m_AutomaticEn = 1;

	m_WaitCylinderUp = GetPrivateProfileIntA("Config", "WaitcylinderUp", 200, strIniFile.c_str());
	m_bNeedSwapSKT1234_5678 = GetPrivateProfileIntA("Config", "SwapSKT1234_5678", 1, strIniFile.c_str());

	memset(TmpBuf, 0, MAX_PATH);
	GetPrivateProfileStringA("Config", "SitesGroup", "", TmpBuf, MAX_PATH, strIniFile.c_str());
	m_SitesGroup = TmpBuf;
	
	memset(TmpBuf, 0, MAX_PATH);
	GetPrivateProfileStringA("Config", "CallExePath", "", TmpBuf, MAX_PATH, strIniFile.c_str());
	m_strCallExePath = TmpBuf;

	memset(TmpBuf, 0, MAX_PATH);
	GetPrivateProfileStringA("Config", "CallExeConfigFile", "", TmpBuf, MAX_PATH, strIniFile.c_str());
	m_strCallExeConfigFile = TmpBuf;

	memset(TmpBuf, 0, MAX_PATH);
	GetPrivateProfileStringA("Config", "CallExeLogFolder", "", TmpBuf, MAX_PATH, strIniFile.c_str());
	m_strCallExeLogFolder = TmpBuf;

	memset(TmpBuf, 0, MAX_PATH);
	GetPrivateProfileStringA("Config", "ChipName", "AG35", TmpBuf, MAX_PATH, strIniFile.c_str());
	m_strChipName = TmpBuf;

	memset(TmpBuf, 0, MAX_PATH);
	GetPrivateProfileStringA("Config", "AutoType", "IPS5000", TmpBuf, MAX_PATH, strIniFile.c_str());
	m_strAutoType = TmpBuf;

	m_MaxSiteNum = GetPrivateProfileIntA("Config", "MAXSiteNum", 9, strIniFile.c_str());

	m_nItemSiteEn = GetPrivateProfileIntA("Config", "ItemSiteEn", 255, strIniFile.c_str());

	m_nCallExeEn = GetPrivateProfileIntA("Config", "CallExeEn", 0, strIniFile.c_str());

	m_nSaveSelPath = GetPrivateProfileIntA("Config", "SaveSelPath", 0, strIniFile.c_str());

	memset(TmpBuf, 0, MAX_PATH);
	GetPrivateProfileStringA("Config", "SelContentPath", "", TmpBuf, MAX_PATH, strIniFile.c_str());
	m_strSelContentPath = TmpBuf;

	memset(TmpBuf, 0, MAX_PATH);
	GetPrivateProfileStringA("Config", "SelProgramPath", "", TmpBuf, MAX_PATH, strIniFile.c_str());
	m_strSelProgramPath = TmpBuf;

	memset(TmpBuf, 0, MAX_PATH);
	GetPrivateProfileStringA("Config", "SelAutotskPath", "", TmpBuf, MAX_PATH, strIniFile.c_str());
	m_strSelAutotskPath = TmpBuf;


	return Ret;
}

