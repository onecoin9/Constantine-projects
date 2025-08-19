
#include "TcpStdMes.h"
#include "../Json/include/json/json.h"
#include "../../AutoInterface.h"

#define TMSG_STARTJOB   (WM_USER+0x502)

CTcpStdMes::CTcpStdMes(void)
:m_strErrMsg("")
,m_bTcpStdMesRdy(false)
,m_bExit(false)
,m_TryCntMax(3)
,m_TryDelayTime(200)
{
	m_SktClient.Port=64100;
	m_SktClient.strIPAddr="127.0.0.1";
	m_SktServer.Port=64101;
	m_SktServer.strIPAddr="127.0.0.1";
}

CTcpStdMes::~CTcpStdMes(void)
{
	Uninit();
}


int WINAPI TcpThreadProc(MSG msg,void *Para)
{
	int Ret=0;
	CTcpStdMes* pTcmClient=(CTcpStdMes*)(Para);
	if(msg.message==TMSG_STARTJOB){
		Ret=pTcmClient->DoJob();
	}
	return Ret;
}

int CTcpStdMes::ServerSendAck(CPackMsg&PckMsg,uint8_t ErrCode)
{
	return SendPckAck(m_SktAccept,PckMsg,ErrCode);
}

int CTcpStdMes::ServerSendAck(uint8_t Pdu, int PLen, uint8_t *pData) {
	int Ret = 0;
	std::string strMSg;
	//strMSg = "[Server]Sending Ack";
	//m_EventDumpMsg(&strMSg);
	Ret = SendPck(m_SktAccept, PFLAG_SA, Pdu, PLen, pData);

	strMSg = "[Server]Send Ack:" + QString::number(Pdu, 16).toStdString() + " Done";
	m_EventDumpMsg(&strMSg);
	return Ret;
}

int CTcpStdMes::HandlePck(CSTSocket &Socket,uint8_t*pData,int Size)
{
	int Ret=0;
	CTSerial lSerial;
	CPackMsg PckMsg;
	lSerial.SerialInBuff(pData,Size);
	if(PckMsg.Serial(lSerial)==false){
		SendPckAck(Socket,PckMsg,EC_FAIL);
		Ret=-1;
	}
	else{
		m_EventDumpPck(&PckMsg);
		if (UseNewVesion() == 1) {
			m_EventHandlePck((void*)&PckMsg);
		}
		else {
			uint8_t PDU = PckMsg.GetPDU();
			switch (PDU) {
			case PDU_TELLSITEEN:
				if (m_EventSiteEn((void*)&PckMsg) == false) {
					Ret = -1;
				}
				break;
			case PDU_TELLCHIPSEN:
				if (m_EventChipsEn((void*)&PckMsg) == false) {
					Ret = -1;
				}
				break;
			case PDU_QUERYICSTATUS:
				if (m_EventQueryICStatus((void*)&PckMsg) == false) {
					Ret = -1;
				}
				break;
			case PDU_TELLREUSLTS:
				if (m_EventHandleProgramResultBack((void*)&PckMsg) == false) {
					Ret = -1;
				}
				break;
			default:
				SendPckAck(Socket, PckMsg, EC_PDUERR);
				Ret = -1;
				break;
			}
		}

	}
	return Ret;
}

int CTcpStdMes::SendPckAck(CSTSocket &Socket,CPackMsg&PckMsg,uint8_t ErrCode)
{
	int Ret=0;
	//std::string strMSg;
	//strMSg = "[Server]Sending Ack";
	//m_EventDumpMsg(&strMSg);
	Ret = SendPck(Socket, PFLAG_SA, PckMsg.GetPDU(), 1, &ErrCode);
	
	std::string strMSg = "[Server]Send Ack:" + QString::number(PckMsg.GetPDU(), 16).toStdString() + " Done";
	m_EventDumpMsg(&strMSg);
	return Ret;
}

/// WSANOTINITIALISED   
int CTcpStdMes::DoJob()
{
	int Ret=0,Rtn=0;
	CSTSocket&sSocket=m_SktAccept;
	uint8_t PckData[PCKDATA_BUFLEN];
	std::string strMsg;
	while(!m_bExit){
		if(m_SktServer.Socket.Accept(sSocket)==false){
			continue;
		}
		memset(PckData,0,PCKDATA_BUFLEN);
		//strMsg = "[Server]Receive....";
		//m_EventDumpMsg(&strMsg);
		Rtn=sSocket.Receive(PckData,PCKDATA_BUFLEN);

		//char buf[100];
		//memset(buf, 0, 100);
		//sprintf(buf, "[Server]Receive Done=%d", Rtn);
		//strMsg = buf;
		//m_EventDumpMsg(&strMsg);
		if(Rtn>0){
			///返回实际接收的字节数	
			Ret=HandlePck(sSocket,PckData,Rtn);
			m_strErrMsg = "";
			//continue;
		}
		else if (Rtn==0){
			///连接被关闭
			// m_strErrMsg = "[StdMesTcp]Connection is closed";
			m_strErrMsg = "";
		}
		else{
			char buf[100];
			sprintf(buf, "[StdMesTcp]Error occurs, ErrCode=%d", sSocket.GetLastError());
			m_strErrMsg = buf;
		}
		if (m_strErrMsg.length() > 0) {
			strMsg = "[Server]Receive Done ErrMsg=";
			strMsg += m_strErrMsg;
			m_EventDumpMsg(&strMsg);
		}
		sSocket.Close();
	}
	sSocket.Close(); 
	return Ret;
}

