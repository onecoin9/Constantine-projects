#ifndef _APPCTRL_H_
#define _APPCTRL_H_

#include "CmdHandler.h"
#include "ACNetComm.h"
#include "AppModels.h"
#include "TcpSvc.h"
#include "IRPCSender.h"
#include "IDataWriter.h"
#include "ACCmdHandler.h"
#include "UplinkDataReceiver.h"

#define DefineExecRPCMethod(_MethodName) \
	int32_t ExecRPCMethod_##_MethodName(IRPCSender* pSender, QString strMethod, tRPCParamsHash& ParamsHash, tRPCParamsHash& ResultHash, QString& ErrMessage);

#define DefineExecDevCallBack(_CallBackFunc) \
	int32_t DevCallBack_##_CallBackFunc(uint32_t HopNum, uint32_t PortID, uint32_t CmdFlag, uint16_t CmdID, QByteArray CmdDataBytes, QByteArray& RespDataBytes);

class CAppCtrls :public QObject
{
	Q_OBJECT
public:
	/// <summary>
	/// 具体执行RPC客户端的函数
	/// </summary>
	/// <param name="pSender">RPC发送客户端</param>
	/// <param name="strMethod">请求执行的方法</param>
	/// <param name="ParamsHash">方法的参数列表</param>
	/// <param name="ResultHash">结果的参数列表，如果有的话需要填充，不同的命令结果不同，根据对应的命令说明解析</param>
	/// <param name="ErrMessage">如果出现错误请填充</param>
	/// <returns>小于0表示失败，ErrMessage需要填充，大于等于0表示成功，根据对应的命令说明解析，</returns>
	typedef int32_t (CAppCtrls::*FnExecRPCMethod)(IRPCSender* pSender, QString strMethod, tRPCParamsHash& ParamsHash, tRPCParamsHash& ResultHash, QString& ErrMessage);

	/// <summary>
	/// 执行来自AG06的回调
	/// </summary>
	/// <param name="HopNum">对应的AG06的HopNum号</param>
	/// <param name="PortID">对应的PortID，为0表示MU发送为其他表示BPU发送</param>
	/// <param name="CmdFlag">对应的CmdID的标识</param>
	/// <param name="CmdID">请求执行的命令码</param>
	/// <param name="CmdDataBytes">命令码对应的命令数据</param>
	/// <param name="RespDataBytes">如果AG06请求的这个CmdID需要有响应数据，则把响应数据放到这个ByteArray中</param>
	/// <returns>命令执行结果，0表示成功，负数表示失败</returns>
	typedef int32_t (CAppCtrls::* FnExecDevCallBack)(uint32_t HopNum, uint32_t PortID, uint32_t CmdFlag, uint16_t CmdID, QByteArray CmdDataBytes,QByteArray& RespDataBytes);

	CAppCtrls();
	void AttachILog(ILog* pILog) {
		m_pILog = pILog;
	};
	void AttachAppModels(CAppModels* pAppModels);
	int32_t RunCtrls();
	int32_t StopCtrls();

public slots:
	void OnTcpSvcReceivePacket(QTcpSocket* pFromSocket, QByteArray JsonPacket);
	//当AG06完成RemoteDoCmd命令之后，如果没有通过RegistCbkCmdComplete注册回调函数，则会发送sigCmdComplete信号
	void OnRemoteCmdComplete(uint32_t HopNum, uint32_t PortID, int32_t ResultCode, QByteArray RespDataBytes);
	void OnRemoteQueryDoCmd(uint32_t HopNum, uint32_t PortID, uint32_t CmdFlag, uint16_t CmdID, QByteArray CmdDataBytes);
protected:
	//根据设定翻译
	QString TranslateFilePath(QString strFilePathOrg);
	/// <summary>
	/// 具体执行RPC客户端的函数
	/// </summary>
	/// <param name="pSender">RPC发送客户端</param>
	/// <param name="strMethod">请求执行的方法</param>
	/// <param name="ParamsHash">方法的参数列表</param>
	/// <param name="ResultHash">结果的参数列表，如果有的话需要填充，不同的命令结果不同，根据对应的命令说明解析</param>
	/// <param name="ErrMessage">如果出现错误请填充</param>
	/// <returns>小于0表示失败，ErrMessage需要填充，大于等于0表示成功，根据对应的命令说明解析，</returns>
	
