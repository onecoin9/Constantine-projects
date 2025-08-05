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
	UINT Port;	///运行的端口
	INT DemoEn;///是否使能Demo
	UINT CharSet; ////字符串编码方式
	CWnd* pMainWnd; ///如果需要显示窗口，则指向窗口的指针
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
	INT SktStatus[8];   ///-1:不用 0:InActive  1:Busy   2:Pass   3:Fail
	UINT ErrCode[8];	///8个站点的错误代码
	CDialog *pDlgDemoSite; //站点对应资源
	CWorkThread m_wtRmtJob;   ///RmtServer的时候处理命令的线程
}tVirtualSite; ///虚拟站点信息

typedef struct tagDemoCfg
{
	CString strDevType; ///;生成的机型
	CString strDevName; ///机型
	INT SiteNum;   ///;设置站点的个数，值<=16
	INT SocketNumPerSite;  ///;设置每个站点拥有的Socket个数，值<=8
	UINT Delay; ///;设置命令执行时的默认Delay时间，单位为ms，后续可在界面上修改
	CString strErrCode;
	CDialog *pDlgServerDemo; ///对应的对话框
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
	@brief 请求客户端执行日志输出
	@param[in] strDevSN:为ALL表示不指定站点，其他值表示指定站点
	@param[in] LogLevel: 日志的级别
	@param[in] fmt:日志字符串
	****************/
	void ClientOutput(CString strDevSN,UINT LogLevel,const char*fmt,...);

	/************
	@brief 请求客户端执行进度条显示
	@param[in] strDevSN:为ALL表示不指定站点，其他值表示指定站点
	@param[in] vCur: 进度当前值
	@param[in] Total:进度总值
	****************/
	void ClientSetProgress(CString strDevSN,UINT vCur,UINT Total);

	/************
	@brief 编程器状态发生变化，请求客户端进行记录
	@param[in] strDevSN:为ALL表示不指定站点，其他值表示指定站点
	@param[in] vCur: 进度当前值
	@param[in] Total:进度总值
	****************/
	INT ClientStatusChange(CString strDevSN,BYTE*pData,INT Size);

	/************
	@brief 烧录座发生变化，请求客户端进行记录
	@param[in] strJson:表示变化的座子信息
	****************/
	INT ClientSktUIDFecthed(CString strJson);

	INT ClientUIDFetched(CString strJson);
	/************
	@brief 请求客户端执行Adapter状态报告
	@param[in] strDevSN:为ALL表示不指定站点，其他值表示指定站点
	@param[in] AdpCnt: 适配板个数
	@param[in] AdpStatus:各个适配板状态，bit0-1表示Adp1，bit2-3表示Adp2，00b表示Inactive，01b表示Busy，
						10b表示Pass，11b表示Error
	****************/
	void ClientSetStatus(CString strDevSN,UINT AdpCnt,UINT AdpStatus);
	
	/************
	@brief 请求客户端执行DoJob完成之后的Adapter状态报告
	@param[in] strDevSN:为ALL表示不指定站点，其他值表示指定站点
	@param[in] strCmd:DoJob完成的命令
	@param[in] AdpCnt: 适配板个数
	@param[in] AdpStatus:各个适配板状态，bit0-1表示Adp1，bit2-3表示Adp2，00b表示Inactive，01b表示Busy，
	10b表示Pass，11b表示Error
	****************/
	void ClientSetJobResult(CString strDevSN, CString strCmd,UINT AdpCnt, UINT AdpStatus);
	/******
	ClientSetJobResultExt为ClientSetJobResult的扩展，增加统计次数的传送，可以让二次开发调用者有机会合适当前统计次数信息
	******/
	void ClientSetJobResultExt(CString strDevSN, CString strCmd,UINT AdpCnt, UINT AdpStatus,UINT PassCntTotal,UINT FailCntTotal);
	
	void ClientSetJobResultExt_Json(CString strDevSN, CString strCmd,UINT AdpCnt, UINT AdpStatus,UINT PassCntTotal,UINT FailCntTotal);
	void ClientSetMissionResult(CString strJson);
	void ClientSetPrintContent(INT SiteIdx, INT SktIdx, BOOL bSwap, CString Manufactor, CString Eid, CString Sn, INT Result);
	///设置客户端请求服务器执行InitSiteEnv命令的结果
	void ClientSetInitResult(CSerial&lSerial);///lSerial的参数布局参看二次开发文档

	///设置客户端请求服务器执行LoadProject命令的结果，strDevSN固定为ALL,Result为结果
	void ClientSetLoadResult(CString strDevSN,INT Result);
	
	///客户端请求服务器执行SetChecksumExt_Json命令用来发送扩展校验值信息
	void ClientSetChecksumExt_Json(CString strChecksumExt_Json);

	///客户端请求服务器执行FileImportInfo_Json命令用来发送加载档案文件信息
	void ClientFileImportInfo_Json(CString strChecksumExt_Json);

	///客户端请求服务器执行eMMCOption_Json命令用来发送emmc母片制作时的档案信息
	void ClienteMMCOption_Json(CString streMMCOption_Json);

	void ConstructAdapterInfo_Json(CSerial& lSerial, std::map<CString,SKBINFO> mapSkbInfo);
	
	void ClientSetFw_Result(CString strJson);
	void ClientSetJobStart(CString strDevSN, CString strCmd,UINT AdpCnt);
	/************
	@brief 请求客户端执行自定义命令
	@param[in] strDevSN:为ALL表示不指定站点，其他值表示指定站点
	@param[in] TagID:指定相互约定的Tag
	@param[in] pData:执行命令时带的参数
	@param[in] DataSize:参数的大小
	@param[out] ppRespData:返回数据所在的内存区域，需要用FreeData释放
	@param[out] pRespDataSize:返回数据大小
	return:
		成功返回0，失败返回非0
	************************/
	INT ClientDoCustom(CString strDevSN,UINT TagID,BYTE*pData,UINT DataSize,BYTE**pRespData=NULL,INT*pRespDataSize=NULL);
	
	///设置客户端请求服务器执行CreateProject命令的结果，strDevSN固定为ALL,Result为结果
	void ClientSetCreateProjResult(CString strDevSN,INT Result);
	void ClientSetCreateProjResultJson(CString strResultJson);
	CString GetCreateProjectResultJson();

	
	static void FreeData(BYTE*pData){if(pData){delete[] pData;}};
	

	static CString GetCmdName(INT DevCmd);///命令转字符串
	INT DoJobInWorkThread(CString DevSN,INT DevCmd);///将Job命令的执行放到线程中
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

	///////设置保存烧录总数的文件并发送至自动化软件
	INT n_PassCntTotal,n_FailCntTotal;
	CString strIniFilePath;
	INT SetChipSNPFile(INT PassCnt,INT FailCnt);
	CString SetIniFileFile();

	tServerCfg m_ServerCfg;///ServerCfg
	ClientPrx m_ClientApp;
	tDemoCfg m_DemoCfg;
	
	INT SiteDoCustom(const CString&strDevSN,UINT TagID,UINT TagDataSize,BYTE*pTagData);
	//////////////////////////////////////////////////////////////////////////
	///界面显示
	BOOL ShowSiteCtrlWindow(INT DevCmd);
	BOOL CloseSiteCtrlWindow();
	INT PrepareStatisticDlgSite(DEVINFO*pDevInfo,INT DevCmd);
	INT UpdateStatisticDlgSite(DEVINFO*pDevInfo,INT DevCmd,INT Result);

	int WaitAdpNotifyConfirm();

protected:
	///服务器允许执行的命令
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


	///为MES准备的函数
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
	CRITICAL_SECTION m_csCmdProtct; ///命令执行保护，避免多线程一起
	CWorkThread m_wtServerApp;
	CString m_strCurProject;
	CSerial m_lCreateProjInfo; ///存放工程文件创建时的函数参数

	CRITICAL_SECTION m_csSiteDoJob;
	DEVINFO *m_pDevInfo;
};


CIceServerApp& GetGlobalServerApp();

#endif 
