#pragma once

#include "LogMsg.h"

class CACException :public CLogMsg
{
public:
	CACException(void);
public:
	virtual ~CACException(void);
};
