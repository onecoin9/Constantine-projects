#ifndef XTPROTOCOL_H
#define XTPROTOCOL_H

#include "IXTProtocol.h"
#include "domain/IProtocolFramer.h" // 包含完整头文件
#include <QMutex>
#include <memory> // For std::unique_ptr

namespace Domain {
namespace Protocols {

/**
 * @brief XT协议实现类
 * 实现XT小转台协议的命令创建和响应解析
 */
class XTProtocol : public IXTProtocol
{
    Q_OBJECT
public:
    explicit XTProtocol(QObject *parent = nullptr);
    ~XTProtocol() override;
    
    void setFramer(std::unique_ptr<IProtocolFramer> framer) override;
    void appendData(const QByteArray& data) override;
    void clearBuffer() override;
    bool parseResponse(uint16_t cmdId, const QByteArray& data) override;
    QString getLastError() const override;

    QByteArray createStartCommand(bool start, uint16_t dutActive, uint32_t timestamp) override;
    QByteArray createPowerControlCommand(bool enable, uint16_t dutPowerEnable) override;
    QByteArray createVoltageQueryCommand() override;
    QByteArray createFaultQueryCommand() override;
    QByteArray createCalibrationCommand(uint8_t dutSel, const QByteArray& command) override;
    QByteArray createRegisterWriteCommand(uint8_t dutSel, uint8_t regAddr, const QByteArray& data) override;
    QByteArray createRegisterReadCommand(uint8_t dutSel, uint8_t regAddr, uint8_t length) override;
    QByteArray createSetChipTypeCommand(uint8_t chipIndex) override;
    
    QString formatPacketForDisplay(const QByteArray& rawPacket) override;
    QString formatSentPacketForDisplay(const QByteArray& rawPacket) override;

private:
    // 关键修复：新增一个私有辅助函数用于格式化
    QString formatPacketDetails(uint16_t cmdId, const QByteArray& data) const;
    
    std::optional<TestFeedbackData> parseTestFeedbackPacket(const QByteArray& data);
    std::optional<PowerFeedbackData> parsePowerFeedbackPacket(const QByteArray& data);
    std::optional<VoltageFeedbackData> parseVoltageFeedbackPacket(const QByteArray& data);
    std::optional<FaultFeedbackData> parseFaultFeedbackPacket(const QByteArray& data);
    std::optional<CalibrationFeedbackData> parseCalibrationFeedbackPacket(const QByteArray& data);
    std::optional<RegisterWriteFeedbackData> parseRegisterWriteFeedbackPacket(const QByteArray& data);
    std::optional<RegisterReadFeedbackData> parseRegisterReadFeedbackPacket(const QByteArray& data);
    std::optional<SetChipTypeFeedbackData> parseSetChipTypeFeedbackPacket(const QByteArray& data);
    
    template<typename T>
    void appendLittleEndian(QByteArray& data, T value) const;
    
    // 辅助方法
    template<typename T>
    T readLittleEndian(const uint8_t* data) const;
    
    void parseDutGyroData(const uint8_t* data, DutGyroData& dut);
    void parseExternalGyroData(const uint8_t* data, ExternalGyroData& gyro);
    
    // 数据成员
    mutable QMutex m_mutex;
    QString m_lastError;
    qint64 m_logCounter; // 日志节流计数器
    std::unique_ptr<IProtocolFramer> m_framer; // 持有Framer实例
};

} // namespace Protocols
} // namespace Domain

#endif // XTPROTOCOL_H 