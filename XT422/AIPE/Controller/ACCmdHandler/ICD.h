#pragma once

#include "ACTypes.h"

#pragma pack(push, 1)

#define MaxNetDataSize (4112)  //网络发送数据包最大的字节数

#define CMDID_PTPACK_SRC		(0x00)		//透传包发送
#define CMDID_PTPACK_ACK   		(0x01)		//透传包的ACK包的CMDID
#define CMDID_PTPACK_COMPLETE 	(0x02)		//透传包的Complete包的CMDID
#define CMDID_PTPACK_QUERYDOCMD (0x10)		//请求PC执行命令
#define CMDID_PTPACK_CUSTOM     (0x20)		//透传custom数据包	
typedef enum _eICDMsgID{
	ICDMsgID_LinkScan		= 0x01,	 //链路扫描包
	ICDMsgID_Heartbeat		= 0x02,  //心跳包
	ICDMsgID_DevInfoGet		= 0x03,  //设备信息获取包
	ICDMsgID_DevInfoResp	= 0x04,  //设备信息响应包
	ICDMsgID_Cmd			= 0x10,  //命令发送包
 	ICDMsgID_CmdRecvResp	= 0x11,  //命令包被接收到之后，需要发这个包表示接收到 ACK包
	ICDMsgID_CmdCompleteResp= 0x12,  //命令执行完成后发送的包 
	ICDMsgID_Interrupt		= 0x14,	 //中断包
	ICDMsgID_DataDownlink	= 0x18,  //下行数据包，从PC发往Device
	ICDMsgID_DataUplink		= 0x19,  //上行数据包，从Device发往PC
	ICDMsgID_PT             = 0x20,	 //透传包
}eICDMsgID;

/************************************************/
/*管理包定义*/
/************************************************/

//链路扫描包（LSP, Link Scan Packet）
//操作方式
//1. 主机发出，从口接收到，给 HOP_NUM 解出后+1，作为本级 HOP 号
//2. 从口收到给 HOP_NUM + 1 后从主口发出
//PC->Device
typedef struct _tLinkScanPacket{
	uint8_t MsgID;   //TYPE : 包类型。链路扫描包类型为 0x1。
	uint8_t HopNum; //HOP_NUM：节点逻辑编号。 
	uint8_t Reserved[30];
}tLinkScanPacket;  //链路扫描包 //Total 32Bytes


//心跳包（HBP, Heartbeat Packet）
/*
操作说明：
1. 各级设备主动从从口发出，每 1 秒钟发送一个。
2. 各级设备主口收到直接从从口转发出去。
3. 主机收到所有的心跳报文，画出 TOPO 关系图。
4. PORT_STATE 指示下挂的 socket 是否是 link 或者正常 ready 的
5. PORT*_TYPE 表示各 PORT 操作的 socket 类型，由 socket 接口模块定义输出，上位机显示和
socket 接口模块一致即可
*/
//Device->PC
typedef struct _tHeartbeatPacket{
	uint8_t MsgID;     //TYPE : 包类型。心跳包类型为 0x2。
	uint8_t HopNum;    //HOP_NUM：节点逻辑编号。
	uint16_t Reserved0 : 5;
	uint16_t LastHopFlag:1;  //L : Last. 末级节点 L 位置 1，非末级节点 L 位置 0。主机可通过 HOP_NUM 和 L 标识确定链路中节点的数量。  
	uint16_t Reserved1 : 10;  
	uint16_t LinkStatus; //LINK_STATUS: 链路状态信息 待完善
	uint8_t Reserved[26];
}tHeartbeatPacket;  //心跳包 Total 32Bytes

//设备信息读取包（DIGP, Device Information Get Packet）
//PC->Device
typedef struct _tDevInfoGetPacket {
	uint8_t MsgID;   //TYPE : 包类型。设备信息读取包 0x3。
	uint8_t HopNum; //HOP_NUM：节点逻辑编号。 
	uint8_t Reserved[30];
}tDevInfoGetPacket; //设备信息读取包 Total 32Bytes


