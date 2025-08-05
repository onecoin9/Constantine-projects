#pragma once

#include "ErrTypes.h"
//#include <afxcmn.h>
#include <map>

#include <vector>
#include <QMetaType>

typedef struct{
	std::string strComm;		///通信方式
	uint32_t dwUartPort;	///Com口
	bool DumpPackEn;
	uint32_t SendTryCnt;	///每个发送包重复发送几次
	uint32_t DelayModule;	///模组结果发送之间的延时，以ms为单位
}tAutoKProtocalSetting;

typedef struct{
	std::string strComm;
	uint32_t dwUartPort;
	bool DumpPackEn;
	bool StartWaitSendedEn; //是否启用了结果检查
	uint32_t dwRetryCnt;
	uint32_t dwTimeout;
	uint32_t dwRetryTimeWait;

	uint32_t dwBaudRate;
	bool ProtocalNegotiationEn; ///是否使能
}tAutoFProtocalSetting;

typedef struct{
	/*string strComm;*/
	uint32_t dwRetryCnt;
	uint32_t dwTimeout;
	uint32_t dwRetryTimeWait;
	bool DumpPackEn;
	uint32_t dwUartPort;
	uint32_t uActControl;
	uint32_t dwBaudRate;
}tAutoEProtocalSetting;

typedef struct{
	std::string strIPAddress;
	uint32_t Port;
	bool DumpPackEn;
	std::string strMachType;
	std::string strNeedSendTaskDataType;
	std::vector<std::string> m_vAutoMachAllMode;
	std::vector<std::string> m_vNeedSendTaskDataType;
}tAutoSProtocalSetting;

typedef struct{
	unsigned short port;
}tAutoICTNetProtocalSetting;

typedef struct{
	std::string strBarCodeDB;	///条形编码的数据库
	std::string strSFCodeDB;	///软件编码数据库
	std::string strProjPath;	///工程文件存放根路径
	uint32_t nTryWhenFailed;///如果失败允许再尝试的次数
	uint32_t nDelay;  ///在尝试之前做的Delay，单位为s。
}tMideaSetting,tExcelMesSetting;

typedef struct{
	bool MESEn;
	bool MESExChangeModeEn;
	bool MESBKEn;		///使能断点
	std::string MESManu;
	std::string MESFirtBoot;		///实行之后首个被启动的命令
	tMideaSetting MideaSetting;	
	tExcelMesSetting ExcelMesSetting;
}tMESInfo;

typedef struct{
	bool SoundEn;
	bool LoopUntilClear;
	std::string strSoundErr;
	std::string strSoundPass;
}tSound;

typedef struct tagOEMInfo{
	std::string OEMName;
	int OEMCompatible;///是否支持兼容
}tOEMInfo;

typedef struct tagOCXInfo{
	bool WindowEn;
	bool OcxCallEn; ///是否是来之OCX控制
	bool InsetionCheckShowPassIfICOut; ///當執行InsetionCheck命令的時候，如果IC沒有放入，則顯示到界面的時候表示為Pass
}tOCXInfo;

typedef struct tagAdpNotify{
	bool bEn;		///是否使能Notify功能，为1表示是，为0表示不是
	bool bAbsEn;	///使用绝对值
	uint32_t AbsValue;	///绝对值
	uint32_t PercValue; ///百分比
}tAdpNotify;

typedef struct tagUIDInfo{
	int bUIDEn;
	int nUIDMODE;
	int nEndType;
	uint32_t uSwapUnit;
	void ReInit(){
		bUIDEn = 0;
		nUIDMODE = 1;
		nEndType = 1;
		uSwapUnit = 0;
	};
	tagUIDInfo(){ReInit();};
}tUIDInfo;

