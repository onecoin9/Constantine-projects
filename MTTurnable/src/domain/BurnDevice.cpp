#include "domain/BurnDevice.h"
#include "core/Logger.h"
#include <QtEndian>
#include <QJsonParseError>
#include <QFileInfo>
#include <QTcpSocket>
#include <QEventLoop>
#include <QTimer>
#include <QMutexLocker>
#include <QProcess>
#include <QCoreApplication>
#include <QThread>
#include <QRegularExpression>

namespace Domain {

const QString BurnDevice::CLIENT_JSONRPC_VERSION = "2.0";

BurnDevice::BurnDevice(QObject *parent)
    : IDevice(parent)
    , m_serverHost("127.0.0.1")
    , m_serverPort(12345)
    , m_timeout(5000)         // 连接超时
    , m_requestTimeout(10000) // 请求超时(缩短到10秒方便调试)
    , m_reconnectInterval(3000) // 重连间隔
    , m_aprogPath("C:/Users/Administrator/Desktop/1/xt422/AIPE/Build/Aprog.exe") // 默认路径
    , m_socket(nullptr)
    , m_connectionTimer(nullptr)
    , m_reconnectTimer(nullptr)
    , m_atomicStatus(IDevice::DeviceStatus::Disconnected)
    , m_isConnected(false)
    , m_isConnecting(false)
    , m_aprogProcess(nullptr)
    , m_isAprogRunning(false)
    , m_nextRequestId(1)
{
    m_name = "BurnDevice";
    m_type = DeviceType::Burn;
    
    // 创建定时器
    m_connectionTimer = new QTimer(this);
    m_connectionTimer->setSingleShot(true);
    connect(m_connectionTimer, &QTimer::timeout, this, &BurnDevice::onConnectionTimeout);
    
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &BurnDevice::onReconnectTimer);
    
    // 创建QProcess
    m_aprogProcess = new QProcess(this);
    connect(m_aprogProcess, &QProcess::started, this, &BurnDevice::onAprogStarted);
    connect(m_aprogProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &BurnDevice::onAprogFinished);
    connect(m_aprogProcess, &QProcess::errorOccurred, this, &BurnDevice::onAprogError);
    connect(m_aprogProcess, &QProcess::readyReadStandardOutput, this, &BurnDevice::onAprogReadyReadStandardOutput);
    connect(m_aprogProcess, &QProcess::readyReadStandardError, this, &BurnDevice::onAprogReadyReadStandardError);
    
    LOG_MODULE_INFO("BurnDevice", "BurnDevice created (long connection mode with Aprog.exe integration)");
}

BurnDevice::~BurnDevice()
{
    release();
    
    // 停止Aprog.exe进程
    stopAprog();
    
    // 清理待处理请求
    failAllPendingRequests("设备正在析构");
    
    LOG_MODULE_INFO("BurnDevice", "BurnDevice destroyed");
}

bool BurnDevice::initialize()
{
    LOG_MODULE_INFO("BurnDevice", "Initializing BurnDevice (long connection mode with Aprog.exe)...");
    
    if (m_atomicStatus.load() == IDevice::DeviceStatus::Ready) {
        LOG_MODULE_WARNING("BurnDevice", "Device already initialized");
        return true;
    }
    
    m_atomicStatus.store(IDevice::DeviceStatus::Initializing);
    emit statusChanged(IDevice::DeviceStatus::Initializing);
    
    // 首先启动Aprog.exe进程
    startAprog();
    
    // 给Aprog.exe一些时间启动，然后尝试连接到服务器
    QTimer::singleShot(2000, this, &BurnDevice::tryConnectToServer);
    
    return true; // 异步初始化，实际状态通过信号更新
}

bool BurnDevice::release()
{
    LOG_MODULE_INFO("BurnDevice", "Releasing BurnDevice...");
    
    disconnectFromServer();
    stopAprog();
    
    m_atomicStatus.store(IDevice::DeviceStatus::Disconnected);
    emit statusChanged(IDevice::DeviceStatus::Disconnected);
    
    LOG_MODULE_INFO("BurnDevice", "BurnDevice released");
    return true;
}

IDevice::DeviceStatus BurnDevice::getStatus() const
{
    return m_atomicStatus.load();
}

QString BurnDevice::getName() const
{
    return m_name;
}

IDevice::DeviceType BurnDevice::getType() const
{
    return m_type;
}

QString BurnDevice::getDescription() const
{
    return QString("JSON-RPC烧录设备 (长连接模式) - %1:%2").arg(m_serverHost).arg(m_serverPort);
}

bool BurnDevice::isConnected() const
{
    return m_isConnected;
}

