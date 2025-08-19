#pragma once
#include <string>

class CConfigSetting
{
public:
	CConfigSetting(void);
	~CConfigSetting(void);
	int LoadSetting();
	int SaveSetting();

	std::string m_strIPAddress;
	uint32_t m_uPort;
	std::string m_strSiteAutoMap;
	std::string m_strComMap;
	std::string m_strSKTMap;
	uint64_t m_nItemSiteEn; //站点的使能情况

	bool m_bCheckCRC;

	bool m_bACSvcDemoEn;	///是否使能服务器Demo模式
	uint32_t m_StopWhenFailCnt;	///某个适配板出错累计达到多少次之后自动不使能，0表示不使能
	bool m_bACSvcVerbose;	///服务器打印是否打开
	bool m_AutomaticEn;		///是否使能自动化设备
	int m_QCtrlDefault; ////在ini中设置的默认量产控制个数
	std::string m_SitesGroup;	///连接的站点组管理
	
	int m_WaitCylinderUp;	////等待气缸抬起的延时时间
	bool m_bSwapSKT1234_5678;///是否将SKT1234与5678位置对调？
	bool m_bNeedSwapSKT1234_5678;///是否需要进行座子位置对调？
	uint8_t m_MaxSiteNum;
	uint8_t m_ElectricInsertionCheck;///是否進行電子檢測
	int m_AdapterCnt;///工程选择之后，当前工程运行的适配板个数

	std::string m_strCallExePath; //唤起的路径
	std::string m_strCallExeConfigFile; //唤起的Exe配置文件
	std::string m_strCallExeLogFolder; //唤起的Exe日志文件夹
	int m_nCallExeEn; //是否开启自动唤醒
	std::string m_strACServerFolder; //MultiAprog.exe的所在路径
	std::string m_strChipName;
	std::string m_strAutoType; //自动化型号IPS5200/IPS5000/IPS3000

	int m_nSaveSelPath;
	std::string m_strSelContentPath;
	std::string m_strSelProgramPath;
	std::string m_strSelAutotskPath;

};
