﻿#pragma once

#include <QObject>
typedef uint64_t ADR;

/*该类作为操作Buffer的初始类，需要操作Buffer数据继承实现即可*/

class IDataBuffer : public QObject
{
	Q_OBJECT

public:
	IDataBuffer(QObject* parent) {};
	virtual ~IDataBuffer() {};

	virtual ADR BufferRead(ADR adrStart, uchar* pBuf, ADR adrLen) = 0;

	virtual ADR BufferWrite(ADR adrStart, uchar* pBuf, ADR adrLen) = 0;

	virtual int BufferSwitchPartition(int nPartIndex) = 0;

	virtual int BufferGetCurPartition() = 0;

	virtual void BufferAddTemporaryDataRemap() = 0;

	virtual void BufferRemoveTemporaryDataRemap() = 0;

	virtual void BufferCheckPartitionExist() = 0;
};
