#include "JedPlayerImp.h"
#include <math.h>
#define __IsHexChar(_Char) ((_Char>='0'&&_Char<='9')||(_Char>='a'&&_Char<='f')||(_Char>='A'&&_Char<='F'))
#define __IsBinaryChar(_Char) (_Char>='0'&&_Char<='1')
#define __IsDecChar(_Char) ((_Char>='0'&&_Char<='9')


///分配FuseData,需要在解析QF之后
int FuseDataMap_AllocData(tJedPlayer*pPlayer,int TotalFuseBit)
{
	int Ret=0;
	tJedPlayerData* pPlayerData=GetJedPlayerData(pPlayer);
	tFuseDataMap *pFuseDataMap=&pPlayerData->FuseDataMap;
	pFuseDataMap->pData=SysAlloc((TotalFuseBit+7)/8);
	if(!pFuseDataMap->pData){
		Ret=-1; 
	}
	else{
		pFuseDataMap->DataBitSize=TotalFuseBit;
	}
	pFuseDataMap->NextFuseListIdx=-1; ///初始为-1
	pFuseDataMap->nFirstFuseFalg = -1;
	pFuseDataMap->nDefaultFalg = -1; //初始为-1，没有F项
	pFuseDataMap->nTotalRealBitSize = 0;
	return Ret;
}

static tBitDataMap* __FuseDataMap_AllocBitMap()
{
	tBitDataMap*pBitDataMap=SysAlloc(sizeof(tBitDataMap));
	if(pBitDataMap){
		memset(pBitDataMap,0,sizeof(tBitDataMap));
	}
	return pBitDataMap;
}

static int __FuseDataMap_AppendBitDataMap(tJedPlayer*pPlayer,tBitDataMap*pBitDataMap)
{
	int Ret=0;
	tJedPlayerData* pPlayerData=GetJedPlayerData(pPlayer);
	if(!pPlayerData->FuseDataMap.pBitMapHead){
		pPlayerData->FuseDataMap.pBitMapHead=pBitDataMap;
	}
	else{
		///将给定的BitDataMap放到链表的最后
		tBitDataMap*pBitDataMapTemp=pPlayerData->FuseDataMap.pBitMapHead;
		while(pBitDataMapTemp){
			if(!pBitDataMapTemp->Next){
				pBitDataMapTemp->Next=pBitDataMap;
				break;
			}
			pBitDataMapTemp = pBitDataMapTemp->Next;
		}
	}
	return 0;
}

static int __SetFuseDataDefault(tJedPlayer*pPlayer,int Defualt)
{
	int Ret=0;
	tJedPlayerData* pPlayerData=GetJedPlayerData(pPlayer);
	if(!pPlayerData->FuseDataMap.pData){
		Ret=-1;
	}
	else{
		pPlayerData->FuseDataMap.nDefaultFalg = Defualt;
		///根据默认值设置全部数据
		if(Defualt){
			memset(pPlayerData->FuseDataMap.pData,0xFF,(pPlayerData->FuseDataMap.DataBitSize+7)/8);
		}
		else{
			memset(pPlayerData->FuseDataMap.pData,0x00,(pPlayerData->FuseDataMap.DataBitSize+7)/8);
		}
	}
	return Ret;
}

///在不需要的时候进行释放
void FuseDataMap_FreeData(tJedPlayer*pPlayer)
{
	tJedPlayerData* pPlayerData=GetJedPlayerData(pPlayer);
	tBitDataMap*pBitDataMap1=NULL,*pBitDataMap2=NULL;
	if(pPlayerData->FuseDataMap.pData){
		SysSafeFree(pPlayerData->FuseDataMap.pData);
	}
	pBitDataMap1=pPlayerData->FuseDataMap.pBitMapHead;
	while(pBitDataMap1){
		pBitDataMap2=pBitDataMap1->Next;
		SysSafeFree(pBitDataMap1);
		pBitDataMap1=pBitDataMap2;
	}
	pPlayerData->FuseDataMap.pBitMapHead=NULL;
}

void FeatureRowBit_Free(tJedPlayer *pPlayer)
{
	tJedPlayerData* pPlayerData=GetJedPlayerData(pPlayer);
	if(pPlayerData->FeatureRowBit.pFuseData){
		SysSafeFree(pPlayerData->FeatureRowBit.pFuseData);
	}
}

void UserCode_Free(tJedPlayer *pPlayer)
{
	tJedPlayerData* pPlayerData=GetJedPlayerData(pPlayer);
	if(pPlayerData->UserCode.pFuseData){
		SysSafeFree(pPlayerData->UserCode.pFuseData);
	}
}

static unsigned int __GetHexNumber(char*pStart,char End,int*pNumLen)
{
	char *pTmp=pStart;
	unsigned int Value=0;
	int StrLen=0;
	while(*pTmp!=0 && *pTmp!=End){
		if(*pTmp>='0' && *pTmp<='9'){
			Value=Value*16+(*pTmp-'0');
			pTmp++;
			StrLen++;
		}
		else if(*pTmp>='a' && *pTmp<='f'){
			Value=Value*16+(*pTmp-'a')+10;
			pTmp++;
			StrLen++;
		}
		else if(*pTmp>='A' && *pTmp<='F'){
			Value=Value*16+(*pTmp-'A')+10;
			pTmp++;
			StrLen++;
		}
		else{
			break;
		}
	}
	if(pNumLen){
		*pNumLen=StrLen;
	}
	return Value;
}

