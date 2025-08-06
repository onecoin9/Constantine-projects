#include "JsonRpcClient.h"


JsonRpcClient::JsonRpcClient(QObject* parent)
    : QObject(parent), m_socket(new QTcpSocket(this)), m_nextRequestId(1)
{
    connect(m_socket, &QTcpSocket::connected, this, &JsonRpcClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &JsonRpcClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &JsonRpcClient::onReadyRead);
    // Use the new QTcpSocket error signal syntax
    connect(m_socket, &QTcpSocket::errorOccurred, this, &JsonRpcClient::onErrorOccurred);

    m_buffer.clear();
    m_pendingRequests.clear();
}

JsonRpcClient::~JsonRpcClient()
{
    disconnectFromServer();
    // Clean up any remaining timers
   // qDeleteAll(m_pendingRequests);
    m_pendingRequests.clear();
}

void JsonRpcClient::connectToServer(const QString& host, quint16 port)
{
    //if (m_socket->state() == QAbstractSocket::UnconnectedState) {
        //qDebug() << "JsonRpcClient: Connecting to" << host << ":" << port;
       // m_buffer.clear(); // 清空缓冲区以便新连接
        //m_socket->connectToHost(host, port);
    //}
    //else {
        //qWarning() << "JsonRpcClient: Already connected or connecting.";
    //}
if (isConnected() || m_isConnecting) {
		qWarning() << "JsonRpcClient: Already connected or connecting.";
		return;
	}

	m_isConnecting = true;
	m_socket->connectToHost(host, port);
}

void JsonRpcClient::disconnectFromServer()
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        qDebug() << "JsonRpcClient: Disconnecting...";
        // Abort connection cleanly
        m_socket->abort(); // Use abort to immediately close and reset
    }
    // Clean up pending requests upon disconnection
    //qDeleteAll(m_pendingRequests); // Deletes timers associated with requests
    m_pendingRequests.clear();
    m_buffer.clear();
}

bool JsonRpcClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