typedef struct tagGlbSetting
{
	std::string strLogPath;
	ulong dwLogSize;
	ulong dwLogDay;
	
	//uint32_t dwReportAutoSave;
	//uint32_t dwReportSaveAlone;
	//uint32_t dwAdpInfoAutoSave;
	//uint32_t dwCreateNewLogWhenLoadProj;
	//std::string strTmpBufPath;
	//bool bScanUSB;
	//bool bScanNet;
	//bool bAutoConnect;    ///应用程序启动的时候是否自动连接扫描到的站点
	//bool bBreakPoint;
	//std::string strSaveSNCfg;
	//ulong dwSNCfgInProjectHandle;///对于工程文件中存在SN配置是如何处理的？0Disable 1使用SN配置，2将SN配置外化为SNC文件并且加载
	//bool bAutoEn;			///自动化设备是否使能
	//bool bAutoScanAndConnect;  ///自动化设备连接时是否自动扫描并连接自动设备内部的站点
	std::string strProtocalType;	///自动化设备协议
	//tAutoKProtocalSetting AutoKProtocal;
	//tAutoFProtocalSetting AutoFProtocal; 
	
	tAutoSProtocalSetting AutoSProtocal;
	//tAutoEProtocalSetting AutoEProtocal;
	//tAutoICTNetProtocalSetting AutoICTNetProtocal;
	std::map<std::string, std::string> AutoSiteMap; ///自动化站点与实际的Site对应表
	//tMESInfo MESInfo;
	//tAdpNotify AdpNotify;
	//tSound Sound;
	//int nSenorinterval;		////AutoSensor扫描间隔，单位为ms
	//int nSecondMinus;		///统计时偷偷减去的秒数
	//int nStopWhenErrorSequence; ///单个Site连续错误次数达到该设置次数时不再进行下去，设置为0表示不使能
	//int OverlapCheckEn;///是否使能叠料检测 为1表示使能，为0表示不使能
	//tOEMInfo OEMInfo;
	//tOCXInfo OCXInfo;
	//std::string CurLang;	///当前的语言
	//
	//////Project
	//ulong dwMatchDevTypeEn;///是否使能编程器类型匹配
	/////NetScan
	//std::string strNetBroadcastAddr;

	//std::string strCurMachType;//
	//int ChipOverLap;//
	//tagUIDInfo UIDInfo;

	////SNNet
	//std::string SNNetIP;
	//uint32_t SNNetPort;

	////Lic
	//std::string strLicServerIP;
	//std::string strLicFilePath;
	//uint32_t uLicServerPort;
	//uint32_t IsLicEn;
	//uint32_t uLicDelayTime;

	//SysLog
	//int nSysLogEn;// 0：disable, 1:enable
	//int nSaveSiteLog; //0：disable, 1:enable
	//int nDumpaSNLogEn;  //0：disable, 1:enable
	//int nCntInfoEn; //0：disable,  1:enable

	//uint32_t uReportSaveFormat;  //0:html 1:json
	//
	////SN Mode
	//uint32_t uSNMode;
	//bool SNEnableUid;
	//std::string BuildVersion; ///Build版本号
}GLBSETTING; ///全局信息设置

namespace AProgSetting
{
	int InitGlbSetting(GLBSETTING *pGlbSetting);
	int SaveGlbSetting(GLBSETTING *pGlbSetting);
	bool IsAutomaticEn(GLBSETTING* pGlbSetting);
	bool IsAutoScanAndConnect(GLBSETTING*pGlbSetting);
	std::string GetAutoManu(GLBSETTING*pGlbSetting);

	bool EnCode(char*pSrcData,int SrcSize,char*pDestData,int DestSize);
	bool DeCode(char*pSrcData,int SrcSize,char*pDestData,int DestSize);

	bool IsMESEn(GLBSETTING*pGlbSetting);
	bool IsMESExChangeModeEn(GLBSETTING*pGlbSetting);
	std::string GetMESManu( GLBSETTING*pGlbSetting);
	std::string GetMESFirstBoot( GLBSETTING*pGlbSetting );


	std::string GetSoundPassPath(GLBSETTING*pGlbSetting);
	std::string GetSoundErrPath(GLBSETTING*pGlbSetting);
};

int SetLang(std::string strLang);
int GetLang(std::string& strLang);

