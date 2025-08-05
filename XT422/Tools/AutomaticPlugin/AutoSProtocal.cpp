#include "./SProtocal/STSocket.h"
#include "AutoInterface.h"

//#include "../WorkThread.h"
//#include "../ComStruct/AprogDevMng.h"
#include "AutoSProtocal.h"
#include "../Json/include/json/json.h"
#include "../ComStruct/ComFunc.h"
#include "../ComStruct/LogMsg.h"
#include "TranslationCtrl.h"
#include <map>
#include <QDebug>
#include <unordered_map>
#include <string>
#include <QString>


//#define PrintLog(_Level,fmt,...) \
//	if(m_pLogMsg){\
//	m_pLogMsg->PrintLog(_Level,fmt,__VA_ARGS__);\
//	}
#define PrintLog(_Level,fmt,...) do{\
	char buf[200];\
	memset(buf, 0, 200);\
	sprintf(buf, fmt, ##__VA_ARGS__);\
	printf(buf);printf("\n");\
	/*if (_Level == LOGLEVEL_ERR)*/\
	emit printMessage(QString(buf));\
}while(0)

CAutoSProtocal::CAutoSProtocal()
:m_ErrMsg("")
,m_bAutomaticReady(false)
{
	InitializeCriticalSection(&m_csAuto);
}

CAutoSProtocal::~CAutoSProtocal()
{
	DeleteCriticalSection(&m_csAuto);
}

int CAutoSProtocal::CloseAllSockets()
{
	int Ret=0;


	return Ret;
}

int CAutoSProtocal::InitStartUpSocket()
{
	int Ret = 0;
	bool Rtn = true;
	int TryCnt=4;
	if(m_SktStartUp.Socket.FromHandle()!=INVALID_SOCKET){///已经初始化完成
		return Ret;
	}
	while(TryCnt>0){
		Rtn=m_SktStartUp.Socket.Connect(m_SktStartUp.strIPAddr,m_SktStartUp.Port);///如果连接失败，可能服务器还没启动
		if(Rtn==true){
			break;
		}
		Sleep(200);
		TryCnt--;
	}
	if(Rtn==false){
		std::string strErrMsg;
		char buf[100];
		sprintf(buf, "[SProtocal]Connect AutoApp Fail, ErrNo=%d", m_SktStartUp.Socket.GetLastError());
		strErrMsg = buf;
		PrintLog(LOGLEVEL_ERR,strErrMsg.c_str());
		Ret=-1; goto __end;
	}

__end:
	return Ret;
}

///告知自動化数据参数的选择
int CAutoSProtocal::LoadTaskData()
{
	int Ret=0;
	uint8_t RetFromAuto[2];
	memset(RetFromAuto, 0, sizeof(RetFromAuto));
	int TryCnt=4;
	std::string strMsg = m_strTask;
	m_SktStartUp.Port=m_Para.Port;
	m_SktStartUp.strIPAddr=m_Para.strIPAddr;
	
	Ret=InitStartUpSocket();
	if(Ret!=0){
		goto __end;
	}
	Ret=m_SktStartUp.Socket.Send((uint8_t*)strMsg.c_str(),strMsg.length());
	Ret=m_SktStartUp.Socket.RawReceive(RetFromAuto, 2);
	if (m_SktStartUp.Socket.GetErrMsg().length() > 0){
		PrintLog(LOGLEVEL_ERR, m_SktStartUp.Socket.GetErrMsg().c_str());
		PrintLog(LOGLEVEL_ERR, "m_SktStartUp RawReceive err!");
	}
	m_SktStartUp.Socket.Close();
	Sleep(100);
	if (Ret > 0) {
		QString RetMsg = (char *)RetFromAuto;
		bool toIntOK = false;
		int errcode = RetMsg.toInt(&toIntOK);
		if (toIntOK) {
			string errMsg = TranslateErrMsg(errcode);
			if (errcode == 0) {
				PrintLog(LOGLEVEL_ERR, "Set Task Data Success.");
				Ret = 0;
			}
			else {
				PrintLog(LOGLEVEL_ERR, "Set Task Data Failed, ErrCode: %d, Reason: %s.", errcode, errMsg.c_str());
				Ret = -1;
			}
		}
		else {
			PrintLog(LOGLEVEL_ERR, "Set Task Data Failed, Recv: %s.", RetMsg.toStdString().c_str());
			Ret = -1;
		}
		goto __end;
	}
	else{
		Ret=0;
	}

__end:
	return Ret;


}

int CAutoSProtocal::LoadMesSetting()
{
	int Ret=0;

	m_ConfigSetting.LoadSetting();

	m_StopWhenFailCnt = m_ConfigSetting.m_StopWhenFailCnt;
	m_AutomaticEn = m_ConfigSetting.m_AutomaticEn;
	m_WaitCylinderUp = m_ConfigSetting.m_WaitCylinderUp;
	m_bNeedSwapSKT1234_5678 = m_ConfigSetting.m_bNeedSwapSKT1234_5678;
	m_SitesGroup = m_ConfigSetting.m_SitesGroup;
	m_MaxSiteNum = m_ConfigSetting.m_MaxSiteNum;
	m_SiteAutoMap = m_ConfigSetting.m_strSiteAutoMap;

	//m_AdapterCnt = 1;

	return Ret;
}

int CAutoSProtocal::InitAllSockets()
{
	int Ret=0;
	bool bOptVal = true;
	int bOptLen = sizeof(bool);

	return Ret;
}


////AuoApp->StdMES，自动化端返回设置情况
int CAutoSProtocal::AutoSetSiteEn(int SiteCnt,uint8_t *pSiteEn)
{
	int Ret=0,i,j;
	PrintLog(LOGLEVEL_LOG,"Automatic Set Site Enable");
	std::lock_guard<std::mutex> lock(m_Mutex);
	for(i=0;i<SiteCnt;++i){
		for(j=0;j<(int)m_vSiteMap.size();++j){
			if(m_vSiteMap[j].SiteIdx==(uint8_t)(i+1)){///找到相应的设置项
				uint8_t SiteEN=pSiteEn[i];
				//SwapSKT1234_5678(SiteEN);
				m_vSiteMap[j].SiteSktEnAutoOrg=SiteEN;
				break;
			}
		}
	}
	m_bAutoSiteEnGet=true;
	return Ret;
}

bool CAutoSProtocal::OnSiteEn(void *Para)
{
	CPackMsg *pPckMsg=(CPackMsg*)Para;
	uint8_t *pData;
	int SiteCnt;
	PrintLog(LOGLEVEL_LOG,"GetSiteEn Form AutoApp");
	pData=pPckMsg->GetData(SiteCnt);
	if(SiteCnt>0){
		m_TcpStdMes.ServerSendAck(*pPckMsg,EC_OK);
		AutoSetSiteEn((int)SiteCnt,pData);
	}
	else{
		m_TcpStdMes.ServerSendAck(*pPckMsg,EC_FAIL);
	}
	return true;
}


