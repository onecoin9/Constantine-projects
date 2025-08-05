#ifndef _ICMDHANDLE_H_
#define _ICMDHANDLE_H_

#include "ACTypes.h"
#include "ILog.h"
#include <QObject>

class ICmdHandler : public QObject
{
public:
	ICmdHandler() :m_pILog(NULL)
	{

	}
	void AttachILog(ILog* pLog)
	{
		m_pILog = pLog;
	}
	virtual int32_t HandlePacket(uint8_t* pPckData, int Size)=0;

protected:
	ILog* m_pILog;
};

#endif 