typedef union _uMngPacket{
	uint8_t PckData[32];
	tLinkScanPacket LinkScan;
	tHeartbeatPacket Heartbeat;
	tDevInfoGetPacket DevInfoGet;
}uMngPacket; ///管理包


//设备信息响应包（DIRP, Device Information Response Packet）
//Device->PC
typedef struct _tDevInfoRespPacket {
	uint8_t MsgID;   //TYPE : 包类型。设备信息响应包 0x4。
	uint8_t HopNum;  //HOP_NUM：节点逻辑编号。 
	uint16_t Reserved0 : 5;
	uint16_t ValidFlag : 1; //V: Valid. 指示随后的 Device Information 是否有效。
							//0: 无效，1: 有效。
							//如果设备无法读取自己的设备信息，设备在发送 DIRP 时应将 V 标识清零，表
							//示设备信息读取失败。如果设备正常读取自身的设备信息，则在发送 DIRP 时
							//应将 V 标识置 1。
	uint16_t Reserved1 : 10;
	uint32_t Reserved[3];
	uint32_t DevInfoWord[8]; //Device Information Word 0~7: 设备信息。指的是硬件序列号、硬件类型、硬件版本号、Socket 个数等硬件相关信息。
}tDevInfoRespPacket;  //设备信息响应包 Total 48Bytes


typedef enum class _eSubCmdID {
	SubCmd_DataTrans_FIBER2SSD	= 0x01,
	SubCmd_DataTrans_FIBER2SKT	= 0x02,
	SubCmd_DataTrans_FIBER2DDR	= 0x06,
	SubCmd_DataTrans_SSD2FIBER	= 0x04,
	SubCmd_DataTrans_SSD2SKT	= 0x03,
	SubCmd_DataTrans_SKT2FIBER	= 0x05,
	SubCmd_DataTrans_DDR2FIBER	= 0x07,
	SubCmd_DataTrans_SSD2DDR	= 0x08,
	SubCmd_DataTrans_DDR2SSD	= 0x09,
	SubCmd_ReadCapacity_SSD		= 0x0A,
	SubCmd_ReadCapacity_DDR		= 0x0B,
	SubCmd_ReadCapacity_SKT		= 0x0C,
	SubCmd_Regist_Read			= 0x10,
	SubCmd_Regist_Write			= 0x11,
	SubCmd_ReadCRC32			= 0x18,
	SubCmd_MU_Start				= 0x400, //MU能够处理的命令起始, 最开始定义为0x40,根据文档修改为0x410
	SubCmd_MU_InstallFPGA		= 0x410,
	SubCmd_MU_InstallDriver		= 0x411,
	SubCmd_MU_SetChipInfo		= 0x412,
	SubCmd_MU_SetDriverSelfPara	= 0x413,
	SubCmd_MU_SetDriverCommon	= 0x414,
	SubCmd_MU_SetDriverPinMap	= 0x415,
	SubCmd_MU_SetDriverPartitionTable = 0x416,
	SubCmd_MU_DoCmdSequence		= 0x417,
	SubCmd_MU_SetDataBufferInfo = 0x418,
	SubCmd_MU_DownloadSSDComplete = 0x419,
	SubCmd_MU_SetBPUAttribute	= 0x420,
	SubCmd_MU_SetSNWithJson		= 0x421,
	SubCmd_MU_SetPartitionTableHeadAddr = 0x422,

	SubCmd_MU_AdapterRead		= 0x430,
	SubCmd_MU_AdapterWrite		= 0x431,
	SubCmd_MU_AdapterIncreaseCount = 0x432,
	SubCmd_MU_GetBPUInfo		= 0x433,
	SubCmd_MU_GetDeviceInfo		= 0x434,
	SubCmd_MU_SetDeviceAlias	= 0x435,
	SubCmd_MU_GetSktInfo		= 0x436,
	SubCmd_MU_GetSktInfoSimple	= 0x437,
	SubCmd_MU_GetMainBoardInfo	= 0x438,

	SubCmd_MU_RebootBPU			= 0x440,
	SubCmd_MU_SetBufferMapInfo	= 0x450,
	SubCmd_MU_UpdateFw			= 0x458, //更新FPGA固件到FW区
	SubCmd_MU_RebootMU			= 0x459, //固件升级完成后，PC主动发送Reset
	SubCmd_MU_UpdateDeviceTime	= 0x461,
	SubCmd_MU_DebugSetting		= 0x462,
	SubCmd_MU_GetRebootCause	= 0x463,
	SubCmd_MU_ProgramSetting	= 0x464,
	SubCmd_MU_GetProgramSetting = 0x465,
	SubCmd_MU_ProgrammerSelfTest = 0x466,
	SubCmd_MU_MasterChipAnalyze = 0x467,
	SubCmd_MU_ReadChipExtcsd	= 0x468,
	SubCmd_MU_GetSKTEnable		= 0x469,
	SubCmd_MU_DoSendCustom		= 0x490,

	//MU主动上报的
	SubCmd_MU_SetProgress		= 0x510,
	SubCmd_MU_SetLog			= 0x511,
	SubCmd_MU_SetEvent			= 0x514,
	SubCmd_ReadBuffData			= 0x520,
	SubCmd_MU_DoCustom			= 0x590,
	SubCmd_MU_End				= 0x9FF, //MU能够处理的命令终点
}eSubCmdID;