////AuoApp->StdMES
int CAutoSProtocal::AutoSetChipEn(int SiteIdx,uint8_t* ChipEn)
{
	int Ret=0,j;
	//PrintLog(LOGLEVEL_LOG,"AutoSetChipEn, SiteIdx=%d, ChipEn=0x%02X, Wait=%dms",SiteIdx,ChipEn,m_WaitCylinderUp);
	Sleep(m_WaitCylinderUp);////等待自动化气缸抬起
	std::lock_guard<std::mutex> lock(m_Mutex);
	for(j=0;j<(int)m_vSiteMap.size();++j){
		if(m_vSiteMap[j].SiteIdx==SiteIdx){///找到相应的设置项
			//SwapSKT1234_5678(ChipEn);
			uint64_t tmpEn = 0;
			for (int i = 0; i < 64; i++) {
				uint64_t flag = 1;
				if (*(ChipEn + i) == 1)
					tmpEn = (tmpEn | (flag << i));
				else 
					tmpEn = ((tmpEn & ~(flag << i)));
			}
			m_vSiteMap[j].ChipEnAuto = tmpEn;
			m_vSiteMap[j].bChipReady=true;
			PrintLog(LOGLEVEL_WARNING,"SiteIdx:%d ChipReady",SiteIdx);

			//PostMessage(m_hParentWnd, MSG_UPDATA_READY, (WPARAM)tvSN, 0);
			emit chipIsInPlace(SiteIdx, tmpEn, m_vSiteMap[j].strSiteSN);
			break;
		}
	}
	return Ret;
}

////自动化告知芯片放置情况
bool CAutoSProtocal::OnChipEn(void *Para)
{
	CPackMsg *pPckMsg=(CPackMsg*)Para;
	uint16_t SKTEn;
	uint8_t *pData;
	int PLen, SiteIdx;
	pData=pPckMsg->GetData(PLen);

	if (!UseNewVesion()) { 
		if (PLen == 65) {
			SiteIdx = pData[0];  
			SKTEn = *(uint16_t*)(pData + 1);
			m_TcpStdMes.ServerSendAck(*pPckMsg, EC_OK);
			AutoSetChipEn(SiteIdx, pData + 1);
		}
		else {
			m_TcpStdMes.ServerSendAck(*pPckMsg, EC_FAIL);
		}
	}
	else{
		std::string jsonString = (char*)pData;
		Json::Reader reader;
		Json::Value jsonData;

		if (reader.parse(jsonString, jsonData)) {
			SiteIdx = jsonData["SiteIdx"].asInt();
			SKTEn = jsonData["ChipIn"].asInt();
			//m_AdapterCnt = jsonData["SlotCnt"].asInt();
			m_TcpStdMes.ServerSendAck(*pPckMsg,EC_OK);
			AutoSetChipEn(SiteIdx, pData + 1);

			// jwl test
			//int xAdapterCnt = jsonData["SlotCnt"].asInt();
			//std::string strJson;
			//Json::FastWriter Writer;
			//Json::Value Root;
			//Json::Value funcName;
			//Json::Value paramNode;
			//Root["Command"] = Json::Value("ProgramResult");
			//Root["SiteIdx"] = Json::Value(SiteIdx);
			//Root["SlotCnt"] = Json::Value(xAdapterCnt);
			//for (INT i = 0; i < xAdapterCnt; ++i) {
			//	Json::Value SiteInfo;
			//	SiteInfo["SKTIdx"] = Json::Value(i + 1);
			//	SiteInfo["Result"] = Json::Value(0 >> i & 0x1);
			//	SiteInfo["ErrCode"] = Json::Value(0);
			//	SiteInfo["ErrMsg"] = Json::Value("");
			//	Root["ProgramResult"].append(SiteInfo);
			//}
			//strJson = Writer.write(Root);
			//printf("%s\n", strJson.c_str());
			//m_TcpStdMes.ClientSendPck(PFLAG_SA, PDU_TELLREUSLTS, (uint16_t)strJson.size(), (unsigned char*)strJson.c_str());


		}
		else {
			m_TcpStdMes.ServerSendAck(*pPckMsg, EC_FAIL);
			return false;
		}
	}
	//jwl
	//if(PLen==2){
	//	SiteIdx=pData[0];
	//	SKTEn=pData[1];
	//	m_TcpStdMes.ServerSendAck(*pPckMsg,EC_OK);
	//	AutoSetChipEn(SiteIdx, SKTEn);
	//}
	//else{
	//	m_TcpStdMes.ServerSendAck(*pPckMsg,EC_FAIL);
	//}
	return true;
}

std::string CAutoSProtocal::SendServerAck(std::string command, int Result, std::string ErrMsg) {
	std::string strJson;
	Json::FastWriter Writer;
	Json::Value Root;
	Root["Command"] = Json::Value("SiteEnableStausBack");
	Root["Result"] = Json::Value(1);
	Root["ErrMsg"] = Json::Value("");
	strJson = Writer.write(Root);
	return strJson;
}