static unsigned int __GetDecNumber(char*pStart,char End,int*pNumLen)
{
	char *pTmp=pStart;
	unsigned int Value=0;
	int StrLen=0;
	while(*pTmp!=0 && *pTmp!=End){
		if(*pTmp>='0' && *pTmp<='9'){
			Value=Value*10+(*pTmp-'0');
			pTmp++;
			StrLen++;
		}
		else{
			break;
		}
	}
	if(pNumLen){
		*pNumLen=StrLen;
	}
	return Value;
}

static int __A_HandlerAccessTime(tJedPlayer*pPlayer,char*pString,int Size)
{
	int Ret=0;
	tIReporter *pReporter=pPlayer->pReporter;
	pReporter->PrintLog(pReporter,LOG_LEVEL_LOG,"A Identifier Not Support Yet\r\n");		
	return -1;
}

static int __C_HandlerChecksum(tJedPlayer*pPlayer,char*pString,int Size)
{
	int Ret=0;
	///如果出现多个Checksum，以最后一个Checksum为准
	tJedPlayerData *pPlayerData=GetJedPlayerData(pPlayer);
	pPlayerData->ChksumFuseListFile=__GetHexNumber(pString+1,'*',NULL);
	return Ret;
}


static int __C_ActionChecksum(tJedPlayer*pPlayer)
{
	int Ret=0;
	/*
	///由于会出现多个Checksum，需要以最后一个为准，所以不能再获取到一个Checksum的时候就进行判断
	tJedPlayerData *pPlayerData=GetJedPlayerData(pPlayer);
	if(pPlayerData->ChksumFuseListCalc!=pPlayerData->ChksumFuseListFile){
		SetErrMsg(pPlayerData,-1,"[ErrMsg] FuseList Checksum Compare Error, CalcChk=0x%04X, FileChk=0x%04X",
			pPlayerData->ChksumFuseListCalc,pPlayerData->ChksumFuseListFile);
		Ret=-1;
	}*/
	return Ret;
}

static int __D_HandlerDeviceType(tJedPlayer*pPlayer,char*pString,int Size)
{
	int Ret=0;
	tIReporter *pReporter=pPlayer->pReporter;
	pReporter->PrintLog(pReporter,LOG_LEVEL_LOG,"D Identifier Not Support Yet\r\n");		
	return -1;
}

////得到一组Fuse Data之后就根据地址将数据处理掉
static int __E_ActionElecticalFuseData(tJedPlayer*pPlayer)
{
	int Ret=0;
	tJedPlayerData *pPlayerData=GetJedPlayerData(pPlayer);
	tFuseData *pFuseList=&pPlayerData->EFuseData;
	tFuseData *pFeatureRowBit=&pPlayerData->FeatureRowBit;
	if(pFuseList->pFuseData){
		int FeatureRowBitBytes=(pFuseList->DataBitLen+7)/8;
		pFeatureRowBit->pFuseData=SysAlloc(FeatureRowBitBytes);
		if(!pFeatureRowBit->pFuseData){
			pPlayer->pReporter->PrintLog(pPlayer->pReporter,LOG_LEVEL_ERR,"Save Feature Row/Bit Failed");
			Ret=-1;
		}
		else{
			memset(pFeatureRowBit->pFuseData,0,FeatureRowBitBytes);
			memcpy(pFeatureRowBit->pFuseData,pFuseList->pFuseData,FeatureRowBitBytes);
			pFeatureRowBit->DataBitLen=pFuseList->DataBitLen;
			pFeatureRowBit->DataBitMaxLen=FeatureRowBitBytes*8;
			pFeatureRowBit->FuseStartNum=0;
		}
		SysSafeFree(pFuseList->pFuseData);
	}
	pFuseList->DataBitLen=0;
	pFuseList->DataBitMaxLen=0;
	pFuseList->FuseStartNum=0;
	return Ret;
}

