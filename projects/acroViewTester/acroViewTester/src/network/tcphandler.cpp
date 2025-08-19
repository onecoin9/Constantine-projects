#include "tcphandler.h"
#include <QDebug>
#include <QTimer>
#include <QList>
#include <QString>

struct SiteInfo {
    int siteId;
    bool enable;
    QString siteSn;
};

struct LotData {
    QString lotid;
    QString productid;
    QString recipe;
    QString hardware;
    QString opid;
    bool selfcheck;
};

struct TestStartData {
    int bibId;
    QList<SiteInfo> sites;
};

TcpHandler::TcpHandler(QObject* parent)
    : QObject(parent)
    , m_server(nullptr)
    , m_socket(nullptr)
    , m_clientsocket(nullptr)
    , m_isServer(false)
    , m_isConnected(false)
    , m_isClientConnected(false)
{
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &TcpHandler::handleNewConnection);
}

TcpHandler::~TcpHandler()
{
    stopServer();
    disconnectFromServer();
}

bool TcpHandler::startServer(quint16 port)
{
    if (m_server->isListening()) {
        return true;
    }

    if (!m_server->listen(QHostAddress::Any, port)) {
        emit errorOccurred(m_server->errorString());
        return false;
    }

    m_isServer = true;
    qDebug() << "Server started on port:" << port;
    return true;
}

void TcpHandler::stopServer()
{
    if (m_server->isListening()) {
        m_server->close();
        m_isServer = false;
        qDebug() << "Server stopped";
    }
}

bool TcpHandler::isServerRunning() const
{
    return m_server->isListening();
}


void TcpHandler::disconnectFromServer()
{
    if (m_socket) {
        m_socket->disconnectFromHost();
        if (m_socket->state() != QAbstractSocket::UnconnectedState) {
            m_socket->waitForDisconnected(1000);
        }
        delete m_socket;
        m_socket = nullptr;
    }
    m_isConnected = false;
    emit connectionStatusChanged(false);
}


bool TcpHandler::sendData(const QByteArray& data,QByteArray& outResponse)
{
    if (m_clientsocket || m_isClientConnected) {
        m_clientsocket->disconnectFromHost();
        delete m_clientsocket;
    }

    m_clientsocket = new QTcpSocket(this);
    m_clientsocket->connectToHost("localhost", 2020);
    if (!m_clientsocket->waitForConnected(5000)) {
        emit errorOccurred(m_clientsocket->errorString());
        return false;
    }
    m_isClientConnected = true;

    qint64 bytesWritten = m_clientsocket->write(data);
    if (bytesWritten == -1) {
        emit errorOccurred(m_clientsocket->errorString());
        m_clientsocket->disconnectFromHost();
        m_isClientConnected = false;
        return false;
    }

    if (!m_clientsocket->waitForReadyRead(3000)) {
        emit errorOccurred("Timeout waiting for response");
        m_clientsocket->disconnectFromHost();
        m_isClientConnected = false;
        return false;
    }

    QByteArray response = m_clientsocket->readAll();
    if (response.isEmpty()) {
        emit errorOccurred("Empty response received");
        m_clientsocket->disconnectFromHost();
        m_isClientConnected = false;
        return false;
    }
    outResponse = response;
    m_clientsocket->disconnectFromHost();
    m_isClientConnected = false;
    return true;
}

void TcpHandler::handleNewConnection()
{
    QTcpSocket* clientSocket = m_server->nextPendingConnection();
    if (clientSocket) {
        if (m_socket) {
            m_socket->disconnectFromHost();
            delete m_socket;
        }
        m_socket = clientSocket;
        connect(m_socket, &QTcpSocket::readyRead, this, &TcpHandler::handleReadyRead);
        connect(m_socket, &QTcpSocket::disconnected, this, &TcpHandler::handleDisconnected);
        connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &TcpHandler::handleError);

        m_isConnected = true;
        emit connectionStatusChanged(true);
        qDebug() << "New client connected";
    }
}

void TcpHandler::handleReadyRead()
{
    if (!m_socket) return;

    QByteArray data = m_socket->readAll();
    if (!data.isEmpty()) {
        emit dataReceived(data);
        handleReadJson(data);
    }
}

