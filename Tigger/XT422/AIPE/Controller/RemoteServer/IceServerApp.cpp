
#pragma warning (disable : 4996)
#include "IceServerApp.h"

#include "../ComStruct/ErrTypes.h"
#include "../ComStruct/Serial.h"
#include "../ComStruct/ProjInfo.h"
#include "../ComStruct/AprogDevMng.h"
#include "../MultiAprog/resource.h"
#include "../MultiAprog/DlgQuCtl.h"
#include "../MultiAprog/DlgServerDemo.h"

#include "../ComStruct/ComTool.h"
#include "../ComStruct/Reporter.h"
#include "json/json.h"
#include "../ComStruct/ReporterFile.h"
#include "../MultiAprog/DlgSite.h"
#include "MTMemsAccess.h"
#include "shlwapi.h"
#include "../ComStruct/eMMCMCardMake.h"
#include "DlgPasswdCheck.h"

#pragma comment(lib,"shlwapi.lib")
CIceServerApp gIceServerApp;

#define SERVER_VERSION (36)	///35表示3.5版本
#define MAX_PATH	260
#define MAX_SNSIZE	(128 + 4*1024*1024)//128byte标头，8个座子4M数据

INT WINAPI ServerAppThreadProc(MSG msg,void *Para);

CIceServerApp& GetGlobalServerApp()
{
	return gIceServerApp;
}

RemoteAprog::tagRESP CIceServerApp::ServerDoCmd(const ::std::string&StrCmd, const ::RemoteAprog::tagMSG&Msg, const ::Ice::Current&)
{
	INT Ret;
	INT i,vSize;
	RemoteAprog::tagRESP Resp;
	FnCmdHandler pFn=NULL;
	Resp.iRespDataLen=0;
	vSize=(INT)vCmdHandler.size();
	if(StrCmd!="QueryStatus")
		CLogMsg::PrintDebugString("ServerDoCmd=%s\n",StrCmd.c_str());
	for(i=0;i<vSize;i++){
		if(vCmdHandler[i].strCmd==StrCmd){
			pFn=vCmdHandler[i].pCmdHandler;
		}
	}

	if(pFn){
		Ret=(this->*pFn)(Msg,Resp);
		Resp.iCmd=Ret;
	}
	else{
		ClientOutput("ALL",LOGLEVEL_ERR,"Server Not Support Command=\"%s\" yet",StrCmd.c_str());
		Resp.iCmd=-1;
	}
	return Resp;
}


void CIceServerApp::InitCmdHandlers()
{
	tCmdHandler CmdHandler;
	CmdHandler.strCmd="SiteScanAndConnect";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleSiteScanAndConnect;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="SiteDisconnect";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleSiteDisconnect;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="LoadProject";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleLoadPorject;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="GetProjectInfo";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleGetProjectInfo;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="InitSiteEnv";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleInitSiteEnv;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="SetAdapterEn";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleSetAdapterEn;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="GetAdapterEn";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleGetAdapterEn;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="EnableSN";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleEnableSN;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="SetSN";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleSetSN;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="DoJob";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleDoJob;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="DoCustom";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleDoCustom;
	vCmdHandler.push_back(CmdHandler);
	
	CmdHandler.strCmd = "DoCustomInThread";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleDoCustomInThread;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="CreateProject";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleCreateProject;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="CreateProjectByTemplate";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleCreateProjectByTemplate;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="SetProjectParamter";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleSetProjectParameter;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="GetProjectInfoExt";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleGetProjectInfoExt;
	vCmdHandler.push_back(CmdHandler);


	CmdHandler.strCmd="QueryStatus";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleQueryStatus;
	vCmdHandler.push_back(CmdHandler);


	CmdHandler.strCmd="GetSktCounter";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleGetSktCounter;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="GetAdapterInfo";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleGetAdapterInfo;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="GetSktInfoExt";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleGetSktCounterExt;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="GetSktInfoJson";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleGetSktInfoJson;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="GetErrCode";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleGetErrCode;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="GetReportJson";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleGetReportJson;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="SetSNStrategy";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleSetSNStrategy;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="GetVersion";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleGetVersion;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="SetChipVID";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleSetChipVID;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="SetLabel";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleSetLabel;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="GetSNCountJson";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleGetSNCount;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="GetStatisticCnt";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleGetStatisticCnt;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="EnableCheckID";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleEnableCheckID;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="SetPrintResult";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleSetPrintResult;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="AllResetcounter";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleAllResetcounter;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="Resetcounter";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleResetcounter;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="SendChemID";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleSendChemID;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="ConnectAutomatic";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleConnectAutomatic;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="DisConnectAutomatic";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleDisConnectAutomatic;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="ShowCmdSite";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleShowCmdSite;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="GetConfig_Json";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleGetConfig_Json;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="ReadBuffer";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleReadBuffer;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="ReadProjBuffer";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleReadProjBuffer;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="ReadChipAllBuffer";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleReadChipAllBuffer;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="ReadProjAllBuffer";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleReadProjAllBuffer;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="GetConfig_String";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleGetChipConfig_String;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="GetSpecialBit";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleGetSpecialBit;
	vCmdHandler.push_back(CmdHandler);

	CmdHandler.strCmd="NewOlderIDBurn";
	CmdHandler.pCmdHandler=&CIceServerApp::HandleNewOlderIDBurn;
	vCmdHandler.push_back(CmdHandler);
}

INT CIceServerApp::HandleGetSpecialBit(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp)
{

	INT Ret=ACERR_OK;
	BYTE *pDataResp;
	CSerial lSerial(m_ServerCfg.CharSet);
	CProjInfo& GProjInfo=GetGlobalProjInfo();

	Ret=GProjInfo.GetSpcBuf(lSerial);
	if(Ret==TRUE){
		Resp.iRespDataLen=lSerial.GetLength();
		pDataResp=lSerial.GetBuffer();
		if(Resp.iRespDataLen){
			std::copy(pDataResp,pDataResp+Resp.iRespDataLen,std::back_inserter(Resp.RespData));
		}
	}
	else{
		OutputDebugString("====================HandleGetSpecialBit=================>>>>");
		ClientOutput("ALL",LOGLEVEL_ERR,"HandleGetSpecialBit Execute Failed, Ret=%d",Ret);
	}
 
	return Ret;
}

CIceServerApp::CIceServerApp()
:m_strCurProject("")
,n_PassCntTotal(0)
,strIniFilePath("")
,n_FailCntTotal(0)
,m_ClientApp(NULL)
{
	memset(&m_ServerCfg,0,sizeof(tServerCfg));
	InitCmdHandlers();
	InitializeCriticalSection(&m_csClient);
	InitializeCriticalSection(&m_csCmdProtct);
	strIniFilePath = SetIniFileFile();
	n_PassCntTotal = GetPrivateProfileInt("Setting", "PassCnt", 0, strIniFilePath);
	n_FailCntTotal = GetPrivateProfileInt("Setting", "FailCnt", 0, strIniFilePath);

	InitializeCriticalSection(&m_csSiteDoJob);
	
}

CIceServerApp::~CIceServerApp()
{
	if(m_DemoCfg.pDlgServerDemo){
		delete m_DemoCfg.pDlgServerDemo;
	}
	m_DemoCfg.ReInit();
	vCmdHandler.clear();
	DeleteCriticalSection(&m_csClient);
	DeleteCriticalSection(&m_csCmdProtct);

	DeleteCriticalSection(&m_csSiteDoJob);
}

CString  CIceServerApp::SetIniFileFile()
{
	CString appPath = _T("");
	CDevMng& GDevMng=GetGlobalDevMng();
	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, appPath.GetBuffer(MAX_PATH));
	appPath.ReleaseBuffer();
	appPath.Format("%s\\MultiAprog",appPath);
	if(!PathIsDirectory(appPath))
	{
		if(!CreateDirectory(appPath,0))   {  
			return "";   
		}
	}
	CString IniFilePath = _T("");
	IniFilePath.Format("%s\\Setting.ini",appPath);
	return IniFilePath;
}



INT CIceServerApp::SetChipSNPFile(INT PassCnt,INT FailCnt)
{
	INT Ret = -1;
	CString strTmp;
	INT n_TotalNum;

	n_TotalNum = PassCnt + FailCnt;
	strTmp.Format("%d", PassCnt);
	WritePrivateProfileString("Setting", "PassCnt", strTmp, strIniFilePath);

	strTmp.Format("%d", FailCnt);
	WritePrivateProfileString("Setting", "FailCnt", strTmp, strIniFilePath);

	strTmp.Format("%d", n_TotalNum);
	WritePrivateProfileString("Setting", "TotalNum", strTmp, strIniFilePath);

	return Ret;
}

INT CIceServerApp::SetAdpStatusDemo(const CString&strDevSN,UINT AdpCnt,const char *Status)
{
	UINT TagID=TAG_SETSTATUS;
	INT i;
	CHAR strStatus[8][64];
	CHAR TmpBuf[256];
	CSerial lSerial;
	DWORD SizeUsed;
	CString tmpStatus;
	BYTE Cnt=0;
	memset(strStatus,0,8*64);
	if(AdpCnt>8){
		AdpCnt=8;
	}
	Cnt=(BYTE)AdpCnt;
	lSerial<<Cnt;
	for(i=0;i<(INT)AdpCnt;++i){
		if(m_ServerCfg.CharSet==CHARSET_UTF8){
			ComTool::MByteToUtf8(Status,TmpBuf,256,SizeUsed);
			if(SizeUsed<=64){
				memcpy(strStatus[i],TmpBuf,SizeUsed);
			}
			else{
				memcpy(strStatus[i],TmpBuf,63);
			}
		}
		else if(m_ServerCfg.CharSet==CHARSET_UNICODE){
			ComTool::MByteToWChar(Status,(LPWSTR)TmpBuf,256/2,SizeUsed);
			if(SizeUsed<=64/2){
				memcpy(strStatus[i],TmpBuf,SizeUsed*2);
			}
			else{
				memcpy(strStatus[i],TmpBuf,62);
			}
		}
		else{
			strncpy(strStatus[i],Status,64);
		}
	}
	lSerial.SerialInBuff((BYTE*)strStatus,AdpCnt*64);
	return ClientDoCustom(strDevSN,TagID,lSerial.GetBuffer(),lSerial.GetLength(),NULL,NULL);
}

INT WINAPI DevDoJobThreadProc(MSG msg,void *Para)
{
	INT Ret=ACERR_OK;
	CIceServerApp& GIceServerApp=GetGlobalServerApp();
	if(msg.message==TMSG_ICEDOJOB){
		DEVINFO *pDevInfo=(DEVINFO*)Para;
		if(!pDevInfo){
			GIceServerApp.ClientOutput("ALL",LOGLEVEL_ERR,"Do Job Thread Normal: Parameter Invalid");
			goto __end;
		}

		INT DevCmd=(INT)msg.wParam;
		UINT AdpCnt=0,AdpStatus=0;
		UINT PassCntTotal=0,FailCntTotal=0;
		if(pDevInfo->IsProjEnvSet==FALSE){
			CDevMng& GDevMng=GetGlobalDevMng();
			CString strCmd=CIceServerApp::GetCmdName(DevCmd);
			GIceServerApp.ClientOutput("ALL",LOGLEVEL_ERR,"%s[%s] Init Failed, Can't Be Controlled To Do The Command",pDevInfo->DevSN,pDevInfo->DevAlias);
			GDevMng.PrintLog(LOGLEVEL_ERR,"%s[%s] Init Failed, Can't Be Controlled To Do The Command",pDevInfo->DevSN,pDevInfo->DevAlias);
			GIceServerApp.ClientOutput("ALL",LOGLEVEL_ERR,"Return ALL Error Directly");
			GDevMng.PrintLog(LOGLEVEL_ERR,"Return ALL Error Directly");
			Ret=pDevInfo->pAprogDev->GetVitrualResultForRmtServer(DevCmd,AdpCnt,AdpStatus,CAprogDev::STATUS_ERR,PassCntTotal,FailCntTotal);
			GIceServerApp.ClientSetJobResult(pDevInfo->DevSN,strCmd,AdpCnt,AdpStatus);//发送最终的结果
			GIceServerApp.ClientSetJobResultExt(pDevInfo->DevSN,strCmd,AdpCnt,AdpStatus,PassCntTotal,FailCntTotal);//发送最终的结果带上该站点的总数
			GIceServerApp.ClientSetJobResultExt_Json(pDevInfo->DevSN,strCmd,AdpCnt,AdpStatus,PassCntTotal,FailCntTotal);
			goto __end;
		}

		if(DevCmd==CMD_DevInsSen){
			if(pDevInfo->pAprogDev){
				INT DoCmdRtn=0;
				CDevMng& GDevMng=GetGlobalDevMng();
				CString strCmd=CIceServerApp::GetCmdName(DevCmd);
				//GDevMng.PrintLog(LOGLEVEL_LOG,"Call DevCmd=%d, InsetionCheck",DevCmd);
				pDevInfo->DevRunTime.CurCmd=CMD_DevInsSen; 
				DoCmdRtn=pDevInfo->pAprogDev->DoCmd(DevCmd);
				if(DoCmdRtn==ACERR_OK){
					GIceServerApp.ClientOutput(pDevInfo->DevSN,LOGLEVEL_LOG,"%s pass",strCmd);
				}
				else{
					GIceServerApp.ClientOutput(pDevInfo->DevSN,LOGLEVEL_ERR,"%s failed",strCmd);
				}
				//GDevMng.PrintLog(LOGLEVEL_WARNING,"有烧录过的进入这个Function");
				Ret=pDevInfo->pAprogDev->GetDoCmdResultForRmtServer(DevCmd,AdpCnt,AdpStatus,PassCntTotal,FailCntTotal);
				GIceServerApp.ClientSetJobResult(pDevInfo->DevSN,strCmd,AdpCnt,AdpStatus);//发送最终的结果
				GIceServerApp.ClientSetJobResultExt(pDevInfo->DevSN,strCmd,AdpCnt,AdpStatus,PassCntTotal,FailCntTotal);//发送最终的结果带上该站点的总数
				GIceServerApp.ClientSetJobResultExt_Json(pDevInfo->DevSN,strCmd,AdpCnt,AdpStatus,PassCntTotal,FailCntTotal);
				//GDevMng.PrintLog(LOGLEVEL_ERR,"n_PassCntTotal = %d, n_FailCntTotal = %d",
				//GIceServerApp.n_PassCntTotal,GIceServerApp.n_FailCntTotal);
			}
		}
		else{
			CString strCmd=CIceServerApp::GetCmdName(DevCmd);
			CDevMng& GDevMng=GetGlobalDevMng();
			GDevMng.PrintLog(LOGLEVEL_LOG, "GIceServerApp Start ShowSiteCtrlWindow...");
			GIceServerApp.ShowSiteCtrlWindow(DevCmd);
			if(pDevInfo->pAprogDev){
				INT DoCmdRtn=0;
				GIceServerApp.PrepareStatisticDlgSite(pDevInfo,DevCmd);
				if(GDevMng.m_GlbSetting.dwReportAutoSave){///保存到Report
					CReporter&GReporter=GetReporter();
					///这个地方每次都要去保存，因为可能会执行ClearStastic动作
					GReporter.SetBtmBoardInfo(pDevInfo->DevSN,pDevInfo->BtmBoardInfo);
					//if(GDevMng.m_GlbSetting.dwAdpInfoAutoSave)
					//pDevInfo->pAprogDev->DoCustom(TAG_GETALLSKBINFO,NULL,0);

					if(GDevMng.m_GlbSetting.dwEnableGetSKTInfoBeginBurn == 1){
						if(pDevInfo->pAprogDev->IsLargerThanVersion(VERSION_0_75)){
							pDevInfo->pAprogDev->DoCustom(TAG_GET_ALLSKBSMPINFO_EXT,NULL,0);
						}
						else{
							pDevInfo->pAprogDev->DoCustom(TAG_GETALLSKBINFO,NULL,0);
						}
						//pDevInfo->pAprogDev->DoCustom(TAG_GETALLSKBUID,NULL,0);
					}
					pDevInfo->pAprogDev->DoCustom(TAG_GETALLSKBUID,NULL,0);

				}
				GIceServerApp.ClientOutput(pDevInfo->DevSN,LOGLEVEL_LOG,"WaitAdpNotifyConfirm,Cmd=%s",strCmd);
				if(GIceServerApp.WaitAdpNotifyConfirm()==1){
					///这个地方一定要先退出线程，否则后续AP  P关闭的时候会因为等待线程结束而死机
					///退出APP 
					CLogMsg::PrintDebugString("GIceServerApp.WaitAdpNotifyConfirm()---APP");
					goto __end;
				}

				if(CDlgSite::SetMultiSNForRemoteCtrl(pDevInfo, DevCmd) != ACERR_OK){
					Ret=pDevInfo->pAprogDev->GetVitrualResultForRmtServer(DevCmd,AdpCnt,AdpStatus,CAprogDev::STATUS_ERR,PassCntTotal,FailCntTotal);///认为全部出错
					//GIceServerApp.ClientSetJobResult(pDevInfo->DevSN,strCmd,AdpCnt,AdpStatus);//发送最终的结果
					GDevMng.PrintLog(LOGLEVEL_ERR,"SetMultiSNForRemoteCtrl Failed, Set All SKT Error");
					DoCmdRtn=ACERR_FAIL;
					goto __UpdateCnt;
				}

				GIceServerApp.ClientSetJobStart(pDevInfo->DevSN,strCmd,AdpCnt);

				pDevInfo->pAprogDev->GetSysLogData(DevCmd, true, 0);
				///这个地方为实际命令处理位置
				DoCmdRtn=pDevInfo->pAprogDev->DoCmd(DevCmd);
				if(DoCmdRtn==ACERR_OK){
					GIceServerApp.ClientOutput(pDevInfo->DevSN,LOGLEVEL_LOG,"%s pass",strCmd);
				}
				else{
					GIceServerApp.ClientOutput(pDevInfo->DevSN,LOGLEVEL_ERR,"%s failed",strCmd);
				}
				Ret=pDevInfo->pAprogDev->GetDoCmdResultForRmtServer(DevCmd,AdpCnt,AdpStatus,PassCntTotal,FailCntTotal);
				//pDevInfo->pAprogDev->GetSysLogData(DevCmd, false, DoCmdRtn);
				//GDevMng.PrintLog(LOGLEVEL_ERR,"n_PassCntTotal = %d, n_FailCntTotal = %d",GIceServerApp.n_PassCntTotal,GIceServerApp.n_FailCntTotal);
__UpdateCnt:	
				CDlgSite::UpdateSNCfgForRemoteCtrl(pDevInfo);
				GIceServerApp.UpdateStatisticDlgSite(pDevInfo,DevCmd,DoCmdRtn);
				GIceServerApp.ClientSetJobResult(pDevInfo->DevSN,strCmd,AdpCnt,AdpStatus);//发送最终的结果
				GIceServerApp.ClientSetJobResultExt(pDevInfo->DevSN,strCmd,AdpCnt,AdpStatus,PassCntTotal,FailCntTotal);//发送最终的结果
				GIceServerApp.ClientSetJobResultExt_Json(pDevInfo->DevSN,strCmd,AdpCnt,AdpStatus,PassCntTotal,FailCntTotal);
			}
		}
	}
	else if(msg.message==TMSG_ICEDOJOBDEMO){
		
		tVirtualSite *pVirtualSite=(tVirtualSite*)Para;
		INT msDelay=0,TotalDelay;
		if(!pVirtualSite){
			GIceServerApp.ClientOutput("ALL",LOGLEVEL_ERR,"Do Job Thread Demo: Parameter Invalid");
			goto __end;
		}
		INT DevCmd=(INT)msg.wParam;
		UINT AdpCnt=1,AdpStatus=0;
		CString strCmd=CIceServerApp::GetCmdName(DevCmd);
		GIceServerApp.ShowSiteCtrlWindow(DevCmd);
		GIceServerApp.ClientSetProgress(pVirtualSite->strSN,0,100);
		Ret=GIceServerApp.GetDemoUserResult(pVirtualSite,AdpCnt,AdpStatus);
		TotalDelay=GIceServerApp.GetDemoCmdExecDelayTime();
		GIceServerApp.SetAdpStatusDemo(pVirtualSite->strSN,AdpCnt,"Insertion Checking");
		if(Ret==0){
			GIceServerApp.ClientOutput(pVirtualSite->strSN,LOGLEVEL_LOG,"%s pass",strCmd);
		}
		else{
			GIceServerApp.ClientOutput(pVirtualSite->strSN,LOGLEVEL_ERR,"%s failed",strCmd);
		}
		while(msDelay<TotalDelay){
			Sleep(500);
			GIceServerApp.ClientSetProgress(pVirtualSite->strSN,msDelay,TotalDelay);
			GIceServerApp.SetAdpStatusDemo(pVirtualSite->strSN,AdpCnt,"Programming");
			msDelay +=500;
		}
		Sleep(TotalDelay-(msDelay-500));
		GIceServerApp.SetAdpStatusDemo(pVirtualSite->strSN,AdpCnt,"Pass");
		GIceServerApp.ClientOutput(pVirtualSite->strSN,LOGLEVEL_LOG,"Delay: %d ms",TotalDelay);
		GIceServerApp.ClientSetProgress(pVirtualSite->strSN,100,100);
		GIceServerApp.ClientSetJobResult(pVirtualSite->strSN,strCmd,AdpCnt,AdpStatus);//发送最终的结果
		GIceServerApp.ClientSetJobResultExt_Json(pVirtualSite->strSN,strCmd,AdpCnt,AdpStatus,100,200);
	}
	
__end:
	
	return -1; ///让线程自动退出
}

