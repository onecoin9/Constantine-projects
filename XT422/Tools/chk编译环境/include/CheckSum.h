#ifndef __CHECKSUM_H__
#define __CHECKSUM_H__

//#include "IOutput.h"
//#include "DataBuffer.h"
#include <string>
class IDataBuffer;
class IOutput;

//日志级别
#define LOGERR (2)			//错误日志
#define LOGWARNING (1)		//警告日志
#define LOGNORMAL (0)		//正常日志
//*******************************************************************//
//此处的顺序不能改变，当有新的checksum type时，只能顺序往下增加,
//同时firmware需作相应修改.
//*******************************************************************//
typedef enum {
	CHECKSUM_NONE   = 0,
	CHECKSUM_BYTE	= 1,
	CHECKSUM_WORD	= 2,
	CHECKSUM_CRC16	= 3,
	CHECKSUM_CRC32  = 4,
}CheckSumType;

typedef struct CheckSumParam{
	uint32_t		 uiCheckSumType;	//外部传入时是客户选择Checksum类型下标，
	//范围为0-DllGetCkSumName返回的值,
	//调用DllCheckSum之后内部需要将其修改实际对应的类型，CHECKSUM_WORD等
	
	uint32_t		 uiAgIc;			//驱动中使用的IC算法编号
	const unsigned char  *pcSpcBuffer;		//ConfigBuffer的数据指针
	uint32_t		 uiSpcBuffLen;		//指明pcSpcBuffer允许使用的最大字节数
	IDataBuffer *pDataBuffer;		//数据Buffer操作指针，不能直接使用内部的函数，
	//如果需要数据，请通过ReadBuffer函数调用读取
	IOutput     *pOutput;			//日志输出操作指针，不能直接使用内部的函数
	//如果需要打印日志请通过PrintLog函数调用
	uint64_t		 uiVChkSum;			//用于保存verify checksum 固定为crc16类型
	std::string blockJsonString;
}CHECKSUMPARAM;



