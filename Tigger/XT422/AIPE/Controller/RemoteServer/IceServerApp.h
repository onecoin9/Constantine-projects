#ifndef _ICESERVERAPP_H_
#define _ICESERVERAPP_H_

#include <afxcmn.h>
#include "RemoteServer.h"
#include "../ComStruct/Serial.h"
#include "../ComStruct/WorkThread.h"
#include "../ComStruct/AprogDev.h"


#define TMSG_ICESERVER			(WM_USER+0x300)
#define TMSG_ICEDOJOB			(WM_USER+0x301)
#define TMSG_ICEINITSITEENV		(WM_USER+0x302)
#define TMSG_ICELOADPROJECT		(WM_USER+0x303)
#define TMSG_ICEDOJOBDEMO			(WM_USER+0x304)
#define TMSG_ICECREATEPROJECT	(WM_USER+0x305)
#define TMSG_ICECREATEPROJBYTEMPLATE (WM_USER+0x306)
#define TMSG_ICEDOCUSTOMINTHREAD			(WM_USER+0x307)
#define TMSG_ICESETPRINTRESULT			(WM_USER+0x308)

typedef struct tagServerCfg
{
	UINT Port;	///���еĶ˿�
	INT DemoEn;///�Ƿ�ʹ��Demo
	UINT CharSet; ////�ַ������뷽ʽ
	CWnd* pMainWnd; ///�����Ҫ��ʾ���ڣ���ָ�򴰿ڵ�ָ��
}tServerCfg;

typedef struct tagDoCustomData{
	CString strDevSN;
	UINT CustomTag;
	UINT TagDataSize;
	BYTE*pTagData;
}tDoCustomData;

enum eDemoSiteStatus{
	DEMOSTATUS_UNUSED=-1,
	DEMOSTATUS_INACTIVE=0,
	DEMOSTATUS_BUSY=1,
	DEMOSTATUS_PASS=2,
	DEMOSTATUS_FAIL=3,
};

typedef struct tagVirtualSite{
	CString strSN;
	CString strAlias;
	INT SktStatus[8];   ///-1:���� 0:InActive  1:Busy   2:Pass   3:Fail
	UINT ErrCode[8];	///8��վ��Ĵ������
	CDialog *pDlgDemoSite; //վ���Ӧ��Դ
	CWorkThread m_wtRmtJob;   ///RmtServer��ʱ����������߳�
}tVirtualSite; ///����վ����Ϣ

typedef struct tagDemoCfg
{
	CString strDevType; ///;���ɵĻ���
	CString strDevName; ///����
	INT SiteNum;   ///;����վ��ĸ�����ֵ<=16
	INT SocketNumPerSite;  ///;����ÿ��վ��ӵ�е�Socket������ֵ<=8
	UINT Delay; ///;��������ִ��ʱ��Ĭ��Delayʱ�䣬��λΪms���������ڽ������޸�
	CString strErrCode;
	CDialog *pDlgServerDemo; ///��Ӧ�ĶԻ���
	std::vector<tVirtualSite*> vpVirtualSites; 
	tagDemoCfg(){
		ReInit();
	};
	void ReInit(){
		INT i=0;
		strDevType="";
		SiteNum=1;
		SocketNumPerSite=1;
		Delay=200;
		pDlgServerDemo=NULL;
		for(i=0;i<(INT)vpVirtualSites.size();i++){
			if(vpVirtualSites[i]){
				if(vpVirtualSites[i]->pDlgDemoSite){
					delete vpVirtualSites[i]->pDlgDemoSite;
				}
				vpVirtualSites[i]->m_wtRmtJob.DeleteThread();
				delete vpVirtualSites[i];
			}
		}
		vpVirtualSites.clear();
	}
}tDemoCfg;