INT CIceServerApp::HandleGetReportJson( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK;
	std::string strReport;
	Ret=CReporterFile::GetReportJson(strReport);	
	Resp.iRespDataLen=strReport.length()+1;
	if(Resp.iRespDataLen){
		std::copy(strReport.c_str(),strReport.c_str()+Resp.iRespDataLen,std::back_inserter(Resp.RespData));
	}
	return Ret;
}

INT CIceServerApp::HandleGetVersion(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp)
{
	INT Ret=ACERR_OK;
	CSerial lSerial;
	UINT Version=SERVER_VERSION;
	lSerial<<Version;
	Resp.iRespDataLen=lSerial.GetLength();
	if(Resp.iRespDataLen){
		std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+Resp.iRespDataLen,std::back_inserter(Resp.RespData));
	}
	return Ret;
}

INT CIceServerApp::HandleSiteScanAndConnect( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK;
	CSerial lSerial(m_ServerCfg.CharSet);
	Json::StyledWriter JWriter;
	Json::Value Root;
	CString strOutJson;

	CDevMng& GDevMng=GetGlobalDevMng();
	INT i,SiteCnt=0;
	BYTE *TmpBuf=NULL;
	LONG RespCode=0;
	if(m_ServerCfg.DemoEn==0){
		if(Msg.iCmdDataLen==0){
			Ret=GDevMng.AutoConnectWhenStart();
			if(Ret!=ACERR_OK){
				goto __end;
			}
		}
		else{
			std::vector<CString> vSiteFix;
			CSerial lSerialTmp(m_ServerCfg.CharSet);
			TmpBuf=new BYTE[Msg.iCmdDataLen];
			if(TmpBuf==NULL){
				Ret=ACERR_MEMALLOC;goto __end;
			}
			std::copy(Msg.CmdData.begin(),Msg.CmdData.end(),TmpBuf);
			lSerialTmp.SerialInBuff(TmpBuf,Msg.iCmdDataLen);
			while(lSerialTmp.GetLength()>0){
				CString strSiteAlias;
				lSerialTmp>>strSiteAlias;
				vSiteFix.push_back(strSiteAlias);
				CLogMsg::PrintDebugString("Query To Connect:%s\n",strSiteAlias);
			}
			if(vSiteFix.size()!=0){
				Ret=GDevMng.SiteScanAndConnectFixAlias(vSiteFix);
				if(Ret!=ACERR_OK){
					goto __end;
				}
			}
		}
		SiteCnt=GDevMng.GetDevCnt(CDevMng::GROUP_ATTACNED);
		for(i=0;i<SiteCnt;i++){
			DEVINFO * pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED,i);
			if(pDevInfo){
				lSerial<<pDevInfo->DevSN<<pDevInfo->DevType<<pDevInfo->DevAlias;
				
				Json::Value Item;
				Item["SiteSN"] = Json::Value(pDevInfo->DevSN);
				Item["Alias"] = Json::Value(pDevInfo->DevAlias);
				Item["FmVer"] = Json::Value(pDevInfo->DevVer.FMVer);
				Root["SiteList"].append(Item);
			}
		}

		std::string strJson=JWriter.write(Root);
		strOutJson.Format("%s", strJson.c_str());
		ClientSetFw_Result(strOutJson); 
	}
	else{
		if(Msg.iCmdDataLen==0){
			SiteCnt=m_DemoCfg.SiteNum;
			for(i=0;i<SiteCnt;i++){
				lSerial<<m_DemoCfg.vpVirtualSites[i]->strSN<<m_DemoCfg.strDevName<<m_DemoCfg.vpVirtualSites[i]->strAlias;
			}
		}
		else{
			INT j=0;
			CSerial lSerialTmp;
			std::map<CString,INT> vSiteFix;
			Ret=SerialCmdData(Msg,lSerialTmp);
			while(lSerialTmp.GetLength()>0){
				CString strSiteAlias;
				lSerialTmp>>strSiteAlias;
				vSiteFix.insert(std::pair<CString,int>(strSiteAlias,j));
				j++;
				CLogMsg::PrintDebugString("Query To Connect:%s\n",strSiteAlias);
			}
			for(i=0;i<m_DemoCfg.SiteNum;i++){
				std::map<CString,int>::iterator it;
				it=vSiteFix.find(m_DemoCfg.vpVirtualSites[i]->strAlias);
				if(it!=vSiteFix.end()){
					SiteCnt++;
					lSerial<<m_DemoCfg.vpVirtualSites[i]->strSN<<m_DemoCfg.strDevName<<m_DemoCfg.vpVirtualSites[i]->strAlias;
				}
			}
		}
	}
	
	
	Resp.iRespDataLen=lSerial.GetLength();
	if(Resp.iRespDataLen){
		std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+Resp.iRespDataLen,std::back_inserter(Resp.RespData));
	}
	Ret=SiteCnt;
	//ClientOutput("ALL",LOGLEVEL_LOG,"ScanSite=%d,%s",Msg.iCmdDataLen,TmpBuf);
__end:
	if(TmpBuf){
		delete[] TmpBuf;
	}
	return Ret;
}


void CIceServerApp::ClientSetFw_Result(CString strJson)
{
	CSerial lSerial(m_ServerCfg.CharSet);
	RemoteAprog::tagMSG ClientMsg;
	lSerial<<strJson;
	ClientMsg.iCmd=0;
	ClientMsg.iCmdDataLen = lSerial.GetLength();
	std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+ClientMsg.iCmdDataLen,std::back_inserter(ClientMsg.CmdData));
	EnterCriticalSection(&m_csClient);
	try{
		m_ClientApp->ClientDoCmd("SetFwResult_Json",ClientMsg);
	}
	catch (...){
	}

	LeaveCriticalSection(&m_csClient);
	
}

INT CIceServerApp::HandleSiteDisconnect( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{

	INT Ret=ACERR_OK;
	BYTE *TmpBuf=NULL;
	CSerial lSerial(m_ServerCfg.CharSet);
	CDevMng& GDevMng=GetGlobalDevMng();
	INT i;
	BYTE SiteCnt;
	CString DevSN;
	LONG RespCode=0;
	if(Msg.iCmdDataLen==0){
		Ret=ACERR_PARA;
		goto __end;
	}
	TmpBuf=new BYTE[Msg.iCmdDataLen];
	if(TmpBuf==NULL){
		Ret=ACERR_MEMALLOC;goto __end;
	}
	CloseSiteCtrlWindow();
	std::copy(Msg.CmdData.begin(),Msg.CmdData.end(),TmpBuf);
	lSerial.SerialInBuff(TmpBuf,Msg.iCmdDataLen);
	lSerial>>SiteCnt;
	for(i=0;i<(INT)SiteCnt;++i){
		CString strDevSN;
		lSerial>>strDevSN;
		if(GDevMng.ReleaseDev(CDevMng::GROUP_ATTACNED,strDevSN)==FALSE){
			ClientOutput("ALL",LOGLEVEL_ERR,"Can't find DevSN=%s in the connected queue",strDevSN);
			Ret=ACERR_FAIL;
		}
	}

__end:
	SAFEDELARRAY(TmpBuf);
	return Ret;
}

INT CIceServerApp::LoadProjectInWorkThread(CString strProject)
{
	INT Ret=ACERR_OK;
	m_strCurProject=strProject;
	m_wtServerApp.SetMsgHandler(ServerAppThreadProc,this);
	if(m_wtServerApp.CreateThread()==FALSE){
		ClientOutput("ALL",LOGLEVEL_ERR,"Work thread for LoadProject create failed");
		ClientOutput("ALL",LOGLEVEL_ERR,"%s",m_wtServerApp.GetErrMsg());
		Ret=ACERR_FAIL;
	}
	else{
		m_wtServerApp.PostMsg(TMSG_ICELOADPROJECT,0,0);
	}
	return Ret;
}



INT CIceServerApp::ServerDoLoadProject()
{
	INT Ret=ACERR_OK;
	CProjInfo& GProjInfo=GetGlobalProjInfo();
	CDevMng& GDevMng=GetGlobalDevMng();
	if(m_strCurProject.Mid(0,3)=="ftp"){
		Ret=GProjInfo.OpenProjectFromFtpServer(m_strCurProject);
		//Ret=ACERR_FAIL;
		//ClientOutput("ALL",LOGLEVEL_ERR,"LoadProject form ftp server not support yet!");
	}
	else{
		CloseSiteCtrlWindow();///重新加载工程之后需要先关闭控制对话框
		GDevMng.ResetAllSiteAttachedStaticsForRemote();
		//Server模式，在普通加载工程时添加密码输入框
		CDlgPasswdCheck DlgPasswd("Verify Project Password");
		GProjInfo.AttachPasswdInputer(&DlgPasswd);
		Ret=GProjInfo.OpenProject(m_strCurProject);
		GProjInfo.DetachPasswdInputer();
	}

	if(GDevMng.m_GlbSetting.OCXInfo.WindowEn){
		if(m_ServerCfg.pMainWnd){
			::SendMessage(m_ServerCfg.pMainWnd->m_hWnd,TMSG_SHOWPROJINFO,0,0);
		}
	}

	if (!GProjInfo.IsInsertCheckEn() && !GProjInfo.IsAutoSensorEn()) {
		GDevMng.PrintLog(LOGLEVEL_LOG, "Send cmd4 to auto disable InsertCheck...");
		char res[8] = { 0 };
		if (SendCmd4ToAuto(3000, "4,{\"Method\":\"SetSysCounts\",\"Data\":{\"RemoveCheckCnt\":\"-1\",\"ContactCheckCnt\":\"-1\",\"Persisit\":false}}", res, 8) && res[0] != '0')
			GDevMng.PrintLog(LOGLEVEL_ERR, "Auto-machine disable InsertCheck failed! error code: %s", res);
	}
	
	ClientSetLoadResult("ALL",Ret);
	//ClientSetMissionResult("");
	return Ret;
}


INT CIceServerApp::CreateProjectInWorkThread()
{
	INT Ret=ACERR_OK;
	m_wtServerApp.SetMsgHandler(ServerAppThreadProc,this);
	if(m_wtServerApp.CreateThread()==FALSE){
		ClientOutput("ALL",LOGLEVEL_ERR,"Work thread for CreateProject create failed");
		ClientOutput("ALL",LOGLEVEL_ERR,"%s",m_wtServerApp.GetErrMsg());
		Ret=ACERR_FAIL;
	}
	else{
		m_wtServerApp.PostMsg(TMSG_ICECREATEPROJECT,0,0);
	}
	return Ret;
}

INT CIceServerApp::ServerDoCreateProjectByTemplate()
{
	INT Ret=ACERR_OK;
	CProjInfo& GProjInfo=GetGlobalProjInfo();
	if(m_lCreateProjInfo.GetLength()!=0){///调用之前需要先填充好
		CString strProjPath,ChipInfo;
		CSerial lSerial;
		try{
			CString strJsonPara;
			m_lCreateProjInfo>>strProjPath>>strJsonPara;
			Ret=GProjInfo.CreateProjectByTemplate(strProjPath,strJsonPara);
			ClientSetCreateProjResultJson(GetCreateProjectResultJson());
		}
		catch (...){
			Ret=ACERR_PARA;	
			goto __end;
		}

	}
	else{
		Ret=ACERR_PARA;
	}

__end:
	m_lCreateProjInfo.ReInit();
	ClientSetCreateProjResult("ALL",Ret);
	return Ret;	
}


INT CIceServerApp::ServerDoCreateProject()
{
	INT Ret=ACERR_OK;
	
	CProjInfo& GProjInfo=GetGlobalProjInfo();
	if(m_lCreateProjInfo.GetLength()!=0){///调用之前需要先填充好
		CString strProjPath,ChipInfo;
		CSerial lSerial;
		try{
			tCreateProjPara ProjPara;
			BOOL bEMMC=FALSE;
			m_lCreateProjInfo>>strProjPath>>ChipInfo>>ProjPara.strTargetFile;
			m_lCreateProjInfo>>ProjPara.nQuantity>>ProjPara.nFuncMode;
			if(ComTool::ExtractParaSet(ChipInfo,lSerial)==FALSE){
				Ret=ACERR_PARA; goto __end;
			}
			lSerial>>ProjPara.strChipName>>ProjPara.strAdapterName;
			Ret=GProjInfo.CheckChipTypeIsEMMC(ProjPara,bEMMC);
			if(Ret==0 && bEMMC==TRUE){
				//Ret=MakeMCardAndCreateProject(strProjPath,ProjPara);
				CString ResultJson;
				Ret=MakeMCardAndCreateProjectinThreads(strProjPath,ProjPara, ResultJson);
				ClientSetCreateProjResultJson(ResultJson);
			}
			else{
				Ret=GProjInfo.CreateProject(strProjPath,ProjPara);
				ClientSetCreateProjResultJson(GetCreateProjectResultJson());
			}
			
		}
		catch (...){
			Ret=ACERR_PARA;	
			goto __end;
		}
		
	}
	else{
		Ret=ACERR_PARA;
	}
	
__end:
	m_lCreateProjInfo.ReInit();
	ClientSetCreateProjResult("ALL",Ret);
	return Ret;	
}


CString CIceServerApp::GetCreateProjectResultJson(){
	Json::StyledWriter JWriter;
	Json::Value Root;
	Json::Value ResultNode;
	CString Ret = _T("");
	ResultNode["TotalCnt"]=Json::Value(0);
	ResultNode["PassCnt"]=Json::Value(0);
	ResultNode["FailCnt"]=Json::Value(0);
	Root["Result"] = ResultNode;
	Root["IsEMMC"] = Json::Value(0);
	Root["SiteInfo"] = Json::arrayValue;

	std::string strJson=JWriter.write(Root);
	Ret.Format("%s", strJson.c_str());
	return Ret;
}

INT CIceServerApp::LoadDemoCfg()
{
	CString strMsg;
	INT Ret=0,i,j;
	CHAR TmpBuf[MAX_PATH];
	CString strIniFile=CComFunc::GetCurrentPath()+"/data/DemoCfg.ini";
	memset(TmpBuf,0,MAX_PATH);
	GetPrivateProfileString("Demo", "DevType","A80U",TmpBuf, MAX_PATH, strIniFile);
	if(TmpBuf[0]!=0){
		m_DemoCfg.strDevType.Format("%s",TmpBuf);	
	}

	memset(TmpBuf,0,MAX_PATH);
	GetPrivateProfileString("Demo", "DevName","AP8000",TmpBuf, MAX_PATH, strIniFile);
	if(TmpBuf[0]!=0){
		m_DemoCfg.strDevName.Format("%s",TmpBuf);	
	}

	m_DemoCfg.SiteNum=GetPrivateProfileInt("Demo","SiteNum",1,strIniFile);
	m_DemoCfg.SocketNumPerSite=GetPrivateProfileInt("Demo","SocketNumPerSite",1,strIniFile);
	m_DemoCfg.Delay=GetPrivateProfileInt("Demo","Delay",200,strIniFile);

	memset(TmpBuf,0,MAX_PATH);
	GetPrivateProfileString("Demo", "ErrCode","0x20AE",TmpBuf, MAX_PATH, strIniFile);
	if(TmpBuf[0]!=0){
		m_DemoCfg.strErrCode.Format("%s",TmpBuf);	
	} 

	if(m_DemoCfg.SiteNum>16){
		//strMsg.Format("DemoCfg.ini文件中的SiteNum不能大于16");
		strMsg.Format("DemoCfg.ini SiteNum in the file cannot be greater than 16");
		AfxMessageBox(strMsg,MB_OK|MB_ICONSTOP);
		Ret=-1;
	}

	if(m_DemoCfg.SocketNumPerSite>8){
		//strMsg.Format("DemoCfg.ini文件中的SocketNumPerSite不能大于8");
		strMsg.Format("DemoCfg.ini SocketNumPerSite in the file cannot be greater than 8");
		AfxMessageBox(strMsg,MB_OK|MB_ICONSTOP);
		Ret=-1;
	}

	for(i=0;i<m_DemoCfg.SiteNum;i++){
		tVirtualSite *pVirtualSize=new tVirtualSite;
		if(!pVirtualSize){
			//strMsg.Format("初始化虚拟站点信息出错");
			strMsg.Format("Error occurred in virtual site information initiation");
			AfxMessageBox(strMsg,MB_OK|MB_ICONSTOP);
			Ret=-1;
			break;
		}
		pVirtualSize->strSN.Format("%s150319000%02d",m_DemoCfg.strDevType,i+1);
		pVirtualSize->strAlias.Format("Site%02d",i+1);
		pVirtualSize->pDlgDemoSite=NULL;
		for(j=0;j<m_DemoCfg.SocketNumPerSite;j++)
			pVirtualSize->SktStatus[j]=DEMOSTATUS_PASS;
		for(;j<8;j++){
			pVirtualSize->SktStatus[j]=DEMOSTATUS_UNUSED;
		}
		m_DemoCfg.vpVirtualSites.push_back(pVirtualSize);
	}
	return Ret;
}

INT CIceServerApp::CreateDemoDiallog()
{
	INT Ret=0;
	if(m_ServerCfg.DemoEn){
		CDlgServerDemo *pServerDemo=new CDlgServerDemo(&m_DemoCfg);
		
		if(pServerDemo==NULL){
			CLogMsg::PrintDebugString("CIceServerApp: Create Demo Dialog failed\n");
			Ret=-1; 
			goto __end;
		}
		pServerDemo->Create(CDlgServerDemo::IDD,NULL);
		pServerDemo->SetWindowPos(CWnd::FromHandle(HWND_TOP),0,0,0,0,SWP_NOSIZE|SWP_SHOWWINDOW);
		pServerDemo->CenterWindow();
		m_DemoCfg.pDlgServerDemo=dynamic_cast<CDialog*>(pServerDemo);
	}

__end:
	return Ret;
}

INT CIceServerApp::HandleLoadPorject( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK;
	CString strProjPath;
	CSerial lSerialTmp(m_ServerCfg.CharSet);
	if(SerialCmdData(Msg,lSerialTmp)!=ACERR_OK){
		Ret=ACERR_PARA;
		goto __end;
	}
	lSerialTmp>>strProjPath;
	Ret=LoadProjectInWorkThread(strProjPath);

	//ClientOutput("ALL",LOGLEVEL_ERR,"LoadProject GetCmdLen=%d,%s",Msg.iCmdDataLen,TmpBuf);
__end:
	return Ret;
}

#include <afxext.h>
#include "../MultiAprog/MainFrm.h"
#include "../MultiAprog/DlgSite.h"
BOOL CIceServerApp::ShowSiteCtrlWindow(INT DevCmd)
{
	CDevMng& GDevMng=GetGlobalDevMng();
	if(GDevMng.m_GlbSetting.OCXInfo.WindowEn){
		EnterCriticalSection(&m_csSiteDoJob);
		if(m_ServerCfg.pMainWnd){
			::SendMessage(m_ServerCfg.pMainWnd->m_hWnd,TMSG_SHOWSITECTRLFORSERVER,DevCmd,0);
		}
		LeaveCriticalSection(&m_csSiteDoJob);
	}
	return TRUE;
}

int CIceServerApp::WaitAdpNotifyConfirm()
{
	BOOL bExit=0;
	CDevMng& GDevMng=GetGlobalDevMng();
	ClientOutput("ALL",LOGLEVEL_WARNING,"WaitAdpNotifyConfirm");
	if(GDevMng.m_GlbSetting.OCXInfo.WindowEn && GDevMng.m_GlbSetting.AdpNotify.bEn){
		ClientOutput("ALL",LOGLEVEL_WARNING,"WaitAdpNotifyConfirm=====1");
		while(1){///等待通知被確認
			//ClientOutput("ALL",LOGLEVEL_WARNING,"Waiting to close the adapter notification dialog");
			if(GDevMng.IsAdpNotifyPass(&bExit)==TRUE){
				ClientOutput("ALL",LOGLEVEL_WARNING,"GDevMng.IsAdpNotifyPass Exit=%d",bExit);
				break;
			}
			else{
				//ClientOutput("ALL",LOGLEVEL_WARNING,"Waiting to close the adapter notification dialog");
				Sleep(200);
			}
		}
		ClientOutput("ALL",LOGLEVEL_WARNING,"WaitAdpNotifyConfirm=====2");
	}
	return bExit;
}

BOOL CIceServerApp::CloseSiteCtrlWindow()
{
	CDevMng& GDevMng=GetGlobalDevMng();
	if(GDevMng.m_GlbSetting.OCXInfo.WindowEn){
		if(m_ServerCfg.pMainWnd){
			::SendMessage(m_ServerCfg.pMainWnd->m_hWnd,TMSG_CLOSESITECTRLFORSERVER,0,0);
		}
	}
	return TRUE;
}


INT CIceServerApp::PrepareStatisticDlgSite(DEVINFO*pDevInfo,INT DevCmd)
{
	CDevMng& GDevMng=GetGlobalDevMng();
	if(GDevMng.m_GlbSetting.OCXInfo.WindowEn){
		if(pDevInfo && pDevInfo->pDlgSite){
			CDlgSite *pDlgSite=dynamic_cast<CDlgSite*>(pDevInfo->pDlgSite);
			::SendMessage(pDlgSite->m_hWnd,TMSG_PREPARESTATISTICFORSERVER,DevCmd,0);
		}
	}
	return TRUE;
}

INT CIceServerApp::UpdateStatisticDlgSite(DEVINFO*pDevInfo,INT DevCmd,INT Result)
{
	CDevMng& GDevMng=GetGlobalDevMng();
	if(GDevMng.m_GlbSetting.OCXInfo.WindowEn){
		if(pDevInfo && pDevInfo->pDlgSite){
			CDlgSite *pDlgSite=dynamic_cast<CDlgSite*>(pDevInfo->pDlgSite);
			::SendMessage(pDlgSite->m_hWnd,TMSG_UPDATESTATISTICFORSERVER,DevCmd,Result);
		}
	}
	return TRUE;
}


INT CIceServerApp::ServerDoInitSiteEnv()
{
	INT SiteCnt=0,i,EnvInitRet;
	CSerial lSerial(m_ServerCfg.CharSet);
	BYTE bCnt=0;
	if(m_ServerCfg.DemoEn==0){
		CWnd *pParent=CWnd::FromHandle(::GetTopWindow(NULL));
		CDlgQuCtl DlgQuCtl(pParent);
		CDevMng& GDevMng=GetGlobalDevMng();
		CProjInfo& GProjInfo=GetGlobalProjInfo();
		GProjInfo.AttachQuCtlIdentifier(&DlgQuCtl);
		GDevMng.InitDevProjectEnv(&GProjInfo);///////二次开发或MES在这里初始化
		GProjInfo.DetachQuCtlIdentifier();
		SiteCnt=GDevMng.GetDevCnt(CDevMng::GROUP_ATTACNED);
		bCnt=(BYTE)SiteCnt;
		lSerial<<bCnt;
		for(i=0;i<SiteCnt;i++){
			DEVINFO * pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED,i);
			if(pDevInfo){
				lSerial<<pDevInfo->DevSN;
				if(pDevInfo->IsProjEnvSet==TRUE){
					EnvInitRet=0;
				}
				else{
					EnvInitRet=-1;
				}
				lSerial<<EnvInitRet;
			}
		}
	}
	else{
		SiteCnt=m_DemoCfg.SiteNum;
		bCnt=(BYTE)m_DemoCfg.SiteNum;
		lSerial<<bCnt;
		for(i=0;i<SiteCnt;i++){
			lSerial<<m_DemoCfg.vpVirtualSites[i]->strSN;
			EnvInitRet=0;
			lSerial<<EnvInitRet;
		}
	}
	ClientSetInitResult(lSerial);
	return 0;
}

