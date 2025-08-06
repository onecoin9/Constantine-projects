/************************
Copyright  Acroview CO.,LTD.
Author : sllin
Email : sllin40@qq.com
*************************/
#include <string.h>
#include <stdlib.h>

typedef unsigned char       	INT8U;
typedef char                	INT8S;
typedef unsigned short      	INT16U;
typedef short               	INT16S;
typedef unsigned int        	INT32U;
typedef int                 	INT32S;

typedef float               	FLOAT32;
typedef double              	FLOAT64;

#define SHA_256_INITIAL_LENGTH    8

INT8U blk_buffer[128];
INT32U H32[8];                                 // last SHA result variables
INT32U M32[16];                                // SHA message schedule
INT32U a32, b32, c32, d32, e32, f32, g32, h32; // SHA working variables

INT32U SHA_256_Initial[] = 
{
	0x6a09e667,
	0xbb67ae85,
	0x3c6ef372,
	0xa54ff53a,
	0x510e527f,
	0x9b05688c,
	0x1f83d9ab,
	0x5be0cd19
};


INT32U SHA_CONSTANTS[] =  
{
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

INT32U __sha_rotr_32(INT32U val,INT16U r)
{
	return (val>>r) | (val<<(32-r));
}

INT32U __sha_shr_32(INT32U val,INT16U r)
{
	return val>>r;
}

INT32U __sha_bigsigma256_0(INT32U x)
{
	return __sha_rotr_32(x,2)^__sha_rotr_32(x,13) ^ __sha_rotr_32(x,22);
}

INT32U __sha_bigsigma256_1(INT32U x)
{
	return __sha_rotr_32(x,6)^__sha_rotr_32(x,11) ^ __sha_rotr_32(x,25);
}

INT32U __sha_littlesigma256_0(INT32U x)
{
	return __sha_rotr_32(x,7)^__sha_rotr_32(x,18) ^ __sha_shr_32(x,3);
}


INT32U __sha_littlesigma256_1(INT32U x)
{
	return __sha_rotr_32(x,17)^__sha_rotr_32(x,19) ^ __sha_shr_32(x,10);
}


INT32U __sha_ch(INT32U x,INT32U y,INT32U z)
{
   	return (x & y) ^ ((~x) & z);
}


INT32U __sha_maj(INT32U x, INT32U y, INT32U z)
{
	INT32U temp = x & y;
	temp ^= (x & z);
	temp ^= (y & z);
	return temp;  //(x & y) ^ (x & z) ^ (y & z);
}



void __sha_copy32(INT32U* src,INT32U* dest,INT32S length)
{
	while (length > 0){
		*dest++ = *src++;
		length--;
	}
}


INT32U __sha_getW(int index)
{
	INT32U newW;
	if (index < 16){
	 	return M32[index];
	}

	newW = 	__sha_littlesigma256_1(M32[(index-2)&0x0f]) + M32[(index-7)&0x0f] +
		 	__sha_littlesigma256_0(M32[(index-15)&0x0f]) + M32[(index-16)&0x0f];
	
	M32[index & 0x0f] = newW;
	return newW;
}


//----------------------------------------------------------------------
// Prepair the block for hashing
//
void __sha_prepare_schedule(INT8U* message)
{
	int i,j;
	INT32U temp;
	for (i = 0; i < 16; i++){
		temp = 0;
		for (j = 0; j < 4;j++){
			temp = temp << 8;
			temp = temp | (*message & 0xff);
			message++;
		}
		M32[i] = temp;
	}
}

int __sha256_hashblock(INT8U *blk_message,int lastblock)
{
	int t=0;
	INT32U Wt, Kt;
	INT32U T1,T2;
	__sha_prepare_schedule(blk_message);
	a32 = H32[0];
	b32 = H32[1];
	c32 = H32[2];
	d32 = H32[3];
	e32 = H32[4];
	f32 = H32[5];
	g32 = H32[6];
	h32 = H32[7];
	
	for(t=0;t<64;t++){
		Wt = __sha_getW(t);
		Kt = SHA_CONSTANTS[t]; 
		T1 = h32+__sha_bigsigma256_1(e32)+__sha_ch(e32,f32,g32)+Kt+Wt;
		T2 = __sha_bigsigma256_0(a32)+__sha_maj(a32,b32,c32);
		h32 = g32;
		g32 = f32;
		f32 = e32;
		e32 = d32+T1;
		d32 = c32;
		c32 = b32;
		b32 = a32;
		a32 = T1+T2;
	}

	if (!lastblock){
		H32[0] += a32;
		H32[1] += b32;
		H32[2] += c32;
		H32[3] += d32;
		H32[4] += e32;
		H32[5] += f32;
		H32[6] += g32;
		H32[7] += h32;
	}
	else{//skip adding constant
		H32[0] = a32;
		H32[1] = b32;
		H32[2] = c32;
		H32[3] = d32;
		H32[4] = e32;
		H32[5] = f32;
		H32[6] = g32;
		H32[7] = h32;
	}

	return 0;
}

/***

按照大端将n个32bit的input 数据放到output中
*****/
void __sha_copyWordsToBytes32(INT32U* input,INT8U* output,int numwords)
{
	INT32U temp;
	int i;

	/****
	output:0表示低字节
	H0[3]	H0[2]	H0[1]	H0[0]
	H1[3]	H1[2]	H1[1]	H1[0]
	.
	.
	.
	H7[3]	H7[2]	H7[1]	H7[0]
	***/
	for (i=0;i<numwords;i++){
		temp = *input++;
		*output++ = (INT8U)(temp >> 24);
		*output++ = (INT8U)(temp >> 16);
		*output++ = (INT8U)(temp >> 8);
		*output++ = (INT8U)(temp);
	}
}

void __sha_write_result(int reverse,INT8U* outpointer)
{
	int i;
	INT8U tmp;

	__sha_copyWordsToBytes32(H32, outpointer, 8); 

	if (reverse){
		for (i = 0; i < 16; i++){  
			tmp = outpointer[i];
			outpointer[i] = outpointer[31-i];
			outpointer[31-i] = tmp;
		}
	}
	/**
	reverse之后:
	output:0表示低字节
	H7[0]	H7[1]	H7[2]	H7[3]
	H6[0]	H6[1]	H6[2]	H6[3]
	.
	.
	.
	H0[0]	H0[1]	H0[2]	H0[3]
	***/

}


/*************************************
Computes SHA-256 given the data block 'message' with no padding. 
The result is returned in 'digest'.   

message :buffer containing the message 
length : the length of the message
skipconst : skip adding constant on last block (skipconst=1)
reverse : reverse order of digest (reverse=1, MSWord first, LSByte first)
digest : result hash digest in byte order used by 1-Wire device
********************************************/

int sha256_compute(INT8U *message,INT32U length,int skipconst,int reverse,INT8U* digest)
{
	INT32U pos=0;
	INT32U bytes_per_blk=64;
	int blk_num=1;
	int i=0,j=0;
	int last_blk=0;
	INT32U bitlength=0;
	__sha_copy32(SHA_256_Initial,H32,SHA_256_INITIAL_LENGTH);

	 // 1 byte for the '80' that follows the message, 8 bytes of length
	blk_num = (length+1+8+(bytes_per_blk-1))/bytes_per_blk; 

	bitlength = 8 * length;
	for(i=0;i<blk_num;i++){
		if (length >= bytes_per_blk){
			memcpy(blk_buffer, message, bytes_per_blk);
			length -= bytes_per_blk;
		}
		else{
			memcpy(blk_buffer, message, length);
			pos = length;
			
			blk_buffer[pos++]=0x80;///需要补充数据，message之后的一个bit为1，其他为0;
			while (pos < bytes_per_blk){// this loop is inserting padding, in this case all zeroes
				blk_buffer[pos++]=0x00;
			}
		}
		last_blk = (i==blk_num-1)?1:0;
		if(last_blk){// point at the last byte in the block
			pos = bytes_per_blk-1;
			for (j = 0; j < 8; j++){
				blk_buffer[pos--] = (INT8U)bitlength;
				bitlength = bitlength >> 8;
			}
		}
		__sha256_hashblock(blk_buffer,last_blk&skipconst);
		message += bytes_per_blk;
	}

	__sha_write_result(reverse,digest);

	return 0;
}

// hold secret for creating a 
int sha256_compute_mac256(INT8U* MT,int length,INT8U * MAC)
{

	if (length == 119){//64*2-1-8 =119    // check for two block format
		// insert secret
		sha256_compute(MT, 119, 1, 1, MAC);
	}
 	else{//one block format
		// insert secret
		sha256_compute(MT, 55, 1, 1, MAC);
	}

  	return 0;
}

/**
校验256
***/
int sha256_verify_mac256(INT8U* MT,int length,INT8U * compare_MAC)
{
	INT8U calc_mac[32];
	int i;
	sha256_compute_mac256(MT, length, calc_mac);

	// Compare calculated mac with one read from device
	for (i = 0; i < 32; i++){
		if (compare_MAC[i] != calc_mac[i])
			return 0;
	}
	return 1;
}

/***
计算包含密码的MAC值，
passwd：6字节的密码首地址，ASCII值，
pmac:32个字节存放MAC的首地址
****/
int Get_MACofUSBKey(const char *passwd,unsigned char *pmac)
{ 
	unsigned char MT[64];
	int i=0;
	memset(MT,0xAA,64);
	for(i=0;i<6;i++)
		MT[i]=passwd[i];
	for(i=6;i<12;i++)
		MT[i]=~passwd[i-6];
	sha256_compute_mac256(MT,55,pmac);

	return 0;
}


/***
计算包含密码的MAC值，
USBKey:  USBKey字符串
Size: 字符串的长度
pmac:32个字节存放MAC的首地址
return 0表示成功，-1表示失败

///通过这个算法验证DLL的有效性
****/
int Get_USBKeySNMac(const char*USBKey,int Size,unsigned char *pmac)
{ 
	int Ret=0;
	unsigned char*pMTData=NULL;
	int MTSize=Size*2;
	int i=0;
	pMTData=(unsigned char*)malloc(MTSize);
	if(!pMTData){
		Ret=-1; goto __end;
	}
	memcpy(pMTData,USBKey,Size);
	for(i=Size;i<MTSize;++i){
		pMTData[i]=USBKey[MTSize-i-1];
	}
	sha256_compute(pMTData, MTSize+1, 1, 1, pmac);

__end:
	if(pMTData){
		free(pMTData);
	}
	return Ret;
}