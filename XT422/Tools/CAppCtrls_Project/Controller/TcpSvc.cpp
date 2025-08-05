#include "TcpSvc.h"
#include <QThread>

#define _PrintLog(_Level,fmt,...) \
	if (m_pILog) {\
		m_pILog->PrintLog(_Level, fmt, __VA_ARGS__);\
	}

CTcpSvc::CTcpSvc()
	:m_pTcpServer(NULL)
	,m_bStopThead(false)
{
}

void CTcpSvc::OnerrorOccurred(QAbstractSocket::SocketError socketError)
{
	QTcpSocket* socket = qobject_cast<QTcpSocket*>(QObject::sender());
	QString strTmp;
	strTmp = QString("[%1:%2] Soket Error:%3")
		.arg(socket->peerAddress().toString())
		.arg(socket->peerPort())
		.arg(socket->errorString());
	_PrintLog(LOGLEVEL_E, "ErrorOccurred: %s\r\n", strTmp.toStdString().c_str());
	tSocketDataHashItr Iter = m_SocketDataHash.find(socket);
	QByteArray* pSocketDataArray = NULL;
	if (Iter != m_SocketDataHash.end()) {///之前已经存在
		pSocketDataArray = Iter.value();
		delete pSocketDataArray;///删除掉这个队列
		m_SocketDataHash.erase(Iter);
		_PrintLog(LOGLEVEL_N, "Delete the socket DataArray\r\n");
	}
	else {
		_PrintLog(LOGLEVEL_N, "No Socket DataArray Need To Be Deleted\r\n");
	}
}


//server的错误信息
//如果发生错误，则serverError()返回错误的类型，
//并且可以调用errorString()以获取对所发生事件的易于理解的描述
void CTcpSvc::OnacceptError(QAbstractSocket::SocketError socketError)
{
	_PrintLog(LOGLEVEL_E, "OnacceptError: %s\r\n", m_listenServer.errorString().toStdString().c_str());
}

#define TcpSvcPacketHeadSize  (4)

void CTcpSvc::OnTckSocketReadReady()
{
	QTcpSocket* socket = qobject_cast<QTcpSocket*>(QObject::sender());
	int32_t ReceivedSize;
	tSocketDataHashItr Iter = m_SocketDataHash.find(socket);
	QByteArray* pSocketDataArray = NULL;
	if (Iter != m_SocketDataHash.end()) {///之前已经存在
		pSocketDataArray = Iter.value();
	}
	else {
		pSocketDataArray = new QByteArray();
		m_SocketDataHash.insert(socket, pSocketDataArray);
	}

	pSocketDataArray->append(socket->readAll());
	ReceivedSize = pSocketDataArray->size();
	//需要处理TCP粘包的问题
	if (ReceivedSize >= TcpSvcPacketHeadSize) {//大于TcpSvcPacketHeadSize个字节
		const char* pPacketHead = pSocketDataArray->constData();
		uint32_t JsonStrSize = *(uint32_t*)pPacketHead;  ///注意这里的头部要注意
		if (ReceivedSize >= (TcpSvcPacketHeadSize + JsonStrSize)) {///已经完成接收到一个包了，可以处理
			QByteArray JsonPacket = pSocketDataArray->left(TcpSvcPacketHeadSize + JsonStrSize);
			emit sigTcpSvcReceivePacket(socket, JsonPacket);
			pSocketDataArray->remove(0, TcpSvcPacketHeadSize + JsonStrSize);//将被处理的包数据移除
		}
	}

	_PrintLog(LOGLEVEL_N, "[ClientPort:%d]BytesRemain:%d\r\n", socket->peerPort(), pSocketDataArray->size());
}