QJsonObject BurnDevice::executeCommand(const QString &command, const QJsonObject &params)
{
    LOG_MODULE_INFO("BurnDevice", QString("Executing command: %1").arg(command).toStdString());
    
    QJsonObject result;
    result["success"] = true; // 在长连接模式下，命令排队总是成功
    
    try {
        if (command == "siteScanAndConnect") {
            result = siteScanAndConnect();
        } else if (command == "loadProject") {
            QString path = params["path"].toString();
            QString taskFileName = params["taskFileName"].toString();
            result = loadProject(path, taskFileName);
        } else if (command == "doJob") {
            QString strIP = params["strIP"].toString();
            int nHopNum = params["nHopNum"].toInt();
            int portID = params["PortID"].toInt();
            int cmdFlag = params["CmdFlag"].toInt();
            int cmdID = params["CmdID"].toInt();
            quint32 sktEn = static_cast<quint32>(params["SKTEn"].toDouble());
            int bpuID = params["BPUID"].toInt();
            QJsonObject docmdSeqJson = params["docmdSeqJson"].toObject();
            result = doJob(strIP, nHopNum, portID, cmdFlag, cmdID, sktEn, bpuID, docmdSeqJson);
        } else if (command == "getProjectInfo") {
            result = getProjectInfo();
        } else if (command == "getProjectInfoExt") {
            QString projectUrl = params["projectUrl"].toString();
            result = getProjectInfoExt(projectUrl);
        } else if (command == "getSKTInfo") {
            result = getSKTInfo();
        } else if (command == "doCustom") {
            QString strIP = params["strIP"].toString();
            int nHopNum = params["nHopNum"].toInt();
            int portID = params["PortID"].toInt();
            int cmdFlag = params["CmdFlag"].toInt();
            int cmdID = params["CmdID"].toInt();
            quint32 sktEn = static_cast<quint32>(params["SKTEn"].toDouble());
            int bpuID = params["BPUID"].toInt();
            QJsonObject doCustomData = params["doCustomData"].toObject();
            result = doCustom(strIP, nHopNum, portID, cmdFlag, cmdID, sktEn, bpuID, doCustomData);
        } else if (command == "sendUUID") {
            QString uuid = params["uuid"].toString();
            QString strIP = params["strIP"].toString();
            int nHopNum = params["nHopNum"].toInt();
            quint32 sktEnable = static_cast<quint32>(params["sktEnable"].toDouble());
            result = sendUUID(uuid, strIP, nHopNum, sktEnable);
        } else {
            result["success"] = false;
            result["error"] = QString("未知命令: %1").arg(command);
        }
    } catch (const std::exception &e) {
        result["success"] = false;
        result["error"] = QString("命令执行异常: %1").arg(e.what());
        LOG_MODULE_ERROR("BurnDevice", QString("Command execution failed: %1").arg(e.what()).toStdString());
    }
    
    emit commandFinished(result);
    return result;
}

void BurnDevice::setCommunicationChannel(std::shared_ptr<Infrastructure::ICommunicationChannel> channel)
{
    m_commChannel = channel;
}

std::shared_ptr<Infrastructure::ICommunicationChannel> BurnDevice::getCommunicationChannel() const
{
    return m_commChannel;
}

void BurnDevice::setConfiguration(const QJsonObject &config)
{
    QMutexLocker locker(&m_configMutex);
    m_config = config;
    
    if (config.contains("serverHost")) {
        m_serverHost = config["serverHost"].toString();
    }
    if (config.contains("serverPort")) {
        m_serverPort = static_cast<quint16>(config["serverPort"].toInt());
    }
    if (config.contains("name")) {
        m_name = config["name"].toString();
    }
    if (config.contains("timeout")) {
        m_timeout = config["timeout"].toInt();
    }
    if (config.contains("requestTimeout")) {
        m_requestTimeout = config["requestTimeout"].toInt();
    }
    if (config.contains("reconnectInterval")) {
        m_reconnectInterval = config["reconnectInterval"].toInt();
    }
    if (config.contains("aprogPath")) {
        m_aprogPath = config["aprogPath"].toString();
        LOG_MODULE_INFO("BurnDevice", QString("Aprog path loaded from config: %1").arg(m_aprogPath).toStdString());
        
        // 验证文件是否存在
        QFileInfo fileInfo(m_aprogPath);
        if (!fileInfo.exists()) {
            LOG_MODULE_WARNING("BurnDevice", QString("Aprog.exe not found at configured path: %1").arg(m_aprogPath).toStdString());
        } else if (!fileInfo.isExecutable()) {
            LOG_MODULE_WARNING("BurnDevice", QString("Configured Aprog path is not executable: %1").arg(m_aprogPath).toStdString());
        } else {
            LOG_MODULE_INFO("BurnDevice", "Aprog.exe path validation successful");
        }
    } else {
        LOG_MODULE_WARNING("BurnDevice", "No aprogPath specified in configuration, manual selection will be required");
    }
    
    LOG_MODULE_INFO("BurnDevice", QString("Configuration set - Server: %1:%2, Aprog: %3, Timeout: %4ms")
                   .arg(m_serverHost).arg(m_serverPort).arg(m_aprogPath).arg(m_timeout).toStdString());
}

QJsonObject BurnDevice::getConfiguration() const
{
    QMutexLocker locker(&m_configMutex);
    return m_config;
}

bool BurnDevice::selfTest()
{
    if (!m_isConnected) {
        LOG_MODULE_WARNING("BurnDevice", "Self test failed - not connected");
        return false;
    }
    
    // 发送一个简单的GetProjectInfo请求作为自检
    queueRequest("selfTest", "GetProjectInfo");
    LOG_MODULE_INFO("BurnDevice", "Self test request queued");
    return true;
}