static int __E_HandlerElecticalFuseData(tJedPlayer*pPlayer,char*pString,int Size)
{
	int Ret=0,bHexMode=0;
	char *pBit=NULL,*pHex=NULL;
	unsigned char *pData=NULL;
	tJedPlayerData *pPlayerData=GetJedPlayerData(pPlayer);
	tFuseData *pFuseList=&pPlayerData->EFuseData;
	unsigned char *pDataWrite=NULL;
	if(pString[1]=='H'){
		bHexMode=1;
	}
	if(bHexMode==0){
		if(pPlayerData->bStringIDIn){///有可能被分为两行，后面一行没有ID
			pBit=pString+1;	
		}
		else{
			pBit=pString;
		}
		while(*pBit!=0){
			if(*pBit>='0' && *pBit<='1'){	
				if(pFuseList->DataBitLen>=pFuseList->DataBitMaxLen){	///需要重新分配空间了
					unsigned int PreLen=pFuseList->DataBitMaxLen;
					pFuseList->DataBitMaxLen +=FUSELIST_CACHEDATA*8;
					pData=(unsigned char*)SysAlloc(pFuseList->DataBitMaxLen/8);
					if(!pData){
						Ret=-1; goto __end;
					}
					memset(pData,0,pFuseList->DataBitMaxLen/8);
					if(pFuseList->pFuseData!=NULL){
						memcpy(pData,pFuseList->pFuseData,PreLen/8);///将原来的数据先拷贝到新的内存中
					}
					pFuseList->pFuseData=pData;
				}
				///LSB 在前,JED标准档案中描述的是MSB在前，但是实际与芯片手册冲突
				//pFuseList->pFuseData[pFuseList->DataBitLen/8] |= (*pBit-'0')<<(7-(pFuseList->DataBitLen%8));
				pFuseList->pFuseData[pFuseList->DataBitLen/8] |= (*pBit-'0')<<(pFuseList->DataBitLen%8);
				pFuseList->DataBitLen++;	
			}
			pBit++;
		}
	}
	else{
		unsigned char TmpChar=0;
		pHex=pString+2;
		while(*pHex!=0){
			if(__IsHexChar(*pHex)){	
				if(pFuseList->DataBitLen>=pFuseList->DataBitMaxLen){	///需要重新分配空间了
					unsigned int PreLen=pFuseList->DataBitMaxLen;
					pFuseList->DataBitMaxLen +=FUSELIST_CACHEDATA*8;
					pData=(unsigned char*)SysAlloc(pFuseList->DataBitMaxLen/8);
					if(!pData){
						Ret=-1; goto __end;
					}
					memset(pData,0,pFuseList->DataBitMaxLen/8);
					if(pFuseList->pFuseData!=NULL){
						memcpy(pData,pFuseList->pFuseData,PreLen/8);///将原来的数据先拷贝到新的内存中
					}
					pFuseList->pFuseData=pData;
				}
				if(*pHex>='0'&&*pHex<='9'){
					TmpChar=*pHex-'0';
				}
				else if(*pHex>='a'&&*pHex<='f'){
					TmpChar=*pHex-'a'+10;
				}
				else{
					TmpChar=*pHex-'A'+10;
				}
				if((pFuseList->DataBitLen/4)%2==0){///为高4bit
					pFuseList->pFuseData[pFuseList->DataBitLen/8] |=TmpChar<<4;
				}
				else{///低4bit
					pFuseList->pFuseData[pFuseList->DataBitLen/8] |=TmpChar;
				}
				pFuseList->DataBitLen+=4;///每获得一个字符，相当于递增4个bit	
			}
			pHex++;
		}
	}
	
__end:
	return Ret;
}

static int __F_HandlerDefaultFuseState(tJedPlayer*pPlayer,char*pString,int Size)
{
	int Ret=0;
	tJedPlayerData *pPlayerData=GetJedPlayerData(pPlayer);
	pPlayerData->DefaultFuseState=(unsigned char)__GetDecNumber(pString+1,JED_ASTERISK,NULL);
	///设置默认的状态
	Ret=__SetFuseDataDefault(pPlayer,pPlayerData->DefaultFuseState);
	return Ret;
}

static int __G_HandlerSecurityFuse(tJedPlayer*pPlayer,char*pString,int Size)
{
	int Ret=0;
	tIReporter *pReporter=pPlayer->pReporter;
	tJedPlayerData *pPlayerData=GetJedPlayerData(pPlayer);
	tFuseDataMap *pFuseDataMap=&pPlayerData->FuseDataMap;
	pPlayerData->SecurityFuseProgramEn=__GetDecNumber(pString+1,'*',NULL);

	//////////
	pFuseDataMap->nGValue = pPlayerData->SecurityFuseProgramEn;
	/////////
	return Ret;
}

