#include "domain/TestBoardDevice.h"
#include "infrastructure/ICommunicationChannel.h"
#include "infrastructure/SerialChannel.h"
#include "infrastructure/BinLogger.h" // 引入BinLogger头文件
#include "domain/protocols/XTProtocol.h"
#include "core/Logger.h"  // 添加Logger头文件
#include <QJsonArray>
#include <QDateTime>
#include <QMutexLocker>
#include <cstring>
#include <QThread>
#include <QTimer>

namespace Domain {

TestBoardDevice::TestBoardDevice(
    std::unique_ptr<Protocols::IXTProtocol> protocol,
    QObject *parent
)
    : IDevice(parent)
    , m_status(IDevice::DeviceStatus::Disconnected)
    , m_protocol(std::move(protocol))
    , m_logger(std::make_unique<Infrastructure::BinLogger>())
    , m_isTestRunning(false)
    , m_lastErrorCode(0)
    , m_lastFaultState(0)
    , m_testFeedbackLogCounter(0)
{
    // Framer的注入现在是Protocol内部的职责，Device层不再关心
    std::memset(&m_latestData, 0, sizeof(m_latestData));
}

TestBoardDevice::~TestBoardDevice()
{
    if (m_status != IDevice::DeviceStatus::Disconnected) {
        release();
    }
    // 确保logger也停止
    m_logger->stopLogging();
}

IDevice::DeviceStatus TestBoardDevice::getStatus() const
{
    QMutexLocker locker(&m_mutex);
    return m_status;
}

void TestBoardDevice::setCommunicationChannel(std::shared_ptr<Infrastructure::ICommunicationChannel> channel)
{
    QMutexLocker locker(&m_mutex);
    m_commChannel = channel;
}

std::shared_ptr<Infrastructure::ICommunicationChannel> TestBoardDevice::getCommunicationChannel() const
{
    QMutexLocker locker(&m_mutex);
    return m_commChannel;
}

QString TestBoardDevice::getName() const
{
    QMutexLocker locker(&m_mutex);
    QString name = m_configuration.value("name").toString();
    if (name.isEmpty()) {
        name = m_configuration.value("deviceId").toString();
    }
    if (name.isEmpty()) {
        name = "TestBoard";
    }
    return name;
}

QString TestBoardDevice::getDeviceName() const
{
    // 直接复用getName()方法
    return getName();
}

void TestBoardDevice::setConfiguration(const QJsonObject &config)
{
    QMutexLocker locker(&m_mutex);
    m_configuration = config;
}

QJsonObject TestBoardDevice::getConfiguration() const
{
    QMutexLocker locker(&m_mutex);
    return m_configuration;
}

bool TestBoardDevice::initialize()
{
    // 关键修复：先获取名称，再加锁，以避免递归锁死
    const QString deviceName = getName();

    QMutexLocker locker(&m_mutex);
    
    if (!m_commChannel || !m_protocol) {
        m_lastError = tr("依赖未注入");
        m_status = IDevice::DeviceStatus::Error;
        return false;
    }
    
    LOG_MODULE_INFO(deviceName.toStdString().c_str(), "设备开始初始化");
    
    // 连接信号
    connectSignals();
    
    // 打开通信通道
    if (!m_commChannel->open()) {
        m_lastError = tr("打开通信通道失败");
        m_status = IDevice::DeviceStatus::Error;
        LOG_ERROR(QString("打开通信通道失败: %1").arg(m_lastError).toStdString());
        return false;
    }
    
    m_status = IDevice::DeviceStatus::Ready;
    m_lastError.clear();
    m_lastErrorCode = 0;
    
    LOG_MODULE_INFO(deviceName.toStdString().c_str(), "设备初始化成功");
    return true;
}

bool TestBoardDevice::release()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_isTestRunning) {
        stopTest();
    }
    
    // 确保logger也停止
    m_logger->stopLogging();
    
    if (m_commChannel) {
        m_commChannel->close();
    }
    
    m_status = IDevice::DeviceStatus::Disconnected;
    return true;
}

bool TestBoardDevice::reset()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_status != IDevice::DeviceStatus::Ready && m_status != IDevice::DeviceStatus::Connected) {
        m_lastError = tr("设备未就绪");
        return false;
    }
    
    // 停止测试
    bool success = stopTest();
    if (success) {
        m_status = IDevice::DeviceStatus::Ready;
    }
    
    return success;
}