typedef enum class _eCustomTagID {
	SubCmd_CustomTag_MT422 = 0x20000014, 
}eCustomTagID;

/********************************************/
/*命令响应包定义*/
/*********************************************/


//数据传输子命令包 （CMDP_TR, CMD_ID: 0x1~0x7）
typedef struct _tCmdPacketDataTrans {
	uint8_t			MsgID;	//0x10
	uint8_t			HopNum;
	uint8_t			SeqNum : 5 ;    //seqnum值
	uint8_t			FirtPacketFlag : 1;	//首个命令包
	uint8_t			Reserve0 : 2; 
	uint8_t			CmdID;	   //0x1 - 0x7
	/*
		Table 13 数据传输子命令列表
		CMD_ID		 CMD_NAME		PORT_ID		SRC_ADDR		DEST_ADDR				 Description
		0x1			FIBER2SSD		0			N/A				X						光纤数据写入 NVMe SSD
		0x2			FIBER2SKT		X			N/A				X						光纤数据写入 Socket
		0x6			FIBER2DDR		0			N/A				X						光纤数据写入 DDR
		0x4			SSD2FIBER		0			X				XNVMe					SSD 数据读出并通过光纤送回上位机
		0x3			SSD2SKT			X			X				X						NVMe SSD 数据读出并写入 Socket
		0x5			SKT2FIBER		X			X				N/A						Socket 数据读出并通过光纤送回上位机
		0x7			DDR2FIBER		0			X				N/A						DDR 数据读出
		注：
		1. N/A 表示该字段对于该子命令不适用，应填充 0.
		2. X 表示非零值。
	*/
	uint32_t		PortID;    //PORT_ID：指定需要要操作的 PORT。对应 bit 为 1 表示对应 PORT 响应。
								//最多 32 个 PORT。PORT_ID 全为 0 表示该 CMDP 是发送给设备本身的命令
								//包，不涉及 PORT 操作。PORT_ID 与 Socket 的对应关系请详见“PORT_ID
								//与 SKT（Socket）的对应关系约定”。
								// AG06 FPGA 内部有 8 个负责烧录的软核 SoC 系统（BPU）和一个负责管理的软核 SoC 系统（MU）。
								// 负责烧录的 SoC 系统我们称为 BPU，负责管理的 SoC 系统我们称为 MU。
								//根据不同的烧录应用，每个 BPU 可能会对接一个 SKT，也可能会接两个 SKT。现对 PORT_ID 和 SKT 的
								//对应关系做如下约定：
								//1. PORT_ID[1:0]对应 BPU0 的 SKT[1:0]，PORT_ID[3:2]对应 BPU1 的 SKT[1:0]，以此类推。
								//2. 当一个 BPU 只对接一个 SKT 时，约定使用 BPU 的 SKT[0]，因此 8 个 BPU 的 8 个 SKT 对应
								//PORT_ID[0, 2, 4, …]，PORT_ID[1, 3, 5, …]保留。
	//下面的定义不同的子命令有不同的含义
	uint32_t		SrcAddrL;  ///读的时候指定Src地址，地址都是Byte地址，SSD时FPGA自己转换
	uint16_t		SrcAddrH;
	uint16_t		Reserved0;
	uint32_t		DestAddrL;  ///写的时候指定Dest地址，地址都是Byte地址，SSD时FPGA自己转换
	uint16_t		DestAddrH;
	uint16_t		Reserved1;
	uint32_t		LengthL;
	uint16_t		LengthH;
	uint16_t		Reserved2;
}tCmdPacketDataTrans; //32Bytes