bool CAutoSProtocal::OnHandleProgramResultBack(void* Para) {
	CPackMsg* pPckMsg = (CPackMsg*)Para;
	uint8_t* pData;
	int pLen;
	pData = pPckMsg->GetData(pLen);

	if (!UseNewVesion()) {
		uint8_t errcode = *pData;
		emit programResultBack(errcode == 0, errcode, "");
	}
	else {
		Json::Reader Reader;
		Json::Value RootParse;
		Json::Value InfoJson;

		if (Reader.parse((const char*)pData, (const char*)pData + pLen, RootParse) == false) {
			std::string strJson = SendServerAck("ProgramResultBack", 0, "Json parse error.");
			m_TcpStdMes.ServerSendAck(pPckMsg->GetPDU(), strJson.size(), (uint8_t*)strJson.c_str());
			return 0;
		}

		int result = RootParse["Result"].asInt();
		std::string errMsg = RootParse["ErrMsg"].asCString();
		emit programResultBack(result, 0x01, errMsg.c_str());
	}
	return true;
}
bool CAutoSProtocal::OnHandlePck(void *Para)
{
	CPackMsg *pPckMsg = (CPackMsg*)Para;
	uint8_t *pData;
	int pLen;
	pData = pPckMsg->GetData(pLen);

	Json::Reader Reader;
	Json::Value RootParse;
	Json::Value InfoJson;

	if (Reader.parse((const char *)pData, (const char *)pData + pLen, RootParse) == false) {
		std::string strJson = SendServerAck("CommandBack", 0, "Json parse error.");
		m_TcpStdMes.ServerSendAck(pPckMsg->GetPDU(), strJson.size(), (uint8_t *)strJson.c_str());
		return 0;
	}
	std::string command;
	uint32_t i, j;
	command = RootParse["Command"].asCString();
	if (command.compare("SiteEnableStaus") == 0) {
		std::string strJson = SendServerAck("SiteEnableStausBack", 1, "");
		m_TcpStdMes.ServerSendAck(pPckMsg->GetPDU(), strJson.size(), (uint8_t *)strJson.c_str());

		uint32_t SiteCnt = RootParse["SiteCnt"].asUInt();
		uint32_t SlotCnt = RootParse["SlotCnt"].asUInt();
		InfoJson = RootParse["EnableInfo"];
		std::map<uint32_t, uint32_t> enableMap;
		for (uint32_t j = 0; j < InfoJson.size(); j++) {
			Json::Value OneItem = InfoJson[j];
			uint32_t SiteIdx = OneItem["SiteIdx"].asUInt();
			char *ptr = NULL;
			int SiteEnable = (int)strtol(OneItem["SiteEnable"].asCString(), &ptr, 16);
			enableMap[SiteIdx] = SiteEnable;
		}

		std::lock_guard<std::mutex> lock(m_Mutex);
		for (i = 0; i < SiteCnt; ++i) {
			for (j = 0; j < (int)m_vSiteMap.size(); ++j) {
				if (m_vSiteMap[j].SiteIdx == (uint8_t)(i + 1)) {///找到相应的设置项
					uint32_t SiteEN = enableMap[i + 1];
					SwapSKT1234_5678_V2(SiteEN, SlotCnt);
					m_vSiteMap[j].SiteSktEnAutoOrg = SiteEN;
					break;
				}
			}
		}
		m_bAutoSiteEnGet = true;
	}else if (command.compare("ChipIn") == 0) {
		std::string strJson = SendServerAck("ChipInBack", 1, "");
		m_TcpStdMes.ServerSendAck(pPckMsg->GetPDU(), strJson.size(), (uint8_t *)strJson.c_str());


		uint32_t SiteIdx = RootParse["SiteIdx"].asUInt();
		char *ptr = NULL;
		int SlotCnt = 16;
		uint32_t ChipEn = (uint32_t)strtol(RootParse["PlaceInfo"].asCString(), &ptr, 16);

		//PrintLog(LOGLEVEL_LOG, "AutoSetChipEn, SiteIdx=%d, ChipEn=0x%02X, Wait=%dms", SiteIdx, ChipEn, m_WaitCylinderUp);
		Sleep(m_WaitCylinderUp);////等待自动化气缸抬起

		std::lock_guard<std::mutex> lock(m_Mutex);
		for (j = 0; j<(int)m_vSiteMap.size(); ++j) {
			if (m_vSiteMap[j].SiteIdx == SiteIdx) {///找到相应的设置项
				SwapSKT1234_5678_V2(ChipEn, SlotCnt);
				m_vSiteMap[j].ChipEnAuto = ChipEn;
				m_vSiteMap[j].bChipReady = true;
				//PrintLog(LOGLEVEL_WARNING, "SiteIdx:%d ChipReady ChipEn: 0x%x", SiteIdx, ChipEn);

				//PostMessage(m_hParentWnd, MSG_UPDATA_READY, (WPARAM)tvSN, 0);
				break;
			}
		}
	}
	else if (command.compare("InsertionCheck") == 0) {
		std::string strJson = SendServerAck("InsertionCheckBack", 1, "");
		m_TcpStdMes.ServerSendAck(pPckMsg->GetPDU(), strJson.size(), (uint8_t *)strJson.c_str());

		uint32_t SiteIdx = RootParse["SiteIdx"].asUInt();
		uint32_t SlotCnt = RootParse["SlotCnt"].asUInt();
		uint32_t ChipIn = RootParse["ChipIn"].asUInt();
		PrintLog(LOGLEVEL_LOG, "AutoQueryICStatus, SiteIdx=%d", SiteIdx);
		AutoTellICStatus(SiteIdx, SlotCnt, &ChipIn);
	}
	return true;
}

bool CAutoSProtocal::UseNewVesion() {
	return false; // jwl
	return m_TcpStdMes.UseNewVesion() == 1;
}


int CAutoSProtocal::AutoTellICStatus(int SiteIdx, uint32_t AdpCnt,uint32_t *pResult)
{
	std::string strMsg;
	int Ret = 0;
	uint32_t i;
	uint8_t ErrCode;
	if(m_TcpStdMes.IsReady()==false){
		strMsg = "NotReady:";
		strMsg += m_TcpStdMes.GetErrMsg();
		Ret=-1; goto __end;
	}

	if(m_TcpStdMes.InitClientSocket()!=0){
		strMsg = "Connect To Auto App Fail,ErrMsg=";
		strMsg += m_TcpStdMes.GetErrMsg();
		Ret=-1; goto __end;
	}
	if (!UseNewVesion()) {
		uint8_t PDATA[10];
		memset(PDATA, 0, 10);
		PDATA[0] = SiteIdx;
		PDATA[1] = AdpCnt;
		for (i = 0; i < AdpCnt; ++i) {
			PDATA[2 + i] = (uint8_t)pResult[i];
		}
		Ret = m_TcpStdMes.ClientSendPck(PFLAG_SA, PDU_TELLICSTATUS, 10, PDATA);
	}
	else {
		std::string strJson;
		Json::FastWriter Writer;
		Json::Value Root;
		Json::Value funcName;
		Json::Value paramNode;
		Root["Command"] = Json::Value("InsertionCheckResult");
		Root["SiteIdx"] = Json::Value(SiteIdx);
		Root["SlotCnt"] = Json::Value(AdpCnt);
		for (i = 0; i < AdpCnt; ++i) {
			Json::Value SiteInfo;
			SiteInfo["SKTIdx"] = Json::Value(i+1);
			SiteInfo["Result"] = Json::Value(pResult[i]);
			Root["CheckResult"].append(SiteInfo);
		}
		strJson = Writer.write(Root);
		Ret = m_TcpStdMes.ClientSendPck(PFLAG_SA, PDU_TELLICSTATUS, (SHORT)strJson.size(), (uint8_t *)strJson.c_str());
	}

	if(Ret!=0){
		strMsg = "Send PD68 Fail, ErrMsg=";
		strMsg += m_TcpStdMes.GetErrMsg();
		Ret=-1; goto __end;
	}
	else{
		Ret=m_TcpStdMes.ClientGetAck(ErrCode);
		if(Ret==1 && ErrCode==EC_OK){
			Ret=0;
		}
		else{
			char buf[200];
			memset(buf, 0, 200);
			sprintf(buf, "Get PD68 Ack Fail,Ret=%d,ErrCode=%d,ErrMsg=%s",Ret,ErrCode,m_TcpStdMes.GetErrMsg().c_str());
			strMsg = buf;
			Ret=-1;
		}
	}

__end:
	m_TcpStdMes.CloseClientSocket();
	if(strMsg!=""){
		PrintLog(LOGLEVEL_ERR,"SktIdx[%d] AutoTellICStatus Fail, %s",SiteIdx,strMsg.c_str());
	}
	return Ret;

}

