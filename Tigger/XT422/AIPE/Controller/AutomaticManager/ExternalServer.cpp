#include "ExternalServer.h"
#include "AngkLogger.h"
#include <QDataStream>

typedef struct tagCmdHead {
    char strCmd[16];	///命令
    uint32_t DataLen;   ///命令数据长度
}tCmdHead;

ExternalServer::ExternalServer(QObject* parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
{
    connect(m_server, &QTcpServer::newConnection,
        this, &ExternalServer::handleNewConnection);
}

ExternalServer::~ExternalServer()
{
    stopServer();
}

bool ExternalServer::startServer(const QString& ip, quint16 port)
{
    QHostAddress serverAddress;
    if (ip == "127.0.0.1" || ip.isEmpty()) {
        serverAddress = QHostAddress::LocalHost;
    } else {
        serverAddress.setAddress(ip);
    }
    if (m_server->listen(serverAddress, port)) {
        //ALOG_INFO("Start listen on port %d.", "CU", "--", port);
        return true;
    }
    //ALOG_INFO("Failed to Start listen on port %d.", "CU", "--", port);
    return false;
}

void ExternalServer::stopServer()
{
    for (QTcpSocket* socket : m_clients.keys()) {
        socket->disconnectFromHost();
    }
    m_clients.clear();
    m_server->close();
}

void ExternalServer::handleNewConnection()
{
    QTcpSocket* clientSocket = m_server->nextPendingConnection();
    QString clientId = QString("%1:%2")
        .arg(clientSocket->peerAddress().toString())
        .arg(clientSocket->peerPort());

    connect(clientSocket, &QTcpSocket::readyRead,
        this, &ExternalServer::handleClientData);

    m_clients.insert(clientSocket, clientId);
    //ALOG_INFO("New client connected", "CU", "--");
}

void ExternalServer::handleClientData()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    // First read 20 bytes for length
    const int header_length = sizeof(tCmdHead);
    QByteArray lengthData = socket->read(header_length);
    if (lengthData.size() < header_length) {
        socket->disconnectFromHost();
        m_clients.remove(socket);
        socket->deleteLater();
        ALOG_ERROR("Failed to read length %d data from client", "CU", "--", header_length);
        return;
    }

    // Parse length from first 4 bytes
    quint32 length = 0;
    memcpy(&length, lengthData.data() + header_length - sizeof(length), sizeof(length));

    // Read remaining data
    QByteArray messageData = socket->read(length);
    if (messageData.size() < length) {
        socket->disconnectFromHost();
        m_clients.remove(socket);
        socket->deleteLater(); 
        ALOG_ERROR("Failed to read length %d data from client", "CU", "--", length);
        return;
    }

    // Get the actual message data
    QString message = QString::fromUtf8(messageData).trimmed();

    if (message == "GetReport") {
        emit reportRequested(socket);
    }

    socket->disconnectFromHost();
    m_clients.remove(socket);
    socket->deleteLater();
}