	/*请求AG06执行的命令*/
	DefineExecRPCMethod(LinkScan)		//链路扫描包
	DefineExecRPCMethod(GetCapacity)	//获取容量
	DefineExecRPCMethod(SendData)		//发送数据
	DefineExecRPCMethod(ReadData)		//读取数据
	DefineExecRPCMethod(SendPTCmd)		//发送透传命令给到MU或BPU
	DefineExecRPCMethod(SSD2DDR)		//将AG06的SSD中的数据读取到AG06的DDR
	DefineExecRPCMethod(DDR2SSD)        //将AG06的DDR数据写入到SSD中

	/// <summary>
	/// 注册JsonRPC的方法对应的处理函数
	/// </summary>
	/// <param name="strMethod">客户端请求执行的方法</param>
	/// <param name="fnMethodFunc">方法对应的处理函数</param>
	/// <returns></returns>
	int32_t RegistRPCMethodHandler(QString strMethod, FnExecRPCMethod fnMethodFunc);
	int32_t RegistAllRPCMethod();
	int32_t ExecJsonRPCMethod(IRPCSender* pSender,const char* strJson);
	int32_t ExecRPCMethod(IRPCSender* pSender, QString strMethod, QJsonValue Id, tRPCParamsHash& ParamsHash);
	QByteArray ConstructJsonRPCResponse(QJsonValue Id, int32_t RetCode, tRPCParamsHash& ResultHash, QString& ErrMessage);
	int32_t SendRPCRespnseWithoutDoingMethod(IRPCSender* pSender, int32_t ErrCode, QString ErrMessage);

	/*AG06的回调相关函数*/
	int32_t RegistDevCallbakFunc(quint16 CmdID, FnExecDevCallBack fnCallBackFunc);
	int32_t RegistAllDevCallbackFunc();
	/*AG06请求PC执行的命令*/
	DefineExecDevCallBack(SetLog)		//设置日志
	DefineExecDevCallBack(SetProgress)	//设置进度
	DefineExecDevCallBack(ReadBuffData) //读取Buffer数据

	/*
	* 使用如下三个函数进行Device返回的数据的保存操作
	* 先调用StoreDataWriter设置写操作指针，当Device的数据到达之后会调用WriteDataByDataWriter进行写入，完成之后调用StoreDataWriter还原回去
	*/
	/// <summary>
	/// 设置新的指针，返回旧指针
	/// </summary>
	//IDataWriter* StoreDataWriter(IDataWriter* pNewIDataWriter);
	/// <summary>
	/// 写操作，pData为写入的数据，Size为实际的字节数，返回实际写入的字节数，小于0表示失败
	/// </summary>
	//qint64 WriteDataByDataWriter(char* pData, qint64 Size);
	//qint64 GetDataWriterCurDataSize();
private:
	ILog* m_pILog;
	CAppModels* m_pAppModels;
	CCmdHandler m_CmdHandler;
	CACCmdHandler m_ACCmdHandler;
	CNetComm m_NetComm;
	CTcpSvc m_TcpSvc;
	QHash<QString, CAppCtrls::FnExecRPCMethod> m_RPCMethodHash; ///<存放RPC Method和对应处理函数的Hash， QString为对应的函数名称，FnExecRPCMethod执行的函数体
	QHash<quint16, CAppCtrls::FnExecDevCallBack> m_DevCallbackFuncHash; ///<存放AG06请求PC处理的回调函数Hash， qint16为请求执行的命令码，FnExecDevCallBack执行的函数体
};


#endif 