///请求编程器确定IC是否在座子里面，做电子检查
///这个地方实际不做电子检查，直接告诉没有芯片存在
int CAutoSProtocal::AutoQueryICStatus(int SiteIdx)
{
	int Ret=0;
	uint32_t Result[8];
	memset(Result,0,8*sizeof(uint32_t));
	PrintLog(LOGLEVEL_LOG,"AutoQueryICStatus, SiteIdx=%d",SiteIdx);
	
	AutoTellICStatus(SiteIdx,m_AdapterCnt,Result);
	return Ret;
}

bool CAutoSProtocal::OnQueryICStatus(void *Para)
{
	CPackMsg *pPckMsg=(CPackMsg*)Para;
	int SiteIdx;
	uint8_t *pData;
	int PLen;
	pData=pPckMsg->GetData(PLen);
	if(PLen==1){
		SiteIdx=pData[0];
		m_TcpStdMes.ServerSendAck(*pPckMsg,EC_OK);
		AutoQueryICStatus(SiteIdx);
	}
	else{
		m_TcpStdMes.ServerSendAck(*pPckMsg,EC_FAIL);
	}
	return true;
}

int CAutoSProtocal::InitAutomatic(eAutoComnType ComnType,void*Para)
{
	int Ret=0;
	//m_Para=*(tTCPAutoPara*)Para;
	m_vSiteMap.clear();
	m_bSwapSKT1234_5678=false;

	LoadMesSetting();

	m_Para.strIPAddr = m_ConfigSetting.m_strIPAddress;
	m_Para.Port = m_ConfigSetting.m_uPort;
	m_Para.DumpPackEn = true;
	m_Para.strMachType = "";

	if(CSTSocket::InitSocket()==false){
		PrintLog(LOGLEVEL_ERR,"TCP WSAStartup Error");
		Ret=-1; goto __end;

	}
	m_TcpStdMes.Uninit();
	m_TcpStdMes.m_EventSiteEn.Clear();
	m_TcpStdMes.m_EventSiteEn+=MakeDelegate<CAutoSProtocal,CAutoSProtocal>(this,&CAutoSProtocal::OnSiteEn);
	m_TcpStdMes.m_EventChipsEn.Clear();
	m_TcpStdMes.m_EventChipsEn+=MakeDelegate<CAutoSProtocal,CAutoSProtocal>(this,&CAutoSProtocal::OnChipEn);
	m_TcpStdMes.m_EventQueryICStatus.Clear();
	m_TcpStdMes.m_EventQueryICStatus+=MakeDelegate<CAutoSProtocal,CAutoSProtocal>(this,&CAutoSProtocal::OnQueryICStatus);
	m_TcpStdMes.m_EventDumpMsg.Clear();
	m_TcpStdMes.m_EventDumpMsg += MakeDelegate<CAutoSProtocal, CAutoSProtocal>(this, &CAutoSProtocal::OnDumpLog);
	m_TcpStdMes.m_EventHandlePck.Clear();
	m_TcpStdMes.m_EventHandlePck += MakeDelegate<CAutoSProtocal, CAutoSProtocal>(this, &CAutoSProtocal::OnHandlePck);
	m_TcpStdMes.m_EventHandleProgramResultBack.Clear();
	m_TcpStdMes.m_EventHandleProgramResultBack += MakeDelegate<CAutoSProtocal, CAutoSProtocal>(this, &CAutoSProtocal::OnHandleProgramResultBack);


	PrintLog(LOGLEVEL_ERR, "InitAutomatic");
	if(m_TcpStdMes.Init()!=0){
		PrintLog(LOGLEVEL_ERR, "StdMes init Fail, %s", m_TcpStdMes.GetErrMsg().c_str());
		m_ErrMsg = m_TcpStdMes.GetErrMsg();
		m_TcpStdMes.Uninit();
		Ret=-1;
	}
	return Ret;
	m_TcpStdMes.Init();

__end:
	return Ret;
}


bool CAutoSProtocal::OnDumpLog(void *Para)
{
	bool bRet = true;
	std::string* strLog = (std::string*)Para;
	std::string strRet;
	strRet = *strLog;

	PrintLog(LOGLEVEL_LOG, "%s", strRet.c_str());
	return bRet;
}

int CAutoSProtocal::ReleaseAutomatic(void)
{
	int Ret=0;
	m_bAutomaticReady=false;
	m_TcpStdMes.Uninit();
	//CSTSocket::UninitSocket();
	return Ret;
}

bool CAutoSProtocal::CheckConnect(void)
{
	return m_bAutomaticReady;
}