QString BurnDevice::getServerHost() const
{
    QMutexLocker locker(&m_configMutex);
    return m_serverHost;
}

quint16 BurnDevice::getServerPort() const
{
    QMutexLocker locker(&m_configMutex);
    return m_serverPort;
}

void BurnDevice::setServerInfo(const QString &host, quint16 port)
{
    {
        QMutexLocker locker(&m_configMutex);
        m_serverHost = host;
        m_serverPort = port;
    }
    
    LOG_MODULE_INFO("BurnDevice", QString("Server info updated: %1:%2").arg(host).arg(port).toStdString());
    
    // 如果当前已连接，重新连接到新的服务器
    if (m_isConnected) {
        disconnectFromServer();
        connectToServer();
    }
}

// JSON-RPC方法实现 - 将请求加入队列
QJsonObject BurnDevice::siteScanAndConnect()
{
    QJsonObject result;
    queueRequest("siteScanAndConnect", "SiteScanAndConnect");
    result["success"] = true;
    result["message"] = "";
    return result;
}

QJsonObject BurnDevice::loadProject(const QString &path, const QString &taskFileName)
{
    QJsonObject result;
    QJsonObject params;
    params["path"] = path;
    params["taskFileName"] = taskFileName;
    
    queueRequest("loadProject", "LoadProject", params);
    result["success"] = true;

    return result;
}

QJsonObject BurnDevice::doJob(const QString &strIP, int nHopNum, int portID, int cmdFlag, 
                                 int cmdID, quint32 sktEn, int bpuID, const QJsonObject &docmdSeqJson)
{
    QJsonObject result;
    QJsonObject params;
    params["strIP"] = strIP;
    params["nHopNum"] = nHopNum;
    params["PortID"] = portID;
    params["CmdFlag"] = cmdFlag;
    params["CmdID"] = cmdID;
    params["SKTEn"] = static_cast<double>(sktEn);
    params["BPUID"] = bpuID;
    params["docmdSeqJson"] = docmdSeqJson;
    
    queueRequest("doJob", "DoJob", params);
    result["success"] = true;

    return result;
}

QJsonObject BurnDevice::getProjectInfo()
{
    QJsonObject result;
    queueRequest("getProjectInfo", "GetProjectInfo");
    result["success"] = true;

    return result;
}

QJsonObject BurnDevice::doCustom(const QString &strIP, int nHopNum, int portID, int cmdFlag, 
                                 int cmdID, quint32 sktEn, int bpuID, const QJsonObject &doCustomData)
{
    QJsonObject result;
    QJsonObject params;
    params["strIP"] = strIP;
    params["nHopNum"] = nHopNum;
    params["PortID"] = portID;
    params["CmdFlag"] = cmdFlag;
    params["CmdID"] = cmdID;
    params["SKTEn"] = static_cast<double>(sktEn);
    params["BPUID"] = bpuID;
    params["doCustomData"] = doCustomData;
    
    queueRequest("doCustom", "DoCustom", params);
    result["success"] = true;
    return result;
}

QJsonObject BurnDevice::getProjectInfoExt(const QString &projectUrl)
{
    QJsonObject result;
    QJsonObject params;
    params["project_url"] = projectUrl;
    
    queueRequest("getProjectInfoExt", "GetProjectInfoExt", params);
    result["success"] = true;

    return result;
}

QJsonObject BurnDevice::getSKTInfo()
{
    QJsonObject result;
    queueRequest("getSKTInfo", "GetSKTInfo");
    result["success"] = true;

    return result;
}

QJsonObject BurnDevice::sendUUID(const QString &uuid, const QString &strIP, int nHopNum, quint32 sktEnable)
{
    QJsonObject result;
    QJsonObject params;
    params["uuid"] = uuid;
    params["strIP"] = strIP;
    params["nHopNum"] = static_cast<double>(nHopNum);
    params["sktEnable"] = static_cast<double>(sktEnable);
    
    queueRequest("sendUUID", "SendUUID", params);
    result["success"] = true;

    return result;
}

// 手动请求连接的公共方法
void BurnDevice::requestConnection()
{
    LOG_MODULE_INFO("BurnDevice", "Manual connection requested");
    if (m_isConnected) {

        return;
    }
    
    if (m_isConnecting) {

        return;
    }
    
    tryConnectToServer();
}

// 尝试连接到服务器（非阻塞）
void BurnDevice::tryConnectToServer()
{
    if (m_isConnecting || m_isConnected) {
        return;
    }
    
    QString host;
    quint16 port;
    {
        QMutexLocker locker(&m_configMutex);
        host = m_serverHost;
        port = m_serverPort;
    }
    
    LOG_MODULE_INFO("BurnDevice", QString("Trying to connect to %1:%2").arg(host).arg(port).toStdString());
    
    m_isConnecting = true;
    
    // 创建socket
    if (m_socket) {
        m_socket->deleteLater();
    }
    
    m_socket = new QTcpSocket(this);
    
    // 连接信号
    connect(m_socket, &QTcpSocket::connected, this, &BurnDevice::onSocketConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &BurnDevice::onSocketDisconnected);
    connect(m_socket, &QAbstractSocket::errorOccurred, this, &BurnDevice::onSocketError);
    connect(m_socket, &QTcpSocket::readyRead, this, &BurnDevice::onSocketReadyRead);
    
    // 开始连接
    m_socket->connectToHost(host, port);
    
    // 启动连接超时定时器
    m_connectionTimer->start(m_timeout);
}

