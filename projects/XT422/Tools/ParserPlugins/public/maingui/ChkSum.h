#ifndef _CHKSUM_H_
#define _CHKSUM_H_


#ifdef __cplusplus
extern "C" {
#endif
//#include "windows.h"
typedef struct tagChkInfo{
	unsigned long long chksum;	///program sum
	unsigned long long sumlen;	///used when calculate chksum	
	void *PrivData;
}CHKINFO;
extern unsigned int Table_CRC32[256];
extern void Crc32CalcSubRoutine(CHKINFO* pChkInfo, unsigned char* buf,unsigned int size);
extern void Crc32GetChkSum(CHKINFO* pChkInfo);


#ifdef __cplusplus
}
#endif

#endif