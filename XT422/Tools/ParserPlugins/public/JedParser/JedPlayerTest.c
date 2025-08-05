#include "JedPlayerAPI.h"
#include "JedPlayerImp.h"

int TestJedPlayer(char *FileName,tIReporter *pReporter)
{
	int Ret=0;
	char StrTmp[128]={0};
	tJedPlayer *pJedPlayer=NULL;
	tFileHandle *pFileHandle=NULL;
	pJedPlayer=JedPlayer_GetPlayer(NULL);
	if(pJedPlayer){
		pFileHandle=GetFileHandle();
		if(pFileHandle==NULL){
			Ret=-1; goto __end;
		}
		JedPlayer_AttachFileHandle(pJedPlayer,pFileHandle);
		JedPlayer_AttachReporter(pJedPlayer,pReporter);
		//Ret=pJedPlayer->Opts.CheckDiskFile(pJedPlayer,FileName);
		Ret=1;
		if(Ret==1){///校验正确
			Ret=pJedPlayer->Opts.ParserDiskFile(pJedPlayer,FileName);
			if(Ret!=0){
			}
		}
		else{		
		}
	}

__end:
	pReporter->PrintLog(pReporter,LOG_LEVEL_LOG,"\r\n================Test Completed=================\r\n");
	if(Ret==0){
		pReporter->PrintLog(pReporter,LOG_LEVEL_LOG,"\r\nPass\r\n");
	}
	else{
		pJedPlayer->Opts.GetErrMsg(pJedPlayer,StrTmp,128);
		pReporter->PrintLog(pReporter,LOG_LEVEL_ERR,StrTmp);
	}


	if(pFileHandle){
		PutFileHandle(pFileHandle);
	}
	if(pJedPlayer){
		JedPlayer_PutPlayer(pJedPlayer);
	}
	
	return Ret;
}