// 检查连接状态并发送请求
void BurnDevice::checkConnectionAndSendRequests()
{
    LOG_MODULE_DEBUG("BurnDevice", QString("checkConnectionAndSendRequests: connected=%1, connecting=%2, socket_state=%3")
                    .arg(m_isConnected)
                    .arg(m_isConnecting)
                    .arg(m_socket ? static_cast<int>(m_socket->state()) : -1).toStdString());
    
    if (m_isConnected && m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        // 已连接，立即发送队列中的请求
        LOG_MODULE_DEBUG("BurnDevice", "Device connected, sending queued requests immediately");
        QTimer::singleShot(0, this, &BurnDevice::sendQueuedRequests);
    } else if (!m_isConnecting) {
        // 未连接且未在连接中，尝试连接
        LOG_MODULE_DEBUG("BurnDevice", "Device not connected, trying to connect");
        tryConnectToServer();
        
        // 给连接较短时间，快速失败以避免卡死
        QTimer::singleShot(2000, this, [this]() {
            if (m_isConnected && m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
                LOG_MODULE_DEBUG("BurnDevice", "Connection established, sending requests");
                sendQueuedRequests();
            } else {
                // 连接失败，失败所有队列中的请求
                LOG_MODULE_WARNING("BurnDevice", "Connection failed after 2s timeout, failing all pending requests");
            }
        });
    } else {
        // 正在连接中，等待连接完成（缩短等待时间）
        LOG_MODULE_DEBUG("BurnDevice", "Connection in progress, waiting for completion");
        QTimer::singleShot(3000, this, [this]() {
            if (m_isConnected && m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
                LOG_MODULE_DEBUG("BurnDevice", "Connection completed, sending requests");
                sendQueuedRequests();
            } else {
                LOG_MODULE_WARNING("BurnDevice", "Connection still not established after 3s timeout");
            }
        });
    }
}

// 连接管理
void BurnDevice::connectToServer()
{
    if (m_isConnecting || m_isConnected) {
        return;
    }
    
    QString host;
    quint16 port;
    {
        QMutexLocker locker(&m_configMutex);
        host = m_serverHost;
        port = m_serverPort;
    }
    
    LOG_MODULE_INFO("BurnDevice", QString("Connecting to %1:%2").arg(host).arg(port).toStdString());
    
    m_isConnecting = true;
    
    // 创建socket
    if (m_socket) {
        m_socket->deleteLater();
    }
    
    m_socket = new QTcpSocket(this);
    
    // 连接信号
    connect(m_socket, &QTcpSocket::connected, this, &BurnDevice::onSocketConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &BurnDevice::onSocketDisconnected);
    connect(m_socket, &QAbstractSocket::errorOccurred, this, &BurnDevice::onSocketError);
    connect(m_socket, &QTcpSocket::readyRead, this, &BurnDevice::onSocketReadyRead);
    
    // 开始连接
    m_socket->connectToHost(host, port);
    
    // 启动连接超时定时器
    m_connectionTimer->start(m_timeout);
}