INT WINAPI ServerAppThreadProc(MSG msg,void *Para)
{
	INT Ret=ACERR_OK;
	CIceServerApp*pIceServerApp=(CIceServerApp*)Para;
	if(msg.message==TMSG_ICEINITSITEENV){
		pIceServerApp->ServerDoInitSiteEnv();
	}
	else if(msg.message==TMSG_ICELOADPROJECT){
		pIceServerApp->ServerDoLoadProject();
	}
	else if(msg.message==TMSG_ICECREATEPROJECT){
		pIceServerApp->ServerDoCreateProject();
	}
	else if(msg.message==TMSG_ICECREATEPROJBYTEMPLATE){
		pIceServerApp->ServerDoCreateProjectByTemplate();
	}
	Ret=-1; ///为了让线程自己退出
	return Ret;
}

INT CIceServerApp::InitEnvInWorkThread(void)
{
	INT Ret=ACERR_OK;
	m_wtServerApp.SetMsgHandler(ServerAppThreadProc,this);
	if(m_wtServerApp.CreateThread()==FALSE){
		ClientOutput("ALL",LOGLEVEL_ERR,"Work thread for InitSiteEnv create failed");
		ClientOutput("ALL",LOGLEVEL_ERR,"%s",m_wtServerApp.GetErrMsg());
		Ret=ACERR_FAIL;
	}
	else{
		m_wtServerApp.PostMsg(TMSG_ICEINITSITEENV,0,0);
	}

	return Ret;
}

INT CIceServerApp::HandleInitSiteEnv( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK;
	Ret=InitEnvInWorkThread();
	return Ret;
}

INT CIceServerApp::HandleGetProjectInfo( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK;
	BYTE *pDataResp;
	CSerial lSerial(m_ServerCfg.CharSet);
	CProjInfo& GProjInfo=GetGlobalProjInfo();

	Ret=GProjInfo.GetProjectInfoForServer(lSerial);
	if(Ret==ACERR_OK){
		Resp.iRespDataLen=lSerial.GetLength();
		pDataResp=lSerial.GetBuffer();
		if(Resp.iRespDataLen){
			std::copy(pDataResp,pDataResp+Resp.iRespDataLen,std::back_inserter(Resp.RespData));
		}
	}
	else{
		ClientOutput("ALL",LOGLEVEL_ERR,"GetProjectInfo Execute Failed, Ret=%d",Ret);
	}

	return Ret;
}


INT CIceServerApp::HandleSetChipVID( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK;
	BYTE *TmpBuf=NULL;
	CDevMng& GDevMng=GetGlobalDevMng();
	DEVINFO *pDevInfo=NULL;
	CString DevSN;
	LONG RespCode=0;
	CSerial lSerial(m_ServerCfg.CharSet);
	BYTE *pDataResp=NULL;
	CString strJsonData;
	std::string strJson;
	Json::Value Root;
	Json::Reader Reader;

	if(Msg.iCmdDataLen==0){
		Ret=ACERR_PARA;
		goto __end;
	}
	TmpBuf=new BYTE[Msg.iCmdDataLen];
	if(TmpBuf==NULL){
		Ret=ACERR_MEMALLOC;goto __end;
	}
	std::copy(Msg.CmdData.begin(),Msg.CmdData.end(),TmpBuf);
	lSerial.SerialInBuff(TmpBuf,Msg.iCmdDataLen);
	SAFEDELARRAY(TmpBuf);
	lSerial>>strJsonData;
	ClientOutput("ALL",LOGLEVEL_LOG,"HandleSetChipVID Serial string :%s", strJsonData);

	strJson.assign(strJsonData);

	if(Reader.parse(strJson, Root)==false){
		Ret=-1; goto __end;
	}

	CProjInfo& GProjInfo=GetGlobalProjInfo();
	Ret = GProjInfo.SetChipVIDInfo(strJsonData);

__end:
	ClientOutput("ALL",LOGLEVEL_WARNING,"Execute HandleSetChipVID  , Ret=%d",Ret);
	return Ret;
}

INT CIceServerApp::HandleSetLabel( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK;
	BYTE *TmpBuf=NULL;
	CDevMng& GDevMng = GetGlobalDevMng();
	CProjInfo& GProjInfo = GetGlobalProjInfo();
	DEVINFO *pDevInfo=NULL;
	CString DevSN;
	LONG RespCode=0;
	CSerial lSerial(m_ServerCfg.CharSet);
	BYTE *pDataResp=NULL;
	CString strJsonData;
	std::string strJson;
	Json::Value Root;
	Json::Reader Reader;
	CString strChipTestSln = _T("");
	Json::Value LabelInfo;
	CString strDevSN, strAdapterEn, strSwap;

	if(Msg.iCmdDataLen==0){
		Ret=ACERR_PARA;
		goto __end;
	}
	TmpBuf=new BYTE[Msg.iCmdDataLen];
	if(TmpBuf==NULL){
		Ret=ACERR_MEMALLOC;goto __end;
	}
	std::copy(Msg.CmdData.begin(),Msg.CmdData.end(),TmpBuf);
	lSerial.SerialInBuff(TmpBuf,Msg.iCmdDataLen);
	SAFEDELARRAY(TmpBuf);
	lSerial>>strJsonData;
	ClientOutput("ALL",LOGLEVEL_LOG,"HandleSetLabel Serial string :%s", strJsonData);

	strJson.assign(strJsonData);

	if(Reader.parse(strJson, Root)==false){
		ClientOutput("ALL",LOGLEVEL_LOG, "UpdateLabel Fail %s", strJson);
		Ret=ACERR_PARA;
		goto __end;
	}
	LabelInfo = Root["LabelInfo"];
	strDevSN.Format("%s", Root["DevSN"].asCString() );
	strAdapterEn.Format("%s", Root["AdapterEn"].asCString() );
	strSwap.Format("%s", Root["Swap"].asCString());

	
	pDevInfo = GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED, strDevSN);

	if (LabelInfo.isArray()){
		for (UINT i = 0; i < LabelInfo.size(); i++){
			Json::Value item;
			item = LabelInfo[i];
			INT sktIdx = item["SKT"].asInt();
			CString sktLabel;
			sktLabel.Format("%s", item["L"].asCString());

			INT setSKTIdx = sktIdx-1;
			if (strSwap.CompareNoCase("true") == 0){
				if (setSKTIdx < 4){
					setSKTIdx = setSKTIdx + 4;
				}else {
					setSKTIdx = setSKTIdx - 4;
				}
			}
			pDevInfo->LabelInfo[setSKTIdx].strLabelInfo = sktLabel;
		}
	}

__end:
	ClientOutput("ALL",LOGLEVEL_WARNING,"Execute HandleSetLabel, Ret=%d",Ret);
	return Ret;
}

INT CIceServerApp::HandleGetSNCount(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp)
{
	INT Ret=ACERR_OK;
	CString strTemp;
	CProjInfo& GProjInfo=GetGlobalProjInfo();
	CMultiChipSN* pMultiChipSN = NULL;
	pMultiChipSN = GProjInfo.GetMultChipSN();
	if(pMultiChipSN->GetSNFileSerNum(strTemp) == TRUE)
	{
		Resp.iRespDataLen = strTemp.GetLength()+1;
	}
	else
	{
		cJSON* RootBuild = cJSON_CreateObject();
		cJSON_AddNumberToObject(RootBuild, "SNCountCanBeUse", 999999);
		strTemp = cJSON_Print(RootBuild);
		Resp.iRespDataLen = strTemp.GetLength()+1;
	}
	
	if(Resp.iRespDataLen){
		std::copy(strTemp.GetBuffer(),strTemp.GetBuffer()+Resp.iRespDataLen,std::back_inserter(Resp.RespData));
	}else{
		ClientOutput("ALL",LOGLEVEL_ERR,"HandleGetSNCount Catch An Exception");
		Ret=ACERR_FAIL;
	}
	return Ret;
}

INT CIceServerApp::HandleGetStatisticCnt(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp)
{
	INT Ret=ACERR_OK;
	CString strTemp,appPath,strIniFile;
	INT n_PassCnt,n_FailCnt,n_TotalNum;
	cJSON* pRoot = NULL;
	
	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, appPath.GetBuffer(MAX_PATH));
	appPath.ReleaseBuffer();
	appPath.Format("%s\\MultiAprog",appPath);
	if(!PathIsDirectory(appPath)){Ret=ACERR_FAIL; return Ret;}
	strIniFile.Format("%s\\Setting.ini",appPath);

	n_PassCnt = GetPrivateProfileInt("Setting", "PassCnt", 0, strIniFile);
	n_FailCnt = GetPrivateProfileInt("Setting", "FailCnt", 0, strIniFile);
	n_TotalNum = GetPrivateProfileInt("Setting", "TotalNum", 0, strIniFile);
	
	pRoot = cJSON_CreateObject();
	cJSON_AddNumberToObject(pRoot, "TotalNum",n_TotalNum);
	cJSON_AddNumberToObject(pRoot, "PassCnt",n_PassCnt);
	cJSON_AddNumberToObject(pRoot, "FailCnt",n_FailCnt);
	strTemp = cJSON_PrintUnformatted(pRoot);

	Resp.iRespDataLen = strTemp.GetLength()+1;
	if(Resp.iRespDataLen){
		std::copy(strTemp.GetBuffer(),strTemp.GetBuffer()+Resp.iRespDataLen,std::back_inserter(Resp.RespData));
	}else{
		ClientOutput("ALL",LOGLEVEL_ERR,"HandleGetStatisticCnt Catch An Exception");
		Ret=ACERR_FAIL;
	}

	if (pRoot != NULL) {
		cJSON_Delete(pRoot);
	}
	return Ret;
}

INT CIceServerApp::HandleEnableCheckID(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp)
{
	INT Ret=ACERR_OK;
	BYTE *TmpBuf=NULL;
	CDevMng& GDevMng=GetGlobalDevMng();
	DEVINFO *pDevInfo=NULL;
	CString DevSN;
	LONG RespCode=0;
	CSerial lSerial(m_ServerCfg.CharSet);
	BYTE *pDataResp=NULL;
	UINT CheckIDEn=0;
	EnterCriticalSection(&m_csCmdProtct);
	if(Msg.iCmdDataLen==0){
		Ret=ACERR_PARA;
		goto __end;
	}
	TmpBuf=new BYTE[Msg.iCmdDataLen];
	if(TmpBuf==NULL){
		Ret=ACERR_MEMALLOC;goto __end;
	}
	std::copy(Msg.CmdData.begin(),Msg.CmdData.end(),TmpBuf);
	lSerial.SerialInBuff(TmpBuf,Msg.iCmdDataLen);
	SAFEDELARRAY(TmpBuf);
	lSerial>>DevSN>>CheckIDEn;
	//CLogMsg::PrintDebugString("Call CheckIDEn SN=%s, CheckIDEn=0x%08X",DevSN,CheckIDEn);
	if(m_ServerCfg.DemoEn==0){
		pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED,DevSN);
		if(pDevInfo==NULL){
			ClientOutput("ALL",LOGLEVEL_ERR,"Can't find DevSN=%s in the connected queue",DevSN);
			GDevMng.PrintLog(LOGLEVEL_LOG,"Can't find DevSN=%s in the connected queue",DevSN);
			Ret=ACERR_FAIL; goto __end;
		}
		GDevMng.PrintLog(LOGLEVEL_LOG,"EnableCheckIDForRmtServer CheckIDEn:%d",CheckIDEn);
		Ret=pDevInfo->pAprogDev->EnableCheckIDForRmtServer(CheckIDEn);
	}
	

__end:
	LeaveCriticalSection(&m_csCmdProtct);
	return Ret;
	
}



INT CIceServerApp::HandleSetAdapterEn( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK;
	BYTE *TmpBuf=NULL;
	CDevMng& GDevMng=GetGlobalDevMng();
	DEVINFO *pDevInfo=NULL;
	CString DevSN;
	LONG RespCode=0;
	CSerial lSerial(m_ServerCfg.CharSet);
	BYTE *pDataResp=NULL;
	UINT AdpEn=0;
	EnterCriticalSection(&m_csCmdProtct);
	if(Msg.iCmdDataLen==0){
		Ret=ACERR_PARA;
		goto __end;
	}
	TmpBuf=new BYTE[Msg.iCmdDataLen];
	if(TmpBuf==NULL){
		Ret=ACERR_MEMALLOC;goto __end;
	}
	std::copy(Msg.CmdData.begin(),Msg.CmdData.end(),TmpBuf);
	lSerial.SerialInBuff(TmpBuf,Msg.iCmdDataLen);
	SAFEDELARRAY(TmpBuf);
	lSerial>>DevSN>>AdpEn;
	CLogMsg::PrintDebugString("Call SetAdapterEn SN=%s, AdpEn=0x%08X",DevSN,AdpEn);
	if(m_ServerCfg.DemoEn==0){
		pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED,DevSN);
		if(pDevInfo==NULL){
			ClientOutput("ALL",LOGLEVEL_ERR,"Can't find DevSN=%s in the connected queue",DevSN);
			GDevMng.PrintLog(LOGLEVEL_LOG,"Can't find DevSN=%s in the connected queue",DevSN);
			Ret=ACERR_FAIL; goto __end;
		}
		Ret=pDevInfo->pAprogDev->SetAdpEnForRmtServer(AdpEn);
	}
	else{///Demo模式下的处理
		INT i=0,j=0;
		for(i=0;i<m_DemoCfg.SiteNum;i++){
			if(m_DemoCfg.vpVirtualSites[i]->strSN==DevSN){
				for(j=0;j<8;j++){
					if(m_DemoCfg.vpVirtualSites[i]->SktStatus[j]==DEMOSTATUS_UNUSED){
						continue;
					}
					if((AdpEn>>j)&0x01){///使能该Socket
						m_DemoCfg.vpVirtualSites[i]->SktStatus[j]=DEMOSTATUS_PASS;
					}
					else{
						m_DemoCfg.vpVirtualSites[i]->SktStatus[j]=DEMOSTATUS_INACTIVE;
					}
				}
				((CDlgServerDemo*)m_DemoCfg.pDlgServerDemo)->UpdateSKTEnabled();
				break;
			}
		}
		if(i==m_DemoCfg.SiteNum){
			ClientOutput("ALL",LOGLEVEL_ERR,"Can't find DevSN=%s in the connected queue",DevSN);
			Ret=ACERR_FAIL; goto __end;
		}
	}
	