static int __J_HandlerDeviceIdentification(tJedPlayer*pPlayer,char*pString,int Size)
{
	int Ret=0;
	int StrLen=0;
	tJedPlayerData *pPlayerData=GetJedPlayerData(pPlayer);
	tIReporter *pReporter=pPlayer->pReporter;
	//pReporter->PrintLog(pReporter,LOGLEVEL_LOG,"J Identifier Not Support Yet\r\n");	
	pPlayerData->ArchCodeNum=__GetDecNumber(pString+1,' ',&StrLen);
	pPlayerData->PinoutCode=__GetDecNumber(pString+1+StrLen+1,'*',&StrLen);
	return 0;
}
////得到一组Fuse Data之后就根据地址将数据处理掉
static int __L_ActionFuseList(tJedPlayer*pPlayer)
{
	int nIsRowBegain = 1;
	int Ret=0;
	int RealFuseBit;
	tJedPlayerData *pPlayerData=GetJedPlayerData(pPlayer);
	tFuseData *pFuseList=&pPlayerData->FuseList;
	tFuseDataMap *pFuseDataMap=&pPlayerData->FuseDataMap;
	tIReporter *pReporter=pPlayer->pReporter;
	pFuseDataMap->nFirstFuseFalg++;

	if ( pPlayerData->FuseDataMap.nDefaultFalg == -1){
		if( pFuseDataMap->nFirstFuseFalg == 0){
			if (pFuseList->FuseStartNum != 0){
				pReporter->PrintLog(pReporter,LOG_LEVEL_ERR,"should be start with 0x00\r\n");		
				return -1;
			}
		}else{ //不能出现不连续
			if(pFuseDataMap->NextFuseListIdx != pFuseList->FuseStartNum){
				pReporter->PrintLog(pReporter,LOG_LEVEL_ERR,"the address is not correct\r\n");		
				return -1;
			}
		}
	}
	if(pFuseList->pFuseData){
		///处理数据操作,后面再实现
		int i,BytesUsed=(pFuseList->DataBitLen+7)/8;///向上取8的整数倍
		///
		RealFuseBit=pFuseList->DataBitLen-pFuseList->FuseStartNum%8;///FuseStartNum不是8的整数倍的时候，DataBitLen会空出余数位不进行填充
		pFuseDataMap->nTotalRealBitSize +=RealFuseBit;
		//////////////////
		if (( (pFuseList->FuseStartNum + RealFuseBit) >  pPlayerData->FuseNum)){
			pReporter->PrintLog(pReporter,LOG_LEVEL_ERR,"the size of fuse is more than expect\r\n");		
			return -1;
		}
		/////////////////
		for(i=0;i<BytesUsed;++i){
			pPlayerData->ChksumFuseListCalc +=pFuseList->pFuseData[i];
			///有可能存在两个FuseList给出的区域刚好都不是8的整数倍数据对齐，被分在两条记录中，需要使用或的方式，避免Bit值为1的数据丢失
			if (pFuseDataMap->NextFuseListIdx > 0 && pFuseDataMap->nPreIsNoRemain > 0){
				int j = 0;
				BYTE cTemp = 0x00;
				/*for ( j = 0; j < pFuseDataMap->nPreIsNoRemain; j++){
					cTemp |= ((BYTE)(pow(2,8-j-1)));
				}*/	
				cTemp =(BYTE)((0xFF<<pFuseDataMap->nPreIsNoRemain)&0xFF);//下个拼凑字节的部分位先与下操作再进行位或
				if (pFuseDataMap->NextFuseListIdx == pFuseList->FuseStartNum){				
					pFuseDataMap->pData[(pFuseList->FuseStartNum>>3)+i] |= (pFuseList->pFuseData[i]&cTemp);
				}else{
					int nPreByteCnt = (pFuseDataMap->NextFuseListIdx+7)/8;
					if( pPlayerData->FuseDataMap.nDefaultFalg == 1 ){
						BYTE LackValue = 0x00;
						int nIdx = 0;
						if ( nPreByteCnt -1 == ((pFuseList->FuseStartNum>>3)+i) ){ //上个和下个的拆分是共占同个字节中的
							
							for (nIdx= 0; nIdx <  pFuseList->FuseStartNum - pFuseDataMap->NextFuseListIdx; nIdx++){
								LackValue|= ((BYTE)(pow(2,nIdx+ (pFuseDataMap->NextFuseListIdx%8))));
							}
							pFuseDataMap->pData[(pFuseList->FuseStartNum>>3)+i] |= LackValue; //缺少的部分
							pFuseDataMap->pData[(pFuseList->FuseStartNum>>3)+i] |= pFuseList->pFuseData[i]; //下半个高字节的部分
						}else{
							int nPreOverBit = 0;
							int nCurUseBit = ((pFuseList->FuseStartNum>>3)+1)*8 - pFuseList->FuseStartNum;
							cTemp =(BYTE)((0xFF<< (8- nCurUseBit))&0xFF);
							pFuseDataMap->pData[(pFuseList->FuseStartNum>>3)+i] =(pFuseList->pFuseData[i]&cTemp); 

							//根据F来重新设置拼凑字节部分的上个字节的位值
							cTemp = (BYTE)((0xFF>> ( nCurUseBit))&0xFF);
							pFuseDataMap->pData[(pFuseList->FuseStartNum>>3)+i] |= (cTemp);

							//恢复设置上次缺省字节的值
							nPreOverBit = pFuseDataMap->NextFuseListIdx%8;
							cTemp = (BYTE)((0xFF<< (nPreOverBit) )&0xFF);
							pFuseDataMap->pData[pFuseDataMap->NextFuseListIdx>>3] |= cTemp ;
						}
						///////
					}else{
						int nCurUseBit = ((pFuseList->FuseStartNum>>3)+1)*8 - pFuseList->FuseStartNum;
						cTemp =(BYTE)((0xFF<< (8- nCurUseBit))&0xFF);
						
						if( nPreByteCnt -1 == ((pFuseList->FuseStartNum>>3)+i) ){ //上个和下个的拆分是共占同个字节中的
							pFuseDataMap->pData[(pFuseList->FuseStartNum>>3)+i] |=(pFuseList->pFuseData[i]&cTemp);
						}else{	
							pFuseDataMap->pData[(pFuseList->FuseStartNum>>3)+i] =(pFuseList->pFuseData[i]&cTemp); 
						}
					}
					//////
				}
				pFuseDataMap->nPreIsNoRemain = 0; 
				nIsRowBegain = 0;
			}else if ( pFuseDataMap->NextFuseListIdx > 0 && pFuseDataMap->nPreIsNoRemain == 0 && pFuseDataMap->NextFuseListIdx != pFuseList->FuseStartNum
				&& pFuseDataMap->NextFuseListIdx != 0xffffffff &&  nIsRowBegain == 1 && (pFuseDataMap->nFirstFuseFalg > 0)){
					
				int nPreOverBit;
				nPreOverBit = (pFuseList->FuseStartNum -  pFuseDataMap->NextFuseListIdx)%8;
				if ( pPlayerData->FuseDataMap.nDefaultFalg == 1 ){
					BYTE cTemp;
					cTemp = (BYTE)((0xFF>> (8 -nPreOverBit) )&0xFF);
					pFuseDataMap->pData[(pFuseList->FuseStartNum>>3)+i] = (pFuseList->pFuseData[i]|cTemp) ;
				}else{
					pFuseDataMap->pData[(pFuseList->FuseStartNum>>3)+i] = pFuseList->pFuseData[i];
				}
				nIsRowBegain= 0;
			}
			else{
				pFuseDataMap->pData[(pFuseList->FuseStartNum>>3)+i] = pFuseList->pFuseData[i];
			}

			if ( (pFuseList->DataBitLen%8 != 0) &&  (i  == BytesUsed -1) ) { 
				int j = 0;
				BYTE cTemp = 0x00;
				/*for ( j = 0; j < BytesUsed*8 - pFuseList->DataBitLen; j++){
					cTemp |= ((BYTE)(pow(2,j)));
				}*/
				cTemp = 0xFF>>( (BytesUsed*8 - pFuseList->DataBitLen));
				pFuseDataMap->pData[(pFuseList->FuseStartNum>>3)+i] &= cTemp;
			}
			////////////////////////方法二//////////////////////////
			//if (pFuseDataMap->NextFuseListIdx > 0 && pFuseDataMap->nPreIsNoRemain > 0 ){
			//	//默认值为1的就要恢复
			//	if (pPlayerData->FuseDataMap.nDefaultFalg == 1){ 
			//		BYTE ReBack = 0x00;
			//		int j = 0;
			//		for (j = 0; j < pFuseDataMap->nPreIsNoRemain; j++){
			//			ReBack |= ((BYTE)(pow(2, j)));
			//		}
			//		pFuseDataMap->pData[(pFuseList->FuseStartNum>>3)+i]  &= ReBack;
			//	}

			//	pFuseDataMap->pData[(pFuseList->FuseStartNum>>3)+i] |=pFuseList->pFuseData[i];
			//	pFuseDataMap->nPreIsNoRemain = 0; //开始的时候才需要进行位或
			//}else{
			//	pFuseDataMap->pData[(pFuseList->FuseStartNum>>3)+i] =pFuseList->pFuseData[i];
			//}
			//
			//if (pPlayerData->FuseDataMap.nDefaultFalg == 1){ //如果是全FF则认为是设置的填充默认值/*pFuseDataMap->pData[(pFuseList->FuseStartNum>>3)+i] == 0xff*/
			//	//
			//	if ( (pFuseList->DataBitLen%8 != 0) &&  (i  == BytesUsed -1) ) { //不足整数个字节的填充为设置的默认值
			//		int j = 0;
			//		BYTE cTemp = 0x00;
			//		for ( j = 0; j < BytesUsed*8 - pFuseList->DataBitLen; j++){
			//			cTemp |= ((BYTE)(pow(2,8-j-1)));
			//		}
			//		pFuseDataMap->pData[(pFuseList->FuseStartNum>>3)+i] |= cTemp;
			//	}
			//}
		}
		if(pFuseDataMap->NextFuseListIdx==pFuseList->FuseStartNum){////当前的起始Index刚好与上一次连接上
			if(pFuseDataMap->pCurBitDataMap){
				///更新LenBit即可，不能更新StartBit
				pFuseDataMap->pCurBitDataMap->LenBit +=RealFuseBit; ///每次累加实际的FuseBit数
			}
			else{
				pReporter->PrintLog(pReporter,LOG_LEVEL_ERR,"FuseListBitMap is NULL, Please check it\r\n");		
				Ret=-1;
			}
		}
		else{///需要有一个新的Map
			tBitDataMap *pBitDataMap=__FuseDataMap_AllocBitMap();
			if(pBitDataMap){
				pBitDataMap->StartBit=pFuseList->FuseStartNum;
				pBitDataMap->LenBit=RealFuseBit;
				__FuseDataMap_AppendBitDataMap(pPlayer,pBitDataMap);
				pFuseDataMap->pCurBitDataMap=pBitDataMap;
			}
			else{
				Ret=-1;
			}
		}
		///记录下次连续的FuseList的Index
		pFuseDataMap->NextFuseListIdx=pFuseList->FuseStartNum+RealFuseBit;
		pFuseDataMap->nPreIsNoRemain = pFuseDataMap->NextFuseListIdx%8;
		SysSafeFree(pFuseList->pFuseData);
	}
	pFuseList->DataBitLen=0;
	pFuseList->DataBitMaxLen=0;
	pFuseList->FuseStartNum=0;
	return Ret;
}


