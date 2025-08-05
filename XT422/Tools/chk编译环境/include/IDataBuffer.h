#pragma once

#include <iostream>
typedef uint64_t ADR;

/*������Ϊ����Buffer�ĳ�ʼ�࣬��Ҫ����Buffer���ݼ̳�ʵ�ּ���*/

class IDataBuffer
{
public:
	IDataBuffer() {};
	virtual ~IDataBuffer() {};

	virtual ADR BufferRead(ADR adrStart, unsigned char* pBuf, ADR adrLen) = 0;

	virtual ADR BufferWrite(ADR adrStart, unsigned char* pBuf, ADR adrLen) = 0;

	virtual int BufferSwitchPartition(int nPartIndex) = 0;

	virtual int BufferGetCurPartition() = 0;

	virtual void BufferAddTemporaryDataRemap() = 0;

	virtual void BufferRemoveTemporaryDataRemap() = 0;

	virtual void BufferCheckPartitionExist() = 0;
};
