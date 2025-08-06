#include "tcpIpmodule.h"
#include <QDebug>

TcpIpModule::TcpIpModule(QObject* parent) : QObject(parent), tcpSocket(new QTcpSocket(this))
{
    connect(tcpSocket, &QTcpSocket::readyRead, this, &TcpIpModule::onReadyRead);
    connect(tcpSocket, &QTcpSocket::connected, this, &TcpIpModule::onConnected);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &TcpIpModule::onDisconnected);
    connect(tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
        this, &TcpIpModule::onErrorOccurred);
}

TcpIpModule::~TcpIpModule()
{
    disconnectFromServer();
}

void TcpIpModule::connectToServer(const QHostAddress& address, quint16 port)
{
    if (!tcpSocket->isOpen()) {
        tcpSocket->connectToHost(address, port);
    }
}

void TcpIpModule::disconnectFromServer()
{
    if (tcpSocket->isOpen()) {
        tcpSocket->disconnectFromHost();
    }
}

bool TcpIpModule::isConnected() const
{
    return tcpSocket->state() == QAbstractSocket::ConnectedState;
}

void TcpIpModule::sendData(const QByteArray& data)
{
    if (isConnected()) {
        tcpSocket->write(data);
    }
}

void TcpIpModule::onReadyRead()
{
    QByteArray data = tcpSocket->readAll();
    emit dataReceived(data);
}

void TcpIpModule::onConnected()
{
    emit connected();
}

void TcpIpModule::onDisconnected()
{
    emit disconnected();
}

void TcpIpModule::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
        emit errorOccurred(tcpSocket->errorString());
}