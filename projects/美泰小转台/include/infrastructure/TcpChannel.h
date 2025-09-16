#ifndef TCPCHANNEL_H
#define TCPCHANNEL_H

#include "ICommunicationChannel.h"
#include <QTcpSocket>
#include <QTimer>
#include <QMutex>
#include <memory>

namespace Infrastructure {

/**
 * @brief TCP短连接通道
 * 专用于发送命令后立即断开的场景（CMD3和PDU命令）
 * 支持单例模式
 */
class TcpChannel : public ICommunicationChannel
{
    Q_OBJECT

public:
    static TcpChannel* getInstance();
    static void destroyInstance();

    // ICommunicationChannel 接口的存根实现
    bool open() override;
    void close() override;
    bool isOpen() const override;
    Status getStatus() const override;
    qint64 send(const QByteArray &data) override;
    QByteArray receive(int timeoutMs = 1000) override;
    void clearBuffer() override;
    void setParameters(const QVariantMap &params) override;
    QVariantMap getParameters() const override;

    // 主要功能：短连接发送
    bool sendToPort(const QByteArray &data, int port, const QString &host = "127.0.0.1", int timeout = 1000, QByteArray* responseDataOut = nullptr);
    bool sendToPortAsync(const QByteArray& data, int port, const QString& host = "127.0.0.1", int timeout = 1000, QByteArray* responseDataOut = nullptr);
public: // 确保是public
    explicit TcpChannel(QObject *parent = nullptr);
    ~TcpChannel() override;
    TcpChannel(const TcpChannel&) = delete;
    TcpChannel& operator=(const TcpChannel&) = delete;

    // 移除了 m_socket 相关的成员
    QString m_host;
    int m_port;
    int m_connectTimeout;
    Status m_status; // 状态仍可用于表示通道的整体配置状态
    QString m_lastError;
    mutable QMutex m_mutex;

    static TcpChannel* s_instance;
    static QMutex s_mutex;
};

} // namespace Infrastructure

#endif // TCPCHANNEL_H
