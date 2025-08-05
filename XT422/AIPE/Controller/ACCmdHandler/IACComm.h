#pragma once


#include <QObject>
#include "ICD.h"
class IACComm: public QObject
{
public:
	IACComm()
	{

	}

	virtual int SendData(std::string devIP, uint8_t *pData,int Size)=0;
	virtual int32_t StartComm()=0;
	virtual int32_t StopComm()=0;

};

