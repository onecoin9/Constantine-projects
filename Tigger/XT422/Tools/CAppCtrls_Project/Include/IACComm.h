#pragma once


#include <QObject>
#include "ICD.h"
#include "ILog.h"
class IACComm: public QObject
{
public:
	IACComm() :m_pILog(NULL)
	{

	}

	void AttachILog(ILog* pLog) 
	{
		m_pILog = pLog;
	}
	virtual int SendData(uint8_t *pData,int Size)=0;
	virtual int32_t StartComm()=0;
	virtual int32_t StopComm()=0;
protected:
	ILog* m_pILog;
};

