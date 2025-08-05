#pragma once

#include "ErrTypes.h"
//#include <afxcmn.h>
#include <map>

#include <vector>
#include <QMetaType>

typedef struct{
	std::string strComm;		///ͨ�ŷ�ʽ
	uint32_t dwUartPort;	///Com��
	bool DumpPackEn;
	uint32_t SendTryCnt;	///ÿ�����Ͱ��ظ����ͼ���
	uint32_t DelayModule;	///ģ��������֮�����ʱ����msΪ��λ
}tAutoKProtocalSetting;

typedef struct{
	std::string strComm;
	uint32_t dwUartPort;
	bool DumpPackEn;
	bool StartWaitSendedEn; //�Ƿ������˽�����
	uint32_t dwRetryCnt;
	uint32_t dwTimeout;
	uint32_t dwRetryTimeWait;

	uint32_t dwBaudRate;
	bool ProtocalNegotiationEn; ///�Ƿ�ʹ��
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
	std::string strBarCodeDB;	///���α�������ݿ�
	std::string strSFCodeDB;	///����������ݿ�
	std::string strProjPath;	///�����ļ���Ÿ�·��
	uint32_t nTryWhenFailed;///���ʧ�������ٳ��ԵĴ���
	uint32_t nDelay;  ///�ڳ���֮ǰ����Delay����λΪs��
}tMideaSetting,tExcelMesSetting;

typedef struct{
	bool MESEn;
	bool MESExChangeModeEn;
	bool MESBKEn;		///ʹ�ܶϵ�
	std::string MESManu;
	std::string MESFirtBoot;		///ʵ��֮���׸�������������
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
	int OEMCompatible;///�Ƿ�֧�ּ���
}tOEMInfo;

typedef struct tagOCXInfo{
	bool WindowEn;
	bool OcxCallEn; ///�Ƿ�����֮OCX����
	bool InsetionCheckShowPassIfICOut; ///������InsetionCheck����ĕr�����IC�]�з��룬�t�@ʾ������ĕr���ʾ��Pass
}tOCXInfo;

typedef struct tagAdpNotify{
	bool bEn;		///�Ƿ�ʹ��Notify���ܣ�Ϊ1��ʾ�ǣ�Ϊ0��ʾ����
	bool bAbsEn;	///ʹ�þ���ֵ
	uint32_t AbsValue;	///����ֵ
	uint32_t PercValue; ///�ٷֱ�
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
	//bool bAutoConnect;    ///Ӧ�ó���������ʱ���Ƿ��Զ�����ɨ�赽��վ��
	//bool bBreakPoint;
	//std::string strSaveSNCfg;
	//ulong dwSNCfgInProjectHandle;///���ڹ����ļ��д���SN��������δ���ģ�0Disable 1ʹ��SN���ã�2��SN�����⻯ΪSNC�ļ����Ҽ���
	//bool bAutoEn;			///�Զ����豸�Ƿ�ʹ��
	//bool bAutoScanAndConnect;  ///�Զ����豸����ʱ�Ƿ��Զ�ɨ�貢�����Զ��豸�ڲ���վ��
	std::string strProtocalType;	///�Զ����豸Э��
	//tAutoKProtocalSetting AutoKProtocal;
	//tAutoFProtocalSetting AutoFProtocal; 
	
	tAutoSProtocalSetting AutoSProtocal;
	//tAutoEProtocalSetting AutoEProtocal;
	//tAutoICTNetProtocalSetting AutoICTNetProtocal;
	std::map<std::string, std::string> AutoSiteMap; ///�Զ���վ����ʵ�ʵ�Site��Ӧ��
	//tMESInfo MESInfo;
	//tAdpNotify AdpNotify;
	//tSound Sound;
	//int nSenorinterval;		////AutoSensorɨ��������λΪms
	//int nSecondMinus;		///ͳ��ʱ͵͵��ȥ������
	//int nStopWhenErrorSequence; ///����Site������������ﵽ�����ô���ʱ���ٽ�����ȥ������Ϊ0��ʾ��ʹ��
	//int OverlapCheckEn;///�Ƿ�ʹ�ܵ��ϼ�� Ϊ1��ʾʹ�ܣ�Ϊ0��ʾ��ʹ��
	//tOEMInfo OEMInfo;
	//tOCXInfo OCXInfo;
	//std::string CurLang;	///��ǰ������
	//
	//////Project
	//ulong dwMatchDevTypeEn;///�Ƿ�ʹ�ܱ��������ƥ��
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
	//int nSysLogEn;// 0��disable, 1:enable
	//int nSaveSiteLog; //0��disable, 1:enable
	//int nDumpaSNLogEn;  //0��disable, 1:enable
	//int nCntInfoEn; //0��disable,  1:enable

	//uint32_t uReportSaveFormat;  //0:html 1:json
	//
	////SN Mode
	//uint32_t uSNMode;
	//bool SNEnableUid;
	//std::string BuildVersion; ///Build�汾��
}GLBSETTING; ///ȫ����Ϣ����

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

