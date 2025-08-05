#ifndef _DEVLISTBASE_H_
#define _DEVLISTBASE_H_
#include "AprogDev.h"

class CDevListBase
{
public:
	CDevListBase(){};
	virtual ~CDevListBase(){};

	virtual int AddItem(const DEVINFO &DevInfo)=0;
	virtual int RemoveItem(int Item,const std::string &DevSN)=0;
	virtual int ClearAllItem()=0;
};

#endif