bool TestBoardDevice::selfTest()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_status != IDevice::DeviceStatus::Ready && m_status != IDevice::DeviceStatus::Connected) {
        m_lastError = tr("设备未就绪，无法执行自检");
        return false;
    }
    
    // 简单的自检：检查通信是否正常
    // 可以发送一个查询命令来验证通信
    return queryFaultStatus()["success"].toBool();
}

QJsonObject TestBoardDevice::executeCommand(const QString &command, const QJsonObject &params)
{
    QJsonObject result;
    result["success"] = false;
    
    if (command.toLower() == "starttest") {
       uint16_t dutActive = static_cast<uint16_t>(params.value("dutActive").toInt(0xFFFF));
       QString binFileName = params.value("binFileName").toString();
       auto packet = m_protocol->createStartCommand(true, dutActive, QDateTime::currentSecsSinceEpoch());
       if(sendCommand(packet)) {
           if (!binFileName.isEmpty()) {
               m_logger->startLogging("cache", binFileName);
           } else {
               m_logger->startLogging("cache");
           }
           m_isTestRunning = true;
           m_status = IDevice::DeviceStatus::Busy;
           emit testStateChanged(true);
           result["success"] = true;
       } else {
           result["error"] = tr("发送启动命令失败");
       }
    } 
    else if (command.toLower() == "stoptest") {
        auto packet = m_protocol->createStartCommand(false, 0, 0);
        if(sendCommand(packet)) {
           m_logger->stopLogging();
           m_protocol->clearBuffer(); // 调用protocol清空其内部的framer缓冲区
           m_isTestRunning = false;
           m_status = IDevice::DeviceStatus::Ready;
           emit testStateChanged(false);
           result["success"] = true;
        } else {
           result["error"] = tr("发送停止命令失败");
        }
    } 
    else if (command.toLower() == "controldutpower") {
        bool enable = params.value("enable").toBool();
        uint16_t dutEnable = static_cast<uint16_t>(params.value("dutEnable").toInt(0xFFFF));
        auto cmdData = m_protocol->createPowerControlCommand(enable, dutEnable);
        if (sendCommand(cmdData)) {
            result["success"] = true;
        } else {
            result["error"] = tr("发送DUT电源控制命令失败");
        }
    } 
    else if (command.toLower() == "queryvoltagestatus") {
        auto cmdData = m_protocol->createVoltageQueryCommand();
        if (sendCommand(cmdData)) {
            result["success"] = true;
            // 实际的电压数据将通过onVoltageDataReceived异步返回
            // 这里可以返回缓存的数据
            if (!m_lastVoltageData.isEmpty()) {
                result["data"] = m_lastVoltageData;
            }
        } else {
            result["error"] = tr("发送电压查询命令失败");
        }
    } 
    else if (command.toLower() == "queryfaultstatus") {
        auto cmdData = m_protocol->createFaultQueryCommand();
        if (sendCommand(cmdData)) {
            result["success"] = true;
            result["faultState"] = static_cast<qint64>(m_lastFaultState);
        } else {
            result["error"] = tr("发送故障查询命令失败");
        }
    } 
    else if (command.toLower() == "sendcalibration") {
        uint8_t dutSel = static_cast<uint8_t>(params.value("dutSel").toInt());
        QByteArray cmd = QByteArray::fromHex(params.value("command").toString().toLatin1());
        auto cmdData = m_protocol->createCalibrationCommand(dutSel, cmd);
        if (sendCommand(cmdData)) {
            result["success"] = true;
        } else {
            result["error"] = tr("发送标定命令失败");
        }
    }
    else if (command.toLower() == "writeregister") {
        uint8_t dutSel = static_cast<uint8_t>(params.value("dutSel").toInt());
        uint8_t regAddr = static_cast<uint8_t>(params.value("regAddr").toInt());
        QByteArray data = QByteArray::fromHex(params.value("data").toString().toLatin1());
        
        // 直接从Protocol获取完整数据包
        auto packet = m_protocol->createRegisterWriteCommand(dutSel, regAddr, data);
        if (sendCommand(packet)) {
            result["success"] = true;
        } else {
            result["error"] = tr("发送寄存器写入命令失败");
        }
    }
    else if (command.toLower() == "readregister") {
        uint8_t dutSel = static_cast<uint8_t>(params.value("dutSel").toInt());
        uint8_t regAddr = static_cast<uint8_t>(params.value("regAddr").toInt());
        uint8_t length = static_cast<uint8_t>(params.value("length").toInt());
        auto cmdData = m_protocol->createRegisterReadCommand(dutSel, regAddr, length);
        if (sendCommand(cmdData)) {
            result["success"] = true;
        } else {
            result["error"] = tr("发送寄存器读取命令失败");
        }
    }
    else if (command.toLower() == "setchiptype") {
        uint8_t chipIndex = static_cast<uint8_t>(params.value("chipIndex").toInt());
        auto cmdData = m_protocol->createSetChipTypeCommand(chipIndex);
        if (sendCommand(cmdData)) {
            result["success"] = true;
        } else {
            result["error"] = tr("发送设置芯片类型命令失败");
        }
    }
    else {
        result["error"] = tr("未知命令: %1").arg(command);
    }
    
    return result;
}

