// CmdPackHeader 命令包头结构体
type CmdPackHeader struct {
	CmdFlag     uint32 // 命令标记，预留使用，默认为0
	CmdID       uint16 // 不同的命令码有不同的功能
	CmdDataSize uint16 // 不同的命令码有不同的命令数据长度
}

// CmdPacket 完整的命令包结构
type CmdPacket struct {
	Header CmdPackHeader
	Data   []byte // 实际的命令数据
}

1.5.启停指令（CmdId=0x01）
命令功能：开启或者关闭小转台主板
发送时机点说明：定时发送，板子进行测试时发送开启，板子停止测试时发送停止

主机发送数据：
CmdID：0x01
CmdDataSize：内容为5字节
CmdData：
成员	说明	类型	大小	备注
State	温控板状态	Uint8	1	01 为开启，00为停止
Dut_active	DUT 控制位	Uint16	1	DUT的Active信息，bit0-7表示DUT1-8，Bit15表示313标准标定数据
Time	时间戳	Uint32	4	启动时间

1.6.测试板反馈数据（CmdId=0x8001）
命令功能：测试板所有DUT及外挂陀螺仪的测试数据
发送时机点说明：开启测试后，定时发送，主动上报


测试板发送数据：
CmdID：0x8001
CmdDataSize：内容为1+8+1+30*8+30+2
CmdData：
成员	说明	类型	大小	备注
Test state	测试状态	Uint8	1	01 为测试中，00为停止 02 故障
SN	主板序列号	Uint32	4	主板序列号或主板地址
Time
	时间序列	Uint32	4	上报时间
Dut_active	DUT 数据使能	Uint16	2	DUT的Active信息，bit0-7表示DUT1-8，Bit15表示313标准标定数据 0：无效  1：有效
Start		Uint8	1	采样数据起始标志 固定为0xBC
Gyro_x
	X轴陀螺数据	Uint32	4	
Gyro_y
	Y轴陀螺数据	Uint32	4	
Gyro_z	Z轴陀螺数据	Uint32	4	
Gyro_acc_x	X轴加表数据	Uint32	4	
Gyro_acc_y	Y轴加表数据	Uint32	4	
Gyro_acc_z	Z轴加表数据	Uint32	4	
Gyro_mix	正交耦合数据	Uint32	4	
Gyro_temperatrue	温度数据	Uint16	2	  
Sample crc		Uint8	1	采样校验值
以上数据*7
Start		Uint8	1	样本起始标志
Gyro_x
	X轴陀螺数据	Uint32	4	外挂陀螺仪数据
Gyro_y
	Y轴陀螺数据	Uint32	4	外挂陀螺仪数据
Gyro_z	Z轴陀螺数据	Uint32	4	外挂陀螺仪数据
Gyro_acc_x	X轴加表数据	Uint32	4	外挂陀螺仪数据
Gyro_acc_y	Y轴加表数据	Uint32	4	外挂陀螺仪数据
Gyro_acc_z	Z轴加表数据	Uint32	4	外挂陀螺仪数据
Gyro_mix	正交耦合数据	Uint32	4	外挂陀螺仪数据
Gyro_temperatrue	温度数据	Uint16	2	外挂陀螺仪数据
Gyro_counter	计数器	Uint16	2	外挂陀螺仪数据计数器 ms



















1.7.dut电源控制（CmdId=0x02）
命令功能：开启或者关闭小转台主板DUT电源
发送时机点说明：事件发送，DUT进行测试或标定时先发送开启电源，DUT停止测试或停止标定时发送关闭电源

主机发送数据：
CmdID：0x02
CmdDataSize：内容为3字节
CmdData：
成员	说明	类型	大小	备注
State	温控板状态	Uint8	1	01 为开启，00为停止
Dut_Power_enable	DUT 电源控制位	Uint16	2	DUT的电源控制位，bit0-7表示DUT1-8，Bit15表示313电源控制 0:关闭  1：开启


测试板反馈数据：
CmdID：0x8002
CmdDataSize：内容为7
CmdData：
成员	说明	类型	大小	备注
SN	主板序列号	Uint32	4	主板序列号或主板地址
State	控制状态反馈	Uint8	1	0：失败，1：成功
Dut_Power_state
	电源控制状态	Uint16	2	DUT的电源状态，bit0-7表示DUT1-8，Bit15表示313电源状态 0：关闭 1：开启



1.8.dut电压电流查询（CmdId=0x03）
命令功能：查询DUT供电电压电流
发送时机点说明：事件发送，需要查询时下发

