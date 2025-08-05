#ifndef _AUTOSUNS_H_
#define _AUTOSUNS_H_
//#include "../LogMsg.h"
//#include "../ILog.h"
#include "./SProtocal/TcpStdMes.h"

#include "./ComStruct/ConfigSetting.h"
//#include "../ACSvc.h"

typedef struct{
	std::string strIPAddr;
	uint32_t Port;
	uint32_t DumpPackEn;
	std::string strMachType;
}tTCPAutoPara;

typedef struct tagSiteMap{
	std::string strSiteAlias;	///站点别名, 来自于Setting中的设置
	uint32_t SiteIdx;			///站点在自动化中的索引， 来自于Setting中的设置
	std::string strAutoAlias;  ///自动化站点的别名

	uint32_t SiteSktEnAutoOrg;///自动化端客户设置的站点中座子的使能原始值
	uint32_t ChipEnAuto; ///自动化告知的芯片放入情况
	volatile bool bChipReady;///芯片是否已经放入
	///后面的信息在站点初始化完成之后设置
	std::string strSiteSN;	///站点的SN
	bool bSiteEnvInit;	///站点是否已经初始化
	uint32_t SKTRdy;		///初始后扫描到的可以使用的座子

}tSProtocalSiteMap;

typedef struct{
	
}tStdMesSetting;

typedef struct {
	
}tReadyStatus;


class CAutoSProtocal:public IAutomatic/*,public CLogMsg*/
{
public:
	CAutoSProtocal();
	~CAutoSProtocal();
public:
	/*
	* \brief 初始化自动化设备
	* \param[in] ComnType : 使用那种通信方式
	* \param[in] Para	  : 通信方式附加的参数
	* \return 成功返回 0， 失败返回其他值，错误值请统一为负数
	*/
	int InitAutomatic(eAutoComnType ComnType,void*Para);

	/*
	* \brief 释放自动化设备
	* \return 成功返回 0，失败返回其他值，失败值请统一为负数
	*/
	int ReleaseAutomatic(void);


	bool CheckConnect(void);

	/*
	* \brief 动态设置设备的一些属性参数操作
	* \param[in] ParaType : 属性参数类型
	* \param[in] Para	  : 属性参数值
	* \return 成功返回 0， 失败返回其他值，错误值请统一为负数
	*/
	int SetAutomaticPara(eAutoParaType ParaType,void*Para);

	/*
	* \brief 获取自动化设备将哪些Site的IC放置好
	* \param[out] pData : bit0-bit31分别表示站点1到站点32，如果该站点IC已经放入，则置1
	*					  如果多余32个站点，则pData[1]表示33-64号站点，依次类推
	* \param[in] Size : pData有多少个单元可用
	* \return 有站点Ready则返回Ready站点的个数，如果没有则返回0,出错则返回负数的错误码
	*/
	int GetICReadySite(uint32_t *pData, int Size);

	int GetICReadySite(int SiteIdx, uint8_t *pReady);

	int GetICReadySite(int SiteIdx, uint8_t *pReady, uint32_t*pICStatus);

	int ClearICReadySite(int SiteIdx);


	/****************
	自动化设备中站点别名与站点索引号的映射关系,返回的模组编号从1开始
	***************/
	int GetSiteIdx(std::string AutoAlias); ///

	int GetSiteIdxBySn(const std::string& sn);

	/****************
	自动化设备中站点别名与站点索引号的映射关系,返回指定的模组编号对应的站点别名
	***************/
	std::string GetAutoAlias(int SiteIdx);

	/*
	* \brief 告诉站点IC烧录结果
	* \param[in] SiteIdx : 站点编号1-N
	* \param[in] Result : 一个站点可能有多个Adapter同时烧录，Result指定每个Adapter的烧录情况
						  bit0-bit7表示Adapter1-Adapter8，AP8000只有Adapter1使用，Bit位为1表示成功
						  Bit位为0表示失败，AP8800最多使用Adapter1-8
						  当为AP8800时，Mask值的Bit位为1的Adapter表示使能，否则不使能，AP8000时Mask总为1
	* \param[in] Mask : 指定哪些Adapter使能
	* \return 成功返回 0，失败返回其他值，失败值请统一为负数
	*/
	int SetDoneSite(int SiteIdx, uint8_t* Result, uint64_t Mask);