void CTcpStdMes::CloseClientSocket()
{
	m_SktClient.Socket.Close();
	Sleep(100);
}

int CTcpStdMes::InitClientSocket()
{
	int TryCnt=m_TryCntMax;
	int Ret = 0;
	bool Rtn = true;
	if(m_SktClient.Socket.FromHandle()!=INVALID_SOCKET){///已经初始化完成
		return Ret;
	}
	while(TryCnt>0){
		Rtn=m_SktClient.Socket.Connect(m_SktClient.strIPAddr,m_SktClient.Port);///如果连接失败，可能服务器还没启动
		if(Rtn==true){
			break;
		}
		Sleep(m_TryDelayTime);
		TryCnt--;
	}
	if(Rtn==false){
		char buf[100];
		memset(buf, 0, 100);
		sprintf(buf, "[StdMesTcp]Connect AutoApp Fail, ErrNo=%d",m_SktClient.Socket.GetLastError());
		m_strErrMsg = buf;
		//AfxMessageBox(m_strErrMsg);
		Ret=-1; goto __end;
	}
	else {
		int timeout = 5000; //5s
		if (m_SktClient.Socket.SetSockOpt(SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout), SOL_SOCKET) == FALSE) {
			Ret = -1; goto __end;
		}
	}

__end:
	return Ret;
}

int CTcpStdMes::IsAutoSupportNewProtocal() {
	std::string ExePath = "C:\\AcroView\\PH5000\\config\\";
	std::string strIniFile = ExePath + "/Config.ini";
	int SupportNewProtocal = GetPrivateProfileIntA("Config", "NewSProtocal", 0, strIniFile.c_str());
	return SupportNewProtocal;
}

int CTcpStdMes::UseNewVesion() {
	if (CURRENT_VERSION >= 0x30) {
		return IsAutoSupportNewProtocal();
	}
	return 0;
}

int CTcpStdMes::InitServerSocket()
{
	int Ret=0;
	if(!m_SktServer.Socket.Create()){
		char buf[100];
		memset(buf, 0, 100);
		sprintf(buf, "Create SktServer Fail, ErrNo=%d",m_SktServer.Socket.GetLastError());
		m_strErrMsg = buf;
		Ret=-1; goto __end;
	}
	
	Ret = UseNewVesion();
	if (Ret == 1) {
		m_SktServer.Socket.SetLenByteSize(2);
	}
	else if (Ret < 0) {
		m_strErrMsg = "More Automatic Config ini file found.";
		Ret = -1; goto __end;
	}

	bool bOptVal = true;
	int bOptLen = sizeof(bool);

	//设置Socket的选项, 解决10048错误必须的步骤
	//允许绑定之前已经绑定的地址
	m_SktServer.Socket.SetSockOpt(SO_REUSEADDR, (void *)&bOptVal, bOptLen, SOL_SOCKET);

	if (!m_SktServer.Socket.Bind(m_SktServer.Port, m_SktServer.strIPAddr)) {


		char buf[100];
		memset(buf, 0, 100);
		sprintf(buf, "Listen Failed: %d", m_SktServer.Socket.GetLastError());
		m_strErrMsg = buf;
		Ret = -1; goto __end;
	}
	//监听
	if(!m_SktServer.Socket.Listen(100)){
		char buf[100];
		memset(buf, 0, 100);
		sprintf(buf, "[StdMesTcp]Listen Failed: %d", m_SktServer.Socket.GetLastError());
		m_strErrMsg = buf;
		Ret=-1; goto __end;
	}


	///创建服务器接收进程
	m_wtServer.SetMsgHandler(TcpThreadProc,this);
	m_wtServer.CreateThread();
	m_wtServer.PostMsg(TMSG_STARTJOB,0,0);

__end:
	return Ret;

}

int CTcpStdMes::Init()
{
	int Ret=0;
	bool Rtn=false;
	m_bExit=false;
	m_bTcpStdMesRdy=false;
	if(!CSTSocket::InitSocket()){
		m_strErrMsg = "[StdMesTcp]Socket Init Failed";
		Ret=-1; goto __end;
	}

	Ret=InitServerSocket();
	if(Ret!=0){
		goto __end;
	}
	m_bTcpStdMesRdy=true;
__end:
	return Ret;
}

