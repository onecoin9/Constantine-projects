#include "ACCmdHandler.h"
#include <QDataStream>
#include <Windows.h>
#include "Crc32_Std.h"
#include "IDataWriter.h"
#include "UplinkDataReceiver.h"
#include <QThread>
#include <QTime>
#include <QDir>

///注意DataSize_PerTrans要是DataSize_PerPacket的整数倍
#define DataSize_PerPacket      (4096) //单次下行包的数据量   不能超过4K，SSD需要每次4K的数据量
#define DataSize_PerTrans       (1*1024*1024) //单次发送的命令中的数据长度要固定为1M
#define TIME_LIMIT_US   (10)    //延迟时间，us为单位

#define _PrintLog(_Level,fmt,...) \
	if (m_pILog) {\
		m_pILog->PrintLog(_Level, fmt, __VA_ARGS__);\
	}

CACCmdHandler::CACCmdHandler()
	:m_pILog(NULL)
	,m_pCmdHandler(NULL)
    ,m_pAppModels(NULL)
    ,m_PrivDataCbkComplete(NULL)
    ,m_fnCbkComplete(NULL)
    ,m_PrivDataCbkQueryDoCmd(NULL)
    ,m_fnCbkQueryDoCmd(NULL)
{

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

void CACCmdHandler::AttachILog(ILog* pLog)
{
	m_pILog = pLog;
}

void CACCmdHandler::AttachCmdHandler(CCmdHandler* pCmdHandler)
{
	m_pCmdHandler = pCmdHandler;
    connect(m_pCmdHandler, &CCmdHandler::sigPTPacketExecComplete, this, &CACCmdHandler::onPTPacketExecComplete);
    connect(m_pCmdHandler, &CCmdHandler::sigQueryDoPTCmd, this, &CACCmdHandler::onQueryDoPTCmd);
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

void CACCmdHandler::onPTPacketExecComplete(qint32 HopNum, qint32 PortID, QByteArray ACCmdCompletePack)
{
    tACCmdPack* pACCmdPack = (tACCmdPack*)ACCmdCompletePack.constData();
    QByteArray DataSerial;
    QDataStream DataStream(&DataSerial,QIODevice::ReadWrite);//将DataStream与ByteArray绑定，设置为读写，为后续序列化作准备
    QByteArray RespDataBytes; 
    int32_t ResultCode;
    int32_t RespDataSize;
    int32_t Offset = 0;
    DataStream.writeRawData((char*)pACCmdPack->CmdData, pACCmdPack->CmdDataSize); //将需要反序列化的数据写入到DataStream中
    //m_pILog->PrintBuf((char*)"ACCmdPack CmdData------",(char*)pACCmdPack->CmdData, pACCmdPack->CmdDataSize);
    DataStream.device()->seek(0); //将位置定位到最开始，准备从最开始读取
    DataStream.setByteOrder(QDataStream::LittleEndian);//设置为小端方式
    DataStream >> ResultCode;
    DataStream >> RespDataSize;
    _PrintLog(LOGLEVEL_D, "onPTPacketExecComplete  HopNum:%d, PortID:0x%08X, ResultCode:%d,RespDataSize:%d, CmdFlags:0x%08X, CallBackFn:%p\r\n", 
                                                                HopNum, PortID, ResultCode, RespDataSize, pACCmdPack->CmdFlag, m_fnCbkComplete);
    if (pACCmdPack->CmdFlag & CmdFlag_DataInSSD) {
        uint64_t SSDAddr = 0;
        DataStream >> SSDAddr;
        ReadDataFromSSD(HopNum, PortID, SSDAddr, RespDataSize, RespDataBytes);
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
        m_fnCbkComplete(m_PrivDataCbkComplete, HopNum, PortID,ResultCode, RespDataBytes);
    }
    else {
        emit sigRemoteCmdComplete(HopNum, PortID, ResultCode, RespDataBytes);
    }
}

void CACCmdHandler::onQueryDoPTCmd(qint32 HopNum, qint32 PortID, QByteArray ACCmdQueryPack)
{
    tACCmdPack* pACCmdPack = (tACCmdPack*)ACCmdQueryPack.constData();
    QByteArray CmdDataBytes;
    _PrintLog(LOGLEVEL_D, "onQueryDoPTCmd HopNum:%d, PortID:0x%08X, CmdFlag:0x%04X, CmdID:0x%02X,CallBackFn:%p\r\n", HopNum, PortID, pACCmdPack->CmdFlag, pACCmdPack->CmdID, m_fnCbkQueryDoCmd);
    if (pACCmdPack->CmdFlag & CmdFlag_DataInSSD) {
        uint64_t SSDAddr = 0;
        memcpy(&SSDAddr, pACCmdPack->CmdData, sizeof(uint64_t));
        ReadDataFromSSD(HopNum, PortID, SSDAddr,pACCmdPack->CmdDataSize, CmdDataBytes);
    }
    else {
        CmdDataBytes.append((char*)pACCmdPack->CmdData, pACCmdPack->CmdDataSize);
    }
    if (m_fnCbkQueryDoCmd) {
        m_fnCbkQueryDoCmd(m_PrivDataCbkQueryDoCmd, HopNum, PortID, pACCmdPack->CmdID, CmdDataBytes);
    }
    else {
        emit sigRemoteQueryDoCmd(HopNum, PortID, pACCmdPack->CmdFlag, pACCmdPack->CmdID, CmdDataBytes);
    }
}

int32_t CACCmdHandler::StoreDataToSSD(uint32_t HopNum, uint32_t PortID, uint64_t SSDAddr, QByteArray& DataBytes)
{
    int32_t Ret = 0,RetCall;
    char* pSrcData = DataBytes.data();
    char* pSrcDataCur = pSrcData;
    bool bFirstPack = true;
    bool bCrcCompare = true;
    tDataPacket DataPack;
    uint32_t UplinkCRC32, DownlinkCRC32;
    tCmdPacketGetCRC32 CmdPacketGetCRC32;
    qint64 i = 0, PacketCount = 0, BytesRemain = 0, ByteSended =0;
    qint64 FileSize = 0, BytesRead, Offset = 0, BufferOffset = 0;
    tCmdPacketDataTrans PacketDataTrans;
    uint64_t SrcAddr = 0, Length = 0;
    uint64_t DestAddr = 0;
    quint32 Checksum = 0;
    qint32 TransCnt = 0,CmdPacketCnt = 0, PacketCnt = 0;
    quint32 WaitCmdQueueTimeoutms = m_pAppModels->m_AppConfigModel.CmdQueueAvailableTimeoutms();
    ICrcCheck* pCrcCheck = new CCrc32Std();

    memset(&PacketDataTrans, 0, sizeof(_tCmdPacketDataTrans));
    PacketDataTrans.MsgID = ICDMsgID_Cmd;
    PacketDataTrans.CmdID = (uint8_t)CICD::TransStrCmd2CmdID("FIBER2SSD");
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



    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        _PrintLog(LOGLEVEL_E, "WaitCmdExeQueueAvailable Timeout at Start, Timemoutms:%d\r\n", WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }

    FileSize = DataBytes.size();

    DestAddr = SSDAddr;

	QElapsedTimer timer;
    //这里让所有发送的数据只有第一个DataSize_PerPacket包有Flag
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
            //_PrintLog(LOGLEVEL_N,"1 SendData DestAddr:0x%08I64X, Length:0x%0I64X\r\n", DestAddr, Length);
            //DDR写只支持1个对列，所以每次发送之前都需要等待
            CmdPacketCnt++;
            RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(WaitCmdQueueTimeoutms);
            if (RetCall == false) {
                Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
            }
            Ret = m_pCmdHandler->SendCmdPacketDataTrans(&PacketDataTrans);//发送命令传输数据包   
            if (Ret != 0) {
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
            memcpy(&DataPack.Data, pSrcDataCur + BufferOffset, DataSize_PerPacket);
            if (bCrcCompare) {
                pCrcCheck->CalcSubRoutine((uint8_t*)&DataPack.Data, DataSize_PerPacket);
            }
            //m_pCmdHandler->SendDownlinkData(&DataPack);//发送数据包
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
            memcpy(&DataPack.Data, pSrcDataCur + BufferOffset, BytesRemain);///最后一个包按照实际的数据量填入
            if (bCrcCompare) {
                pCrcCheck->CalcSubRoutine((uint8_t*)&DataPack.Data, BytesRemain);
            }
            Ret = m_pCmdHandler->SendDownlinkData(&DataPack);//发送数据包
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
            _PrintLog(LOGLEVEL_N, "2 SendData DestAddr:0x%08I64X, Length:0x%0I64X\r\n", DestAddr, Length);
            CmdPacketCnt++;
            Ret = m_pCmdHandler->SendCmdPacketDataTrans(&PacketDataTrans);//发送命令传输数据包   
            if (Ret != 0) {
                goto __end;
            }
        }

        TransCnt++;
        ByteSended += BytesRead;
        DestAddr += BytesRead;
        Offset += BytesRead;
        pSrcDataCur += BytesRead;
    }

	qint64 elapsedTime = timer.elapsed();  // 获取经过的时间（毫秒）
// 将毫秒转换成秒
	qint64 totalSeconds = elapsedTime / 1000;
	// 计算小时数
	qint64 hours = totalSeconds / 3600;
	// 计算剩余的秒数（去除小时后的秒数）
	qint64 remainingSeconds = totalSeconds % 3600;
	// 获取完整的分钟数
	int minutes = static_cast<int>(remainingSeconds / 60);
	// 获取剩余的秒数
	int seconds = static_cast<int>(remainingSeconds % 60);
    _PrintLog(LOGLEVEL_N, "DownlinkData Total Time : %d hours %d min %d seconds.", hours, minutes, seconds);

    _PrintLog(LOGLEVEL_N, "WaitCmdExeQueueAvailable\r\n");
    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        _PrintLog(LOGLEVEL_E, "WaitCmdExeQueueAvailable Timeout at End, Timemoutms:%d\r\n", WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }

    _PrintLog(LOGLEVEL_N, "SendCmdGetCRC32\r\n");
    m_pCmdHandler->SendCmdGetCRC32(&CmdPacketGetCRC32, DownlinkCRC32, UplinkCRC32);
    if (bCrcCompare) {
        pCrcCheck->GetChecksum((uint8_t*)&Checksum, 4);
        if (DownlinkCRC32 == Checksum) {
            _PrintLog(LOGLEVEL_N, "Crc32 Checksum Compare Pass, DownlinkCRC32:0x%08X, Crc32Calc:0x%08X\r\n", DownlinkCRC32, Checksum);
        }
        else {
            _PrintLog(LOGLEVEL_E, "Crc32 Checksum Compare Failed, DownlinkCRC32:0x%08X, Crc32Calc:0x%08X\r\n", DownlinkCRC32, Checksum);
        }
    }
    else {
        _PrintLog(LOGLEVEL_N, "Crc32 Checksum Downlink :0x%08X\r\n", DownlinkCRC32);
    }

__end:
    if (Ret != 0) {
        m_pCmdHandler->ReleaseCmdExeQueueSemaphoreWhenFail();
    }
	return Ret;
}

int32_t CACCmdHandler::SSD2DDR(uint32_t HopNum, uint32_t PortID, uint64_t SrcSSDAddr, uint64_t DestDDRAddr, uint64_t DataSize)
{
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
    PacketDataTrans.SrcAddrL = (uint32_t)(SrcSSDAddr & 0xFFFFFFFF);
    PacketDataTrans.SrcAddrH = (uint16_t)((SrcSSDAddr >> 32) & 0xFFFFFFFF);
    PacketDataTrans.DestAddrL = (uint32_t)(DestDDRAddr & 0xFFFFFFFF);
    PacketDataTrans.DestAddrH = (uint16_t)((DestDDRAddr >> 32) & 0xFFFFFFFF);
    PacketDataTrans.LengthL = (uint32_t)(DataSize & 0xFFFFFFFF);
    PacketDataTrans.LengthH = (uint16_t)((DataSize >> 32) & 0xFFFFFFFF);

    //等待所有命令队列空闲，这样方便后续确认所有发送的命令包都Complete
    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        _PrintLog(LOGLEVEL_E, "WaitCmdExeQueueAvailable Timeout at Start, Timemoutms:%d\r\n", WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }

    Ret = m_pCmdHandler->SendCmdPacketDataTrans(&PacketDataTrans);//发送命令传输数据包   
    if (Ret != 0) {
        goto __end;
    }

    //等待所有命令队列空闲，表明所有的命令包都Complete
    _PrintLog(LOGLEVEL_D, "WaitCmdExeQueueAvailable....\r\n");
    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        _PrintLog(LOGLEVEL_E, "WaitCmdExeQueueAvailable Timeout at End, Timemoutms:%d\r\n", WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }
    QThread::msleep(10);///等待这个ms数，因为命令完成包达到之后，可能还有数据未到达，延迟等待一下！

__end:
    if (Ret != 0) {
        m_pCmdHandler->ReleaseCmdExeQueueSemaphoreWhenFail();
    }
    return Ret;
}

int32_t CACCmdHandler::DDR2SSD(uint32_t HopNum, uint32_t PortID, uint64_t SrcDDRAddr, uint64_t DestSSDAddr, uint64_t DataSize)
{
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
    PacketDataTrans.SrcAddrL = (uint32_t)(SrcDDRAddr & 0xFFFFFFFF);
    PacketDataTrans.SrcAddrH = (uint16_t)((SrcDDRAddr >> 32) & 0xFFFFFFFF);
    PacketDataTrans.DestAddrL = (uint32_t)(DestSSDAddr & 0xFFFFFFFF);
    PacketDataTrans.DestAddrH = (uint16_t)((DestSSDAddr >> 32) & 0xFFFFFFFF);
    PacketDataTrans.LengthL = (uint32_t)(DataSize & 0xFFFFFFFF);
    PacketDataTrans.LengthH = (uint16_t)((DataSize >> 32) & 0xFFFFFFFF);

    //等待所有命令队列空闲，这样方便后续确认所有发送的命令包都Complete
    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        _PrintLog(LOGLEVEL_E, "WaitCmdExeQueueAvailable Timeout at Start, Timemoutms:%d\r\n", WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }

    Ret = m_pCmdHandler->SendCmdPacketDataTrans(&PacketDataTrans);//发送命令传输数据包   
    if (Ret != 0) {
        goto __end;
    }

    //等待所有命令队列空闲，表明所有的命令包都Complete
    _PrintLog(LOGLEVEL_D, "WaitCmdExeQueueAvailable....\r\n");
    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        _PrintLog(LOGLEVEL_E, "WaitCmdExeQueueAvailable Timeout at End, Timemoutms:%d\r\n", WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }
    QThread::msleep(10);///等待这个ms数，因为命令完成包达到之后，可能还有数据未到达，延迟等待一下！

__end:
    if (Ret != 0) {
        m_pCmdHandler->ReleaseCmdExeQueueSemaphoreWhenFail();
    }
    return Ret;
}

int32_t CACCmdHandler::ReadDataFromSSD(uint32_t HopNum, uint32_t PortID, uint64_t SSDAddr, int32_t BytesToRead, QByteArray& DataBytes)
{
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
    UplinkDataReceiver.AttachLog(m_pILog);
    UplinkDataReceiver.ConnectCmdHandler(m_pCmdHandler);


    //准备好DataWriter
    FileWriter.SetByteArray(&DataBytes);
    FileWriter.SetMaxDataSize(Length);
    if (bCrcCompare) {
        FileWriter.AttachICrcCheck(&Crc32Std);
    }
    _PrintLog(LOGLEVEL_N, "CRCCompare Enable:%s\r\n", bCrcCompare?"YES" : "NO");
    pPrevDataWriter = UplinkDataReceiver.StoreDataWriter(&FileWriter);

    //开始数据传输包传输并获取数据
    memset(&PacketDataTrans, 0, sizeof(_tCmdPacketDataTrans));
    PacketDataTrans.MsgID = ICDMsgID_Cmd;
    PacketDataTrans.CmdID = (uint8_t)CICD::TransStrCmd2CmdID("SSD2FIBER");
    PacketDataTrans.HopNum = HopNum;
    PacketDataTrans.PortID = PortID;

    //等待所有命令队列空闲，这样方便后续确认所有发送的命令包都Complete
    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        _PrintLog(LOGLEVEL_E, "WaitCmdExeQueueAvailable Timeout at Start, Timemoutms:%d\r\n", WaitCmdQueueTimeoutms);
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
        //_PrintLog(LOGLEVEL_N, "FirtPacketFlag:%d, FirstDWord:0x%08X\r\n", PacketDataTrans.FirtPacketFlag,*(uint32_t*)(&PacketDataTrans));
        //重新指定长度和读取的起始位置
        PacketDataTrans.LengthL = (uint32_t)(BytesRead & 0xFFFFFFFF);
        PacketDataTrans.LengthH = (uint16_t)((BytesRead >> 32) & 0xFFFFFFFF);
        PacketDataTrans.SrcAddrL = (uint32_t)(SrcAddr & 0xFFFFFFFF);
        PacketDataTrans.SrcAddrH = (uint16_t)((SrcAddr >> 32) & 0xFFFFFFFF);

        //_PrintLog(LOGLEVEL_N, "1 ReadData SrcAddr:0x%0I64X, Length:0x%0I64X\r\n", SrcAddr, BytesRead);
        CmdPacketCnt++;
        Ret = m_pCmdHandler->SendCmdPacketDataTrans(&PacketDataTrans);//发送命令传输数据包   
        if (Ret != 0) {
            goto __end;
        }
        if (PacketDataTrans.CmdID == (uint8_t)eSubCmdID::SubCmd_DataTrans_DDR2FIBER) {//如果是DDR读取，不能使用队列只能等到Complete包返回才能继续
            RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(WaitCmdQueueTimeoutms);
            if (RetCall == false) {
                Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
            }
        }
        Offset += BytesRead;
        SrcAddr += BytesRead;
    }

    //等待所有命令队列空闲，表明所有的命令包都Complete
    _PrintLog(LOGLEVEL_D, "WaitCmdExeQueueAvailable....\r\n");
    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        _PrintLog(LOGLEVEL_E, "WaitCmdExeQueueAvailable Timeout at End, Timemoutms:%d\r\n", WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }
    QThread::msleep(10);///等待这个ms数，因为命令完成包达到之后，可能还有数据未到达，延迟等待一下！

    memset(&CmdPacketGetCRC32, 0, sizeof(tCmdPacketGetCRC32));
    CmdPacketGetCRC32.MsgID = ICDMsgID_Cmd;
    CmdPacketGetCRC32.HopNum = HopNum;
    CmdPacketGetCRC32.CmdID = (uint8_t)eSubCmdID::SubCmd_ReadCRC32;
    CmdPacketGetCRC32.PortID = PortID;

    Ret = m_pCmdHandler->SendCmdGetCRC32(&CmdPacketGetCRC32, DownlinkCRC32, UplinkCRC32);
    if (Ret == 0) {
        _PrintLog(LOGLEVEL_N, "CRC32 UplinkCRC32:0x%08X,DownlinCRC32:0x%08X\r\n", UplinkCRC32, DownlinkCRC32);
    }
    else {
        goto __end;
    }
    Crc32Std.GetChecksum((uint8_t*)&TotalCrc32CheckSum, 4);///中间过程已经由DataWriter完成，这里直接获取校验值
    if (bCrcCompare) {
        if (UplinkCRC32 != TotalCrc32CheckSum) {
            _PrintLog(LOGLEVEL_E, "CRC32 Compare Failed, UplinkCRC32:0x%08X, Crc32Calc:0x%08X\r\n", UplinkCRC32, TotalCrc32CheckSum);
        }
        else {
            _PrintLog(LOGLEVEL_N, "CRC32 Compare Pass\r\n");
        }
    }

    BytesTotalWrite = UplinkDataReceiver.GetDataWriterCurDataSize();
    _PrintLog(LOGLEVEL_N, "TimeEscape_ms:%d ms, Offset:0x%I64X, BytesTotalReceived:0x%I64X, %s:0x%08X, TotalCmdPacket:%d\r\n", TimeEscapems, \
        Offset, BytesTotalWrite, Crc32Std.GetName().c_str(), TotalCrc32CheckSum, CmdPacketCnt);


__end:
    if (Ret != 0) {
        m_pCmdHandler->ReleaseCmdExeQueueSemaphoreWhenFail();
    }
    _PrintLog(LOGLEVEL_N, "ReadData Ret:%d\r\n", Ret);
    return Ret;
}



