#pragma once

#include "AngKIOHeapDevice.h"

class IDataBuffer;
class AngKIOHeapManager : public AngKIOHeapDevice
{
	Q_OBJECT

public:
	AngKIOHeapManager(QObject *parent = nullptr);
	~AngKIOHeapManager();

	virtual int32_t HeapWrite(uint64_t offset, uint32_t uAlgo, IOChannel channelType, DataArea areaType, QByteArray& pData);

	virtual int32_t HeapRead(uint64_t offset, uint32_t uAlgo, IOChannel channelType, DataArea areaType, QByteArray& pData);

	void setDataBuffer(IDataBuffer* _dataBuf);
private:
	uint64_t m_nBaseAddr;
	IDataBuffer* m_DataBuffer;
};