主机发送数据：
CmdID：0x03
CmdDataSize：内容为0字节
CmdData：
成员	说明	类型	大小	备注
				
				


测试板反馈数据：
CmdID：0x8003
CmdDataSize：内容为72
CmdData：
成员	说明	类型	大小	备注
SN	主板序列号	Uint32	4	主板序列号或主板地址
主板供电电压	电压	Uint16	2	单位mV
主板供电电流	电流	Uint16	2	单位mA
DUT 1 5V供电电压	电压	Uint16	2	单位mV 
DUT 1 5V供电电流	电流	Uint16	2	单位mA
DUT 1 3.3V供电电压	电压	Uint16	2	单位mV
DUT 13.3V供电电流	电流	Uint16	2	单位mA
依次 DUT2-DUT7 电压电流数据 7*8


1.9.测试板故障状态查询（CmdId=0x04）
命令功能：查询测试板故障状态
发送时机点说明：事件发送，需要查询时下发

主机发送数据：
CmdID：0x04
CmdDataSize：内容为0字节
CmdData：
成员	说明	类型	大小	备注
				
				


测试板反馈数据：
CmdID：0x8004
CmdDataSize：内容为8
CmdData：
成员	说明	类型	大小	备注
SN	主板序列号	Uint32	4	主板序列号或主板地址
Fault state	电压	Uint32	4	Bit0-Bit31 代表不同故障，0：无故障 1：有故障 故障定义：保留
				

1.10.测试标定指令（CmdId=0x05）
命令功能：测试标定DUT
发送时机点说明：事件发送，需要标定时下发

主机发送数据：
CmdID：0x05
CmdDataSize：内容为1+n字节 (n为实际指令长度)
CmdData：
成员	说明	类型	大小	备注
Dut_sel	DUT 选择位	Uint8	1	DUT的选择为，bit0-7表示DUT1-8  0：不标定1：标定，0xFF位全部同时标定
lenght	指令长度	Uint8	1	
command	指令	Uint8	lenght	


测试板反馈数据：
CmdID：0x8005
CmdDataSize：内容为5+n 字节(n为实际写入指令长度)
CmdData：
成员	说明	类型	大小	备注
SN	主板序列号	Uint32	4	主板序列号或主板地址
State	标定状态反馈	Uint8	1	0：失败 1：成功
Command 	写入指令反馈	Uint8	n	
				

1.11.寄存器写入指令（CmdId=0x06）
命令功能：寄存器写入指令
发送时机点说明：事件发送，需要写寄存器时下发

主机发送数据：
CmdID：0x06
CmdDataSize：内容为2+n字节 (n为实际数据长度)
CmdData：
成员	说明	类型	大小	备注
Dut_sel	DUT 选择位	Uint8	1	DUT的选择位，bit0-7表示DUT1-8  0：不写入1：写入，Bit位不同时使用
Reg_addr	寄存器地址	Uint8	1	
lenght	数据长度	Uint8	1	
value	写入数据	Uint8	n	


测试板反馈数据：
CmdID：0x8006
CmdDataSize：内容为5+2+n字节 (n为实际数据长度)

CmdData：
成员	说明	类型	大小	备注
SN	主板序列号	Uint32	4	主板序列号或主板地址
State	写入状态反馈	Uint8	1	0：失败 1：成功
Reg_addr	写入寄存器地址	Uint8	1	
lenght	数据长度	Uint8	1	
value	写入寄存器读取数据	Uint8	n	
				


1.12.寄存器查询指令（CmdId=0x07）
命令功能：寄存器查询指令
发送时机点说明：事件发送，需要查询寄存器时下发

主机发送数据：
CmdID：0x07
CmdDataSize：内容为2字节 
CmdData：
成员	说明	类型	大小	备注
Dut_sel	DUT 选择位	Uint8	1	DUT的选择位，bit0-7表示DUT1-8  0：不查询1：查询，Bit位不同时使用
Reg_addr	寄存器地址	Uint8	1	
lenght	数据长度	Uint8	1	


测试板反馈数据：
CmdID：0x8007
CmdDataSize：内容为4+2+n字节 (n为实际数据长度)

CmdData：
成员	说明	类型	大小	备注
SN	主板序列号	Uint32	4	主板序列号或主板地址
Reg_addr	被查询寄存器地址	Uint8	1	
lenght	数据长度	Uint8	1	
value	寄存器读取数据	Uint8	n	
				
