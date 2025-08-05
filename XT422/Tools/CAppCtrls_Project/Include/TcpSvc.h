#ifndef _TCPSVC_H_
#define _TCPSVC_H_

#include "ACError.h"
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QList>
#include <QByteArray>
#include "ILog.h"
#include <QThread>

#define JsonRPC_ErrorCode_ParseErr				(-32700)	//Parse error
#define JsonRPC_ErrorCode_Methodnotfound		(-32601)    //Method not found
	
typedef QHash<QTcpSocket*, QByteArray*> tSocketDataHash;
typedef QHash<QTcpSocket*, QByteArray*>::iterator tSocketDataHashItr;
/// <summary>
/// 本类用来接收外部的命令输入，通过TCP协议外部程序可以控制本程序的动作
/// </summary>
class CTcpSvc : public QObject
{
	Q_OBJECT
public:
	CTcpSvc();
	void AttachILog(ILog* pLog) {
		m_pILog = pLog;
	};
	int32_t Start();
	int32_t Stop();

public slots:
	//TcpServer信号对应的槽
	void OnnewConnection();
	void OnacceptError(QAbstractSocket::SocketError socketError);

	//TcpSocket信号对应的槽
	void OnerrorOccurred(QAbstractSocket::SocketError socketError);
	void OnTckSocketReadReady();

	void ThreadHandler();

protected:
	int32_t Setup();

signals:
	void sigTcpSvcReceivePacket(QTcpSocket* pFromSocket,QByteArray JsonPacket);
	void sigWorkFinished(int RetCode);

private:
	QTcpServer  m_listenServer;
	QTcpServer* m_pTcpServer;
	QTcpSocket m_TcpSocket;
	QList<QTcpSocket*> m_clientList; ///允许多个客户端同时连接进来
	tSocketDataHash m_SocketDataHash; //每个客户端都对应一个Received Array
	ILog* m_pILog;
	QThread m_WorkThread;
	volatile bool m_bStopThead;
	
};	

#endif 