void BurnDevice::disconnectFromServer()
{
    m_connectionTimer->stop();
    m_reconnectTimer->stop();
    
    if (m_socket) {
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    
    m_isConnected = false;
    m_isConnecting = false;
    
}

void BurnDevice::queueRequest(const QString &operation, const QString &method, const QJsonValue &params)
{
    QMutexLocker locker(&m_requestMutex);
    
    auto request = new PendingRequest();
    request->operation = operation;
    request->method = method;
    request->requestId = generateRequestId();
    request->params = params;
    
    // 创建超时定时器
    request->timeoutTimer = new QTimer();
    request->timeoutTimer->setSingleShot(true);
    request->timeoutTimer->setInterval(m_requestTimeout);
    connect(request->timeoutTimer, &QTimer::timeout, this, &BurnDevice::onRequestTimeout);
    
    m_requestQueue.enqueue(request);
    
    LOG_MODULE_DEBUG("BurnDevice", QString("Request queued: %1 (ID: %2)").arg(operation).arg(request->requestId).toStdString());
    
    locker.unlock();
    
    // 每次发送命令时检查连接状态
    checkConnectionAndSendRequests();
}

void BurnDevice::sendQueuedRequests()
{
    if (!m_isConnected || !m_socket) {
        return;
    }
    
    QMutexLocker locker(&m_requestMutex);
    
    while (!m_requestQueue.isEmpty()) {
        auto request = m_requestQueue.dequeue();
        m_pendingRequests[request->requestId] = request;
        
        sendJsonRpcRequest(request);
    }
}

void BurnDevice::sendJsonRpcRequest(PendingRequest* request)
{
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        return;
    }
    
    // 构建JSON-RPC请求
    QJsonObject requestObj;
    requestObj["jsonrpc"] = CLIENT_JSONRPC_VERSION;
    requestObj["method"] = request->method;
    if (!request->params.isNull() && !request->params.isUndefined()) {
        requestObj["params"] = request->params;
    }
    requestObj["id"] = request->requestId;
    
    QJsonDocument doc(requestObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    
    LOG_MODULE_DEBUG("BurnDevice", QString("Sending JSON-RPC request: %1").arg(QString::fromUtf8(jsonData)).toStdString());
    
    // 创建协议包并发送
    QByteArray packet = createPacket(jsonData);
    qint64 written = m_socket->write(packet);
    
    if (written != packet.size()) {
        return;
    }
    
    // 强制刷新发送缓冲区
    if (!m_socket->flush()) {
        LOG_MODULE_WARNING("BurnDevice", "Failed to flush socket buffer");
    }
    
    // 启动请求超时定时器
    request->timeoutTimer->start();
    
    LOG_MODULE_DEBUG("BurnDevice", QString("Sent %1 bytes for request ID %2, waiting for response...").arg(written).arg(request->requestId).toStdString());
}

// 网络事件处理
void BurnDevice::onSocketConnected()
{
    LOG_MODULE_INFO("BurnDevice", "Connected to server successfully");
    
    m_connectionTimer->stop();
    m_isConnecting = false;
    m_isConnected = true;
    
    m_atomicStatus.store(IDevice::DeviceStatus::Ready);
    emit statusChanged(IDevice::DeviceStatus::Ready);
    emit connected();
    
    // 发出连接成功信号
    emit operationCompleted("connection", QJsonObject{{"message", "成功连接到服务器"}});
    
    // 发送队列中的请求
    sendQueuedRequests();
}

void BurnDevice::onSocketDisconnected()
{
    LOG_MODULE_WARNING("BurnDevice", "Disconnected from server");
    
    m_isConnected = false;
    m_isConnecting = false;
    
    m_atomicStatus.store(IDevice::DeviceStatus::Disconnected);
    emit statusChanged(IDevice::DeviceStatus::Disconnected);
    emit disconnected();
    
    // 失败所有待处理的请求
    failAllPendingRequests("连接已断开");
    
    // 启动重连定时器
    if (m_atomicStatus.load() != IDevice::DeviceStatus::Disconnected) { // 只有在非主动断开时才重连
        m_reconnectTimer->start(m_reconnectInterval);
    }
}

void BurnDevice::onSocketError(QAbstractSocket::SocketError error)
{
    QString errorString = m_socket ? m_socket->errorString() : "Unknown socket error";
    LOG_MODULE_ERROR("BurnDevice", QString("Socket error: %1 (%2)").arg(errorString).arg(static_cast<int>(error)).toStdString());
    
    m_connectionTimer->stop(); // 停止连接定时器
    m_isConnecting = false;
    m_isConnected = false;
    
    // 更新设备状态为断开连接
    m_atomicStatus.store(IDevice::DeviceStatus::Disconnected);
    emit statusChanged(IDevice::DeviceStatus::Disconnected);
    
    // 失败所有待处理的请求（在发送连接失败信号之前）
    failAllPendingRequests(QString("网络错误: %1").arg(errorString));
    
    // 发出连接失败信号
    QTimer::singleShot(0, this, [this, errorString]() {
        emit operationFailed("connection", QString("连接失败: %1").arg(errorString));
    });
}

void BurnDevice::onSocketReadyRead()
{
    if (!m_socket) return;
    
    QByteArray newData = m_socket->readAll();
    LOG_MODULE_DEBUG("BurnDevice", QString("Received %1 bytes from server").arg(newData.size()).toStdString());
    
    m_receiveBuffer.append(newData);
    processIncomingData();
}

void BurnDevice::onConnectionTimeout()
{
    LOG_MODULE_WARNING("BurnDevice", "Connection timeout");
    
    m_isConnecting = false;
    m_isConnected = false;
    
    if (m_socket) {
        m_socket->abort();
    }
    
    // 更新设备状态为断开连接
    m_atomicStatus.store(IDevice::DeviceStatus::Disconnected);
    emit statusChanged(IDevice::DeviceStatus::Disconnected);
    
}

void BurnDevice::onRequestTimeout()
{
    QTimer* timer = qobject_cast<QTimer*>(sender());
    if (!timer) return;
    
    // 查找超时的请求
    QMutexLocker locker(&m_requestMutex);
    for (auto it = m_pendingRequests.begin(); it != m_pendingRequests.end(); ++it) {
        if (it.value()->timeoutTimer == timer) {
            LOG_MODULE_WARNING("BurnDevice", QString("Request timeout: %1 (ID: %2)")
                              .arg(it.value()->operation).arg(it.key()).toStdString());
            
            completeRequest(it.key(), false, QJsonObject(), "请求超时");
            break;
        }
    }
}

void BurnDevice::onReconnectTimer()
{
    LOG_MODULE_INFO("BurnDevice", "Attempting to reconnect...");
    connectToServer();
}

void BurnDevice::processIncomingData()
{
    while (m_receiveBuffer.size() >= CLIENT_HEADER_LENGTH) {
        // 检查是否有完整的包
        quint32 payloadLength;
        memcpy(&payloadLength, m_receiveBuffer.constData() + 6, 4);
        payloadLength = qFromBigEndian(payloadLength);
        
        int totalPacketSize = CLIENT_HEADER_LENGTH + payloadLength;
        
        if (m_receiveBuffer.size() < totalPacketSize) {
            // 数据还不够一个完整包
            return;
        }
        
        // 提取完整包
        QByteArray packet = m_receiveBuffer.left(totalPacketSize);
        m_receiveBuffer.remove(0, totalPacketSize);
        
        // 处理包
        processReceivedPacket(packet);
    }
}

void BurnDevice::processReceivedPacket(const QByteArray &packet)
{
    if (packet.size() < CLIENT_HEADER_LENGTH) {
        LOG_MODULE_WARNING("BurnDevice", "Received packet too small");
        return;
    }
    
    // 检查魔数
    quint32 receivedMagic;
    memcpy(&receivedMagic, packet.constData(), 4);
    receivedMagic = qFromBigEndian(receivedMagic);
    if (receivedMagic != CLIENT_MAGIC_NUMBER) {
        LOG_MODULE_WARNING("BurnDevice", "Invalid magic number in received packet");
        return;
    }
    
    // 提取载荷
    QByteArray payload = packet.mid(CLIENT_HEADER_LENGTH);
    
    // 解析JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        LOG_MODULE_ERROR("BurnDevice", QString("JSON parse error: %1").arg(parseError.errorString()).toStdString());
        return;
    }
    
    if (!doc.isObject()) {
        LOG_MODULE_WARNING("BurnDevice", "Response is not a JSON object");
        return;
    }
    
    handleJsonRpcResponse(doc.object());
}