static int __L_HandlerFuseList(tJedPlayer*pPlayer,char*pString,int Size)
{
	int Ret=0;
	char *pBit=NULL;
	unsigned char *pData=NULL;
	tIReporter *pReporter=pPlayer->pReporter;
	tJedPlayerData *pPlayerData=GetJedPlayerData(pPlayer);
	tFuseData *pFuseList=&pPlayerData->FuseList;
	if(pPlayerData->bStringIDIn){
		int StrNumLen=0;
		pFuseList->FuseStartNum=__GetDecNumber(pString+1,' ',&StrNumLen);
		///有些Fuse的StartNum不一定是从8的整数倍开始，需要让后面填入数据的时候进行8bit对齐;
		///所以一开始让其占有pFuseList->FuseStartNum%8个bit
		///但实际的使用长度应该是pFuseList->DataBitLen-pFuseList->FuseStartNum%8;
		pFuseList->DataBitLen=pFuseList->FuseStartNum%8;
		pFuseList->DataBitMaxLen=0;
		pBit=pString+1+StrNumLen;
	}
	else{
		pBit=pString;

	}
	while(*pBit!=0){
		if(*pBit>='0' && *pBit<='1'){	
			if(pFuseList->DataBitLen>=pFuseList->DataBitMaxLen){	///需要重新分配空间了
				char strMsg[128]={0};
				unsigned int PreLen=pFuseList->DataBitMaxLen;
				pFuseList->DataBitMaxLen +=FUSELIST_CACHEDATA*8;
				pData=(unsigned char*)SysAlloc(pFuseList->DataBitMaxLen/8);
				if(pData==NULL){
					char strMsg[128];
					memset(strMsg,0,128);
					sprintf(strMsg,"Malloc Size:0x%X Failed\r\n",pFuseList->DataBitMaxLen/8);
					pReporter->PrintLog(pReporter,LOG_LEVEL_ERR,strMsg);	
					Ret=-1; goto __end;
				}
				memset(pData,0,pFuseList->DataBitMaxLen/8);
				if(pFuseList->pFuseData!=NULL){
					memcpy(pData,pFuseList->pFuseData,PreLen/8);///将原来的数据先拷贝到新的内存中
					SysSafeFree(pFuseList->pFuseData);
				}
				pFuseList->pFuseData=pData;
			}

			pFuseList->pFuseData[pFuseList->DataBitLen/8] |= (*pBit-'0')<<(pFuseList->DataBitLen%8);
			pFuseList->DataBitLen++;	
		}
		pBit++;
	}

__end:
	return Ret;
}

