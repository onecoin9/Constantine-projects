#include "AppCtrls.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDataStream>
#include "Crc32_Std.h"
#include "ACCmdPacket.h"

///注意DataSize_PerTrans要是DataSize_PerPacket的整数倍
#define DataSize_PerPacket      (4096) //单次下行包的数据量   不能超过4K，SSD需要每次4K的数据量
#define DataSize_PerTrans       (1*1024*1024) //单次发送的命令中的数据长度要固定为1M
#define TIME_LIMIT_US   (10)    //延迟时间，us为单位

#define _PrintLog(_Level,fmt,...) \
	if (m_pILog) {\
		m_pILog->PrintLog(_Level, fmt, __VA_ARGS__);\
	}

CAppCtrls::CAppCtrls()
    :m_pAppModels(NULL)
    ,m_pILog(NULL)
{
    m_RPCMethodHash.clear();
}

void CAppCtrls::AttachAppModels(CAppModels* pAppModels)
{
	m_pAppModels = pAppModels;
}

#define RegistRPCMethod(_MethodName) \
    RegistRPCMethodHandler(#_MethodName, &CAppCtrls::ExecRPCMethod_##_MethodName);

int32_t CAppCtrls::RegistAllRPCMethod()
{
    //
   //在这里添加其他RPC方法的调用
    RegistRPCMethod(LinkScan)
    RegistRPCMethod(GetCapacity)
    RegistRPCMethod(SendData)
    RegistRPCMethod(ReadData)
    RegistRPCMethod(SendPTCmd)
    RegistRPCMethod(SSD2DDR)
    RegistRPCMethod(DDR2SSD)
    return 0;
}


#define RegistDevCallbackFunc(_CmdID,_CallbackFunc) \
    RegistDevCallbakFunc(_CmdID, &CAppCtrls::DevCallBack_##_CallbackFunc);

int32_t CAppCtrls::RegistAllDevCallbackFunc()
{
    RegistDevCallbackFunc(CmdID_SetLog, SetLog);
    RegistDevCallbackFunc(CmdID_SetProgress, SetProgress);
    RegistDevCallbackFunc(CmdID_ReadBuffData, ReadBuffData);
    return 0;
}

int32_t CAppCtrls::RunCtrls()
{
    int32_t Ret = 0;
    tIPInfo RemoteIPInfo, LocalIPInfo;
    RemoteIPInfo.Addr = m_pAppModels->m_AppConfigModel.RemoteIP();
    RemoteIPInfo.Port = m_pAppModels->m_AppConfigModel.RemotePort();
    LocalIPInfo.Addr = m_pAppModels->m_AppConfigModel.LocalIP();
    LocalIPInfo.Port = m_pAppModels->m_AppConfigModel.LocalPort();
    //信号和槽的连接
    Qt::ConnectionType ConnetType = Qt::AutoConnection;//Qt::DirectConnection;

    //底层命令处理的初始化
    m_CmdHandler.AttachILog(m_pILog);
    m_CmdHandler.AttachIACComm(&m_NetComm);
    m_CmdHandler.AttachAppModels(m_pAppModels);

    //应用层命令处理的初始化
    m_ACCmdHandler.AttachILog(m_pILog);
    m_ACCmdHandler.AttachAppModels(m_pAppModels);
    m_ACCmdHandler.AttachCmdHandler(&m_CmdHandler);
    connect(&m_ACCmdHandler, &CACCmdHandler::sigRemoteCmdComplete,this,&CAppCtrls::OnRemoteCmdComplete);
    connect(&m_ACCmdHandler, &CACCmdHandler::sigRemoteQueryDoCmd, this, &CAppCtrls::OnRemoteQueryDoCmd);
  
    //与Device的网络通信初始化
    m_NetComm.AttachILog(m_pILog);
    m_NetComm.AttachICmdHandler(&m_CmdHandler);

    
    m_NetComm.SetIPInfo(&RemoteIPInfo, &LocalIPInfo);
    m_NetComm.StartComm();
    
    //Tcp服务的初始化，用来
    m_TcpSvc.AttachILog(m_pILog);
    connect(&m_TcpSvc, &CTcpSvc::sigTcpSvcReceivePacket, this, &CAppCtrls::OnTcpSvcReceivePacket);
    m_TcpSvc.Start();


    RegistAllRPCMethod();
    RegistAllDevCallbackFunc();

    return Ret;
   
}

int32_t CAppCtrls::StopCtrls()
{
    m_TcpSvc.Stop();
    m_NetComm.StopComm();
    return 0;
}


/// <summary>
/// 设置新的指针，返回旧指针
/// </summary>
#if 0
IDataWriter* CAppCtrls::StoreDataWriter(IDataWriter* pNewIDataWriter)
{
    IDataWriter* pPrev = NULL;
    m_DataWriterMutex.lock();
    pPrev = m_pIDataWriter;
    m_pIDataWriter = pNewIDataWriter;
    m_DataWriterMutex.unlock();
    return pPrev;
}

/// <summary>
/// 写操作，pData为写入的数据，Size为实际的字节数，返回实际写入的字节数，小于0表示失败
/// </summary>
qint64 CAppCtrls::WriteDataByDataWriter(char* pData, qint64 Size)
{
    qint64 WriteBytes = 0;
    m_DataWriterMutex.lock();
    if (m_pIDataWriter) {
        WriteBytes = m_pIDataWriter->WriteData(pData, Size);
    }
    else {
        _PrintLog(LOGLEVEL_E, "WriteDataByDataWriter m_pIDataWriter=NULL\r\n");
    }
    m_DataWriterMutex.unlock();
    return WriteBytes;
}

qint64 CAppCtrls::GetDataWriterCurDataSize()
{
    qint64 WriteBytes = 0;
    m_DataWriterMutex.lock();
    WriteBytes = m_pIDataWriter->GetCurDataSize();
    m_DataWriterMutex.unlock();
    return WriteBytes;
}
#endif 

int32_t CAppCtrls::RegistRPCMethodHandler(QString strMethod, FnExecRPCMethod fnMethodFunc)
{
    int32_t Ret = 0;
    m_RPCMethodHash.insert(strMethod, fnMethodFunc);
    return Ret;
}

int32_t CAppCtrls::RegistDevCallbakFunc(quint16 CmdID, FnExecDevCallBack fnCallBackFunc)
{
    int32_t Ret = 0;
    m_DevCallbackFuncHash.insert(CmdID, fnCallBackFunc);
    return Ret;
}


QByteArray CAppCtrls::ConstructJsonRPCResponse(QJsonValue Id, int32_t RetCode, tRPCParamsHash& ResultHash, QString&ErrMessage)
{
    QJsonDocument  RootDoc;
    QJsonObject RootObj;
    QByteArray TmpByteArray;

    RootObj.insert("jsonrpc", "2.0");
    if (Id.isUndefined() == false) {///有包含ID
        RootObj.insert("id", Id);
    }
    if (RetCode < 0) {
        QJsonObject ErrObj;
        ErrObj.insert("code", RetCode);
        if (ErrMessage.isEmpty()) {
            ErrMessage = "undefined";
        }
        ErrObj.insert("message", ErrMessage);
        RootObj.insert("error", ErrObj);
    }
    else {
        QJsonObject ResultObj;
        ResultObj.insert("code", RetCode);
        if (ResultHash.size() > 0) {
            QJsonObject DataObj;
            tRPCParamsHashItr ResultHashItr;
            ResultHashItr = ResultHash.begin();
            while (ResultHashItr != ResultHash.end()) {
                DataObj.insert(ResultHashItr.key(), ResultHashItr.value());
                ResultHashItr++;
            }
            ResultObj.insert("data", DataObj);
        }
        RootObj.insert("result", ResultObj);
    }
    RootDoc.setObject(RootObj);
    return RootDoc.toJson();
}

int32_t CAppCtrls::ExecRPCMethod(IRPCSender* pSender, QString strMethod, QJsonValue Id, tRPCParamsHash& ParamsHash)
{
    int32_t Ret = 0;
    QHash<QString, CAppCtrls::FnExecRPCMethod>::iterator Itr;
    tRPCParamsHash ResultHash;
    QString ErrMessage;
    QByteArray TmpByteArray;
    Itr = m_RPCMethodHash.find(strMethod);
    if (Itr != m_RPCMethodHash.end()) {
        CAppCtrls::FnExecRPCMethod fnMethod = Itr.value();
        Ret=(this->*fnMethod)(pSender, strMethod,ParamsHash, ResultHash, ErrMessage);
    }
    else {
        Ret = ERR_JSONRPC_MethodNotSupport;
        SendRPCRespnseWithoutDoingMethod(pSender, JsonRPC_ErrorCode_Methodnotfound, "Method no found");
        goto __end;
    }
    TmpByteArray = ConstructJsonRPCResponse(Id, Ret, ResultHash, ErrMessage);
    pSender->SendRespJson(TmpByteArray);

__end:
    return Ret;
}

 

int32_t CAppCtrls::SendRPCRespnseWithoutDoingMethod(IRPCSender* pSender,int32_t ErrCode,QString ErrMessage)
{
    int32_t Ret = 0;
    QByteArray TmpByteArray;
    QJsonValue Id;
    tRPCParamsHash ResultHash;
    TmpByteArray = ConstructJsonRPCResponse(Id, ErrCode, ResultHash, ErrMessage);
    pSender->SendRespJson(TmpByteArray);
    return Ret;
}

int32_t CAppCtrls::ExecJsonRPCMethod(IRPCSender* pSender,const char* strJson)
{
    int32_t Ret = 0;
    QJsonParseError err_rpt;
    QJsonDocument  RootDoc = QJsonDocument::fromJson(strJson, &err_rpt); // 字符串格式化为JSON

    if (err_rpt.error != QJsonParseError::NoError) { 
        _PrintLog(LOGLEVEL_E, "JsonParser failed: %s\r\n", err_rpt.errorString().toStdString().c_str());
        Ret = ERR_JSONRPC_Parser;
        SendRPCRespnseWithoutDoingMethod(pSender, JsonRPC_ErrorCode_ParseErr, "Parse error");
        goto __end;
    }
    else {
        QJsonObject RootObj = RootDoc.object();
        QJsonValue RpcVersion = RootDoc["jsonrpc"];
        QJsonValue MethodValue = RootDoc["method"]; // method键的值，是一个数组
        QJsonValue IDValue = RootDoc["id"];
        QJsonValue Params = RootDoc["params"];
        tRPCParamsHash ParamHash;
        bool bNotify = false;
        bool bParamsNeed = false;
        if (RpcVersion.isNull()) {
            _PrintLog(LOGLEVEL_E, "JsonParser failed, Need to have \"jsonrpc\"\r\n");
            Ret = ERR_JSONRPC_NeedJsonRpc; goto __end;
        }

        if (MethodValue.isNull()){
            _PrintLog(LOGLEVEL_E, "JsonParser failed, Need to have \"method\"\r\n");
            Ret = ERR_JSONRPC_NeedMethod;goto __end;
        }
        bNotify = IDValue.isUndefined();//id为空表示是一个通知，不需要返回
        bParamsNeed = Params.isObject();//isObject为ture表示存在，为false表示不存在

        _PrintLog(LOGLEVEL_N,"RPCExec [Method]: %s , [Notify]: %s, [ParamsNeed]: %s\r\n",\
            MethodValue.toString().toStdString().c_str(),bNotify?"true":"false", bParamsNeed?"true":"false");

        if (bParamsNeed) {
            int KeyIdx = 0;
            QJsonObject ParamsObj = Params.toObject();
            QStringList KeyList=ParamsObj.keys();
            foreach (QString strKey, KeyList) {
                _PrintLog(LOGLEVEL_N, "Key[%d]:%s, Value:%s\r\n", ++KeyIdx, strKey.toStdString().c_str(), ParamsObj[strKey].toVariant().toString().toStdString().c_str());
                ParamHash.insert(strKey, ParamsObj[strKey]);
            }
        }
        Ret = ExecRPCMethod(pSender, MethodValue.toString(), IDValue, ParamHash);
    }

__end:
    return Ret;
}



void CAppCtrls::OnTcpSvcReceivePacket(QTcpSocket* pFromSocket, QByteArray JsonPacket)
{
    int32_t Ret = 0;
    const char* Packet = JsonPacket.constData();
    uint32_t Size = *(uint32_t*)Packet;
    CJsonRPCSender JsonRPCSender(pFromSocket);
    const char* strJson = Packet + 4;
    _PrintLog(LOGLEVEL_N, "JsonStr: %s\r\n", strJson);
   
    Ret = ExecJsonRPCMethod(&JsonRPCSender, strJson);

}


void CAppCtrls::OnRemoteCmdComplete(uint32_t HopNum, uint32_t PortID, int32_t ResultCode, QByteArray RespDataBytes)
{
    _PrintLog(LOGLEVEL_N, "RemoteCmdComplete,HopNum:%d, PortID:0x%X, ResultCode:%d, RespDataBytesSize:%d\r\n", HopNum,PortID,ResultCode, RespDataBytes.size());
    if (RespDataBytes.size() > 0) {
        int32_t i = 0;
        int32_t DiffCnt = 0;
        uint8_t* pRespData = (uint8_t*)RespDataBytes.constData();
       // m_pILog->PrintBuf((char*)"========PTComplete ResponseData======", (char*)RespDataBytes.constData(), RespDataBytes.size());
#if 0
        for (i = 0; i < RespDataBytes.size(); ++i) {
            if (pRespData[i] != (i % 0x100)) {
                _PrintLog(LOGLEVEL_E, "DataCompare Failed,Data[%04d]:0x%02X, Expected:0x%02X\r\n", i, pRespData[i], i % 0x100);
                DiffCnt++;
            }
        }
        _PrintLog(LOGLEVEL_N, "DiffCnt:%d\r\n", DiffCnt);
#endif 
    }
}
void CAppCtrls::OnRemoteQueryDoCmd(uint32_t HopNum, uint32_t PortID,uint32_t CmdFlag,uint16_t CmdID, QByteArray CmdDataBytes)
{
    _PrintLog(LOGLEVEL_D, "RemoteQueryDoCmd, HopNum:%d, PortID:0x%X, CmdID:0x%04X, CmdDataBytesSize:%d\r\n", HopNum, PortID, CmdID, CmdDataBytes.size());
    int32_t Ret = 0,RetCall=0;
    QByteArray RespData;
    QHash<quint16, CAppCtrls::FnExecDevCallBack>::iterator Itr;
    Itr = m_DevCallbackFuncHash.find(CmdID);
    if (Itr != m_DevCallbackFuncHash.end()) {
        CAppCtrls::FnExecDevCallBack fnDevCallbackFunc = Itr.value();
        RetCall = (this->*fnDevCallbackFunc)(HopNum, PortID, CmdFlag, CmdID, CmdDataBytes,RespData);
    }
    else {
        RetCall = ERR_NETCOMM_CMDIDNOSUPPORT;
    }
    if (CmdFlag & CmdFlag_Notify) {//是一个Notify命令，不需要响应回去，否组需要响应数据
        _PrintLog(LOGLEVEL_D, "QueryDoCmd CmdID:0x%04X is a Notify Cmd\r\n");
    }
    else {
        CACCmdHandler* pACCmdHandler = &m_ACCmdHandler;
        _PrintLog(LOGLEVEL_D, "QueryDoCmd CmdID:0x%04X is a Response Cmd\r\n");
        Ret=pACCmdHandler->SendDevCallBackResult(HopNum, PortID, RetCall, RespData);
    }
}


int32_t CAppCtrls::ExecRPCMethod_ReadData(IRPCSender* pSender, QString strMethod, tRPCParamsHash& ParamsHash, tRPCParamsHash& ResultHash, QString& ErrMessage)
{
    int32_t Ret = 0;
    _PrintLog(LOGLEVEL_N, "Exec %s Method\r\n", strMethod.toStdString().c_str());
    QString strFilePath = ParamsHash.find("FilePath").value().toString();
    uint32_t HopNum = ParamsHash.find("HopNum").value().toInt();
    QString Type = ParamsHash.find("Type").value().toString();
    uint32_t PortID = ParamsHash.find("PortID").value().toString().toUInt(NULL, 16);
    uint64_t SrcAddr = ParamsHash.find("SrcAddr").value().toString().toULongLong(NULL, 16);
    uint64_t Length = ParamsHash.find("Length").value().toString().toULongLong(NULL, 16);
    CACCmdHandler* pACCmdHandler = &m_ACCmdHandler;
    strFilePath = TranslateFilePath(strFilePath);
    Ret = pACCmdHandler->ReadDataAndSaveToFile(strFilePath, Type, HopNum, PortID, SrcAddr, Length, ResultHash);

__end:
    _PrintLog(LOGLEVEL_N, "ReadData Ret:%d\r\n", Ret);

    return Ret;
}

static void Delayus(int Timeus)
{
    LARGE_INTEGER cpuFreq;
    LARGE_INTEGER startTime;
    LARGE_INTEGER endTime;
    QueryPerformanceFrequency(&cpuFreq);
    QueryPerformanceCounter(&startTime);
    do {
        QueryPerformanceCounter(&endTime);
    } while ((((endTime.QuadPart - startTime.QuadPart) * 1000000.0) / (cpuFreq.QuadPart * 1.0)) < (Timeus * 1.0));
}

QString CAppCtrls::TranslateFilePath(QString strFilePathOrg)
{
    QString strFileDest;
    if (strFilePathOrg != "") {
        if (strFilePathOrg.contains(":") == true) {//为一个绝对路径
            strFileDest = strFilePathOrg;
        }
        else {//这是一个相对路径，需要用到ini文件中的配置
            QString strDataFilePrefix = m_pAppModels->m_AppConfigModel.DataFilePath();
            strFileDest = strDataFilePrefix + "\\" + strFilePathOrg;
            _PrintLog(LOGLEVEL_N, "FilePathDest===>%s\r\n", strFileDest.toStdString().c_str());
        }
    }
    return strFileDest;
}

///RPC命令出来函数

int32_t CAppCtrls::ExecRPCMethod_SendData(IRPCSender* pSender, QString strMethod, tRPCParamsHash& ParamsHash, tRPCParamsHash& ResultHash, QString& ErrMessage)
{
    int32_t Ret = 0;
    _PrintLog(LOGLEVEL_N, "Exec %s Method\r\n", strMethod.toStdString().c_str());
    QString strFilePath = ParamsHash.find("FilePath").value().toString();
    uint32_t HopNum = ParamsHash.find("HopNum").value().toInt();
    QString Type = ParamsHash.find("Type").value().toString();
    uint32_t PortID = ParamsHash.find("PortID").value().toString().toUInt(NULL, 16);
    uint64_t DestAddr = ParamsHash.find("DestAddr").value().toString().toULongLong(NULL, 16);
    CACCmdHandler* pACCmdHandler = &m_ACCmdHandler;
    strFilePath=TranslateFilePath(strFilePath);
    Ret = pACCmdHandler->WriteDataFromFile(strFilePath, Type, HopNum, PortID, DestAddr, ResultHash);
    return Ret;
}


int32_t CAppCtrls::ExecRPCMethod_SendPTCmd(IRPCSender* pSender, QString strMethod, tRPCParamsHash& ParamsHash, tRPCParamsHash& ResultHash, QString& ErrMessage)
{
    int32_t Ret = 0;
    uint32_t CmdFlag = 0;
    QString strDataFilePath = "";
    _PrintLog(LOGLEVEL_N, "Exec %s Method\r\n", strMethod.toStdString().c_str());
    uint32_t HopNum = ParamsHash.find("HopNum").value().toInt();
    uint32_t PortID = ParamsHash.find("PortID").value().toString().toUInt(NULL, 16);
    QString strCmdFlag = ParamsHash.find("CmdFlag").value().toString();
    uint32_t CmdID= ParamsHash.find("CmdID").value().toString().toUInt(NULL, 16);
    uint32_t SktEn = ParamsHash.find("SktEn").value().toString().toUInt(NULL, 16);
    uint16_t BPUID = ParamsHash.find("BPUID").value().toString().toUInt(NULL, 10);

    QString strCmdDataFile = ParamsHash.find("CmdDataFile").value().toString();
    QByteArray CmdDataArray = QByteArray::fromHex(ParamsHash.find("CmdData").value().toString().toStdString().c_str());
    CACCmdHandler* pACCmdHandler = &m_ACCmdHandler;
    if (strCmdFlag == "Notify") {
        CmdFlag |= CmdFlag_Notify;
    }

    strDataFilePath = TranslateFilePath(strCmdDataFile);

    if (strDataFilePath != "") {
        QFile File(strDataFilePath);
        File.open(QIODevice::ReadOnly);
        if (File.isOpen() == false) {
            _PrintLog(LOGLEVEL_E, "Open File Failed,Path:%s\r\n", strDataFilePath.toUtf8().toStdString().c_str());
            Ret = ERR_ReadFile;
            goto __end;
        }
        CmdDataArray = File.readAll();
        File.close();
    }
    
    _PrintLog(LOGLEVEL_D, "SentPTCmd, HopNum:%d,PortID:0x%08X,CmdID:0x%04X,CmdFlag:0x%08X\r\n", HopNum, PortID, CmdID, CmdFlag);
    Ret=pACCmdHandler->RemoteDoPTCmd(HopNum, PortID, CmdFlag, CmdID, SktEn, BPUID, CmdDataArray);

__end:
    return Ret;
}

int32_t CAppCtrls::ExecRPCMethod_SSD2DDR(IRPCSender* pSender, QString strMethod, tRPCParamsHash& ParamsHash, tRPCParamsHash& ResultHash, QString& ErrMessage)
{
    int32_t Ret = 0;
    uint32_t CmdFlag = 0;
    QString strDataFilePath = "";
    _PrintLog(LOGLEVEL_N, "Exec %s Method\r\n", strMethod.toStdString().c_str());
    uint32_t HopNum = ParamsHash.find("HopNum").value().toInt();
    uint32_t PortID = ParamsHash.find("ProtID").value().toString().toUInt(NULL, 16);
    uint64_t SrcAddr = ParamsHash.find("SrcAddr").value().toString().toULongLong(NULL, 16);
    uint64_t DestAddr = ParamsHash.find("DestAddr").value().toString().toULongLong(NULL, 16);
    uint64_t DataSize = ParamsHash.find("DataSize").value().toString().toULongLong(NULL, 16);

    CACCmdHandler* pACCmdHandler = &m_ACCmdHandler;

    pACCmdHandler->SSD2DDR(HopNum, PortID, SrcAddr, DestAddr, DataSize);

    return Ret;
}

int32_t CAppCtrls::ExecRPCMethod_DDR2SSD(IRPCSender* pSender, QString strMethod, tRPCParamsHash& ParamsHash, tRPCParamsHash& ResultHash, QString& ErrMessage)
{
    int32_t Ret = 0;
    uint32_t CmdFlag = 0;
    QString strDataFilePath = "";
    _PrintLog(LOGLEVEL_N, "Exec %s Method\r\n", strMethod.toStdString().c_str());
    uint32_t HopNum = ParamsHash.find("HopNum").value().toInt();
    uint32_t PortID = ParamsHash.find("ProtID").value().toString().toUInt(NULL, 16);
    uint64_t SrcAddr = ParamsHash.find("SrcAddr").value().toString().toULongLong(NULL, 16);
    uint64_t DestAddr = ParamsHash.find("DestAddr").value().toString().toULongLong(NULL, 16);
    uint64_t DataSize = ParamsHash.find("DataSize").value().toString().toULongLong(NULL, 16);

    CACCmdHandler* pACCmdHandler = &m_ACCmdHandler;

    pACCmdHandler->DDR2SSD(HopNum, PortID, SrcAddr, DestAddr, DataSize);

    return Ret;
}



int32_t CAppCtrls::ExecRPCMethod_LinkScan(IRPCSender* pSender, QString strMethod, tRPCParamsHash& ParamsHash, tRPCParamsHash& ResultHash, QString& ErrMessage)
{
    int32_t Ret = 0;
    _PrintLog(LOGLEVEL_N, "Exec %s Method\r\n", strMethod.toStdString().c_str());
    uint32_t HopNum = ParamsHash.find("HopNum").value().toInt();
    Ret = m_ACCmdHandler.LinkScan(HopNum);
    return int32_t();
}


int32_t CAppCtrls::ExecRPCMethod_GetCapacity(IRPCSender* pSender, QString strMethod, tRPCParamsHash& ParamsHash, tRPCParamsHash& ResultHash, QString& ErrMessage)
{
    int32_t Ret = 0;
    int32_t BPUPos = 0, SKTPos = 0;
    _PrintLog(LOGLEVEL_N, "Exec %s Method\r\n", strMethod.toStdString().c_str());
    uint32_t HopNum = ParamsHash.find("HopNum").value().toInt();
    QString Type = ParamsHash.find("Type").value().toString();
    uint32_t PortID = ParamsHash.find("ProtID").value().toString().toUInt(NULL, 16);

    Ret=m_ACCmdHandler.GetCapacity(Type, HopNum, PortID);
   
__end:
    return Ret;
}



int32_t CAppCtrls::DevCallBack_SetLog(uint32_t HopNum, uint32_t PortID, uint32_t CmdFlag, uint16_t CmdID, QByteArray CmdDataBytes, QByteArray& RespDataBytes)
{
    int32_t Ret = 0;
    uint32_t LogLevel;
    uint32_t LogFrom;
    uint16_t LogMsgSize;
    char LogMsg[1024];
    QDataStream DataStream(&CmdDataBytes,QIODevice::ReadOnly);
    DataStream.setByteOrder(QDataStream::LittleEndian);//设置为小端方式
    
    memset(LogMsg, 0, 1024);
    DataStream >> LogLevel >> LogFrom >> LogMsgSize;
    DataStream.readRawData(LogMsg, LogMsgSize);

    _PrintLog(LOGLEVEL_N, "MsgFromDev Level[%d] From[%d]: %s\r\n", LogLevel, LogFrom,LogMsg);
    return Ret;
}

int32_t CAppCtrls::DevCallBack_SetProgress(uint32_t HopNum, uint32_t PortID, uint32_t CmdFlag, uint16_t CmdID, QByteArray CmdDataBytes, QByteArray& RespDataBytes)
{
    int32_t Ret = 0;
    uint32_t Current;
    uint32_t Total;
    QDataStream DataStream(&CmdDataBytes, QIODevice::ReadOnly);
    DataStream.setByteOrder(QDataStream::LittleEndian);//设置为小端方式

    DataStream >> Current >> Total;

    _PrintLog(LOGLEVEL_N, "MsgFromDev Progress [%d/%d]\r\n", Current, Total);
    return Ret;
}

int32_t CAppCtrls::DevCallBack_ReadBuffData(uint32_t HopNum, uint32_t PortID, uint32_t CmdFlag, uint16_t CmdID, QByteArray CmdDataBytes, QByteArray& RespDataBytes)
{
    int32_t Ret = 0;
    uint32_t PartIdx;
    uint64_t BufferOffset;
    uint32_t DataSize;
    char RespData[256];
    QDataStream DataStream(&CmdDataBytes, QIODevice::ReadOnly);
    DataStream.setByteOrder(QDataStream::LittleEndian);//设置为小端方式

    DataStream >> PartIdx >> BufferOffset >> DataSize;

    memset(RespData, 0x89, 256);
    RespDataBytes.append(RespData, 256);

    _PrintLog(LOGLEVEL_N, "MsgFromDev ReadBuffData PartIdx:%d, BufferOffset:0x%llX, DataSize:%d\r\n", PartIdx, BufferOffset, DataSize);
    return Ret;
}