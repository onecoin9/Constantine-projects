#include "UplinkDataReceiver.h"
#include "ICD.h"


#define _PrintLog(_Level,fmt,...) \
	if (m_pILog) {\
		m_pILog->PrintLog(_Level, fmt, __VA_ARGS__);\
	}

CUplinkDataReceiver::CUplinkDataReceiver()
    :m_pILog(NULL)
    ,m_pIDataWriter(NULL)
    ,m_pCmdHandler(NULL)
    ,m_PacketCntIdx(0)
    ,m_PacketLost(0)
    ,m_PacketTotal(0)
{

}

CUplinkDataReceiver::~CUplinkDataReceiver()
{
    DisconnectCmdHandler();
}

void CUplinkDataReceiver::ConnectCmdHandler(CCmdHandler * pCmdHandler)
{
    connect(pCmdHandler, &CCmdHandler::sigRecvDataUplink, this,&CUplinkDataReceiver::OnRecvDataUplink, Qt::DirectConnection);//这个地方要采用直接调用的方式，否则数据处理对象要放到另外一个线程中
    //connect(pCmdHandler, &CCmdHandler::sigRecvDataUplink, this, &CUplinkDataReceiver::OnRecvDataUplink, Qt::QueuedConnection);
    m_pCmdHandler = pCmdHandler;
}

void CUplinkDataReceiver::DisconnectCmdHandler()
{
    if (m_pCmdHandler) {
        disconnect(m_pCmdHandler, &CCmdHandler::sigRecvDataUplink, this, 0);
    }
}

/// <summary>
/// 设置新的指针，返回旧指针
/// </summary>
IDataWriter* CUplinkDataReceiver::StoreDataWriter(IDataWriter * pNewIDataWriter)
{
    IDataWriter* pPrev = NULL;
    m_DataWriterMutex.lock();
    pPrev = m_pIDataWriter;
    m_pIDataWriter = pNewIDataWriter;
    m_DataWriterMutex.unlock();
    return pPrev;
}

IDataWriter* CUplinkDataReceiver::GetCurrentWriter()
{
    IDataWriter* pCur = NULL;
    m_DataWriterMutex.lock();
    pCur = m_pIDataWriter;
    m_DataWriterMutex.unlock();
    return pCur;
}

/// <summary>
/// 写操作，pData为写入的数据，Size为实际的字节数，返回实际写入的字节数，小于0表示失败
/// </summary>
qint64 CUplinkDataReceiver::WriteDataByDataWriter(char* pData, qint64 Size)
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

qint64 CUplinkDataReceiver::GetDataWriterCurDataSize()
{
    qint64 WriteBytes = 0;
    m_DataWriterMutex.lock();
    WriteBytes = m_pIDataWriter->GetCurDataSize();
    m_DataWriterMutex.unlock();
    return WriteBytes;
}

void CUplinkDataReceiver::OnRecvDataUplink(const QByteArray PckData)
{
    tDataPacket* pDataPacket = (tDataPacket*)PckData.constData();
    //_PrintLog(LOGLEVEL_D, "CUplinkDataReceiver::DataArray Ptr:0x%I64X, Data[0]=0x%08X, PackNum:%d\r\n", PckData.constData(), *(uint32_t*)pDataPacket->Data,pDataPacket->PNum);
    if (pDataPacket->FristFlag == 1) {
        m_PacketCntIdx = 0;
        if (pDataPacket->PNum != 0) {
            _PrintLog(LOGLEVEL_E, "FirstFlag=1,But PNum=%d\r\n", pDataPacket->PNum);
        }
    }
    
    if (pDataPacket->PNum != m_PacketCntIdx) {
        _PrintLog(LOGLEVEL_E, "CUplinkDataReceiver DataLost::DataArray PackNumRecv:%d,Expected:%d,CurTotalPacket:%d,Length:0x%08X\r\n",pDataPacket->PNum, m_PacketCntIdx, m_PacketTotal, pDataPacket->Length*4);
        m_PacketLost += pDataPacket->PNum - m_PacketCntIdx;
        m_PacketCntIdx = pDataPacket->PNum;
    }
    WriteDataByDataWriter((char*)&pDataPacket->Data, (qint64)pDataPacket->Length * 4);
    m_PacketCntIdx++;
    m_PacketTotal++;
   
    if (m_PacketCntIdx > 65535) {//这个地方如果大于255则重新回到0
        m_PacketCntIdx = 0;
    }
}