static int __N_HandlerNote(tJedPlayer*pPlayer,char*pString,int Size)
{
	int Ret=0;
	tIReporter *pReporter=pPlayer->pReporter;
	tJedPlayerData *pPlayerData=GetJedPlayerData(pPlayer);
	if(pPlayerData->NotePrintEn)
		pReporter->PrintLog(pReporter,LOG_LEVEL_LOG,pString);		
	return Ret;
}

static int __P_HandlerPinSequence(tJedPlayer*pPlayer,char*pString,int Size)
{
	int Ret=0;
	tIReporter *pReporter=pPlayer->pReporter;
	pReporter->PrintLog(pReporter,LOG_LEVEL_LOG,"P Identifier Not Support Yet\r\n");		
	return -1;
}



static int __Q_HandlerValue(tJedPlayer*pPlayer,char*pString,int Size)
{
	int Ret=0;
	tJedPlayerData *pPlayerData=GetJedPlayerData(pPlayer);
	unsigned char SubFiled=pString[1];
	pPlayerData->NotePrintEn=0; ///解析到Q识别器之后，后面的Note就不再打印出来
	switch(SubFiled){
		case 'F':///Number of Fuses
			pPlayerData->FuseNum=__GetDecNumber(pString+2,JED_ASTERISK,NULL);
			if(pPlayerData->FuseNum>0){
				////在这里先分配足够的空间用来存放后续的FuseList数据
				Ret=FuseDataMap_AllocData(pPlayer,pPlayerData->FuseNum);
			}
			break;
		case 'P'://Number of Pins
			pPlayerData->PinNum=__GetDecNumber(pString+2,JED_ASTERISK,NULL);
			break;
		case 'V':///Max Number of test Vectors
			pPlayerData->MaxVectorNum=__GetDecNumber(pString+2,JED_ASTERISK,NULL);
			break;
		default:
			Ret=-1;
			break;
	}
	return Ret;
}

