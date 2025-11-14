#ifndef BURNDEVICE_H
#define BURNDEVICE_H

#include "IDevice.h"
#include "JsonRpcClient.h"
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMutex>
#include <QProcess>
#include <memory>
#include <atomic>

namespace Domain {

/**
 * @brief BurnDevice - 烧录设备类
 * 
 * 使用JsonRpcClient进行通信，保留Aprog.exe程序管理功能
 */
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
    
    // Aprog.exe程序管理
    QString getAprogPath() const;
    void setAprogPath(const QString &path);
    bool isAprogRunning() const;
    void startAprog();
    void stopAprog();
    
    // 获取JsonRpcClient实例
    JsonRpcClient* getJsonRpcClient() const { return m_jsonRpcClient; }

signals:
    // Aprog.exe进程相关信号
    void aprogStarted();
    void aprogFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void aprogError(QProcess::ProcessError error);
    void aprogOutputReceived(const QString &output);

    // 站点与SKT信息（用于与 DutManager 对接）
    void deviceDiscovered(const QJsonObject &deviceInfo);
    // data为qstring的格式,"BPUEn:63 recvIP:192.168.10.103 nHop:0;..."
    void getSKTEnable(const QString &data);
    // DoJob 结果：ip、0基座子索引、是否成功、原始明细
    void doJobResult(const QString &ip, int nHopNum, const QJsonObject &detail);
    // 烧录完成信号
    void doJobCompleted(const QJsonObject &result);

private slots:
    // JsonRpcClient连接状态变化
    void onConnectionStateChanged(JsonRpcClient::ConnectionState state);
    
    // Aprog.exe进程事件处理
    void onAprogStarted();
    void onAprogFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onAprogError(QProcess::ProcessError error);
    void onAprogReadyReadStandardOutput();
    void onAprogReadyReadStandardError();

private:
    // 配置属性（线程安全访问）
    mutable QMutex m_configMutex;
    QMutex m_cmdMutex;
    QJsonObject m_config;
    QString m_serverHost;
    quint16 m_serverPort;
    QString m_aprogPath;  // Aprog.exe的完整路径
    
    // JsonRpcClient实例
    JsonRpcClient* m_jsonRpcClient;
    
    // 设备状态
    std::atomic<IDevice::DeviceStatus> m_atomicStatus;
    QMap<QString, QJsonObject> m_cmdSequences;
    
    // Aprog.exe进程管理
    QProcess* m_aprogProcess;
    bool m_isAprogRunning;

    // 通知处理
    void handleNotification(const QString &method, const QJsonObject &params);


    void extractCommandSequences(const QJsonObject& jsonObject, const QString& parentKey = "");
};

} // namespace Domain

#endif // BURNDEVICE_H