	/*
	* \brief 请求自动化设备告知哪些站点被使能
	* \return 成功返回0，失败返回负值
	*/
	int QuerySiteEn();

	/*
	* \brief 告知自动化设备站点已经初始化完成，可以开始取IC进行烧录
	* \return 成功返回0，失败返回负值
	*/
	int TellDevReady(std::string strRdyInfo);

	/*
	* \brief 通过错误码查询错误信息
	* \param[in] ErrNo : 错误码
	* \param[out] ErrMsg : 错误码对应的消息
	*/
	void GetErrMsg(int ErrNo,std::string& ErrMsg);

	int SetTask(std::string strTask);

	bool GetProtocalVersion();
	bool UseNewVesion();

	void GetSiteMap(std::map<int, std::string>& vSiteMap);

	static bool IsNeedTransferTaskData(std::string strMachType);
	std::string GetMachType();
protected:
	uint32_t GetSiteReadySKTInfo(uint32_t SiteIdx);
	int LoadMesSetting();
	int InitAllSockets();
	int CloseAllSockets();
	int LoadTaskData();
	int AutoTellSiteReady(std::string strRdyInfo);
	int ParserSiteRdyInfo(std::string strRdyInfo);
	void SwapSKT1234_5678(uint8_t& SktEn);
	void SwapSKT1234_5678_V2(uint32_t& SktEn, int SlotCnt);
	int AutoQuerySiteEn();
	bool OnSiteEn(void *Para);

	int AutoSetSiteEn(int SiteCnt,uint8_t *pSiteEn);
	int AutoSetChipEn(int SiteIdx, uint8_t* ChipEn);
	bool OnChipEn(void *Para);
	bool OnHandlePck(void *Para);
	bool OnHandleProgramResultBack(void* Para);
	std::string SendServerAck(std::string command, int Result, std::string ErrMsg);

	int AutoTellICStatus(int SiteIdx, uint32_t AdpCnt, uint32_t *pResult);
	int AutoQueryICStatus(int SiteIdx);
	bool OnQueryICStatus(void *Para);

	int AutoSetResult(int SiteIdx, uint8_t* Result);

	int InitStartUpSocket();
	int SendPD63TellSiteReady(std::string RdyInfo);

private:
	bool m_bAutomaticReady;
	std::string m_ErrMsg;
	tSktOpt m_SktStartUp;
	CTcpStdMes m_TcpStdMes;
	tTCPAutoPara m_Para;
	std::string m_strTask;
	std::mutex m_Mutex;

public:
	std::vector<tSProtocalSiteMap> m_vSiteMap;

	volatile bool m_bAutoSiteEnGet;
	///ini中的设置
	bool m_bACSvcDemoEn;	///是否使能服务器Demo模式
	uint32_t m_StopWhenFailCnt;	///某个适配板出错累计达到多少次之后自动不使能，0表示不使能
	bool m_bACSvcVerbose;	///服务器打印是否打开
	bool m_AutomaticEn;		///是否使能自动化设备
	int m_QCtrlDefault; ////在ini中设置的默认量产控制个数
	std::string m_SitesGroup;	///连接的站点组管理
	std::string m_SiteAutoMap;	///自动化与站点的映射关系
	int m_WaitCylinderUp;	////等待气缸抬起的延时时间
	bool m_bSwapSKT1234_5678;///是否将SKT1234与5678位置对调？
	bool m_bNeedSwapSKT1234_5678;///是否需要进行座子位置对调？
	uint8_t m_MaxSiteNum;
	uint8_t m_ElectricInsertionCheck;///是否進行電子檢測
	int m_AdapterCnt;///工程选择之后，当前工程运行的适配板个数
	CRITICAL_SECTION m_csAuto;

public:
		void AttachHWND(HWND hWnd) { m_AttachHwd = hWnd; };
		HWND m_AttachHwd;

		//void SetLogMsg(ILog*pLogMsg) { m_pLogMsg = pLogMsg; }
		//ILog*m_pLogMsg;
		CConfigSetting m_ConfigSetting;
		bool OnDumpLog(void *Para);

		/*void AttachACServer(CACSvc ACSVC){ m_ACSVC = ACSVC; }
		CACSvc m_ACSVC;*/
};


#endif