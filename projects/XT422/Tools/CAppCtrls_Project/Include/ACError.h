#ifndef _ACERROR_H_
#define _ACERROR_H_

#include <QString>

#define ERR_MemoryAlloc						(-100)			//内存分配错误
#define ERR_DataPtrNull						(-101)			//数据指针为NULL	
#define ERR_ReadFile						(-102)			//读取文件出错

#define ERR_NETCOMM_WSAStartup				(-1000)			//Device通信时Windows Sockets环境初始化错误
#define ERR_NETCOMM_CreateSocket			(-1001)			//Device通信时创建Socket错误
#define ERR_NETCOMM_SetSktOpts				(-1002)			//Device通信时设置Socket的Option属性错误
#define ERR_NETCOMM_SocketInvalid			(-1003)			//Device通信时Socket无效
#define ERR_NETCOMM_SendData				(-1004)			//Device通信时发送数据错误
#define ERR_NETCOMM_BindSocket				(-1005)			//Device通信时绑定Socket错误
#define ERR_NETCOMM_PckSize					(-1006)			//Device通信时通信包大小错误
#define ERR_NETCOMM_HopNumError				(-1007)			//Device通信时HopNum不对
#define ERR_NETCOMM_CmdACKTimeout			(-1008)			//Device通信时发送的命令包的ACK接收超时
#define ERR_NETCOMM_CmdCRCGetTimeout		(-1009)			//Device通信时等待CRC32的Complete包超时
#define ERR_NETCOMM_PTPACKTYPE				(-1010)			//透传包CmdID错误
#define ERR_NETCOMM_CMDIDNOSUPPORT			(-1026)			//CmdID没有被支持


#define ERR_CMDHAND_CmdNotSupport			(-1100)		//命令处理模块交互命令不支持
#define ERR_CMDHAND_SubCmdNotSupport		(-1101)		//命令处理模块交互命令的子命令不支持
#define ERR_CMDHAND_CmdQueueAvailable		(-1102)		//命令处理模块等待命令队列可用时超时
#define ERR_CMDHAND_HopNum					(-1103)		//HopNum超出的大小

#define ERR_MODEL_CapacityType				(-1200)		//Device交互时，容量类型错误


#define ERR_TCPSVC_Listen					(-1300)		//TcpSvc服务器监听错误

#define ERR_JSONRPC_Parser					(-1400)		//JsonRPC解析错误
#define ERR_JSONRPC_NeedMethod				(-1401)		//JsonRPC需要指定method
#define ERR_JSONRPC_NeedJsonRpc				(-1402)     //JsonRPC需要指定jsonrpc
//#define ERR_JSON_ParamsObject				(-1403)		//JsonRPC参数对象错误
#define ERR_JSONRPC_MethodNotSupport		(-1404)		//JsonRPC方法不支持
#define ERR_JSONRPC_ParaType				(-1405)		//JsonRPC参数类型错误
#define ERR_JSONRPC_SendData				(-1406)		//JsonRPC发送数据错误
#define ERR_JSONRPC_ParaInvalid				(-1407)		//JsonRPC发送的参数无效

#define ERR_JSONRPC_FileOpen				(-1408)		//JsonRPC执行打开文件失败
#define ERR_JSONRPC_ParamError				(-1409)		//JsonRPC方法所带的参数错误

#define ERR_CONFIG_LoadIni					(-1500)		//Config加载ini错误



class CACError
{
public:
	static QString GetErrMessage(qint32 Errcode);
};
#endif 