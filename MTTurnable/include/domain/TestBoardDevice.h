#ifndef TESTBOARDDEVICE_H
#define TESTBOARDDEVICE_H

#include "domain/IDevice.h"
#include "domain/protocols/IXTProtocol.h" // 包含协议定义
#include "infrastructure/BinLogger.h" // 引入BinLogger头文件
#include <QTimer>
#include <QMutex>
#include <memory>
#include <QVariantMap>

namespace Infrastructure {
class ICommunicationChannel;
class SerialPortManager;
}

namespace Domain {

// 使用协议层定义的数据结构
using TestBoardData = Protocols::TestFeedbackData;
using DutGyroData = Protocols::DutGyroData;
using ExternalGyroData = Protocols::ExternalGyroData;

/**
 * @brief 测试板设备实现类
 * 基于XT小转台RS-422通信协议
 * 使用四层架构：应用层(本类) -> 协议层 -> 传输层 -> 通信层
 */
class TestBoardDevice : public IDevice
{
    Q_OBJECT
public:
    explicit TestBoardDevice(
        std::unique_ptr<Protocols::IXTProtocol> protocol,
        QObject *parent = nullptr
    );
    ~TestBoardDevice() override;

    // IDevice接口实现
    QString getName() const override;
    QString getDeviceName() const;  // 添加缺少的方法声明
    QString getDescription() const override { return "XT Turntable Test Board via RS-422"; }
    DeviceType getType() const override { return DeviceType::TestBoard; }
    DeviceStatus getStatus() const override;

    bool initialize() override;
    bool release() override;
    
    QJsonObject executeCommand(const QString &command, const QJsonObject &params) override;
    
    void setCommunicationChannel(std::shared_ptr<Infrastructure::ICommunicationChannel> channel) override;
    std::shared_ptr<Infrastructure::ICommunicationChannel> getCommunicationChannel() const override;
    
    void setConfiguration(const QJsonObject &config) override;
    QJsonObject getConfiguration() const override;
    
    bool selfTest() override;

    // 设备特定方法
    bool reset();
    QJsonObject getCapabilities() const;
    QJsonObject getLastError() const; // 移除override以解决编译问题
    Protocols::IXTProtocol* getProtocol() const;

    // 测试控制
    bool startTest(uint16_t dutActive);
    bool stopTest();
    bool isTestRunning() const;
    
    // DUT电源控制
    bool controlDutPower(bool enable, uint16_t dutEnable = 0xFFFF);
    
    // 查询功能
    QJsonObject queryVoltageStatus();
    QJsonObject queryFaultStatus();
    
    // 获取最新测试数据
    TestBoardData getLatestData() const;
    
    // 获取所有通道的温度
    QVector<double> getAllTemperatures() const;

signals:
    void testDataReceived(const TestBoardData &data);
    void testStateChanged(bool isRunning);
    void powerStateChanged(const Protocols::PowerFeedbackData &data);
    void voltageDataReceived(const QJsonObject &voltageData);
    void faultStatusReceived(uint32_t faultState);
    void dataError(const QString &error);

    // 新增
    void calibrationResultReceived(const QJsonObject &result);
    void registerWriteResultReceived(const QJsonObject &result);
    void registerReadResultReceived(const QJsonObject &result);
    void chipTypeSetResultReceived(const QJsonObject &result);

    // 实时测试数据，按 DUT 维度抛出
    void testDataAvailable(const QString &dutId, const QVariantMap &data);

private slots:
    // 处理协议层的数据
    void onTestDataReceived(const Protocols::TestFeedbackData &data);
    void onPowerStateChanged(const Protocols::PowerFeedbackData &data);
    void onVoltageDataReceived(const Protocols::VoltageFeedbackData &data);
    void onFaultStatusReceived(const Protocols::FaultFeedbackData &data);
    void onCalibrationResultReceived(const Protocols::CalibrationFeedbackData &data);
    void onRegisterWriteResultReceived(const Protocols::RegisterWriteFeedbackData &data);
    void onRegisterReadResultReceived(const Protocols::RegisterReadFeedbackData &data);
    void onSetChipTypeResultReceived(const Protocols::SetChipTypeFeedbackData &data);
    
    // 处理传输层的数据包
    // void onPacketReady(uint16_t cmdId, const QByteArray &cmdData); // 移除
    
    // 处理通信层的错误
    void onCommunicationError(const QString &error);
    void onProtocolError(const QString &error);

private:
    void connectSignals();
    
    // 发送命令的辅助方法
    bool sendCommand(const QByteArray &fullPacket); // 修改为发送完整包
    
    // 数据成员
    mutable QMutex m_mutex;
    DeviceStatus m_status;
    
    // 四层架构组件
    std::shared_ptr<Infrastructure::ICommunicationChannel> m_commChannel;
    std::unique_ptr<Protocols::IXTProtocol> m_protocol;
    std::unique_ptr<Infrastructure::BinLogger> m_logger; // 新增BinLogger成员
    
    // 业务数据
    bool m_isTestRunning;
    TestBoardData m_latestData;
    
    // 配置
    QJsonObject m_configuration;
    
    // 错误信息
    QString m_lastError;
    int m_lastErrorCode;
    
    // 查询结果缓存
    QJsonObject m_lastVoltageData;
    uint32_t m_lastFaultState;
    qint64 m_testFeedbackLogCounter; // 日志节流计数器
};

} // namespace Domain

#endif // TESTBOARDDEVICE_H