QJsonObject TestBoardDevice::getCapabilities() const
{
    QJsonObject caps;
    caps["name"] = getName();
    caps["description"] = getDescription();
    caps["type"] = "TestBoard";
    
    QJsonArray commands;
    commands.append("startTest");
    commands.append("stopTest");
    commands.append("controlDutPower");
    commands.append("queryVoltageStatus");
    commands.append("queryFaultStatus");
    commands.append("sendCalibration");
    commands.append("writeRegister");
    commands.append("readRegister");
    commands.append("setChipType");
    caps["commands"] = commands;
    
    return caps;
}

QJsonObject TestBoardDevice::getLastError() const
{
    QMutexLocker locker(&m_mutex);
    QJsonObject error;
    error["code"] = m_lastErrorCode;
    error["message"] = m_lastError;
    return error;
}

Protocols::IXTProtocol* TestBoardDevice::getProtocol() const
{
    return m_protocol.get();
}

bool TestBoardDevice::startTest(uint16_t dutActive)
{
    bool needEmit = false;
    bool sendOk = false;

    {
        QMutexLocker locker(&m_mutex);

        if (m_status != IDevice::DeviceStatus::Ready && m_status != IDevice::DeviceStatus::Connected) {
            m_lastError = tr("设备未就绪，无法开始测试");
            m_lastErrorCode = -1;
            return false;
        }
        
        if (m_isTestRunning) {
            m_lastError = tr("测试已在运行中");
            m_lastErrorCode = -2;
            return false;
        }

        // 开始二进制日志记录
        m_logger->startLogging("cache");

        // 使用协议层创建命令
        uint32_t timestamp = QDateTime::currentSecsSinceEpoch();
        auto cmdData = m_protocol->createStartCommand(true, dutActive, timestamp);

        // 在锁外发送命令，避免潜在死锁
        locker.unlock();
        sendOk = sendCommand(cmdData);
        locker.relock();

        if (sendOk) {
            m_isTestRunning = true;
            m_status = IDevice::DeviceStatus::Busy;
            m_lastError.clear();
            m_lastErrorCode = 0;
            needEmit = true;
        } else {
            m_lastError = tr("发送启动测试命令失败");
            m_lastErrorCode = -3;
            m_status = IDevice::DeviceStatus::Error;
        }
    }

    if (needEmit) {
        emit testStateChanged(true);
    }

    return sendOk;
}

bool TestBoardDevice::stopTest()
{
    // 停止二进制日志记录
    m_logger->stopLogging();

    bool needEmit = false;
    {
        QMutexLocker locker(&m_mutex);
        if (!m_isTestRunning) {
            m_lastError = tr("测试已停止");
            return true; // Already stopped, consider it a success
        }
        m_isTestRunning = false; // <<< IMMEDIATE STATE CHANGE
        m_status = IDevice::DeviceStatus::Ready;
        needEmit = true;
    }

    // 在发送命令前，立即发射信号更新UI状态
    if (needEmit) {
        emit testStateChanged(false);
    }

    // 现在，发送命令并清空缓冲区
    auto cmdData = m_protocol->createStartCommand(false, 0, 0);
    if (sendCommand(cmdData)) {
        m_lastError.clear();
        m_lastErrorCode = 0;
        return true;
    } else {
        QMutexLocker locker(&m_mutex);
        m_lastError = tr("发送停止测试命令失败");
        m_lastErrorCode = -4;
        m_status = IDevice::DeviceStatus::Error;
        return false;
    }
}

