#include "services/DeviceManager.h"
#include "domain/IDevice.h"
#include "domain/TestBoardDevice.h"
#include "domain/HandlerDevice.h"
#include "domain/ProcessDevice.h"
#include "domain/BurnDevice.h"
#include "domain/IProtocolFramer.h"
#include "domain/protocols/IXTProtocol.h"
#include "protocols/XTProtocolFramer.h"
#include "domain/protocols/XTProtocol.h"
#include "infrastructure/ICommunicationChannel.h"
#include "infrastructure/SerialChannel.h"
#include "infrastructure/SerialPortManager.h"
#include "infrastructure/TcpChannel.h"
#include "services/DutManager.h"
#include "domain/AsyncProcessDevice.h"

#include <QJsonArray>
#include <QVariantMap>
#include "core/Logger.h"

namespace Services {

DeviceManager::DeviceManager(QObject *parent)
    : QObject(parent)
{
    LOG_MODULE_DEBUG("DeviceManager", "DeviceManager constructed.");
}

DeviceManager::~DeviceManager()
{
    releaseAllDevices();
}

bool DeviceManager::registerDevice(const QString &deviceId, std::shared_ptr<Domain::IDevice> device)
{
    if (deviceId.isEmpty() || !device) {
        LOG_MODULE_WARNING("DeviceManager", "Cannot register device: deviceId is empty or device object is null.");
        return false;
    }
    
    if (m_devices.contains(deviceId)) {
        LOG_MODULE_WARNING("DeviceManager", QString("Device ID '%1' already exists.").arg(deviceId).toStdString());
        return false;
    }
    
    m_devices[deviceId] = device;
    
    // 连接设备信号
    connect(device.get(), &Domain::IDevice::statusChanged,
            [this, deviceId](Domain::IDevice::DeviceStatus status) {
        emit deviceStatusChanged(deviceId, static_cast<int>(status));
    });
    
    connect(device.get(), &Domain::IDevice::errorOccurred,
            [this, deviceId](const QString &error) {
        emit errorOccurred(deviceId, error);
    });
    
    emit deviceRegistered(deviceId);
    LOG_MODULE_INFO("DeviceManager", QString("Device '%1' registered.").arg(deviceId).toStdString());
    
    // 如果是 TestBoardDevice 并且我们有 DutManager，则连接实时数据
    auto dutManagerPtr = m_dutManager.lock();
    if (dutManagerPtr) {
        auto testBoard = std::dynamic_pointer_cast<Domain::TestBoardDevice>(device);
        if (testBoard) {
            connect(testBoard.get(), &Domain::TestBoardDevice::testDataAvailable,
                    this, [dutManagerPtr](const QString& dutId, const QVariantMap& data){
                dutManagerPtr->setTestData(dutId, data);
            });
        }
    }
    
    return true;
}

bool DeviceManager::unregisterDevice(const QString &deviceId)
{
    if (!m_devices.contains(deviceId)) {
        LOG_MODULE_WARNING("DeviceManager", QString("Device '%1' does not exist.").arg(deviceId).toStdString());
        return false;
    }
    
    // 先释放设备
    releaseDevice(deviceId);
    
    // 移除设备
    m_devices.remove(deviceId);
    
    emit deviceUnregistered(deviceId);
    LOG_MODULE_INFO("DeviceManager", QString("Device '%1' unregistered.").arg(deviceId).toStdString());
    
    return true;
}

std::shared_ptr<Domain::IDevice> DeviceManager::getDevice(const QString &deviceId) const
{
    return m_devices.value(deviceId, nullptr);
}

QStringList DeviceManager::getAllDeviceIds() const
{
    return m_devices.keys();
}

bool DeviceManager::initializeDevice(const QString &deviceId)
{
    auto device = getDevice(deviceId);
    if (!device) {
        LOG_MODULE_WARNING("DeviceManager", QString("Device '%1' not found for initialization.").arg(deviceId).toStdString());
        return false;
    }
    
    bool result = device->initialize();
    if (result) {
        emit deviceInitialized(deviceId);
        LOG_MODULE_INFO("DeviceManager", QString("Device '%1' initialized successfully.").arg(deviceId).toStdString());
    } else {
        LOG_MODULE_WARNING("DeviceManager", QString("Failed to initialize device '%1'.").arg(deviceId).toStdString());
    }
    
    return result;
}

bool DeviceManager::initializeAllDevices()
{
    LOG_MODULE_INFO("DeviceManager", "Starting initialization of all devices...");
    LOG_MODULE_DEBUG("DeviceManager", QString("Total devices registered: %1. IDs: %2").arg(m_devices.size()).arg(QStringList(m_devices.keys()).join(", ")).toStdString());
    
    if (m_devices.isEmpty()) {
        LOG_MODULE_WARNING("DeviceManager", "No devices registered to initialize.");
        return false;  // 改为返回false，因为没有设备可以初始化
    }
    
    bool allSuccess = true;
    
    for (const QString &deviceId : m_devices.keys()) {
        LOG_MODULE_DEBUG("DeviceManager", QString("Initializing device: %1").arg(deviceId).toStdString());
        if (!initializeDevice(deviceId)) {
            allSuccess = false;
        }
    }
    
    LOG_MODULE_INFO("DeviceManager", QString("Initialization of all devices completed. Result: %1").arg(allSuccess ? "SUCCESS" : "FAILED").toStdString());
    return allSuccess;
}

bool DeviceManager::releaseDevice(const QString &deviceId)
{
    auto device = getDevice(deviceId);
    if (!device) {
        LOG_MODULE_WARNING("DeviceManager", QString("Device '%1' not found for release.").arg(deviceId).toStdString());
        return false;
    }
    
    bool result = device->release();
    if (result) {
        emit deviceReleased(deviceId);
        LOG_MODULE_INFO("DeviceManager", QString("Device '%1' released.").arg(deviceId).toStdString());
    } else {
        LOG_MODULE_WARNING("DeviceManager", QString("Failed to release device '%1'.").arg(deviceId).toStdString());
    }
    
    return result;
}

bool DeviceManager::releaseAllDevices()
{
    bool allSuccess = true;
    
    for (const QString &deviceId : m_devices.keys()) {
        if (!releaseDevice(deviceId)) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

bool DeviceManager::isDeviceRegistered(const QString &deviceId) const
{
    return m_devices.contains(deviceId);
}

bool DeviceManager::isDeviceInitialized(const QString &deviceId) const
{
    auto device = getDevice(deviceId);
    if (!device) {
        return false;
    }
    
    return device->getStatus() == Domain::IDevice::DeviceStatus::Ready ||
            device->getStatus() == Domain::IDevice::DeviceStatus::Connected ||
            device->getStatus() == Domain::IDevice::DeviceStatus::Busy;
}

QStringList DeviceManager::getDevicesByType(Domain::IDevice::DeviceType type) const
{
    QStringList deviceIds;
    
    for (auto it = m_devices.begin(); it != m_devices.end(); ++it) {
        if (it.value() && it.value()->getType() == type) {
            deviceIds.append(it.key());
        }
    }
    
    return deviceIds;
}

std::shared_ptr<Domain::IDevice> DeviceManager::createDevice(const QJsonObject &deviceConfig)
{
    QString deviceType = deviceConfig["deviceType"].toString();
    
    if (deviceType == "TestBoard") {
        return createTestBoardDevice(deviceConfig);
    } else if (deviceType == "Handler") {
        return createHandlerDevice(deviceConfig["deviceConfig"].toObject());
    } else if (deviceType == "Process") {
        return std::make_shared<Domain::ProcessDevice>();
    } else if (deviceType == "AsyncActivation") {
        return createAsyncProcessDevice(deviceConfig["deviceConfig"].toObject(), Domain::AsyncProcessDevice::Activation);
    } else if (deviceType == "AsyncCalibration") {
        return createAsyncProcessDevice(deviceConfig["deviceConfig"].toObject(), Domain::AsyncProcessDevice::Calibration);
    } else if (deviceType == "Burn") {
        return createBurnDevice(deviceConfig["deviceConfig"].toObject());
    } else {
        LOG_MODULE_WARNING("DeviceManager", QString("Unknown device type '%1' in configuration.").arg(deviceType).toStdString());
        return nullptr;
    }
}

std::shared_ptr<Infrastructure::ICommunicationChannel> DeviceManager::createCommunicationChannel(const QJsonObject &commConfig)
{
    QString type = commConfig["channelType"].toString();

    if (type.toLower() == "serial") {
        auto channel = std::make_shared<Infrastructure::SerialChannel>();
        
        QJsonObject paramsObj = commConfig["parameters"].toObject();

        // 转换并设置参数
        QVariantMap params;
        params["portName"] = paramsObj["portName"].toString();
        params["portIndex"] = paramsObj["portIndex"].toInt();
        params["baudRate"] = paramsObj["baudRate"].toString().toInt();
        params["dataBits"] = paramsObj["dataBits"].toString().toInt();
        params["parity"] = paramsObj["parity"].toString().toInt();
        params["stopBits"] = paramsObj["stopBits"].toString().toInt();
        params["flowControl"] = paramsObj["flowControl"].toString().toInt();

        channel->setParameters(params);

        // 关键修复：执行依赖注入，而不是设置共享指针
        LOG_MODULE_INFO("DeviceManager", "SerialPortManager before.");
        channel->setSerialPortManager(&Infrastructure::SerialPortManager::getInstance());
        LOG_MODULE_INFO("DeviceManager", "SerialPortManager after.");
        return channel;
    }
    return nullptr;
}

bool DeviceManager::loadDevicesFromConfig(const QJsonObject &config)
{
    QJsonArray devices = config["devices"].toArray();
    
    if (devices.isEmpty()) {
        m_lastError = "配置文件中未找到 'devices' 数组。";
        LOG_MODULE_WARNING("DeviceManager", m_lastError.toStdString());
        return false;
    }
    
    bool allSuccess = true;
    
    for (const QJsonValue &value : devices) {
        QJsonObject deviceConfig = value.toObject();
        
        QString deviceId = deviceConfig["deviceId"].toString();
        bool enabled = deviceConfig["enabled"].toBool(true);
        
        if (!enabled) {
            LOG_MODULE_INFO("DeviceManager", QString("Device '%1' is disabled, skipping.").arg(deviceId).toStdString());
            continue;
        }
        
        if (deviceId.isEmpty()) {
            LOG_MODULE_WARNING("DeviceManager", "Device ID is empty, skipping configuration entry.");
            allSuccess = false;
            continue;
        }
        
        // 关键修复：将 deviceType 的声明提前，解决作用域问题
        //QString deviceType = deviceConfig["deviceType"].toString();
        
        auto device = createDevice(deviceConfig);
        
        if (!device) {
            m_lastError = QString("创建设备 '%1' 失败。").arg(deviceId);
            LOG_MODULE_WARNING("DeviceManager", m_lastError.toStdString());
            allSuccess = false;
            continue;
        }
        // 注册设备
        if (!registerDevice(deviceId, device)) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

QString DeviceManager::getLastError() const
{
    return m_lastError;
}


std::shared_ptr<Domain::IDevice> DeviceManager::createTestBoardDevice(const QJsonObject &config)
{
    // 1. 创建协议栈的具体实现
    auto framer = std::make_unique<Protocols::XTProtocolFramer>();
    auto protocol = std::make_unique<Domain::Protocols::XTProtocol>();

    // 2. 将framer注入到protocol中
    protocol->setFramer(std::move(framer));

    // 3. 创建设备实例，并注入配置好的protocol
    auto device = std::make_shared<Domain::TestBoardDevice>(std::move(protocol));

    // 创建通信通道
    qDebug()<<config;
    QJsonObject commConfig = config["communicationConfig"].toObject();
    QString deviceId = config["deviceId"].toString();
    auto channel = createCommunicationChannel(commConfig);

    if (!channel) {
        m_lastError = QString("为设备 '%1' 创建通信通道失败。").arg(deviceId);
        LOG_MODULE_WARNING("DeviceManager", m_lastError.toStdString());
        return nullptr;
    }

    // 设置通信通道
    device->setCommunicationChannel(channel);
    
    return device;
}

std::shared_ptr<Domain::IDevice> DeviceManager::createHandlerDevice(const QJsonObject &config)
{
    // 创建Handler设备实例（不需要外部协议和通信层）
    auto device = std::make_shared<Domain::HandlerDevice>();
    
    // 设置Handler特定的配置
    QJsonObject handlerConfig = config;
    
    // 设置协议类型（默认使用SProtocol）
    if (!handlerConfig.contains("protocolType")) {
        handlerConfig["protocolType"] = "SProtocol";
    }
    
    // 设置TCP通信参数（这些参数会被插件内部使用）
    if (!handlerConfig.contains("tcpConfig")) {
        QJsonObject tcpConfig;
        tcpConfig["serverPort"] = 64101;
        tcpConfig["clientPort"] = 64100;
        tcpConfig["ipAddress"] = "127.0.0.1";
        handlerConfig["tcpConfig"] = tcpConfig;
    }
    
    device->setConfiguration(handlerConfig);
    
    return device;
}

std::shared_ptr<Domain::IDevice> DeviceManager::createAsyncProcessDevice(const QJsonObject &config, Domain::AsyncProcessDevice::ProcessType type)
{
    auto device = std::make_shared<Domain::AsyncProcessDevice>(type);
    
    // 设置配置
    QJsonObject fullConfig = config;
    
    if (type == Domain::AsyncProcessDevice::Activation) {
        // 激活设备默认配置
        if (!fullConfig.contains("executable")) {
            fullConfig["executable"] = "aprog.exe";
        }
        if (!fullConfig.contains("jsonRpcUrl")) {
            fullConfig["jsonRpcUrl"] = "http://localhost:8080/jsonrpc";
        }
    } else {
        // 标定设备默认配置
        if (!fullConfig.contains("executable")) {
            fullConfig["executable"] = "MTCaliTest.exe";
        }
    }
    
    device->setConfiguration(fullConfig);
    
    return device;
}

void DeviceManager::setDutManager(std::shared_ptr<DutManager> dutManager)
{
    m_dutManager = dutManager;
}

std::shared_ptr<Domain::IDevice> DeviceManager::createBurnDevice(const QJsonObject &config)
{
    auto device = std::make_shared<Domain::BurnDevice>();
    
    // 设置默认配置
    QJsonObject burnConfig = config;
    if (!burnConfig.contains("serverHost")) {
        burnConfig["serverHost"] = "127.0.0.1";
    }
    if (!burnConfig.contains("serverPort")) {
        burnConfig["serverPort"] = 12345;
    }
    if (!burnConfig.contains("name")) {
        burnConfig["name"] = "BurnDevice";
    }
    
    // 连接设备发现信号到DutManager，需要先检查weak_ptr是否有效
    auto dutManagerPtr = m_dutManager.lock();
    if (dutManagerPtr) {
        connect(device.get(), &Domain::BurnDevice::deviceDiscovered,
                dutManagerPtr.get(), &DutManager::updateSiteFromScan);
        connect(device.get(), &Domain::BurnDevice::getSKTEnable,
                dutManagerPtr.get(), &DutManager::updateSktFromDownLoadProject);
    }
    
    device->setConfiguration(burnConfig);
    
    return device;
}

} // namespace Services 
