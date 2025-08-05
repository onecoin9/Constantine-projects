#ifndef _ICMDHANDLE_H_
#define _ICMDHANDLE_H_

#include "ACTypes.h"
#include <QObject>

class ICmdHandler : public QObject
{
public:
	ICmdHandler()
	{

	}

	virtual int32_t HandlePacket(std::string recvDevIP, uint8_t* pPckData, int Size)=0;

};

#endif 