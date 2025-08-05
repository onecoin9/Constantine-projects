//#include "JedPlayerImp.h"
//#include <Windows.h>
//
//#include <stdio.h>
//#include <string.h> 
//
//int __GetChecksum(tJedPlayer*pPlayer)
//{
//	int Ret=0,i,j=0,Phase=0;
//	int BytesRead;
//	unsigned char TmpByte;
//	tJedPlayerData *pPlayerData=GetJedPlayerData(pPlayer);
//	unsigned char *pData=NULL;
//	int DataSize=1024*1024;
//	unsigned short ChkSumCalc=0,ChkSumFile=0;
//	tFileHandle* pFileHandle=pPlayer->pFileHandle;
//	pData=(unsigned char*)SysAlloc(DataSize);
//	if(pData==NULL){
//		SetErrMsg(pPlayerData,-1,"[Errmsg] Data Malloc failed");
//		Ret=-1; goto __end;
//	}
//
//	while(1){
//		BytesRead=pFileHandle->Opts.Read(pFileHandle,pData,DataSize);
//		if(BytesRead<0){
//			Ret=-1; goto __end;
//		}
//		for(i=0;i<BytesRead;++i){
//			TmpByte=pData[i];
//			if(TmpByte==JED_STX){
//				Phase=1;
//			}
//			else if(TmpByte==JED_ETX){
//				Phase=2;
//				ChkSumCalc +=TmpByte;
//				continue;
//			}
//			if(Phase==1){
//				ChkSumCalc +=TmpByte;
//			}
//			else if(Phase==2){
//				if(j<4){
//					if(TmpByte>='0' && TmpByte<='9')
//						ChkSumFile = (ChkSumFile<<4)|(TmpByte-'0');
//					else if(pData[i]>='A' && pData[i]<='F')
//						ChkSumFile = (ChkSumFile<<4)|(TmpByte-'A'+10);
//					else if(pData[i]>='a' && pData[i]<='f')
//						ChkSumFile = (ChkSumFile<<4)|(TmpByte-'a'+10);
//					else{
//						SetErrMsg(pPlayerData,-1,"[Errmsg] Checksum include invalid char");
//						Ret=-1; goto __end;
//					}
//					j++;
//				}
//				else{
//					Phase=3;///得到文件中的校验值，后面数据都不需要了
//					break;
//				}
//			}
//		}
//		if(Phase==3){
//			break;
//		}
//		if(BytesRead<DataSize){///全部读取完成，需要退出了
//			break;
//		}
//	}
//__end:
//	SysSafeFree(pData);
//	if(Ret==0){
//		pPlayerData->ChksumCalc=ChkSumCalc;
//		pPlayerData->ChkSumFile=ChkSumFile;
//		//pPlayerData->ChkSumFile=ChkSumCalc;//这个地方可能会存在不匹配的情况，只要保证Fuse的是对的就行
//	}
//	return Ret;
//}
//
//static int __CompareFuseListChecksum(tJedPlayer *pPlayer)
//{
//	int Ret=1;
//	tJedPlayerData *pPlayerData=GetJedPlayerData(pPlayer);
//	if(pPlayerData->ChksumFuseListCalc!=pPlayerData->ChksumFuseListFile){
//		SetErrMsg(pPlayerData,-1,"[ErrMsg] FuseList Checksum Compare Error, CalcChk=0x%04X, FileChk=0x%04X",
//			pPlayerData->ChksumFuseListCalc,pPlayerData->ChksumFuseListFile);
//		Ret=0;
//	}
//	return Ret;
//}
//
//static int __CheckDiskFile(tJedPlayer *pPlayer,char*FileName)
//{
//	int Ret=0;
//	tJedPlayerData *pPlayerData=GetJedPlayerData(pPlayer);
//	tFileHandle* pFileHandle=pPlayer->pFileHandle;
//	Ret=pFileHandle->Opts.Open(pFileHandle,FileName);
//	if(Ret!=0){
//		SetErrMsg(pPlayerData,-1,"[Errmsg] Open file failed\r\n");
//		goto __end;
//	}
//	Ret=__GetChecksum(pPlayer);
//
//__end:	
//	pFileHandle->Opts.Close(pFileHandle);
//	if(Ret==0){
//		if(pPlayerData->ChksumCalc==pPlayerData->ChkSumFile){
//			Ret=1;
//		}
//		else{
//			char TmpErrMsg[128]={0};
//			sprintf(TmpErrMsg,"File Checksum Compare Failed, FileChk=0x%04X,CalcChk=0x%04X",
//				pPlayerData->ChkSumFile,pPlayerData->ChksumCalc);
//			pPlayer->pReporter->PrintLog(pPlayer->pReporter,LOG_LEVEL_WARNING,TmpErrMsg);
//			Ret=1; goto __end;
//		}
//	}
//	return Ret;
//}
//
//
//static int __CheckMemFile(tJedPlayer *pPlayer,unsigned char*pData,int Size)
//{
//	int Ret=0;
//	tJedPlayerData *pPlayerData=GetJedPlayerData(pPlayer);
//	tFileHandle* pFileHandle=pPlayer->pFileHandle;
//	Ret=pFileHandle->Opts.Attach(pFileHandle,pData,Size);
//	if(Ret!=0){
//		SetErrMsg(pPlayerData,-1,"[Errmsg] Open file failed");
//		goto __end;
//	}
//	Ret=__GetChecksum(pPlayer);
//
//__end:	
//	pFileHandle->Opts.Close(pFileHandle);
//	if(Ret==0){
//		if(pPlayerData->ChksumCalc==pPlayerData->ChkSumFile){
//			Ret=1;
//		}
//
//	}
//	return Ret;
//}
//
//static int __CheckHandleFile(tJedPlayer *pPlayer,HANDLE hFile)
//{
//	int Ret=0;
//	tJedPlayerData *pPlayerData=GetJedPlayerData(pPlayer);
//	tFileHandle* pFileHandle=pPlayer->pFileHandle;
//	Ret=pFileHandle->Opts.AttachHandle(pFileHandle,hFile);
//	if(Ret!=0){
//		SetErrMsg(pPlayerData,-1,"[Errmsg] Open file failed");
//		goto __end;
//	}
//	Ret=__GetChecksum(pPlayer);
//
//__end:	
//	pFileHandle->Opts.Close(pFileHandle);
//	if(Ret==0){
//		if(pPlayerData->ChksumCalc==pPlayerData->ChkSumFile){
//			Ret=1;
//		}
//		else{
//			char TmpErrMsg[128]={0};
//			sprintf(TmpErrMsg,"File Checksum Compare Failed, FileChk=0x%04X,CalcChk=0x%04X",
//				pPlayerData->ChkSumFile,pPlayerData->ChksumCalc);
//			pPlayer->pReporter->PrintLog(pPlayer->pReporter,LOG_LEVEL_WARNING,TmpErrMsg);
//			Ret=1; goto __end;
//		}
//	}
//	return Ret;
//}
//
//
//
//static int __CheckEndWithAsterisk(tJedPlayer*pPlayer,char*pString,int Size)
//{	
//	int i;
//	for(i=Size-1;i>=0;--i){
//		if(pString[i]==JED_ASTERISK){
//			return 1;
//		}
//	}
//	return 0;
//}
//
//static int __ParserFile(tJedPlayer*pPlayer)
//{
//	int LineNum=0;
//	int Ret=0,RtnApi=0,ByteStr;
//	int Phase=0;
//	tJedID *pJedID=NULL;
//	unsigned char ID=0;
//	unsigned char *pStrHead=NULL;
//	char TmpErrMsg[128]={0};
//	int StrLen=0;
//	IDParser pIDHandle=NULL;
//	unsigned char *pString=NULL;
//	tFileHandle *pFileHandle=pPlayer->pFileHandle;
//	tJedPlayerData *pPlayerData=GetJedPlayerData(pPlayer);
//	unsigned char CurID=0;///记录当前的ID，有可能一个ID域分在多行上，后面的行就不会有ID出现
//	pPlayerData->ChksumFuseListCalc=0;
//	pPlayerData->ChksumFuseListFile=0;
//	pPlayerData->NotePrintEn=1;
//	pPlayerData->FuseDataMap.nFirstFuseFalg = -1;
//
//	FuseDataMap_FreeData(pPlayer);///重新解析，需要删除空间
//	FeatureRowBit_Free(pPlayer);
//	UserCode_Free(pPlayer);
//	
//
//	while(1){
//		ByteStr=pFileHandle->Opts.ReadString(pFileHandle,&pString);
//		if(ByteStr<=0){
//			if(ByteStr==-1){///没有字符串可以读取了
//				Ret=-1; goto __end;
//			}
//			break;
//		}
//		LineNum++;
//
//__NeedRepeatCurrRow:
//		if(pString[0]==0x0D || pString[0]==0x0A || pString[0]==0x44){///是空行，不需要进入命令解析
//			pFileHandle->Opts.FreeString(pFileHandle,&pString);
//			continue;
//		}
//		if(CurID==0){///在没有找到*结束符之前不能再次获取命令
//			if(pString[0]==JED_STX){	///在JED_STX后面可能会跟QF，不能将QF抛弃不处理
//				if(ByteStr>=3 && pString[1]=='Q' && pString[2]=='F'){
//					CurID=pString[1];
//					pStrHead=pString+1;
//					StrLen=ByteStr-1;
//				}
//				else{
//					CurID=pString[0];
//					pStrHead=pString;
//					StrLen=ByteStr;
//				}
//				Phase=1;///找到STX，开始进入解析，还没找到之前其他都忽略
//			}
//			else if(pString[0]==JED_ETX){///找到ETX，后面有Checksum，之后就不用再解析了
//				break;
//			}
//			else{
//				CurID=pString[0];
//				pStrHead=pString;
//				StrLen=ByteStr;
//			}
//			pPlayerData->bStringIDIn=1;
//		}
//		else{
//			pPlayerData->bStringIDIn=0;
//			pStrHead=pString;
//			StrLen=ByteStr;
//		}
//		if(CurID=='*' || CurID==0x0D || CurID==0x0A){///只有一个空行
//			CurID=0;
//			pFileHandle->Opts.FreeString(pFileHandle,&pString);
//			continue;
//		}
//		if(Phase==1){
//			pJedID=GetJedID(CurID);
//			if(pJedID){
//				if(pJedID->pIDParser){
//					RtnApi=pJedID->pIDParser(pPlayer,pStrHead,StrLen);
//					if(RtnApi!=0){
//						SetErrMsg(pPlayerData,-1,"[Errmsg]Line=%d,Parser ID=0x%02X[%C] Error",LineNum,CurID,CurID);
//						sprintf(TmpErrMsg,"[Errmsg]Line=%d,Parser ID=0x%02X[%C] Error\r\n",LineNum,CurID,CurID);
//						pPlayer->pReporter->PrintLog(pPlayer->pReporter,LOG_LEVEL_ERR,TmpErrMsg);
//						Ret=-1; goto __end;
//					}
//				}
//				///解析完成之后，如果发现域已经结束，则需要进行Action操作
//				if(__CheckEndWithAsterisk(pPlayer,pStrHead,StrLen)){
//					if(pJedID->pIDAction){
//						RtnApi=pJedID->pIDAction(pPlayer);
//						if(RtnApi!=0){
//							SetErrMsg(pPlayerData,-1,"[Errmsg]Line=%d,Action ID=0x%02X[%C] Error",LineNum,CurID,CurID);
//							sprintf(TmpErrMsg,"[Errmsg]Line=%d,Action ID=0x%02X[%C] Error\r\n",LineNum,CurID,CurID);
//							pPlayer->pReporter->PrintLog(pPlayer->pReporter,LOG_LEVEL_ERR,TmpErrMsg);
//							Ret=-1; goto __end;
//						}
//					}
//					CurID=0;///已经结束了一个域信息，所以需要重新设置CurID为0，为下次解析接纳新的ID
//					/*if (pString[0] == JED_ASTERISK && strlen(pString) > 1){
//						int nLen = strlen(pString) -1;
//						memcpy(pString, pString+1,nLen );
//						pString[nLen] = 0;
//						goto __NeedRepeatCurrRow;
//					}*/
//					
//				}
//			}
//			else{
//				sprintf(TmpErrMsg,"[Errmsg]Line=%d,ID=0x%02X[%C] Not Support Yet\r\n",LineNum,CurID,CurID);
//				pPlayer->pReporter->PrintLog(pPlayer->pReporter,LOG_LEVEL_ERR,TmpErrMsg);
//				Ret=-1; goto __end;
//			}
//		}	
//		else if(Phase==0){
//			CurID=0;///还没有开始任何ID之前，需要全部忽略
//		}
//		///注意这个地方需要释放
//		pFileHandle->Opts.FreeString(pFileHandle,&pString);
//	}
//
//__end:
//	if(Ret == 0){ //成功解析
//		if ( pPlayerData->FuseDataMap.nDefaultFalg == -1){
//			//if (pPlayerData->FuseDataMap.nTotalRealBitSize !=pPlayerData->FuseNum){			
//			//	Ret=-1;
//			//	sprintf(TmpErrMsg,"there is no [F], and the size of fuse is no correct, real=%d, expect=%d \r\n", pPlayerData->FuseDataMap.nTotalRealBitSize, pPlayerData->FuseNum);
//			//	pPlayer->pReporter->PrintLog(pPlayer->pReporter,LOGLEVEL_ERR,TmpErrMsg);
//			//} 
//		}
//		
//	}
//
//	pFileHandle->Opts.FreeString(pFileHandle,&pString);
//
//
//	return Ret;
//}
//
//static int __ParserDiskFile(tJedPlayer*pPlayer,char*FileName)
//{
//	int Ret=0;
//	tJedPlayerData *pPlayerData=GetJedPlayerData(pPlayer);
//	tFileHandle* pFileHandle=pPlayer->pFileHandle;
//	Ret=pFileHandle->Opts.Open(pFileHandle,FileName);
//	if(Ret!=0){
//		SetErrMsg(pPlayerData,-1,"[Errmsg] Open file failed");
//		goto __end;
//	}
//	Ret=__ParserFile(pPlayer);
//
//__end:	
//	pFileHandle->Opts.Close(pFileHandle);
//	return Ret;
//}
//
//static int __ParserHandleFile(tJedPlayer*pPlayer,HANDLE hFile)
//{
//	int Ret=0;
//	tJedPlayerData *pPlayerData=GetJedPlayerData(pPlayer);
//	tFileHandle* pFileHandle=pPlayer->pFileHandle;
//	Ret=pFileHandle->Opts.AttachHandle(pFileHandle,hFile);
//	if(Ret!=0){
//		SetErrMsg(pPlayerData,-1,"[Errmsg] Open file failed");
//		goto __end;
//	}
//
//	Ret=__ParserFile(pPlayer);
//
//__end:	
//	pFileHandle->Opts.Close(pFileHandle);
//	return Ret;
//}
//
//
//static int __GetErrMsg(tJedPlayer *pPlayer,char* pErrMsg,int Size)
//{
//	int Ret=0;
//	tJedPlayerData* pPlayerData=GetJedPlayerData(pPlayer);
//	memset(pErrMsg,0,Size);
//	if((int)strlen(pPlayerData->ErrMsg)<=Size-1){
//		sprintf(pErrMsg,"%s\r\n",pPlayerData->ErrMsg);
//	}
//	else{
//		Ret=-1;
//	}
//	return Ret;
//}
//
//static int __GetRealSize(tJedPlayer *pPlayer,int* Size)
//{
//	int Ret=0;
//	tJedPlayerData* pPlayerData=GetJedPlayerData(pPlayer);
//	*Size = pPlayerData->FuseNum;
//
//	return Ret;
//}
//
//
//
//static tFuseDataMap* __GetFuseDataMap(tJedPlayer*pPlayer)
//{
//	tJedPlayerData* pPlayerData=GetJedPlayerData(pPlayer);
//	return &pPlayerData->FuseDataMap;
//}
//
//static tFuseData* __GetFeatureRowBit(tJedPlayer*pPlayer)
//{
//	tJedPlayerData* pPlayerData=GetJedPlayerData(pPlayer);
//	return &pPlayerData->FeatureRowBit;
//}
//
//static tFuseData* __GetUserCode(tJedPlayer*pPlayer)
//{
//	tJedPlayerData* pPlayerData=GetJedPlayerData(pPlayer);
//	return &pPlayerData->UserCode;
//}
//
//void JedPlayer_PutPlayer(tJedPlayer* pPlayer)
//{
//	if(pPlayer){
//		tJedPlayerData* pPlayerData=GetJedPlayerData(pPlayer);
//		SysSafeFree(pPlayerData->FuseList.pFuseData);
//		SysSafeFree(pPlayerData->EFuseData.pFuseData);
//		SysSafeFree(pPlayerData->UserData.pFuseData);
//		FuseDataMap_FreeData(pPlayer);
//		FeatureRowBit_Free(pPlayer);
//		UserCode_Free(pPlayer);
//	}
//	
//	SysSafeFree(pPlayer);
//}
//
//
//tJedPlayer* JedPlayer_GetPlayer(void *PrivData)
//{
//	int Size=sizeof(tJedPlayer)+sizeof(tJedPlayerData);
//	tJedPlayer *pPlayer=SysAlloc(Size);
//	if(pPlayer){
//		memset((char*)pPlayer,0,Size);
//		pPlayer->pPlayerData=(void*)((char*)pPlayer+sizeof(tJedPlayer));
//		pPlayer->PrivData=PrivData;
//		pPlayer->Opts.CheckDiskFile=__CheckDiskFile;
//		pPlayer->Opts.CheckMemFile=__CheckMemFile;
//		pPlayer->Opts.GetErrMsg=__GetErrMsg;
//		pPlayer->Opts.ParserDiskFile=__ParserDiskFile;
//		pPlayer->Opts.CheckHandleFile=__CheckHandleFile;
//		pPlayer->Opts.ParserHandleFile=__ParserHandleFile;
//		pPlayer->Opts.GetFuseDataMap=__GetFuseDataMap;
//		pPlayer->Opts.GetFeatureRowBit=__GetFeatureRowBit;
//		pPlayer->Opts.GetUserCode=__GetUserCode;
//		pPlayer->Opts.CompareFuseListChecksum=__CompareFuseListChecksum;
//		pPlayer->Opts.GetRealSize = __GetRealSize;
//	}
//	return pPlayer;
//}
//
//
//
//void JedPlayer_AttachFileHandle(tJedPlayer *pPlayer,tFileHandle *pFileHandle)
//{
//	if(pPlayer){
//		pPlayer->pFileHandle=pFileHandle;
//	}
//}
//
//
//void JedPlayer_AttachReporter(tJedPlayer *pPlayer,tIReporter *pReporter)
//{
//	if(pPlayer){
//		pPlayer->pReporter=pReporter;
//	}
//}