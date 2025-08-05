
#ifndef _STALK_H_
#define _STALK_H_


#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif


#include "IFileStalk.h"


typedef struct tagIReporter tIReporter;

typedef enum{
	LOG_LEVEL_LOG,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_ERR,
}enLogLevel;

struct tagIReporter{
	void (*SetProgress)(tIReporter*pReporter,unsigned int Cur,unsigned int Total);
	void (*PrintLog)(tIReporter*pReporter,enLogLevel Level,char*pString);
	void *PrivData;
};

#define SysAlloc(_Size) (malloc(_Size))
#define SysSafeFree(_Ptr) do{if(_Ptr)free(_Ptr);_Ptr=NULL;}while(0)


#endif 