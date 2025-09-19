#include "JsonRpcClient.h"
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QHostAddress>
#include <QNetworkProxy>
#include <QtEndian>
#include <QDateTime>

// 静态常量定义
const QString JsonRpcClient::JSONRPC_VERSION = "2.0";

JsonRpcClient::JsonRpcClient(QObject *parent)
    : QObject(parent)
    , m_socket(nullptr)
    , m_port(0)
    , m_connectionState(Disconnected)
    , m_autoReconnect(false)
    , m_reconnectTimer(nullptr)
    , m_reconnectInterval(5000) // 默认5秒重连间隔
    , m_nextRequestId(1)
{
    // 初始化TCP套接字
    m_socket = new QTcpSocket(this);
    
    // 连接网络信号
    connect(m_socket, &QTcpSocket::connected, this, &JsonRpcClient::onSocketConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &JsonRpcClient::onSocketDisconnected);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &JsonRpcClient::onSocketError);
    connect(m_socket, &QTcpSocket::readyRead, this, &JsonRpcClient::onSocketReadyRead);
    
    // 初始化重连定时器
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &JsonRpcClient::onReconnectTimer);
    
    qDebug() << "JsonRpcClient 初始化完成";
}

JsonRpcClient::~JsonRpcClient()
{
    disconnectFromServer();
    qDebug() << "JsonRpcClient 已销毁";
}

// === 连接管理实现 ===

void JsonRpcClient::connectToServer(const QString& host, quint16 port, bool autoReconnect)
{
    if (m_connectionState == Connected || m_connectionState == Connecting) {
        qWarning() << "已经连接或正在连接中，无需重复连接";
        return;
    }
    
    m_host = host;
    m_port = port;
    m_autoReconnect = autoReconnect;
    
    setConnectionState(Connecting);
    stopReconnectTimer();
    
    qDebug() << QString("正在连接到服务器 %1:%2").arg(host).arg(port);
    m_socket->connectToHost(host, port);
}

void JsonRpcClient::disconnectFromServer()
{
    m_autoReconnect = false;
    stopReconnectTimer();
    
    if (m_socket && m_socket->state() != QAbstractSocket::UnconnectedState) {
        qDebug() << "正在断开服务器连接";
        m_socket->disconnectFromHost();
        if (m_socket->state() != QAbstractSocket::UnconnectedState) {
            m_socket->waitForDisconnected(3000);
        }
    }
    
    setConnectionState(Disconnected);
}

// === 网络事件处理 ===

void JsonRpcClient::onSocketConnected()
{
    qDebug() << QString("已成功连接到服务器 %1:%2").arg(m_host).arg(m_port);
    m_receiveBuffer.clear();
    setConnectionState(Connected);
    stopReconnectTimer();
}

void JsonRpcClient::onSocketDisconnected()
{
    qDebug() << "与服务器的连接已断开";
    m_receiveBuffer.clear();
    
    // 清理待处理的回调
    for (auto it = m_pendingCallbacks.begin(); it != m_pendingCallbacks.end(); ++it) {
        if (it.value()) {
            it.value()(false, QJsonObject(), "连接已断开");
        }
    }
    m_pendingCallbacks.clear();
    
    if (m_autoReconnect && m_connectionState != Disconnected) {
        setConnectionState(Reconnecting);
        startReconnectTimer();
    } else {
        setConnectionState(Disconnected);
    }
}

void JsonRpcClient::onSocketError(QAbstractSocket::SocketError error)
{
    QString errorString = m_socket->errorString();
    qWarning() << QString("网络错误: %1 (%2)").arg(errorString).arg(error);
    emit errorOccurred(errorString);
    
    if (m_autoReconnect && error != QAbstractSocket::HostNotFoundError) {
        if (m_connectionState != Reconnecting) {
            setConnectionState(Reconnecting);
            startReconnectTimer();
        }
    }
}

void JsonRpcClient::onSocketReadyRead()
{
    if (!m_socket) return;
    
    m_receiveBuffer.append(m_socket->readAll());
    processReceivedData();
}

void JsonRpcClient::onReconnectTimer()
{
    if (m_connectionState == Reconnecting) {
        qDebug() << "尝试重新连接到服务器";
        setConnectionState(Connecting);
        m_socket->connectToHost(m_host, m_port);
    }
}

// === 数据处理实现 ===

