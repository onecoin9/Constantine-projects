#pragma once

#include <QIODevice>
#include "IOHeapDeviceDefine.h"

class AngKIOHeapDevice : public QIODevice
{
	Q_OBJECT

public:
	AngKIOHeapDevice(QObject *parent = nullptr);
	~AngKIOHeapDevice();

	virtual int32_t HeapWrite(uint64_t offset, uint32_t uAlgo, IOChannel channelType, DataArea areaType, QByteArray& pData) = 0;

	virtual int32_t HeapRead(uint64_t offset, uint32_t uAlgo, IOChannel channelType, DataArea areaType, QByteArray& pData) = 0;

	qint64 readData(char* data, qint64 maxSize) override;

	qint64 writeData(const char* data, qint64 maxSize) override;

private:
	QByteArray buffer;
};