////StdMES->AuoApp
int CAutoSProtocal::AutoSetResult(int SiteIdx, uint8_t* Result)
{
	std::string strMsg;
	int Ret=0;
	uint8_t ErrCode = EC_OK;

	if(m_TcpStdMes.IsReady()==false){
		strMsg = "NotReady:";
		strMsg += m_TcpStdMes.GetErrMsg();
		Ret=-1; goto __end;
	}
	if(m_TcpStdMes.InitClientSocket()!=0){
		strMsg = "Connect To Auto App Fail,ErrMsg=";
		strMsg += m_TcpStdMes.GetErrMsg();
		Ret=-1; goto __end;
	}
	if (!UseNewVesion()) {
		uint8_t PDATA[65];
		memset(PDATA, 0, 65);
		//SwapSKT1234_5678((uint8_t &)Result);
		PDATA[0] = (SiteIdx); //站点从1开始
		memcpy(PDATA + 1, Result, 64);
		Ret = m_TcpStdMes.ClientSendPck(PFLAG_SA, PDU_TELLREUSLTS, 65, PDATA);
	}
	else {
		std::string strJson;
		Json::FastWriter Writer;
		Json::Value Root;
		Json::Value funcName;
		Json::Value paramNode;
		Root["Command"] = Json::Value("ProgramResult");
		Root["SiteIdx"] = Json::Value(SiteIdx);
		Root["SlotCnt"] = Json::Value(m_AdapterCnt);
		//int tmpInt = Result;
		for (int i = 0; i < m_AdapterCnt; ++i) {
			Json::Value SiteInfo;
			SiteInfo["SKTIdx"] = Json::Value(i + 1);
			//SiteInfo["Result"] = Json::Value(tmpInt & 0x1);
			SiteInfo["ErrCode"] = Json::Value(0);
			SiteInfo["ErrMsg"] = Json::Value("");
			Root["ProgramResult"].append(SiteInfo);
			//tmpInt = tmpInt >> 1;
		}
		strJson = Writer.write(Root);
		Ret = m_TcpStdMes.ClientSendPck(PFLAG_SA, PDU_TELLREUSLTS, (SHORT)strJson.size(), (uint8_t *)strJson.c_str());
	}

	if(Ret !=0){
		strMsg = "Send PD67 Fail, ErrMsg=";
		strMsg += m_TcpStdMes.GetErrMsg();
		Ret=-1; goto __end;
	}
	else{
		CPackMsg packMsg;
		Ret=m_TcpStdMes.ClientGetAck(ErrCode, &packMsg);
		if(Ret==1 && ErrCode==EC_OK){
			Ret=0;
			//Json::Reader reader;
			//Json::Value jsonData;
			//int len;
			//std::string jsonstr = (char*)packMsg.GetData(len);
			//// 解析 JSON 字符串  
			//if (reader.parse(jsonstr, jsonData)) {
			//	strMsg = "Get PD67 Ack Fail, JSON error";
			//}
			//else {
			//	emit programResultBack(jsonData["Result"].asInt(), ErrCode, jsonData["ErrMsg"].asCString());
			//}
		}
		else{
			char buf[200];
			memset(buf, 0, 200);
			sprintf(buf, "Get PD67 Ack Fail,Ret=%d,ErrCode=%d,ErrMsg=%s",Ret,ErrCode,m_TcpStdMes.GetErrMsg().c_str());
			strMsg = buf;
			Ret=-1;
			//emit programResultBack(0, ErrCode, "Get PD67 Ack Fail");
		}


	}

__end:
	m_TcpStdMes.CloseClientSocket();
	if(strMsg!=""){
		PrintLog(LOGLEVEL_ERR,"SktIdx[%d] AutoSetResult Fail, %s",SiteIdx,strMsg.c_str());
	}
	return Ret;
}

int CAutoSProtocal::SetAutomaticPara(eAutoParaType ParaType,void*Para)
{
	int Ret=0;
	return Ret;
}

int CAutoSProtocal::GetICReadySite(uint32_t*pData,int Size)
{
	int Ret=0;
	return -1;
}

int CAutoSProtocal::GetICReadySite(int SiteIdx, uint8_t *pReady)
{
	int Ret=0,j;
	EnterCriticalSection(&m_csAuto);
	std::lock_guard<std::mutex> lock(m_Mutex);
	for(j=0;j<(int)m_vSiteMap.size();++j){
		if(m_vSiteMap[j].SiteIdx==SiteIdx){///找到相应的设置项
			*pReady=m_vSiteMap[j].bChipReady;
			break;
		}
	}
	LeaveCriticalSection(&m_csAuto);

	std::string strMsg;
	char buf[200];
	sprintf(buf, "GetICReadySite... SiteIdx=%d, 0X%02X", SiteIdx, *pReady);
	strMsg = buf;
	PrintLog(LOGLEVEL_LOG, strMsg.c_str());

	return Ret;
}

int CAutoSProtocal::GetICReadySite(int SiteIdx, uint8_t *pReady, uint32_t *pICStatus)
{
	int Ret=0,j;
	EnterCriticalSection(&m_csAuto);
	std::lock_guard<std::mutex> lock(m_Mutex);

	for(j=0;j<(int)m_vSiteMap.size();++j){
		if(m_vSiteMap[j].SiteIdx==SiteIdx){///找到相应的设置项
			*pReady=m_vSiteMap[j].bChipReady;
			*pICStatus=m_vSiteMap[j].ChipEnAuto;////
			break;
		}
	}
	LeaveCriticalSection(&m_csAuto);
	return Ret;
}

int CAutoSProtocal::ClearICReadySite(int SiteIdx)
{
	int Ret=0,j;
	EnterCriticalSection(&m_csAuto);

	std::lock_guard<std::mutex> lock(m_Mutex);
	for(j=0;j<(int)m_vSiteMap.size();++j){
		if(m_vSiteMap[j].SiteIdx==SiteIdx){///找到相应的设置项
			m_vSiteMap[j].bChipReady=false;
			PrintLog(LOGLEVEL_ERR, "ClearICReadySite, SiteIdx=%d", SiteIdx);
			break;
		}
	}
	LeaveCriticalSection(&m_csAuto);
	return Ret;
}

int CAutoSProtocal::GetSiteIdx(std::string AutoAlias)
{
	int Ret=0,i;
	for(i=0;i<(int)m_vSiteMap.size();++i){
		if(m_vSiteMap[i].strSiteAlias ==AutoAlias){
			return m_vSiteMap[i].SiteIdx;
		}
	}
	return Ret;
}

int CAutoSProtocal::GetSiteIdxBySn(const std::string& sn){
	for (int j = 0; j < (int)m_vSiteMap.size(); ++j) {
		if (m_vSiteMap[j].strSiteSN == sn) {///找到相应的设置项
			
			return m_vSiteMap[j].SiteIdx;
		}
	}
	return -1;
}

std::string CAutoSProtocal::GetAutoAlias(int SiteIdx)
{
	std::string Alias="";
	return Alias;
}

int CAutoSProtocal::SetDoneSite(int SiteIdx, uint8_t* Result, uint64_t Mask)
{
	int Ret=0;
	EnterCriticalSection(&m_csAuto);
	Ret=AutoSetResult(SiteIdx,Result);
	LeaveCriticalSection(&m_csAuto);
	return Ret;
}