void JsonRpcClient::processReceivedData()
{
    while (true) {
        // 检查是否有足够的数据来解析头部
        if (m_receiveBuffer.size() < HEADER_LENGTH) {
            break; // 等待更多数据
        }
        
        // 解析32字节头部
        QByteArray header = m_receiveBuffer.left(HEADER_LENGTH);
        
        // 读取头部字段（大端序）
        quint32 magicNumber = qFromBigEndian<quint32>(reinterpret_cast<const uchar*>(header.constData()));
        quint16 version = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(header.constData() + 4));
        quint32 payloadLength = qFromBigEndian<quint32>(reinterpret_cast<const uchar*>(header.constData() + 6));
        
        // 验证头部
        if (magicNumber != MAGIC_NUMBER) {
            qCritical() << QString("收到无效的Magic Number: 0x%1").arg(magicNumber, 8, 16, QChar('0'));
            emit errorOccurred("协议错误：无效的Magic Number");
            disconnectFromServer();
            return;
        }
        
        if (version != HEADER_VERSION) {
            qCritical() << QString("不支持的协议版本: %1").arg(version);
            emit errorOccurred(QString("协议错误：不支持的版本 %1").arg(version));
            disconnectFromServer();
            return;
        }
        
        // 检查载荷长度是否合理
        const quint32 MAX_PAYLOAD_SIZE = 16 * 1024 * 1024; // 16MB
        if (payloadLength > MAX_PAYLOAD_SIZE) {
            qCritical() << QString("载荷长度过大: %1").arg(payloadLength);
            emit errorOccurred("协议错误：载荷长度超出限制");
            disconnectFromServer();
            return;
        }
        
        // 检查是否接收到完整的数据包
        int totalPacketSize = HEADER_LENGTH + payloadLength;
        if (m_receiveBuffer.size() < totalPacketSize) {
            break; // 等待更多数据
        }
        
        // 提取JSON载荷
        QByteArray jsonData = m_receiveBuffer.mid(HEADER_LENGTH, payloadLength);
        
        // 解析JSON
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
        
        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "JSON解析失败:" << parseError.errorString();
            emit errorOccurred("JSON解析错误: " + parseError.errorString());
        } else {
            QJsonObject jsonObj = doc.object();
            processJsonRpcMessage(jsonObj);
        }
        
        // 移除已处理的数据
        m_receiveBuffer.remove(0, totalPacketSize);
    }
}

void JsonRpcClient::processJsonRpcMessage(const QJsonObject& message)
{
    qDebug() << "收到服务器消息:" << QJsonDocument(message).toJson(QJsonDocument::Compact);
    
    // 检查是否是通知消息（没有ID的消息）
    if (!message.contains("id")) {
        // 这是一个通知
        QString method = message.value("method").toString();
        // 兼容：优先使用标准 JSON-RPC 的 params；若无，则回退到 result
        QJsonObject params;
        if (message.contains("params") && message.value("params").isObject()) {
            params = message.value("params").toObject();
        } else {
            params = message.value("result").toObject();
        }
        
        if (!method.isEmpty()) {
            qDebug() << QString("收到服务器通知: %1").arg(method);
            emit notificationReceived(method, params);
            
            if (m_notificationCallback) {
                m_notificationCallback(method, params);
            }
        }
    } else {
        // 这是一个响应消息
        QString id = message.value("id").toString();
        
        if (m_pendingCallbacks.contains(id)) {
            RpcCallback callback = m_pendingCallbacks.take(id);
            
            if (message.contains("error")) {
                // 错误响应
                QJsonObject errorObj = message.value("error").toObject();
                int errorCode = errorObj.value("code").toInt();
                QString errorMessage = errorObj.value("message").toString();
                qWarning() << QString("RPC调用失败 [%1]: %2").arg(errorCode).arg(errorMessage);
                
                if (callback) {
                    callback(false, QJsonObject(), errorMessage);
                }
            } else {
                // 成功响应
                QJsonObject result = message.value("result").toObject();
                qDebug() << QString("RPC调用成功，ID: %1").arg(id);
                
                if (callback) {
                    callback(true, result, QString());
                }
            }
        }
    }
}

// === RPC方法发送实现 ===

