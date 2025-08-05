#ifndef ACNETCOMM_H
#define ACNETCOMM_H

#include "IACComm.h"
#include "ICmdHandler.h"
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

	int32_t SendData(std::string devIP, uint8_t* pData, int32_t Size);

public slots:
	void ThreadHandler();
	void ThreadExit(int RetCode);

protected:
	//设置网络操作相关的配置
	int32_t SetupNetWork();
	//清除网络相关配置
	int32_t CleanNetWork();

	//处理命令消息
	int32_t HandleMsg(std::string recvDevIP, uint8_t* PckData, int32_t PckSize);
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
#endif // !ACNETCOMM_H