int32_t CACCmdHandler::RemoteDoPTCmd(uint32_t HopNum, uint32_t PortID, uint32_t CmdFlag, uint16_t CmdID, uint32_t SktEn, uint16_t BPUID, QByteArray& CmdDataBytes)
{
	int32_t Ret = 0;
    int32_t StructSize = 0;
    int32_t PTPacketRealSize = 0;
    int32_t CmdDataSize = CmdDataBytes.size();
	int32_t ACCmdPacketPayloadMax = m_pCmdHandler->GetACCmdPacketPayloadMax();
	tCmdPacketPT CmdPacketPT,*pCmdPacketPT=NULL;
	tACCmdPack * pACCmdPack = NULL;
	pCmdPacketPT = &CmdPacketPT;
	pACCmdPack = (tACCmdPack *)pCmdPacketPT->Data;

	memset(pCmdPacketPT, 0, sizeof(tCmdPacketPT));

	pCmdPacketPT->HopNum = HopNum;
    pCmdPacketPT->MsgID = ICDMsgID_PT;
	pCmdPacketPT->PortID = PortID;
	pCmdPacketPT->CMD_ID = CMDID_PTPACK_SRC;

    pACCmdPack->BPUID = BPUID;
    pACCmdPack->SKTEn = SktEn;
	pACCmdPack->CmdFlag = CmdFlag;
	pACCmdPack->CmdID = CmdID;

#if 1
	pACCmdPack->CmdDataSize = CmdDataSize;
	if (CmdDataSize > ACCmdPacketPayloadMax) {//不能放到透传包中,先用FIBER2SSD发送数据到指定的位置
        uint64_t SSDAddr= SSD_PC2MUDataExchangeOffset;
        _PrintLog(LOGLEVEL_D, "CmdDataSize(%d Bytes)>%d Bytes, NeedToUse SSD To ExChange Data\r\n", CmdDataSize, ACCmdPacketPayloadMax);
        Ret = StoreDataToSSD(HopNum, PortID, SSDAddr, CmdDataBytes);
        if (Ret == 0) {
            pACCmdPack->CmdFlag |= CmdFlag_DataInSSD;
            memcpy(pACCmdPack->CmdData,(char*)&SSDAddr, sizeof(uint64_t)); //如果不能一个包发送，将数据存到SSD中之后，将SSD的位置放到CmdData中，占8个字节
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
    //_PrintLog(LOGLEVEL_D,"StructSize:%d, CmdDataSize:%d\r\n", sizeof(tCmdPacketPT), CmdDataSize);
    //m_pILog->PrintBuf((char*)"=============RemoteDoPTCmd==============", (char*)pCmdPacketPT, sizeof(tCmdPacketPT));// sizeof(tCmdPacketPT)>64?64:sizeof(tCmdPacketPT)
    _PrintLog(LOGLEVEL_D, "RemoteDoPTCmd HopNum:%d, PortID:0x%08X,CmdFlag:0x%08X, CmdID:0x%02X, CmdDataSize:0x%X\r\n", HopNum, PortID, CmdFlag, CmdID, CmdDataBytes.size());
	Ret=m_pCmdHandler->SendCmdPTCmd(pCmdPacketPT);
    _PrintLog(LOGLEVEL_D, "SendCmdPTCmd Ret:%d\r\n", Ret);
	return Ret;
}


int32_t CACCmdHandler::SendDevCallBackResult(uint32_t HopNum, uint32_t PortID, int32_t ResultCode, QByteArray RespData)
{
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
    pCmdPacketPT->CMD_ID = CMDID_PTPACK_QUERYDOCMD;

    //设置ACCmdPack包的信息
    pACCmdPack->CmdFlag = 0;
    pACCmdPack->CmdID = CmdID_CmdDone;
    pACCmdPack->CmdDataSize = CmdDataSize;

    //设置对应的ACCmdPack数据部分信息
    DataStream.setByteOrder(QDataStream::LittleEndian);//设置为小端方式
    DataStream << ResultCode;
    DataStream << RespDataSize;
    if (CmdDataSize > ACCmdPacketPayloadMax) {//不能放到透传包中,先用FIBER2SSD发送数据到指定的位置
        uint64_t SSDAddr = SSD_PC2MUDataExchangeOffset;
        _PrintLog(LOGLEVEL_D, "CmdDataSize(%d Bytes)>%d Bytes, NeedToUse SSD To ExChange Data\r\n", CmdDataSize, ACCmdPacketPayloadMax);
        Ret = StoreDataToSSD(HopNum, PortID, SSDAddr, RespData);
        if (Ret == 0) {
            pACCmdPack->CmdFlag |= CmdFlag_DataInSSD;
            DataStream << SSDAddr; //如果不能一个包发送，将数据存到SSD中之后，将SSD的位置放到CmdData中，占8个字节
        }
    }
    else {
        DataStream.writeRawData(RespData.constData(), RespDataSize);
    }
    memcpy(pACCmdPack->CmdData, DataSerial.constData(), DataSerial.size());//将序列化之后的数据放到ACCmdPack的数据部分

    m_pILog->PrintBuf((char*)"=====DevCallBack SetResult===", (char*)pCmdPacketPT, DataSerial.size()+20);// sizeof(tCmdPacketPT)>64?64:sizeof(tCmdPacketPT)
    Ret = m_pCmdHandler->SendCmdPTCmd(pCmdPacketPT);
    _PrintLog(LOGLEVEL_D, "DevCallBack SetResult Done Ret:%d\r\n", Ret);
    return Ret;
}

int32_t CACCmdHandler::GetCapacity(QString Type, uint32_t HopNum, uint32_t PortID)
{
    int32_t Ret = 0;
    int32_t BPUPos = 0, SKTPos = 0;
    tCmdPacketGetCapacity PckGetCapacity;
   

    memset(&PckGetCapacity, 0, sizeof(tCmdCplPacketGetCapacity));

    PckGetCapacity.MsgID = ICDMsgID_Cmd;
    PckGetCapacity.HopNum = HopNum;
    if (Type == "SSD") { PckGetCapacity.CmdID = (uint8_t)eSubCmdID::SubCmd_ReadCapacity_SSD; }
    else if (Type == "DDR") { PckGetCapacity.CmdID = (uint8_t)eSubCmdID::SubCmd_ReadCapacity_DDR; }
    else if (Type == "SKT") { PckGetCapacity.CmdID = (uint8_t)eSubCmdID::SubCmd_ReadCapacity_SKT; }
    else { Ret = ERR_JSONRPC_ParaInvalid; goto __end; }

    PckGetCapacity.PortID = PortID;

    Ret = m_pCmdHandler->SendCmdGetCapacity(&PckGetCapacity);

__end:
    return Ret;
}


int32_t CACCmdHandler::WriteDataFromFile(QString strFilePath, QString Type, uint32_t HopNum, uint32_t PortID, uint64_t DestAddr, tJsonParaHash& ResultHash)
{
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

    ICrcCheck* pCrcCheck = new CCrc32Std();

    QTime StartTime, StopTime;
    quint32 OnePacketChecksum;
    quint32 Checksum = 0;
    qint32 TimeEscapems = 0;
    qint32 TransCnt = 0, PacketCnt = 0;
    char* TmpBuffer = NULL;

    QFile File(strFilePath);
    if (File.open(QIODevice::ReadOnly) == false) {
        _PrintLog(LOGLEVEL_E, "Open File Failed:%s\r\n", strFilePath.toStdString().c_str());
        Ret = ERR_JSONRPC_FileOpen; goto __end;
    }
    FileSize = File.size();

    memset(&PacketDataTrans, 0, sizeof(_tCmdPacketDataTrans));
    PacketDataTrans.MsgID = ICDMsgID_Cmd;
    PacketDataTrans.CmdID = (uint8_t)CICD::TransStrCmd2CmdID(Type);
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

    TmpBuffer = new char[DataSize_PerTrans];
    if (TmpBuffer == NULL) {
        Ret = ERR_MemoryAlloc; goto __end;
    }
    if (pCrcCheck == NULL) {
        Ret = ERR_MemoryAlloc; goto __end;
    }
    StartTime = QTime::currentTime();

    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        _PrintLog(LOGLEVEL_E, "WaitCmdExeQueueAvailable Timeout at Start, Timemoutms:%d\r\n", WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }

    CmdPacketCnt = 0;
    QElapsedTimer timer;
    timer.start();
    //开始文件操作
    //这里让所有发送的数据只有第一个DataSize_PerPacket包有Flag
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
            //_PrintLog(LOGLEVEL_N,"1 SendData DestAddr:0x%08I64X, Length:0x%0I64X\r\n", DestAddr, Length);
            //DDR写只支持1个对列，所以每次发送之前都需要等待
            CmdPacketCnt++;
            RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(WaitCmdQueueTimeoutms);
            if (RetCall == false) {
                Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
            }
            Ret = m_pCmdHandler->SendCmdPacketDataTrans(&PacketDataTrans);//发送命令传输数据包   
            if (Ret != 0) {
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
            m_pCmdHandler->SendDownlinkData(&DataPack);//发送数据包
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
            memcpy(&DataPack.Data, TmpBuffer + BufferOffset, BytesRemain);///最后一个包按照实际的数据量填入
            if (bCrcCompare) {
                pCrcCheck->CalcSubRoutine((uint8_t*)&DataPack.Data, BytesRemain);
            }
            Ret = m_pCmdHandler->SendDownlinkData(&DataPack);//发送数据包
            PacketCnt++;
        }
        
        _PrintLog(LOGLEVEL_D, "WriteDataFromFile PacketDataTrans.CmdID = %d\r\n", PacketDataTrans.CmdID);
        if (PacketDataTrans.CmdID == (uint8_t)eSubCmdID::SubCmd_DataTrans_FIBER2SSD ||
            PacketDataTrans.CmdID == (uint8_t)eSubCmdID::SubCmd_DataTrans_FIBER2SKT) {//发送数据到SSD或SKT需要先数据
            //再发送命令传输数据包，告知Device上面的这些数据要被存放到哪里
            Length = BytesRead;
            PacketDataTrans.DestAddrL = (DestAddr & 0xFFFFFFFF);
            PacketDataTrans.DestAddrH = (DestAddr >> 32) & 0xFFFF;
            PacketDataTrans.LengthL = (Length & 0xFFFFFFFF);
            PacketDataTrans.LengthH = (Length >> 32) & 0xFFFF;
            _PrintLog(LOGLEVEL_D, "2 SendData DestAddr:0x%08I64X, Length:0x%0I64X\r\n", DestAddr, Length);
            CmdPacketCnt++;
            Ret = m_pCmdHandler->SendCmdPacketDataTrans(&PacketDataTrans);//发送命令传输数据包   
            if (Ret != 0) {
                goto __end;
            }
        }

        TransCnt++;
        ByteSended += BytesRead;
        DestAddr += BytesRead;
        Offset += BytesRead;
    }

	qint64 elapsedTime = timer.elapsed();  // 获取经过的时间（毫秒）

	_PrintLog(LOGLEVEL_D, "DownlinkData Total Time : TimeEscape_ms :%d.\r\n", elapsedTime);
    _PrintLog(LOGLEVEL_N, "WaitCmdExeQueueAvailable\r\n");
    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        _PrintLog(LOGLEVEL_E, "WaitCmdExeQueueAvailable Timeout at End, Timemoutms:%d\r\n", WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }
    StopTime = QTime::currentTime();

    File.flush();
    File.close();

    _PrintLog(LOGLEVEL_N, "SendCmdGetCRC32\r\n");
    m_pCmdHandler->SendCmdGetCRC32(&CmdPacketGetCRC32, DownlinkCRC32, UplinkCRC32);
    if (bCrcCompare) {
        pCrcCheck->GetChecksum((uint8_t*)&Checksum, 4);
        if (DownlinkCRC32 == Checksum) {
            _PrintLog(LOGLEVEL_N, "Crc32 Checksum Compare Pass, DownlinkCRC32:0x%08X, Crc32Calc:0x%08X\r\n", DownlinkCRC32, Checksum);
        }
        else {
            _PrintLog(LOGLEVEL_E, "Crc32 Checksum Compare Failed, DownlinkCRC32:0x%08X, Crc32Calc:0x%08X\r\n", DownlinkCRC32, Checksum);
        }
    }
    else {
        _PrintLog(LOGLEVEL_N, "Crc32 Checksum Downlink :0x%08X\r\n", DownlinkCRC32);
    }

    TimeEscapems = (StopTime.msecsSinceStartOfDay() - StartTime.msecsSinceStartOfDay());
    Speed = (double)FileSize / (double)(TimeEscapems) * 1000 / (double)(1024 * 1024);
    _PrintLog(LOGLEVEL_N, "FileSize:0x%I64X (Bytes), TimeEscape:%d ms, Speed:%.2lf MBytes/S, PacketCnt:%d, TransCnt:%d, %s:0x%08X, TotalCmdPacket:%d\r\n",
        FileSize, TimeEscapems, Speed, PacketCnt, TransCnt, pCrcCheck->GetName().c_str(), Checksum, CmdPacketCnt);

    //反馈给外部的结果
    ResultHash.insert("TotalSize", QString::number(FileSize, 16).toUpper().insert(0, "0x"));
    ResultHash.insert("TimeEscape_ms", TimeEscapems);
    ResultHash.insert("BytesSend", QString::number(Offset, 16).toUpper().insert(0, "0x"));
    ResultHash.insert("Crc32", QString::number(DownlinkCRC32, 16).toUpper().insert(0, "0x"));//将校验值转为16进制字符串

__end:
    if (Ret != 0) {
        m_pCmdHandler->ReleaseCmdExeQueueSemaphoreWhenFail();
    }
    if (pCrcCheck) {
        delete pCrcCheck;
    }
    if (TmpBuffer) {
        delete[] TmpBuffer;
    }
    return Ret;
}

int32_t CACCmdHandler::ReadDataAndSaveToFile(QString strFilePath, QString Type, uint32_t HopNum, uint32_t PortID, uint64_t SrcAddr, uint64_t Length,tJsonParaHash& ResultHash)
{
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
    CUplinkDataReceiver UplinkDataReceiver;
    strFilePath = QDir::fromNativeSeparators(strFilePath);
    QFile File(strFilePath);

    UplinkDataReceiver.AttachLog(m_pILog);
    UplinkDataReceiver.ConnectCmdHandler(m_pCmdHandler);

    //准备好DataWriter
    if (strFilePath == "") {
        FileWriter.SetQFile(NULL);
    }
    else {
        if (File.open(QIODevice::ReadWrite) == false) {
            QString errorStrign = File.errorString();
            _PrintLog(LOGLEVEL_E, "Open File Failed:%s\r\n", strFilePath.toStdString().c_str());
            Ret = ERR_JSONRPC_FileOpen; goto __end;
        }
        FileWriter.SetQFile(&File);
    }
    FileWriter.SetMaxDataSize(Length);
    if (bCrcCompare) {//只有需要校验的时候才进行计算
        FileWriter.AttachICrcCheck(&Crc32Std);
    }
    _PrintLog(LOGLEVEL_N, "CRCCompare Enable:%s\r\n", bCrcCompare?"YES" : "NO");
    pPrevDataWriter = UplinkDataReceiver.StoreDataWriter(&FileWriter);

    //开始数据传输包传输并获取数据
    memset(&PacketDataTrans, 0, sizeof(_tCmdPacketDataTrans));
    PacketDataTrans.MsgID = ICDMsgID_Cmd;
    PacketDataTrans.CmdID = (uint8_t)CICD::TransStrCmd2CmdID(Type);
    PacketDataTrans.HopNum = HopNum;
    PacketDataTrans.PortID = PortID;

    //等待所有命令队列空闲，这样方便后续确认所有发送的命令包都Complete
    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        _PrintLog(LOGLEVEL_E, "WaitCmdExeQueueAvailable Timeout at Start, Timemoutms:%d\r\n", WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }
    StartTime = QTime::currentTime();

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
        //_PrintLog(LOGLEVEL_N, "FirtPacketFlag:%d, FirstDWord:0x%08X\r\n", PacketDataTrans.FirtPacketFlag,*(uint32_t*)(&PacketDataTrans));
        //重新指定长度和读取的起始位置
        PacketDataTrans.LengthL = (uint32_t)(BytesRead & 0xFFFFFFFF);
        PacketDataTrans.LengthH = (uint16_t)((BytesRead >> 32) & 0xFFFFFFFF);
        PacketDataTrans.SrcAddrL = (uint32_t)(SrcAddr & 0xFFFFFFFF);
        PacketDataTrans.SrcAddrH = (uint16_t)((SrcAddr >> 32) & 0xFFFFFFFF);

        //_PrintLog(LOGLEVEL_N, "1 ReadData SrcAddr:0x%0I64X, Length:0x%0I64X\r\n", SrcAddr, BytesRead);
        CmdPacketCnt++;
        Ret = m_pCmdHandler->SendCmdPacketDataTrans(&PacketDataTrans);//发送命令传输数据包   
        if (Ret != 0) {
            _PrintLog(LOGLEVEL_E, "SrcAddr:0x%I64X, BytesRead:0x%08X,CmdPacketCnt:%d\r\n", SrcAddr, BytesRead, CmdPacketCnt);
            goto __end;
        }
        if (PacketDataTrans.CmdID == (uint8_t)eSubCmdID::SubCmd_DataTrans_DDR2FIBER) {//如果是DDR读取，不能使用队列只能等到Complete包返回才能继续
            RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(WaitCmdQueueTimeoutms);
            if (RetCall == false) {
                _PrintLog(LOGLEVEL_E, "DDR2FIBER WaitCmdExeQueueAvailable Timeout\r\n");
                Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
            }
        }
        Offset += BytesRead;
        SrcAddr += BytesRead;
    }

    //等待所有命令队列空闲，表明所有的命令包都Complete
    _PrintLog(LOGLEVEL_D, "WaitCmdExeQueueAvailable....\r\n");
    RetCall = m_pCmdHandler->WaitCmdExeQueueAvailable(WaitCmdQueueTimeoutms);
    if (RetCall == false) {
        _PrintLog(LOGLEVEL_E, "WaitCmdExeQueueAvailable Timeout at End, Timemoutms:%d\r\n", WaitCmdQueueTimeoutms);
        Ret = ERR_CMDHAND_CmdQueueAvailable; goto __end;
    }
    QThread::msleep(10);///等待这个ms数，因为命令完成包达到之后，可能还有数据未到达，延迟等待一下！
    StopTime = QTime::currentTime();

    memset(&CmdPacketGetCRC32, 0, sizeof(tCmdPacketGetCRC32));
    CmdPacketGetCRC32.MsgID = ICDMsgID_Cmd;
    CmdPacketGetCRC32.HopNum = HopNum;
    CmdPacketGetCRC32.CmdID = (uint8_t)eSubCmdID::SubCmd_ReadCRC32;
    CmdPacketGetCRC32.PortID = PortID;

    Ret = m_pCmdHandler->SendCmdGetCRC32(&CmdPacketGetCRC32, DownlinkCRC32, UplinkCRC32);
    if (Ret == 0) {
        _PrintLog(LOGLEVEL_N, "CRC32 UplinkCRC32:0x%08X,DownlinCRC32:0x%08X\r\n", UplinkCRC32, DownlinkCRC32);
    }
    else {
        goto __end;
    }
    Crc32Std.GetChecksum((uint8_t*)&TotalCrc32CheckSum, 4);///中间过程已经由DataWriter完成，这里直接获取校验值
    if (bCrcCompare) {
        if (UplinkCRC32 != TotalCrc32CheckSum) {
            _PrintLog(LOGLEVEL_E, "CRC32 Compare Failed, UplinkCRC32:0x%08X, Crc32Calc:0x%08X\r\n", UplinkCRC32, TotalCrc32CheckSum);
        }
        else {
            _PrintLog(LOGLEVEL_N, "CRC32 Compare Pass\r\n");
        }
    }


    TimeEscapems = (StopTime.msecsSinceStartOfDay() - StartTime.msecsSinceStartOfDay());
    BytesTotalRead = UplinkDataReceiver.GetDataWriterCurDataSize();
    Speed = (double)BytesTotalRead / (double)(TimeEscapems) * 1000/(double)(1024*1024);
    _PrintLog(LOGLEVEL_N, "************PacketLost:%d\r\n", UplinkDataReceiver.GetPacketLost());
    _PrintLog(LOGLEVEL_N, "TimeEscape_ms:%d ms ,Speed:%.2lf MBytes/S, Offset:0x%I64X, BytesTotalReceived:0x%I64X, %s:0x%08X, TotalCmdPacket:%d\r\n", TimeEscapems,Speed,\
        Offset, BytesTotalRead, Crc32Std.GetName().c_str(), TotalCrc32CheckSum, CmdPacketCnt);

    _PrintLog(LOGLEVEL_N, "Uplink DataPacket:%d\r\n", UplinkDataReceiver.GetTotalPacket());

    ResultHash.insert("BytesRead", QString::number(BytesTotalRead, 16).toUpper().insert(0, "0x"));
    ResultHash.insert("TimeEscape_ms", TimeEscapems);
    ResultHash.insert("Crc32", QString::number(TotalCrc32CheckSum, 16).toUpper().insert(0, "0x"));//将校验值转为16进制字符串


__end:
    if (Ret != 0) {
        m_pCmdHandler->ReleaseCmdExeQueueSemaphoreWhenFail();//出现错误，则是否所有命令队列资源，准备下次可以使用
    }
    _PrintLog(LOGLEVEL_N, "ReadData Ret:%d\r\n", Ret);

    return Ret;
}


int32_t CACCmdHandler::LinkScan(uint32_t HopNum)
{
    int32_t Ret = 0;
    tLinkScanPacket LinkScanPacket;
    memset(&LinkScanPacket, 0, sizeof(tLinkScanPacket));
    LinkScanPacket.MsgID = ICDMsgID_LinkScan;
    LinkScanPacket.HopNum = HopNum;
    _PrintLog(LOGLEVEL_N, "SendLinkScan....HopNum:%d\r\n",HopNum);
    m_pCmdHandler->SendLinkScanPacket(&LinkScanPacket);
    return Ret;
}