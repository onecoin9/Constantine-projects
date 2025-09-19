#ifndef _CRC_H_
#define _CRC_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t calc_crc16sum(uint8_t* buf, uint32_t size,uint16_t *pCRC16Sum);


#ifdef __cplusplus
};
#endif


#endif
