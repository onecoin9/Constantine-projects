#ifndef TCPIPMODULE_H
#define TCPIPMODULE_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>

class TcpIpModule : public QObject
{
    Q_OBJECT

public:
    explicit TcpIpModule(QObject* parent = nullptr);
    ~TcpIpModule();

    void connectToServer(const QHostAddress& address, quint16 port);
    void disconnectFromServer();
    bool isConnected() const;

signals:
    void dataReceived(const QByteArray& data);
    void connected();
    void disconnected();
    void errorOccurred(const QString& errorString);

public slots:
    void sendData(const QByteArray& data);

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket* tcpSocket;
};

#endif // TCPIPMODULE_H