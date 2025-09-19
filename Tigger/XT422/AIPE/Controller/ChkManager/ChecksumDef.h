#pragma once
#include <string>
class IDataBuffer;
class IOutput;

typedef enum {
	CHECKSUM_NONE = 0,
	CHECKSUM_BYTE = 1,
	CHECKSUM_WORD = 2,
	CHECKSUM_CRC16 = 3,
	CHECKSUM_CRC32 = 4,
}CheckSumType;

typedef struct CheckSumParam {
	uint32_t		 uiCheckSumType;	//外部传入时是客户选择Checksum类型下标，
	//范围为0-DllGetCkSumName返回的值,
	//调用DllCheckSum之后内部需要将其修改实际对应的类型，CHECKSUM_WORD等

	uint32_t		 uiAgIc;			//驱动中使用的IC算法编号
	const unsigned char* pcSpcBuffer;		//ConfigBuffer的数据指针
	uint32_t		 uiSpcBuffLen;		//指明pcSpcBuffer允许使用的最大字节数
	IDataBuffer* pDataBuffer;		//数据Buffer操作指针，不能直接使用内部的函数，
	//如果需要数据，请通过ReadBuffer函数调用读取
	IOutput* pOutput;			//日志输出操作指针，不能直接使用内部的函数
	//如果需要打印日志请通过PrintLog函数调用
	uint64_t		 uiVChkSum;			//用于保存verify checksum 固定为crc16类型
	std::string blockJsonString;
}CHECKSUMPARAM;