void BurnDevice::handleJsonRpcResponse(const QJsonObject &response)
{
    // 1. 验证 "jsonrpc" 版本
    if (!response.contains("jsonrpc") || response["jsonrpc"].toString() != CLIENT_JSONRPC_VERSION) {
        LOG_MODULE_WARNING("BurnDevice", "Received message with missing or invalid 'jsonrpc' version.");
        emit protocolError("Invalid 'jsonrpc' version in received message");
        return;
    }

    // 2. 根据消息类型分发处理
    bool hasId = response.contains("id") && !response.value("id").isNull();
    bool hasMethod = response.contains("method");
    bool hasResult = response.contains("result");

    if (hasId) {
        processResponse(response);
    } else if (hasMethod) {
        processMethodNotification(response);
    } else if (hasResult) { // id 为 null
        processResultNotification(response);
    } else {
        LOG_MODULE_WARNING("BurnDevice", "Received an invalid JSON-RPC message (neither response nor notification).");
        emit protocolError("Received unknown JSON-RPC message type");
    }
}

void BurnDevice::processResponse(const QJsonObject &response)
{
    qint64 responseId = response.value("id").toVariant().toLongLong();

    if (response.contains("result")) {
        // 成功响应
        QJsonObject result;
        result["success"] = true;
        result["response"] = response["result"];
        completeRequest(responseId, true, result);
    } else if (response.contains("error")) {
        // 错误响应
        QJsonObject errorObj = response["error"].toObject();
    } else {
        // 无效响应
    }
}

void BurnDevice::processResultNotification(const QJsonObject &response)
{
    QJsonObject resultObj = response["result"].toObject();
    if (resultObj.contains("cmd") && resultObj["cmd"].isString()) {
        QString cmd = resultObj["cmd"].toString();
        QJsonObject data = resultObj.value("data").toObject();
        LOG_MODULE_INFO("BurnDevice", QString("Received server command (null-id style): %1").arg(cmd).toStdString());
        emit serverCommandReceived(cmd, data);
    } else {
        LOG_MODULE_WARNING("BurnDevice", "Received message with null id but invalid 'result' format.");
        emit protocolError("Received malformed server command");
    }
}

void BurnDevice::processMethodNotification(const QJsonObject &response)
{
    QString method = response.value("method").toString();
    QJsonObject params = response.value("params").toObject();
    LOG_MODULE_INFO("BurnDevice", QString("Received server notification (method style): %1").arg(method).toStdString());

    // 对特定通知的详细处理逻辑
    if (method == "DeviceDiscovered") {
        if (params.contains("device") && params["device"].isObject() && params.contains("ipHop") && params["ipHop"].isString()) {
            QJsonObject deviceData = params["device"].toObject();
            QString ipHop = params["ipHop"].toString();
            LOG_MODULE_INFO("BurnDevice", QString("DeviceDiscovered notification for ipHop: %1").arg(ipHop).toStdString());
            LOG_MODULE_INFO("BurnDevice", QString("Device Info: %1").arg(QString(QJsonDocument(deviceData).toJson(QJsonDocument::Compact))).toStdString());
            
            // 发射信号，将发现的设备信息传递出去
            emit deviceDiscovered(deviceData);
            
        } else {
            LOG_MODULE_WARNING("BurnDevice", "Invalid 'params' for DeviceDiscovered notification.");
            emit protocolError("Invalid params format for DeviceDiscovered notification");
        }
    }
    else if (method == "ClientDoCmd") {
         if (params.contains("cmd") && params.contains("data")) {
             LOG_MODULE_INFO("BurnDevice", QString("ClientDoCmd: cmd=%1, data=%2")
                             .arg(params["cmd"].toVariant().toString())
                             .arg(params["data"].toString())
                             .toStdString());
             QString cmd = params["cmd"].toString();
             QString data = params["data"].toString();
             if(cmd == "setSKTEnResult"){
                    //把收到的所有站点的sktenable信息发送出去
                    emit getSKTEnable(data);
             }
         }
    }

    // 统一发出通用信号
    emit serverCommandReceived(method, params);
}