__end:
	LeaveCriticalSection(&m_csCmdProtct);
	return Ret;
}

INT CIceServerApp::HandleGetConfig_String(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp)
{

	INT Ret=ACERR_OK;
	BYTE *pDataResp;
	CSerial lSerial(m_ServerCfg.CharSet);
	CProjInfo& GProjInfo=GetGlobalProjInfo();

	Ret=GProjInfo.GetProjectConfig_String(lSerial);
	if(Ret==ACERR_OK){
		Resp.iRespDataLen=lSerial.GetLength();
		pDataResp=lSerial.GetBuffer();
		if(Resp.iRespDataLen){
			std::copy(pDataResp,pDataResp+Resp.iRespDataLen,std::back_inserter(Resp.RespData));
		}
	}
	else{
		ClientOutput("ALL",LOGLEVEL_ERR,"HandleGetConfig_String Execute Failed, Ret=%d",Ret);
	}

	return Ret;
}


INT CIceServerApp::HandleGetConfig_Json( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK;
	BYTE *pDataResp;
	CSerial lSerial(m_ServerCfg.CharSet);
	CProjInfo& GProjInfo=GetGlobalProjInfo();

	Ret=GProjInfo.GetProjectConfig_Json(lSerial);
	if(Ret==ACERR_OK){
		Resp.iRespDataLen=lSerial.GetLength();
		pDataResp=lSerial.GetBuffer();
		if(Resp.iRespDataLen){
			std::copy(pDataResp,pDataResp+Resp.iRespDataLen,std::back_inserter(Resp.RespData));
		}
	}
	else{
		ClientOutput("ALL",LOGLEVEL_ERR,"HandleGetConfig_Json Execute Failed, Ret=%d",Ret);
	}

	return Ret;
}

INT CIceServerApp::HandleGetAdapterEn( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK;
	BYTE *TmpBuf=NULL;
	CDevMng& GDevMng=GetGlobalDevMng();
	DEVINFO *pDevInfo=NULL;
	CString DevSN;
	LONG RespCode=0;
	CSerial lSerial(m_ServerCfg.CharSet);
	BYTE *pDataResp=NULL;
	UINT AdpEn=0;
	if(Msg.iCmdDataLen==0){
		Ret=ACERR_PARA;
		goto __end;
	}
	TmpBuf=new BYTE[Msg.iCmdDataLen];
	if(TmpBuf==NULL){
		Ret=ACERR_MEMALLOC;goto __end;
	}
	std::copy(Msg.CmdData.begin(),Msg.CmdData.end(),TmpBuf);
	lSerial.SerialInBuff(TmpBuf,Msg.iCmdDataLen);
	SAFEDELARRAY(TmpBuf);
	lSerial>>DevSN;
	if(m_ServerCfg.DemoEn==0){
		pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED,DevSN);
		if(pDevInfo==NULL){
			ClientOutput("ALL",LOGLEVEL_ERR,"Can't find DevSN=%s in the connected queue",DevSN);
			Ret=ACERR_FAIL; goto __end;
		}
		AdpEn=pDevInfo->pAprogDev->GetAdpEnForRmtServer();
	}
	else{
		INT i=0,j=0;
		for(i=0;i<m_DemoCfg.SiteNum;i++){
			if(m_DemoCfg.vpVirtualSites[i]->strSN==DevSN){
				for(j=0;j<8;j++){
					if(m_DemoCfg.vpVirtualSites[i]->SktStatus[j]<=DEMOSTATUS_INACTIVE){
						AdpEn |=0<<j;
					}
					else{
						AdpEn |=1<<j;
					}
				}
				break;
			}
		}
		if(i==m_DemoCfg.SiteNum){
			ClientOutput("ALL",LOGLEVEL_ERR,"Can't find DevSN=%s in the connected queue",DevSN);
			Ret=ACERR_FAIL; goto __end;
		}
	}
	Resp.iRespDataLen=4;
	pDataResp=(BYTE*)&AdpEn;
	if(Resp.iRespDataLen){
		std::copy(pDataResp,pDataResp+Resp.iRespDataLen,std::back_inserter(Resp.RespData));
	}
	//ClientOutput("ALL",LOGLEVEL_ERR,"LoadProject GetCmdLen=%d,%s",Msg.iCmdDataLen,TmpBuf);
__end:
	return Ret;
}

INT CIceServerApp::HandleEnableSN( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK;
	BYTE *TmpBuf=NULL;
	CDevMng& GDevMng=GetGlobalDevMng();
	DEVINFO *pDevInfo=NULL;
	CString DevSN;
	LONG RespCode=0;
	CSerial lSerial(m_ServerCfg.CharSet);
	BYTE *pDataResp=NULL;
	UINT SNEn=0;

	if(Msg.iCmdDataLen==0){
		Ret=ACERR_PARA;
		goto __end;
	}
	TmpBuf=new BYTE[Msg.iCmdDataLen];
	if(TmpBuf==NULL){
		Ret=ACERR_MEMALLOC;goto __end;
	}
	std::copy(Msg.CmdData.begin(),Msg.CmdData.end(),TmpBuf);
	lSerial.SerialInBuff(TmpBuf,Msg.iCmdDataLen);
	SAFEDELARRAY(TmpBuf);
	lSerial>>DevSN>>SNEn;
	pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED,DevSN);
	if(pDevInfo==NULL){
		ClientOutput("ALL",LOGLEVEL_ERR,"Can't find DevSN=%s in the connected queue",DevSN);
		Ret=ACERR_FAIL; goto __end;
	}
	
	if(pDevInfo->pAprogDev){
		if(SNEn){
			if(pDevInfo->pAprogDev->IsSNMultDirectModeEn()==FALSE){///先判断，避免不必要的重复设置
				Ret=pDevInfo->pAprogDev->SetSNMultDirectMode(TRUE);
				if(Ret!=ACERR_OK){///设置失败
					goto __end;
				}
			}
		}
		else{
			Ret=pDevInfo->pAprogDev->SetSNMultDirectMode(FALSE);
			if(Ret!=ACERR_OK){///设置失败
				goto __end;
			}
		}
	}
__end:
	return Ret;
}

INT CIceServerApp::SerialCmdData(const ::RemoteAprog::tagMSG&Msg, CSerial& lSerial)
{
	INT Ret=ACERR_OK;
	BYTE *TmpBuf=NULL;
	lSerial.ReInit();
	if(Msg.iCmdDataLen==0){
		Ret=ACERR_PARA;
		goto __end;
	}
	TmpBuf=new BYTE[Msg.iCmdDataLen];
	if(TmpBuf==NULL){
		Ret=ACERR_MEMALLOC;goto __end;
	}
	std::copy(Msg.CmdData.begin(),Msg.CmdData.end(),TmpBuf);
	try{
		lSerial.SerialInBuff(TmpBuf,Msg.iCmdDataLen);
	}
	catch (...){
		Ret=ACERR_PARA;	
	}
__end:
	SAFEDELARRAY(TmpBuf);
	return Ret;
}


INT CIceServerApp::CreateProjectByTempleateInWorkThread()
{
	INT Ret=ACERR_OK;
	m_wtServerApp.SetMsgHandler(ServerAppThreadProc,this);
	if(m_wtServerApp.CreateThread()==FALSE){
		ClientOutput("ALL",LOGLEVEL_ERR,"Work thread for CreateProjectByTemplate create failed");
		ClientOutput("ALL",LOGLEVEL_ERR,"%s",m_wtServerApp.GetErrMsg());
		Ret=ACERR_FAIL;
	}
	else{
		m_wtServerApp.PostMsg(TMSG_ICECREATEPROJBYTEMPLATE,0,0);
	}
	return Ret;
}

INT CIceServerApp::HandleCreateProjectByTemplate( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp)
{
	INT Ret=ACERR_OK;
	CSerial lSerial;
	ClientOutput("ALL",LOGLEVEL_ERR,"Server Handle Command=CreateProjectByTemplate");
	m_lCreateProjInfo.SetCharSet(m_ServerCfg.CharSet);
	m_lCreateProjInfo.ReInit();
	Ret=SerialCmdData(Msg,m_lCreateProjInfo);
	if(Ret==ACERR_OK){
		Ret= CreateProjectByTempleateInWorkThread();
	}
	return Ret;
}

INT CIceServerApp::HandleSetSN( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK;
	BYTE *TmpBuf=NULL;
	CDevMng& GDevMng=GetGlobalDevMng();
	CProjInfo& GProjInfo=GetGlobalProjInfo();
	DEVINFO *pDevInfo=NULL;
	CString DevSN;
	LONG RespCode=0;
	CSerial lSerial(m_ServerCfg.CharSet);
	BYTE *pDataResp=NULL;

	if(Msg.iCmdDataLen==0){
		Ret=ACERR_PARA;
		goto __end;
	}
	TmpBuf=new BYTE[Msg.iCmdDataLen];
	if(TmpBuf==NULL){
		Ret=ACERR_MEMALLOC;goto __end;
	}
	std::copy(Msg.CmdData.begin(),Msg.CmdData.end(),TmpBuf);
	lSerial.SerialInBuff(TmpBuf,Msg.iCmdDataLen);
	SAFEDELARRAY(TmpBuf);
	lSerial>>DevSN;
	pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED,DevSN);
	if(pDevInfo==NULL){
		ClientOutput("ALL",LOGLEVEL_ERR,"Can't find DevSN=%s in the connected queue",DevSN);
		Ret=ACERR_FAIL; goto __end;
	}

	if(pDevInfo->pAprogDev){
		if(pDevInfo->pAprogDev->IsSNMultDirectModeEn()==TRUE){///先判断，避免不必要的重复设置
			if(lSerial.GetLength()<= MAX_SNSIZE){///修改限制为512K一个SKT，总的包长度不超过4M
				Ret=pDevInfo->pAprogDev->DownLoadMultSn(&GProjInfo,lSerial);
			}
			else{///需要分多个包下载下去
				BYTE*pData=lSerial.GetBuffer();
				UINT DataSize=lSerial.GetLength();
				ClientOutput("ALL",LOGLEVEL_ERR,"Multi Set SN DataSize=%d",lSerial.GetLength());;
				ClientOutput("ALL",LOGLEVEL_ERR,"BYTE[0-7]=%02X %02X %02X %02X %02X %02X %02X %02X",
					pData[0],pData[1],pData[2],pData[3],pData[4],pData[5],pData[6],pData[7]);
				ClientOutput("ALL",LOGLEVEL_ERR,"Last BYTE[0-7]=%02X %02X %02X %02X %02X %02X %02X %02X",
					pData[DataSize-8],pData[DataSize-7],pData[DataSize-6],pData[DataSize-5],pData[DataSize-4],
					pData[DataSize-3],pData[DataSize-2],pData[DataSize-1]);
				GDevMng.PrintLog(LOGLEVEL_ERR,"Multi Set SN DataSize=%d，MaxSize:%d",lSerial.GetLength(),MAX_SNSIZE);
				Ret=ACERR_FAIL;
				goto __end;
			}
			if(Ret!=ACERR_OK){///设置失败
				goto __end;
			}
		}
		else{
			ClientOutput("ALL",LOGLEVEL_ERR,"Please enable the SN for %s first",DevSN);
			Ret=ACERR_FAIL;
		}
	}
__end:
	return Ret;
}

INT CIceServerApp::GetCheckedCmd(INT Cmd)
{
	INT Ret = Cmd;
	CProjInfo& GProjInfo = GetGlobalProjInfo();
	CDevMng& GDevMng = GetGlobalDevMng();
	BOOL FactoryModeEn = GProjInfo.CheckFactoryModeCmdEn(Cmd);
	if(!FactoryModeEn){
		//是工厂模式但没有支持此指令
		GDevMng.PrintLog(LOGLEVEL_LOG,"Operation %d not enabled in factory mode.", Cmd);
		Ret = 0;
	}
	return Ret;
}

INT CIceServerApp::GetDevCmd(const CString strCmd)
{
	INT DevCmd=0;
	CProjInfo& GProjInfo=GetGlobalProjInfo();
	if(strCmd=="Program"){
		DevCmd = GetCheckedCmd(CMD_DevProgram);
	}
	else if(strCmd=="Verify"){
		DevCmd = GetCheckedCmd(CMD_DevVerify);
	}
	else if(strCmd=="Erase"){
		DevCmd = GetCheckedCmd(CMD_DevErase);
	}
	else if(strCmd=="BlankCheck"){
		DevCmd = GetCheckedCmd(CMD_DevBlankChk);
	}
	else if(strCmd=="Secure"){
		DevCmd = GetCheckedCmd(CMD_DevSecure);
	}
	else if(strCmd=="InsertionCheck"){
		DevCmd=CMD_DevInsSen;
	}
	else if(strCmd=="AutoStart"){///自动化执行命令，由工程文件中的工厂模式中选择的命令确定
		//DevCmd=CMD_ExecCmd;

		DevCmd=GProjInfo.GetFactoryCmdForRmtAutoStart();
		if(DevCmd==-2){///没有使能Factory模式
			DevCmd=GProjInfo.GetStdMesCmdForRmtAutoStart();
			if(DevCmd==-2){
				ClientOutput("ALL",LOGLEVEL_ERR,"The project must be created with factory mode enabled or created by MES interface");
				ClientOutput("ALL",LOGLEVEL_ERR,"Execute Program function for default");
				DevCmd=CMD_DevProgram;
			}
			else if(DevCmd==-1){
				ClientOutput("ALL",LOGLEVEL_ERR,"Can't fix the command to be executed in the project created by MES interface");
				DevCmd=0;
			}
		}
		else if(DevCmd==-1){
			ClientOutput("ALL",LOGLEVEL_ERR,"Can't fix the command to be executed in the project under factory mode");
			DevCmd=0;
		}
	}
	if(DevCmd>0 && DevCmd!=CMD_DevInsSen){ ///不能去匹配InsertionCheck
		if(GProjInfo.CheckProjectSupportCmd(DevCmd)!=0){
			ClientOutput("ALL",LOGLEVEL_ERR,"Project Not Support Command:%s",strCmd);
			DevCmd=0;
		}
	}
	return DevCmd;
}

CString CIceServerApp::GetCmdName(INT DevCmd)
{
	CString strCmd="";
	switch(DevCmd){
		case CMD_DevProgram:
			strCmd="Program";
			break;
		case CMD_DevVerify:
			strCmd="Verify";
			break;
		case CMD_DevErase:
			strCmd="Erase";
			break;
		case CMD_DevBlankChk:
			strCmd="BlankCheck";
			break;
		case CMD_DevSecure:
			strCmd="Secure";
			break;
		case CMD_DevInsSen:
			strCmd="InsertionCheck";
			break;
	}
	return strCmd;
}


INT CIceServerApp::GetDemoCmdExecDelayTime()
{
	return m_DemoCfg.Delay;
}

INT CIceServerApp::GetDemoUserResult(tVirtualSite *pVirtualSite,UINT&AdpCnt,UINT&AdpStatus)
{
	INT Ret=0,i;
	AdpStatus=0;
	if(m_DemoCfg.pDlgServerDemo){
		CDlgServerDemo *pServerDemo=dynamic_cast<CDlgServerDemo*>(m_DemoCfg.pDlgServerDemo);
		pServerDemo->GetUserSetting();
		AdpCnt=(INT)m_DemoCfg.SocketNumPerSite;
		for(i=0;i<(INT)AdpCnt;i++){
			if(pVirtualSite->SktStatus[i]==DEMOSTATUS_PASS){
				AdpStatus |=0x00000002<<(i*2);
				pVirtualSite->ErrCode[i]=DEVERR_PASS;
			}
			else if(pVirtualSite->SktStatus[i]==DEMOSTATUS_FAIL){
				AdpStatus |=0x00000003<<(i*2);
				UINT ErrCode = 0;
				CString strErrCode;
				strErrCode.Format("%s", m_DemoCfg.strErrCode);
				strErrCode.Replace("0x", "");
				strErrCode.Replace("0X", "");
				sscanf(strErrCode.GetBuffer(),"%X",&ErrCode);
				pVirtualSite->ErrCode[i]=ErrCode;
			}
		}
	}
	else{
		Ret=-1;
	}
	return Ret;
}


INT CIceServerApp::DoJobInWorkThread(CString DevSN,INT DevCmd)
{
	INT Ret=ACERR_OK;
	if(m_ServerCfg.DemoEn==0){
		DEVINFO *pDevInfo=NULL;
		CDevMng& GDevMng=GetGlobalDevMng();
		pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED,DevSN);
		if(pDevInfo==NULL){
			ClientOutput("ALL",LOGLEVEL_ERR,"Can't find DevSN=%s in the connected queue",DevSN);
			Ret=ACERR_FAIL; goto __end;
		}
		m_pDevInfo = pDevInfo;
		pDevInfo->m_wtRmtJob.SetMsgHandler(DevDoJobThreadProc,pDevInfo);
		if(pDevInfo->m_wtRmtJob.CreateThread()==FALSE){
			ClientOutput("ALL",LOGLEVEL_ERR,"Work thread for %s create failed",DevSN);
			Ret=ACERR_FAIL;
		}
		else{
			pDevInfo->m_wtRmtJob.PostMsg(TMSG_ICEDOJOB,(WPARAM)DevCmd,0);
		}
	}
	else{
		INT i=0;
		for(i=0;i<m_DemoCfg.SiteNum;i++){
			if(m_DemoCfg.vpVirtualSites[i]->strSN==DevSN){
				m_DemoCfg.vpVirtualSites[i]->m_wtRmtJob.SetMsgHandler(DevDoJobThreadProc,m_DemoCfg.vpVirtualSites[i]);
				if(m_DemoCfg.vpVirtualSites[i]->m_wtRmtJob.CreateThread()==FALSE){
					ClientOutput("ALL",LOGLEVEL_ERR,"Work thread for %s create failed",DevSN);
					Ret=ACERR_FAIL;
				}
				else{
					m_DemoCfg.vpVirtualSites[i]->m_wtRmtJob.PostMsg(TMSG_ICEDOJOBDEMO,(WPARAM)DevCmd,0);
				}
				break;
			}
		}
		if(i==m_DemoCfg.SiteNum){
			ClientOutput("ALL",LOGLEVEL_ERR,"Can't find DevSN=%s in the connected queue",DevSN);
			Ret=ACERR_FAIL; goto __end;
		}
	}
	

__end:
	return Ret;
}