qint64 JsonRpcClient::sendRequest(const QString& method, const QJsonValue params,
    std::function<void(const QJsonValue& result)> successCallback,
    std::function<void(int code, const QString& message, const QJsonValue& data)> errorCallback,
    int timeoutMsecs)
{
    if (!isConnected()) {
        qWarning() << "JsonRpcClient: Cannot send request, not connected.";
        return -1;
    }

    qint64 id = generateRequestId();

    QJsonObject requestObj;
    requestObj["jsonrpc"] = CLIENT_JSONRPC_VERSION;
    requestObj["method"] = method;
    if (!params.isNull() && !params.isUndefined()) {
        requestObj["params"] = params;
    }
    requestObj["id"] = id;

    QJsonDocument doc(requestObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    if (sendPacket(jsonData)) {
        PendingRequest pending;
        pending.id = id;
        pending.method = method;
        pending.successCb = successCallback;
        pending.errorCb = errorCallback;

        // Setup timeout timer
        pending.timer = new QTimer(this);
        pending.timer->setSingleShot(true);
        // Use lambda to capture necessary context for timeout handling
        connect(pending.timer, &QTimer::timeout, this, [this, id]() {
            // Find the request by id and handle timeout
            if (m_pendingRequests.contains(id)) {
                PendingRequest req = m_pendingRequests.take(id);
                qWarning() << "JsonRpcClient: Request" << id << "(method:" << req.method << ") timed out.";
                if (req.errorCb) {
                    req.errorCb(-32000, "Request timed out", QJsonValue()); // Example timeout error
                }
                else {
                    emit errorReceived(id, -32000, "Request timed out", QJsonValue());
                }
                delete req.timer; // Clean up timer
            }
            });
        pending.timer->start(timeoutMsecs);

        m_pendingRequests.insert(id, pending);
        qDebug() << "JsonRpcClient: Sent request id=" << id << "method=" << method;
        return id;
    }
    else {
        return -1;
    }
}

bool JsonRpcClient::sendNotification(const QString& method, const QJsonValue params)
{
    if (!isConnected()) {
        qWarning() << "JsonRpcClient: Cannot send notification, not connected.";
        return false;
    }

    QJsonObject notificationObj;
    notificationObj["jsonrpc"] = CLIENT_JSONRPC_VERSION;
    notificationObj["method"] = method;
    if (!params.isNull() && !params.isUndefined()) {
        notificationObj["params"] = params;
    }
    // No "id" field for notifications

    QJsonDocument doc(notificationObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    qDebug() << "JsonRpcClient: Sent notification method=" << method;
    return sendPacket(jsonData);
}


bool JsonRpcClient::sendPacket(const QByteArray& jsonData)
{
    if (!isConnected()) return false;

    QByteArray header;
    header.resize(CLIENT_HEADER_LENGTH);
    header.fill(0); // Fill reserved bytes with 0

    // 将 magic 声明为 constexpr
    static constexpr quint32 magic = qToBigEndian(CLIENT_MAGIC_NUMBER);
    memcpy(header.data(), &magic, 4);

    // Header Version (Big Endian)
    static constexpr quint16 version = qToBigEndian(CLIENT_HEADER_VERSION);
    memcpy(header.data() + 4, &version, 2);

    // Payload Length (Big Endian)
    quint32 payloadLength = qToBigEndian(static_cast<quint32>(jsonData.size()));
    memcpy(header.data() + 6, &payloadLength, 4);

    // Combine header and payload
    QByteArray packet = header + jsonData;

    qint64 bytesWritten = m_socket->write(packet);
    if (bytesWritten != packet.size()) {
        qWarning() << "JsonRpcClient: Failed to write entire packet to socket. Written:" << bytesWritten << "Expected:" << packet.size();
        // Consider error handling or retrying logic here
        return false;
    }
    // Ensure data is sent immediately (optional, depends on application needs)
    // m_socket->flush();
    return true;
}

void JsonRpcClient::onConnected()
{
	qDebug() << "JsonRpcClient: Connected to server.";
	m_isConnecting = false;
	emit connected();
}

void JsonRpcClient::onDisconnected()
{
    qWarning() << "JsonRpcClient: Disconnected from server.";
    // Clean up pending requests on disconnect
// qDeleteAll(m_pendingRequests); // Stops and deletes timers
    m_pendingRequests.clear();
    m_buffer.clear(); // Clear buffer on disconnect
    emit disconnected();
}

void JsonRpcClient::onErrorOccurred(QAbstractSocket::SocketError socketErrortemp)
{
	qWarning() << "JsonRpcClient: Socket error:" << m_socket->errorString();
	// Optionally clean up pending requests on critical errors
	// qDeleteAll(m_pendingRequests);
	// m_pendingRequests.clear();
	// m_buffer.clear();
	m_isConnecting = false;
	emit socketError(socketErrortemp);
}

void JsonRpcClient::onReadyRead()
{
    // Append all available data to the buffer
    m_buffer.append(m_socket->readAll());

    // Process buffer for complete packets
    while (true) {
        if (m_buffer.size() < CLIENT_HEADER_LENGTH) {
            // Not enough data for a header yet
            break;
        }

        // Peek at the header without removing it yet
        QByteArray header = m_buffer.left(CLIENT_HEADER_LENGTH);

        // 1. Check Magic Number
        quint32 receivedMagic;
        memcpy(&receivedMagic, header.constData(), 4);
        receivedMagic = qFromBigEndian(receivedMagic);
        if (receivedMagic != CLIENT_MAGIC_NUMBER) {
            qWarning() << "JsonRpcClient: Invalid magic number received. Disconnecting.";
            emit protocolError("Invalid magic number");
            disconnectFromServer();
            return; // Stop processing buffer after disconnect
        }

        // 2. Check Header Version
        quint16 receivedVersion;
        memcpy(&receivedVersion, header.constData() + 4, 2);
        receivedVersion = qFromBigEndian(receivedVersion);
        if (receivedVersion != CLIENT_HEADER_VERSION) {
            qWarning() << "JsonRpcClient: Unsupported header version received:" << receivedVersion << ". Disconnecting.";
            emit protocolError(QString("Unsupported header version: %1").arg(receivedVersion));
            disconnectFromServer();
            return; // Stop processing buffer after disconnect
        }

        // 3. Get Payload Length
        quint32 payloadLength;
        memcpy(&payloadLength, header.constData() + 6, 4);
        payloadLength = qFromBigEndian(payloadLength);

        // Check if the complete packet (header + payload) is available
        int totalPacketSize = CLIENT_HEADER_LENGTH + payloadLength;
        if (m_buffer.size() < totalPacketSize) {
            // Need more data for this packet
            break;
        }

        // Extract the JSON payload
        QByteArray payload = m_buffer.mid(CLIENT_HEADER_LENGTH, payloadLength);

        // Remove the processed packet from the buffer
        m_buffer.remove(0, totalPacketSize);

        // Process the extracted JSON payload
        processPayload(payload);
    } // End while loop
}

void JsonRpcClient::processPayload(const QByteArray& payload)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
    QString jsonString = doc.toJson(QJsonDocument::Indented);
    qDebug() << jsonString;
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JsonRpcClient: JSON parse error:" << parseError.errorString() << "Payload:" << payload;
        emit protocolError(QString("JSON parse error: %1").arg(parseError.errorString()));
        // Decide how to handle parse errors, maybe disconnect or just log
        return;
    }

    if (!doc.isObject()) {
        qWarning() << "JsonRpcClient: Received JSON is not an object:" << payload;
        emit protocolError("Received JSON is not an object");
        return;
    }

	QJsonObject obj = doc.object();

	QJsonDocument debugDoc(obj);
	qDebug() << "JsonRpcClient: Object content:";
	qDebug() << debugDoc.toJson(QJsonDocument::Indented);

	// 检查 jsonrpc 版本，仅当字段存在时才检查
	if (obj.contains("jsonrpc") && obj["jsonrpc"].toString() != CLIENT_JSONRPC_VERSION) {
		qWarning() << "JsonRpcClient: Received message with invalid 'jsonrpc' version:" << payload;
		emit protocolError("Invalid 'jsonrpc' version in received message");
		return;
	}

	// 使用辅助函数确定消息类型
	if (isNotification(obj)) {
		processNotification(obj);
	}
	else if (isResponse(obj)) {
		processResponse(obj);
	}
	else if (isServerCommand(obj)) {
		processServerCommand(obj);
	}
	else {
		qWarning() << "JsonRpcClient: Received unknown JSON-RPC message type:" << payload;
		emit protocolError("Received unknown JSON-RPC message type");
	}
}