void TcpHandler::handleReadJson(QByteArray& data)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON解析错误:" << parseError.errorString();
        return;
    }

    QJsonObject jsonObj = doc.object();
    QString method = jsonObj["method"].toString();

    if (method == "getversion") {
        QJsonObject responseObj;
        responseObj["method"] = method;
        responseObj["status"] = "success";

        QJsonObject responsedataObj;
        responsedataObj["version"] = "1.0.0.0";
        responseObj["data"] = responsedataObj;

        QJsonDocument responseDoc(responseObj);
        QByteArray responseData = responseDoc.toJson(QJsonDocument::Compact);

        int retry = 3;
        while (m_socket->write(responseData)==-1 && retry>0)
        {
            retry--;
        }
        QTimer::singleShot(100, this, [this]() {
            disconnectFromServer();
        });
    }
    else if (method == "lotstart")
    {
        QJsonObject dataObj = jsonObj["data"].toObject();

        LotData lotdata;
        // 解析各个字段
        lotdata.lotid = dataObj["lotid"].toString();
        lotdata.productid = dataObj["productid"].toString();
        lotdata.recipe = dataObj["recipe"].toString();
        lotdata.hardware = dataObj["hardware"].toString();
        lotdata.opid = dataObj["operator"].toString();
        lotdata.selfcheck = dataObj["selfcheck"].toBool();

        QJsonObject responseObj;
        responseObj["method"] = method;
        responseObj["status"] = "success";
        QJsonObject responsedataObj;
        responseObj["data"] = responsedataObj;

        QJsonDocument responseDoc(responseObj);
        QByteArray responseData = responseDoc.toJson(QJsonDocument::Compact);

        int retry = 3;
        while (m_socket->write(responseData) == -1 && retry > 0)
        {
            retry--;
        }
        QTimer::singleShot(100, this, [this]() {
            disconnectFromServer();
        });
    }
    else if (method == "lotend")
    {
        QJsonObject responseObj;
        responseObj["method"] = method;
        responseObj["status"] = "success";
        QJsonObject responsedataObj;
        responseObj["data"] = responsedataObj;

        QJsonDocument responseDoc(responseObj);
        QByteArray responseData = responseDoc.toJson(QJsonDocument::Compact);

        int retry = 3;
        while (m_socket->write(responseData) == -1 && retry > 0)
        {
            retry--;
        }
        QTimer::singleShot(100, this, [this]() {
            disconnectFromServer();
        });
    }
    else if (method == "teststart")
    {
        QJsonObject responseObj;
        responseObj["method"] = method;
        responseObj["status"] = "success";
        QJsonObject responsedataObj;
        responseObj["data"] = responsedataObj;

        QJsonDocument responseDoc(responseObj);
        QByteArray responseData = responseDoc.toJson(QJsonDocument::Compact);

        int retry = 3;
        while (m_socket->write(responseData) == -1 && retry > 0)
        {
            retry--;
        }
        QTimer::singleShot(100, this, [this]() {
            disconnectFromServer();
        });

        QJsonObject dataObj = jsonObj["data"].toObject();
        parseTestStartData(dataObj);
    }
}

void TcpHandler::parseTestStartData(const QJsonObject& dataObj) 
{
    TestStartData data;
    
    data.bibId = dataObj["bibid"].toInt();
    
    QJsonArray sitesArray = dataObj["sites"].toArray();
    for (const QJsonValue& siteValue : sitesArray) {
        QJsonObject siteObj = siteValue.toObject();
        SiteInfo site;
        site.siteId = siteObj["siteid"].toInt();
        site.enable = siteObj["enable"].toBool();
        site.siteSn = siteObj["sitesn"].toString();
        data.sites.append(site);
    }
}

void TcpHandler::handleDisconnected()
{
    m_isConnected = false;
    emit connectionStatusChanged(false);
    qDebug() << "Client disconnected";
}

void TcpHandler::handleError(QAbstractSocket::SocketError socketError)
{
    QString errorMessage = m_socket ? m_socket->errorString() : "Unknown error";
    emit errorOccurred(errorMessage);
    qDebug() << "Socket error:" << errorMessage;
}

