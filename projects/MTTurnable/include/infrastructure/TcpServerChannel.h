#ifndef TCPSERVERCHANNEL_H
#define TCPSERVERCHANNEL_H

#include "ICommunicationChannel.h"
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVariantMap>
#include <QString>
#include <QThread>
#include <QMutex>
#include <memory>

QT_FORWARD_DECLARE_CLASS(QTcpServer)
QT_FORWARD_DECLARE_CLASS(QTcpSocket)

namespace Infrastructure {

/**
 * @brief TCP服务器监听通道
 * 专用于监听64101端口接收响应和AutoApp发送的命令
 * 支持单例模式，服务器保持持续监听状态
 */
class TcpServerChannel : public ICommunicationChannel
{
    Q_OBJECT
    
public:
    // 单例访问
    static TcpServerChannel* getInstance();
    static void destroyInstance();

    ~TcpServerChannel() override;
    
    // ICommunicationChannel接口实现
    bool open() override;
    void close() override;
    bool isOpen() const override;
    Status getStatus() const override;
    qint64 send(const QByteArray& data) override;
    QByteArray receive(int timeoutMs = 1000) override;
    void clearBuffer() override;
    void setParameters(const QVariantMap& params) override;
    QVariantMap getParameters() const override;
    
    // TcpServerChannel特有方法
    bool isListening() const;
    int getPort() const;
    int getActualPort() const;
    QString getBindAddress() const;
    bool hasClient() const;
    QString getClientInfo() const;

signals:
    // 服务器特有信号
    void clientConnected(const QString& clientInfo);
    void clientDisconnected(const QString& clientInfo);
    void clientError(const QString& clientInfo, const QString& error);

private slots:
    void onNewConnection();
    void onAcceptError(QAbstractSocket::SocketError error);
    void onClientDataReady();
    void onClientDisconnected();
    void onClientError(QAbstractSocket::SocketError error);

private:
    explicit TcpServerChannel(QObject *parent = nullptr);
    void setStatus(Status newStatus);
    
    // 单例相关
    static TcpServerChannel* s_instance;
    static QMutex s_mutex;

    // 数据成员
    std::unique_ptr<QTcpServer> m_server;
    QThread* m_workerThread;  // 工作线程，用于TCP操作
    QTcpSocket* m_clientSocket;
    QString m_bindAddress;
    int m_port;
    int m_actualPort;
    int m_maxConnections;
    int m_connectionTimeout;
    Status m_status;
    QByteArray m_readBuffer;
    mutable QMutex m_dataMutex;
    
    QString m_lastError;
};

} // namespace Infrastructure

#endif // TCPSERVERCHANNEL_H 