void BurnDevice::completeRequest(qint64 requestId, bool success, const QJsonObject &result, const QString &error)
{
    LOG_MODULE_DEBUG("BurnDevice", QString("Completing request ID %1, success: %2").arg(requestId).arg(success).toStdString());
    
    QString operation;
    {
        QMutexLocker locker(&m_requestMutex);
        
        auto it = m_pendingRequests.find(requestId);
        if (it == m_pendingRequests.end()) {
            LOG_MODULE_WARNING("BurnDevice", QString("Request ID %1 not found in pending requests").arg(requestId).toStdString());
            return;
        }
        
        auto request = it.value();
        operation = request->operation;
        
        m_pendingRequests.erase(it);
        
        // 停止超时定时器
        if (request->timeoutTimer) {
            request->timeoutTimer->stop();
        }
        
        delete request;
    }
    
    // 使用QTimer::singleShot异步发送信号，避免在锁中发送信号可能导致的死锁
    if (success) {
        LOG_MODULE_INFO("BurnDevice", QString("Operation %1 completed successfully").arg(operation).toStdString());
        QTimer::singleShot(0, this, [this, operation, result]() {
            emit operationCompleted(operation, result);
        });
    } else {
        LOG_MODULE_ERROR("BurnDevice", QString("Operation %1 failed: %2").arg(operation).arg(error).toStdString());
        QTimer::singleShot(0, this, [this, operation, error]() {
            emit operationFailed(operation, error);
        });
    }
}

void BurnDevice::failAllPendingRequests(const QString &reason)
{
    QStringList failedOperations;
    
    {
        QMutexLocker locker(&m_requestMutex);
        
        // 收集所有要失败的操作名称
        while (!m_requestQueue.isEmpty()) {
            auto request = m_requestQueue.dequeue();
            failedOperations.append(request->operation);
            delete request;
        }
        
        // 收集所有待处理的请求
        for (auto it = m_pendingRequests.begin(); it != m_pendingRequests.end(); ++it) {
            auto request = it.value();
            failedOperations.append(request->operation);
            
            if (request->timeoutTimer) {
                request->timeoutTimer->stop();
            }
            delete request;
        }
        
        m_pendingRequests.clear();
    }
    
    // 在锁外发送所有失败信号
    for (const QString &operation : failedOperations) {
        QTimer::singleShot(0, this, [this, operation, reason]() {
            emit operationFailed(operation, reason);
        });
    }
}

QByteArray BurnDevice::createPacket(const QByteArray &jsonData)
{
    QByteArray header;
    header.resize(CLIENT_HEADER_LENGTH);
    header.fill(0);
    
    // 魔数 (大端序)
    quint32 magic = qToBigEndian(CLIENT_MAGIC_NUMBER);
    memcpy(header.data(), &magic, 4);
    
    // 协议头版本 (大端序)
    quint16 version = qToBigEndian(CLIENT_HEADER_VERSION);
    memcpy(header.data() + 4, &version, 2);
    
    // 载荷长度 (大端序)
    quint32 payloadLength = qToBigEndian(static_cast<quint32>(jsonData.size()));
    memcpy(header.data() + 6, &payloadLength, 4);
    
    return header + jsonData;
}

qint64 BurnDevice::generateRequestId()
{
    return m_nextRequestId.fetch_add(1);
}

// Aprog.exe程序管理方法
QString BurnDevice::getAprogPath() const
{
    QMutexLocker locker(&m_configMutex);
    return m_aprogPath;
}

void BurnDevice::setAprogPath(const QString &path)
{
    {
        QMutexLocker locker(&m_configMutex);
        m_aprogPath = path;
        // 同时更新配置对象
        m_config["aprogPath"] = path;
    }
    
    LOG_MODULE_INFO("BurnDevice", QString("Aprog path updated: %1").arg(path).toStdString());
}

bool BurnDevice::isAprogRunning() const
{
    return m_isAprogRunning && m_aprogProcess && 
           (m_aprogProcess->state() == QProcess::Running);
}