static int __R_HandlerResultingVector(tJedPlayer*pPlayer,char*pString,int Size)
{
	int Ret=0;
	tIReporter *pReporter=pPlayer->pReporter;
	pReporter->PrintLog(pReporter,LOG_LEVEL_LOG,"R Identifier Not Support Yet\r\n");		
	return -1;
}

static int __S_HandlerStartingVector(tJedPlayer*pPlayer,char*pString,int Size)
{
	int Ret=0;
	tIReporter *pReporter=pPlayer->pReporter;
	pReporter->PrintLog(pReporter,LOG_LEVEL_LOG,"S Identifier Not Support Yet\r\n");		
	return -1;
}

static int __T_HandlerTestCycles(tJedPlayer*pPlayer,char*pString,int Size)
{
	int Ret=0;
	tIReporter *pReporter=pPlayer->pReporter;
	pReporter->PrintLog(pReporter,LOG_LEVEL_LOG,"T Identifier Not Support Yet\r\n");		
	return -1;
}

////得到一组User Data之后就根据地址将数据处理掉
static int __U_ActionUserData(tJedPlayer*pPlayer)
{
	int Ret=0;
	tJedPlayerData *pPlayerData=GetJedPlayerData(pPlayer);
	tFuseData *pUserData=&pPlayerData->UserData;
	tFuseData *pUserCode=&pPlayerData->UserCode;
	
	if(pUserData->pFuseData){
		///处理数据操作,后面再实现
		//int i,BytesUsed=(pUserData->DataBitLen+7)/8;///取8的整数倍
		int UserCodeBitBytes=(pUserData->DataBitLen+7)/8;
		pUserCode->pFuseData=SysAlloc(UserCodeBitBytes);
		if(!pUserCode->pFuseData){
			pPlayer->pReporter->PrintLog(pPlayer->pReporter,LOG_LEVEL_ERR,"Save User Code Failed");
			Ret=-1;
		}
		else{
			memset(pUserCode->pFuseData,0,UserCodeBitBytes);
			memcpy(pUserCode->pFuseData,pUserData->pFuseData,UserCodeBitBytes);
			pUserCode->DataBitLen=pUserData->DataBitLen;
			pUserCode->DataBitMaxLen=UserCodeBitBytes*8;
			pUserCode->FuseStartNum=0;
		}
		SysSafeFree(pUserData->pFuseData);
	}
	pUserData->DataBitLen=0;
	pUserData->DataBitMaxLen=0;
	pUserData->FuseStartNum=0;
	return Ret;
}

static int __GetValidCharLength(char*pStart,char EndCh,int *pSize)
{
	char *pTmp=pStart;
	*pSize=0;
	while(*pTmp!=EndCh || *pTmp!=0){
		*pSize++;
	}
	return 0;
}

static int __U_HandlerUserData(tJedPlayer*pPlayer,char*pString,int Size)
{
	////User Data 是从MSB->LSB
	int Ret=0,CharSize=0,IsOdd=1;
	char *pBit=NULL,*pHex=NULL,*pStr=NULL;
	unsigned char *pData=NULL;
	tJedPlayerData *pPlayerData=GetJedPlayerData(pPlayer);
	unsigned char SubFiled=pString[1];
	tFuseData *pUserData=&pPlayerData->UserData;
	pPlayer->UserCodeWriteMode=1; ///全部使用高bit优先输出的方式
	if(SubFiled=='H'){///16进制模式
		unsigned char TmpChar=0;
		pHex=pString+2;
		pPlayer->UserCodeWriteMode=1; //按照高bit先写入的方式进行
		while(*pHex){
			if(__IsHexChar(*pHex)){	
				if(pUserData->DataBitLen>=pUserData->DataBitMaxLen){	///需要重新分配空间了
					unsigned int PreLen=pUserData->DataBitMaxLen;
					pUserData->DataBitMaxLen +=FUSELIST_CACHEDATA*8;
					pData=(unsigned char*)SysAlloc(pUserData->DataBitMaxLen/8);
					if(!pData){

						Ret=-1; goto __end;
					}
					memset(pData,0,pUserData->DataBitMaxLen/8);
					if(pUserData->pFuseData!=NULL){
						memcpy(pData,pUserData->pFuseData,PreLen/8);///将原来的数据先拷贝到新的内存中
					}
					pUserData->pFuseData=pData;
				}
				if(*pHex>='0'&&*pHex<='9'){
					TmpChar=*pHex-'0';
				}
				else if(*pHex>='a'&&*pHex<='f'){
					TmpChar=*pHex-'a'+10;
				}
				else{
					TmpChar=*pHex-'A'+10;
				}
				if(IsOdd){
					TmpChar =TmpChar<<4;
				}
				IsOdd =!IsOdd;
				//pUserData->pFuseData[pUserData->DataBitLen/8] =(pUserData->pFuseData[pUserData->DataBitLen/8]<<4)|TmpChar;
				
				pUserData->pFuseData[pUserData->DataBitLen/8] |=TmpChar;
				pUserData->DataBitLen+=4;///每获得一个字符，相当于递增4个bit	
			}
			pHex++;
		}
	}
	else if(SubFiled=='A'){//ASCII模式
		unsigned char TmpChar=0;
		pStr=pString+2;
		pPlayer->UserCodeWriteMode=1; //按照高bit先写入的方式进行
		while(*pStr!='*'){
			if(pUserData->DataBitLen>=pUserData->DataBitMaxLen){	///需要重新分配空间了
				unsigned int PreLen=pUserData->DataBitMaxLen;
				pUserData->DataBitMaxLen +=FUSELIST_CACHEDATA*8;
				pData=(unsigned char*)SysAlloc(pUserData->DataBitMaxLen/8);
				if(!pData){
					Ret=-1; goto __end;
				}
				memset(pData,0,pUserData->DataBitMaxLen/8);
				if(pUserData->pFuseData!=NULL){
					memcpy(pData,pUserData->pFuseData,PreLen/8);///将原来的数据先拷贝到新的内存中
				}
				pUserData->pFuseData=pData;
			}
			pUserData->pFuseData[pUserData->DataBitLen/8] =*pStr;
			pUserData->DataBitLen+=8;///每获得一个字符，相当于递增8个bit	
			pStr++;
		}
	}
	else{///Binary 模式
		pBit=pString+1;
		while(*pBit){
			if(*pBit>='0' && *pBit<='1'){	
				if(pUserData->DataBitLen>=pUserData->DataBitMaxLen){	///需要重新分配空间了
					unsigned int PreLen=pUserData->DataBitMaxLen;
					pUserData->DataBitMaxLen +=FUSELIST_CACHEDATA*8;
					pData=(unsigned char*)SysAlloc(pUserData->DataBitMaxLen/8);
					if(!pData){
						Ret=-1; goto __end;
					}
					memset(pData,0,pUserData->DataBitMaxLen/8);
					if(pUserData->pFuseData!=NULL){
						memcpy(pData,pUserData->pFuseData,PreLen/8);///将原来的数据先拷贝到新的内存中
					}
					pUserData->pFuseData=pData;
				}
				pUserData->pFuseData[pUserData->DataBitLen/8] |= (*pBit-'0')<<((7-pUserData->DataBitLen)%8);///MSB First
				pUserData->DataBitLen++;	
			}
			pBit++;
		}
	}
__end:
	return Ret;
}