extern "C" {
	///////////////////////////////////////////
	///@brief : DllGetCkSumName 获取Chk支持的Checksum类型
	///@param[in] lpBuff : 外部传入的字符串存放区域
	///@param[in] uiLen : lpBuff允许使用的长度
	///@return : 返回支持的类型个数
	///@note : 每种类型后面需要跟一个0x00作为该类型Checksum名称的结束符
	///示例：比如驱动支持Byte和Word两个种类型，则lpBuff最后填充为"Byte.Word.",其中.为0x00
	///返回值为2
	///////////////////////////////////////////
	__declspec(dllexport) uint32_t  DllGetCkSumName(char *lpBuff, uint32_t uiLen);

	///////////////////////////////////////////
	///@brief : DllCheckSum 获得Buffer的校验值
	///@param[in] pParam：外部传入的结构体参数，参看CHECKSUMPARAM结构体说明
	///@return : 计算得到的Checksum的值
	///////////////////////////////////////////
	__declspec(dllexport) uint64_t DllCheckSum(CHECKSUMPARAM *pParam);
	
	///////////////////////////////////////////
	///@brief : Std_Checksum_Cal 公共库CommonChk.lib提供的接口，用于帮助计算Checksum
	///@param[in] pParam：外部传入的结构体参数，参看CHECKSUMPARAM结构体说明
	///@param[in] adrPos: Buffer的起始地址
	///@param[in] dwLen: 计算的长度
	///@param[in] dwCheckSum : Checksum初始值, 第一次初始值为0
	///@param[in] dwMaskValue : Mask值，unsigned char,CRC16,CRC32类型在计算之前将字节内容与该值进行与操作，
	///							WORD类型在计算之前按照2字节与该值进行与操作
	///@param[in,out] dwSumCount: CRC32计算时最后需要反转，通过该值存放当前已经处理的字节数，
	///								外部需传入，第一次初始值为0, CRC32计算时最后还要调用Get_CRC32sum
	///@param[in] showprocessFlag : 是否需要显示进度，0表示不显示，1表示显示
	///@return :计算得到的Checksum值
	///////////////////////////////////////////
	__declspec(dllexport) uint64_t Std_Checksum_Cal(const  CHECKSUMPARAM * param, unsigned long adrPos, unsigned long dwLen,
		uint64_t dwCheckSum, unsigned long dwMaskValue, unsigned long *dwSumCount,unsigned long showprocessFlag);

	///////////////////////////////////////////
	///@brief : Std_VFCRC16_Checksum_Cal 公共库CommonChk.lib提供的接口，用于帮助计算Verify的Checksum
	///				固定为CRC16方式
	///参数与返回值参看 Std_Checksum_Cal
	//////////////////////////////////////////////////
	__declspec(dllexport) unsigned long Std_VFCRC16_Checksum_Cal(CHECKSUMPARAM * param, unsigned long adrPos, unsigned long dwLen,
		unsigned long uiVChkSum, unsigned long dwMaskValue, unsigned long *dwSumCount,unsigned long showprocessFlag);


	///////////////////////////////////////////
	///@brief : Std_VFCRC64_Checksum_Cal 公共库CommonChk.lib提供的接口，用于帮助计算Verify的Checksum
	/// 			固定为CRC64方式
	///参数与返回值参看 Std_Checksum_Cal 次方法还未有实际函数对应
	//////////////////////////////////////////////////
	__declspec(dllexport) uint64_t Std_VFCRC64_Checksum_Cal(CHECKSUMPARAM * param, unsigned long adrPos, unsigned long dwLen,
		uint64_t uiVChkSum, unsigned long dwMaskValue, unsigned long *dwSumCount,unsigned long showprocessFlag);
	
	///////////////////////////////////////////
	///@brief : Get_CRC32sum 获取CRC32的最终Checksum值，在调用完Std_Checksum_Cal之后需要调用该函数
	///@param[in] pParam：外部传入的结构体参数，参看CHECKSUMPARAM结构体说明
	///@param[in] dwSumCount ： Std_Checksum_Cal统计的已经计算的Checksum字节数
	///@return : CRC32的最终校验值
	///////////////////////////////////////////
  __declspec(dllexport) unsigned long Get_CRC32sum(unsigned long CRC32,unsigned long *dwSumCount);

  ///////////////////////////////////////////
  ///@brief : PrintLog 打印日志
  ///@param[in] pParam：外部传入的结构体参数，参看CHECKSUMPARAM结构体说明
  ///@param[in] LogLevel ： 日志级别LOGNORMAL等
  ///@param[in] Msg ： 消息字符串以\0结尾
  ///@return : 无
  ///////////////////////////////////////////
  __declspec(dllexport) void PrintLog(const  CHECKSUMPARAM * param,int32_t LogLevel,char*Msg);

  ///////////////////////////////////////////
  ///@brief : ReadBuffer读取数据Buffer的内容
  ///@param[in] pParam：外部传入的结构体参数，参看CHECKSUMPARAM结构体说明
  ///@param[in] StartAddr: 起始Buffer地址
  ///@param[in] pData:数据存放位置
  ///@param[in] Size：读取的字节数
  ///@return ：实际读取的字节数
  ///////////////////////////////////////////
  __declspec(dllexport) int32_t ReadBuffer(const  CHECKSUMPARAM * param,uint64_t StartAddr,unsigned char*pData,int32_t Size);
  __declspec(dllexport) int32_t WriteBuffer(const  CHECKSUMPARAM * param,uint64_t StartAddr,unsigned char*pData,int32_t Size);

  

 //设置进度条总长度
  __declspec(dllexport) void SetProgress(const  CHECKSUMPARAM * param,uint32_t uiComplete);

   //设置进度条位置
   __declspec(dllexport) void SetProgPos(const  CHECKSUMPARAM * param,uint32_t uiPos);

   
};


typedef uint32_t (*fnDllGetCkSumName)(char * lpBuff, uint32_t uiLen);
typedef uint64_t (*fnDllCheckSum)(CHECKSUMPARAM *pParam);

#define FUNC_NAME "DllCheckSum"
#define GET_CHECKSUM_NAME "DllGetCkSumName"

#endif