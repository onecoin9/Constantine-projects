#include "infrastructure/TcpServerChannel.h"
#include "core/Logger.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QMutexLocker>

namespace Infrastructure {

// 单例实现
TcpServerChannel* TcpServerChannel::s_instance = nullptr;
QMutex TcpServerChannel::s_mutex;

TcpServerChannel* TcpServerChannel::getInstance()
{
    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new TcpServerChannel();
    }
    return s_instance;
}

void TcpServerChannel::destroyInstance()
{
    QMutexLocker locker(&s_mutex);
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

TcpServerChannel::TcpServerChannel(QObject *parent)
    : ICommunicationChannel(parent)
    , m_server(std::make_unique<QTcpServer>(this))
    , m_clientSocket(nullptr)
    , m_bindAddress("127.0.0.1")
    , m_port(64101)
    , m_actualPort(0)
    , m_maxConnections(1)
    , m_connectionTimeout(30000)
    , m_status(Status::Disconnected)
{
    // 连接服务器信号
    connect(m_server.get(), &QTcpServer::newConnection,
            this, &TcpServerChannel::onNewConnection);
    connect(m_server.get(), &QTcpServer::acceptError,
            this, &TcpServerChannel::onAcceptError);
    
    LOG_MODULE_DEBUG("TcpServerChannel", "TcpServerChannel singleton instance created");
}

TcpServerChannel::~TcpServerChannel()
{
    close();
    LOG_MODULE_DEBUG("TcpServerChannel", "TcpServerChannel singleton instance destroyed");
}

bool TcpServerChannel::open()
{
    if (m_status == Status::Connected && m_server->isListening()) {
        return true;
    }

    QHostAddress address(m_bindAddress);
    if (!m_server->listen(address, m_port)) {
        m_lastError = QString("Failed to start server on %1:%2 - %3")
                     .arg(m_bindAddress).arg(m_port).arg(m_server->errorString());
        LOG_MODULE_ERROR("TcpServerChannel", m_lastError.toStdString());
        setStatus(Status::Error);
        return false;
    }

    m_actualPort = m_server->serverPort();
    LOG_MODULE_INFO("TcpServerChannel", QString("Server listening on %1:%2")
                   .arg(m_bindAddress).arg(m_actualPort).toStdString());

    setStatus(Status::Connected);
    return true;
}

void TcpServerChannel::close()
{
    // 断开客户端连接
    if (m_clientSocket) {
        disconnect(m_clientSocket, nullptr, this, nullptr);
        
        if (m_clientSocket->state() != QAbstractSocket::UnconnectedState) {
            m_clientSocket->disconnectFromHost();
        }
        m_clientSocket->deleteLater();
        m_clientSocket = nullptr;
    }

    // 停止服务器监听
    if (m_server && m_server->isListening()) {
        m_server->close();
        LOG_MODULE_INFO("TcpServerChannel", "Server stopped");
    }

    // 清理缓冲区
    {
        QMutexLocker locker(&m_dataMutex);
        m_readBuffer.clear();
    }

    setStatus(Status::Disconnected);
}

bool TcpServerChannel::isOpen() const
{
    return m_status == Status::Connected && m_clientSocket && 
           m_clientSocket->state() == QAbstractSocket::ConnectedState;
}

qint64 TcpServerChannel::send(const QByteArray& data)
{
    if (!m_clientSocket || m_clientSocket->state() != QAbstractSocket::ConnectedState) {
        m_lastError = "No client connected to server";
        LOG_MODULE_ERROR("TcpServerChannel", m_lastError.toStdString());
        return -1;
    }

    qint64 bytesWritten = m_clientSocket->write(data);
    if (bytesWritten > 0) {
        m_clientSocket->flush();
        LOG_MODULE_DEBUG("TcpServerChannel", QString("Server sent %1 bytes").arg(bytesWritten).toStdString());
    } else {
        m_lastError = QString("Failed to send data: %1").arg(m_clientSocket->errorString());
        LOG_MODULE_ERROR("TcpServerChannel", m_lastError.toStdString());
    }

    return bytesWritten;
}

ICommunicationChannel::Status TcpServerChannel::getStatus() const
{
    return m_status;
}

QByteArray TcpServerChannel::receive(int timeoutMs)
{
    Q_UNUSED(timeoutMs);
    
    QMutexLocker locker(&m_dataMutex);
    
    if (m_readBuffer.isEmpty()) {
        return QByteArray();
    }

    QByteArray data = m_readBuffer;
    m_readBuffer.clear();
    return data;
}

void TcpServerChannel::clearBuffer()
{
    QMutexLocker locker(&m_dataMutex);
    m_readBuffer.clear();
}

void TcpServerChannel::setParameters(const QVariantMap& params)
{
    if (params.contains("bindAddress")) {
        m_bindAddress = params["bindAddress"].toString();
    }
    
    if (params.contains("port")) {
        m_port = params["port"].toInt();
    }
    
    if (params.contains("maxConnections")) {
        m_maxConnections = params["maxConnections"].toInt();
    }
    
    if (params.contains("connectionTimeout")) {
        m_connectionTimeout = params["connectionTimeout"].toInt();
    }
    
    LOG_MODULE_INFO("TcpServerChannel", QString("Parameters set: bind=%1:%2, maxConn=%3, timeout=%4ms")
                   .arg(m_bindAddress).arg(m_port).arg(m_maxConnections).arg(m_connectionTimeout).toStdString());
}

QVariantMap TcpServerChannel::getParameters() const
{
    QVariantMap params;
    params["bindAddress"] = m_bindAddress;
    params["port"] = m_port;
    params["actualPort"] = m_actualPort;
    params["maxConnections"] = m_maxConnections;
    params["connectionTimeout"] = m_connectionTimeout;
    params["isListening"] = m_server->isListening();
    params["hasClient"] = (m_clientSocket != nullptr);
    params["status"] = static_cast<int>(m_status);
    return params;
}

bool TcpServerChannel::isListening() const
{
    return m_server->isListening();
}

int TcpServerChannel::getPort() const
{
    return m_port;
}

int TcpServerChannel::getActualPort() const
{
    return m_actualPort;
}

QString TcpServerChannel::getBindAddress() const
{
    return m_bindAddress;
}

bool TcpServerChannel::hasClient() const
{
    return m_clientSocket != nullptr && isOpen();
}

QString TcpServerChannel::getClientInfo() const
{
    if (!m_clientSocket) {
        return "No client connected";
    }
    
    return QString("%1:%2").arg(m_clientSocket->peerAddress().toString())
                          .arg(m_clientSocket->peerPort());
}

void TcpServerChannel::onNewConnection()
{
    QTcpSocket* newClient = m_server->nextPendingConnection();
    if (!newClient) {
        return;
    }

    // 如果已经有客户端连接且只允许一个连接，关闭新连接
    if (m_clientSocket && m_maxConnections == 1) {
        LOG_MODULE_WARNING("TcpServerChannel", QString("Rejecting new client %1:%2 - already have active connection")
                          .arg(newClient->peerAddress().toString())
                          .arg(newClient->peerPort()).toStdString());
        newClient->disconnectFromHost();
        newClient->deleteLater();
        return;
    }

    // 如果有旧的客户端，先断开
    if (m_clientSocket) {
        LOG_MODULE_INFO("TcpServerChannel", "Replacing existing client connection");
        disconnect(m_clientSocket, nullptr, this, nullptr);
        m_clientSocket->disconnectFromHost();
        m_clientSocket->deleteLater();
    }

    // 设置新的客户端连接
    m_clientSocket = newClient;

    // 连接客户端信号
    connect(m_clientSocket, &QTcpSocket::readyRead,
            this, &TcpServerChannel::onClientDataReady);
    connect(m_clientSocket, &QTcpSocket::disconnected,
            this, &TcpServerChannel::onClientDisconnected);
    connect(m_clientSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &TcpServerChannel::onClientError);

    // 设置超时定时器
    if (m_connectionTimeout > 0) {
        QTimer::singleShot(m_connectionTimeout, this, [this, newClient]() {
            if (m_clientSocket == newClient && 
                m_clientSocket->state() != QAbstractSocket::ConnectedState) {
                LOG_MODULE_WARNING("TcpServerChannel", "Client connection timeout");
                onClientDisconnected();
            }
        });
    }

    QString clientInfo = getClientInfo();
    LOG_MODULE_INFO("TcpServerChannel", QString("Client connected from %1").arg(clientInfo).toStdString());

    setStatus(Status::Connected);
    emit clientConnected(clientInfo);
}

void TcpServerChannel::onAcceptError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    QString errorMsg = QString("Accept error: %1").arg(m_server->errorString());
    LOG_MODULE_ERROR("TcpServerChannel", errorMsg.toStdString());
    setStatus(Status::Error);
    emit errorOccurred(errorMsg);
}

