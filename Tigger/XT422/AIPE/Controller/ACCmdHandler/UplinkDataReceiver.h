#ifndef _UPLINKDATAERCEIVER_H_
#define _UPLINKDATAERCEIVER_H_


#include <QObject>
#include "IDataWriter.h"
#include "CmdHandler.h"
#include <QMutex>

//本类使用了将AG06的上行数据包写入给定的Writer而设计的。
//使用之前将本类的信号和CCmdHandler的Uplink信号连接
class CUplinkDataReceiver : public QObject
{
	Q_OBJECT
public:
	CUplinkDataReceiver();
	~CUplinkDataReceiver();
	void ConnectCmdHandler(CCmdHandler* pCmdHandler);
	void DisconnectCmdHandler();
	IDataWriter* StoreDataWriter(IDataWriter* pNewIDataWriter);
	IDataWriter* GetCurrentWriter();

	qint64 GetDataWriterCurDataSize();
	uint32_t GetTotalPacket() { return m_PacketTotal; };
	uint32_t GetPacketLost() { return m_PacketLost; }
protected:
	qint64 WriteDataByDataWriter(char* pData, qint64 Size);

public slots:
	void OnRecvDataUplink(const QByteArray PckData);
	void onSLotDoWork();
private:
	IDataWriter* m_pIDataWriter;
	QMutex m_DataWriterMutex;		//访问互斥锁
	CCmdHandler* m_pCmdHandler;
	uint32_t m_PacketCntIdx;
	uint32_t m_PacketLost;
	uint32_t m_PacketTotal;
};

#endif 