//容量获取子命令包 （CMDP_CAP, CMD_ID: 0xA~0xC）
/*
Table 15 容量获取子命令列表
CMD_ID			CMD_NAME			PORT_ID				Description
0xA				CAPSSD				0					获取 NVMe SSD 容量
0xB				CAPDDR				0					获取 DDR 容量
0xC				CAPSKT				X					获取 SKT 设备容量
注：
1. X 表示非零值。
*/
typedef struct _tCmdPacketGetCapacity {
	uint8_t			MsgID;   // 0x10
	uint8_t			HopNum;
	uint8_t			SeqNum : 5 ;    //seqnum值
	uint8_t			Reserved0 : 3;   
	uint8_t			CmdID;	   // 0xA~0xC
	uint32_t		PortID;    //与上面的命令相同
	uint32_t		Reserved[6];
}tCmdPacketGetCapacity; //容量获取子命令包


/*寄存器访问子命令包（CMDP_REG, CMD_ID: 0x10~0x11）*/
/*
Table 18 寄存器访问子命令列表
CMD_ID		CMD_NAME		PORT_ID			REG_ADDR		REG_VALUE		Description
0x10		REG_WR			0				X				X				读寄存器命令
0x11		REG_RD			0				X				N/A				写寄存器命令
注：
1. N / A 表示该字段对于该子命令不适用，应填充 0.
2. X 表示任意值。

操作说明：
1. 从机完成寄存器读写操作后，需要回复操作完成包（CCRP_REG_R）.
*/
typedef struct _tCmdPacketRegister {
	uint8_t			MsgID;   // 0x10
	uint8_t			HopNum;
	uint8_t			SeqNum : 5;    //seqnum值
	uint8_t			Reserved0 : 3;
	uint8_t			CmdID;	   // 0x10~0x11
	uint32_t		PortID;    //与上面的命令相同
	uint32_t		Reserved1[2];
	uint32_t		RegAddr;
	uint32_t		Reserved2;
	uint32_t		RegValue;
	uint32_t		Reserved3;
}tCmdPacketRegister; //寄存器访问子命令包


/*
数据块 CRC32 读取子命令包（CMDP_CRC, CMD_ID:0x18）
Table 20 数据块 CRC32 读取子命令包（CMDP_CRC）列表
CMD_ID		CMD_NAME		PORT_ID			Description
0x18		CRC32_RD		X				数据块CRC读取子命令
注：
1. X 表示任意值。
操作说明：
1. 该命令用于数据块传输完成后，主机读取从机计算的数据块 CRC32 值。
*/

typedef struct _tCmdPacketGetCRC32 {
	uint8_t			MsgID;   // 0x10
	uint8_t			HopNum;
	uint8_t			SeqNum : 5;    //seqnum值
	uint8_t			Reserved0 : 3;
	uint8_t			CmdID;	   // 0xA~0xC
	uint32_t		PortID;    //与上面的命令相同
	uint32_t		Reserved[6];
}tCmdPacketGetCRC32; //容量获取子命令包

typedef struct _tCmdPacketCommon{
	uint8_t			MsgID;		
	uint8_t			HopNum;
	uint8_t			SeqNum : 5;    //seqnum值
	uint8_t			Reserved0 : 3;
	uint8_t			CmdID;	   
	uint32_t		PortID;    
	uint8_t 		CmdSpec[24];
}tCmdPacketCommon;


