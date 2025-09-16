#ifndef PROCESSDEVICE_H
#define PROCESSDEVICE_H

#include "domain/IDevice.h"

namespace Domain {

/**
 * @brief 代表一个可执行外部程序的设备。
 * 这个设备是无状态的，其主要功能是通过 executeCommand 运行命令行程序。
 */
class ProcessDevice : public IDevice {
    Q_OBJECT
public:
    explicit ProcessDevice(QObject *parent = nullptr);
    ~ProcessDevice() override;

    // IDevice interface
    bool initialize() override;
    bool release() override;
    QJsonObject executeCommand(const QString &command, const QJsonObject &params) override;
    DeviceType getType() const override;

    // 补全其他纯虚函数的实现
    DeviceStatus getStatus() const override;
    QString getName() const override;
    QString getDescription() const override;
    void setCommunicationChannel(std::shared_ptr<Infrastructure::ICommunicationChannel> channel) override;
    std::shared_ptr<Infrastructure::ICommunicationChannel> getCommunicationChannel() const override;
    void setConfiguration(const QJsonObject &config) override;
    QJsonObject getConfiguration() const override;
    bool selfTest() override;

private:
    QJsonObject m_config;
};

} // namespace Domain

#endif // PROCESSDEVICE_H 