bool TestBoardDevice::isTestRunning() const
{
    QMutexLocker locker(&m_mutex);
    return m_isTestRunning;
}

bool TestBoardDevice::controlDutPower(bool enable, uint16_t dutEnable)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_status != IDevice::DeviceStatus::Ready && m_status != IDevice::DeviceStatus::Connected) {
        m_lastError = tr("设备未就绪");
        return false;
    }
    
    // 使用协议层创建命令
    auto cmdData = m_protocol->createPowerControlCommand(enable, dutEnable);
    
    // 发送命令
    return sendCommand(cmdData);
}

QJsonObject TestBoardDevice::queryVoltageStatus()
{
    //QMutexLocker locker(&m_mutex);
    
    QJsonObject result;
    result["success"] = false;
    
    if (m_status != IDevice::DeviceStatus::Ready && m_status != IDevice::DeviceStatus::Connected) {
        result["error"] = tr("设备未就绪");
        return result;
    }
    
    // 使用协议层创建命令
    auto cmdData = m_protocol->createVoltageQueryCommand();
    
    // 发送命令
    if (sendCommand(cmdData)) {
        result["success"] = true;
        // 实际的电压数据将通过onVoltageDataReceived异步返回
        // 这里可以返回缓存的数据
        if (!m_lastVoltageData.isEmpty()) {
            result["data"] = m_lastVoltageData;
        }
    } else {
        result["error"] = tr("发送电压查询命令失败");
    }
    
    return result;
}

QJsonObject TestBoardDevice::queryFaultStatus()
{
    //QMutexLocker locker(&m_mutex);
    
    QJsonObject result;
    result["success"] = false;
    
    if (m_status != IDevice::DeviceStatus::Ready && m_status != IDevice::DeviceStatus::Connected) {
        result["error"] = tr("设备未就绪");
        return result;
    }
    
    // 使用协议层创建命令
    auto cmdData = m_protocol->createFaultQueryCommand();
    
    // 发送命令
    if (sendCommand(cmdData)) {
        result["success"] = true;
        result["faultState"] = static_cast<qint64>(m_lastFaultState);
    } else {
        result["error"] = tr("发送故障查询命令失败");
    }
    
    return result;
}

TestBoardData TestBoardDevice::getLatestData() const
{
    QMutexLocker locker(&m_mutex);
    return m_latestData;
}

QVector<double> TestBoardDevice::getAllTemperatures() const
{
    QMutexLocker locker(&m_mutex);
    QVector<double> temps;
    
    // 8个DUT的温度
    for (int i = 0; i < 8; ++i) {
        temps.append(static_cast<double>(m_latestData.dutData[i].gyro_temperature));
    }
    
    // 外挂陀螺仪温度
    temps.append(static_cast<double>(m_latestData.externalGyro.gyro_temperature));
    
    return temps;
}

bool TestBoardDevice::sendCommand(const QByteArray &fullPacket)
{
    // 不再需要调用Framer，直接发送
    if (m_commChannel && m_commChannel->isOpen()) {
        emit rawFrameSent(fullPacket);
        return m_commChannel->send(fullPacket) > 0;
    }
    return false;
}

