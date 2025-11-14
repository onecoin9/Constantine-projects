#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QMap>
#include <memory>
#include "domain/IDevice.h"
#include "domain/AsyncProcessDevice.h"

namespace Services {

class DutManager;
class IDevice;

class DeviceManager : public QObject
{
    Q_OBJECT

public:
    explicit DeviceManager(QObject *parent = nullptr);
    ~DeviceManager();

    bool registerDevice(const QString &deviceId, std::shared_ptr<Domain::IDevice> device);
    bool unregisterDevice(const QString &deviceId);

    std::shared_ptr<Domain::IDevice> getDevice(const QString &deviceId) const;
    QStringList getAllDeviceIds() const;

    bool initializeDevice(const QString &deviceId);
    bool initializeAllDevices();
    bool releaseDevice(const QString &deviceId);
    bool releaseAllDevices();

    bool isDeviceRegistered(const QString &deviceId) const;
    bool isDeviceInitialized(const QString &deviceId) const;

    QStringList getDevicesByType(Domain::IDevice::DeviceType type) const;
    std::shared_ptr<Domain::IDevice> getBurnDevice() const;
    std::shared_ptr<Domain::IDevice> getHandlerDevice() const;
    QString getLastError() const;

    bool loadDevicesFromConfig(const QJsonObject &config);
    
    void setDutManager(DutManager* dutManager);

signals:
    void deviceRegistered(const QString &deviceId);
    void deviceUnregistered(const QString &deviceId);
    void deviceInitialized(const QString &deviceId);
    void deviceReleased(const QString &deviceId);
    void deviceStatusChanged(const QString &deviceId, int status);
    void errorOccurred(const QString &deviceId, const QString &error);

private:
    std::shared_ptr<Domain::IDevice> createDevice(const QJsonObject &deviceConfig);
    std::shared_ptr<Infrastructure::ICommunicationChannel> createCommunicationChannel(const QJsonObject &commConfig);

    std::shared_ptr<Domain::IDevice> createTestBoardDevice(const QJsonObject &config);
    std::shared_ptr<Domain::IDevice> createHandlerDevice(const QJsonObject &config);
    std::shared_ptr<Domain::IDevice> createAsyncProcessDevice(const QJsonObject &config, Domain::AsyncProcessDevice::ProcessType type);
    std::shared_ptr<Domain::IDevice> createBurnDevice(const QJsonObject& config);
    std::shared_ptr<Domain::IDevice> createMesDevice(const QJsonObject& config);
    
    QMap<QString, std::shared_ptr<Domain::IDevice>> m_devices;
    DutManager* m_dutManager = nullptr;  // 修改为指针类型
    QString m_lastError;
};

}

#endif // DEVICEMANAGER_H 