int CTcpStdMes::Uninit()
{
	m_bExit=true;
	m_SktClient.Socket.Close();
	m_SktAccept.Close();
	m_SktServer.Socket.Close();
	m_wtServer.DeleteThread();
	//CSTSocket::UninitSocket();
	m_bTcpStdMesRdy=false;
	return 0;
}

/********************
读取一个包消息
-1表示失败
0表示Socket终止
1表示获取包消息成功
********************/
int CTcpStdMes::RecvPck(CSTSocket&Socket,CPackMsg&PckMsg)
{
	int TryCnt=0;
	int Ret=0,ByteRead=0;
	CTSerial lSerial;
	uint8_t TmpBuf[PCKDATA_BUFLEN];
	memset(TmpBuf,0,PCKDATA_BUFLEN);
	ByteRead=Socket.Receive(TmpBuf,PCKDATA_BUFLEN);
	if(ByteRead>0){
		lSerial.SerialInBuff(TmpBuf,ByteRead);
		if(lSerial.length()>0){
			if(PckMsg.Serial(lSerial)==false){
				m_strErrMsg = "[StdMesTcp]Receive PackMsg Fail, PackMsg Format Error";
				Ret=-1; goto __end;
			}
		}
		m_EventDumpPck(&PckMsg);
		Ret=1;
	}
	else if(ByteRead==0){
		m_strErrMsg = "[StdMesTcp]Receive PackMsg Fail, AutoApp Closed";
		Ret=0;
	}
	else{
		char buf[100];
		memset(buf, 0, 100);
		sprintf(buf, "[StdMesTcp]Receive PackMsg Fail, ErrCode=%d",Socket.GetLastError());
		m_strErrMsg = buf;
		Ret=-1;
	}
__end:
	return Ret;
}

/*******************
成功返回1
中断返回0
出现错误返回-1
*********************/
int CTcpStdMes::ClientRecvPck(CPackMsg&PckMsg)
{
	int Ret=0;
	std::string strMSg;
	//strMSg = "[Client]Receving Pck";
	//m_EventDumpMsg(&strMSg);
	Ret=RecvPck(m_SktClient.Socket,PckMsg);
	strMSg = "[Client]Receving Pck:" + QString::number(PckMsg.GetPDU(), 16).toStdString() + " Done";
	m_EventDumpMsg(&strMSg);
	return Ret;
}

int CTcpStdMes::ClientGetAck(uint8_t&ErrCode, CPackMsg* pckMsg)
{
	int Ret=0;
	CPackMsg tmpMsg;
	CPackMsg* msgPtr = pckMsg ? pckMsg : &tmpMsg;

	//std::string strMSg;
	//strMSg = "[Client]Receving Ack";
	//m_EventDumpMsg(&strMSg);
	Ret=RecvPck(m_SktClient.Socket,*msgPtr);
	if(Ret==1){
		int PLen;
		uint8_t *pData= msgPtr->GetData(PLen);
		if (pData != NULL && PLen == 1) {
			ErrCode = pData[0];
		}
	}
	std::string strMSg = std::string("[Client]Recevie Ack:") + QString::number(msgPtr->GetPDU(), 16).toStdString() + " Done";
	m_EventDumpMsg(&strMSg);
	return Ret;
}


int CTcpStdMes::SendPck(CSTSocket&Socket,uint16_t PFlag,uint8_t Pdu, int PLen,uint8_t *pData)
{
	int Ret=0;
	int ByteSend;
	CTSerial lSerial;
	CPackMsg PckMsg; 
	PckMsg.SetMsg(PFlag,Pdu,PLen,pData);
	PckMsg.UnSerial(lSerial);
	m_EventDumpPck(&PckMsg);
	ByteSend=(int)lSerial.length();
	if(Socket.Send(lSerial.GetBuffer(),ByteSend)!=ByteSend){
		char buf[100];
		memset(buf, 0, 100);
		sprintf(buf, "[StdMesTcp]Send PackMsg Failed, ErrCode=%d",Socket.GetLastError());
		m_strErrMsg = buf;
		Ret=-1;
	}
	return Ret;
}
int CTcpStdMes::ClientSendPck(uint16_t PFlag,uint8_t Pdu, int PLen,uint8_t *pData)
{
	int Ret=0;
	//strMSg = "[Client]Sending Pck";
	//m_EventDumpMsg(&strMSg);
	Ret=SendPck(m_SktClient.Socket,PFlag,Pdu,PLen,pData);
	std::string strMSg = "[Client]Send Pck:";
	strMSg += QString::number(Pdu, 16).toStdString() + " Done";
	m_EventDumpMsg(&strMSg);
	return Ret;
}