void JsonRpcClient::sendJsonRpcRequest(const QString& method, const QJsonObject& params, const RpcCallback& callback)
{
    if (m_connectionState != Connected) {
        qWarning() << "无法发送RPC请求：未连接到服务器";
        if (callback) {
            callback(false, QJsonObject(), "未连接到服务器");
        }
        return;
    }
    
    QString requestId = generateRequestId();
    
    // 打印请求日志到控制台
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString requestLog = QString("[%1] REQUEST: [请求 #%2] %3")
                          .arg(timestamp)
                          .arg(requestId)
                          .arg(method);
    
    qDebug().noquote() << requestLog;
    
    // 如果有参数，也打印出来
    if (!params.isEmpty()) {
        QJsonDocument paramDoc(params);
        QString paramLog = QString("请求参数: %1").arg(QString::fromUtf8(paramDoc.toJson(QJsonDocument::Compact)));
        qDebug().noquote() << paramLog;
    }
    
    // 构造JSON-RPC请求
    QJsonObject request;
    request["jsonrpc"] = JSONRPC_VERSION;
    request["method"] = method;
    request["params"] = params;
    request["id"] = requestId;
    
    // 存储回调
    if (callback) {
        m_pendingCallbacks[requestId] = callback;
    }
    
    // 序列化JSON
    QJsonDocument doc(request);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    
    // 打印完整的JSON-RPC请求数据包
    qDebug().noquote() << QString("发送数据包: %1").arg(QString::fromUtf8(jsonData));
    
    // 构造协议头部（32字节）
    QByteArray packet;
    packet.reserve(HEADER_LENGTH + jsonData.size());
    
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    
    // 写入头部字段
    stream << MAGIC_NUMBER;          // Magic Number (4字节)
    stream << HEADER_VERSION;        // 版本 (2字节)
    stream << static_cast<quint32>(jsonData.size()); // 载荷长度 (4字节)
    
    // 添加22字节的保留字段
    packet.append(QByteArray(22, '\0'));
    
    // 打印协议头部信息
    qDebug().noquote() << QString("协议头部: Magic=0x%1, Version=%2, PayloadLength=%3, TotalSize=%4")
                          .arg(MAGIC_NUMBER, 8, 16, QChar('0'))
                          .arg(HEADER_VERSION)
                          .arg(jsonData.size())
                          .arg(HEADER_LENGTH + jsonData.size());
    
    // 添加JSON载荷
    packet.append(jsonData);
    
    // 发送数据
    qint64 bytesWritten = m_socket->write(packet);
    if (bytesWritten != packet.size()) {
        qWarning() << "数据发送不完整";
        if (callback) {
            m_pendingCallbacks.remove(requestId);
            callback(false, QJsonObject(), "数据发送失败");
        }
    } else {
        qDebug().noquote() << QString("数据包已发送: 总大小=%1字节").arg(packet.size());
    }
    
    // 打印分隔线
    qDebug().noquote() << QString("-").repeated(80);
}

// === 公共API实现 ===

void JsonRpcClient::loadProject(const QString& projectPath, const QString& taskFileName, const RpcCallback& callback)
{
    QJsonObject params;
    params["path"] = projectPath;
    params["taskFileName"] = taskFileName;
    
    sendJsonRpcRequest("LoadProject", params, callback);
}

void JsonRpcClient::siteScanAndConnect(const RpcCallback& callback)
{
    sendJsonRpcRequest("SiteScanAndConnect", QJsonObject(), callback);
}

void JsonRpcClient::getProjectInfo(const RpcCallback& callback)
{
    sendJsonRpcRequest("GetProjectInfo", QJsonObject(), callback);
}

void JsonRpcClient::getProjectInfoExt(const RpcCallback& callback)
{
    sendJsonRpcRequest("GetProjectInfoExt", QJsonObject(), callback);
}

void JsonRpcClient::getSKTInfo(const RpcCallback& callback)
{
    sendJsonRpcRequest("GetSKTInfo", QJsonObject(), callback);
}

void JsonRpcClient::sendUUID(const QString& uuid, const QString& strIP, quint32 hopNum, quint32 sktEnable, const RpcCallback& callback)
{
    QJsonObject params;
    params["uuid"] = uuid;
    params["strIP"] = strIP;
    params["nHopNum"] = static_cast<qint64>(hopNum);
    params["sktEnable"] = static_cast<qint64>(sktEnable);
    
    sendJsonRpcRequest("SendUUID", params, callback);
}

void JsonRpcClient::doJob(const QString& strIP, quint32 sktEnable, const QJsonObject& cmdSequence, const RpcCallback& callback)
{
    QJsonObject params;
    params["strIP"] = strIP;
    params["sktEn"] = static_cast<qint64>(sktEnable);
    params["docmdSeqJson"] = cmdSequence;
    
    sendJsonRpcRequest("DoJob", params, callback);
}

void JsonRpcClient::doCustom(const QJsonObject& params, const RpcCallback& callback)
{
    sendJsonRpcRequest("DoCustom", params, callback);
}

void JsonRpcClient::callRpcMethod(const QString& method, const QJsonObject& params, const RpcCallback& callback)
{
    sendJsonRpcRequest(method, params, callback);
}

// === 辅助方法实现 ===

void JsonRpcClient::setConnectionState(ConnectionState state)
{
    if (m_connectionState != state) {
        ConnectionState oldState = m_connectionState;
        m_connectionState = state;
        
        qDebug() << QString("连接状态变更: %1 -> %2").arg(oldState).arg(state);
        emit connectionStateChanged(state);
    }
}

void JsonRpcClient::startReconnectTimer()
{
    if (m_reconnectTimer && !m_reconnectTimer->isActive()) {
        qDebug() << QString("启动重连定时器，间隔: %1ms").arg(m_reconnectInterval);
        m_reconnectTimer->start(m_reconnectInterval);
    }
}

void JsonRpcClient::stopReconnectTimer()
{
    if (m_reconnectTimer && m_reconnectTimer->isActive()) {
        qDebug() << "停止重连定时器";
        m_reconnectTimer->stop();
    }
}

QString JsonRpcClient::generateRequestId()
{
    return QString::number(m_nextRequestId++);
} 