typedef union _uCmdPacket {
	tCmdPacketCommon  Common;
	tCmdPacketGetCapacity GetCapcity;
	tCmdPacketDataTrans Trans;
	tCmdPacketRegister Register;
	tCmdPacketGetCRC32 GetCRC32;
}uCmdPacket;


/*命令接收响应包（CRRP，Command Reception Response Packet）*/
/*
MsgID = 0x11
命令接收响应包，由各从机发送，用于响应主机发送的命令包。从机接收到任何 2 命令包（CMDP,
Command Packet）中定义的命令包时，都需要立即发送 CRRP 包进行响应。
*/

typedef struct _tCmdRespPacket{
	uint8_t			MsgID;   // 0x11
	uint8_t			HopNum;
	uint8_t			SeqNum : 5;    //seqnum值
	uint8_t			Reserved0 : 3;
	uint8_t			CmdID;	   // 与命令包的CmdID一致
	uint32_t		PortID;    // 与命令包的PortID一致
	uint8_t			Reserved[24];
}tCmdRespPacket;


/*
命令完成响应包（CCRP，Command Completed Response Packet）
命令完成响应包，由各从机发送，用于响应主机发送的命令包。从机接收到 2 命令包（CMDP,
Command Packet）中定义的命令包，并完成指定的操作后，需要向主机发送 CCRP 包进行响应
*/

/*数据传输子命令完成响应包 （CMDP_TR_R , CMD_ID: 0x1~0x7）*/
typedef struct _tCmdCplPacketTrans {
	uint8_t			MsgID;   // 0x12
	uint8_t			HopNum;
	uint8_t			SeqNum : 5;    //seqnum值
	uint8_t			Reserved0 : 3;
	uint8_t			CmdID;	   // 与命令包的CmdID一致
	uint32_t		Status;    // STATUS: 每个 bit 代表对应 SKT 的命令完成状态。
							   // 0：命令执行失败，1：命令执行成功。
	uint8_t		    Reserved[24];
}tCmdCplPacketTrans;


/*容量获取子命令完成响应包 （CMDP_CAP_R , CMD_ID: 0xA~0xC）*/
typedef struct _tCmdCplPacketGetCapacity {
	uint8_t			MsgID;   // 0x12
	uint8_t			HopNum;
	uint8_t			SeqNum : 5;    //seqnum值
	uint8_t			Reserved0 : 3;
	uint8_t			CmdID;	   // 与命令包的CmdID一致
	uint32_t		Status;    // STATUS: 每个 bit 代表对应 SKT 的命令完成状态。
							   // 0：命令执行失败，1：命令执行成功。
	uint32_t		Reserved[4];
	uint32_t		CapacityL;  //1. CAPACITY 每 bit 含义由应用层决定。比如，在烧录 SATA 的应用中，每 bit 代表 4KB，而在烧录
								//eMMC 的应用中，每 bit 代表 512B。
	uint16_t		CapacityH;
	uint16_t		Reserved1;
}tCmdCplPacketGetCapacity;

/*寄存器访问子命令完成响应包 （CMDP_REG_R , CMD_ID: 0x10~0x11）*/
typedef struct _tCmdCplPacketReadReg {
	uint8_t			MsgID;   // 0x12
	uint8_t			HopNum;
	uint8_t			SeqNum : 5;    //seqnum值
	uint8_t			Reserved0 : 3;
	uint8_t			CmdID;	   // 与命令包的CmdID一致
	uint32_t		Status;    // STATUS: 每个 bit 代表对应 SKT 的命令完成状态。
							   // 0：命令执行失败，1：命令执行成功。
	uint32_t		Reserved[2];
	uint32_t		RegAddr;  //
	uint32_t		Reserved1;
	uint16_t		RegValue;
	uint32_t		Reserved2;
}tCmdCplPacketReadReg;

