#include "AngKIOHeapDevice.h"
#include <QtGlobal>

AngKIOHeapDevice::AngKIOHeapDevice(QObject *parent)
	: QIODevice(parent)
{
}

AngKIOHeapDevice::~AngKIOHeapDevice()
{
}

qint64 AngKIOHeapDevice::readData(char* data, qint64 maxSize)
{
	qint64 bytesRead = qMin(maxSize, (qint64)buffer.size());

	if (bytesRead > 0) {
		// 从缓冲区中复制数据到传入的 data 中
		memcpy(data, buffer.constData(), bytesRead);

		// 从缓冲区中移除已读取的数据
		buffer.remove(0, bytesRead);
	}

	return bytesRead;
}

qint64 AngKIOHeapDevice::writeData(const char* data, qint64 maxSize)
{
	// 将数据添加到缓冲区中
	buffer.append(data, maxSize);

	// 发送 readyRead 信号，表示有新的数据可读
	emit readyRead();

	return maxSize;
}