INT CIceServerApp::HandleDoJob( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	//ClientOutput("ALL",LOGLEVEL_ERR,"Server Handle Command=DoJob");
	INT Ret=ACERR_OK;
	BYTE *TmpBuf=NULL;
	CDevMng& GDevMng=GetGlobalDevMng();
	CString DevSN,strCmd;
	DEVINFO *pDevInfo=NULL;
	LONG RespCode=0;
	INT DevCmd=0;
	CSerial lSerial(m_ServerCfg.CharSet);
	if(Msg.iCmdDataLen==0){
		Ret=ACERR_PARA;
		goto __end;
	}
	TmpBuf=new BYTE[Msg.iCmdDataLen];
	if(TmpBuf==NULL){
		goto __end;
	}
	std::copy(Msg.CmdData.begin(),Msg.CmdData.end(),TmpBuf);
	lSerial.SerialInBuff(TmpBuf,Msg.iCmdDataLen);
	SAFEDELARRAY(TmpBuf);
	//GDevMng.DumpBuf(LOGLEVEL_WARNING,lSerial.GetBuffer(),lSerial.GetLength(),16);
	lSerial>>DevSN>>strCmd;
	DevCmd=GetDevCmd(strCmd);
	CLogMsg::PrintDebugString("Handle Do Job DevSN=%s,strCmd=%s\n",DevSN,strCmd);
	if(DevCmd==0){
		GDevMng.PrintLog(LOGLEVEL_ERR,"DevSN=%s, Can't Support Job Command=%s",DevSN,strCmd);
		Ret=ACERR_CMDUNSUPPORT; goto __end;
	}
	GDevMng.PrintLog(LOGLEVEL_LOG,"Handle Do Job DevSN=%s,strCmd=%s",DevSN,strCmd);
	Ret=DoJobInWorkThread(DevSN,DevCmd);
	//ClientOutput("ALL",LOGLEVEL_ERR,"LoadProject GetCmdLen=%d,%s",Msg.iCmdDataLen,TmpBuf);
__end:
	
	return Ret;
}
////处理Custom Tag
INT CIceServerApp::HandleCTagGetAllSkbInfoDemo(const CString& strDevSN,UINT TagID,UINT TagDataSize,BYTE *pTagData)
{
	INT Ret=ACERR_OK,RetCmd=ACERR_OK;
	CSerial lSerial;
	INT i,j;
	INT LimitCnt=20000,CurrentCnt=1000;
	for(i=0;i<m_DemoCfg.SiteNum;++i){
		if(m_DemoCfg.vpVirtualSites[i]->strSN==strDevSN){
			lSerial<<m_DemoCfg.SocketNumPerSite;
			for(j=0;j<m_DemoCfg.SocketNumPerSite;++j){
				lSerial<<CurrentCnt<<LimitCnt;	
			}
			break;
		}
	}
	if(i==m_DemoCfg.SiteNum){
		ClientOutput("ALL",LOGLEVEL_ERR,"Can't find DevSN=%s in the connected queue",strDevSN);
		Ret=ACERR_FAIL; goto __end;
	}
	Ret=ClientDoCustom(strDevSN,TagID,lSerial.GetBuffer(),lSerial.GetLength(),NULL,NULL);
__end:
	return Ret;
}

////处理Custom Tag
#include <time.h>

static UINT64 GetRandom()
{
	INT i;
	UINT64 Rv=0;
	BYTE *pData=(BYTE*)&Rv;
	srand((unsigned)time(NULL));
	for(i=0;i<8;i++){
		pData[i]=rand()%255;
	}
	return Rv;
}
INT CIceServerApp::HandleCTagGetAllSkbUIDDemo(const CString& strDevSN,UINT TagID,UINT TagDataSize,BYTE *pTagData)
{
	CSerial lSerial;
	INT Ret=ACERR_OK,RetCmd=ACERR_OK;
	INT i;
	UINT64 SktUID[8];

	for(i=0;i<m_DemoCfg.SiteNum;++i){
		if(m_DemoCfg.vpVirtualSites[i]->strSN==strDevSN){
			for(i=0;i<8;++i){
				SktUID[i]=GetRandom();
				lSerial<<SktUID[i];
			}
			break;
		}
	}
	if(i==m_DemoCfg.SiteNum){
		ClientOutput("ALL",LOGLEVEL_ERR,"Can't find DevSN=%s in the connected queue",strDevSN);
		Ret=ACERR_FAIL; goto __end;
	}
	Ret=ClientDoCustom(strDevSN,TagID,lSerial.GetBuffer(),lSerial.GetLength(),NULL,NULL);
__end:
	return Ret;
}

/****************
PC端出来Custom
返回0表示处理成功
返回负数表示处理失败
返回1表示没有处理
****************/
INT CIceServerApp::LocalDoCustm(const CString&strDevSN,UINT TagID,UINT TagDataSize,BYTE*pTagData)
{
	INT Ret=1;
	switch(TagID){
		case TAG_GETALLSKBINFO:
			if(m_ServerCfg.DemoEn==1){
				Ret=HandleCTagGetAllSkbInfoDemo(strDevSN,TagID,TagDataSize,pTagData);
			}
			break;	
		case TAG_GET_ALLSKBSMPINFO_EXT:
			if(m_ServerCfg.DemoEn==1){
				//Ret=HandleCTagGetAllSkbInfoDemo(strDevSN,TagID,TagDataSize,pTagData);
			}
			break;
		case TAG_GETALLSKBUID:
			if(m_ServerCfg.DemoEn==1){
				Ret=HandleCTagGetAllSkbUIDDemo(strDevSN,TagID,TagDataSize,pTagData);
			}
			break;
		case TAG_INIT_AUTOSENSORCMD:
			if(m_ServerCfg.DemoEn==1){
				Ret=0;
			}
			break;
	}
	return Ret;
}

INT CIceServerApp::SiteDoCustom(const CString&strDevSN,UINT TagID,UINT TagDataSize,BYTE*pTagData)
{
	INT Ret=ACERR_OK,RetCmd=ACERR_OK;
	if(m_ServerCfg.DemoEn==0){
		CDevMng&GDevMng= GetGlobalDevMng();
		DEVINFO *pDevInfo=NULL;
		pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED,strDevSN);
		if(pDevInfo==NULL){
			ClientOutput("ALL",LOGLEVEL_ERR,"Can't find DevSN=%s in the connected queue",strDevSN);
			Ret=ACERR_FAIL; goto __end;
		}

		DWORD StartTime = GetTickCount();//ms
		BOOL isBusy = TRUE;
		while ((GetTickCount() - StartTime) < 1000){
			if (pDevInfo->m_wtRmtJob.GetThreadID() == 0){
				isBusy = FALSE;
				break;
			}
			Sleep(200);
		}
		if (isBusy){
			ClientOutput("ALL",LOGLEVEL_ERR,"DevSN=%s in the dojob thread, later DoCustom",strDevSN);
			Ret=ACERR_FAIL; goto __end;
		}
		///请求执行该CustomTag之后，如果有数据返回，则设备会请求PC也执行该TAG，
		///这个时候会请求客户端执行该Tag并把数据发给客户端
		RetCmd=pDevInfo->pAprogDev->DoCustom(TagID,pTagData,TagDataSize);
		if(RetCmd!=ACERR_OK){
			Ret=ACERR_FAIL;///如果失败则会退出
		}
	}
	else{
		ClientOutput(strDevSN,LOGLEVEL_ERR,"Not Support CustomTag=0x%08X In DemoMode",TagID);
		Ret=ACERR_FAIL;
	}

__end:
	return Ret;
}

INT WINAPI DevDoCustomThreadProc(MSG msg,void *Para)
{
	INT Ret=ACERR_OK;
	CString strDevSN;
	UINT CustomTag = 0;
	UINT TagDataSize= 0;
	BYTE*pTagData = NULL;

	CIceServerApp& GIceServerApp=GetGlobalServerApp();
	if(msg.message==TMSG_ICEDOCUSTOMINTHREAD){
		DEVINFO *pDevInfo=(DEVINFO*)Para;
		if(!pDevInfo){
			GIceServerApp.ClientOutput("ALL",LOGLEVEL_ERR,"Do Custom Thread Normal: Parameter Invalid");
			goto __end;
		}

		tDoCustomData* pPostData = (tDoCustomData*)msg.wParam;
		if ( pPostData == NULL){
			goto __end;
		}
		////////////////////添加dump数据打印///////////////////////////
		if (true){
			CString strMsg;
			strMsg.Format(">>>>>DevSN=%s, CustomTag=0x%X,TagDataSize=%u",pPostData->strDevSN, pPostData->CustomTag, pPostData->TagDataSize);
			GIceServerApp.ClientOutput("ALL",LOGLEVEL_ERR,strMsg);
			strMsg.Format(">>>>>Data is:");
			for (int i = 0; i <pPostData->TagDataSize; i++ ){
				CString strItem;
				strItem.Format("0x%02X, ", pPostData->pTagData[i]);
				strMsg+=strItem;
			}
			GIceServerApp.ClientOutput("ALL",LOGLEVEL_ERR,strMsg);
		}

		Ret =GIceServerApp.SiteDoCustom(pPostData->strDevSN, pPostData->CustomTag, pPostData->TagDataSize, pPostData->pTagData);
		
		SAFEDELARRAY(pPostData->pTagData);
		/*delete [] pPostData->pTagData;
		pPostData->pTagData = NULL;*/

		delete pPostData;
		pPostData = NULL;
	}

__end:
	return  -1/*Ret*/;
}

INT CIceServerApp::CustomInWorkThread(CString strDevSN,UINT CustomTag, UINT TagDataSize, BYTE*pTagData)
{
	CString strMsg;

	INT Ret=ACERR_OK;
	DEVINFO *pDevInfo=NULL;
	CDevMng& GDevMng=GetGlobalDevMng();
	pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED, strDevSN);
	if(pDevInfo==NULL){
		ClientOutput("ALL",LOGLEVEL_ERR,"Can't find DevSN=%s in the connected queue",strDevSN);
		Ret=ACERR_FAIL; goto __end;
	}
	pDevInfo->m_wtRmtCustom.SetMsgHandler(DevDoCustomThreadProc ,pDevInfo);
	if(pDevInfo->m_wtRmtCustom.CreateThread()==FALSE){
		ClientOutput("ALL",LOGLEVEL_ERR,"CustomInWorkThread::Work thread for %s create failed",strDevSN);
		Ret=ACERR_FAIL;
		pDevInfo->m_wtRmtCustom.DeleteThread();
	}
	else{
		tDoCustomData* pPostData = new tDoCustomData;
		pPostData->strDevSN.Format("%s", strDevSN);
		pPostData->CustomTag = CustomTag;
		pPostData->TagDataSize = TagDataSize;
		pPostData->pTagData = NULL;

		if (TagDataSize >0){
			pPostData->pTagData = new BYTE[TagDataSize];
			if (pPostData->pTagData == NULL){
				pDevInfo->m_wtRmtCustom.DeleteThread();
				ClientOutput("ALL",LOGLEVEL_ERR,"malloc memory for pTagData failed!");
				Ret=ACERR_FAIL; goto __end;
			}
			memset(pPostData->pTagData, 0, TagDataSize);
			memcpy(pPostData->pTagData,pTagData,  TagDataSize);
		}else if ( TagDataSize == 0){ //允许为0
		}else{
			pDevInfo->m_wtRmtCustom.DeleteThread();
			ClientOutput("ALL",LOGLEVEL_ERR,"allocate fail, TagDataSize=%d", TagDataSize);
			Ret=ACERR_FAIL; goto __end;
		}

		if ( pDevInfo->m_wtRmtCustom.PostMsg(TMSG_ICEDOCUSTOMINTHREAD,(WPARAM)pPostData,0) == FALSE){
			pDevInfo->m_wtRmtCustom.DeleteThread();
			ClientOutput("ALL",LOGLEVEL_ERR,"Post thread msg failed, TMSG_ICEDOCUSTOMINTHREAD");
			Ret=ACERR_FAIL;
		}
	}
	
__end:
	return Ret;
}

INT CIceServerApp::HandleDoCustomInThread(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp)
{
	INT Ret=ACERR_OK,Rtn;
	CSerial lSerial(m_ServerCfg.CharSet);
	UINT CustomTag,TagDataSize = 0;
	CString strDevSN;
	CString strMsg;
	Ret=SerialCmdData(Msg,lSerial);
	if(Ret!=ACERR_OK){
		goto __end;
	}
	try{
		lSerial>>strDevSN>>CustomTag>>TagDataSize;
		ClientOutput(strDevSN,LOGLEVEL_ERR,"Recv src; strDevSN=%s, CustomTag=0x%X, TagDataSize=0x%X",strDevSN, CustomTag, TagDataSize);
		if(TagDataSize!=lSerial.GetLength()){
			ClientOutput(strDevSN,LOGLEVEL_ERR,"CustomTag=0x%08X Parameter Fail",CustomTag);
			Ret=ACERR_FAIL;
		}
		Rtn=LocalDoCustm(strDevSN,CustomTag,TagDataSize,lSerial.GetBuffer());
		if(Rtn==1){///没有处理
			CustomInWorkThread(strDevSN,CustomTag,TagDataSize,lSerial.GetBuffer());
		}
		else{
			Ret=Rtn;
		} 

	}
	catch (...){

	}

__end:
	return Ret;

}

INT CIceServerApp::HandleDoCustom( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK,Rtn;
	CSerial lSerial(m_ServerCfg.CharSet);
	UINT CustomTag,TagDataSize;
	CString strDevSN;
	//ClientOutput("ALL",LOGLEVEL_ERR,"Server Handle Command=DoCustom");
	Ret=SerialCmdData(Msg,lSerial);
	if(Ret!=ACERR_OK){
		goto __end;
	}
	try{
		lSerial>>strDevSN>>CustomTag>>TagDataSize;
		if(TagDataSize!=lSerial.GetLength()){
			ClientOutput(strDevSN,LOGLEVEL_ERR,"CustomTag=0x%08X Parameter Fail",CustomTag);
			Ret=ACERR_FAIL;
		}
		Rtn=LocalDoCustm(strDevSN,CustomTag,TagDataSize,lSerial.GetBuffer());
		if(Rtn==1){///没有处理
			Ret=SiteDoCustom(strDevSN,CustomTag,TagDataSize,lSerial.GetBuffer());
		}
		else{
			Ret=Rtn;
		} 

	}
	catch (...){
		
	}
	
__end:
	return Ret;
}



INT CIceServerApp::HandleSetSNStrategy(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp)
{
	INT Ret=ACERR_OK;
	CString strJson;
	CSerial lSerial(m_ServerCfg.CharSet);
	CProjInfo& GProjInfo=GetGlobalProjInfo();
	CMultiChipSN* pMultiChipSN=NULL;
	try{
		Ret=SerialCmdData(Msg,lSerial);
		if(Ret==ACERR_OK){
			lSerial>>strJson;
			pMultiChipSN=GProjInfo.GetMultChipSN();
			Ret=pMultiChipSN->LoadSNCfgFileForRemoteCtrl(strJson);
		}
		else{
			Ret=ACERR_FAIL;
		}
	}
	catch (...){
		ClientOutput("ALL",LOGLEVEL_ERR,"HandleSetSNStrategy Catch An Exception");
		Ret=ACERR_FAIL;
	}
	
	return Ret;
}

INT CIceServerApp::HandleCreateProject( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK;
	CSerial lSerial;
	ClientOutput("ALL",LOGLEVEL_ERR,"Server Handle Command=CreateProject");
	m_lCreateProjInfo.SetCharSet(m_ServerCfg.CharSet);
	m_lCreateProjInfo.ReInit();
	Ret=SerialCmdData(Msg,m_lCreateProjInfo);
	if(Ret==ACERR_OK){
		Ret= CreateProjectInWorkThread();
	}
	return Ret;
}

INT CIceServerApp::HandleSetProjectParameter( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK;
	CSerial lSerial(m_ServerCfg.CharSet);
	ClientOutput("ALL",LOGLEVEL_ERR,"Server Handle Command=SetProjectParamter");
	Ret=SerialCmdData(Msg,lSerial);
	if(Ret==ACERR_OK){
		CString PropField,PropData;
		lSerial>>PropField>>PropData;
		CProjInfo& GProjInfo=GetGlobalProjInfo();
		Ret=GProjInfo.SetProjectParamter(PropField,PropData);
	}
	return Ret;
}

INT CIceServerApp::HandleGetProjectInfoExt( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK;
	ClientOutput("ALL",LOGLEVEL_ERR,"Server Handle Command=GetProjectInofExt");
	BYTE *pDataResp;
	CSerial lSerial(m_ServerCfg.CharSet);
	CProjInfo& GProjInfo=GetGlobalProjInfo();
	Ret=GProjInfo.GetProjectInfoExt(lSerial);
	if(Ret==ACERR_OK){
		Resp.iRespDataLen=lSerial.GetLength();
		pDataResp=lSerial.GetBuffer();
		if(Resp.iRespDataLen){
			std::copy(pDataResp,pDataResp+Resp.iRespDataLen,std::back_inserter(Resp.RespData));
		}
	}
	else{
		ClientOutput("ALL",LOGLEVEL_ERR,"GetProjectInfoExt Execute Failed, Ret=%d",Ret);
	}
	return Ret;
}

INT CIceServerApp::HandleQueryStatus( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK;
	CSerial lSerial(m_ServerCfg.CharSet);
	CString DevSN;
	DEVINFO *pDevInfo;
	UINT DevCurStatus=0;
	BYTE *pDataResp=NULL;
	CDevMng&GDevMng=GetGlobalDevMng();
	Ret=SerialCmdData(Msg,lSerial);
	if(Ret==ACERR_OK){
		INT RetCmd=ACERR_OK;
		RESPMSG RespMsg;
		lSerial>>DevSN;
		if(m_ServerCfg.DemoEn==0){///实际联机状态
			pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED,DevSN);
			if(pDevInfo==NULL){
				ClientOutput("ALL",LOGLEVEL_ERR,"Can't find DevSN=%s in the connected queue",DevSN);
				Ret=ACERR_FAIL; goto __end;
			}
			RetCmd=pDevInfo->pAprogDev->DoCmd(CMD_QueryStatus,0,NULL,&RespMsg);
			if(RetCmd!=ACERR_OK){
				ClientOutput(DevSN,LOGLEVEL_ERR,"Query Status Failed");
				Ret=ACERR_FAIL;///如果失败则会退出
			}
			else{
				if(RespMsg.byErrNo==RESPERR_OK){
					if(RespMsg.dwDataSize!=4){
						Ret=ACERR_FAIL;
					}
					else{///返回当前的状态
						DevCurStatus=*(UINT*)RespMsg.pData;
						Resp.iRespDataLen=4;
						pDataResp=(BYTE*)&DevCurStatus;
						if(Resp.iRespDataLen){
							std::copy(pDataResp,pDataResp+Resp.iRespDataLen,std::back_inserter(Resp.RespData));
						}
					}
				}
				pDevInfo->pAprogDev->ReleaseRespMsg(&RespMsg);
			}
		}
		else{
			Sleep(500);///检测为500 ms响应
			DevCurStatus=0x00000100;///有Enter键被按下
			Resp.iRespDataLen=4;
			pDataResp=(BYTE*)&DevCurStatus;
			if(Resp.iRespDataLen){
				std::copy(pDataResp,pDataResp+Resp.iRespDataLen,std::back_inserter(Resp.RespData));
			}
		}
	}
	
	//ClientOutput("ALL",LOGLEVEL_ERR,"LoadProject GetCmdLen=%d,%s",Msg.iCmdDataLen,TmpBuf);
__end:
	return Ret;
}

