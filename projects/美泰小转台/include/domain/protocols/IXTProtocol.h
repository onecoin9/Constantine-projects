#ifndef IXTPROTOCOL_H
#define IXTPROTOCOL_H

#include <QObject>
#include <QByteArray>
#include <QJsonObject>
#include <optional>
#include <memory>

namespace Domain {
    class IProtocolFramer; // Forward declaration

namespace Protocols {

// 恢复数据结构定义
#pragma pack(push, 1)

struct DutGyroData {
    int32_t gyro_x;
    int32_t gyro_y;
    int32_t gyro_z;
    int32_t gyro_acc_x;
    int32_t gyro_acc_y;
    int32_t gyro_acc_z;
    int32_t gyro_mix;
    int16_t gyro_temperature;
};

struct ExternalGyroData {
    float gyro_x;
    float gyro_y;
    float gyro_z;
    float gyro_acc_x;
    float gyro_acc_y;
    float gyro_acc_z;
    float gyro_mix;
    int16_t gyro_temperature;
    int16_t gyro_counter;
};

struct TestFeedbackData {
    uint8_t testState;
    uint32_t serialNumber;
    uint32_t timestamp;
    uint16_t dutActive;
    uint8_t chipIndex;
    DutGyroData dutData[8];
    ExternalGyroData externalGyro;
};

struct PowerFeedbackData {
    uint32_t serialNumber;
    uint8_t state;
    uint16_t dutPowerState;
};

struct VoltageFeedbackData {
    uint32_t serialNumber;
    uint16_t mainVoltage;
    uint16_t mainCurrent;
    struct {
        uint16_t voltage_5v;
        uint16_t current_5v;
        uint16_t voltage_3v3;
        uint16_t current_3v3;
    } dutPower[8];
};

struct FaultFeedbackData {
    uint32_t serialNumber;
    uint32_t faultState;
};

struct CalibrationFeedbackData {
    uint32_t serialNumber;
    uint8_t state;
    QByteArray command;
};

struct RegisterWriteFeedbackData {
    uint32_t serialNumber;
    uint8_t state;
    uint8_t dutSel;
    uint8_t regAddr;
    uint8_t length;
    QByteArray values;
};

struct RegisterReadFeedbackData {
    uint32_t serialNumber;
    uint8_t state;
    uint8_t dutSel;
    uint8_t regAddr;
    uint8_t length;
    QByteArray values;
};

struct SetChipTypeFeedbackData {
    uint32_t serialNumber;
    uint8_t state;
    uint8_t chipIndex;
};

#pragma pack(pop)


class IXTProtocol : public QObject
{
    Q_OBJECT
public:
    // 添加接收parent的构造函数
    explicit IXTProtocol(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IXTProtocol() = default;

    // 恢复CommandId枚举
    enum CommandId {
        CMD_START_STOP = 0x01,
        CMD_POWER_CONTROL = 0x02,
        CMD_VOLTAGE_QUERY = 0x03,
        CMD_FAULT_QUERY = 0x04,
        CMD_CALIBRATION = 0x05,
        CMD_REG_WRITE = 0x06,
        CMD_REG_READ = 0x07,
        CMD_SET_CHIP_TYPE = 0x08,
        
        CMD_TEST_FEEDBACK = 0x8001,
        CMD_POWER_FEEDBACK = 0x8002,
        CMD_VOLTAGE_FEEDBACK = 0x8003,
        CMD_FAULT_FEEDBACK = 0x8004,
        CMD_CALIBRATION_FEEDBACK = 0x8005,
        CMD_REG_WRITE_FEEDBACK = 0x8006,
        CMD_REG_READ_FEEDBACK = 0x8007,
        CMD_SET_CHIP_TYPE_FEEDBACK = 0x8008
    };

    virtual void setFramer(std::unique_ptr<IProtocolFramer> framer) = 0;
    virtual void appendData(const QByteArray& data) = 0;
    virtual void clearBuffer() = 0;
    virtual bool parseResponse(uint16_t cmdId, const QByteArray& data) = 0;
    virtual QString getLastError() const = 0;

    virtual QByteArray createStartCommand(bool start, uint16_t dutActive, uint32_t timestamp) = 0;
    virtual QByteArray createPowerControlCommand(bool enable, uint16_t dutPowerEnable) = 0;
    virtual QByteArray createVoltageQueryCommand() = 0;
    virtual QByteArray createFaultQueryCommand() = 0;
    virtual QByteArray createCalibrationCommand(uint8_t dutSel, const QByteArray& command) = 0;
    virtual QByteArray createRegisterWriteCommand(uint8_t dutSel, uint8_t regAddr, const QByteArray& data) = 0;
    virtual QByteArray createRegisterReadCommand(uint8_t dutSel, uint8_t regAddr, uint8_t length) = 0;
    virtual QByteArray createSetChipTypeCommand(uint8_t chipIndex) = 0;
    
    virtual QString formatPacketForDisplay(const QByteArray& rawPacket) = 0;
    virtual QString formatSentPacketForDisplay(const QByteArray& rawPacket) = 0;

signals:
    void testDataReceived(const Protocols::TestFeedbackData& data);
    void powerStateChanged(const Protocols::PowerFeedbackData& data);
    void voltageDataReceived(const Protocols::VoltageFeedbackData& data);
    void faultStatusReceived(const Protocols::FaultFeedbackData& data);
    void calibrationResultReceived(const Protocols::CalibrationFeedbackData& data);
    void registerWriteResultReceived(const Protocols::RegisterWriteFeedbackData& data);
    void registerReadResultReceived(const Protocols::RegisterReadFeedbackData& data);
    void chipTypeSetResultReceived(const Protocols::SetChipTypeFeedbackData& data);
    void protocolError(const QString& error);
    void rawPacketReady(const QByteArray& rawPacket);
};

} // namespace Protocols
} // namespace Domain

// 统一的类型声明，让Qt知道如何处理这些类型
Q_DECLARE_METATYPE(Domain::Protocols::TestFeedbackData)
Q_DECLARE_METATYPE(Domain::Protocols::PowerFeedbackData)
Q_DECLARE_METATYPE(Domain::Protocols::VoltageFeedbackData)
Q_DECLARE_METATYPE(Domain::Protocols::FaultFeedbackData)
Q_DECLARE_METATYPE(Domain::Protocols::CalibrationFeedbackData)
Q_DECLARE_METATYPE(Domain::Protocols::RegisterWriteFeedbackData)
Q_DECLARE_METATYPE(Domain::Protocols::RegisterReadFeedbackData)
Q_DECLARE_METATYPE(Domain::Protocols::SetChipTypeFeedbackData)

#endif // IXTPROTOCOL_H 