static int __V_HandlerTestVector(tJedPlayer*pPlayer,char*pString,int Size)
{
	int Ret=0;
	tIReporter *pReporter=pPlayer->pReporter;
	//pReporter->PrintLog(pReporter,LOGLEVEL_LOG,"V Identifier Not Support Yet");		
	return 0;
}

static int __X_HandlerTestCondition(tJedPlayer*pPlayer,char*pString,int Size)
{
	int Ret=0;
	tIReporter *pReporter=pPlayer->pReporter;
	//pReporter->PrintLog(pReporter,LOGLEVEL_LOG,"X Identifier Not Support Yet\r\n");		
	return 0;
}


static tJedID JedIDArray[]={
	{"A",__A_HandlerAccessTime,NULL},
	{"B",NULL,NULL},
	{"C",__C_HandlerChecksum,__C_ActionChecksum},
	{"D",__D_HandlerDeviceType,NULL},
	{"E",__E_HandlerElecticalFuseData,__E_ActionElecticalFuseData},
	{"F",__F_HandlerDefaultFuseState,NULL},
	{"G",__G_HandlerSecurityFuse,NULL},
	{"H",NULL,NULL},
	{"I",NULL,NULL},
	{"J",__J_HandlerDeviceIdentification,NULL},
	{"K",__L_HandlerFuseList,__L_ActionFuseList},
	{"L",__L_HandlerFuseList,__L_ActionFuseList},
	{"M",NULL},
	{"N",__N_HandlerNote,NULL},
	{"O",NULL},
	{"P",__P_HandlerPinSequence,NULL},
	{"Q",__Q_HandlerValue,NULL},
	{"R",__R_HandlerResultingVector,NULL},
	{"S",__S_HandlerStartingVector,NULL},
	{"T",__T_HandlerTestCycles,NULL},
	{"U",__U_HandlerUserData,__U_ActionUserData},
	{"V",__V_HandlerTestVector,NULL},
	{"W",NULL},
	{"X",__X_HandlerTestCondition,NULL},
	{"Y",NULL},
	{"Z",NULL},
};

static int __A_HandlerJED_STX(tJedPlayer*pPlayer,char*pString,int Size)
{
	///JED_STX命令不做任何处理
	return 0;
}

static int __U_ActionJED_STX(tJedPlayer*pPlayer)
{
	return 0;
}

tJedID JedIDJED_STX={
	"",NULL,NULL
};

tJedID* GetJedID(unsigned char ID)
{
	int Idx;
	if(ID==JED_STX || ID == JED_ASTERISK){
		return &JedIDJED_STX;
	}
	Idx=ID-'A';
	if(Idx>sizeof(JedIDArray)/sizeof(tJedID)){
		return NULL;
	}
	else{
		if(ID==JedIDArray[Idx].ID[0]){
			return &JedIDArray[Idx];
		}
		else{
			return NULL;
		}
	}
}