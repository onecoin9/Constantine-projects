#include "infrastructure/TcpChannel.h"
#include "core/Logger.h"
#include <QMutexLocker>
#include <QTcpSocket> // 仅用于 sendToPort
#include <QJsonDocument>
#include <QJsonObject>
#include <QtConcurrent>

namespace Infrastructure {

namespace { // Anonymous namespace for local helper function
bool isKnownJsonCommand(const QByteArray& data) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isNull() && doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("Command")) {
            QString command = obj["Command"].toString();
            // 仅记录与AxisMove相关的和未来cmd4的JSON指令
            if (command.startsWith("AxisMove") || command.startsWith("cmd4")) {
                return true;
            }
        }
    }
    return false;
}
}

// 单例实现
TcpChannel* TcpChannel::s_instance = nullptr;
QMutex TcpChannel::s_mutex;

TcpChannel* TcpChannel::getInstance()
{
    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new TcpChannel();
    }
    return s_instance;
}

void TcpChannel::destroyInstance()
{
    QMutexLocker locker(&s_mutex);
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

TcpChannel::TcpChannel(QObject *parent)
    : ICommunicationChannel(parent)
    , m_host("127.0.0.1")
    , m_port(0)
    , m_connectTimeout(5000)
    , m_status(Status::Disconnected)
{
    // 构造函数现在为空，因为 m_socket 已被移除
    LOG_MODULE_DEBUG("TcpChannel", "TcpChannel singleton instance created (short-connection mode).");
}

TcpChannel::~TcpChannel()
{
    // 析构函数也为空
    LOG_MODULE_DEBUG("TcpChannel", "TcpChannel singleton instance destroyed.");
}

// --- ICommunicationChannel 存根实现 ---

bool TcpChannel::open()
{
    LOG_MODULE_WARNING("TcpChannel", "open() is not supported in this short-connection-only channel.");
    return false;
}

void TcpChannel::close()
{
    // 不需要任何操作
}

bool TcpChannel::isOpen() const
{
    // 总是返回 false，因为没有持久连接
    return false;
}

ICommunicationChannel::Status TcpChannel::getStatus() const
{
    // 返回一个固定的状态，因为它没有持久连接
    return Status::Disconnected;
}

qint64 TcpChannel::send(const QByteArray &data)
{
    LOG_MODULE_WARNING("TcpChannel", "send() is not supported. Use sendToPort() instead.");
    Q_UNUSED(data);
    return -1;
}

QByteArray TcpChannel::receive(int timeoutMs)
{
    LOG_MODULE_WARNING("TcpChannel", "receive() is not supported in this short-connection-only channel.");
    Q_UNUSED(timeoutMs);
    return QByteArray();
}

void TcpChannel::clearBuffer()
{
    // 不需要任何操作
}

void TcpChannel::setParameters(const QVariantMap &params)
{
    QMutexLocker locker(&m_mutex);
    
    if (params.contains("host")) {
        m_host = params["host"].toString();
    }
    if (params.contains("port")) {
        m_port = params["port"].toInt();
    }
    if (params.contains("timeout")) {
        m_connectTimeout = params["timeout"].toInt();
    }
    
    LOG_MODULE_DEBUG("TcpChannel", QString("Parameters set for short-connection mode: default host=%1, default port=%2, timeout=%3ms")
                    .arg(m_host).arg(m_port).arg(m_connectTimeout).toStdString());
}

QVariantMap TcpChannel::getParameters() const
{
    QMutexLocker locker(&m_mutex);
    
    QVariantMap params;
    params["host"] = m_host;
    params["port"] = m_port;
    params["timeout"] = m_connectTimeout;
    params["status"] = static_cast<int>(m_status);
    
    return params;
}
bool TcpChannel::sendToPortAsync(const QByteArray &data, int port, const QString &host, int timeout, QByteArray* responseDataOut)
{
    if (responseDataOut) {
        responseDataOut->clear();
    }

    // 使用 QtConcurrent 在线程池中运行阻塞发送逻辑
    return QtConcurrent::run([this, data, port, host, timeout, responseDataOut]() -> bool {
        // 为短连接创建临时socket
        QTcpSocket tempSocket;
        
        // 设置更短的连接超时时间（最大1秒），避免UI卡死
        int actualTimeout = qMin(timeout, 1000);
        
        LOG_MODULE_INFO("TcpChannel", QString("==> [BEGIN] Short connection to %1:%2").arg(host).arg(port).toStdString());
        LOG_MODULE_DEBUG("TcpChannel", QString("Attempting to connect with timeout: %1ms").arg(actualTimeout).toStdString());
        
        tempSocket.connectToHost(host, port);
        
        if (!tempSocket.waitForConnected(actualTimeout)) {
            QString errorString = tempSocket.errorString();
            m_lastError = QString("Failed to connect to %1:%2 - %3")
                         .arg(host).arg(port).arg(errorString);
            
            // 根据错误类型调整日志级别，提供更明确的指引
            if (tempSocket.error() == QAbstractSocket::ConnectionRefusedError) {
                LOG_MODULE_WARNING("TcpChannel", QString("Connect WARNING: Connection refused by %1:%2. Is the target service running and listening?").arg(host).arg(port).toStdString());
            } else {
                LOG_MODULE_ERROR("TcpChannel", QString("Connect FAILED: %1").arg(m_lastError).toStdString());
            }
            LOG_MODULE_INFO("TcpChannel", QString("==> [END] Short connection to %1:%2 (Connection Failed)").arg(host).arg(port).toStdString());
            return false;
        }
        LOG_MODULE_INFO("TcpChannel", QString("Connect SUCCESS: Connection established to %1:%2").arg(host).arg(port).toStdString());
        
        // 记录发送前的数据内容用于调试
        LOG_MODULE_INFO("TcpChannel", QString("TX -> Sending %1 bytes to %2:%3").arg(data.size()).arg(host).arg(port).toStdString());
        
        qint64 bytesWritten = tempSocket.write(data);
        
        // 立即强制发送数据
        if (!tempSocket.flush()) {
            LOG_MODULE_WARNING("TcpChannel", "Socket flush failed");
        }
        
        // 等待应答
        if (!tempSocket.waitForReadyRead(timeout)) {
            m_lastError = QString("No response received from %1:%2 - read timeout").arg(host).arg(port);
            LOG_MODULE_WARNING("TcpChannel", m_lastError.toStdString());
            //即使没有应答，也认为发送成功，因为某些命令可能没有应答
        } else {
            QByteArray response = tempSocket.readAll();
            LOG_MODULE_INFO("TcpChannel", QString("RX <- Received %1 bytes from %2:%3").arg(response.size()).arg(host).arg(port).toStdString());
            if (responseDataOut) {
                *responseDataOut = response;
            }
        }
        
        LOG_MODULE_INFO("TcpChannel", QString("Successfully wrote %1 bytes").arg(bytesWritten).toStdString());
        
        bool success = (bytesWritten == data.size());
        if (success) {
            LOG_MODULE_INFO("TcpChannel", QString("Short connection sent %1 bytes to %2:%3")
                           .arg(bytesWritten).arg(host).arg(port).toStdString());
        } else {
            m_lastError = QString("Failed to send data to %1:%2 - only %3/%4 bytes written")
                         .arg(host).arg(port).arg(bytesWritten).arg(data.size());
            LOG_MODULE_ERROR("TcpChannel", m_lastError.toStdString());
        }
        
        tempSocket.disconnectFromHost();
        LOG_MODULE_INFO("TcpChannel", QString("==> [END] Short connection to %1:%2 (Success)").arg(host).arg(port).toStdString());
        return success;
    });
}
bool TcpChannel::sendToPort(const QByteArray &data, int port, const QString &host, int timeout, QByteArray* responseDataOut)
{
    if (responseDataOut) {
        responseDataOut->clear();
    }

    // 为短连接创建临时socket
    QTcpSocket tempSocket;
    
    // 设置更短的连接超时时间（最大1秒），避免UI卡死
    int actualTimeout = qMin(timeout, 1000);
    
    LOG_MODULE_INFO("TcpChannel", QString("==> [BEGIN] Short connection to %1:%2").arg(host).arg(port).toStdString());
    LOG_MODULE_DEBUG("TcpChannel", QString("Attempting to connect with timeout: %1ms").arg(actualTimeout).toStdString());
    
    tempSocket.connectToHost(host, port);
    
    if (!tempSocket.waitForConnected(actualTimeout)) {
        QString errorString = tempSocket.errorString();
        m_lastError = QString("Failed to connect to %1:%2 - %3")
                     .arg(host).arg(port).arg(errorString);
        
        // 根据错误类型调整日志级别，提供更明确的指引
        if (tempSocket.error() == QAbstractSocket::ConnectionRefusedError) {
            LOG_MODULE_WARNING("TcpChannel", QString("Connect WARNING: Connection refused by %1:%2. Is the target service running and listening?").arg(host).arg(port).toStdString());
        } else {
            LOG_MODULE_ERROR("TcpChannel", QString("Connect FAILED: %1").arg(m_lastError).toStdString());
        }
        LOG_MODULE_INFO("TcpChannel", QString("==> [END] Short connection to %1:%2 (Connection Failed)").arg(host).arg(port).toStdString());
        return false;
    }
    LOG_MODULE_INFO("TcpChannel", QString("Connect SUCCESS: Connection established to %1:%2").arg(host).arg(port).toStdString());
    
    // 记录发送前的数据内容用于调试
    LOG_MODULE_INFO("TcpChannel", QString("TX -> Sending %1 bytes to %2:%3").arg(data.size()).arg(host).arg(port).toStdString());
    
    qint64 bytesWritten = tempSocket.write(data);
    
    // 立即强制发送数据
    if (!tempSocket.flush()) {
        LOG_MODULE_WARNING("TcpChannel", "Socket flush failed");
    }
    
    // 等待应答
    if (!tempSocket.waitForReadyRead(timeout)) {
        m_lastError = QString("No response received from %1:%2 - read timeout").arg(host).arg(port);
        LOG_MODULE_WARNING("TcpChannel", m_lastError.toStdString());
        //即使没有应答，也认为发送成功，因为某些命令可能没有应答
    } else {
        QByteArray response = tempSocket.readAll();
        LOG_MODULE_INFO("TcpChannel", QString("RX <- Received %1 bytes from %2:%3").arg(response.size()).arg(host).arg(port).toStdString());
        if (responseDataOut) {
            *responseDataOut = response;
        }
    }
    
    LOG_MODULE_INFO("TcpChannel", QString("Successfully wrote %1 bytes").arg(bytesWritten).toStdString());
    
    bool success = (bytesWritten == data.size());
    if (success) {
        LOG_MODULE_INFO("TcpChannel", QString("Short connection sent %1 bytes to %2:%3")
                       .arg(bytesWritten).arg(host).arg(port).toStdString());
    } else {
        m_lastError = QString("Failed to send data to %1:%2 - only %3/%4 bytes written")
                     .arg(host).arg(port).arg(bytesWritten).arg(data.size());
        LOG_MODULE_ERROR("TcpChannel", m_lastError.toStdString());
    }
    
    tempSocket.disconnectFromHost();
    LOG_MODULE_INFO("TcpChannel", QString("==> [END] Short connection to %1:%2 (Success)").arg(host).arg(port).toStdString());
    return success;
}

} // namespace Infrastructure
