#ifndef BURNDEVICE_H
#define BURNDEVICE_H

#include "IDevice.h"
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMutex>
#include <QQueue>
#include <QProcess>
#include <memory>
#include <atomic>

namespace Domain {

// 长连接BurnDevice - 保持持久TCP连接
class BurnDevice : public IDevice
{
    Q_OBJECT
public:
    explicit BurnDevice(QObject *parent = nullptr);
    ~BurnDevice() override;

    // IDevice接口实现
    bool initialize() override;
    bool release() override;
    IDevice::DeviceStatus getStatus() const override;
    QString getName() const override;
    IDevice::DeviceType getType() const override;
    QString getDescription() const override;
    QJsonObject executeCommand(const QString &command, const QJsonObject &params) override;
    void setCommunicationChannel(std::shared_ptr<Infrastructure::ICommunicationChannel> channel) override;
    std::shared_ptr<Infrastructure::ICommunicationChannel> getCommunicationChannel() const override;
    void setConfiguration(const QJsonObject &config) override;
    QJsonObject getConfiguration() const override;
    bool selfTest() override;

    // BurnDevice特有方法
    QString getServerHost() const;
    quint16 getServerPort() const;
    void setServerInfo(const QString &host, quint16 port);
    bool isConnected() const;
    void requestConnection();  // 手动请求连接
    
    // Aprog.exe程序管理
    QString getAprogPath() const;
    void setAprogPath(const QString &path);
    bool isAprogRunning() const;
    void startAprog();
    void stopAprog();
    
    // JSON-RPC方法 - 返回立即结果，真正的结果通过信号返回
    QJsonObject siteScanAndConnect();
    QJsonObject loadProject(const QString &path, const QString &taskFileName);
    QJsonObject doJob(const QString &strIP, int nHopNum, int portID, int cmdFlag, 
                      int cmdID, quint32 sktEn, int bpuID, const QJsonObject &docmdSeqJson);
    QJsonObject getProjectInfo();
    QJsonObject doCustom(const QString &strIP, int nHopNum, int portID, int cmdFlag, 
                         int cmdID, quint32 sktEn, int bpuID, const QJsonObject &doCustomData);
    QJsonObject getProjectInfoExt(const QString &projectUrl);
    QJsonObject getSKTInfo();
    QJsonObject sendUUID(const QString &uuid, const QString &strIP, int nHopNum, quint32 sktEnable);

signals:
    void operationCompleted(const QString &operation, const QJsonObject &result);
    void operationFailed(const QString &operation, const QString &error);
    void connected();
    void disconnected();
    void serverCommandReceived(const QString &command, const QJsonObject &data);
    void protocolError(const QString &error);
    //data为qstring的格式,"BPUEn:63 recvIP:192.168.10.103 nHop:0;BPUEn:65280 recvIP:192.168.10.102 nHop:0;BPUEn:65343 recvIP:192.168.10.108 nHop:0"
    void getSKTEnable(const QString &data);
    
    /**
     * @brief 当通过扫描发现一个设备（站点）时发射
     * @param deviceInfo 包含设备详细信息的JSON对象
     */
    void deviceDiscovered(const QJsonObject &deviceInfo);

    // Aprog.exe进程相关信号
    void aprogStarted();
    void aprogFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void aprogError(QProcess::ProcessError error);
    void aprogOutputReceived(const QString &output);

private slots:
    // TCP连接事件处理
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onSocketReadyRead();
    void onConnectionTimeout();
    void onRequestTimeout();
    void onReconnectTimer();
    
    // Aprog.exe进程事件处理
    void onAprogStarted();
    void onAprogFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onAprogError(QProcess::ProcessError error);
    void onAprogReadyReadStandardOutput();
    void onAprogReadyReadStandardError();

private:
    // 请求队列管理
    struct PendingRequest {
        QString operation;  // 用于信号发射的操作名
        QString method;     // JSON-RPC方法名
        qint64 requestId;
        QJsonValue params;
        QTimer* timeoutTimer;
        
        PendingRequest() : timeoutTimer(nullptr) {}
        ~PendingRequest() { 
            if (timeoutTimer) { 
                timeoutTimer->stop(); 
                timeoutTimer->deleteLater(); 
            } 
        }
    };

    // 内部方法
    void processResponse(const QJsonObject &response);
    void processMethodNotification(const QJsonObject &response);
    void processResultNotification(const QJsonObject &response);
    void connectToServer();
    void disconnectFromServer();
    void tryConnectToServer();  // 非阻塞尝试连接
    void checkConnectionAndSendRequests();  // 检查连接状态并发送请求
    void sendQueuedRequests();
    void queueRequest(const QString &operation, const QString &method, const QJsonValue &params = QJsonValue());
    void sendJsonRpcRequest(PendingRequest* request);
    void processIncomingData();
    void processReceivedPacket(const QByteArray &packet);
    void handleJsonRpcResponse(const QJsonObject &response);
    void completeRequest(qint64 requestId, bool success, const QJsonObject &result = QJsonObject(), const QString &error = QString());
    void failAllPendingRequests(const QString &reason);
    qint64 generateRequestId();
    
    // 协议实现
    QByteArray createPacket(const QByteArray &jsonData);

    // 配置属性（线程安全访问）
    mutable QMutex m_configMutex;
    QJsonObject m_config;
    QString m_serverHost;
    quint16 m_serverPort;
    int m_timeout;
    int m_requestTimeout;
    int m_reconnectInterval;
    QString m_aprogPath;  // Aprog.exe的完整路径
    
    // 网络连接和状态
    QTcpSocket* m_socket;
    QTimer* m_connectionTimer;
    QTimer* m_reconnectTimer;
    QByteArray m_receiveBuffer;
    std::atomic<IDevice::DeviceStatus> m_atomicStatus;
    bool m_isConnected;
    bool m_isConnecting;
    
    // Aprog.exe进程管理
    QProcess* m_aprogProcess;
    bool m_isAprogRunning;
    
    // 请求管理
    mutable QMutex m_requestMutex;
    QQueue<PendingRequest*> m_requestQueue;
    QMap<qint64, PendingRequest*> m_pendingRequests; // requestId -> request
    std::atomic<qint64> m_nextRequestId;
    
    // 协议常量
    static const quint32 CLIENT_MAGIC_NUMBER = 0x4150524F; // "APRO"
    static const quint16 CLIENT_HEADER_VERSION = 1;
    static const int CLIENT_HEADER_LENGTH = 32;
    static const QString CLIENT_JSONRPC_VERSION;
};

} // namespace Domain

#endif // BURNDEVICE_H 
