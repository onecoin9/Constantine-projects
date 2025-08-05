#ifndef _IFILESTALK_H_
#define _IFILESTALK_H_

#include <Windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum{
	IFILE_BEGIN=0,		///0从文件的起始位置
	IFILE_CUR=1,		///1从文件当前位置
	IFILE_END=2,		///2从文件末尾
}eFileFrom;

typedef struct tagFileHandle tFileHandle;

typedef struct tagFileOpts
{
/************************************************************************/
	/*@brief 打开档案文件                                                                  
	/*@param[in] pHandle : GetFileHandle返回的指针
	/*@param[in] FileName : 希望打开的文件路径
	/*@return
	/*  成功返回0 失败返回-1, 用GetErrMsg错误信息
	************************************************************************/
	int (*Open)(tFileHandle *pHandle,char*FileName);

	/********************************************************************/
	/*@brief 如果数据已经加载到内容中，可以将该内存块Attach到文件上，用于MemFile操作,代替Open函数
	/*在操作过程中，pData的数据不能被外部释放，否则会导致不可预料的后果
	/*@param[in] pHandle : GetFileHandle返回的指针
	/*@param[in] pData : 内存数据块
	/*@param[in] Size : 数据大小
	/*@return
	/*	0表示成功， -1表示失败
	*********************************************************************/
	int (*Attach)(tFileHandle *pHandle,unsigned char*pData,int Size);

	/********************************************************************/
	/*@brief 如果文件已经在外部被打开，传入的是Handle
	/*在操作过程中，pData的数据不能被外部释放，否则会导致不可预料的后果
	/*@param[in] pHandle : GetFileHandle返回的指针
	/*@param[in] pData : 内存数据块
	/*@param[in] Size : 数据大小
	/*@return
	/*	0表示成功， -1表示失败
	*********************************************************************/
	int (*AttachHandle)(tFileHandle*pHandle,HANDLE hFile);

	/************************************************************************/
	/*@brief 关闭档案文件                                                                 
	/*@param[in] pHandle : GetFileHandle返回的指针
	/*@return
	/*  成功返回0 失败返回-1, 用GetErrMsg错误信息
	************************************************************************/
	int (*Close)(tFileHandle *pHandle);

	/************************************************************************/
	/*@brief 从当前档案文件指针位置开始读取指定字节数的档案文件数据                                                            
	/*@param[in] pHandle : GetFileHandle返回的指针
	/*@param[in] pData : 读取数据存放的位置
	/*@param[in] Size : 希望读取的字节数
	/*@return
	/*  失败返回-1, >=0为实际读取到的字节数，返回-1时用GetErrMsg错误信息
	************************************************************************/
	int (*Read)(tFileHandle *pHandle,unsigned char*pData,int Size);
	
	/************************************************************************/
	/*@brief 从文件中读取一行数据 以CR LF结尾  
	/*@param[in] pHandle : GetFileHandle返回的指针
	/*@param[out] ppData : 返回数据存放的位置指针
	/*@return
	/*   -1 读取失败，>=0，读取成功，实际的字节数包括CR LF+0x00结尾，ppData返回相应的数据和大小
	************************************************************************/
	int (*ReadString)(tFileHandle*pHandle,unsigned char**ppData);

	/************************************************************************/
	/*@brief 将ReadString返回的内存释放
	/*@param[in] ppData : 和ReadString第二个参数一致
	***********************************************************************/
	void (*FreeString)(tFileHandle *pHandle,unsigned char**ppData);
	/************************************************************************/
	/*@brief 从当前档案文件指针位置开始读取指定字节数的档案文件数据                                                            
	/*@param[in] pHandle : GetFileHandle返回的指针
	/*@param[in] Offset : Seek的偏移量
	/*@param[in] From  : 从哪里开始偏移 IFILE_BEGIN etc
	/*@return
	/* -1表示失败，其他值为新的文件位置
	************************************************************************/
	int (*Seek)(tFileHandle *pHandle,unsigned int Offset,eFileFrom From);

	int (*Ftell)(tFileHandle *pHandle);

	/************************************************************************/
	/*@brief 从当前档案文件指针位置开始读取指定字节数的档案文件数据                                                            
	/*@param[in] pHandle : GetFileHandle返回的指针
	/*@param[out] pErrMsg : 
	/*@return
	/*	返回值-1表示出错，返回0表示没有错误消息，返回1表示有错误消息，
	************************************************************************/
	int (*GetErrMsg)(tFileHandle *pHandle,char* pErrMsg);
}tFileOpts;

struct tagFileHandle
{	
	void* pFileData;	///实际指向tFileData结构体
	tFileOpts Opts;
};


tFileHandle*GetFileHandle();
void PutFileHandle(tFileHandle*pHandle);

#ifdef __cplusplus
};
#endif 


#endif 