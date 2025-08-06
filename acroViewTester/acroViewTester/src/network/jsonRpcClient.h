#ifndef JSONRPCCLIENT_H
#define JSONRPCCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QByteArray>
#include <QHash>
#include <QTimer>
#include <functional> // For std::function
#include <QtEndian>
#include <QJsonParseError>
#include <QJsonArray> // Include if params can be arrays
#include <QDebug>     // For logging/debugging
// 与服务器匹配的常量
static const quint32 CLIENT_MAGIC_NUMBER = 0x4150524F;//MagicNumber ("APRO")
static const quint16 CLIENT_HEADER_VERSION = 1;//协议版本号，2字节
static const int CLIENT_HEADER_LENGTH = 32;//MagicNumber（4字节）+协议版本号（2字节）+请求ID（8字节）+数据长度（4字节）= 32字节
static const QString CLIENT_JSONRPC_VERSION = "2.0";//jsonrpc版本号

class JsonRpcClient : public QObject
{
    Q_OBJECT

public:
    explicit JsonRpcClient(QObject* parent = nullptr);
    virtual ~JsonRpcClient();

    // 连接到服务器
    void connectToServer(const QString& host, quint16 port);
    // 断开连接
    void disconnectFromServer();
    // 检查是否连接
    bool isConnected() const;

    // 发送 JSON-RPC 请求 (使用回调处理响应)
    // 返回请求 ID，如果发送失败则返回 -1
    qint64 sendRequest(const QString& method, const QJsonValue params = QJsonValue(),
        std::function<void(const QJsonValue& result)> successCallback = nullptr,
        std::function<void(int code, const QString& message, const QJsonValue& data)> errorCallback = nullptr,
        int timeoutMsecs = 5000); // 超时时间，默认5秒

// 发送 JSON-RPC 通知 (不需要响应)
    bool sendNotification(const QString& method, const QJsonValue params = QJsonValue());


signals:
    // 连接/断开信号
    void connected();
    void disconnected();
    // Socket 错误信号
    void socketError(QAbstractSocket::SocketError socketError);
    // 协议错误信号（例如，收到无效帧）
    void protocolError(const QString& errorMessage);
    // 通用的响应/错误信号 (如果不想用回调)
    void responseReceived(qint64 id, const QJsonValue& result);
    void errorReceived(qint64 id, int code, const QString& message, const QJsonValue& data);
    // 收到服务器主动发送的命令 (对应服务器的 SendClientDoCmd)
    void serverCommandReceived(const QString& cmd, const QJsonObject& data);
    // 新增信号，专门用于通知
    void notificationReceived(const QString& method, const QJsonObject& params);


private slots:
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);
    void onReadyRead();
    // void onRequestTimeout();

private:
    // 处理从缓冲区解析出的完整 JSON Payload
    void processPayload(const QByteArray& payload);
    // 发送原始数据包
    bool sendPacket(const QByteArray& jsonData);
    // 生成唯一的请求 ID
    qint64 generateRequestId();

    // 内部结构体，用于跟踪待处理的请求及其回调和定时器
    struct PendingRequest {
        qint64 id;
        QString method;
        std::function<void(const QJsonValue& result)> successCb;
        std::function<void(int code, const QString& message, const QJsonValue& data)> errorCb;
        QTimer* timer; // 用于处理超时的定时器
    };

    QTcpSocket* m_socket;
    QByteArray m_buffer; // 接收缓冲区
    qint64 m_nextRequestId; // 用于生成请求 ID
    QHash<qint64, PendingRequest> m_pendingRequests; // 存储待处理的请求

    bool isNotification(const QJsonObject& obj) const;
    bool isResponse(const QJsonObject& obj) const;
    bool isServerCommand(const QJsonObject& obj) const;
    void processNotification(const QJsonObject& obj);
    void processResponse(const QJsonObject& obj);
    void processServerCommand(const QJsonObject& obj);

    private:
    bool m_isConnecting = false;

public:
    bool isConnecting() const { return m_isConnecting; }

};

#endif // JSONRPCCLIENT_H