INT CIceServerApp::HandleGetErrCode( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK,SktRealCnt=0;
	CSerial lSerial(m_ServerCfg.CharSet);
	CString strDevSN;
	DEVINFO *pDevInfo;
	UINT DevCurStatus=0;
	BYTE *pDataResp=NULL;
	CDevMng&GDevMng=GetGlobalDevMng();
	Ret=SerialCmdData(Msg,lSerial);
	if(Ret==ACERR_OK){
		CSerial TmpSerial;
		INT Rtn=0,i;
		INT RetCmd=ACERR_OK;
		lSerial>>strDevSN;
		if(m_ServerCfg.DemoEn==0){
			pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED,strDevSN);
			if(pDevInfo==NULL){
				ClientOutput("ALL",LOGLEVEL_ERR,"Can't find DevSN=%s in the connected queue",strDevSN);
				Ret=ACERR_FAIL; goto __end;
			}
			SktRealCnt=pDevInfo->DevRunTime.AdpRealNum;
			TmpSerial<<SktRealCnt;
			for(i=0;i<SktRealCnt;++i){
				TmpSerial<<pDevInfo->DevRunTime.SktErrCode[i];
			}
		}
		else{
			INT i=0,j=0;
			for(i=0;i<m_DemoCfg.SiteNum;i++){
				if(m_DemoCfg.vpVirtualSites[i]->strSN==strDevSN){
					SktRealCnt=m_DemoCfg.SocketNumPerSite;
					TmpSerial<<SktRealCnt;
					for(j=0;j<SktRealCnt;++j){
						TmpSerial<<m_DemoCfg.vpVirtualSites[i]->ErrCode[j];
					}
				}
			}
			if(i==m_DemoCfg.SiteNum){
				ClientOutput("ALL",LOGLEVEL_ERR,"Can't find DevSN=%s in the connected queue",strDevSN);
			//	Ret=ACERR_FAIL; goto __end;
			}
		}
		Resp.iRespDataLen=4+SktRealCnt*4;
		pDataResp=TmpSerial.GetBuffer();
		if(Resp.iRespDataLen){
			std::copy(pDataResp,pDataResp+Resp.iRespDataLen,std::back_inserter(Resp.RespData));
		}
	}

__end:
	return Ret;
}

void CIceServerApp::ConstructAdapterInfo_Json(CSerial& lSerial,std::map<CString,SKBINFO> mapSkbInfo)
{
	Json::StyledWriter JWriter;
	std::string strJson;
	Json::Value Root;
	Json::Value OneAdapter;
	Json::Value AdapterInfo;
	CString strTmp;

	std::map<CString,SKBINFO>::iterator it;
	for (it=mapSkbInfo.begin(); it!=mapSkbInfo.end(); ++it){

		strTmp.Format("%04X", it->second.ID);
		OneAdapter["ID"]=Json::Value(strTmp);
		OneAdapter["SN"]=Json::Value(it->first);
		OneAdapter["Num"]=Json::Value(it->second.skt_num);
		strTmp.Format("%s",it->second.serial_str);
		OneAdapter["Serial"]=Json::Value(strTmp);
		OneAdapter["Version"]=Json::Value(it->second.skb_ver);
		OneAdapter["Limit"]=Json::Value(it->second.lmit_cnt);
		OneAdapter["Pass"]=Json::Value(it->second.pass_cnt);
		strTmp.Format("%02d%02d%02d", it->second.date[0], it->second.date[1], it->second.date[2]);
		OneAdapter["Date"]=Json::Value(strTmp);
		OneAdapter["Fail"]=Json::Value(it->second.SKTDetails.FailCnt);
		OneAdapter["LicFlag"]=Json::Value(it->second.SKTDetails.LicFlag);
		AdapterInfo.append(OneAdapter);

	}
	Root["AdapterInfo"]=AdapterInfo;

	strJson=JWriter.write(Root);
	lSerial.SerialInBuff((BYTE*)strJson.c_str(),strJson.length()+1);
}

INT CIceServerApp::HandleGetAdapterInfo(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK;
	CSerial lSerial(m_ServerCfg.CharSet);
	CString DevSN;
	DEVINFO *pDevInfo;
	UINT DevCurStatus=0;
	BYTE *pDataResp=NULL;
	CDevMng&GDevMng=GetGlobalDevMng();
	std::map<CString,SKBINFO> mapSkbInfo;
	if(Ret==ACERR_OK){
		INT RetCmd=ACERR_OK;

		if(m_ServerCfg.DemoEn==0){///实际联机状态
			INT SiteCnt=GDevMng.GetDevCnt(CDevMng::GROUP_ATTACNED);
			for(INT i = 0;i<SiteCnt;i++){
				pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED, i);
				if(pDevInfo==NULL){
					ClientOutput("ALL",LOGLEVEL_ERR,"Can't find devInfo in the connected queue");
					Ret=ACERR_FAIL; goto __end;
				}
				RESPMSG RespMsg;
				RetCmd=pDevInfo->pAprogDev->DoCmd(CMD_QuerySocket,0,NULL,&RespMsg);
				if(RetCmd!=ACERR_OK){
					ClientOutput(DevSN,LOGLEVEL_ERR,"Query Adapter Info Failed");
					Ret=ACERR_FAIL;///如果失败则会退出
				}
				else{
					if(RespMsg.byErrNo==RESPERR_OK){
						SKBINFO Skb;
						if(RespMsg.dwDataSize!=sizeof(SKBINFO)){
							Ret=ACERR_FAIL;
						}
						else{///返回当前的状态

							SKBINFO Skb;
							memcpy((char*)&Skb, RespMsg.pData, RespMsg.dwDataSize);
							mapSkbInfo[pDevInfo->DevSN] = Skb;
							
						}
					}
					pDevInfo->pAprogDev->ReleaseRespMsg(&RespMsg);
				}
			}


			CSerial lSerial;
			ConstructAdapterInfo_Json(lSerial, mapSkbInfo);

			Resp.iRespDataLen=lSerial.GetLength();
			pDataResp=lSerial.GetBuffer();
			if(Resp.iRespDataLen){
				std::copy(pDataResp,pDataResp+Resp.iRespDataLen,std::back_inserter(Resp.RespData));
			}
		}
	}
__end:
	return Ret;
}

INT CIceServerApp::HandleGetSktCounter( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK;
	CSerial lSerial(m_ServerCfg.CharSet);
	CString strDevSN;
	DEVINFO *pDevInfo;
	UINT DevCurStatus=0;
	BYTE *pDataResp=NULL;
	CDevMng&GDevMng=GetGlobalDevMng();
	Ret=SerialCmdData(Msg,lSerial);
	if(Ret==ACERR_OK){
		INT Rtn=0;
		//RESPMSG RespMsg;
		INT RetCmd=ACERR_OK;
		UINT CustomTag=TAG_GETALLSKBINFO;
		lSerial>>strDevSN;
		Rtn=LocalDoCustm(strDevSN,CustomTag,0,NULL);
		if(Rtn==1){///没有处理
			INT i;
			pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED,strDevSN);
			pDevInfo->bTransRmtCmd2Local=TRUE;
			Ret=SiteDoCustom(strDevSN,CustomTag,0,NULL);
			if(Ret==ACERR_OK){
				CSerial lSerial;
				Resp.iRespDataLen=pDevInfo->SktInfoRealCnt*8+4;
				lSerial<<pDevInfo->SktInfoRealCnt;
				for(i=0;i<(INT)pDevInfo->SktInfoRealCnt;++i){
					lSerial<<pDevInfo->SktInfo[i].CurCnt<<pDevInfo->SktInfo[i].LimitCnt;
				}
				pDataResp=lSerial.GetBuffer();
				if(Resp.iRespDataLen){
					std::copy(pDataResp,pDataResp+Resp.iRespDataLen,std::back_inserter(Resp.RespData));
				}
			}
		}
		else{
			Ret=Rtn;
		} 
	}

	//ClientOutput("ALL",LOGLEVEL_ERR,"LoadProject GetCmdLen=%d,%s",Msg.iCmdDataLen,TmpBuf);
//__end:
	return Ret;
}

INT CIceServerApp::ConstructSktInfoExt_Json(CSerial& lSerial,DEVINFO*pDevInfo)
{
	Json::StyledWriter JWriter;
	std::string strJson;
	Json::Value Root;
	Json::Value SktInfo;
	Json::Value OneSkt;
	INT i;
	Root["AdapterCnt"]=Json::Value(pDevInfo->SktInfoRealCnt);
	for(i=0;i<pDevInfo->SktInfoRealCnt;++i){
		CString strID;
		strID.Format("%016I64X",pDevInfo->SktInfo[i].UID);
		OneSkt["Index"]=Json::Value(i+1);
		OneSkt["ID"]=Json::Value(strID);
		OneSkt["CurrentCnt"]=Json::Value((INT)pDevInfo->SktInfo[i].CurCnt);
		OneSkt["LimitedCnt"]=Json::Value((INT)pDevInfo->SktInfo[i].LimitCnt);
		OneSkt["FailCnt"]=Json::Value((INT)pDevInfo->SktInfo[i].FailCnt);
		OneSkt["LicenseFlag"]=Json::Value(pDevInfo->SktInfo[i].LicenseFlag);
		SktInfo.append(OneSkt);
	}
	Root["SktInfo"]=SktInfo;
	strJson=JWriter.write(Root);
	lSerial.SerialInBuff((BYTE*)strJson.c_str(),strJson.length()+1);
	return 0;
}




INT CIceServerApp::HandleGetSktInfoJson( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK;
	CSerial lSerial(m_ServerCfg.CharSet);

	CSerial lSerialSum;
	std::string strJson;
	Json::StyledWriter JWriter;
	Json::Value Root;
	CString strDevSN;
	CString strMsg;

	DEVINFO *pDevInfo;
	UINT DevCurStatus=0;
	BYTE *pDataResp=NULL;
	CDevMng&GDevMng=GetGlobalDevMng();
	CProjInfo& GProjInfo=GetGlobalProjInfo();
	//Ret=SerialCmdData(Msg,lSerial);

	if(m_ServerCfg.DemoEn!=0){///实际联机状态
		goto __end;
	}
	if(Ret==ACERR_OK){
		INT Rtn=0;
		///////////////////////////////////////////////////
		INT SiteCnt=GDevMng.GetDevCnt(CDevMng::GROUP_ATTACNED);

		for(INT j = 0;j<SiteCnt;j++){
			pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED, j);
			if(pDevInfo==NULL){
				ClientOutput("ALL",LOGLEVEL_ERR,"Can't find devInfo in the connected queue");
				Ret=ACERR_FAIL; goto __end;
			}
			
			//////////////////////////////////////////////////
			//RESPMSG RespMsg;
			INT RetCmd=ACERR_OK;
			UINT CustomTag=TAG_GET_ALLSKBSMPINFO_EXT;

			strDevSN.Format(pDevInfo->DevSN);
			Rtn=LocalDoCustm(strDevSN,CustomTag,0,NULL);

			if(Rtn==1){///没有处理
				pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED,strDevSN);
				pDevInfo->bTransRmtCmd2Local=TRUE;
				Ret=SiteDoCustom(strDevSN,CustomTag,0,NULL);
				SiteDoCustom(strDevSN,TAG_GETALLSKBUID,0,NULL);
				if(Ret!=ACERR_OK){
					GDevMng.PrintLog(LOGLEVEL_LOG,"SiteDoCustom Fail, Ret:%d",Ret);
					Ret=ACERR_FAIL; goto __end;
				}
			}
			else{
				GDevMng.PrintLog(LOGLEVEL_LOG,"LocalDoCustm");
				Ret=Rtn;
			} 

			Json::Value OneSite;
			OneSite["SiteSN"]=Json::Value(strDevSN);
			OneSite["AdapterCnt"]=Json::Value(pDevInfo->SktInfoRealCnt);
			
			for( int i=0;i<pDevInfo->SktInfoRealCnt;++i){
				Json::Value EachSkt;
				
				CString strID;
				strID.Format("%016I64X",pDevInfo->SktInfo[i].UID);
				EachSkt["Index"]=Json::Value(i+1);
				EachSkt["SN"]=Json::Value(strID);
				EachSkt["ID"]=Json::Value(pDevInfo->SktInfoExt[i].SKTID);
				EachSkt["CurrentCnt"]=Json::Value((INT)pDevInfo->SktInfo[i].CurCnt);
				EachSkt["LimitedCnt"]=Json::Value((INT)pDevInfo->SktInfo[i].LimitCnt);
				EachSkt["FailCnt"]=Json::Value((INT)pDevInfo->SktInfo[i].FailCnt);
				EachSkt["LicenseFlag"]=Json::Value(pDevInfo->SktInfo[i].LicenseFlag);
				OneSite["SlotInfo"].append(EachSkt);
			}

			Root["SiteInfo"].append(OneSite);
		}
		Root["PrjSktId"] = Json::Value(GProjInfo.GetAdpIDForSdk());
		///////////////////////////////////////////
		strJson=JWriter.write(Root);
		lSerialSum<<strJson;
		/*strMsg.Format("=================>%s", strJson.c_str());
		OutputDebugString(strMsg);*/
		Resp.iRespDataLen=lSerialSum.GetLength();
		pDataResp=lSerialSum.GetBuffer();
		if(Resp.iRespDataLen){
			std::copy(pDataResp,pDataResp+Resp.iRespDataLen,std::back_inserter(Resp.RespData));
		}
		///////////////////////////////////////////////////
	}

__end:
	return Ret;
}


INT CIceServerApp::HandleGetSktCounterExt( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK;
	CSerial lSerial(m_ServerCfg.CharSet);
	CString strDevSN;
	DEVINFO *pDevInfo;
	UINT DevCurStatus=0;
	BYTE *pDataResp=NULL;
	CDevMng&GDevMng=GetGlobalDevMng();
	Ret=SerialCmdData(Msg,lSerial);
	if(Ret==ACERR_OK){
		INT Rtn=0;
		//RESPMSG RespMsg;
		INT RetCmd=ACERR_OK;
		UINT CustomTag=TAG_GET_ALLSKBSMPINFO_EXT;
		lSerial>>strDevSN;
		Rtn=LocalDoCustm(strDevSN,CustomTag,0,NULL);
		if(Rtn==1){///没有处理
			INT i;
			pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED,strDevSN);
			pDevInfo->bTransRmtCmd2Local=TRUE;
			Ret=SiteDoCustom(strDevSN,CustomTag,0,NULL);
			SiteDoCustom(strDevSN,TAG_GETALLSKBUID,0,NULL);
			if(Ret==ACERR_OK){
				CSerial lSerial;
				ConstructSktInfoExt_Json(lSerial,pDevInfo);
				Resp.iRespDataLen=lSerial.GetLength();
				pDataResp=lSerial.GetBuffer();
				if(Resp.iRespDataLen){
					std::copy(pDataResp,pDataResp+Resp.iRespDataLen,std::back_inserter(Resp.RespData));
				}
			}
		}
		else{
			Ret=Rtn;
		} 
	}

	//ClientOutput("ALL",LOGLEVEL_ERR,"LoadProject GetCmdLen=%d,%s",Msg.iCmdDataLen,TmpBuf);
	//__end:
	return Ret;
}


INT CIceServerApp::ClientSktUIDFecthed(CString strJson)
{
	INT Ret=0;
	CSerial lSerial;
	RemoteAprog::tagMSG ClientMsg;

	lSerial<<strJson;
	ClientMsg.iCmd=0;
	ClientMsg.iCmdDataLen = lSerial.GetLength();
	std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+ClientMsg.iCmdDataLen,std::back_inserter(ClientMsg.CmdData));
	
	EnterCriticalSection(&m_csClient);
	try{
		m_ClientApp->ClientDoCmd("SktUIDFetched",  ClientMsg);
	}
	catch (...){
	}	
	LeaveCriticalSection(&m_csClient);
	return Ret;
}


INT CIceServerApp::ClientUIDFetched(CString strJson){
	INT Ret=0;
	CSerial lSerial;
	RemoteAprog::tagMSG ClientMsg;

	lSerial<<strJson;
	ClientMsg.iCmd=0;
	ClientMsg.iCmdDataLen = lSerial.GetLength();
	std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+ClientMsg.iCmdDataLen,std::back_inserter(ClientMsg.CmdData));

	EnterCriticalSection(&m_csClient);
	try{
		m_ClientApp->ClientDoCmd("UIDFetched",  ClientMsg);
	}
	catch (...){
	}	
	LeaveCriticalSection(&m_csClient);
	return Ret;
}

/************
请求服务器执行StatusChange设置
TagPara域中成员组成
参数	类型	长度	描述
AdpCnt	    字节	1	当前的Socket个数，决定下面AdpNStatus的组数，总字节数为1+64*AdpCnt
AdpNStatus	字符串	64个字节	每个Socket状态最多占用63个字节，不需要的字节设置为0


*********************/
INT CIceServerApp::ClientStatusChange(CString strDevSN,BYTE*pData,INT Size)
{
	Json::StyledWriter Writer;
	Json::Value Root;
	Json::Value SKTStatus;
	CSerial lSerial;
	std::string strJson;
	CString strCstJson;
	CSerial ltSerial(m_ServerCfg.CharSet);
	RemoteAprog::tagMSG ClientMsg;
	CHAR*pTmpBuf=NULL;
	INT TmpBufSize=1024,SizeUsed;
	UINT CharSet=m_ServerCfg.CharSet;
	INT Ret=0,i;
	pTmpBuf=new CHAR[TmpBufSize];
	if(!pTmpBuf){
		Ret=-1; goto __end;
	}
	Root["SiteSN"]=Json::Value(strDevSN);
	try{
		BYTE AdpCnt;
		BYTE strStatus[64];
		BYTE TmpBuf[256];
		DWORD SizeUsed;
		lSerial.SerialInBuff(pData,Size);
		lSerial>>AdpCnt;
		CString strAdpStatus;
		for(i=0;i<(INT)AdpCnt;++i){
			CString strTmp;
			lSerial.SerialOutBuff(strStatus,64);
			if(strStatus[0]==0){
				strTmp.Format("No Message");
			}
			else{
				strTmp.Format("%s",strStatus);
			}
			SKTStatus["SKT"]=Json::Value(i+1);
			SKTStatus["Status"]=Json::Value(strTmp);
			Root["SKTStatus"].append(SKTStatus);
		}
		Ret=1;
	}
	catch (...){
		Ret=-1; goto __end;
	}

	strJson=Writer.write(Root);
	strCstJson.Format("%s",strJson.c_str());


	ltSerial<<strCstJson;
	ClientMsg.iCmd=0;
	ClientMsg.iCmdDataLen = ltSerial.GetLength();
	std::copy(ltSerial.GetBuffer(),ltSerial.GetBuffer()+ClientMsg.iCmdDataLen,std::back_inserter(ClientMsg.CmdData));
	EnterCriticalSection(&m_csClient);
	try{
		m_ClientApp->ClientDoCmd("StatusChange",ClientMsg);
	}
	catch (...){
	}	
	LeaveCriticalSection(&m_csClient);

__end:
	if(pTmpBuf){
		delete[] pTmpBuf;
	}
	return Ret;
}

void CIceServerApp::ClientSetProgress( CString strDevSN,UINT vCur,UINT Total )
{
	CSerial lSerial(m_ServerCfg.CharSet);
	RemoteAprog::tagMSG ClientMsg;
	if(vCur>Total || Total==0){
		return;
	}
	lSerial<<strDevSN<<vCur<<Total;
	ClientMsg.iCmd=0;
	ClientMsg.iCmdDataLen = lSerial.GetLength();
	std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+ClientMsg.iCmdDataLen,std::back_inserter(ClientMsg.CmdData));
	EnterCriticalSection(&m_csClient);
	try{
		m_ClientApp->ClientDoCmd("SetProgress",ClientMsg);
	}
	catch (...){
	}	
	LeaveCriticalSection(&m_csClient);

}
void CIceServerApp::ClientOutput( CString strDevSN,UINT LogLevel,const char*fmt,... )
{
	int ret=0;
	char sprint_buf[1024]; 
	int strSize=0,offset=0;
	CSerial lSerial(m_ServerCfg.CharSet);
	CString strOutput;
	BYTE Level=(BYTE)LogLevel;
	va_list args;
	va_start(args,fmt);  
	_vsnprintf(sprint_buf+offset,1024,fmt,args); 
	va_end(args); /* 将argp置为NULL */
	strSize=(INT)strlen(sprint_buf);
	if(strSize>1024-1){
		return;
	}
	strOutput.Format("%s",sprint_buf);
	RemoteAprog::tagMSG ClientMsg;
	lSerial<<Level<<strDevSN<<strOutput;
	ClientMsg.iCmd=0;
	ClientMsg.iCmdDataLen = lSerial.GetLength();
	std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+ClientMsg.iCmdDataLen,std::back_inserter(ClientMsg.CmdData));
	EnterCriticalSection(&m_csClient);
	try{
		m_ClientApp->ClientDoCmd("Output",ClientMsg);
		//OutputDebugString(sprint_buf); 去掉这个Debug输出，意义不大
	}
	catch (...){
	}
	LeaveCriticalSection(&m_csClient);
}

