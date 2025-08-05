#ifndef _JEDPLAYERAPI_H_
#define _JEDPLAYERAPI_H_

#include "Stalk.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define JED_STX (0x02)
#define JED_ETX	(0x03)
#define JED_ASTERISK ('*')

/////////实现一次解析，多次使用
typedef struct tagBitDataMap{////需要烧录的Fuse数据bit索引
	int StartBit;		///起始位置
	int LenBit;		///有效Bit数
	struct tagBitDataMap *Next;
}tBitDataMap;

typedef struct tagFuseDataMap{
	unsigned char *pData;	///数据存放位置
	int DataBitSize;	////总的Bit数
	tBitDataMap *pBitMapHead; ////Bit映射，避免
	///内部使用
	unsigned int NextFuseListIdx;///目前的FuseList位置，临时使用
	int nDefaultFalg;
	int nPreIsNoRemain;
	int nGValue; //G域的值
	int nFirstFuseFalg;
	int nTotalRealBitSize;
	tBitDataMap *pCurBitDataMap; ///当前正在使用的有效的BitDataMap;
}tFuseDataMap;///Fuse数据映射，解析之后会将所有需要被烧录的Fuse List数据存放到这个位置，

//#define FUSELIST_CACHEDATA (16)

#define FUSELIST_CACHEDATA (1*1024*1024*8)
typedef struct tagFuseList{///记录从FuseStartNum开始的DataBitLen长度的Fuse状态
	unsigned int FuseStartNum;  ///当前记录的起始Fuse位置
	//	unsigned char FuseData[FUSELIST_CACHEDATA]; ///为了避免每次动态分配，需要使用这个预先分配好的
	unsigned char *pFuseData;	///如果实际的数据大于FUSELIST_CACHEDATA，则会采用此成员，进行动态分配
	unsigned int DataBitLen;	///实际使用的Bit数
	unsigned int DataBitMaxLen; ///最大使用的Bit数
}tFuseData;
//////////////////////////////////////////////////////////////////////////////////////

typedef struct tagJedPlayer tJedPlayer;
typedef struct tagPlayerOpts
{
	/************************************************************************/
	/*@brief 确认给定的JED文件的Checksum是否正确
	/*@param[in] pPlayer: JedPlayer_GetPlayer得到的句柄指针
	/*@param[in] FileName : 文件路径
	/*@return
	/*	校验成功返回1，校验不正确返回0，获取失败返回-1
	***********************************************************************/
	int (*CheckDiskFile)(tJedPlayer *pPlayer,char*FileName);
	int (*ParserDiskFile)(tJedPlayer*pPlayer,char*FileName);
	/************************************************************************/
	/*@brief 确认给定的内存JED文件的Checksum是否正确
	/*@param[in] pPlayer: JedPlayer_GetPlayer得到的句柄指针
	/*@param[in] pData : 内存起始地址
	/*@param[in] Size : 大小
	/*@return
	/*	校验成功返回1，校验不正确返回0，获取失败返回-1
	***********************************************************************/
	int (*CheckMemFile)(tJedPlayer *pPlayer,unsigned char*pData,int Size);
	int (*ParserMemFile)(tJedPlayer*pPlayer,unsigned char*pData,int Size);

	int (*CheckHandleFile)(tJedPlayer *pPlayer,HANDLE hFile);
	int (*ParserHandleFile)(tJedPlayer*pPlayer,HANDLE hFile);

	/***************************
	/*@brief 解析完成之后调用该函数确定FuseListChecksum是否匹配成功
	/*@return
	     校验成功返回1，校验不正确返回0，获取失败返回-1
	*****************************/
	int (*CompareFuseListChecksum)(tJedPlayer *pPlayer);
	

	///需要在ParserDiskFile或者ParserMemFile函数调用之后才可使用GetBitDataMap
	tFuseDataMap* (*GetFuseDataMap)(tJedPlayer*pPlayer);
	tFuseData* (*GetFeatureRowBit)(tJedPlayer*pPlayer);
	tFuseData* (*GetUserCode)(tJedPlayer*pPlayer);
	/************************************************************************/
	/*@brief 在失败的情况下，可以通过该函数获取失败信息
	/*@param[in] pPlayer: JedPlayer_GetPlayer得到的句柄指针
	/*@param[in] pErrMsg : 返回失败信息
	/*@param[in] Size    : 实际能够使用的pErrMsg区域的字节数
	/*@return
	/*	返回值-1表示出错，返回0表示没有错误消息，返回1表示有错误消息，
	***********************************************************************/
	int (*GetErrMsg)(tJedPlayer *pPlayer,char* pErrMsg,int Size);
	int (*GetRealSize)(tJedPlayer *pPlayer, int* nSize);
}tOpts;



struct tagJedPlayer
{
	void *pPlayerData;
	tOpts Opts;
	tFileHandle *pFileHandle;
	tIReporter *pReporter;
	void *PrivData;///外部定义的私有数据
	UINT UserCodeWriteMode; ////UserCode的Buffer写入方式，为0表示每个字节按照低bit优先的方式，为1表示按照高bit优先的方式
};


tJedPlayer* JedPlayer_GetPlayer(void *PrivData);
void JedPlayer_AttachFileHandle(tJedPlayer *pPlayer,tFileHandle *pFileHandle);
void JedPlayer_AttachReporter(tJedPlayer *pPlayer,tIReporter *pReporter);
void JedPlayer_PutPlayer(tJedPlayer* pPlayer);


int TestJedPlayer(char *FileName,tIReporter *pReporter);

#ifdef __cplusplus
}


#endif 

#endif 