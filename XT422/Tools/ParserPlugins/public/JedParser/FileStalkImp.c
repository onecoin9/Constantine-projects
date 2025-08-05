#include "IFileStalk.h"
#include "FileStalkImp.h"

static int __Handle_Seek(tFileHandle *pHandle,unsigned int Offset,eFileFrom From)
{
	int Ret=0;
	tFileData* pFileData=GetFileData(pHandle);
	if(pFileData->hFile){
		SetFilePointer((HANDLE)pFileData->hFile,Offset,0,From);
	}
	else{
		Ret=-1;
	}
	return Ret;
}

static int __Handle_Ftell(tFileHandle *pHandle)
{
	int Ret=0;
	tFileData* pFileData=GetFileData(pHandle);
	if(pFileData->hFile){	
		DWORD dwPtr=SetFilePointer((HANDLE)pFileData->hFile,0,0,IFILE_CUR);
		if(dwPtr==INVALID_SET_FILE_POINTER){
			Ret=-1;
		}
		else{
			Ret=dwPtr;
		}
	}
	else{
		Ret=-1;
	}
	return Ret;
}

static int __Handle_Read(tFileHandle *pHandle,unsigned char*pData,int Size)
{
	int Ret=0;
	int BytesRead;
	tFileData* pFileData=GetFileData(pHandle);
	if(pFileData->hFile){
		if(ReadFile((HANDLE)pFileData->hFile,pData,Size,&BytesRead,NULL)==0){
			Ret=-1;
		}
		else{
			Ret=BytesRead;
		}
	}
	else{
		Ret=-1;
	}
	return Ret;
}

static int __Handle_Close(tFileHandle *pHandle)
{
	tFileData* pFileData=GetFileData(pHandle);
	return 0; 
}

static int __AttachHandle(tFileHandle*pHandle,HANDLE hFile)
{
	int Ret=0;
	tFileData* pFileData=GetFileData(pHandle);	
	pFileData->FileType=FILETYPE_HANDLE;
	pFileData->hFile=(void*)hFile;
	pHandle->Opts.Close=__Handle_Close;
	pHandle->Opts.Read=__Handle_Read;
	pHandle->Opts.Seek=__Handle_Seek;
	pHandle->Opts.Ftell=__Handle_Ftell;
	///需要将文件的指针移动到文件开始位置
	pHandle->Opts.Seek(pHandle,0,IFILE_BEGIN);
	return Ret;
}


static int __Seek(tFileHandle *pHandle,unsigned int Offset,eFileFrom From)
{
	int Ret=0;
	tFileData* pFileData=GetFileData(pHandle);
	if(pFileData->hFile){
		Ret=fseek(pFileData->hFile,Offset,From);
	}
	else{
		Ret=-1;
	}
	return Ret;
}


static int __Ftell(tFileHandle *pHandle)
{
	int Ret=0;
	tFileData* pFileData=GetFileData(pHandle);
	if(pFileData->hFile){
		Ret=ftell(pFileData->hFile);
	}
	else{
		Ret=-1;
	}
	return Ret;
}


static int __Close(tFileHandle *pHandle)
{
	tFileData* pFileData=GetFileData(pHandle);
	if(pFileData->hFile){
		fclose(pFileData->hFile);
	}
	return 0; 
}

static int __Read(tFileHandle *pHandle,unsigned char*pData,int Size)
{
	int Ret=0;
	tFileData* pFileData=GetFileData(pHandle);
	if(pFileData->hFile){
		Ret=(int)fread(pData,1,Size,pFileData->hFile);
	}
	else{
		Ret=-1;
	}
	return Ret;
}

static int __Open(tFileHandle *pHandle,char*FileName)
{
	int Ret=0;
	tFileData* pFileData=GetFileData(pHandle);
	pFileData->hFile=(FILE*)fopen(FileName,"rb");
	if(pFileData->hFile==NULL){
		sprintf(pFileData->ErrMsg,"Open File Failed\r\n");
		Ret=-1;
	}
	else{
		pHandle->Opts.Close=__Close;
		pHandle->Opts.Seek=__Seek;
		pHandle->Opts.Read=__Read;
		pHandle->Opts.Ftell=__Ftell;
	}
	return Ret;
}

static int __ReadString(tFileHandle*pHandle,unsigned char**ppData)
{
	int Ret=0,i,Len=0,bFind=0;
	int Size=1024,BytesRead;
	int CurPos=pHandle->Opts.Ftell(pHandle);
	tFileData *pFileData=GetFileData(pHandle);
	unsigned char* pTmpData=NULL;
	unsigned char* pData=NULL;
	pTmpData=SysAlloc(Size);
	if(!pTmpData){
		Ret=-1;goto __end;
	}
	while(1){
		memset(pTmpData,0,Size);
		BytesRead=pHandle->Opts.Read(pHandle,pTmpData,Size);
		if(BytesRead>0){
			for(i=0;i<BytesRead;++i){
				Len++;
				if(pTmpData[i]==0x0A){
					bFind=1;
					break;
				}
			}
		}
		else{
			break;
		}
		if(bFind){
			break;
		}
	}

	if(bFind==0){///没有找到字符串
		Ret=0;
	}
	else{///找到了字符串
		pHandle->Opts.Seek(pHandle,CurPos,IFILE_BEGIN);
		if(Len+1<=BUFSTRING_MAX){
			pData=pFileData->LineString;
		}
		else{
			pData=SysAlloc(Len+1);
			if(!pData){
				Ret=-1; goto __end;
			}
		}
		memset(pData,0,Len+1);
		BytesRead=pHandle->Opts.Read(pHandle,pData,Len);
		if(BytesRead!=Len){
			Ret=-1; goto __end;
		}
		else{
			Ret=Len+1;///返回实际Buffer的长度
		}
	}
	
__end:
	SysSafeFree(pTmpData);
	if(ppData){
		if(Ret<=0){
			*ppData=NULL;
		}
		else{
			*ppData=pData;
		}
	}
	return Ret;
}

static void __FreeString(tFileHandle *pHandle,unsigned char**ppData)
{
	if(ppData){
		tFileData *pFileData=GetFileData(pHandle);
		if(*ppData!=pFileData->LineString){///不是固定String缓存位置才需要释放
			SysSafeFree(*ppData);
		}
	}
}



static int __GetErrMsg(tFileHandle *pHandle,char* pErrMsg)
{
	int Ret=0;
	tFileData* pFileData=GetFileData(pHandle);
	sprintf(pErrMsg,"%s",pFileData->ErrMsg);
	return Ret;
}

tFileHandle* GetFileHandle()
{
	int Size=sizeof(tFileHandle)+sizeof(tFileData);
	tFileHandle *pHandle=(tFileHandle *)SysAlloc(Size);
	if(pHandle){
		memset((char*)pHandle,0,Size);
		pHandle->pFileData = (void*)((char*)pHandle+sizeof(tFileHandle));
		///初始化的时候只初始化必要的函数指针，根据外部调用环境在初始化其他函数指针
		pHandle->Opts.Open=__Open;
		pHandle->Opts.AttachHandle=__AttachHandle;
		pHandle->Opts.ReadString=__ReadString;
		pHandle->Opts.FreeString=__FreeString;
		pHandle->Opts.GetErrMsg=__GetErrMsg;
	}
	return pHandle;
}

void PutFileHandle(tFileHandle*pHandle)
{
	if(pHandle){
		SysSafeFree(pHandle);
	}
}
