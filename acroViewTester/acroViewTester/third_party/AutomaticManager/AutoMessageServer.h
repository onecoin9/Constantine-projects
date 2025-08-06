#pragma once

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <memory>

class ExternalServer;

class AutoMessageServer : public QObject
{
    Q_OBJECT
public:
    explicit AutoMessageServer(QObject* parent = nullptr);
    ~AutoMessageServer();

    // 简化为单一端口的启动接口
    void startServer(const QString& serverIP = "127.0.0.1", quint16 licPort = 2030);
    void stopServer(int stopWhenExit);

signals:
    void reportRequested(QTcpSocket* client);

private:
    void startExternalServerAsync();
private:
    std::unique_ptr<ExternalServer> m_externalServer;
    QString m_serverIP;
    quint16 m_licPort = 0;
};