class CIceServerApp
{
	typedef INT (CIceServerApp::*FnCmdHandler)(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	typedef struct tagCmdHandle{
		::std::string strCmd;
		FnCmdHandler pCmdHandler;
	}tCmdHandler;

public:
	CIceServerApp();
	virtual ~CIceServerApp();
	RemoteAprog::tagRESP ServerDoCmd( const ::std::string&StrCmd, const ::RemoteAprog::tagMSG&Msg, const ::Ice::Current& =::Ice::Current());
	
	/************
	@brief ����ͻ���ִ����־���
	@param[in] strDevSN:ΪALL��ʾ��ָ��վ�㣬����ֵ��ʾָ��վ��
	@param[in] LogLevel: ��־�ļ���
	@param[in] fmt:��־�ַ���
	****************/
	void ClientOutput(CString strDevSN,UINT LogLevel,const char*fmt,...);

	/************
	@brief ����ͻ���ִ�н�������ʾ
	@param[in] strDevSN:ΪALL��ʾ��ָ��վ�㣬����ֵ��ʾָ��վ��
	@param[in] vCur: ���ȵ�ǰֵ
	@param[in] Total:������ֵ
	****************/
	void ClientSetProgress(CString strDevSN,UINT vCur,UINT Total);

	/************
	@brief �����״̬�����仯������ͻ��˽��м�¼
	@param[in] strDevSN:ΪALL��ʾ��ָ��վ�㣬����ֵ��ʾָ��վ��
	@param[in] vCur: ���ȵ�ǰֵ
	@param[in] Total:������ֵ
	****************/
	INT ClientStatusChange(CString strDevSN,BYTE*pData,INT Size);

	/************
	@brief ��¼�������仯������ͻ��˽��м�¼
	@param[in] strJson:��ʾ�仯��������Ϣ
	****************/
	INT ClientSktUIDFecthed(CString strJson);

	INT ClientUIDFetched(CString strJson);
	/************
	@brief ����ͻ���ִ��Adapter״̬����
	@param[in] strDevSN:ΪALL��ʾ��ָ��վ�㣬����ֵ��ʾָ��վ��
	@param[in] AdpCnt: ��������
	@param[in] AdpStatus:���������״̬��bit0-1��ʾAdp1��bit2-3��ʾAdp2��00b��ʾInactive��01b��ʾBusy��
						10b��ʾPass��11b��ʾError
	****************/
	void ClientSetStatus(CString strDevSN,UINT AdpCnt,UINT AdpStatus);
	
	/************
	@brief ����ͻ���ִ��DoJob���֮���Adapter״̬����
	@param[in] strDevSN:ΪALL��ʾ��ָ��վ�㣬����ֵ��ʾָ��վ��
	@param[in] strCmd:DoJob��ɵ�����
	@param[in] AdpCnt: ��������
	@param[in] AdpStatus:���������״̬��bit0-1��ʾAdp1��bit2-3��ʾAdp2��00b��ʾInactive��01b��ʾBusy��
	10b��ʾPass��11b��ʾError
	****************/
	void ClientSetJobResult(CString strDevSN, CString strCmd,UINT AdpCnt, UINT AdpStatus);
	/******
	ClientSetJobResultExtΪClientSetJobResult����չ������ͳ�ƴ����Ĵ��ͣ������ö��ο����������л�����ʵ�ǰͳ�ƴ�����Ϣ
	******/
	void ClientSetJobResultExt(CString strDevSN, CString strCmd,UINT AdpCnt, UINT AdpStatus,UINT PassCntTotal,UINT FailCntTotal);
	
	void ClientSetJobResultExt_Json(CString strDevSN, CString strCmd,UINT AdpCnt, UINT AdpStatus,UINT PassCntTotal,UINT FailCntTotal);
	void ClientSetMissionResult(CString strJson);
	void ClientSetPrintContent(INT SiteIdx, INT SktIdx, BOOL bSwap, CString Manufactor, CString Eid, CString Sn, INT Result);
	///���ÿͻ������������ִ��InitSiteEnv����Ľ��
	void ClientSetInitResult(CSerial&lSerial);///lSerial�Ĳ������ֲο����ο����ĵ�

	///���ÿͻ������������ִ��LoadProject����Ľ����strDevSN�̶�ΪALL,ResultΪ���
	void ClientSetLoadResult(CString strDevSN,INT Result);
	
	///�ͻ������������ִ��SetChecksumExt_Json��������������չУ��ֵ��Ϣ
	void ClientSetChecksumExt_Json(CString strChecksumExt_Json);

	///�ͻ������������ִ��FileImportInfo_Json�����������ͼ��ص����ļ���Ϣ
	void ClientFileImportInfo_Json(CString strChecksumExt_Json);

	///�ͻ������������ִ��eMMCOption_Json������������emmcĸƬ����ʱ�ĵ�����Ϣ
	void ClienteMMCOption_Json(CString streMMCOption_Json);

	void ConstructAdapterInfo_Json(CSerial& lSerial, std::map<CString,SKBINFO> mapSkbInfo);
	
	void ClientSetFw_Result(CString strJson);
	void ClientSetJobStart(CString strDevSN, CString strCmd,UINT AdpCnt);
	/************
	@brief ����ͻ���ִ���Զ�������
	@param[in] strDevSN:ΪALL��ʾ��ָ��վ�㣬����ֵ��ʾָ��վ��
	@param[in] TagID:ָ���໥Լ����Tag
	@param[in] pData:ִ������ʱ���Ĳ���
	@param[in] DataSize:�����Ĵ�С
	@param[out] ppRespData:�����������ڵ��ڴ�������Ҫ��FreeData�ͷ�
	@param[out] pRespDataSize:�������ݴ�С
	return:
		�ɹ�����0��ʧ�ܷ��ط�0
	************************/
	INT ClientDoCustom(CString strDevSN,UINT TagID,BYTE*pData,UINT DataSize,BYTE**pRespData=NULL,INT*pRespDataSize=NULL);
	
	///���ÿͻ������������ִ��CreateProject����Ľ����strDevSN�̶�ΪALL,ResultΪ���
	void ClientSetCreateProjResult(CString strDevSN,INT Result);
	void ClientSetCreateProjResultJson(CString strResultJson);
	CString GetCreateProjectResultJson();

	
	static void FreeData(BYTE*pData){if(pData){delete[] pData;}};
	

	static CString GetCmdName(INT DevCmd);///����ת�ַ���
	INT DoJobInWorkThread(CString DevSN,INT DevCmd);///��Job�����ִ�зŵ��߳���
	INT CustomInWorkThread(CString strDevSN,UINT CustomTag, UINT TagDataSize, BYTE*pTagData);
	
	INT ServerDoInitSiteEnv();
	INT ServerDoLoadProject();
	INT ServerDoCreateProject();
	INT ServerDoCreateProjectByTemplate();

	INT LoadDemoCfg();
	INT CreateDemoDiallog();
	INT GetDemoCmdExecDelayTime();
	INT GetDemoUserResult(tVirtualSite *pVirtualSite,UINT&AdpCnt,UINT&AdpStatus);
	INT SetAdpStatusDemo(const CString&strDevSN,UINT AdpCnt,const char *Status);

	///////���ñ�����¼�������ļ����������Զ������
	INT n_PassCntTotal,n_FailCntTotal;
	CString strIniFilePath;
	INT SetChipSNPFile(INT PassCnt,INT FailCnt);
	CString SetIniFileFile();

	tServerCfg m_ServerCfg;///ServerCfg
	ClientPrx m_ClientApp;
	tDemoCfg m_DemoCfg;
	
	INT SiteDoCustom(const CString&strDevSN,UINT TagID,UINT TagDataSize,BYTE*pTagData);
	//////////////////////////////////////////////////////////////////////////
	///������ʾ
	BOOL ShowSiteCtrlWindow(INT DevCmd);
	BOOL CloseSiteCtrlWindow();
	INT PrepareStatisticDlgSite(DEVINFO*pDevInfo,INT DevCmd);
	INT UpdateStatisticDlgSite(DEVINFO*pDevInfo,INT DevCmd,INT Result);

	int WaitAdpNotifyConfirm();

protected:
	///����������ִ�е�����
	INT HandleGetVersion(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleSiteScanAndConnect(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleSiteDisconnect(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleLoadPorject(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleGetProjectInfo( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp );
	INT HandleInitSiteEnv(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleSetAdapterEn(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleGetAdapterEn(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleEnableSN( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp );
	INT HandleSetSN(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleDoJob(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleDoCustom(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleDoCustomInThread(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleSetSNStrategy(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleSetLabel(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleSetChipVID(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleGetSNCount(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleGetStatisticCnt(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleEnableCheckID(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleSetPrintResult(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleConnectAutomatic(const ::RemoteAprog::tagMSG& Msg, RemoteAprog::tagRESP& Resp);
	INT HandleDisConnectAutomatic(const ::RemoteAprog::tagMSG& Msg, RemoteAprog::tagRESP& Resp);
	INT HandleShowCmdSite(const ::RemoteAprog::tagMSG& Msg, RemoteAprog::tagRESP& Resp);
	INT HandleGetConfig_Json(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleGetConfig_String(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleReadBuffer(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleReadProjBuffer(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleReadChipAllBuffer(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleReadProjAllBuffer(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleGetChipConfig_String(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);


	///ΪMES׼���ĺ���
	INT HandleCreateProjectByTemplate( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleCreateProject(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleSetProjectParameter(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleGetProjectInfoExt(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleQueryStatus(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);
	INT HandleGetSktCounter( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp );
	INT HandleGetAdapterInfo( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp );
	INT HandleGetSktCounterExt( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp );
	INT HandleGetReportJson( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp );
	INT HandleGetSktInfoJson( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp );
	INT HandleGetSpecialBit(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp);

	INT HandleGetErrCode( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp );

	INT HandleAllResetcounter( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp );
	INT HandleResetcounter( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp );

	INT HandleSendChemID( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp );

	INT CreateProjectInWorkThread();
	INT CreateProjectByTempleateInWorkThread();
	

	INT GetDevCmd(const CString strCmd);
	INT GetCheckedCmd(INT Cmd);

	void InitCmdHandlers();

	INT InitEnvInWorkThread(void);
	INT LoadProjectInWorkThread(CString strProject);

	INT SerialCmdData(const ::RemoteAprog::tagMSG&Msg, CSerial& lSerial);

	////Custom
	INT LocalDoCustm(const CString&strDevSN,UINT TagID,UINT TagDataSize,BYTE*pTagData);
	//INT SiteDoCustom(const CString&strDevSN,UINT TagID,UINT TagDataSize,BYTE*pTagData);
	INT HandleCTagGetAllSkbInfoDemo(const CString& strDevSN,UINT TagID,UINT TagDataSize,BYTE *pTagData);
	INT HandleCTagGetAllSkbUIDDemo(const CString& strDevSN,UINT TagID,UINT TagDataSize,BYTE *pTagData);

	INT ConstructSktInfoExt_Json(CSerial& lSerial,DEVINFO*pDevInfo);
	INT BuildSktInfoJson(CSerial& lSerial,DEVINFO*pDevInfo);
	INT HandleNewOlderIDBurn( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp );

private:
	CIceServerApp(const CIceServerApp&);
	CIceServerApp& operator=(const CIceServerApp&);
	std::vector<tCmdHandler> vCmdHandler;
	CRITICAL_SECTION m_csClient;
	CRITICAL_SECTION m_csCmdProtct; ///����ִ�б�����������߳�һ��
	CWorkThread m_wtServerApp;
	CString m_strCurProject;
	CSerial m_lCreateProjInfo; ///��Ź����ļ�����ʱ�ĺ�������

	CRITICAL_SECTION m_csSiteDoJob;
	DEVINFO *m_pDevInfo;
};


CIceServerApp& GetGlobalServerApp();

#endif 