void TestBoardDevice::connectSignals()
{
    // 通信层 -> 协议层
    connect(m_commChannel.get(), &Infrastructure::ICommunicationChannel::dataReceived,
            m_protocol.get(), &Protocols::IXTProtocol::appendData, Qt::QueuedConnection);
    
    // 关键修复：确保所有从协议层到设备层的信号都已连接，并使用QueuedConnection
    
    // 原始数据包，用于日志记录
    connect(m_protocol.get(), &Protocols::IXTProtocol::rawPacketReady,
            this, &IDevice::rawFrameReceived, Qt::QueuedConnection);

    // 二进制日志记录
    connect(m_protocol.get(), &Protocols::IXTProtocol::rawPacketReady,
            m_logger.get(), &Infrastructure::BinLogger::onRawPacketReceived, Qt::QueuedConnection);
    
    // 解析后的业务数据
    connect(m_protocol.get(), &Protocols::IXTProtocol::testDataReceived, this, &TestBoardDevice::onTestDataReceived, Qt::QueuedConnection);
    // 关键修复：添加被遗漏的powerStateChanged信号连接
    connect(m_protocol.get(), &Protocols::IXTProtocol::powerStateChanged, this, &TestBoardDevice::onPowerStateChanged, Qt::QueuedConnection);
    connect(m_protocol.get(), &Protocols::IXTProtocol::voltageDataReceived, this, &TestBoardDevice::onVoltageDataReceived, Qt::QueuedConnection);
    connect(m_protocol.get(), &Protocols::IXTProtocol::faultStatusReceived, this, &TestBoardDevice::onFaultStatusReceived, Qt::QueuedConnection);
    connect(m_protocol.get(), &Protocols::IXTProtocol::calibrationResultReceived, this, &TestBoardDevice::onCalibrationResultReceived, Qt::QueuedConnection);
    connect(m_protocol.get(), &Protocols::IXTProtocol::registerWriteResultReceived, this, &TestBoardDevice::onRegisterWriteResultReceived, Qt::QueuedConnection);
    connect(m_protocol.get(), &Protocols::IXTProtocol::registerReadResultReceived, this, &TestBoardDevice::onRegisterReadResultReceived, Qt::QueuedConnection);
    connect(m_protocol.get(), &Protocols::IXTProtocol::chipTypeSetResultReceived, this, &TestBoardDevice::onSetChipTypeResultReceived, Qt::QueuedConnection);
    connect(m_protocol.get(), &Protocols::IXTProtocol::protocolError, this, &TestBoardDevice::onProtocolError, Qt::QueuedConnection);
    
    // 通信层错误
    connect(m_commChannel.get(), &Infrastructure::ICommunicationChannel::errorOccurred,
            this, &TestBoardDevice::onCommunicationError, Qt::QueuedConnection);
}

void TestBoardDevice::onTestDataReceived(const Protocols::TestFeedbackData &data)
{
    bool shouldEmitStateChange = false;
    bool newTestState = false;

    {
        QMutexLocker locker(&m_mutex);
        m_latestData = data;

        // 根据反馈更新设备状态
        bool currentStateIsRunning = m_isTestRunning;
        // 0x01 表示测试中, 0x00 表示停止
        bool feedbackStateIsRunning = (data.testState == 0x01);

        if (currentStateIsRunning != feedbackStateIsRunning) {
            // 状态发生了变化
            m_isTestRunning = feedbackStateIsRunning;
            if (feedbackStateIsRunning) {
                m_status = IDevice::DeviceStatus::Busy;
            } else {
                m_status = IDevice::DeviceStatus::Ready;
            }
            shouldEmitStateChange = true;
            newTestState = feedbackStateIsRunning;
        }

        // 0x02 表示故障
        if (data.testState == 0x02) {
            if (m_isTestRunning) {
                shouldEmitStateChange = true;
                newTestState = false; // 测试因故障而停止
            }
            m_isTestRunning = false;
            m_status = IDevice::DeviceStatus::Error;
            m_lastError = tr("测试板报告故障状态");
        }
    }

    // 在锁外发射信号
    // 高速模式下，这个信号会被高频触发，导致UI卡死
    // 我们将移除UI对此信号的直接响应，改为UI定时轮询
    // emit testDataReceived(data); 
    
    if(shouldEmitStateChange) {
        emit testStateChanged(newTestState);
    }

    // 高频信号发射已被禁用，以防止UI卡死
    // UI应该通过定时器调用getLatestData()来获取最新数据
    /*
    // 拆分并抛出单 DUT 数据，便于 UI/MES 直接使用
    for(int i=0;i<8;++i){
        QVariantMap m;
        m["gyro_x"] = static_cast<qulonglong>(data.dutData[i].gyro_x);
        m["gyro_y"] = static_cast<qulonglong>(data.dutData[i].gyro_y);
        m["gyro_z"] = static_cast<qulonglong>(data.dutData[i].gyro_z);
        m["gyro_acc_x"] = static_cast<qulonglong>(data.dutData[i].gyro_acc_x);
        m["gyro_acc_y"] = static_cast<qulonglong>(data.dutData[i].gyro_acc_y);
        m["gyro_acc_z"] = static_cast<qulonglong>(data.dutData[i].gyro_acc_z);
        m["gyro_mix"] = static_cast<qulonglong>(data.dutData[i].gyro_mix);
        m["temperature"] = static_cast<int>(data.dutData[i].gyro_temperature);
        m["timestamp"] = static_cast<quint32>(data.timestamp);

        QString dutId = QString("Socket-%1").arg(i+1); // TODO: 可由配置映射
        emit testDataAvailable(dutId, m);
    }
    */
}

