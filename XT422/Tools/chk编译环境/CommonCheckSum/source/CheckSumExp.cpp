// CheckSumExp.cpp : Defines the initialization routines for the DLL.
//

#include "../header/json.hpp"
#include "../header/CheckSum.h"
#include <iostream>

#define EMMCSUM_ADDR (1000) ////eMMCSum Address
#define EMMCSUM_EXTCSDADDR (1004)
#define EMMCSUM_PARA (900) ///eMMC Parameters
//***********************************************************************************//
//�˴����ַ������Ƽ�˳��������ı�(checksum.h)����̶�,ȷ�����ֵ��û������һ���ԣ�
//�����µ�checksum typeʱ��һ��ȷ�Ϻ���������checksum�����е�typeҲ��������ı�*****//
//���м���ĳ��checksum��֧��ʱ��sumtype_idx[]��ֵ��ƫ�ƣ���������Ӧ��CHECKSUM_xxɾ��.
//**********************************************************************************//
//                          none         , Byte_sum    ,word_sum,     crc16          crc32,         crc64,
unsigned int sumtype_idx[]={CHECKSUM_NONE,CHECKSUM_BYTE};


uint32_t  DllGetCkSumName(char *lpBuff, uint32_t uiLen)
{
	char *p = lpBuff;
	char *pend = lpBuff + uiLen;
	memset(lpBuff, 0, uiLen);
 
	strncpy(p, "Byte", strlen("Byte"));	//crc16 sum
	p += strlen("Byte") + 1;	 
	return 1;		//��sum type����ʱ������������Ӧ����.		
}
//**************************************************************************//

#ifndef SP_BUFF_SIZE
#define SP_BUFF_SIZE (1024)
#endif

#define PHISON_CHIP_OFFSET (0x90000)

#define BUFLEN_MAX (0x80000)

uint64_t DllCheckSum(CHECKSUMPARAM *param)
{
	if (!param){
		std::cout << "param is NULL, CheckSum out !!!!!" << std::endl;
		return 0;
	}
	unsigned long dwPos;
	unsigned long dwLen=0;
	unsigned long dwReadLen;
	unsigned long showprocessFlag=1;    //default set show process flag
	uint32_t i;
	unsigned long dwCheckSum = 0;
	unsigned long dwSumCount = 0;
	unsigned long uiVChkSum = 0;
	unsigned short dwMaskValue = 0xFFFF;
	unsigned short BlkNum;
	char Msg[128]={0};
    const unsigned char *pcSpcBuffer =param->pcSpcBuffer;     //special bit buffer data
	if((param->uiAgIc&0xFFFF0000)==PHISON_CHIP_OFFSET){
		char SumType=*(char*)(pcSpcBuffer+EMMCSUM_PARA);
		//pOutput->Log("Call Phison Checksum Calcuation...");
		sprintf(Msg,"Phison Checksum Calcuating");
		PrintLog(param,0,Msg);
		if(SumType==1){
			sprintf(Msg,"Checkum from master chip");
			PrintLog(param,0,Msg);
			dwCheckSum=*(unsigned long*)(pcSpcBuffer+EMMCSUM_ADDR);
		}
		else{
			sprintf(Msg,"Checkum from data buffer");
			PrintLog(param,0,Msg);
			param->uiCheckSumType=sumtype_idx[param->uiCheckSumType];
			dwCheckSum=(unsigned long)Std_Checksum_Cal(param,0,BUFLEN_MAX,dwCheckSum,dwMaskValue,&dwSumCount,showprocessFlag);
			if(param->uiCheckSumType==CHECKSUM_CRC32){
				dwCheckSum = Get_CRC32sum(dwCheckSum,&dwSumCount);
			}
		}
		
		//strMsg.Format("ChecksumType=%d, Checksum=0x%08X",param->uiCheckSumType,dwCheckSum);
		//pOutput->Log((LPSTR)(LPCSTR)strMsg);
		return dwCheckSum;
	}
	else{
		param->uiCheckSumType= sumtype_idx[param->uiCheckSumType];
		dwCheckSum=*(unsigned long*)(pcSpcBuffer+EMMCSUM_ADDR);
		return dwCheckSum;
	}
	return 0;
}

#if 1
///Return 0 means successful, -1 means error, -2 means pStrShow is too small,
//extern "C" __declspec(dllexport)
//int DllGetChecksumShow(CHECKSUMPARAM *param,char*pStrShow,int Size)
//{
//	memset(pStrShow,0,Size);
//	const unsigned char*pcSpcBuffer =param->pcSpcBuffer;
//	unsigned long ExtCSDChecksum=*(unsigned long*)(pcSpcBuffer+EMMCSUM_EXTCSDADDR);
//	char cbuff[128]={0};  
//	sprintf(pStrShow,"{\
//\"ChecksumToShow\":[\
//{\"Name\":\"ExtCSDSum\",\"Value\":\"0x%08X\"}\
//	]}",ExtCSDChecksum);
//
//	sprintf(cbuff, (char*)"ExtCSDSum:0x%08X",ExtCSDChecksum);		
//        PrintLog(param,LOGNORMAL,cbuff);
//	return 0;
//}
#endif

////////////////////////////////
