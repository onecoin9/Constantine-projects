#ifndef _JEDPLAYERIMP_H_
#define _JEDPLAYERIMP_H_

#include "JedPlayerAPI.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************************************************************/
///@brief 解析相应的域内容
///@param[in] pPlayer : JedPlayer_GetPlayer得到的句柄指针
///@param[in] pString : 待解析的字符串
///@param[in] Size : 字符串的总长度，包括\0
///@return
///  -1 表示失败， 0表示成功，可通过GetErrMsg获取失败信息
/***********************************************************************/
typedef int (*IDParser)(tJedPlayer*pPlayer,char*pString,int Size);

/************************************************************************/
///@brief 解析相应的域内容后遇到*结束符，执行该操作
///@param[in] pPlayer : JedPlayer_GetPlayer得到的句柄指针
///@return
///  -1 表示失败， 0表示成功，可通过GetErrMsg获取失败信息
/***********************************************************************/
typedef int (*IDAction)(tJedPlayer*pPlayer);



typedef struct tagJedPlayerData
{
	int ErrNo;
	char ErrMsg[256];				///错误消息
	int NotePrintEn;				///是否打印Note信息
	unsigned short ChkSumFile;		///文件中的Checksum
	unsigned short ChksumCalc;		///计算得到的Checksum
	unsigned int FuseNum;			///Number of Fuses
	unsigned int PinNum;			///Number of Pins
	unsigned int MaxVectorNum;		///Maximum Number of Test Vectors
	unsigned char DefaultFuseState;	///默认的Fuse状态，0或者1
	unsigned char bStringIDIn;		///当前解析是否存在识别器ID
	tFuseData FuseList;		///Fuse List，临时性解析得到的数据存放的位置，
	tFuseData EFuseData;	////Electical Fuse Data; 临时性解析得到的数据存放的位置，
	tFuseData UserData;	////User Data Field; 临时性解析得到的数据存放的位置，
	unsigned short ChksumFuseListFile;	///所有Fuselist的Checksum值，来源于文件中
	unsigned short ChksumFuseListCalc;	///所有Fuselist的Checksum值，来源于软件计算
	unsigned int ArchCodeNum;  ///Architecture Code Number
	unsigned int PinoutCode;   ///Pinout Code Number
	unsigned int SecurityFuseProgramEn; ///Security Fuse 是否能够被编程，0为不行，1为可以
	tFuseDataMap FuseDataMap;///所有FuseList数据的集合，外部访问之后最后释放
	tFuseData FeatureRowBit;///FeatureRowBit的数据，外部访问之后最后释放
	tFuseData UserCode; ///UserCode数据，外部访问之后最后释放
}tJedPlayerData;

typedef struct tagJedID{
	char* ID;
	IDParser pIDParser;
	IDAction pIDAction;///执行相应的ID处理
}tJedID;


#define GetJedPlayerData(_pPlayer) ((tJedPlayerData*)((_pPlayer)->pPlayerData))
#define SetErrMsg(_pPlayData,_ErrNo,fmt,...) \
do{\
if((_pPlayData)->ErrNo==0){(_pPlayData)->ErrNo=_ErrNo;sprintf((_pPlayData)->ErrMsg,fmt,__VA_ARGS__);}\
} while(0)

tJedID* GetJedID(unsigned char ID);
int FuseDataMap_AllocData(tJedPlayer*pPlayer,int TotalFuseBit);
void FuseDataMap_FreeData(tJedPlayer*pPlayer);
void FeatureRowBit_Free(tJedPlayer *pPlayer);
void UserCode_Free(tJedPlayer*pPlayer);

#ifdef __cplusplus
};
#endif 


#endif 