void CIceServerApp::ClientSetChecksumExt_Json(CString strChecksumExt_Json)
{
	CSerial lSerial(m_ServerCfg.CharSet);
	RemoteAprog::tagMSG ClientMsg;
	lSerial<<strChecksumExt_Json;
	ClientMsg.iCmd=0;
	ClientMsg.iCmdDataLen = lSerial.GetLength();
	std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+ClientMsg.iCmdDataLen,std::back_inserter(ClientMsg.CmdData));
	EnterCriticalSection(&m_csClient);
	try{
		m_ClientApp->ClientDoCmd("SetChecksumExt_Json",ClientMsg);
	}
	catch (...){
	}

	LeaveCriticalSection(&m_csClient);
}

void CIceServerApp:: ClienteMMCOption_Json(CString streMMCOption_Json)
{
	CSerial lSerial(m_ServerCfg.CharSet);
	RemoteAprog::tagMSG ClientMsg;
	lSerial<<streMMCOption_Json;
	ClientMsg.iCmd=0;
	ClientMsg.iCmdDataLen = lSerial.GetLength();
	std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+ClientMsg.iCmdDataLen,std::back_inserter(ClientMsg.CmdData));
	EnterCriticalSection(&m_csClient);
	try{
		m_ClientApp->ClientDoCmd("eMMCOption_Json",ClientMsg);
	}
	catch (...){
	}

	LeaveCriticalSection(&m_csClient);
}

void CIceServerApp::ClientFileImportInfo_Json(CString strFileImportInfo_Json)
{
	CSerial lSerial(m_ServerCfg.CharSet);
	RemoteAprog::tagMSG ClientMsg;
	lSerial<<strFileImportInfo_Json;
	ClientMsg.iCmd=0;
	ClientMsg.iCmdDataLen = lSerial.GetLength();
	std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+ClientMsg.iCmdDataLen,std::back_inserter(ClientMsg.CmdData));
	EnterCriticalSection(&m_csClient);
	try{
		m_ClientApp->ClientDoCmd("FileImportInfo_Json",ClientMsg);
	}
	catch (...){
	}

	LeaveCriticalSection(&m_csClient);
}

void CIceServerApp::ClientSetLoadResult(CString strDevSN,INT Result)
{
	CSerial lSerial(m_ServerCfg.CharSet);
	RemoteAprog::tagMSG ClientMsg;
	lSerial<<strDevSN<<Result;
	ClientMsg.iCmd=0;
	ClientMsg.iCmdDataLen = lSerial.GetLength();
	std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+ClientMsg.iCmdDataLen,std::back_inserter(ClientMsg.CmdData));
	EnterCriticalSection(&m_csClient);
	try{
		m_ClientApp->ClientDoCmd("SetLoadResult",ClientMsg);
	}
	catch (...){
	}
	
	LeaveCriticalSection(&m_csClient);
}

void CIceServerApp::ClientSetMissionResult(CString strJson)
{
	CSerial lSerial;
	RemoteAprog::tagMSG ClientMsg;

	cJSON* pRoot = NULL;
	CString strBuildJson;
	CString strTemp;
	strTemp.Format("over");

	pRoot = cJSON_CreateObject();
	 cJSON_AddStringToObject(pRoot, "CurrMission",(LPSTR)(LPCSTR)strTemp);

	 strBuildJson.Format("%s", cJSON_Print(pRoot));
	 lSerial.SerialInBuff((BYTE*)strBuildJson.GetBuffer(), strBuildJson.GetLength());

	 if (pRoot){
		 cJSON_Delete(pRoot);
		 pRoot = NULL;
	 }

	ClientMsg.iCmd=0;
	ClientMsg.iCmdDataLen = lSerial.GetLength();
	std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+ClientMsg.iCmdDataLen,std::back_inserter(ClientMsg.CmdData));
	EnterCriticalSection(&m_csClient);
	try{
		m_ClientApp->ClientDoCmd("SetMissionResult",ClientMsg);
	}
	catch (...){
	}
	LeaveCriticalSection(&m_csClient);

}

void CIceServerApp::ClientSetPrintContent(INT SiteIdx, INT SktIdx, BOOL bSwap, CString Manufactor, CString Eid, CString Sn, INT Result)
{
	CSerial lSerial;
	RemoteAprog::tagMSG ClientMsg;

	cJSON* pRoot = NULL;
	CString strBuildJson;

	pRoot = cJSON_CreateObject();
	cJSON_AddNumberToObject(pRoot, "SiteIdx", SiteIdx);
	cJSON_AddNumberToObject(pRoot, "ContentEn", 1);
	cJSON_AddStringToObject(pRoot, "Manufactor", Manufactor);
	cJSON_AddNumberToObject(pRoot, "Swap", bSwap?1:0);
	cJSON *pSNInfo = cJSON_AddArrayToObject(pRoot, "Content");
	for(UINT i=0; i<1;i++){
		cJSON* pContent = cJSON_CreateObject();
		cJSON_AddNumberToObject(pContent, "SKTIdx", SktIdx);
		cJSON_AddStringToObject(pContent, "Uid",(LPSTR)(LPCSTR)Eid);
		cJSON_AddStringToObject(pContent, "Text",(LPSTR)(LPCSTR)Sn);
		cJSON_AddNumberToObject(pContent, "Valid", Result);
		cJSON_AddItemToArray(pSNInfo, pContent);

	}

	strBuildJson.Format("%s", cJSON_Print(pRoot));
	lSerial.SerialInBuff((BYTE*)strBuildJson.GetBuffer(), strBuildJson.GetLength());

	if (pRoot){
		cJSON_Delete(pRoot);
		pRoot = NULL;
	}

	ClientMsg.iCmd=0;
	ClientMsg.iCmdDataLen = lSerial.GetLength();
	std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+ClientMsg.iCmdDataLen,std::back_inserter(ClientMsg.CmdData));
	EnterCriticalSection(&m_csClient);
	try{
		m_ClientApp->ClientDoCmd("SetPrintContent",ClientMsg);
	}
	catch (...){
	}
	LeaveCriticalSection(&m_csClient);

}

void CIceServerApp::ClientSetJobResultExt_Json(CString strDevSN, CString strCmd,UINT AdpCnt, UINT AdpStatus,UINT PassCntTotal,UINT FailCntTotal)
{
	CSerial lSerial/*(m_ServerCfg.CharSet)*/;
	RemoteAprog::tagMSG ClientMsg;
	if(AdpCnt==0){
		return;
	}

	CString strIniFile=CComFunc::GetCurrentPath()+"/MultiCfg.ini";
	int nErrCodeSwitch=GetPrivateProfileInt("Project","ErrCodeSwitch",0,strIniFile);
	//lSerial<<strDevSN<<strCmd<<AdpCnt<<AdpStatus<<PassCntTotal<<FailCntTotal;
	/////////////////////////////////获取fail的信息/////////////////////////////////////////因为没有做其它的Docmd所以这里不影响
	 RemoteAprog::tagMSG BuildCmdMsg; 
	 RemoteAprog::tagRESP cmdResp;
	 CSerial BuildCmdlSerial;
	 BYTE* pRespData = NULL; 
	 CSerial RespSerial;

	 cJSON* pRoot = NULL;
	 CString strJson;
	 CString strTemp;

	 pRoot = cJSON_CreateObject();
	 if (pRoot == NULL){
		 ClientOutput("ALL",LOGLEVEL_ERR,"build json file failed");
		 return; 
	 }

	 cJSON_AddStringToObject(pRoot, "DevSN",(LPSTR)(LPCSTR)strDevSN); 
	 cJSON_AddStringToObject(pRoot, "strCmd",(LPSTR)(LPCSTR)strCmd);
	 cJSON_AddNumberToObject(pRoot, "AdpCnt", AdpCnt);
	 strTemp.Format("0x%X", AdpStatus);
	 cJSON_AddStringToObject(pRoot, "AdpResult",(LPSTR)(LPCSTR)strTemp);
	 cJSON_AddNumberToObject(pRoot, "PassCnt",PassCntTotal);
	 cJSON_AddNumberToObject(pRoot, "FailCnt",FailCntTotal);

	 cJSON* pArr=cJSON_AddArrayToObject(pRoot, "FailMsg");
	
	BuildCmdlSerial<<strDevSN;
	BuildCmdMsg.iCmd = 0;
	BuildCmdMsg.iCmdDataLen =BuildCmdlSerial.GetLength();
	std::copy(BuildCmdlSerial.GetBuffer(),BuildCmdlSerial.GetBuffer()+BuildCmdMsg.iCmdDataLen, std::back_inserter(BuildCmdMsg.CmdData));
	if (HandleGetErrCode(BuildCmdMsg, cmdResp) != 0){
		ClientOutput("ALL",LOGLEVEL_ERR,"GetErrCode failed");
	}

	if(cmdResp.iRespDataLen > 0){		
		pRespData=new BYTE[cmdResp.iRespDataLen];
		if(pRespData!=NULL){
			std::copy(cmdResp.RespData.begin(), cmdResp.RespData.end(), pRespData);
			RespSerial.SerialInBuff(pRespData, cmdResp.iRespDataLen);

			INT SktCnt,i;
			UINT ErrCode;
			RespSerial>>SktCnt;

			for(i=0;i<SktCnt;i++){
				CString strTmp;
				RespSerial>>ErrCode;

				//如果AdpStatus提示有错误，而ErrCode正常，以AdpStatus为准。
				//这种情况发生在连接失败的时候，没有收到SktErrCode的更新。
				const UINT ResultCodeError = 0x03;
				const UINT ResultCodeInit = 0x0;
				UINT iSktResult = (AdpStatus>>(2*i)) & ResultCodeError;
				if (iSktResult == ResultCodeError && ErrCode == 0x4000){
					ErrCode = 0x2010;
				}else if (iSktResult == ResultCodeInit && ErrCode != ResultCodeInit){
					ErrCode = 0x0;
				}
				
				if ( (ErrCode == 0x207B) ||  (ErrCode ==0x20AF)  || (ErrCode == 0x2018)  ||
					(ErrCode ==  0x208C) || (ErrCode == 0x208F) ||  (ErrCode ==  0x20F0) ){
						ClientOutput("ALL",LOGLEVEL_WARNING,"Current real errcode = 0x%04X", ErrCode);
						if (nErrCodeSwitch == 1){
							ErrCode = E_NoDevInSocket;
						}	
				}

				strTmp.Format("0x%04X", ErrCode);

				cJSON *element = cJSON_CreateObject();
				cJSON_AddNumberToObject(element, "SKTIdx", i+1);
				cJSON_AddStringToObject(element, "ErrCode", (LPSTR)(LPCSTR)strTmp); 
				//
				strTmp.Format("%s", GetErrMsg(ErrCode));
				if (ErrCode == 0x4000){
					strTmp.Format("Do cmd pass");
				}
				
				cJSON_AddStringToObject(element, "ErrMsg", (LPSTR)(LPCSTR)strTmp); 
				cJSON_AddItemToArray(pArr, element);
			}	
			strJson.Format("%s", cJSON_Print(pRoot));
			lSerial.SerialInBuff((BYTE*)strJson.GetBuffer(), strJson.GetLength());
			ClientOutput("ALL",LOGLEVEL_WARNING,strJson);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////
	ClientMsg.iCmd=0;
	ClientMsg.iCmdDataLen = lSerial.GetLength();
	std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+ClientMsg.iCmdDataLen,std::back_inserter(ClientMsg.CmdData));
	EnterCriticalSection(&m_csClient);
	try{
		m_ClientApp->ClientDoCmd("SetJobResultExt_Json",ClientMsg);
	}
	catch (...){
	}
	LeaveCriticalSection(&m_csClient);

	if (pRespData){
		delete [] pRespData;
		pRespData = NULL;
	}

	if (pRoot){
		cJSON_Delete(pRoot);
	}
}

void CIceServerApp::ClientSetJobResultExt(CString strDevSN, CString strCmd,UINT AdpCnt, UINT AdpStatus,UINT PassCntTotal,UINT FailCntTotal)
{
	CSerial lSerial(m_ServerCfg.CharSet);
	RemoteAprog::tagMSG ClientMsg;
	if(AdpCnt==0){
		return;
	}
	lSerial<<strDevSN<<strCmd<<AdpCnt<<AdpStatus<<PassCntTotal<<FailCntTotal;
	ClientMsg.iCmd=0;
	ClientMsg.iCmdDataLen = lSerial.GetLength();
	std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+ClientMsg.iCmdDataLen,std::back_inserter(ClientMsg.CmdData));
	EnterCriticalSection(&m_csClient);
	try{
		m_ClientApp->ClientDoCmd("SetJobResultExt",ClientMsg);
	}
	catch (...){
	}
	LeaveCriticalSection(&m_csClient);
}

void CIceServerApp::ClientSetJobResult(CString strDevSN, CString strCmd,UINT AdpCnt, UINT AdpStatus)
{
	CSerial lSerial(m_ServerCfg.CharSet);
	RemoteAprog::tagMSG ClientMsg;
	if(AdpCnt==0){
		return;
	}
	lSerial<<strDevSN<<strCmd<<AdpCnt<<AdpStatus;
	ClientMsg.iCmd=0;
	ClientMsg.iCmdDataLen = lSerial.GetLength();
	std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+ClientMsg.iCmdDataLen,std::back_inserter(ClientMsg.CmdData));
	EnterCriticalSection(&m_csClient);
	try{
		m_ClientApp->ClientDoCmd("SetJobResult",ClientMsg);
	}
	catch (...){
	}
	LeaveCriticalSection(&m_csClient);
}

void CIceServerApp::ClientSetJobStart(CString strDevSN, CString strCmd,UINT AdpCnt)
{
	CSerial lSerial(m_ServerCfg.CharSet);
	RemoteAprog::tagMSG ClientMsg;
	OutputDebugString("=====================>567==============");
	if(AdpCnt==0){
	//	return;
	}
		OutputDebugString("=====================>789==============");
	lSerial<<strDevSN<<strCmd<<AdpCnt;
	ClientMsg.iCmd=0;
	ClientMsg.iCmdDataLen = lSerial.GetLength();
	std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+ClientMsg.iCmdDataLen,std::back_inserter(ClientMsg.CmdData));
	EnterCriticalSection(&m_csClient);
	try{
		m_ClientApp->ClientDoCmd("SetJobStart",ClientMsg);
	}
	catch (...){
	}
	LeaveCriticalSection(&m_csClient);
}

void CIceServerApp::ClientSetInitResult(CSerial&lSerial)
{
	RemoteAprog::tagMSG ClientMsg;
	ClientMsg.iCmd=0;
	ClientMsg.iCmdDataLen = lSerial.GetLength();
	std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+ClientMsg.iCmdDataLen,std::back_inserter(ClientMsg.CmdData));
	EnterCriticalSection(&m_csClient);
	try{
		m_ClientApp->ClientDoCmd("SetInitResult",ClientMsg);
	}
	catch (...){
	}
	LeaveCriticalSection(&m_csClient);
}

void CIceServerApp::ClientSetStatus( CString strDevSN,UINT AdpCnt,UINT AdpStatus )
{
	CSerial lSerial(m_ServerCfg.CharSet);
	RemoteAprog::tagMSG ClientMsg;
	if(AdpCnt==0){
		return;
	}
	lSerial<<strDevSN<<AdpCnt<<AdpStatus;
	ClientMsg.iCmd=0;
	ClientMsg.iCmdDataLen = lSerial.GetLength();
	std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+ClientMsg.iCmdDataLen,std::back_inserter(ClientMsg.CmdData));
	EnterCriticalSection(&m_csClient);
	try{
		m_ClientApp->ClientDoCmd("SetStatus",ClientMsg);
	}
	catch (...){
	}
	LeaveCriticalSection(&m_csClient);
}

INT CIceServerApp::ClientDoCustom( CString strDevSN,UINT TagID,BYTE*pData,UINT DataSize,BYTE**pRespData,INT*pRespDataSize)
{
	CSerial lSerial(m_ServerCfg.CharSet);
	INT Rtn=ACERR_OK;
	RemoteAprog::tagMSG ClientMsg;
	RemoteAprog::tagRESP Resp;	
	lSerial<<strDevSN<<TagID<<DataSize;
	if(pData){
		lSerial.SerialInBuff(pData,DataSize);
	}
	ClientMsg.iCmd=0;
	ClientMsg.iCmdDataLen = lSerial.GetLength();
	std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+ClientMsg.iCmdDataLen,std::back_inserter(ClientMsg.CmdData));
	EnterCriticalSection(&m_csClient);
	try{
		Resp=m_ClientApp->ClientDoCmd("DoCustom",ClientMsg);
	}
	catch (...){
		Resp.iCmd=-1;
		Resp.iRespDataLen=0;
	}
	LeaveCriticalSection(&m_csClient);
	if(Resp.iRespDataLen){
		if(pRespDataSize==NULL || pRespData==NULL){
			Rtn=ACERR_FAIL;
			CLogMsg::PrintDebugString("客户端执行自定义命令有返回值，但是函数调用并未提供数据存放区域\n");
			goto __end;
		}
		else{
			*pRespDataSize=Resp.iRespDataLen;
		}
		*pRespData=new BYTE[Resp.iRespDataLen];
		if(*pRespData==NULL){
			Rtn=ACERR_MEMALLOC;
			CLogMsg::PrintDebugString("客户端执行自定义命令有返回值，但是分配内存失败，大小为%d\n",Resp.iRespDataLen);
			goto __end;
		}
		std::copy(Resp.RespData.begin(),Resp.RespData.end(),*pRespData);
	}

	Rtn=Resp.iCmd;
__end:

	return Rtn;
}

void CIceServerApp::ClientSetCreateProjResult(CString strDevSN,INT Result)
{
	CSerial lSerial(m_ServerCfg.CharSet);
	RemoteAprog::tagMSG ClientMsg;
	lSerial<<strDevSN<<Result;
	ClientMsg.iCmd=0;
	ClientMsg.iCmdDataLen = lSerial.GetLength();
	std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+ClientMsg.iCmdDataLen,std::back_inserter(ClientMsg.CmdData));
	EnterCriticalSection(&m_csClient);
	try{
		m_ClientApp->ClientDoCmd("SetCreateProjResult",ClientMsg);
	}
	catch (...){
	}
	LeaveCriticalSection(&m_csClient);
}

void CIceServerApp::ClientSetCreateProjResultJson(CString strResultJson)
{
	CSerial lSerial(m_ServerCfg.CharSet);
	RemoteAprog::tagMSG ClientMsg;

	lSerial.SerialInBuff((BYTE*)strResultJson.GetBuffer(), strResultJson.GetLength());

	ClientMsg.iCmd=0;
	ClientMsg.iCmdDataLen = lSerial.GetLength();
	std::copy(lSerial.GetBuffer(),lSerial.GetBuffer()+ClientMsg.iCmdDataLen,std::back_inserter(ClientMsg.CmdData));
	EnterCriticalSection(&m_csClient);
	try{
		m_ClientApp->ClientDoCmd("SetCreateProjResultJson",ClientMsg);
	}
	catch (...){
	}
	LeaveCriticalSection(&m_csClient);
}