int GetSiteAutoMap(const std::string& strSiteAutoMap,std::vector<tSProtocalSiteMap>& vSiteMap)
{
	CHAR ch;
	int Ret=0,i,Cnt;
	int BracketCnt=0;
	int Comma=0,Pos;
	std::string strSiteAilas;
	std::string strSiteIdx;
	std::string strSiteMap;
	tSProtocalSiteMap SiteMap;
	if(strSiteAutoMap!=""){
		Cnt=strSiteAutoMap.length();
		for(i=0;i<Cnt;++i){
			ch=strSiteAutoMap.at(i);
			if(ch=='<'){
				strSiteMap="";
				BracketCnt++;
				Comma=0;
			}
			else if(ch=='>'){
				Pos=strSiteMap.find(',');
				if(Pos==-1){
					Ret=-1; goto __end;
				}
				strSiteAilas=strSiteMap.substr(0,Pos);
				strSiteIdx=strSiteMap.substr(Pos+1,100);
				if(strSiteAilas=="" || strSiteIdx==""){
					Ret=-1; goto __end;
				}
				SiteMap.strSiteAlias=strSiteAilas;
				SiteMap.bChipReady = 0;
				SiteMap.ChipEnAuto = 0;
				sscanf((LPSTR)(LPCSTR)strSiteIdx.c_str(),"%d",(int *)&SiteMap.SiteIdx);
				vSiteMap.push_back(SiteMap);	
				BracketCnt--;
			}
			else{
				strSiteMap +=ch;
			}
		}
	}
	if(BracketCnt!=0){
		Ret=-1;
	}
__end:
	return Ret;
}

void CAutoSProtocal::GetSiteMap(std::map<int, std::string>& vSiteMap) { 
	vSiteMap.clear();
	for (auto iter : m_vSiteMap) {
		vSiteMap.insert(std::pair<uint32_t, std::string>(iter.SiteIdx, iter.strSiteAlias));
	}
}

bool CAutoSProtocal::IsNeedTransferTaskData(std::string strMachType)
{
	/*CDevMng&GDevMng=GetGlobalDevMng();
	std::vector<std::string> & vNeedSendTaskDataType=GDevMng.m_GlbSetting.AutoSProtocal.m_vNeedSendTaskDataType;
	int i;
	///遍历ini文件中列出来的需要进行Task设置的机型，如果满足，则返回成功
	for(i=0;i<(int)vNeedSendTaskDataType.size();++i){
		if(strMachType==vNeedSendTaskDataType[i]){
			return true;
		}
	}*/
	return false;
}

std::string CAutoSProtocal::GetMachType()
{
	return m_Para.strMachType;
}



int CAutoSProtocal::QuerySiteEn()
{
	int Ret=0,Rtn;
	//CDevMng&GDevMng=GetGlobalDevMng();
	//std::vector<tAutoSiteEn> vAutoSiteEn;
	///从设置中直接选择，然后连接所有设置的站点

	///先查询当前自动化端的客户设置情况
#if 0
	Ret=AutoQuerySiteEn();
	if(Ret!=0){
		goto __end;
	}
	/*
	else{
		///等待自动化端返回设置
		int timoutMax=4000;
		while(m_bAutoSiteEnGet==false){
			Sleep(200);
			timoutMax -=200;
			if(timoutMax <=0){
				Ret=-1; goto __end;
			}
		}
	}*/
#endif 

	EnterCriticalSection(&m_csAuto);
	m_vSiteMap.clear();
	Rtn=::GetSiteAutoMap(m_SiteAutoMap,m_vSiteMap);
	if(Rtn!=0){
		PrintLog(LOGLEVEL_ERR,"Get GetSiteAutoMap Failed");
		goto __end;
	}

	//for (int j = 0; j < (int)m_vSiteMap.size(); j++){
	//	std::string strInfo;
	//	char buf[300];
	//	memset(buf, 0, 300);
	//	sprintf(buf, "QuerySiteEn m_vSiteMap[%d]:, bChipReady=%d, bSiteEnvInit=%d, ChipEnAuto=%u, SiteIdx=%d, SKTRdy=%d, strAutoAlias = %s, strSiteAlias = %s, strSiteSN=%s",
	//		j, m_vSiteMap[j].bChipReady, m_vSiteMap[j].bSiteEnvInit,
	//		m_vSiteMap[j].ChipEnAuto, m_vSiteMap[j].SiteIdx, m_vSiteMap[j].SKTRdy,
	//		m_vSiteMap[j].strAutoAlias.c_str(), m_vSiteMap[j].strSiteAlias.c_str(), m_vSiteMap[j].strSiteSN.c_str());
	//	strInfo = buf;
	//	PrintLog(LOGLEVEL_ERR, strInfo.c_str());
	//}
	///设置使能，准备连接编程器
	//for(i=0;i<(int)m_vSiteMap.size();++i){
	//	tAutoSiteEn AutoSiteEn;
	//	AutoSiteEn.AdapterEn=0xFF;
	//	AutoSiteEn.strAutoAlias.Format("%d",m_vSiteMap[i].SiteIdx);
	//	m_vSiteMap[i].strAutoAlias=AutoSiteEn.strAutoAlias;
	//	vAutoSiteEn.push_back(AutoSiteEn);
	//}
	//
	//Ret=GDevMng.AutomaticHandleSiteEn(vAutoSiteEn);


__end:
	LeaveCriticalSection(&m_csAuto);
	return Ret;
}

void CAutoSProtocal::SwapSKT1234_5678(uint8_t& SktEn)
{
	if(m_bSwapSKT1234_5678){
		PrintLog(LOGLEVEL_LOG,"SKT Position Swap");
		uint8_t ls=SktEn&0x0F;
		uint8_t ms=(SktEn&0xF0)>>4;
		SktEn=(ls<<4)|ms;
	}
	else{
		PrintLog(LOGLEVEL_LOG,"SKT Position Not Swap");
	}
}

void CAutoSProtocal::SwapSKT1234_5678_V2(uint32_t& SktEn, int SlotCnt)
{
	if (m_bSwapSKT1234_5678) {
		PrintLog(LOGLEVEL_LOG, "SKT Position Swap");
		if (SlotCnt == 8) {
			uint8_t ls = SktEn & 0x0F;
			uint8_t ms = (SktEn & 0xF0) >> 4;
			SktEn = (ls << 4) | ms;
		}
		else if (SlotCnt == 16) {
			SHORT ls = SktEn & 0xFF;
			SHORT ms = (SktEn & 0xFF) >> 8;
			SktEn = (ls << 8) | ms;
		}
		else {
			uint8_t ls = SktEn & 0x0F;
			uint8_t ms = (SktEn & 0xF0) >> 4;
			SktEn = (ls << 4) | ms;
		}

	}
	else {
		PrintLog(LOGLEVEL_LOG, "SKT Position Not Swap");
	}
}