void TestBoardDevice::onPowerStateChanged(const Protocols::PowerFeedbackData &data)
{
    // 强制调试日志：确认设备层方法被调用
    LOG_INFO(QString("DEBUG: TestBoardDevice::onPowerStateChanged called - State: 0x%1, DUT Mask: 0x%2")
        .arg(QString::number(data.state, 16))
        .arg(QString::number(data.dutPowerState, 16).toUpper().rightJustified(4, '0')).toStdString());
    
    // 直接转发信号，不再进行不必要的转换
    emit powerStateChanged(data);
}

void TestBoardDevice::onVoltageDataReceived(const Protocols::VoltageFeedbackData &data)
{
    // 转换为JSON格式
    QJsonObject voltageData;
    voltageData["serialNumber"] = static_cast<qint64>(data.serialNumber);
    voltageData["mainVoltage"] = data.mainVoltage;
    voltageData["mainCurrent"] = data.mainCurrent;
    
    QJsonArray dutArray;
    for (int i = 0; i < 8; ++i) {
        QJsonObject dut;
        dut["voltage_5v"] = data.dutPower[i].voltage_5v;
        dut["current_5v"] = data.dutPower[i].current_5v;
        dut["voltage_3v3"] = data.dutPower[i].voltage_3v3;
        dut["current_3v3"] = data.dutPower[i].current_3v3;
        dutArray.append(dut);
    }
    voltageData["dutPower"] = dutArray;
    
    {
        QMutexLocker locker(&m_mutex);
        m_lastVoltageData = voltageData;
    }
    emit voltageDataReceived(voltageData);
}

void TestBoardDevice::onFaultStatusReceived(const Protocols::FaultFeedbackData &data)
{
    {
        QMutexLocker locker(&m_mutex);
        m_lastFaultState = data.faultState;
    }
    emit faultStatusReceived(data.faultState);
}

void TestBoardDevice::onCalibrationResultReceived(const Protocols::CalibrationFeedbackData &data)
{
    QJsonObject result;
    result["serialNumber"] = static_cast<qint64>(data.serialNumber);
    result["state"] = data.state;
    result["command"] = QString(data.command.toHex());
    emit calibrationResultReceived(result);
}

void TestBoardDevice::onRegisterWriteResultReceived(const Protocols::RegisterWriteFeedbackData &data)
{
    QJsonObject result;
    result["serialNumber"] = static_cast<qint64>(data.serialNumber);
    result["state"] = data.state;
    result["dutSel"] = data.dutSel;
    result["regAddr"] = data.regAddr;
    result["length"] = data.length;
    result["values"] = QString(data.values.toHex());
    emit registerWriteResultReceived(result);
}

void TestBoardDevice::onRegisterReadResultReceived(const Protocols::RegisterReadFeedbackData &data)
{
    QJsonObject result;
    result["serialNumber"] = static_cast<qint64>(data.serialNumber);
    result["state"] = data.state;
    result["dutSel"] = data.dutSel;
    result["regAddr"] = data.regAddr;
    result["length"] = data.length;
    result["values"] = QString(data.values.toHex());
    emit registerReadResultReceived(result);
}

void TestBoardDevice::onSetChipTypeResultReceived(const Protocols::SetChipTypeFeedbackData &data)
{
    QJsonObject result;
    result["serialNumber"] = static_cast<qint64>(data.serialNumber);
    result["state"] = data.state;
    result["chipIndex"] = data.chipIndex;
    
    QString chipName;
    switch(data.chipIndex) {
        case 1: chipName = "A300"; break;
        case 2: chipName = "G300"; break;
        case 3: chipName = "270"; break;
        default: chipName = QString("Unknown(%1)").arg(data.chipIndex); break;
    }
    result["chipName"] = chipName;
    
    emit chipTypeSetResultReceived(result);
}

void TestBoardDevice::onCommunicationError(const QString &error)
{
    QMutexLocker locker(&m_mutex);
    
    m_lastError = tr("通信错误: %1").arg(error);
    m_status = IDevice::DeviceStatus::Error;
    emit dataError(m_lastError);
}

void TestBoardDevice::onProtocolError(const QString &error)
{
    QMutexLocker locker(&m_mutex);
    
    m_lastError = tr("协议错误: %1").arg(error);
    emit dataError(m_lastError);
}

} // namespace Domain
