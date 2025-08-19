#ifndef _BASE64_H_
#define _BASE64_H_


typedef unsigned char     uint8;
typedef unsigned long    uint32;
uint32 base64_encode(const uint8 *text, uint32 text_len, uint8 *encode);
uint32 base64_decode(const uint8 *code, uint32 code_len, uint8 *plain);

#endif 