////StdMES->AuoApp 请求告知站点设置情况
int CAutoSProtocal::AutoQuerySiteEn()
{
	std::string strMsg;
	int Ret=0;
	uint8_t ErrCode = EC_OK;
	m_bAutoSiteEnGet=false;
	if(m_TcpStdMes.IsReady()==false){
		strMsg = "NotReady:" + m_TcpStdMes.GetErrMsg();
		Ret=-1; goto __end;
	}

	if(m_TcpStdMes.InitClientSocket()!=0){
		strMsg = "Connect To Auto App Fail,ErrMsg=" + m_TcpStdMes.GetErrMsg();
		Ret=-1; goto __end;
	}
	if (!UseNewVesion()) {
		Ret = m_TcpStdMes.ClientSendPck(PFLAG_SA, PDU_QUERYSITEEN, 0, NULL);
	}
	else {
		std::string strJson;
		Json::FastWriter Writer;
		Json::Value Root;
		Json::Value funcName;
		Json::Value paramNode;
		Root["Command"] = Json::Value("QuerySiteInfo");
		Root["Version"] = Json::Value(CURRENT_VERSION);
		strJson = Writer.write(Root);
		Ret = m_TcpStdMes.ClientSendPck(PFLAG_SA, PDU_QUERYSITEEN, strJson.size(), (uint8_t *)strJson.c_str());
	}
	if(Ret!=0){
		strMsg = "Send PD64 Fail, ErrMsg=" + m_TcpStdMes.GetErrMsg();
		Ret=-1; goto __end;
	}
	else{
		Ret=m_TcpStdMes.ClientGetAck(ErrCode);
		if(Ret==1 && ErrCode==EC_OK){
			Ret=0;
		}
		else{
			char buf[300];
			memset(buf, 0, 300);
			sprintf(buf, "Get PD64 Ack Fail,Ret=%d,ErrCode=%d,ErrMsg=%s",Ret,ErrCode,m_TcpStdMes.GetErrMsg().c_str());
			strMsg = buf;
			Ret=-1;
		}
	}

__end:
	m_TcpStdMes.CloseClientSocket();
	if(strMsg!=""){
		PrintLog(LOGLEVEL_ERR,"AutoQuerySiteEn Fail, %s",strMsg.c_str());
	}
	return Ret;
}

uint32_t CAutoSProtocal::GetSiteReadySKTInfo(uint32_t SiteIdx) {
	uint32_t SKTRdy = 0;
	for (uint32_t j = 0; j<(uint32_t)m_vSiteMap.size(); j++) {///设置站点使能
		if (m_vSiteMap[j].SiteIdx == SiteIdx + 1) {
			if (m_vSiteMap[j].bSiteEnvInit == true) {
				SKTRdy = m_vSiteMap[j].SKTRdy;
				PrintLog(LOGLEVEL_WARNING, "[%s]EnvInit Ready, Site Adapter Input=0x%02X", m_vSiteMap[j].strSiteAlias.c_str(), SKTRdy);
			}
			else {///如果站点没初始化成功，则不能告诉自动化Ready
				SKTRdy = 0x00;
				PrintLog(LOGLEVEL_WARNING, "[%s]EnvInit Fail, Tell AutoApp Not Ready", m_vSiteMap[j].strSiteAlias.c_str());
			}
			//SwapSKT1234_5678(PDATA[SiteIdx]);///自动化是否需要交换座子位置
			break;
		}
	}
	return SKTRdy;
	///如果有找到则设置站点Ready情况，否则设置为0x00
}

///告知自动化站点Ready情况 发送PD63
int CAutoSProtocal::AutoTellSiteReady(std::string strRdyInfo)
{
	std::string strMsg;
	int Ret=0,i;
	uint8_t ErrCode = EC_OK;

	PrintLog(LOGLEVEL_LOG,"TellAutoSiteReady");
	if(m_TcpStdMes.IsReady()==false){
		strMsg = "NotReady:" + m_TcpStdMes.GetErrMsg();
		Ret=-1; goto __end;
	}

	if(m_TcpStdMes.InitClientSocket()!=0){
		strMsg = "Connect To Auto App Fail,ErrMsg=" + m_TcpStdMes.GetErrMsg();
		Ret=-1; goto __end;
	}


	//for (int i = 0; i < m_MaxSiteNum; i++){
	//	PDATA[i] = 0x00;
	//	int itemBit = (nEnItemSite >> i) & 0x01;
	//	if (itemBit == 0x01){  //加上哪个超限了就不使能。
	//		PDATA[i] = 0x01;
	//	}
	//}
	if (!UseNewVesion()) {
		uint8_t* PDATA = new uint8_t[m_MaxSiteNum * 8]; 
		memset(PDATA, 0, m_MaxSiteNum * 8);
		for (i = 0; i<m_MaxSiteNum; i++) {
			uint32_t ReadyInfo = GetSiteReadySKTInfo(i);
			//SwapSKT1234_5678(ReadyInfo);
			*(uint64_t*)(PDATA + i * 8) = (uint64_t)ReadyInfo;
		}
		Ret = m_TcpStdMes.ClientSendPck(PFLAG_SA, DPU_TELLAUTOSITERDY, m_MaxSiteNum * 8, PDATA);
		delete[] PDATA;
	}
	else {
		std::string strJson;
		Json::FastWriter Writer;
		Json::Value Root;
		Json::Value funcName;
		Json::Value paramNode;
		Root["Command"] = Json::Value("SiteInitStaus");
		//Root["Version"] = Json::Value(CURRENT_VERSION);
		Root["SiteCnt"] = Json::Value(m_MaxSiteNum);
		//m_AdapterCnt = 8;
		Root["SlotCnt"] = Json::Value(m_AdapterCnt);
		for (int i = 0; i < m_MaxSiteNum/*m_AdapterCnt*/; ++i) {
			Json::Value SiteInfo;
			SiteInfo["SiteIdx"] = Json::Value(i + 1);
			uint32_t ReadyInfo = (uint32_t)GetSiteReadySKTInfo(i);
			SwapSKT1234_5678_V2(ReadyInfo, m_AdapterCnt);
			SiteInfo["SiteEnable"] = Json::Value(ReadyInfo);
			Root["EnableInfo"].append(SiteInfo);
		}
		strJson = Writer.write(Root);
		Ret = m_TcpStdMes.ClientSendPck(PFLAG_SA, DPU_TELLAUTOSITERDY, strJson.size(), (uint8_t *)strJson.c_str());
	}

	if(Ret !=0){
		strMsg = "Send PD63 Fail, ErrMsg=" + m_TcpStdMes.GetErrMsg();
		Ret=-1; goto __end;
	}
	else{
		Ret=m_TcpStdMes.ClientGetAck(ErrCode);
		if(Ret==1 && ErrCode==EC_OK){
			Ret=0;
		}
		else{
			char buf[300];
			memset(buf, 0, 300);
			sprintf(buf, "Get PD63 Ack Fail,Ret=%d,ErrCode=%d,ErrMsg=%s",Ret,ErrCode,m_TcpStdMes.GetErrMsg().c_str());
			strMsg = buf;
			Ret=-1;
		}
	}
__end:
	m_TcpStdMes.CloseClientSocket();
 
	if(strMsg!=""){
		PrintLog(LOGLEVEL_ERR,"[Error]%s",strMsg.c_str());
	}
	return Ret;
}


