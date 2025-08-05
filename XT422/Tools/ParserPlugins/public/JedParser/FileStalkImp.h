#ifndef _FILESTALKIMP_H_
#define _FILESTALKIMP_H_

#include "Stalk.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MSGLEN_MAX		(256)
#define BUFSTRING_MAX	(512)

typedef enum{
	FILETYPE_DISK=0,	////本地磁盘文件
	FILETYPE_MEM=1,		///内存文件
	FILETYPE_NET=2,		///网络文件
	FILETYPE_HANDLE=3,	///外部传入已经打开的文件句柄
}eFileType;

typedef struct tagFileData
{
	eFileType FileType;
	void *hFile;///保存文件的句柄
	char ErrMsg[MSGLEN_MAX];
	char LineString[BUFSTRING_MAX];///为避免获取字符串总是分配和释放空间，这个地方做缓存
}tFileData;


#define GetFileData(_pFHandle) ((tFileData*)((_pFHandle)->pFileData))

#ifdef __cplusplus
};
#endif 

#endif 