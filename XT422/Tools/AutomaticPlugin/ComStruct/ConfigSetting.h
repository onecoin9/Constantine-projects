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
	uint64_t m_nItemSiteEn; //վ���ʹ�����

	bool m_bCheckCRC;

	bool m_bACSvcDemoEn;	///�Ƿ�ʹ�ܷ�����Demoģʽ
	uint32_t m_StopWhenFailCnt;	///ĳ�����������ۼƴﵽ���ٴ�֮���Զ���ʹ�ܣ�0��ʾ��ʹ��
	bool m_bACSvcVerbose;	///��������ӡ�Ƿ��
	bool m_AutomaticEn;		///�Ƿ�ʹ���Զ����豸
	int m_QCtrlDefault; ////��ini�����õ�Ĭ���������Ƹ���
	std::string m_SitesGroup;	///���ӵ�վ�������
	
	int m_WaitCylinderUp;	////�ȴ�����̧�����ʱʱ��
	bool m_bSwapSKT1234_5678;///�Ƿ�SKT1234��5678λ�öԵ���
	bool m_bNeedSwapSKT1234_5678;///�Ƿ���Ҫ��������λ�öԵ���
	uint8_t m_MaxSiteNum;
	uint8_t m_ElectricInsertionCheck;///�Ƿ��M����әz�y
	int m_AdapterCnt;///����ѡ��֮�󣬵�ǰ�������е���������

	std::string m_strCallExePath; //�����·��
	std::string m_strCallExeConfigFile; //�����Exe�����ļ�
	std::string m_strCallExeLogFolder; //�����Exe��־�ļ���
	int m_nCallExeEn; //�Ƿ����Զ�����
	std::string m_strACServerFolder; //MultiAprog.exe������·��
	std::string m_strChipName;
	std::string m_strAutoType; //�Զ����ͺ�IPS5200/IPS5000/IPS3000

	int m_nSaveSelPath;
	std::string m_strSelContentPath;
	std::string m_strSelProgramPath;
	std::string m_strSelAutotskPath;

};
