#ifndef _CHKSUM_H_
#define _CHKSUM_H_


#ifdef __cplusplus
extern "C" {
#endif
#include "windows.h"

typedef struct tagChkInfo{
	UINT64 chksum;	///program sum
	UINT64 sumlen;	///used when calculate chksum	
	void *PrivData;
}CHKINFO;

void Crc32CalcSubRoutine(CHKINFO*pChkInfo,UINT8* buf,UINT32 size);
void Crc32GetChkSum(CHKINFO*pChkInfo);


#ifdef __cplusplus
}
#endif

#endif