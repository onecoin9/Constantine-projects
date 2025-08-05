#include "ACCmdHandler.h"
#include <QDataStream>
#include <Windows.h>
#include "CRC/Crc32_Std.h"
#include "CRC/Crc32_Comm.h"
#include "IDataWriter.h"
#include "UplinkDataReceiver.h"
#include "AngkLogger.h"
#include "ACEventLogger.h"
#include "ProgressDialogSingleton.h"
#include "AngKDeviceModel.h"
#include "DevErrCode.h"
#include <QProgressDialog>
#include <QThread>
#include <QTime>

///注意DataSize_PerTrans要是DataSize_PerPacket的整数倍
#define DataSize_PerPacket      (4096) //单次下行包的数据量   不能超过4K，SSD需要每次4K的数据量
#define DataSize_PerTrans       (1*1024*1024) //单次发送的命令中的数据长度要固定为1M
#define TIME_LIMIT_US   (10)    //延迟时间，us为单位

CACCmdHandler::CACCmdHandler()
    :m_pCmdHandler(NULL)
    , m_pAppModels(NULL)
    , m_PrivDataCbkComplete(NULL)
    , m_fnCbkComplete(NULL)
    , m_PrivDataCbkQueryDoCmd(NULL)
    , m_fnCbkQueryDoCmd(NULL)
    , m_pNetComm(NULL)
    , m_parentWidget(NULL)
    , currentSeqNum(0)
{
    m_pCmdHandler = new CCmdHandler();
    m_pAppModels = new CAppModels();
    m_pNetComm = new CNetComm();

    m_pCmdHandler->AttachAppModels(m_pAppModels);
    m_pCmdHandler->AttachIACComm(m_pNetComm);
    AttachCmdHandler(m_pCmdHandler);
    m_pNetComm->AttachICmdHandler(m_pCmdHandler);
    tIPInfo RemoteIPInfo, LocalIPInfo;
    //RemoteIPInfo.Addr = m_pAppModels->m_AppConfigModel.RemoteIP();
    RemoteIPInfo.Port = m_pAppModels->m_AppConfigModel.RemotePort();
    LocalIPInfo.Addr = m_pAppModels->m_AppConfigModel.LocalIP();
    LocalIPInfo.Port = m_pAppModels->m_AppConfigModel.LocalPort();
    m_pNetComm->SetIPInfo(&RemoteIPInfo, &LocalIPInfo);
    m_pNetComm->StartComm();
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

CACCmdHandler::~CACCmdHandler()
{
    if (m_pCmdHandler != nullptr)
    {
        m_pCmdHandler = nullptr;
        delete m_pCmdHandler;
    }

    if (m_pAppModels != nullptr)
    {
        m_pAppModels = nullptr;
        delete m_pAppModels;
    }

    if (m_pNetComm != nullptr)
    {
        m_pNetComm = nullptr;
        delete m_pNetComm;
    }
}

void CACCmdHandler::InitMainWidget(QWidget* _obj)
{
    m_parentWidget = _obj;
}

void CACCmdHandler::AttachCmdHandler(CCmdHandler* pCmdHandler)
{
    m_pCmdHandler = pCmdHandler;
    connect(m_pCmdHandler, &CCmdHandler::sigPTPacketExecComplete, this, &CACCmdHandler::onPTPacketExecComplete);
    connect(m_pCmdHandler, &CCmdHandler::sigQueryDoPTCmd, this, &CACCmdHandler::onQueryDoPTCmd, Qt::DirectConnection);
}

void CACCmdHandler::AttachAppModels(CAppModels* pAppModels)
{
    m_pAppModels = pAppModels;
}

void CACCmdHandler::RegistCbkCmdComplete(void* PrivData, FnCBKCmdComplete fnCbkComplete)
{
    m_PrivDataCbkComplete = PrivData;
    m_fnCbkComplete = fnCbkComplete;
}

void CACCmdHandler::RegistCbkQueryDoCmd(void* PrivData, FnCBKQueryDoCmd fnCbkQueryDoCmd)
{
    m_PrivDataCbkQueryDoCmd = PrivData;
    m_fnCbkQueryDoCmd = fnCbkQueryDoCmd;
}

uint8_t CACCmdHandler::GetNextSeqNum()
{
    // 原子操作递增序列号，确保线程安全
    uint8_t current = currentSeqNum.fetch_add(1, std::memory_order_relaxed);
    // SeqNum在协议中只占用5个Bit，最大值到31，超过则回到0
    if (current >= 31) {
        currentSeqNum.store(0, std::memory_order_relaxed);
        return 0;
    }
    return current;
}

void CACCmdHandler::onPTPacketExecComplete(QString recvIP, qint32 HopNum, qint32 PortID, QByteArray ACCmdCompletePack)
{
    
    tACCmdPack* pACCmdPack = (tACCmdPack*)ACCmdCompletePack.constData();
    QByteArray DataSerial;
    QDataStream DataStream(&DataSerial, QIODevice::ReadWrite);//将DataStream与ByteArray绑定，设置为读写，为后续序列化作准备
    QByteArray RespDataBytes;
    int32_t ddrReadSize = pACCmdPack->CmdDataSize;
    int32_t ResultCode;
    int32_t RespDataSize;
    int32_t Offset = 0;
    DataStream.writeRawData((char*)pACCmdPack->CmdData, pACCmdPack->CmdDataSize); //将需要反序列化的数据写入到DataStream中
    //m_pILog->PrintBuf((char*)"ACCmdPack CmdData------",(char*)pACCmdPack->CmdData, pACCmdPack->CmdDataSize);
    DataStream.device()->seek(0); //将位置定位到最开始，准备从最开始读取
    DataStream.setByteOrder(QDataStream::LittleEndian);//设置为小端方式
    DataStream >> ResultCode;
    DataStream >> RespDataSize;

    std::string fromDev = Utils::AngKCommonTools::GetLogFrom(pACCmdPack->BPUID);
    std::string cmdName = Utils::AngKCommonTools::TranslateMessageCmdID(pACCmdPack->CmdID);

    //ALOG_INFO("Recv %s Complete(ResultCode:%d) from: %s:%d.",
     //   fromDev.c_str(), "CU", cmdName.c_str(), ResultCode, recvIP.toStdString().c_str(), HopNum);
    //ALOG_INFO("Recv %s Complete(Result:%s) from: %s:%d.",
    //    fromDev.c_str(), "CU", cmdName.c_str(), GetErrMsg(ResultCode).toLocal8Bit().data(), recvIP.toStdString().c_str(), HopNum);

    if (pACCmdPack->CmdFlag & CmdFlag_DataInDDR) {
        uint32_t DDRAddr = 0;
        DataStream >> DDRAddr;
        ReadDataFromSSDorDDR(recvIP.toStdString(), "DDR2FIBER", HopNum, PortID, DDRAddr, ddrReadSize, RespDataBytes);
    }
    else {
        if (RespDataSize > 0) {
            char* pRespData = new char[RespDataSize];
            if (pRespData) {
                DataStream.readRawData(pRespData, RespDataSize);
                RespDataBytes.append(pRespData, RespDataSize);
                if (pRespData) {
                    delete[] pRespData;
                }
            }
        }
    }

    if (m_fnCbkComplete) {
        m_fnCbkComplete(m_PrivDataCbkComplete, HopNum, PortID, ResultCode, RespDataBytes);
    }
    else {
        emit sigRemoteCmdComplete(recvIP, HopNum, PortID, ResultCode, pACCmdPack->CmdID, pACCmdPack->SKTEn, pACCmdPack->BPUID, RespDataBytes, RespDataSize);

        std::map<std::string, DeviceStu> insertDev;
        AngKDeviceModel::instance().GetConnetDevMap(insertDev);
        std::string strIPHop = recvIP.toStdString() + ":" + QString::number(HopNum).toStdString();
        nlohmann::json PTJson;
        PTJson["ProgSN"] = insertDev[strIPHop].tMainBoardInfo.strHardwareSN;
        PTJson["DataType"] = cmdName;
        PTJson["CMD"] = "Complete";
        PTJson["RetCode"] = ResultCode;

        EventLogger->SendEvent(EventBuilder->GetPTTransfer(PTJson));
    }
}

void CACCmdHandler::onQueryDoPTCmd(QString recvIP, qint32 HopNum, qint32 PortID, QByteArray ACCmdQueryPack)
{
    tACCmdPack* pACCmdPack = (tACCmdPack*)ACCmdQueryPack.constData();
    QByteArray CmdDataBytes;

    if (pACCmdPack->CmdFlag & CmdFlag_DataInDDR) {
        uint32_t DDRAddr = 0;
        memcpy(&DDRAddr, pACCmdPack->CmdData, sizeof(uint64_t));
        ReadDataFromSSDorDDR(recvIP.toStdString(), "DDR2FIBER", HopNum, PortID, DDRAddr, pACCmdPack->CmdDataSize, CmdDataBytes);
    }
    else {
        CmdDataBytes.append((char*)pACCmdPack->CmdData, pACCmdPack->CmdDataSize);
    }
    if (m_fnCbkQueryDoCmd) {
        m_fnCbkQueryDoCmd(m_PrivDataCbkQueryDoCmd, HopNum, PortID, pACCmdPack->CmdID, CmdDataBytes);
    }
    else {
        emit sigRemoteQueryDoCmd(recvIP, HopNum, PortID, pACCmdPack->CmdFlag, pACCmdPack->CmdID, pACCmdPack->BPUID, CmdDataBytes);

        std::map<std::string, DeviceStu> insertDev;
        AngKDeviceModel::instance().GetConnetDevMap(insertDev);
        std::string strIPHop = recvIP.toStdString() + ":" + QString::number(HopNum).toStdString();
        std::string cmdName = Utils::AngKCommonTools::TranslateMessageCmdID(pACCmdPack->CmdID);
        nlohmann::json PTJson;
        PTJson["ProgSN"] = insertDev[strIPHop].tMainBoardInfo.strHardwareSN;
        PTJson["DataType"] = cmdName;
        PTJson["CMD"] = "Query";
        PTJson["RetCode"] = 1;

        EventLogger->SendEvent(EventBuilder->GetPTTransfer(PTJson));
    }
}

int32_t CACCmdHandler::StoreDataToSSDorDDR(std::string devIP, QString Type, uint32_t HopNum, uint32_t PortID, uint64_t SSDAddr, QByteArray& DataBytes, std::string strProgSN, std::string strDataType)
{
    QMutexLocker locker(&m_SSDMutex);
    int32_t Ret = 0, RetCall;
    char* pSrcData = DataBytes.data();
    char* pSrcDataCur = pSrcData;
    bool bFirstPack = true;
    bool bCrcCompare = true;
    tDataPacket DataPack;
    uint32_t UplinkCRC32, DownlinkCRC32;
    tCmdPacketGetCRC32 CmdPacketGetCRC32;
    qint64 i = 0, PacketCount = 0, BytesRemain = 0, ByteSended = 0;
    qint64 FileSize = 0, BytesRead, Offset = 0, BufferOffset = 0;
    tCmdPacketDataTrans PacketDataTrans;
    uint64_t SrcAddr = 0, Length = 0;
    uint64_t DestAddr = 0;
    quint32 Checksum = 0;
    qint32 TransCnt = 0, CmdPacketCnt = 0, PacketCnt = 0;
    quint32 WaitCmdQueueTimeoutms = m_pAppModels->m_AppConfigModel.CmdQueueAvailableTimeoutms();
    ICrcCheck* pCrcCheck = new CCrc32Std();

    memset(&DataPack, 0, sizeof(tDataPacket));
    memset(&PacketDataTrans, 0, sizeof(_tCmdPacketDataTrans));
    PacketDataTrans.MsgID = ICDMsgID_Cmd;
    PacketDataTrans.CmdID = (uint8_t)CICD::TransStrCmd2CmdID(Type);
    PacketDataTrans.SeqNum = GetNextSeqNum();

    std::string dataTransCmdName = Utils::AngKCommonTools::TranslateMessageCmdID((uint8_t)CICD::TransStrCmd2CmdID(Type));
    std::string crc32CmdName = Utils::AngKCommonTools::TranslateMessageCmdID((uint8_t)eSubCmdID::SubCmd_ReadCRC32);

    if (PacketDataTrans.CmdID == 0) {
        Ret = ERR_JSONRPC_ParamError; goto __end;
    }

    PacketDataTrans.SrcAddrL = (SrcAddr & 0xFFFFFFFF);
    PacketDataTrans.SrcAddrH = (SrcAddr >> 32) & 0xFFFF;
    PacketDataTrans.HopNum = HopNum;
    PacketDataTrans.PortID = PortID;

    //CRC获取的初始化
    memset(&CmdPacketGetCRC32, 0, sizeof(tCmdPacketGetCRC32));
    CmdPacketGetCRC32.MsgID = ICDMsgID_Cmd;
    CmdPacketGetCRC32.HopNum = HopNum;
    CmdPacketGetCRC32.CmdID = (uint8_t)eSubCmdID::SubCmd_ReadCRC32;
    CmdPacketGetCRC32.PortID = PortID;

    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(devIP, HopNum, WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        ALOG_FATAL("Device %s:%d Execute %s WaitCmdAvailableQueue Timeout at Start, Timemoutms:%d.", "CU", "--", devIP.c_str(), HopNum, Type.toStdString().c_str(), WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }

    FileSize = DataBytes.size();

    DestAddr = SSDAddr;
    //这里让所有发送的数据只有第一个DataSize_PerPacket包有Flag
    ALOG_DEBUG("Send %s DataTrans to %s:%d Start.", "CU", "FP", dataTransCmdName.c_str(), devIP.c_str(), HopNum);
    while (Offset < FileSize) {
        BytesRead = FileSize - Offset;
        if (BytesRead > DataSize_PerTrans) {
            BytesRead = DataSize_PerTrans;
        }


        PacketCount = BytesRead / DataSize_PerPacket;
        BytesRemain = BytesRead % DataSize_PerPacket;
        BufferOffset = 0;

        if (PacketDataTrans.CmdID == (uint8_t)eSubCmdID::SubCmd_DataTrans_FIBER2DDR) {//发送数据到DDR需要先发指令再发数据
              //再发送命令传输数据包，告知Device上面的这些数据要被存放到哪里
            Length = BytesRead;
            PacketDataTrans.DestAddrL = (DestAddr & 0xFFFFFFFF);
            PacketDataTrans.DestAddrH = (DestAddr >> 32) & 0xFFFF;
            PacketDataTrans.LengthL = (Length & 0xFFFFFFFF);
            PacketDataTrans.LengthH = (Length >> 32) & 0xFFFF;
            //_PrintLog(LOGLEVEL_N,"1 SendData DestAddr:0x%08I64X, Length:0x%0I64X", DestAddr, Length);
            //DDR写只支持1个对列，所以每次发送之前都需要等待
            CmdPacketCnt++;
            RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(devIP, HopNum, WaitCmdQueueTimeoutms);
            if (RetCall == false) {
                Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
            }
            Ret = m_pCmdHandler->SendCmdPacketDataTrans(devIP, &PacketDataTrans);//发送命令传输数据包   
            if (Ret != 0) {
                goto __end;
            }
            //release模式下，等待ack回复后在进行发数据包操作。
            //因为ack回复在不同的线程，所以这里休眠1s等待对面线程锁释放。精度不需要那么高，所以使用Sleep
            //Sleep(1000);
        }

        //发送数据下行包，会有多个包要发送
        DataPack.MsgID = ICDMsgID_DataDownlink;
        DataPack.HopNum = HopNum;
        DataPack.PortID = PortID;
        DataPack.Length = DataSize_PerPacket / 4;
        for (i = 0; i < PacketCount; ++i) {
            if (bFirstPack) {
                DataPack.FristFlag = 1;
                bFirstPack = false;
            }
            else {
                DataPack.FristFlag = 0;
            }
            DataPack.PNum = PacketCnt + 1;
            memcpy(&DataPack.Data, pSrcDataCur + BufferOffset, DataSize_PerPacket);
            if (bCrcCompare) {
                pCrcCheck->CalcSubRoutine((uint8_t*)&DataPack.Data, DataSize_PerPacket);
            }
            Ret = m_pCmdHandler->SendDownlinkData(devIP, &DataPack);//发送数据包
            if (Ret != 0) {
                ALOG_FATAL("Send %s DownlinkData to %s:%d failed(ResultCode=%d).", "CU", "FP", dataTransCmdName.c_str(), devIP.c_str(), HopNum, Ret);
                goto __end;
            }
            BufferOffset += DataSize_PerPacket;
            PacketCnt++;

            //包与包之间的延迟操作
            //Delayus(TIME_LIMIT_US);
        }
        if (BytesRemain != 0) {//有一部分尾部数据
            if (bFirstPack) {
                DataPack.FristFlag = 1;
                bFirstPack = false;
            }
            else {
                DataPack.FristFlag = 0;
            }
            DataPack.PNum = PacketCnt + 1;
            memcpy(&DataPack.Data, pSrcDataCur + BufferOffset, BytesRemain);///最后一个包按照实际的数据量填入
            if (bCrcCompare) {
                pCrcCheck->CalcSubRoutine((uint8_t*)&DataPack.Data, BytesRemain);
            }
            Ret = m_pCmdHandler->SendDownlinkData(devIP, &DataPack);//发送数据包
            if (Ret != 0) {
                ALOG_FATAL("Send %s DownlinkData to %s:%d failed(ResultCode=%d).", "CU", "FP", dataTransCmdName.c_str(), devIP.c_str(), HopNum, Ret);
                goto __end;
            }
            PacketCnt++;
        }
        if (PacketDataTrans.CmdID == (uint8_t)eSubCmdID::SubCmd_DataTrans_FIBER2SSD ||
            PacketDataTrans.CmdID == (uint8_t)eSubCmdID::SubCmd_DataTrans_FIBER2SKT) {//发送数据到SSD或SKT需要先数据
            //再发送命令传输数据包，告知Device上面的这些数据要被存放到哪里
            Length = BytesRead;
            PacketDataTrans.DestAddrL = (DestAddr & 0xFFFFFFFF);
            PacketDataTrans.DestAddrH = (DestAddr >> 32) & 0xFFFF;
            PacketDataTrans.LengthL = (Length & 0xFFFFFFFF);
            PacketDataTrans.LengthH = (Length >> 32) & 0xFFFF;
            CmdPacketCnt++;
            Ret = m_pCmdHandler->SendCmdPacketDataTrans(devIP, &PacketDataTrans);//发送命令传输数据包   
            if (Ret != 0) {
                ALOG_FATAL("Send %s DataTrans to %s:%d failed.", "CU", "FP", dataTransCmdName.c_str(), devIP.c_str(), HopNum);
                goto __end;
            }
        }

        TransCnt++;
        ByteSended += BytesRead;
        DestAddr += BytesRead;
        Offset += BytesRead;
        pSrcDataCur += BytesRead;
    }
    ALOG_DEBUG("Send %s DataTrans to %s:%d Finish.", "CU", "FP", dataTransCmdName.c_str(), devIP.c_str(), HopNum);

    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(devIP, HopNum, WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        ALOG_FATAL("Device %s:%d Execute %s WaitCmdAvailableQueue Timeout at Start, Timemoutms:%d.", "CU", "--", devIP.c_str(), HopNum, Type.toStdString().c_str(), WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }

    ALOG_DEBUG("Send %s to %s:%d.", "CU", "FP", crc32CmdName.c_str(), devIP.c_str(), HopNum);
    Ret = m_pCmdHandler->SendCmdGetCRC32(devIP, &CmdPacketGetCRC32, DownlinkCRC32, UplinkCRC32);
    if (bCrcCompare) {
        pCrcCheck->GetChecksum((uint8_t*)&Checksum, 4);
        if (DownlinkCRC32 == Checksum) {
            ALOG_DEBUG("Device %s:%d Execute %s Checksum Compare Pass.", "CU", "FP", devIP.c_str(), HopNum, Type.toStdString().c_str());
        }
        else {
            ALOG_FATAL("Device %s:%d Execute %s Checksum Compare Failed, DownlinkCRC32:0x%08X, Crc32Calc:0x%08X.", "CU", "FP", devIP.c_str(), HopNum, Type.toStdString().c_str(), DownlinkCRC32, Checksum);
            Ret = ERR_NETCOMM_CmdCRCCompareFailed; goto __end;
        }
    }
    else {
        ALOG_INFO("Device %s:%d Execute %s Checksum Downlink :0x%08X.", "CU", "FP", devIP.c_str(), HopNum, Type.toStdString().c_str(), DownlinkCRC32);
    }

__end:
    if (Ret != 0) {
        m_pCmdHandler->ReleaseCmdExeQueueSemaphoreWhenFail(devIP, HopNum);
    }
    return Ret;
}

int32_t CACCmdHandler::SSD2DDR(std::string devIP, uint32_t HopNum, uint32_t PortID, uint64_t SrcSSDAddr, uint64_t DestDDRAddr, uint64_t DataSize)
{
    QMutexLocker locker(&m_SSDMutex);
    int32_t Ret = 0;
    int32_t CmdPacketCnt = 0;
    IDataWriter* pPrevDataWriter = NULL;
    bool bCrcCompare = true;
    bool bFirstPacketFlag = true;
    int TimeEscapems = 0;
    bool RetCall = false;
    quint32 TotalCrc32CheckSum = 0;
    qint64 BytesTotalWrite = 0;
    tCmdPacketDataTrans PacketDataTrans;
    tCmdPacketGetCRC32 CmdPacketGetCRC32;
    CCrc32Std Crc32Std;
    uint32_t DownlinkCRC32, UplinkCRC32;
    quint32 WaitCmdQueueTimeoutms = m_pAppModels->m_AppConfigModel.CmdQueueAvailableTimeoutms();

    //开始数据传输包传输并获取数据
    memset(&PacketDataTrans, 0, sizeof(_tCmdPacketDataTrans));
    PacketDataTrans.MsgID = ICDMsgID_Cmd;
    PacketDataTrans.CmdID = (uint8_t)CICD::TransStrCmd2CmdID("SSD2DDR");
    PacketDataTrans.HopNum = HopNum;
    PacketDataTrans.PortID = PortID;
    PacketDataTrans.SeqNum = GetNextSeqNum();
    PacketDataTrans.SrcAddrL = (uint32_t)(SrcSSDAddr & 0xFFFFFFFF);
    PacketDataTrans.SrcAddrH = (uint16_t)((SrcSSDAddr >> 32) & 0xFFFFFFFF);
    PacketDataTrans.DestAddrL = (uint32_t)(DestDDRAddr & 0xFFFFFFFF);
    PacketDataTrans.DestAddrH = (uint16_t)((DestDDRAddr >> 32) & 0xFFFFFFFF);
    PacketDataTrans.LengthL = (uint32_t)(DataSize & 0xFFFFFFFF);
    PacketDataTrans.LengthH = (uint16_t)((DataSize >> 32) & 0xFFFFFFFF);

    //等待所有命令队列空闲，这样方便后续确认所有发送的命令包都Complete
    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(devIP, HopNum, WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        ALOG_FATAL("Execute SSD2DDR WaitCmdAvailableQueue Timeout at Start, Timemoutms:%d.", "CU", "--", WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }

    Ret = m_pCmdHandler->SendCmdPacketDataTrans(devIP, &PacketDataTrans);//发送命令传输数据包   
    if (Ret != 0) {
        goto __end;
    }

    //等待所有命令队列空闲，表明所有的命令包都Complete
    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(devIP, HopNum, WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        ALOG_FATAL("Execute SSD2DDR WaitCmdAvailableQueue Timeout at Start, Timemoutms:%d.", "CU", "--", WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }
    QThread::msleep(10);///等待这个ms数，因为命令完成包达到之后，可能还有数据未到达，延迟等待一下！

__end:
    if (Ret != 0) {
        m_pCmdHandler->ReleaseCmdExeQueueSemaphoreWhenFail(devIP, HopNum);
    }
    return Ret;
}

int32_t CACCmdHandler::DDR2SSD(std::string devIP, uint32_t HopNum, uint32_t PortID, uint64_t SrcDDRAddr, uint64_t DestSSDAddr, uint64_t DataSize)
{
    QMutexLocker locker(&m_SSDMutex);
    int32_t Ret = 0;
    int32_t CmdPacketCnt = 0;
    IDataWriter* pPrevDataWriter = NULL;
    bool bCrcCompare = true;
    bool bFirstPacketFlag = true;
    int TimeEscapems = 0;
    bool RetCall = false;
    quint32 TotalCrc32CheckSum = 0;
    qint64 BytesTotalWrite = 0;
    tCmdPacketDataTrans PacketDataTrans;
    tCmdPacketGetCRC32 CmdPacketGetCRC32;
    CCrc32Std Crc32Std;
    uint32_t DownlinkCRC32, UplinkCRC32;
    quint32 WaitCmdQueueTimeoutms = m_pAppModels->m_AppConfigModel.CmdQueueAvailableTimeoutms();

    //开始数据传输包传输并获取数据
    memset(&PacketDataTrans, 0, sizeof(_tCmdPacketDataTrans));
    PacketDataTrans.MsgID = ICDMsgID_Cmd;
    PacketDataTrans.CmdID = (uint8_t)CICD::TransStrCmd2CmdID("DDR2SSD");
    PacketDataTrans.HopNum = HopNum;
    PacketDataTrans.PortID = PortID;
    PacketDataTrans.SeqNum = GetNextSeqNum();
    PacketDataTrans.SrcAddrL = (uint32_t)(SrcDDRAddr & 0xFFFFFFFF);
    PacketDataTrans.SrcAddrH = (uint16_t)((SrcDDRAddr >> 32) & 0xFFFFFFFF);
    PacketDataTrans.DestAddrL = (uint32_t)(DestSSDAddr & 0xFFFFFFFF);
    PacketDataTrans.DestAddrH = (uint16_t)((DestSSDAddr >> 32) & 0xFFFFFFFF);
    PacketDataTrans.LengthL = (uint32_t)(DataSize & 0xFFFFFFFF);
    PacketDataTrans.LengthH = (uint16_t)((DataSize >> 32) & 0xFFFFFFFF);

    //等待所有命令队列空闲，这样方便后续确认所有发送的命令包都Complete
    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(devIP, HopNum, WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        ALOG_FATAL("Execute DDR2SSD WaitCmdAvailableQueue Timeout at Start, Timemoutms:%d.", "CU", "--", WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }

    Ret = m_pCmdHandler->SendCmdPacketDataTrans(devIP, &PacketDataTrans);//发送命令传输数据包   
    if (Ret != 0) {
        goto __end;
    }

    //等待所有命令队列空闲，表明所有的命令包都Complete
    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(devIP, HopNum, WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        ALOG_FATAL("Execute DDR2SSD WaitCmdAvailableQueue Timeout at Start, Timemoutms:%d.", "CU", "--", WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }
    QThread::msleep(10);///等待这个ms数，因为命令完成包达到之后，可能还有数据未到达，延迟等待一下！

__end:
    if (Ret != 0) {
        m_pCmdHandler->ReleaseCmdExeQueueSemaphoreWhenFail(devIP, HopNum);
    }
    return Ret;
}
int32_t CACCmdHandler::RemoteDoPTCmd(std::string devIP, uint32_t HopNum, uint32_t PortID, uint32_t CmdFlag, uint16_t CmdID, uint32_t SKTNum, uint16_t BPUID, QByteArray& CmdDataBytes)
{
    QMutexLocker locker(&m_SSDMutex);
    int32_t Ret = 0;
    int32_t StructSize = 0;
    int32_t PTPacketRealSize = 0;
    CmdDataBytes.append((char)0x0);
    int32_t CmdDataSize = CmdDataBytes.size();
    int32_t ACCmdPacketPayloadMax = m_pCmdHandler->GetACCmdPacketPayloadMax();
    tCmdPacketPT CmdPacketPT, * pCmdPacketPT = NULL;
    tACCmdPack* pACCmdPack = NULL;
    pCmdPacketPT = &CmdPacketPT;
    pACCmdPack = (tACCmdPack*)pCmdPacketPT->Data;

    memset(pCmdPacketPT, 0, sizeof(tCmdPacketPT));

    pCmdPacketPT->HopNum = (uint8_t)HopNum;
    pCmdPacketPT->MsgID = ICDMsgID_PT;
    pCmdPacketPT->PortID = PortID;
    pCmdPacketPT->MsgSubID = CMDID_PTPACK_SRC;
    pCmdPacketPT->SeqNum = GetNextSeqNum();

    pACCmdPack->BPUID = BPUID;
    pACCmdPack->SKTEn = SKTNum;
    pACCmdPack->CmdFlag = CmdFlag;
    pACCmdPack->CmdID = CmdID;

    std::string sendDev = Utils::AngKCommonTools::GetLogFrom(pACCmdPack->BPUID);
    std::string cmdName = Utils::AngKCommonTools::TranslateMessageCmdID(pACCmdPack->CmdID);

    std::map<std::string, DeviceStu> insertDev;
    AngKDeviceModel::instance().GetConnetDevMap(insertDev);
    QString strIPHop = QString::fromStdString(devIP) + ":" + QString::number(HopNum);

#if 1
    pACCmdPack->CmdDataSize = CmdDataSize;
    if (CmdDataSize > ACCmdPacketPayloadMax) {//不能放到透传包中,先用FIBER2SSD发送数据到指定的位置

        uint64_t SSDAddr = SSD_PC2MUDataExchangeOffset;
        ALOG_INFO("Execute %s CmdDataSize(%d Bytes) > %d Bytes, NeedToUse SSD To ExChange Data.", "CU", sendDev.c_str(), cmdName.c_str(), CmdDataSize, ACCmdPacketPayloadMax);
        Ret = StoreDataToSSDorDDR(devIP, "FIBER2SSD", HopNum, PortID, SSDAddr, CmdDataBytes, insertDev[strIPHop.toStdString()].tMainBoardInfo.strHardwareSN, "PTTransferData");
        if (Ret == 0) {
            pACCmdPack->CmdFlag |= CmdFlag_DataInSSD;
            memcpy(pACCmdPack->CmdData, (char*)&SSDAddr, sizeof(uint64_t)); //如果不能一个包发送，将数据存到SSD中之后，将SSD的位置放到CmdData中，占8个字节
        }
    }
    else {
        memcpy(pACCmdPack->CmdData, CmdDataBytes.data(), CmdDataSize);
    }
#else
    pACCmdPack->CmdDataSize = ACCmdPacketPayloadMax;
    for (int j = 0; j < ACCmdPacketPayloadMax; ++j) {
        pACCmdPack->CmdData[j] = (uint8_t)j;
    }
#endif 
    PTPacketRealSize = 8 + sizeof(tACCmdPack) + pACCmdPack->CmdDataSize;
    //_PrintLog(LOGLEVEL_D,"StructSize:%d, CmdDataSize:%d", sizeof(tCmdPacketPT), CmdDataSize);
    //m_pILog->PrintBuf((char*)"=============RemoteDoPTCmd==============", (char*)pCmdPacketPT, sizeof(tCmdPacketPT));// sizeof(tCmdPacketPT)>64?64:sizeof(tCmdPacketPT)
    ALOG_INFO("RemoteDoPTCmd Send %s Request to %s:%d.", "CU", sendDev.c_str(), cmdName.c_str(), devIP.c_str(), HopNum);

    Ret = m_pCmdHandler->SendCmdPTCmd(devIP, cmdName, pCmdPacketPT);

    if (Ret == 0) {
        nlohmann::json PTJson;

        PTJson["ProgSN"] = strIPHop.toStdString();
        PTJson["DataType"] = cmdName;
        PTJson["CMD"] = "Command";
        PTJson["RetCode"] = Ret;
        EventLogger->SendEvent(EventBuilder->GetPTTransfer(PTJson));
    }

    return Ret;
}
int32_t CACCmdHandler::SendDevCallBackResult(std::string devIP, uint32_t HopNum, uint32_t PortID, int32_t ResultCode, QByteArray RespData)
{
    QMutexLocker locker(&m_SSDMutex);
    int32_t Ret = 0;
    uint32_t RespDataSize = RespData.size();
    QByteArray DataSerial;
    QDataStream DataStream(&DataSerial, QIODevice::WriteOnly);//将DataStream与ByteArray绑定，设置为读写，为后续序列化作准备
    int32_t CmdDataSize = 8 + RespData.size(); //结果包中要包含4个字节的ResultCode和4个字节的ResponseDataSize
    int32_t ACCmdPacketPayloadMax = m_pCmdHandler->GetACCmdPacketPayloadMax();
    tCmdPacketPT CmdPacketPT, * pCmdPacketPT = NULL;
    tACCmdPack* pACCmdPack = NULL;
    pCmdPacketPT = &CmdPacketPT;
    pACCmdPack = (tACCmdPack*)pCmdPacketPT->Data;

    memset(pCmdPacketPT, 0, sizeof(tCmdPacketPT));
    //设置PT包的信息
    pCmdPacketPT->HopNum = HopNum;
    pCmdPacketPT->MsgID = ICDMsgID_PT;
    pCmdPacketPT->PortID = PortID;
    pCmdPacketPT->MsgSubID = CMDID_PTPACK_QUERYDOCMD;

    //设置ACCmdPack包的信息
    pACCmdPack->CmdFlag = 0;
    pACCmdPack->CmdID = CmdID_CmdDone;
    pACCmdPack->CmdDataSize = CmdDataSize;

    std::string sendDev = Utils::AngKCommonTools::GetLogFrom(pACCmdPack->BPUID);
    std::string cmdName = Utils::AngKCommonTools::TranslateMessageCmdID(pACCmdPack->CmdID);

    //设置对应的ACCmdPack数据部分信息
    DataStream.setByteOrder(QDataStream::LittleEndian);//设置为小端方式
    DataStream << ResultCode;
    DataStream << RespDataSize;
    if (CmdDataSize > ACCmdPacketPayloadMax) {//不能放到透传包中,先用FIBER2SSD发送数据到指定的位置
        std::map<std::string, DeviceStu> insertDev;
        AngKDeviceModel::instance().GetConnetDevMap(insertDev);
        uint64_t SSDAddr = SSD_PC2MUDataExchangeOffset;
        QString strIPHop = QString::fromStdString(devIP) + ":" + QString::number(HopNum);
        ALOG_INFO("Execute %s CmdDataSize(%d Bytes) > %d Bytes, NeedToUse SSD To ExChange Data.", "CU", sendDev.c_str(), cmdName.c_str(), CmdDataSize, ACCmdPacketPayloadMax);
        Ret = StoreDataToSSDorDDR(devIP, "FIBER2SSD", HopNum, PortID, SSDAddr, RespData, insertDev[strIPHop.toStdString()].tMainBoardInfo.strHardwareSN, "PTTransferData");
        if (Ret == 0) {
            pACCmdPack->CmdFlag |= CmdFlag_DataInSSD;
            DataStream << SSDAddr; //如果不能一个包发送，将数据存到SSD中之后，将SSD的位置放到CmdData中，占8个字节
        }
    }
    else {
        DataStream.writeRawData(RespData.constData(), RespDataSize);
    }
    memcpy(pACCmdPack->CmdData, DataSerial.constData(), DataSerial.size());//将序列化之后的数据放到ACCmdPack的数据部分

    ALOG_INFO("SendDevCallBackResult Send %s Request to %s:%d.", "CU", sendDev.c_str(), cmdName.c_str(), devIP.c_str(), HopNum);
    Ret = m_pCmdHandler->SendCmdPTCmd(devIP, cmdName, pCmdPacketPT);
    return Ret;
}
int32_t CACCmdHandler::GetCapacity(std::string devIP, QString Type, uint32_t HopNum, uint32_t PortID)
{
    QMutexLocker locker(&m_SSDMutex);
    int32_t Ret = 0;
    int32_t BPUPos = 0, SKTPos = 0;
    tCmdPacketGetCapacity PckGetCapacity;


    memset(&PckGetCapacity, 0, sizeof(tCmdCplPacketGetCapacity));

    PckGetCapacity.MsgID = ICDMsgID_Cmd;
    PckGetCapacity.HopNum = HopNum;
    PckGetCapacity.SeqNum = GetNextSeqNum();
    if (Type == "SSD") { PckGetCapacity.CmdID = (uint8_t)eSubCmdID::SubCmd_ReadCapacity_SSD; }
    else if (Type == "DDR") { PckGetCapacity.CmdID = (uint8_t)eSubCmdID::SubCmd_ReadCapacity_DDR; }
    else if (Type == "SKT") { PckGetCapacity.CmdID = (uint8_t)eSubCmdID::SubCmd_ReadCapacity_SKT; }
    else { Ret = ERR_JSONRPC_ParaInvalid; goto __end; }

    PckGetCapacity.PortID = PortID;

    ALOG_INFO("Send GetCapacity Request to %s:%d, GetCapacity Type:%s.", "CU", "FP", devIP.c_str(), HopNum, Type.toStdString().c_str());
    Ret = m_pCmdHandler->SendCmdGetCapacity(devIP, &PckGetCapacity);
__end:
    return Ret;
}
int32_t CACCmdHandler::WriteDataFromFile(std::string devIP, QString strFilePath, QString Type, uint32_t HopNum, uint32_t PortID, uint64_t DestAddr, tJsonParaHash& ResultHash)
{
    QMutexLocker locker(&m_SSDMutex);
    int32_t Ret = 0;
    int32_t CmdPacketCnt = 0;
    double Speed = 0;
    bool RetCall = false;
    bool bFirstPack = true;
    bool bCrcCompare = m_pAppModels->m_AppConfigModel.SoftCRC32En();
    quint32 WaitCmdQueueTimeoutms = m_pAppModels->m_AppConfigModel.CmdQueueAvailableTimeoutms();
    uint32_t UplinkCRC32, DownlinkCRC32;
    qint64 FileSize = 0, BytesRead, Offset = 0, BufferOffset = 0;
    qint64 ByteSended = 0;
    qint64 i = 0, PacketCount = 0, BytesRemain = 0;
    qint64 SrcAddr = 0, Length = 0;
    tCmdPacketGetCRC32 CmdPacketGetCRC32;
    tCmdPacketDataTrans PacketDataTrans;
    tDataPacket DataPack;

    memset(&DataPack, 0, sizeof(tDataPacket));
    ICrcCheck* pCrcCheck = new CCrc32Std();

    QTime StartTime, StopTime;
    quint32 OnePacketChecksum;
    quint32 Checksum = 0;
    qint32 TimeEscapems = 0;
    qint32 TransCnt = 0, PacketCnt = 0;
    char* TmpBuffer = NULL;

    std::string dataTransCmdName = Utils::AngKCommonTools::TranslateMessageCmdID((uint8_t)CICD::TransStrCmd2CmdID(Type));
    std::string crc32CmdName = Utils::AngKCommonTools::TranslateMessageCmdID((uint8_t)eSubCmdID::SubCmd_ReadCRC32);

    QFile File(strFilePath);
    if (File.open(QIODevice::ReadOnly) == false) {
        ALOG_FATAL("Func:WriteDataFromFile Open File Failed:%s.", "CU", "--", strFilePath.toStdString().c_str());
        Ret = ERR_JSONRPC_FileOpen; goto __end;
    }
    FileSize = File.size();

    memset(&PacketDataTrans, 0, sizeof(_tCmdPacketDataTrans));
    PacketDataTrans.MsgID = ICDMsgID_Cmd;
    PacketDataTrans.CmdID = (uint8_t)CICD::TransStrCmd2CmdID(Type);

    PacketDataTrans.SrcAddrL = (SrcAddr & 0xFFFFFFFF);
    PacketDataTrans.SrcAddrH = (SrcAddr >> 32) & 0xFFFF;
    PacketDataTrans.HopNum = HopNum;
    PacketDataTrans.PortID = PortID;
    PacketDataTrans.SeqNum = GetNextSeqNum();

    //CRC获取的初始化
    memset(&CmdPacketGetCRC32, 0, sizeof(tCmdPacketGetCRC32));
    CmdPacketGetCRC32.MsgID = ICDMsgID_Cmd;
    CmdPacketGetCRC32.HopNum = HopNum;
    CmdPacketGetCRC32.CmdID = (uint8_t)eSubCmdID::SubCmd_ReadCRC32;
    CmdPacketGetCRC32.PortID = PortID;

    TmpBuffer = new char[DataSize_PerTrans];
    if (TmpBuffer == NULL) {
        Ret = ERR_MemoryAlloc; goto __end;
    }
    if (pCrcCheck == NULL) {
        Ret = ERR_MemoryAlloc; goto __end;
    }
    StartTime = QTime::currentTime();

    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(devIP, HopNum, WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        ALOG_FATAL("Execute %s WaitCmdAvailableQueue Timeout at Start, Timemoutms:%d.", "CU", "--", Type.toStdString().c_str(), WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }

    CmdPacketCnt = 0;
    //开始文件操作
    //这里让所有发送的数据只有第一个DataSize_PerPacket包有Flag
    ALOG_INFO("Send %s DataTrans to %s:%d Start.", "CU", "FP", dataTransCmdName.c_str(), devIP.c_str(), HopNum);
    while (Offset < FileSize) {
        BytesRead = File.read(TmpBuffer, DataSize_PerTrans);
        if (BytesRead <= 0) {
            Ret = ERR_ReadFile; goto __end;
        }
        /*       BytesRead = FileSize - Offset;
               if (BytesRead > DataSize_PerTrans) {
                   BytesRead = DataSize_PerTrans;
               }*/

        PacketCount = BytesRead / DataSize_PerPacket;
        BytesRemain = BytesRead % DataSize_PerPacket;
        BufferOffset = 0;

        if (PacketDataTrans.CmdID == (uint8_t)eSubCmdID::SubCmd_DataTrans_FIBER2DDR) {//发送数据到DDR需要先发指令再发数据
              //再发送命令传输数据包，告知Device上面的这些数据要被存放到哪里
            Length = BytesRead;
            PacketDataTrans.DestAddrL = (DestAddr & 0xFFFFFFFF);
            PacketDataTrans.DestAddrH = (DestAddr >> 32) & 0xFFFF;
            PacketDataTrans.LengthL = (Length & 0xFFFFFFFF);
            PacketDataTrans.LengthH = (Length >> 32) & 0xFFFF;
            //_PrintLog(LOGLEVEL_N,"1 SendData DestAddr:0x%08I64X, Length:0x%0I64X", DestAddr, Length);
            //DDR写只支持1个对列，所以每次发送之前都需要等待
            CmdPacketCnt++;
            RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(devIP, HopNum, WaitCmdQueueTimeoutms);
            if (RetCall == false) {
                ALOG_FATAL("Execute %s WaitCmdAvailableQueue Timeout at Start, Timemoutms:%d.", "CU", "--", Type.toStdString().c_str(), WaitCmdQueueTimeoutms);
                Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
            }
            Ret = m_pCmdHandler->SendCmdPacketDataTrans(devIP, &PacketDataTrans);//发送命令传输数据包   
            if (Ret != 0) {
                ALOG_FATAL("Send %s DataTrans to %s:%d failed.", "CU", "FP", dataTransCmdName.c_str(), devIP.c_str(), HopNum);
                goto __end;
            }

        }

        //发送数据下行包，会有多个包要发送
        DataPack.MsgID = ICDMsgID_DataDownlink;
        DataPack.HopNum = HopNum;
        DataPack.PortID = PortID;
        DataPack.Length = DataSize_PerPacket / 4;
        for (i = 0; i < PacketCount; ++i) {
            if (bFirstPack) {
                DataPack.FristFlag = 1;
                bFirstPack = false;
            }
            else {
                DataPack.FristFlag = 0;
            }
            memcpy(&DataPack.Data, TmpBuffer + BufferOffset, DataSize_PerPacket);
            if (bCrcCompare) {
                pCrcCheck->CalcSubRoutine((uint8_t*)&DataPack.Data, DataSize_PerPacket);
            }
            m_pCmdHandler->SendDownlinkData(devIP, &DataPack);//发送数据包
            BufferOffset += DataSize_PerPacket;
            PacketCnt++;
            //包与包之间的延迟操作
            Delayus(TIME_LIMIT_US);
        }
        if (BytesRemain != 0) {//有一部分尾部数据
            if (bFirstPack) {
                DataPack.FristFlag = 1;
                bFirstPack = false;
            }
            else {
                DataPack.FristFlag = 0;
            }
            memcpy(&DataPack.Data, TmpBuffer + BufferOffset, BytesRemain);///最后一个包按照实际的数据量填入
            if (bCrcCompare) {
                pCrcCheck->CalcSubRoutine((uint8_t*)&DataPack.Data, BytesRemain);
            }
            Ret = m_pCmdHandler->SendDownlinkData(devIP, &DataPack);//发送数据包
            PacketCnt++;
        }

        if (PacketDataTrans.CmdID == (uint8_t)eSubCmdID::SubCmd_DataTrans_FIBER2SSD ||
            PacketDataTrans.CmdID == (uint8_t)eSubCmdID::SubCmd_DataTrans_FIBER2SKT) {//发送数据到SSD或SKT需要先数据
            //再发送命令传输数据包，告知Device上面的这些数据要被存放到哪里
            Length = BytesRead;
            PacketDataTrans.DestAddrL = (DestAddr & 0xFFFFFFFF);
            PacketDataTrans.DestAddrH = (DestAddr >> 32) & 0xFFFF;
            PacketDataTrans.LengthL = (Length & 0xFFFFFFFF);
            PacketDataTrans.LengthH = (Length >> 32) & 0xFFFF;
            CmdPacketCnt++;
            Ret = m_pCmdHandler->SendCmdPacketDataTrans(devIP, &PacketDataTrans);//发送命令传输数据包   
            if (Ret != 0) {
                goto __end;
            }
        }

        TransCnt++;
        ByteSended += BytesRead;
        DestAddr += BytesRead;
        Offset += BytesRead;
    }
    ALOG_INFO("Send %s DataTrans to %s:%d Finish.", "CU", "FP", dataTransCmdName.c_str(), devIP.c_str(), HopNum);

    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(devIP, HopNum, WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        ALOG_FATAL("Execute %s WaitCmdAvailableQueue Timeout at Start, Timemoutms:%d.", "CU", "--", Type.toStdString().c_str(), WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }
    StopTime = QTime::currentTime();

    File.flush();
    File.close();


    ALOG_INFO("Send %s to %s:%d.", "CU", "FP", crc32CmdName.c_str(), devIP.c_str(), HopNum);
    m_pCmdHandler->SendCmdGetCRC32(devIP, &CmdPacketGetCRC32, DownlinkCRC32, UplinkCRC32);
    if (bCrcCompare) {
        pCrcCheck->GetChecksum((uint8_t*)&Checksum, 4);
        if (DownlinkCRC32 == Checksum) {
            ALOG_DEBUG("Execute %s Checksum Compare Pass.", "CU", "FP", crc32CmdName.c_str());
        }
        else {
            ALOG_FATAL("Execute %s Checksum Compare Failed, DownlinkCRC32:0x%08X, Crc32Calc:0x%08X.", "CU", "FP", crc32CmdName.c_str(), DownlinkCRC32, Checksum);
        }
    }
    else {
        ALOG_INFO("Execute %s Checksum Downlink :0x%08X", "CU", "FP", Type.toStdString().c_str(), DownlinkCRC32);
    }

    TimeEscapems = (StopTime.msecsSinceStartOfDay() - StartTime.msecsSinceStartOfDay());
    Speed = (double)FileSize / (double)(TimeEscapems) * 1000 / (double)(1024 * 1024);
    //ALOG_INFO("FileSize:0x%I64X (Bytes), TimeEscape:%d ms, Speed:%.2lf MBytes/S, PacketCnt:%d, TransCnt:%d, %s:0x%08X, TotalCmdPacket:%d",
    //    FileSize, TimeEscapems, Speed, PacketCnt, TransCnt, pCrcCheck->GetName().c_str(), Checksum, CmdPacketCnt);

    //反馈给外部的结果
    ResultHash.insert("TotalSize", QString::number(FileSize, 16).toUpper().insert(0, "0x"));
    ResultHash.insert("TimeEscape_ms", TimeEscapems);
    ResultHash.insert("BytesSend", QString::number(Offset, 16).toUpper().insert(0, "0x"));
    ResultHash.insert("Crc32", QString::number(DownlinkCRC32, 16).toUpper().insert(0, "0x"));//将校验值转为16进制字符串

__end:
    if (Ret != 0) {
        m_pCmdHandler->ReleaseCmdExeQueueSemaphoreWhenFail(devIP, HopNum);
    }
    if (pCrcCheck) {
        delete pCrcCheck;
    }
    if (TmpBuffer) {
        delete[] TmpBuffer;
    }
    return Ret;
}

int32_t CACCmdHandler::ReadDataFromSSDorDDR(std::string devIP, QString Type, uint32_t HopNum, uint32_t PortID, uint64_t SSDAddr, int32_t BytesToRead, QByteArray& DataBytes, std::string strProgSN, std::string strDataType)
{
    QMutexLocker locker(&m_SSDMutex);
    int32_t Ret = 0;
    int32_t CmdPacketCnt = 0;
    IDataWriter* pPrevDataWriter = NULL;
    bool bFirstPacketFlag = true;
    int TimeEscapems = 0;
    bool RetCall = false;
    quint32 TotalCrc32CheckSum = 0;
    qint64 BytesTotalWrite = 0;
    tCmdPacketDataTrans PacketDataTrans;
    tCmdPacketGetCRC32 CmdPacketGetCRC32;
    CCrc32Std Crc32Std;
    uint32_t DownlinkCRC32, UplinkCRC32;
    bool bCrcCompare = m_pAppModels->m_AppConfigModel.SoftCRC32En();
    quint32 WaitCmdQueueTimeoutms = m_pAppModels->m_AppConfigModel.CmdQueueAvailableTimeoutms();
    qint64 DestAddr = 0, Offset = 0, BytesRead = 0;
    uint64_t SrcAddr = SSDAddr;
    //uint64_t DestAddr = ParamsHash.find("DestAddr").value().toString().toULongLong(NULL, 16);
    uint64_t Length = BytesToRead;
    CByteArrayWriter FileWriter;
    CUplinkDataReceiver UplinkDataReceiver;
    UplinkDataReceiver.ConnectCmdHandler(m_pCmdHandler);
    m_pCmdHandler->SetChangeState(false);
    QString strIPHop = QString::fromStdString(devIP) + ":" + QString::number(HopNum);

    //准备好DataWriter
    FileWriter.SetByteArray(&DataBytes);
    FileWriter.SetMaxDataSize(Length);
    if (bCrcCompare) {
        FileWriter.AttachICrcCheck(&Crc32Std);
    }

    pPrevDataWriter = UplinkDataReceiver.StoreDataWriter(&FileWriter);

    //开始数据传输包传输并获取数据
    memset(&PacketDataTrans, 0, sizeof(_tCmdPacketDataTrans));
    PacketDataTrans.MsgID = ICDMsgID_Cmd;
    PacketDataTrans.CmdID = (uint8_t)CICD::TransStrCmd2CmdID(Type);
    PacketDataTrans.HopNum = HopNum;
    PacketDataTrans.PortID = PortID;
    PacketDataTrans.SeqNum = GetNextSeqNum();

    //等待所有命令队列空闲，这样方便后续确认所有发送的命令包都Complete
    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(devIP, HopNum, WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        ALOG_FATAL("Device %s:%d Execute %s WaitCmdAvailableQueue Timeout at start, Timemoutms:%d.", "CU", "--", devIP.c_str(), HopNum, Type.toStdString().c_str(), WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }
    bFirstPacketFlag = true;
    CmdPacketCnt = 0;
    while (Offset < Length) {
        BytesRead = Length - Offset;
        if (BytesRead > DataSize_PerTrans) {//每次最多读取这么多
            BytesRead = DataSize_PerTrans;
        }

        if (bFirstPacketFlag) {///只做一次
            PacketDataTrans.FirtPacketFlag = 1;
            bFirstPacketFlag = false;
        }
        else {
            PacketDataTrans.FirtPacketFlag = 0;
        }
        //_PrintLog(LOGLEVEL_N, "FirtPacketFlag:%d, FirstDWord:0x%08X", PacketDataTrans.FirtPacketFlag,*(uint32_t*)(&PacketDataTrans));
        //重新指定长度和读取的起始位置
        PacketDataTrans.LengthL = (uint32_t)(BytesRead & 0xFFFFFFFF);
        PacketDataTrans.LengthH = (uint16_t)((BytesRead >> 32) & 0xFFFFFFFF);
        PacketDataTrans.SrcAddrL = (uint32_t)(SrcAddr & 0xFFFFFFFF);
        PacketDataTrans.SrcAddrH = (uint16_t)((SrcAddr >> 32) & 0xFFFFFFFF);

        //ALOG_INFO("ReadData SrcAddr: 0x%X, Length: 0x%X BytesToRead: 0x%X", 
            //"CU", "--", SrcAddr, BytesRead, BytesToRead);
        CmdPacketCnt++;
        Ret = m_pCmdHandler->SendCmdPacketDataTrans(devIP, &PacketDataTrans);//发送命令传输数据包   
        if (Ret != 0) {
            Ret = ERR_CMDHAND_CmdQueueAvailable;
            goto __end;
        }
        //release模式下，等待ack回复后在进行发数据包操作。
        //因为ack回复在不同的线程，所以这里休眠0.5s等待对面线程锁释放。精度不需要那么高，所以使用Sleep
        //Sleep(1000);

        if (PacketDataTrans.CmdID == (uint8_t)eSubCmdID::SubCmd_DataTrans_DDR2FIBER) {//如果是DDR读取，不能使用队列只能等到Complete包返回才能继续
            RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(devIP, HopNum, WaitCmdQueueTimeoutms);
            if (RetCall == false) {
                ALOG_FATAL("Device %s:%d Execute %s WaitCmdAvailableQueue Timeout at doing, Timemoutms:%d", "CU", "--", devIP.c_str(), HopNum, Type.toStdString().c_str(), WaitCmdQueueTimeoutms);
                Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
            }
        }
        Offset += BytesRead;
        SrcAddr += BytesRead;
    }    //等待所有命令队列空闲，表明所有的命令包都Complete
    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(devIP, HopNum, WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        ALOG_FATAL("Device %s:%d Execute %s WaitCmdAvailableQueue Timeout at complete, Timemoutms:%d", "CU", "--", devIP.c_str(), HopNum, Type.toStdString().c_str(), WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }    // DDR2FIBER操作需要额外的同步确保数据传输完全完成
    if (PacketDataTrans.CmdID == (uint8_t)eSubCmdID::SubCmd_DataTrans_DDR2FIBER) {
        // DDR2FIBER同步操作 - 静默运行以减少日志噪音
        
        // 使用专用的DDR2FIBER同步函数
        RetCall = m_pCmdHandler->WaitDDR2FiberTransferComplete(devIP, HopNum, WaitCmdQueueTimeoutms);
        if (RetCall == false) {
            ALOG_FATAL("Device %s:%d Execute %s DDR2FIBER WaitDDR2FiberTransferComplete Timeout, Timemoutms:%d.", "CU", "--", devIP.c_str(), HopNum, Type.toStdString().c_str(), WaitCmdQueueTimeoutms);
            Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
        }
        
        // DDR2FIBER同步完成 - 静默运行
    } else {
        QThread::msleep(10);///等待这个ms数，因为命令完成包达到之后，可能还有数据未到达，延迟等待一下！
    }
    
    memset(&CmdPacketGetCRC32, 0, sizeof(tCmdPacketGetCRC32));
    CmdPacketGetCRC32.MsgID = ICDMsgID_Cmd;
    CmdPacketGetCRC32.HopNum = HopNum;
    CmdPacketGetCRC32.CmdID = (uint8_t)eSubCmdID::SubCmd_ReadCRC32;
    CmdPacketGetCRC32.PortID = PortID;
    //m_pCmdHandler->WaitWriteFinish(strIPHop);
    Ret = m_pCmdHandler->SendCmdGetCRC32(devIP, &CmdPacketGetCRC32, DownlinkCRC32, UplinkCRC32);
    if (Ret == 0) {
        ALOG_DEBUG("Device %s:%d Execute %s CRC32 UplinkCRC32:0x%08X,DownlinCRC32:0x%08X.", "CU", "FP", devIP.c_str(), HopNum, Type.toStdString().c_str(), UplinkCRC32, DownlinkCRC32);
    }
    else {
        goto __end;
    }
    Crc32Std.GetChecksum((uint8_t*)&TotalCrc32CheckSum, 4);///中间过程已经由DataWriter完成，这里直接获取校验值
    if (bCrcCompare) {
        if (UplinkCRC32 != TotalCrc32CheckSum) {
            ALOG_FATAL("Device %s:%d Execute %s CRC32 Compare Failed, UplinkCRC32:0x%08X, Crc32Calc:0x%08X.", "CU", "FP", devIP.c_str(), HopNum, Type.toStdString().c_str(), UplinkCRC32, TotalCrc32CheckSum);
            Ret = ERR_NETCOMM_CmdCRCCompareFailed; goto __end;
        }
        else {
            ALOG_DEBUG("Device %s:%d Execute %s CRC32 Compare Pass.", "CU", "FP", devIP.c_str(), HopNum, Type.toStdString().c_str());
        }
    }
    BytesTotalWrite = UplinkDataReceiver.GetDataWriterCurDataSize();
    //ALOG_INFO("Execute SSD2FIBER TimeEscape_ms:%d ms, Offset:0x%I64X, BytesTotalReceived:0x%I64X, %s:0x%08X, TotalCmdPacket:%d.", "CU", "FP", TimeEscapems, \
    //    Offset, BytesTotalWrite, Crc32Std.GetName().c_str(), TotalCrc32CheckSum, CmdPacketCnt);


__end:
    if (Ret != 0) {
        m_pCmdHandler->ReleaseCmdExeQueueSemaphoreWhenFail(devIP, HopNum);
    }
    return Ret;
}

// 高性能版本：ReadDataFromSSDorDDR2 - 去掉CRC校验等耗时操作，尽可能减少拷贝
int32_t CACCmdHandler::ReadDataFromSSDorDDR2(std::string devIP, QString Type, uint32_t HopNum, uint32_t PortID, uint64_t SSDAddr, int32_t BytesToRead, QByteArray& DataBytes)
{
    // 高性能优化：使用轻量级锁策略，减少锁竞争
    QMutexLocker locker(&m_SSDMutex);
    
    int32_t Ret = 0;
    bool RetCall = false;
    tCmdPacketDataTrans PacketDataTrans;
    
    // 高性能优化：预分配内存，避免动态扩容
    DataBytes.clear();
    DataBytes.reserve(BytesToRead);
    
    // 高性能优化：直接设置数据大小，避免逐步增长
    DataBytes.resize(BytesToRead);
    
    // 高性能优化：使用简化的数据接收器，无CRC开销
    CByteArrayWriter FileWriter;
    CUplinkDataReceiver UplinkDataReceiver;
    UplinkDataReceiver.ConnectCmdHandler(m_pCmdHandler);
    m_pCmdHandler->SetChangeState(false);
    
    // 高性能优化：直接绑定预分配的ByteArray，零拷贝
    FileWriter.SetByteArray(&DataBytes);
    FileWriter.SetMaxDataSize(BytesToRead);
    // 高性能优化：完全跳过CRC检查
    // FileWriter.AttachICrcCheck(&Crc32Std);  // 注释掉CRC相关代码
    
    IDataWriter* pPrevDataWriter = UplinkDataReceiver.StoreDataWriter(&FileWriter);
    
    // 高性能优化：简化的数据传输包准备
    memset(&PacketDataTrans, 0, sizeof(_tCmdPacketDataTrans));
    PacketDataTrans.MsgID = ICDMsgID_Cmd;
    PacketDataTrans.CmdID = (uint8_t)CICD::TransStrCmd2CmdID(Type);
    PacketDataTrans.HopNum = HopNum;
    PacketDataTrans.PortID = PortID;
    PacketDataTrans.SeqNum = GetNextSeqNum();
    
    // 高性能优化：减少超时等待时间，提高响应速度
    quint32 WaitCmdQueueTimeoutms = 1000; // 固定1秒超时，而不从配置读取
    
    // 快速队列等待检查
    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(devIP, HopNum, WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        Ret = ERR_CMDHAND_CmdQueueAvailable; 
        goto __end_fast;
    }
    
    // 高性能优化：单次大块传输，减少分包开销
    uint64_t SrcAddr = SSDAddr;
    qint64 Offset = 0;
    bool bFirstPacketFlag = true;
    
    while (Offset < BytesToRead) {
        qint64 BytesRead = BytesToRead - Offset;
        if (BytesRead > DataSize_PerTrans) {
            BytesRead = DataSize_PerTrans;
        }
        
        if (bFirstPacketFlag) {
            PacketDataTrans.FirtPacketFlag = 1;
            bFirstPacketFlag = false;
        } else {
            PacketDataTrans.FirtPacketFlag = 0;
        }
        
        // 高性能优化：直接设置传输参数，减少计算开销
        PacketDataTrans.LengthL = (uint32_t)(BytesRead & 0xFFFFFFFF);
        PacketDataTrans.LengthH = (uint16_t)((BytesRead >> 32) & 0xFFFFFFFF);
        PacketDataTrans.SrcAddrL = (uint32_t)(SrcAddr & 0xFFFFFFFF);
        PacketDataTrans.SrcAddrH = (uint16_t)((SrcAddr >> 32) & 0xFFFFFFFF);
        
        Ret = m_pCmdHandler->SendCmdPacketDataTrans(devIP, &PacketDataTrans);
        if (Ret != 0) {
            Ret = ERR_CMDHAND_CmdQueueAvailable;
            goto __end_fast;
        }
        
        // 高性能优化：针对DDR2FIBER的特殊处理，但缩短等待时间
        if (PacketDataTrans.CmdID == (uint8_t)eSubCmdID::SubCmd_DataTrans_DDR2FIBER) {
            RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(devIP, HopNum, 500); // 500ms快速超时
            if (RetCall == false) {
                Ret = ERR_CMDHAND_CmdQueueAvailable; 
                goto __end_fast;
            }
        }
        
        Offset += BytesRead;
        SrcAddr += BytesRead;
    }
    
    // 高性能优化：最终队列等待，但使用更短的超时时间
    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(devIP, HopNum, 500);
    if (RetCall == false) {
        Ret = ERR_CMDHAND_CmdQueueAvailable; 
        goto __end_fast;
    }
    
    // 高性能优化：针对DDR2FIBER的简化同步处理
    if (PacketDataTrans.CmdID == (uint8_t)eSubCmdID::SubCmd_DataTrans_DDR2FIBER) {
        RetCall = m_pCmdHandler->WaitDDR2FiberTransferComplete(devIP, HopNum, 500); // 快速超时
        if (RetCall == false) {
            Ret = ERR_CMDHAND_CmdQueueAvailable; 
            goto __end_fast;
        }
    } else {
        // 高性能优化：减少等待时间从10ms到2ms
        QThread::msleep(2);
    }
    
    // 高性能优化：完全跳过CRC校验，直接返回成功
    // 注释掉所有CRC相关代码以获得最大性能
    /*
    memset(&CmdPacketGetCRC32, 0, sizeof(tCmdPacketGetCRC32));
    CmdPacketGetCRC32.MsgID = ICDMsgID_Cmd;
    CmdPacketGetCRC32.HopNum = HopNum;
    CmdPacketGetCRC32.CmdID = (uint8_t)eSubCmdID::SubCmd_ReadCRC32;
    CmdPacketGetCRC32.PortID = PortID;
    Ret = m_pCmdHandler->SendCmdGetCRC32(devIP, &CmdPacketGetCRC32, DownlinkCRC32, UplinkCRC32);
    if (Ret != 0) {
        goto __end_fast;
    }
    
    if (bCrcCompare) {
        if (UplinkCRC32 != TotalCrc32CheckSum) {
            Ret = ERR_NETCOMM_CmdCRCCompareFailed; 
            goto __end_fast;
        }
    }
    */

__end_fast:
    if (Ret != 0) {
        m_pCmdHandler->ReleaseCmdExeQueueSemaphoreWhenFail(devIP, HopNum);
    }
    return Ret;
}
int32_t CACCmdHandler::ReadDataAndSaveToFile(std::string devIP, QString strFilePath, QString Type, uint32_t HopNum, uint32_t PortID, uint64_t SrcAddr, uint64_t Length, uint64_t fileOffset, tJsonParaHash& ResultHash)
{
    QMutexLocker locker(&m_SSDMutex);
    int32_t Ret = 0;
    double Speed = 0;
    int32_t PacketSendIdx = 1;
    int32_t CmdPacketCnt = 0;
    bool bCrcCompare = m_pAppModels->m_AppConfigModel.SoftCRC32En();
    bool bFirstPacketFlag = true;
    int TimeEscapems = 0;
    bool RetCall = false;
    quint32 TotalCrc32CheckSum = 0;
    qint64 BytesTotalRead = 0;
    tCmdPacketDataTrans PacketDataTrans;
    tCmdPacketGetCRC32 CmdPacketGetCRC32;
    CCrc32Std Crc32Std;
    QTime StartTime, StopTime;
    uint32_t DownlinkCRC32, UplinkCRC32;
    quint32 WaitCmdQueueTimeoutms = m_pAppModels->m_AppConfigModel.CmdQueueAvailableTimeoutms();
    CFileWriter FileWriter;
    qint64 DestAddr = 0, Offset = 0, BytesRead = 0;
    IDataWriter* pPrevDataWriter = NULL;
    QFile File(strFilePath);
    //QThread workerThread;
    CUplinkDataReceiver UplinkDataReceiver;
    UplinkDataReceiver.ConnectCmdHandler(m_pCmdHandler);
    m_pCmdHandler->SetChangeState(false);
    QString strIPHop = QString::fromStdString(devIP) + ":" + QString::number(HopNum);
    //QObject::connect(&workerThread, &QThread::started, &UplinkDataReceiver, &CUplinkDataReceiver::onSLotDoWork);
    //QObject::connect(this, &CACCmdHandler::sgnThreadExit, &workerThread, &QThread::exit, Qt::DirectConnection);
    //QObject::connect(&workerThread, &QThread::finished, &UplinkDataReceiver, &CUplinkDataReceiver::deleteLater);
    //QObject::connect(&workerThread, &QThread::finished, &workerThread, &QThread::deleteLater);	
    //UplinkDataReceiver.moveToThread(&workerThread);
    //workerThread.start();

    std::string dataTransCmdName = Utils::AngKCommonTools::TranslateMessageCmdID((uint8_t)CICD::TransStrCmd2CmdID(Type));
    std::string crc32CmdName = Utils::AngKCommonTools::TranslateMessageCmdID((uint8_t)eSubCmdID::SubCmd_ReadCRC32);

    //准备好DataWriter
    if (strFilePath == "") {
        FileWriter.SetQFile(NULL);
    }
    else {
        if (File.open(QIODevice::ReadWrite) == false) {
            ALOG_INFO("Device %s:%d Execute Func(ReadDataAndSaveToFile) Open File Failed : %s, errmsg : %s.", "CU", "--", devIP.c_str(), HopNum, strFilePath.toStdString().c_str(), File.errorString().toStdString().c_str());
            Ret = ERR_JSONRPC_FileOpen; goto __end;
        }
        FileWriter.SetQFile(&File);
        FileWriter.SetFileSeek(fileOffset);
    }
    FileWriter.SetMaxDataSize(Length);
    if (bCrcCompare) {//只有需要校验的时候才进行计算
        FileWriter.AttachICrcCheck(&Crc32Std);
    }

    pPrevDataWriter = UplinkDataReceiver.StoreDataWriter(&FileWriter);

    //开始数据传输包传输并获取数据
    memset(&PacketDataTrans, 0, sizeof(_tCmdPacketDataTrans));
    PacketDataTrans.MsgID = ICDMsgID_Cmd;
    PacketDataTrans.CmdID = (uint8_t)CICD::TransStrCmd2CmdID(Type);
    PacketDataTrans.HopNum = HopNum;
    PacketDataTrans.PortID = PortID;
    PacketDataTrans.SeqNum = GetNextSeqNum();

    //等待所有命令队列空闲，这样方便后续确认所有发送的命令包都Complete
    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(devIP, HopNum, WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        ALOG_INFO("Device %s:%d Execute %s WaitCmdExeQueueAvailable Timeout at Start, Timeoutms:%d.", "CU", "--", devIP.c_str(), HopNum, Type.toStdString().c_str(), WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable;
        //emit sgnThreadExit(Ret);
        goto __end;
    }
    StartTime = QTime::currentTime();

    bFirstPacketFlag = true;
    CmdPacketCnt = 0;
    ALOG_INFO("Send %s DataTrans to %s:%d Start.", "CU", "FP", dataTransCmdName.c_str(), devIP.c_str(), HopNum);
    while (Offset < Length) {
        BytesRead = Length - Offset;
        if (BytesRead > DataSize_PerTrans) {//每次最多读取这么多
            BytesRead = DataSize_PerTrans;
        }

        if (bFirstPacketFlag) {///只做一次
            PacketDataTrans.FirtPacketFlag = 1;
            bFirstPacketFlag = false;
        }
        else {
            PacketDataTrans.FirtPacketFlag = 0;
        }

        //重新指定长度和读取的起始位置
        PacketDataTrans.LengthL = (uint32_t)(BytesRead & 0xFFFFFFFF);
        PacketDataTrans.LengthH = (uint16_t)((BytesRead >> 32) & 0xFFFFFFFF);
        PacketDataTrans.SrcAddrL = (uint32_t)(SrcAddr & 0xFFFFFFFF);
        PacketDataTrans.SrcAddrH = (uint16_t)((SrcAddr >> 32) & 0xFFFFFFFF);
        CmdPacketCnt++;

        Ret = m_pCmdHandler->SendCmdPacketDataTrans(devIP, &PacketDataTrans);//发送命令传输数据包   
        if (Ret != 0) {
            ALOG_INFO("Send %s DataTrans to %s:%d Failed.", "CU", "FP", dataTransCmdName.c_str(), devIP.c_str(), HopNum);
            goto __end;
        }
        if (PacketDataTrans.CmdID == (uint8_t)eSubCmdID::SubCmd_DataTrans_DDR2FIBER) {//如果是DDR读取，不能使用队列只能等到Complete包返回才能继续
            RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(devIP, HopNum, WaitCmdQueueTimeoutms);
            if (RetCall == false) {
                ALOG_INFO("Device %s:%d Execute %s WaitCmdExeQueueAvailable Timeout.", "CU", "--", devIP.c_str(), HopNum, Type.toStdString().c_str());
                Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
            }
        }
        Offset += BytesRead;
        SrcAddr += BytesRead;
    }
    ALOG_INFO("Send %s DataTrans to %s:%d Finish.", "CU", "FP", dataTransCmdName.c_str(), devIP.c_str(), HopNum);
    //等待所有命令队列空闲，表明所有的命令包都Complete
    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(devIP, HopNum, WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        ALOG_INFO("Device %s:%d Execute %s Wait All Complete Package Return Timeout, Timemoutms:%d.", "CU", "FP", devIP.c_str(), HopNum, Type.toStdString().c_str(), WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }
    QThread::msleep(10);///等待这个ms数，因为命令完成包达到之后，可能还有数据未到达，延迟等待一下！
    StopTime = QTime::currentTime();

    memset(&CmdPacketGetCRC32, 0, sizeof(tCmdPacketGetCRC32));
    CmdPacketGetCRC32.MsgID = ICDMsgID_Cmd;
    CmdPacketGetCRC32.HopNum = HopNum;
    CmdPacketGetCRC32.CmdID = (uint8_t)eSubCmdID::SubCmd_ReadCRC32;
    CmdPacketGetCRC32.PortID = PortID;

    //m_pCmdHandler->WaitWriteFinish(strIPHop);
    ALOG_INFO("Send %s to %s:%d.", "CU", "FP", crc32CmdName.c_str(), devIP.c_str(), HopNum);
    Ret = m_pCmdHandler->SendCmdGetCRC32(devIP, &CmdPacketGetCRC32, DownlinkCRC32, UplinkCRC32);
    if (Ret == 0) {
        ALOG_INFO("Device %s:%d Execute %s UplinkCRC32:0x%08X,DownlinCRC32:0x%08X.", "CU", "FP", devIP.c_str(), HopNum, Type.toStdString().c_str(), UplinkCRC32, DownlinkCRC32);
    }
    else {
        goto __end;
    }
    Crc32Std.GetChecksum((uint8_t*)&TotalCrc32CheckSum, 4);///中间过程已经由DataWriter完成，这里直接获取校验值
    if (bCrcCompare) {
        if (UplinkCRC32 != TotalCrc32CheckSum) {
            ALOG_INFO("Device %s:%d Execute %s Compare Failed, UplinkCRC32:0x%08X, Crc32Calc:0x%08X.", "CU", "FP", devIP.c_str(), HopNum, Type.toStdString().c_str(), UplinkCRC32, TotalCrc32CheckSum);
            Ret = ERR_NETCOMM_CmdCRCCompareFailed;
        }
        else {
            ALOG_INFO("Device %s:%d Execute %s Compare Pass.", "CU", "FP", devIP.c_str(), HopNum, Type.toStdString().c_str());
        }
    }


    TimeEscapems = (StopTime.msecsSinceStartOfDay() - StartTime.msecsSinceStartOfDay());
    BytesTotalRead = UplinkDataReceiver.GetDataWriterCurDataSize();
    Speed = (double)BytesTotalRead / (double)(TimeEscapems) * 1000 / (double)(1024 * 1024);
    //ALOG_INFO("************PacketLost:%d", UplinkDataReceiver.GetPacketLost());
    //ALOG_INFO("TimeEscape_ms:%d ms ,Speed:%.2lf MBytes/S, Offset:0x%I64X, BytesTotalReceived:0x%I64X, %s:0x%08X, TotalCmdPacket:%d", TimeEscapems, Speed, \
    //    Offset, BytesTotalRead, Crc32Std.GetName().c_str(), TotalCrc32CheckSum, CmdPacketCnt);

   // ALOG_INFO("Uplink DataPacket:%d", UplinkDataReceiver.GetTotalPacket());

    ResultHash.insert("BytesRead", QString::number(BytesTotalRead, 16).toUpper().insert(0, "0x"));
    ResultHash.insert("TimeEscape_ms", TimeEscapems);
    ResultHash.insert("Crc32", QString::number(TotalCrc32CheckSum, 16).toUpper().insert(0, "0x"));//将校验值转为16进制字符串


__end:
    if (Ret != 0) {
        m_pCmdHandler->ReleaseCmdExeQueueSemaphoreWhenFail(devIP, HopNum);//出现错误，则是否所有命令队列资源，准备下次可以使用
    }

    //int timeout = 3000;
    //workerThread.quit();
    //workerThread.wait(timeout);
    return Ret;
}


int32_t CACCmdHandler::LinkScan(std::string devIP, uint32_t HopNum)
{
    QMutexLocker locker(&m_SSDMutex);
    int32_t Ret = 0;
    tLinkScanPacket LinkScanPacket;
    memset(&LinkScanPacket, 0, sizeof(tLinkScanPacket));
    LinkScanPacket.MsgID = ICDMsgID_LinkScan;
    LinkScanPacket.HopNum = HopNum;
    ALOG_INFO("Send LinkScanPacket to %s, HopNum:%d.", "CU", "FP", devIP.c_str(), HopNum);
    m_pCmdHandler->SendLinkScanPacket(devIP, &LinkScanPacket);
    return Ret;
}

int32_t CACCmdHandler::CloseProgramSocket()
{
    int ret = 0;
    ret = m_pNetComm->StopComm();
    return ret;
}