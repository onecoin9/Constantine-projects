#ifndef IDEVICE_H
#define IDEVICE_H

#include <QObject>
#include <QJsonObject>
#include <QString>
#include <memory>

namespace Infrastructure {
    class ICommunicationChannel;
}

namespace Domain {

/**
 * @brief 设备接口
 * 定义所有设备的通用行为
 */
class IDevice : public QObject
{
    Q_OBJECT
public:
    enum class DeviceStatus {
        Disconnected,
        Connected,
        Initializing,
        Ready,
        Scanned,
        Loaded,
        Busy,
        Error
    };

    enum class DeviceType {
        Turntable,
        TestBoard,
        Handler,
        ProcessRunner,
        Burn,
        Unknown
    };

    explicit IDevice(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IDevice() = default;

    // 基本操作
    virtual bool initialize() = 0;
    virtual bool release() = 0;
    virtual DeviceStatus getStatus() const = 0;
    virtual QString getName() const = 0;
    virtual DeviceType getType() const = 0;
    virtual QString getDescription() const = 0;

    // 命令执行
    virtual QJsonObject executeCommand(const QString &command, const QJsonObject &params) = 0;
    
    // 通信通道管理
    virtual void setCommunicationChannel(std::shared_ptr<Infrastructure::ICommunicationChannel> channel) = 0;
    virtual std::shared_ptr<Infrastructure::ICommunicationChannel> getCommunicationChannel() const = 0;

    // 设备特定配置
    virtual void setConfiguration(const QJsonObject &config) = 0;
    virtual QJsonObject getConfiguration() const = 0;

    // 设备自检
    virtual bool selfTest() = 0;

signals:
    void statusChanged(DeviceStatus status);
    void errorOccurred(const QString &error);
    void dataReceived(const QByteArray &data);
    void dataSent(const QByteArray &data);
    void commandFinished(const QJsonObject &result);
    
    // 用于记录原始通信数据的信号
    void rawFrameSent(const QByteArray &frame);
    void rawFrameReceived(const QByteArray &frame);

protected:
    std::shared_ptr<Infrastructure::ICommunicationChannel> m_commChannel;
    DeviceStatus m_status = DeviceStatus::Disconnected;
    QString m_name;
    DeviceType m_type = DeviceType::Unknown;
};

} // namespace Domain

#endif // IDEVICE_H 