INT CIceServerApp::HandleSetPrintResult( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp)
{
	INT Ret = 0;
	CSerial lSerial(m_ServerCfg.CharSet);
	if(Msg.iCmdDataLen==0){
		Ret=ACERR_PARA;
		return Ret;
	}

	BYTE* TmpBuf=new BYTE[Msg.iCmdDataLen];
	if(TmpBuf==NULL){
		Ret=ACERR_MEMALLOC;
		return Ret;
	}
	std::copy(Msg.CmdData.begin(),Msg.CmdData.end(),TmpBuf);
	lSerial.SerialInBuff(TmpBuf,Msg.iCmdDataLen);
	SAFEDELARRAY(TmpBuf);

	ClientOutput("ALL", LOGLEVEL_ERR, "Server Handle Command=SetPrintResult");

	CString str_json = lSerial.GetBuffer();
	if(!str_json.IsEmpty()){
		Json::Reader json_reader;
		Json::Value ret_json;
		if(json_reader.parse(str_json.GetString(), ret_json)){
			int auto_ret = ret_json["Result"].asInt();
			std::string str_ret;
			switch(auto_ret){
			case 0:
				str_ret = "M";
				break;
			case 1:
				str_ret = "G";
				break;
			case 2:
				str_ret = "F";
				break;
			case 3:
				str_ret = "L";
				break;
			default:
				str_ret = "E";
				break;
			}
			CDlgSite::UpdateSNSerForRemoteCtrl(ret_json["Text"].asCString(), str_ret, true);
		}
	}
	return Ret;
}


INT CIceServerApp::HandleConnectAutomatic(const ::RemoteAprog::tagMSG& Msg, RemoteAprog::tagRESP& Resp) {
	if (Msg.iCmdDataLen == 0)
		return ACERR_PARA;

	BYTE* temp_buff = new BYTE[Msg.iCmdDataLen];
	std::copy(Msg.CmdData.begin(), Msg.CmdData.end(), temp_buff);
	CSerial lSerial(m_ServerCfg.CharSet);
	lSerial.SerialInBuff(temp_buff, Msg.iCmdDataLen);
	SAFEDELARRAY(temp_buff);

	INT ret_code = 0;
	CString protocol;
	lSerial >> protocol;
	CDevMng& GDevMng=GetGlobalDevMng();
	if (GDevMng.m_GlbSetting.OCXInfo.WindowEn) {
		if (m_ServerCfg.pMainWnd) {
			UINT args[1] = { (WPARAM)&ret_code };
			::SendMessage(m_ServerCfg.pMainWnd->m_hWnd, TMSG_CONNECTAUTOMATIC, (WPARAM)args, 0);
		}
	}
	return ret_code;
}


INT CIceServerApp::HandleDisConnectAutomatic(const ::RemoteAprog::tagMSG& Msg, RemoteAprog::tagRESP& Resp) {
	if (Msg.iCmdDataLen == 0)
		return ACERR_PARA;

	BYTE* temp_buff = new BYTE[Msg.iCmdDataLen];
	std::copy(Msg.CmdData.begin(), Msg.CmdData.end(), temp_buff);
	CSerial lSerial(m_ServerCfg.CharSet);
	lSerial.SerialInBuff(temp_buff, Msg.iCmdDataLen);
	SAFEDELARRAY(temp_buff);

	CString protocol;
	lSerial >> protocol;
	CDevMng& GDevMng=GetGlobalDevMng();
	if (GDevMng.m_GlbSetting.OCXInfo.WindowEn) {
		if (m_ServerCfg.pMainWnd) {
			::SendMessage(m_ServerCfg.pMainWnd->m_hWnd, TMSG_DISCONNECTAUTOMATIC, 0, 0);
		}
	}
	return 0;
}


INT CIceServerApp::HandleShowCmdSite(const ::RemoteAprog::tagMSG& Msg, RemoteAprog::tagRESP& Resp) {
	if (Msg.iCmdDataLen == 0)
		return ACERR_PARA;

	BYTE* temp_buff = new BYTE[Msg.iCmdDataLen];
	std::copy(Msg.CmdData.begin(), Msg.CmdData.end(), temp_buff);
	CSerial lSerial(m_ServerCfg.CharSet);
	lSerial.SerialInBuff(temp_buff, Msg.iCmdDataLen);
	SAFEDELARRAY(temp_buff);

	CString str_cmd;
	lSerial >> str_cmd;
	ShowSiteCtrlWindow(GetDevCmd(str_cmd));
	return 0;
}


INT CIceServerApp::HandleAllResetcounter( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp)
{
	INT Ret = 0;
	DEVINFO *pDevInfo;

	CDevMng&GDevMng=GetGlobalDevMng();
	std::map<CString,SKBINFO> mapSkbInfo;
	if(m_ServerCfg.DemoEn==0){
		INT SiteCnt=GDevMng.GetDevCnt(CDevMng::GROUP_ATTACNED);
		for(INT i = 0;i<SiteCnt;i++){
			pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED, i);
			if(pDevInfo==NULL){
				ClientOutput("ALL",LOGLEVEL_ERR,"Can't find devInfo in the connected queue");
				Ret=ACERR_FAIL; goto __end;
			}
			if (pDevInfo && pDevInfo->pDlgSite){
				CDlgSite *pDlgSite=dynamic_cast<CDlgSite*>(pDevInfo->pDlgSite);
				::SendMessage(pDlgSite->m_hWnd,TMSG_ONESITE_RESETCOUNTER,NULL,NULL);
			}

		}

	}
			
	/*if(m_pDevInfo && m_pDevInfo->pDlgSite){
		CDlgSite *pDlgSite=dynamic_cast<CDlgSite*>(m_pDevInfo->pDlgSite);
		SITECTX& TmpSiteCtx=pDlgSite->GetSiteContext();
		if (TmpSiteCtx.hDlgSiteCtl){
			::SendMessage(TmpSiteCtx.hDlgSiteCtl,TMSG_ALLSITE_RESETCOUNTER,NULL,NULL);
		}
	}*/
__end:
	return Ret;
}

INT CIceServerApp::HandleResetcounter( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp)
{
	INT Ret = 0;
	CSerial lSerial(m_ServerCfg.CharSet);
	CString strDevSN;
	DEVINFO *pDevInfo;
	UINT DevCurStatus=0;
	CDevMng&GDevMng=GetGlobalDevMng();
	Ret=SerialCmdData(Msg,lSerial);
	if(Ret==ACERR_OK){
		CSerial TmpSerial;
		INT Rtn=0,i;
		INT RetCmd=ACERR_OK;
		lSerial>>strDevSN;
		if(m_ServerCfg.DemoEn==0){
			pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED,strDevSN);
			if(pDevInfo==NULL){
				ClientOutput("ALL",LOGLEVEL_ERR,"Can't find DevSN=%s in the connected queue",strDevSN);
				Ret=ACERR_FAIL; goto __end;
			}
			if (pDevInfo && pDevInfo->pDlgSite){
				CDlgSite *pDlgSite=dynamic_cast<CDlgSite*>(pDevInfo->pDlgSite);
				::SendMessage(pDlgSite->m_hWnd,TMSG_ONESITE_RESETCOUNTER,NULL,NULL);
			}
			
		}
	}
	/*if(m_pDevInfo && m_pDevInfo->pDlgSite){
		CDlgSite *pDlgSite=dynamic_cast<CDlgSite*>(m_pDevInfo->pDlgSite);
		::SendMessage(pDlgSite->m_hWnd,TMSG_ONESITE_RESETCOUNTER,NULL,NULL);
	}*/
__end:
	return Ret;
}


INT CIceServerApp::HandleSendChemID( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK,Rtn;
	CSerial lSerial(m_ServerCfg.CharSet);
	CSerial TmpSerial(m_ServerCfg.CharSet);
	UINT CustomTag =TAG_CHEMISTRY_ID_SEND;
	UINT32 nType=0,Size=0,CRCcheck = 0;
	CString strChemID;
	CString strDevSN;
	CString strMsg;
	DEVINFO *pDevInfo;
	UINT8 *ChemID =NULL;
	std::vector<UINT8> vMac;
	CDevMng&GDevMng=GetGlobalDevMng();
	Ret=SerialCmdData(Msg,lSerial);
	if(Ret!=ACERR_OK){
		goto __end;
	}
	try{
		lSerial>>nType>>Size>>strChemID;
		INT SiteCnt=GDevMng.GetDevCnt(CDevMng::GROUP_ATTACNED);

		for(INT j = 0;j<SiteCnt;j++){
			TmpSerial.ReInit();
			pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED, j);
			if(pDevInfo==NULL){
				ClientOutput("ALL",LOGLEVEL_ERR,"Can't find devInfo in the connected queue");
				Ret=ACERR_FAIL; goto __end;
			}
			strDevSN.Format(pDevInfo->DevSN);
			ClientOutput(strDevSN,LOGLEVEL_LOG,"Recv src;strDevSN=%s, ChemID=%d,nType=%d,Size=%d",strDevSN,ChemID,nType,Size);
			
			Str2Hex(strChemID, ComTool::ENDIAN_LIT, vMac);
			Size = vMac.size();
			ChemID = new UINT8[Size];
			memset(ChemID, 0, Size);
			std::copy(vMac.begin(), vMac.end(), ChemID);
			TmpSerial<<nType<<Size;
			TmpSerial.SerialInBuff(ChemID,Size);
			for (int i = 0; i < Size; i++)
			{
				CRCcheck += ChemID[i];
			}
			CRCcheck +=(nType+Size);
			TmpSerial<<CRCcheck;
			Ret=pDevInfo->pAprogDev->DoCustom(CustomTag,TmpSerial.GetBuffer(),TmpSerial.GetLength());
			if(Ret!=ACERR_OK){
				Ret=ACERR_FAIL;///如果失败则会退出
			}

			if(ChemID){
				delete[] ChemID;
				ChemID = NULL;
			}
			CRCcheck = 0;
			
		}
	}
	catch (...){

	}

__end:
	if(ChemID){
		delete[] ChemID;
	}
	return Ret;
}

INT CIceServerApp::HandleReadBuffer(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp)
{

	INT Ret=ACERR_OK;
	CString DevSN,FilePath;
	CSerial lSerial(m_ServerCfg.CharSet);
	DEVINFO *pDevInfo;
	CDevMng&GDevMng=GetGlobalDevMng();
	Ret=SerialCmdData(Msg,lSerial);
	if(Ret==ACERR_OK){
		INT RetCmd=ACERR_OK;
		lSerial>>DevSN>>FilePath;
		RESPMSG RespMsg;
		if(m_ServerCfg.DemoEn==0){///实际联机状态
			pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED, DevSN);
			if(pDevInfo==NULL){
				ClientOutput("ALL",LOGLEVEL_ERR,"Can't find devInfo in the connected queue");
				Ret=ACERR_FAIL; goto __end;
			}
			RetCmd=pDevInfo->pAprogDev->DoCmd(CMD_DevRead,NULL,0,&RespMsg);
			if(RetCmd!=ACERR_OK){
				ClientOutput(DevSN,LOGLEVEL_ERR,"ReadBuffer Failed");
				Ret=ACERR_FAIL;///如果失败则会退出
			}
			else{
				RetCmd=pDevInfo->pAprogDev->ReadBufferAndSave(FilePath);
				pDevInfo->pAprogDev->ReleaseRespMsg(&RespMsg);
			}
		}
	}

__end:
	return Ret;

}

INT CIceServerApp::HandleReadProjBuffer(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp)
{

	INT Ret=ACERR_OK;
	CString DevSN,FilePath;
	CProjInfo& GProjInfo=GetGlobalProjInfo();
	CSerial lSerial(m_ServerCfg.CharSet);
	DEVINFO *pDevInfo;
	CFile SrcFile;
	CDevMng&GDevMng=GetGlobalDevMng();
	Ret=SerialCmdData(Msg,lSerial);
	if(Ret==ACERR_OK){
		INT RetCmd=ACERR_OK;
		lSerial>>FilePath;
		if(m_ServerCfg.DemoEn==0){///实际联机状态
			pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED,0);
			if(pDevInfo==NULL){
				ClientOutput("ALL",LOGLEVEL_ERR,"Can't find devInfo in the connected queue");
				Ret=ACERR_FAIL; goto __end;
			}
			//GProjInfo.ReadBufData(&SrcFile,TRUE);
			RetCmd=pDevInfo->pAprogDev->ReadBufferAndSave(FilePath);
			if(RetCmd!=ACERR_OK){
				ClientOutput(DevSN,LOGLEVEL_ERR,"ReadBuffer Failed");
				Ret=ACERR_FAIL;///如果失败则会退出
			}
		}
	}

__end:
	return Ret;
}


INT CIceServerApp::HandleReadChipAllBuffer(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp)
{

	INT Ret=ACERR_OK;
	CString DevSN,FilePath;
	CSerial lSerial(m_ServerCfg.CharSet);
	DEVINFO *pDevInfo;
	CDevMng&GDevMng=GetGlobalDevMng();
	Ret=SerialCmdData(Msg,lSerial);
	if(Ret==ACERR_OK){
		INT RetCmd=ACERR_OK;
		lSerial>>DevSN>>FilePath;
		RESPMSG RespMsg;
		if(m_ServerCfg.DemoEn==0){///实际联机状态
			pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED, DevSN);
			if(pDevInfo==NULL){
				ClientOutput("ALL",LOGLEVEL_ERR,"Can't find devInfo in the connected queue");
				Ret=ACERR_FAIL; goto __end;
			}
			RetCmd=pDevInfo->pAprogDev->DoCmd(CMD_DevRead,NULL,0,&RespMsg);
			if(RetCmd!=ACERR_OK){
				ClientOutput(DevSN,LOGLEVEL_ERR,"ReadBuffer Failed");
				Ret=ACERR_FAIL;///如果失败则会退出
			}
			else{
				RetCmd=pDevInfo->pAprogDev->ReadBufferAndSave(FilePath,TRUE);
				pDevInfo->pAprogDev->ReleaseRespMsg(&RespMsg);
			}
		}
	}

__end:
	return Ret;

}

INT CIceServerApp::HandleReadProjAllBuffer(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp)
{

	INT Ret=ACERR_OK;
	CString DevSN,FilePath;
	CProjInfo& GProjInfo=GetGlobalProjInfo();
	CSerial lSerial(m_ServerCfg.CharSet);
	DEVINFO *pDevInfo;
	CFile SrcFile;
	CDevMng&GDevMng=GetGlobalDevMng();
	Ret=SerialCmdData(Msg,lSerial);
	if(Ret==ACERR_OK){
		INT RetCmd=ACERR_OK;
		lSerial>>FilePath;
		if(m_ServerCfg.DemoEn==0){///实际联机状态
			pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED,0);
			if(pDevInfo==NULL){
				ClientOutput("ALL",LOGLEVEL_ERR,"Can't find devInfo in the connected queue");
				Ret=ACERR_FAIL; goto __end;
			}
			//GProjInfo.ReadBufData(&SrcFile,TRUE);
			RetCmd=pDevInfo->pAprogDev->ReadBufferAndSave(FilePath,TRUE);
			if(RetCmd!=ACERR_OK){
				ClientOutput(DevSN,LOGLEVEL_ERR,"ReadBuffer Failed");
				Ret=ACERR_FAIL;///如果失败则会退出
			}
		}
	}

__end:
	return Ret;
}

INT CIceServerApp::HandleGetChipConfig_String(const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp)
{

	INT Ret=ACERR_OK;
	BYTE *pDataResp;
	CSerial lSerial(m_ServerCfg.CharSet);
	CProjInfo& GProjInfo=GetGlobalProjInfo();

	Ret=GProjInfo.GetChipConfig_String(lSerial);
	if(Ret==ACERR_OK){
		Resp.iRespDataLen=lSerial.GetLength();
		pDataResp=lSerial.GetBuffer();
		if(Resp.iRespDataLen){
			std::copy(pDataResp,pDataResp+Resp.iRespDataLen,std::back_inserter(Resp.RespData));
		}
	}
	else{
		ClientOutput("ALL",LOGLEVEL_ERR,"HandleGetConfig_String Execute Failed, Ret=%d",Ret);
	}

	return Ret;
}


INT CIceServerApp::HandleNewOlderIDBurn( const ::RemoteAprog::tagMSG&Msg,RemoteAprog::tagRESP &Resp )
{
	INT Ret=ACERR_OK,Rtn;
	CSerial lSerial(m_ServerCfg.CharSet);
	CSerial TmpSerial(m_ServerCfg.CharSet);
	UINT CustomTag =TAG_NEWOLDER_ID_BURN;

	CString strDevSN;
	CString strMsg;
	DEVINFO *pDevInfo;
	CDevMng&GDevMng=GetGlobalDevMng();

	CString strIn;
	Json::Value Root;
	Json::Reader JReader;

	Ret=SerialCmdData(Msg,lSerial);
	if(Ret!=ACERR_OK){
		goto __end;
	}

	try{
		lSerial>>strIn;
		std::string strJson(strIn.GetBuffer());
		strIn.ReleaseBuffer();

		CString strNewID;
		CString strOlderID;
		std::vector<BYTE> vNew;
		std::vector<BYTE> vOlder;
		WORD nNewLen = 0;
		WORD nOlderLen = 0;
		UINT CRC = 0;

		if ( !JReader.parse(strJson, Root) ) {
			ClientOutput("ALL", LOGLEVEL_LOG, "MesNewOlderIDBurn Parse json failed");
			Ret=ACERR_FAIL;  goto __end;
		}

		strNewID.Format("%s", Root["NewID"].asCString());
		strOlderID.Format("%s", Root["OlderID"].asCString());

		if (strNewID.IsEmpty() && strOlderID.IsEmpty())
		{
			ClientOutput("ALL", LOGLEVEL_LOG, "MesNewOlderIDBurn, NewID and OlderID is all empty");
			Ret=ACERR_FAIL;  goto __end;
		}

		if (!strNewID.IsEmpty())
		{
			if (ComTool::Str2Hex(strNewID, ComTool::ENDIAN_BIG|ComTool::PREHEADZERO_NEED, vNew) == FALSE )
			{
				ClientOutput("ALL",LOGLEVEL_ERR,"The string contains illegal characters.");
				Ret=ACERR_FAIL; goto __end;
			}
			nNewLen = vNew.size();
		}

		if (!strOlderID.IsEmpty())
		{
			if (ComTool::Str2Hex(strOlderID, ComTool::ENDIAN_BIG|ComTool::PREHEADZERO_NEED, vOlder) == FALSE)
			{
				ClientOutput("ALL",LOGLEVEL_ERR,"The string contains illegal characters.");
				Ret=ACERR_FAIL; goto __end;
			}
			nOlderLen = vOlder.size();
		}

		TmpSerial.ReInit();
		TmpSerial<<nNewLen<<nOlderLen;

		for (int i = 0; i < nNewLen; i++)
		{
			BYTE item = vNew[i];
			TmpSerial<<item;
			CRC+= item;
		}

		for (int i = 0; i < nOlderLen; i++)
		{
			BYTE item = vOlder[i];
			TmpSerial<<item;
			CRC+= item;
		}

		TmpSerial<<CRC;

		INT SiteCnt=GDevMng.GetDevCnt(CDevMng::GROUP_ATTACNED);

		for(INT j = 0;j<SiteCnt;j++){
			
			pDevInfo=GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED, j);
			if(pDevInfo==NULL){
				ClientOutput("ALL",LOGLEVEL_ERR,"Can't find devInfo in the connected queue");
				Ret=ACERR_FAIL; goto __end;
			}
			strDevSN.Format(pDevInfo->DevSN);
			
			Ret=pDevInfo->pAprogDev->DoCustom(CustomTag,TmpSerial.GetBuffer(),TmpSerial.GetLength());
			if(Ret!=ACERR_OK){
				Ret=ACERR_FAIL;///如果失败则会退出
			}

		}
	}
	catch (...){

	}

__end:

	return Ret;
}