void TcpServerChannel::onClientDataReady()
{
    if (!m_clientSocket) {
        return;
    }

    QByteArray data = m_clientSocket->readAll();
    if (!data.isEmpty()) {
        {
            QMutexLocker locker(&m_dataMutex);
            m_readBuffer.append(data);
        }
        emit dataReceived(data);
    }
}

void TcpServerChannel::onClientDisconnected()
{
    if (m_clientSocket) {
        QString clientInfo = getClientInfo();
        
        LOG_MODULE_INFO("TcpServerChannel", QString("Client disconnected: %1. Server remains listening.").arg(clientInfo).toStdString());
        
        disconnect(m_clientSocket, nullptr, this, nullptr);
        m_clientSocket->deleteLater();
        m_clientSocket = nullptr;
        
        emit clientDisconnected(clientInfo);
    }
    
    // 服务器继续监听，状态保持Connected（监听状态）
    // 只有在客户端错误时才改变状态
}

void TcpServerChannel::onClientError(QAbstractSocket::SocketError error)
{
    // 正常断开不算错误
    if (error == QAbstractSocket::RemoteHostClosedError) {
        LOG_MODULE_DEBUG("TcpServerChannel", "Client closed the connection gracefully.");
        return;
    }

    if (m_clientSocket) {
        QString errorMsg = QString("Client error: %1").arg(m_clientSocket->errorString());
        LOG_MODULE_ERROR("TcpServerChannel", errorMsg.toStdString());
        
        QString clientInfo = getClientInfo();
        emit clientError(clientInfo, errorMsg);
        emit errorOccurred(errorMsg);
        
        // 如果是严重错误，断开客户端
        onClientDisconnected();
    }
}

void TcpServerChannel::setStatus(Status newStatus)
{
    if (m_status != newStatus) {
        Status oldStatus = m_status;
        m_status = newStatus;
        
        LOG_MODULE_DEBUG("TcpServerChannel", QString("Status changed from %1 to %2")
                        .arg(static_cast<int>(oldStatus))
                        .arg(static_cast<int>(newStatus)).toStdString());
        
        emit statusChanged(m_status);
    }
}

} // namespace Infrastructure 