/*数据块 CRC32 读取 子命令完成响应包（CMDP_CRC_R, CMD_ID:0x18）*/
typedef struct _tCmdCplPacketGetCRC32{
	uint8_t			MsgID;   // 0x12
	uint8_t			HopNum;
	uint8_t			SeqNum : 5;    //seqnum值
	uint8_t			Reserved0 : 3;
	uint8_t			CmdID;	   // 与命令包的CmdID一致
	uint32_t		Status;    // STATUS: 每个 bit 代表对应 SKT 的命令完成状态。
							   // 0：命令执行失败，1：命令执行成功。
	uint32_t		Reserved[4];
	uint32_t		CRC32_Downlink; //下行（Downlink）数据块 CRC32 值
	uint32_t		CRC32_Uplink;  //上行（Uplink）数据块 CRC32 值。
}tCmdCplPacketGetCRC32;


typedef union _uCmdCplPacket {
	uint8_t PckData[32];
	tCmdCplPacketTrans Trans;
	tCmdCplPacketGetCapacity GetCapacity;
	tCmdCplPacketReadReg ReadReg;
	tCmdCplPacketGetCRC32  GetCRC32;
}uCmdCplPacket;



/*中断包（INTP, Interrupt Packet）*/
typedef struct _tInterruptPacket {
	uint8_t			MsgID;   // 0x14
	uint8_t			HopNum;
	uint8_t			SeqNum : 5;    //seqnum值
	uint8_t			Reserved0 : 3;
	uint8_t			INTID;	   // 中断ID
	uint32_t		PortID;    
	uint32_t		InterruptSpec[6];
}tInterruptPacket;


#define FirstPacket_Flag (1<<5)
typedef struct _tDataPacket {
	uint8_t			MsgID;   // 0x18 0x019
	uint8_t			HopNum;
	uint16_t		Reserved0 : 5;
	uint16_t		FristFlag : 1 ; // F : First，数据块首个数据包标识  (从bit0开始)
	uint16_t		Reserved1 : 10;
	uint32_t		PortID;
	uint16_t		PNum;	//Packet Number,数据块的数据包编号。
	uint16_t		Length;	//实际使用0-10共11bit，其他5位Reserved LENGTH 最大值为 1024, 1表示4个字节
							//Data playload 长度，以 32-bit word 为单位，用于说明随后
							//Data payload 的长度。LENGTH 最大值为 1024, 1表示4个字节
	uint32_t		Reserved;
	//uint32_t		Data[1024]; ///为避免每次都进行内存分配，按照实际最大的能够发送的数据量进行操作
	uint8_t			Data[4096];
}tDataPacket;  //包长：4112 byte

#define PTPACKET_PAYLOADLEN  (1016)  //透传包的Payload整体长度，1024-tCmdPTPacket中Data的前面部分
typedef struct _tCmdPacketPT {
	uint8_t			MsgID;   // 0x20
	uint8_t			HopNum;
	uint8_t			SeqNum : 5;    //seqnum值
	uint8_t			Reserved0 : 3;
	uint8_t			MsgSubID;	   // PT包子命令ID
	uint32_t		PortID;
	uint8_t			Data[PTPACKET_PAYLOADLEN];
}tCmdPacketPT;
typedef struct _tCmdPacketDP {
	uint8_t			HEAD1;   // 0xAA
	uint8_t			HEAD2;	//0X55
	uint8_t			BPUID;    //0~7
	uint8_t			SEQ_NUM;	//循环使用每个bpu有自己的seq_num
	uint8_t			UART_IDX;	   // 0~4
	uint16_t		DATA_LEN;
	uint8_t			RES; //保留
}tCmdPacketDP;
typedef struct {
	int32_t RecvBufSize;   //接收的Buffer大小
	int32_t SendBufSize;   //发送的Buf大小
	int32_t RecvTimeoutms; //发送超时时间单位毫秒
	int32_t SendTimeoutms; //接收超时时间单位毫秒
	bool bReuseaddr;
	int32_t ZeroCopy;	  //是否不要进行Socket的拷贝，设置为0表示不经过Socket到缓存区的拷贝
}tSocketOpts;

class CICD
{
public:
	static eSubCmdID TransStrCmd2CmdID(QString strCmd);
};

#pragma pack(pop)