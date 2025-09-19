#ifndef JSONRPCCLIENT_H
#define JSONRPCCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <functional>

/**
 * @brief JsonRpcClient - JSON-RPC 2.0 客户端
 * 
 * 这是一个与 JsonRpcServer 配套的客户端类，用于连接到基于TCP的JSON-RPC服务器。
 * 客户端实现了服务器要求的32字节二进制头部协议和JSON-RPC 2.0通信规范。
 * 
 * 主要特性：
 * - 支持TCP长连接通信
 * - 自动重连机制
 * - 异步回调处理
 * - 服务器通知接收
 * - 完整的错误处理
 */
class JsonRpcClient : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief 连接状态枚举
     */
    enum ConnectionState {
        Disconnected,   // 未连接
        Connecting,     // 连接中
        Connected,      // 已连接
        Reconnecting    // 重连中
    };
    
    /**
     * @brief RPC调用结果回调函数类型
     * @param success 是否成功
     * @param result 返回的JSON数据
     * @param errorMessage 错误信息（仅在失败时有效）
     */
    using RpcCallback = std::function<void(bool success, const QJsonObject& result, const QString& errorMessage)>;
    
    /**
     * @brief 服务器通知回调函数类型
     * @param method 通知方法名
     * @param params 通知参数
     */
    using NotificationCallback = std::function<void(const QString& method, const QJsonObject& params)>;
    
public:
    explicit JsonRpcClient(QObject *parent = nullptr);
    virtual ~JsonRpcClient();
    
    // === 连接管理 ===
    
    /**
     * @brief 连接到服务器
     * @param host 服务器主机地址
     * @param port 服务器端口
     * @param autoReconnect 是否启用自动重连
     */
    void connectToServer(const QString& host, quint16 port, bool autoReconnect = true);
    
    /**
     * @brief 断开与服务器的连接
     */
    void disconnectFromServer();
    
    /**
     * @brief 获取当前连接状态
     */
    ConnectionState getConnectionState() const { return m_connectionState; }
    
    /**
     * @brief 设置自动重连间隔（毫秒）
     */
    void setReconnectInterval(int milliseconds) { m_reconnectInterval = milliseconds; }
    
    // === 通知回调设置 ===
    
    /**
     * @brief 设置服务器通知回调函数
     */
    void setNotificationCallback(const NotificationCallback& callback) { m_notificationCallback = callback; }
    
    // === RPC方法调用接口 ===
    
    /**
     * @brief 加载项目文件
     * @param projectPath 项目路径
     * @param taskFileName 任务文件名
     * @param callback 结果回调函数
     */
    void loadProject(const QString& projectPath, const QString& taskFileName, const RpcCallback& callback = nullptr);
    
    /**
     * @brief 站点扫描和连接
     * @param callback 结果回调函数
     */
    void siteScanAndConnect(const RpcCallback& callback = nullptr);
    
    /**
     * @brief 获取项目信息
     * @param callback 结果回调函数
     */
    void getProjectInfo(const RpcCallback& callback = nullptr);
    
    /**
     * @brief 获取项目扩展信息
     * @param callback 结果回调函数
     */
    void getProjectInfoExt(const RpcCallback& callback = nullptr);
    
    /**
     * @brief 获取SKT信息
     * @param callback 结果回调函数
     */
    void getSKTInfo(const RpcCallback& callback = nullptr);
    
    /**
     * @brief 发送UUID
     * @param uuid UUID字符串
     * @param strIP IP地址
     * @param hopNum 跳数
     * @param sktEnable 座子使能
     * @param callback 结果回调函数
     */
    void sendUUID(const QString& uuid, const QString& strIP, quint32 hopNum, quint32 sktEnable, const RpcCallback& callback = nullptr);
    
    /**
     * @brief 执行作业任务
     * @param strIP 目标IP地址
     * @param sktEnable 座子使能位图
     * @param cmdSequence 命令序列JSON
     * @param callback 结果回调函数
     */
    void doJob(const QString& strIP, quint32 sktEnable, const QJsonObject& cmdSequence, const RpcCallback& callback = nullptr);
    
    /**
     * @brief 执行自定义命令
     * @param params 自定义参数
     * @param callback 结果回调函数
     */
    void doCustom(const QJsonObject& params, const RpcCallback& callback = nullptr);
    
    /**
     * @brief 通用RPC调用方法
     * @param method 方法名
     * @param params 参数对象
     * @param callback 结果回调函数
     */
    void callRpcMethod(const QString& method, const QJsonObject& params, const RpcCallback& callback = nullptr);

signals:
    /**
     * @brief 连接状态改变信号
     * @param state 新的连接状态
     */
    void connectionStateChanged(ConnectionState state);
    
    /**
     * @brief 服务器通知信号
     * @param method 通知方法名
     * @param params 通知参数
     */
    void notificationReceived(const QString& method, const QJsonObject& params);
    
    /**
     * @brief 错误发生信号
     * @param errorMessage 错误信息
     */
    void errorOccurred(const QString& errorMessage);

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onSocketReadyRead();
    void onReconnectTimer();

private:
    // 内部方法
    void setConnectionState(ConnectionState state);
    void processReceivedData();
    void processJsonRpcMessage(const QJsonObject& message);
    void sendJsonRpcRequest(const QString& method, const QJsonObject& params, const RpcCallback& callback);
    void startReconnectTimer();
    void stopReconnectTimer();
    
    // 协议相关
    static const quint32 MAGIC_NUMBER = 0x4150524F; // "APRO"
    static const quint16 HEADER_VERSION = 1;
    static const int HEADER_LENGTH = 32;
    static const QString JSONRPC_VERSION;
    
    // 网络相关
    QTcpSocket* m_socket;
    QString m_host;
    quint16 m_port;
    ConnectionState m_connectionState;
    bool m_autoReconnect;
    QTimer* m_reconnectTimer;
    int m_reconnectInterval; // 毫秒
    
    // 数据处理
    QByteArray m_receiveBuffer;
    
    // 回调管理
    QMap<QString, RpcCallback> m_pendingCallbacks;
    NotificationCallback m_notificationCallback;
    
    // 请求ID管理
    quint32 m_nextRequestId;
    QString generateRequestId();
};

#endif // JSONRPCCLIENT_H 