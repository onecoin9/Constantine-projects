#pragma once
#ifndef TCPHANDLER_H
#define TCPHANDLER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <QJsonArray>

class TcpHandler : public QObject
{
    Q_OBJECT

public:
    explicit TcpHandler(QObject* parent = nullptr);
    ~TcpHandler();

    bool startServer(quint16 port = 8080);
    void stopServer();
    bool isServerRunning() const;
    void disconnectFromServer();

    bool sendData(const QByteArray& data, QByteArray& outResponse);
    void handleReadJson(QByteArray& data);
    void parseTestStartData(const QJsonObject& dataObj);


signals:
    void dataReceived(const QByteArray& data);
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString& error);

private slots:
    void handleNewConnection();
    void handleReadyRead();
    void handleDisconnected();
    void handleError(QAbstractSocket::SocketError socketError);

private:
    QTcpServer* m_server;
    QTcpSocket* m_socket;
    QTcpSocket* m_clientsocket;
    bool m_isServer;
    bool m_isConnected;
    bool m_isClientConnected;
};


#endif // TCPHANDLER_H 