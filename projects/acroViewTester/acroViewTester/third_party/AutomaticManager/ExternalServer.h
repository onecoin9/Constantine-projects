#pragma once
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>

class ExternalServer : public QObject {
    Q_OBJECT

public:
    explicit ExternalServer(QObject* parent = nullptr);
    ~ExternalServer();

    bool startServer(const QString& ip, quint16 port = 2030);
    void stopServer();

signals:
    void reportRequested(QTcpSocket* socket);

private slots:
    void handleNewConnection();
    void handleClientData();

private:
    QTcpServer* m_server;
    QMap<QTcpSocket*, QString> m_clients;
};