// 判断消息是否为通知
bool JsonRpcClient::isNotification(const QJsonObject& obj) const
{
	// 通知必须包含 method 字段，且不包含 id 字段或 id 为 null
	return obj.contains("method") && (!obj.contains("id") || obj["id"].isNull());
}

// 判断消息是否为响应
bool JsonRpcClient::isResponse(const QJsonObject& obj) const
{
	// 响应必须包含 id 字段且不为 null，并包含 result 或 error 字段
	return obj.contains("id") && !obj["id"].isNull() &&
		(obj.contains("result") || obj.contains("error"));
}

// 判断消息是否为服务器命令（特殊格式）
bool JsonRpcClient::isServerCommand(const QJsonObject& obj) const
{
	// 特殊格式：包含 result 字段，id 为 null，result 中包含 cmd 字段
	return obj.contains("result") &&
		obj.value("id").isNull() &&
		obj["result"].isObject() &&
		obj["result"].toObject().contains("cmd");
}

// 处理通知消息
void JsonRpcClient::processNotification(const QJsonObject& obj)
{
	QString method = obj["method"].toString();
	qDebug() << "JsonRpcClient: Received notification with method:" << method;

	// 检查并处理 params
	if (obj.contains("params") && obj["params"].isObject()) {
		// 发出通知信号
		emit notificationReceived(method, obj["params"].toObject());

		// 为了向后兼容，也发出 responseReceived 信号
		emit responseReceived(0, obj);
	}
	else {
		qWarning() << "JsonRpcClient: Missing or invalid 'params' for notification with method:" << method;
		emit protocolError("Missing or invalid params for notification");
	}
}

// 处理响应消息
void JsonRpcClient::processResponse(const QJsonObject& obj)
{
	qint64 id = obj["id"].toVariant().toLongLong();

	if (obj.contains("result")) {
		// 处理成功响应
		qDebug() << "JsonRpcClient: Received standard result for id=" << id;

		if (m_pendingRequests.contains(id)) {
			PendingRequest pending = m_pendingRequests.take(id);
			pending.timer->stop();
			delete pending.timer;

			if (pending.successCb) {
				pending.successCb(obj["result"]);
			}
		}
		else {
			qDebug() << "JsonRpcClient: No pending request for id=" << id << ", but still emitting response";
		}

		// 发送响应信号
		emit responseReceived(id, obj);
	}
	else if (obj.contains("error")) {
		// 处理错误响应
		QJsonObject errorObj = obj["error"].toObject();
		int code = errorObj.value("code").toInt(-1);
		QString message = errorObj.value("message").toString("Unknown error");
		QJsonValue data = errorObj.value("data");

		qWarning() << "JsonRpcClient: Received error for id=" << id << "code=" << code << "message=" << message;

		if (m_pendingRequests.contains(id)) {
			PendingRequest pending = m_pendingRequests.take(id);
			pending.timer->stop();
			delete pending.timer;

			if (pending.errorCb) {
				pending.errorCb(code, message, data);
			}
		}

		// 发送错误信号
		emit errorReceived(id, code, message, data);
	}
	else {
		// 非标准响应
		qWarning() << "JsonRpcClient: Received invalid response object (no result or error) for id=" << id;
		emit protocolError(QString("Invalid response object for id %1").arg(id));

		if (m_pendingRequests.contains(id)) {
			PendingRequest pending = m_pendingRequests.take(id);
			pending.timer->stop();
			delete pending.timer;

			if (pending.errorCb) {
				pending.errorCb(-32603, "Invalid response object received", QJsonValue());
			}
		}
	}
}

// 处理服务器命令（特殊格式）
void JsonRpcClient::processServerCommand(const QJsonObject& obj)
{
	QJsonObject resultObj = obj["result"].toObject();
	QString cmd = resultObj["cmd"].toString();
	QJsonObject data = resultObj.value("data").toObject();

	qDebug() << "JsonRpcClient: Received server command:" << cmd;

	// 创建一个模拟的通知对象
	QJsonObject notificationObj;
	notificationObj["method"] = cmd;
	notificationObj["params"] = QJsonObject{ {"cmd", cmd}, {"data", data} };

	// 发送服务器命令信号
	emit serverCommandReceived(cmd, data);

	// 为了向后兼容，也发出 responseReceived 信号
	emit responseReceived(0, notificationObj);
}
qint64 JsonRpcClient::generateRequestId()
{
    // Simple incrementing ID. Consider using atomic or UUID for more robustness
    // especially in multi-threaded clients.
    return m_nextRequestId++;
}
