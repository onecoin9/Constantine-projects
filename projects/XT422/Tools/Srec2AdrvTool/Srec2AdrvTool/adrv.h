#include <iostream>

//昂科ADRV格式说明
//tADrvHeader+tDrvALV
//tADrvHeader结构体占用128个字节

#pragma pack(push, 1)

typedef struct _tADrvHeader{
	uint16_t Checksum; 			///<从2字节开始，到头部结束的数据的CRC16值
	uint8_t Magic[4] ; 			///<ADRV
	uint8_t UUID[16];   		///<16个字节的档案的UUID，可以被用于加密算法，秘钥生成算法
	uint32_t ALVOffset;  		///<ALV区域的偏移
	uint32_t ALVSize;    		///<ALV区域的总字节数
	uint32_t ALVNum;     		///<ALV区域的ALV数量
	uint32_t ALVChksum;  		///<ALV区域的校验值，从ALVOffset开始的ALVSize字节CRC16值
	uint8_t EncrypType; 		///<加密类型，0表示数据不加密，其他表示加密，后面再定义
	uint32_t EntryAddr;			///<程子的人口地址，米自S7，s8或S9
	uint8_t Reserved[85];		///<保留，默认为0
}tADrvHeader;

typedef struct _tBinALV{
	uint64_t Offset;     	///<数据要被放置的位置，可以是DDR的位置，
							///<也可以是Flash等存储的位置，支持eMMC等大容量超过4G
	uint32_t Len;        	///<数据长度
	uint8_t Data[0];		///<实际的数据N个字节
}tDrvALV;

#pragma pack(pop)