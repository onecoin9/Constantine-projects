#ifndef _ACCMDPACKET_H_
#define _ACCMDPACKET_H_

#include <stdint.h>


#pragma pack(push)
#pragma pack(1)
#define PACKED 

#define CmdFlag_MemAlloc (1<<0)  //使用动态库分配得到
#define CmdFlag_Notify	 (1<<1)  //是一个通知包，不需要有对方的返回
#define CmdFlag_DataInSSD  (1<<8)
#define CmdFlag_DataInDDR  (1<<9)
#define CmdFlag_DataInPCK  (1<<10)


#define CmdID_CmdDone	  (0xFFFF)

/***********MU<-->BPU**************/
/**下面定义的是MU发送诶BPU的命令**/
#define CmdID_SetChipInfo 	(0x0012)
#define CmdID_EnableSN		(0x0020)
#define CmdID_ReadChipUID  	(0x0022)

/**下面定义的是BPU发送诶MU的命令**/
#define CmdID_SetProgress   (0x0110)
#define CmdID_SetLog		(0x0111)
#define CmdID_SetVoltage	(0x0120)
#define CmdID_ReadVoltage	(0x0121)

/***********PC<-->MU**************/
/**下面是PC发送给MU的命令**/
#define CmdID_InstallFPGA	(0x04010)
#define CmdID_InstallDriver (0x04011)


#define CmdID_SetProgress   (0x0110)
#define CmdID_SetLog		(0x0111)
#define CmdID_ReadBuffData  (0x0120)

/**下面是MU发送给PC的命令**/

typedef struct _bitCmdFlag{
	uint32_t MemAlloc:1;
	uint32_t Notify:1;
	uint32_t Reserved1:6;
	uint32_t DataInSSD:1;
	uint32_t DataInDDR:1;
	uint32_t DataInPCK:1;
	uint32_t Reserved2:21;
}bitCmdFlag;

typedef struct _tACCmdPack{
	uint32_t CmdFlag; 		//命令标识，bit1为1表示pCmdData malloc得到，需要释放，为0表示使用的静态，不需要释放 CmdFlag_MemAlloc等
	uint32_t SKTEn;			//烧录座使能SKTEn[1:0]->BPUO 的 SKT[1:0]
						//			SKTEn[3:2]->BPUl 的 SKI[1:0]
						//			....
						//			SKIEn[15:14]->BPU7 的 SKT[l:0]
	uint16_t BPUID:4;		//BPUID的ID，目标BPU的ID,0-7
	uint16_t Reserved:12;  //保留
	uint16_t CmdID;   		//命令ID
	uint32_t CmdDataSize;   //命令ID对应的数据大小
	uint8_t  CmdData[0];    //命令ID对应的数据所在的内存位置
}PACKED tACCmdPack;	

#pragma pack(pop)

#define ACCmdPack_GetTotalSize(_pCmdPack) ((_pCmdPack)->CmdDataSize+sizeof(tACCmdPack))

#endif