/*******************************
strRdyInfo为如下所示的Json字符串
{
	"ProjInfo":{
		"AdapterNum":1   ///适配板的个数，1对1还是1对8
	},
	"SiteReady":[
	{
		"SiteSN":"XXXXX",
			"SiteAlias":"XXXX",
			"SiteEnvRdy":1, ///该站点是否已经初始化好编程环境，如果为1表示已经OK，否则为0 
			"SKTRdy":"FF" ///表示站点中的SKT是否准备好，bit0-7表示SKT1到SKT8，为1表示Ready，为0表示没有
	}
	]
}
*************************************/
int CAutoSProtocal::ParserSiteRdyInfo(std::string strRdyInfo)
{
	//PrintLog(LOGLEVEL_ERR, strRdyInfo.c_str());

	int Ret=0,i,j=0;
	Json::Reader Reader;
	Json::Value Root;
	Json::Value SiteReady;
	Json::Value ProjInfo;
	int AdapterCnt=0;
	if(Reader.parse(strRdyInfo.c_str(),strRdyInfo.c_str()+strRdyInfo.length(),Root)==false){
		Ret=-1; goto __end;
	}
	ProjInfo=Root["ProjInfo"];
	AdapterCnt=ProjInfo["AdapterNum"].asInt();

	m_AdapterCnt = AdapterCnt;
	if(m_bNeedSwapSKT1234_5678){///发送给自动机的需要进行Swap
		if(AdapterCnt==8){///8连板需要Swap
			m_bSwapSKT1234_5678=true;
		}
		else{
			m_bSwapSKT1234_5678=false;
		}
	}
	else{
		m_bSwapSKT1234_5678=false;
	}

	//for (int k= 0; k < (int)m_vSiteMap.size(); k++) {
	//	std::string strInfo;
	//	char buf[300];
	//	memset(buf, 0, 300);
	//	sprintf(buf, "ParserSiteRdyInfo m_vSiteMap[%d]:, bChipReady =%d, bSiteEnvInit=%d, ChipEnAuto=%u,SiteIdx=%d, SKTRdy=%d, strAutoAlias = %s, strSiteAlias = %s, strSiteSN=%s",
	//		k, m_vSiteMap[k].bChipReady, m_vSiteMap[k].bSiteEnvInit,
	//		m_vSiteMap[k].ChipEnAuto, m_vSiteMap[k].SiteIdx, m_vSiteMap[j].SKTRdy,
	//		m_vSiteMap[k].strAutoAlias.c_str(), m_vSiteMap[k].strSiteAlias.c_str(), m_vSiteMap[k].strSiteSN.c_str());
	//	strInfo = buf;
	//	//PrintLog(LOGLEVEL_ERR, strInfo);
	//}

	SiteReady=Root["SiteReady"];
	for(i=0;i<(int)SiteReady.size();++i){
		Json::Value OneSite=SiteReady[i];
		std::string SiteAlias;
		SiteAlias = OneSite["SiteAlias"].asCString();
		///在映射表中找到对应站点别名，设置其他值
		for(j=0;j<(int)m_vSiteMap.size();++j){
			if(m_vSiteMap[j].strSiteAlias==SiteAlias){//找到了匹配的
				uint32_t SktEn=0;
				m_vSiteMap[j].strSiteSN = OneSite["SiteSN"].asCString();
				sscanf(OneSite["SKTRdy"].asCString(),"%X",&SktEn);
				m_vSiteMap[j].SKTRdy= SktEn;
				m_vSiteMap[j].bSiteEnvInit=OneSite["SiteEnvRdy"].asInt();

				/*SendMessage(m_AttachHwd, MSG_OVER_LIMITED, (WPARAM)1, 0);*/
				break;
			}
		}
		if(j==m_vSiteMap.size()){///没有找打匹配的，提示错误
			PrintLog(LOGLEVEL_WARNING,"%s站点初始化完成，但是在自动化映射表中没有找到与之匹配的映射项",SiteAlias.c_str());
			Ret=-1; goto __end;
		}
	}

__end:
	return Ret;
}

int CAutoSProtocal::TellDevReady(std::string RdyInfo)
{
	int Ret=0;
	Ret= SendPD63TellSiteReady(RdyInfo);
	if(Ret==0){
		PrintLog(LOGLEVEL_LOG,"Send Task To AutoApp %s ...",GetMachType().c_str());

		Ret=LoadTaskData();
	}
	return Ret;
}
int CAutoSProtocal::SendPD63TellSiteReady(std::string RdyInfo)
{
	int Ret=0;
	std::string strRdyInfo;
	EnterCriticalSection(&m_csAuto);
	//if(m_fnGetDevRdyInfo==NULL){
	//	Ret=-1; goto __end;
	//}

	//Ret = m_fnGetDevRdyInfo(m_GetDevRdyInfoPara, strRdyInfo);
	//if (Ret != 0) {
	//	PrintLog(LOGLEVEL_LOG, "GetDevRdyInfo return error");
	//	goto __end;
	//}

	Ret = ParserSiteRdyInfo(RdyInfo);
	if (Ret != 0) {
		PrintLog(LOGLEVEL_LOG, "ParserSiteRdyInfo return error");
		goto __end;
	}
	///////////////////////////////

	Ret=AutoTellSiteReady(RdyInfo);

__end:
	LeaveCriticalSection(&m_csAuto);
	return Ret;
}

void CAutoSProtocal::GetErrMsg(int ErrNo,std::string& ErrMsg)
{
	//return m_ErrMsg;
	ErrMsg=m_ErrMsg;
}

int CAutoSProtocal::SetTask(std::string strTask)
{
	int Ret=0;
	if(IsNeedTransferTaskData(m_Para.strMachType)==true){
		PrintLog(LOGLEVEL_LOG,"%s SetTask...",GetMachType().c_str());
		m_strTask=strTask;
		m_bAutomaticReady=true;///这里不能直接发送，需要等到发送PD63告诉自动化编程器已经初始化完成之后再进行
	}
	else{
		m_strTask = strTask; //添加
		m_bAutomaticReady=true;
	}
	return Ret;
}

bool CAutoSProtocal::GetProtocalVersion()
{
	///默认1.0
	m_ProtocalVer[1]=1;
	m_ProtocalVer[0]=0;
	return true;
}

