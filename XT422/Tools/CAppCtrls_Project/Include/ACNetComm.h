#pragma once

#include "IACComm.h"
#include "ICmdHandler.h"
#include "ILog.h"
#include "ACError.h"
#include <QThread>

#include <stdio.h>
#include <winsock2.h>
#include <Windows.h>

#pragma comment(lib,"ws2_32.lib")

typedef struct _tIPInfo{
	QString Addr;
	uint16_t Port;
}tIPInfo;


typedef struct {
	int32_t RecvBufSize;   //接收的Buffer大小
	int32_t SendBufSize;   //发送的Buf大小
	int32_t RecvTimeoutms; //发送超时时间单位毫秒
	int32_t SendTimeoutms; //接收超时时间单位毫秒
	bool bReuseaddr;
	int32_t ZeroCopy;	  //是否不要进行Socket的拷贝，设置为0表示不经过Socket到缓存区的拷贝
}tSocketOpts;


class CNetComm : public IACComm
{
	Q_OBJECT
public:
	CNetComm();
	void AttachICmdHandler(ICmdHandler* pCmdHandler);

	//如果有需要下面的Set函数需要被先调用，用来设置变量，保证StartNetComm的运行
	int32_t SetSocketOpts(tSocketOpts* pSocketOpts);
	int32_t SetIPInfo(tIPInfo*pRemote, tIPInfo*pLocal);

	int32_t StartComm();
	int32_t StopComm();

	int32_t SendData(uint8_t* pData, int32_t Size);

public slots:
	void ThreadHandler();
	void ThreadExit(int RetCode);

protected:
	//设置网络操作相关的配置
	int32_t SetupNetWork();
	//清除网络相关配置
	int32_t CleanNetWork();

	//处理命令消息
	int32_t HandleMsg(uint8_t* PckData, int32_t PckSize);
signals:
	void sigWorkFinished(int RetCode);
	void Error(QString err);

private:
	volatile bool m_bStopThead;
	QThread m_WorkThread;
	tSocketOpts m_SocketOpts;
	SOCKET m_LocalSocket;
	tIPInfo m_Remote;
	tIPInfo m_Local;

	SOCKADDR_IN m_LocalAddr;
	SOCKADDR_IN m_RemoteAddr;


	ICmdHandler* m_pCmdHandler;
};