void BurnDevice::startAprog()
{
    if (isAprogRunning()) {
        LOG_MODULE_INFO("BurnDevice", "Aprog.exe is already running");
        return;
    }
    
    QString aprogPath;
    {
        QMutexLocker locker(&m_configMutex);
        aprogPath = m_aprogPath;
    }
    
    if (aprogPath.isEmpty()) {
        LOG_MODULE_ERROR("BurnDevice", "Aprog path is empty");
        emit aprogError(QProcess::FailedToStart);
        return;
    }
    
    QFileInfo fileInfo(aprogPath);
    if (!fileInfo.exists()) {
        LOG_MODULE_ERROR("BurnDevice", QString("Aprog.exe not found at: %1").arg(aprogPath).toStdString());
        emit aprogError(QProcess::FailedToStart);
        return;
    }
    
    LOG_MODULE_INFO("BurnDevice", QString("Starting Aprog.exe: %1 -r").arg(aprogPath).toStdString());
    
    // 设置工作目录为Aprog.exe所在目录
    m_aprogProcess->setWorkingDirectory(fileInfo.absolutePath());
    
    // 启动程序，使用-r参数
    QStringList arguments;
    arguments << "-r";
    
    // 【关键修复】移除同步等待，改为异步启动
    // 进程启动成功与否会通过 onAprogStarted() 和 onAprogError() 信号通知
    m_aprogProcess->start(aprogPath, arguments);
    
    // 【移除阻塞调用】不再使用 waitForStarted()
    // if (!m_aprogProcess->waitForStarted(5000)) {
    //     LOG_MODULE_ERROR("BurnDevice", QString("Failed to start Aprog.exe: %1").arg(m_aprogProcess->errorString()).toStdString());
    //     emit aprogError(m_aprogProcess->error());
    // }
}

void BurnDevice::stopAprog()
{
    if (!m_aprogProcess || m_aprogProcess->state() == QProcess::NotRunning) {
        m_isAprogRunning = false;
        return;
    }
    
    LOG_MODULE_INFO("BurnDevice", "Stopping Aprog.exe...");
    
    // 先尝试正常终止
    m_aprogProcess->terminate();
    
    // 等待5秒让程序正常退出
    if (!m_aprogProcess->waitForFinished(5000)) {
        LOG_MODULE_WARNING("BurnDevice", "Aprog.exe did not terminate gracefully, killing...");
        m_aprogProcess->kill();
        m_aprogProcess->waitForFinished(2000);
    }
    
    m_isAprogRunning = false;
    LOG_MODULE_INFO("BurnDevice", "Aprog.exe stopped");
}

// Aprog.exe进程事件处理槽函数
void BurnDevice::onAprogStarted()
{
    m_isAprogRunning = true;
    LOG_MODULE_INFO("BurnDevice", "Aprog.exe started successfully");
    emit aprogStarted();
}

void BurnDevice::onAprogFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_isAprogRunning = false;
    
    QString statusStr = (exitStatus == QProcess::NormalExit) ? "normally" : "crashed";
    LOG_MODULE_INFO("BurnDevice", QString("Aprog.exe finished %1 with exit code %2").arg(statusStr).arg(exitCode).toStdString());
    
    emit aprogFinished(exitCode, exitStatus);
    
    // 如果程序异常退出，断开TCP连接
    if (exitStatus == QProcess::CrashExit) {
        LOG_MODULE_WARNING("BurnDevice", "Aprog.exe crashed, disconnecting from server");
        disconnectFromServer();
        m_atomicStatus.store(IDevice::DeviceStatus::Error);
        emit statusChanged(IDevice::DeviceStatus::Error);
    }
}

void BurnDevice::onAprogError(QProcess::ProcessError error)
{
    m_isAprogRunning = false;
    
    QString errorStr;
    switch (error) {
        case QProcess::FailedToStart:
            errorStr = "Failed to start";
            break;
        case QProcess::Crashed:
            errorStr = "Crashed";
            break;
        case QProcess::Timedout:
            errorStr = "Timed out";
            break;
        case QProcess::WriteError:
            errorStr = "Write error";
            break;
        case QProcess::ReadError:
            errorStr = "Read error";
            break;
        case QProcess::UnknownError:
        default:
            errorStr = "Unknown error";
            break;
    }
    
    LOG_MODULE_ERROR("BurnDevice", QString("Aprog.exe error: %1").arg(errorStr).toStdString());
    emit aprogError(error);
    
    // 程序启动失败或崩溃时，更新设备状态
    if (error == QProcess::FailedToStart || error == QProcess::Crashed) {
        m_atomicStatus.store(IDevice::DeviceStatus::Error);
        emit statusChanged(IDevice::DeviceStatus::Error);
        emit operationFailed("aprog", QString("Aprog.exe error: %1").arg(errorStr));
    }
}

void BurnDevice::onAprogReadyReadStandardOutput()
{
    if (!m_aprogProcess) return;
    
    QByteArray data = m_aprogProcess->readAllStandardOutput();
    QString output = QString::fromLocal8Bit(data).trimmed();
    
    if (!output.isEmpty()) {
        LOG_MODULE_DEBUG("BurnDevice", QString("Aprog.exe stdout: %1").arg(output).toStdString());
        emit aprogOutputReceived(output);
    }
}

void BurnDevice::onAprogReadyReadStandardError()
{
    if (!m_aprogProcess) return;
    
    QByteArray data = m_aprogProcess->readAllStandardError();
    QString output = QString::fromLocal8Bit(data).trimmed();
    
    if (!output.isEmpty()) {
        LOG_MODULE_WARNING("BurnDevice", QString("Aprog.exe stderr: %1").arg(output).toStdString());
        emit aprogOutputReceived(QString("[ERROR] %1").arg(output));
    }
}

} // namespace Domain 