/// <summary>
/// TcpServer有新的连接进来之后，发送newConnection信号，由本槽函数响应
/// </summary>
void CTcpSvc::OnnewConnection()
{
	while (m_pTcpServer->hasPendingConnections()){
		//nextPendingConnection返回下一个挂起的连接作为已连接的QTcpSocket对象
		//套接字是作为服务器的子级创建的，这意味着销毁QTcpServer对象时会自动删除该套接字。
		//最好在完成处理后显式删除该对象，以避免浪费内存。
		//返回的QTcpSocket对象不能从另一个线程使用，如有需要可重写incomingConnection().
		QTcpSocket* socket = m_pTcpServer->nextPendingConnection();
		m_clientList.append(socket);
		_PrintLog(LOGLEVEL_N, "[%s:%d]TcpSvc Socket Connected\r\n", socket->peerAddress().toString().toStdString().c_str(),socket->peerPort());

		connect(socket,SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this,SLOT(OnerrorOccurred(QAbstractSocket::SocketError)));
		connect(socket, &QTcpSocket::readyRead, this, &CTcpSvc::OnTckSocketReadReady);
	}
}

int32_t CTcpSvc::Setup()
{
	int32_t Ret = 0;
	bool RetCall = false;
	QHostAddress Address = QHostAddress::LocalHost;//QHostAddress::Any;
	uint16_t Port = 1030;

	m_pTcpServer = new QTcpServer();
	RetCall = m_pTcpServer->listen(Address, Port);
	if (RetCall == false) {
		_PrintLog(LOGLEVEL_E, "TcpSvc Listen Failed, ErrString: %s\r\n", m_pTcpServer->errorString().toStdString().c_str());
		Ret = ERR_TCPSVC_Listen;
		goto __end;
	}
	RetCall = connect(this->m_pTcpServer, SIGNAL(newConnection()), this, SLOT(OnnewConnection()));
	RetCall = connect(m_pTcpServer, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(OnacceptError(QAbstractSocket::SocketError)));

	_PrintLog(LOGLEVEL_N, "TcpSvc[ %s:%d ] Running...\r\n", 
				m_pTcpServer->serverAddress().toString().toStdString().c_str(), m_pTcpServer->serverPort());

__end:
	return Ret;
}


void CTcpSvc::ThreadHandler()
{
	int32_t Ret = 0;
	Ret = Setup();
	while (!m_bStopThead) {
		QThread::sleep(1);
	}
	emit sigWorkFinished(Ret);
}

/// <summary>
/// 启动TCP服务
/// </summary>
/// <returns></returns>
int32_t CTcpSvc::Start()
{
	int32_t Ret = 0;
#if 1
	Ret = Setup();

#else
	_PrintLog(LOGLEVEL_N, "TcpSvcThreaid:0x%p\r\n", QThread::currentThreadId());
	//将工作函数和QThread的信号相关联
	QObject::connect(&m_WorkThread, &QThread::started, this, &CTcpSvc::ThreadHandler);
	//将信号和QThread的exit关联
	QObject::connect(this, &CTcpSvc::sigWorkFinished, &m_WorkThread, &QThread::exit, Qt::DirectConnection);//采用直接方式避免线程主题被先释放
	this->moveToThread(&m_WorkThread);
	m_bStopThead = false;
	m_WorkThread.start();
#endif 
	return Ret;
}

/// <summary>
/// 停止TCP服务
/// </summary>
/// <returns></returns>
int32_t CTcpSvc::Stop()
{
	int32_t Ret = 0;
	int32_t Timeoutms = 3000;
	m_pTcpServer->close();
	for (QTcpSocket* socket : m_clientList){
		//断开与客户端的连接
		socket->disconnectFromHost();
		if (socket->state() != QAbstractSocket::UnconnectedState) {
			socket->abort();
		}
	}

	tSocketDataHashItr Itr = m_SocketDataHash.begin();
	while (Itr != m_SocketDataHash.end()) {
		delete Itr.value(); ///删除new对应的QByteArray
		Itr++;
	}
	m_SocketDataHash.clear();

	m_bStopThead = true;
	m_WorkThread.wait(Timeoutms);
	return Ret;
}

