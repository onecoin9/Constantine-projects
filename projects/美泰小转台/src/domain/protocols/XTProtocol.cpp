#include "domain/protocols/XTProtocol.h"
#include "utils/ByteUtils.h"
#include <QMutexLocker>
#include <QDateTime>
#include <QThread>
#include <optional>
#include <QTextStream>
#include <Logger.h>
#include <QMetaType> // 关键修复：引入头文件

namespace Domain {
namespace Protocols {

// 关键修复：使用静态对象在程序启动时自动注册所有自定义类型。
// 这样既能保证注册时机足够早，又能将注册代码保留在所属模块内。
namespace {
    struct TypeRegistrar {
        TypeRegistrar() {
            // 使用Q_DECLARE_METATYPE后，只需注册一次，使用完整命名空间
            qRegisterMetaType<Domain::Protocols::TestFeedbackData>();
            qRegisterMetaType<Domain::Protocols::PowerFeedbackData>();
            qRegisterMetaType<Domain::Protocols::VoltageFeedbackData>();
            qRegisterMetaType<Domain::Protocols::FaultFeedbackData>();
            qRegisterMetaType<Domain::Protocols::CalibrationFeedbackData>();
            qRegisterMetaType<Domain::Protocols::RegisterWriteFeedbackData>();
            qRegisterMetaType<Domain::Protocols::RegisterReadFeedbackData>();
            qRegisterMetaType<Domain::Protocols::SetChipTypeFeedbackData>();
        }
        // 添加一个无意义的函数，以防止链接器在优化时丢弃这个静态对象
        void touch() const {}
    };
    // 声明一个静态实例，它的构造函数将在main()之前被调用。
    const static TypeRegistrar registrar;
}


// 定义日志节流的速率
static constexpr int LOG_THROTTLE_RATE = 500; // 每500个包记录一次日志

XTProtocol::XTProtocol(QObject *parent)
    : IXTProtocol(parent)
    , m_logCounter(0)
{
    // 显式“接触”静态实例，以确保其构造函数被链接和执行。
    registrar.touch();
}

XTProtocol::~XTProtocol()
{
    // 这个空的析构函数定义对于解决vtable链接错误至关重要
}

void XTProtocol::setFramer(std::unique_ptr<IProtocolFramer> framer)
{
    m_framer = std::move(framer);
    connect(m_framer.get(), &IProtocolFramer::packetReady,
                this, &XTProtocol::parseResponse, Qt::QueuedConnection);
                
    // 新增：连接原始数据包信号，用于数据记录
    connect(m_framer.get(), &IProtocolFramer::rawPacketReady,
                this, &XTProtocol::rawPacketReady, Qt::QueuedConnection);
}


QByteArray XTProtocol::createStartCommand(bool start, uint16_t dutActive, uint32_t timestamp)
{
    QByteArray cmdData;
    cmdData.reserve(7);
    cmdData.append(start ? 0x01 : 0x00);
    appendLittleEndian(cmdData, dutActive);
    appendLittleEndian(cmdData, timestamp);

    if (!m_framer) return QByteArray();
    return m_framer->buildPacket(CMD_START_STOP, cmdData);
}

QByteArray XTProtocol::createPowerControlCommand(bool enable, uint16_t dutPowerEnable)
{
    QByteArray cmdData;
    cmdData.reserve(3);
    cmdData.append(enable ? 0x01 : 0x00);
    appendLittleEndian(cmdData, dutPowerEnable);
    
    if (!m_framer) return QByteArray();
    return m_framer->buildPacket(CMD_POWER_CONTROL, cmdData);
}

QByteArray XTProtocol::createVoltageQueryCommand()
{
    if (!m_framer) return QByteArray();
    return m_framer->buildPacket(CMD_VOLTAGE_QUERY, QByteArray());
}

QByteArray XTProtocol::createFaultQueryCommand()
{
    if (!m_framer) return QByteArray();
    return m_framer->buildPacket(CMD_FAULT_QUERY, QByteArray());
}

QByteArray XTProtocol::createCalibrationCommand(uint8_t dutSel, const QByteArray& command)
{
    QByteArray cmdData;
    cmdData.reserve(2 + command.size());
    cmdData.append(static_cast<char>(dutSel));
    cmdData.append(static_cast<char>(command.size()));
    cmdData.append(command);
    
    if (!m_framer) return QByteArray();
    return m_framer->buildPacket(CMD_CALIBRATION, cmdData);
}

QByteArray XTProtocol::createRegisterWriteCommand(uint8_t dutSel, uint8_t regAddr, const QByteArray& data)
{
    QByteArray cmdData;
    cmdData.reserve(3 + data.size());
    cmdData.append(static_cast<char>(dutSel));
    cmdData.append(static_cast<char>(regAddr));
    cmdData.append(static_cast<char>(data.size()));
    cmdData.append(data);
    
    if (!m_framer) return QByteArray();
    return m_framer->buildPacket(CMD_REG_WRITE, cmdData);
}

QByteArray XTProtocol::createRegisterReadCommand(uint8_t dutSel, uint8_t regAddr, uint8_t length)
{
    QByteArray cmdData;
    cmdData.reserve(3);
    cmdData.append(static_cast<char>(dutSel));
    cmdData.append(static_cast<char>(regAddr));
    cmdData.append(static_cast<char>(length));
    
    if (!m_framer) return QByteArray();
    return m_framer->buildPacket(CMD_REG_READ, cmdData);
}

QByteArray XTProtocol::createSetChipTypeCommand(uint8_t chipIndex)
{
    QByteArray cmdData;
    cmdData.append(static_cast<char>(chipIndex));
    
    if (!m_framer) return QByteArray();
    return m_framer->buildPacket(CMD_SET_CHIP_TYPE, cmdData);
}

bool XTProtocol::parseResponse(uint16_t cmdId, const QByteArray& data)
{  
    QMutexLocker locker(&m_mutex);
    
    bool result = false;
    
    switch (cmdId) {
        case CMD_TEST_FEEDBACK:
            if(auto feedback = parseTestFeedbackPacket(data)) {
                emit testDataReceived(feedback.value());
                result = true;
            }
            break;
            
        case CMD_POWER_FEEDBACK:
            if(auto feedback = parsePowerFeedbackPacket(data)) {
                emit powerStateChanged(feedback.value());
                result = true;
            }
            break;
            
        case CMD_VOLTAGE_FEEDBACK:
            if(auto feedback = parseVoltageFeedbackPacket(data)) {
                emit voltageDataReceived(feedback.value());
                result = true;
            }
            break;
            
        case CMD_FAULT_FEEDBACK:
            if(auto feedback = parseFaultFeedbackPacket(data)) {
                emit faultStatusReceived(feedback.value());
                result = true;
            }
            break;
            
        case CMD_CALIBRATION_FEEDBACK:
            if(auto feedback = parseCalibrationFeedbackPacket(data)) {
                emit calibrationResultReceived(feedback.value());
                result = true;
            }
            break;
            
        case CMD_REG_WRITE_FEEDBACK:
            if(auto feedback = parseRegisterWriteFeedbackPacket(data)) {
                emit registerWriteResultReceived(feedback.value());
                result = true;
            }
            break;
            
        case CMD_REG_READ_FEEDBACK:
            if(auto feedback = parseRegisterReadFeedbackPacket(data)) {
                emit registerReadResultReceived(feedback.value());
                result = true;
            }
            break;
            
        case CMD_SET_CHIP_TYPE_FEEDBACK:
            if(auto feedback = parseSetChipTypeFeedbackPacket(data)) {
                emit chipTypeSetResultReceived(feedback.value());
                result = true;
            }
            break;
            
        default:
            m_lastError = QString("未知的命令ID: 0x%1").arg(cmdId, 4, 16, QChar('0'));
            emit protocolError(m_lastError);
            break;
    }
    
    // 不再需要EXIT日志，简化
    return result;
}

QString XTProtocol::getLastError() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastError;
}

// --- 实现新的纯解析函数 ---

std::optional<TestFeedbackData> XTProtocol::parseTestFeedbackPacket(const QByteArray& data)
{
    const int expectedSize = 284;
    if (data.size() != expectedSize) {
        return std::nullopt;
    }

    TestFeedbackData feedback;
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data.constData());
    
    feedback.testState = *ptr++;
    feedback.serialNumber = Utils::ByteUtils::readLittleEndian<uint32_t>(ptr); ptr += 4;
    feedback.timestamp = Utils::ByteUtils::readLittleEndian<uint32_t>(ptr); ptr += 4;
    feedback.dutActive = Utils::ByteUtils::readLittleEndian<uint16_t>(ptr); ptr += 2;
    feedback.chipIndex = *ptr++;
    
    for (int i = 0; i < 8; ++i) {
        parseDutGyroData(ptr, feedback.dutData[i]);
        ptr += 30;
    }
    
    parseExternalGyroData(ptr, feedback.externalGyro);
    
    return feedback;
}

std::optional<PowerFeedbackData> XTProtocol::parsePowerFeedbackPacket(const QByteArray& data) 
{
    const int expectedSize = 7;
    if (data.size() != expectedSize) {
        return std::nullopt;
    }

    PowerFeedbackData feedback;
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data.constData());
    feedback.serialNumber = Utils::ByteUtils::readLittleEndian<uint32_t>(ptr); ptr += 4;
    feedback.state = *ptr++;
    feedback.dutPowerState = Utils::ByteUtils::readLittleEndian<uint16_t>(ptr);
    return feedback;
}

std::optional<VoltageFeedbackData> XTProtocol::parseVoltageFeedbackPacket(const QByteArray& data)
{
    const int expectedSize = 72;
    if (data.size() != expectedSize) {
        return std::nullopt;
    }
    
    VoltageFeedbackData feedback;
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data.constData());
    feedback.serialNumber = Utils::ByteUtils::readLittleEndian<uint32_t>(ptr); ptr += 4;
    feedback.mainVoltage = Utils::ByteUtils::readLittleEndian<uint16_t>(ptr); ptr += 2;
    feedback.mainCurrent = Utils::ByteUtils::readLittleEndian<uint16_t>(ptr); ptr += 2;
    for (int i = 0; i < 8; ++i) {
        feedback.dutPower[i].voltage_5v = Utils::ByteUtils::readLittleEndian<uint16_t>(ptr); ptr += 2;
        feedback.dutPower[i].current_5v = Utils::ByteUtils::readLittleEndian<uint16_t>(ptr); ptr += 2;
        feedback.dutPower[i].voltage_3v3 = Utils::ByteUtils::readLittleEndian<uint16_t>(ptr); ptr += 2;
        feedback.dutPower[i].current_3v3 = Utils::ByteUtils::readLittleEndian<uint16_t>(ptr); ptr += 2;
    }
    return feedback;
}

std::optional<FaultFeedbackData> XTProtocol::parseFaultFeedbackPacket(const QByteArray& data) {
    const int expectedSize = 8;
    if (data.size() != expectedSize) return std::nullopt;
    FaultFeedbackData feedback;
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data.constData());
    feedback.serialNumber = Utils::ByteUtils::readLittleEndian<uint32_t>(ptr); ptr += 4;
    feedback.faultState = Utils::ByteUtils::readLittleEndian<uint32_t>(ptr);
    return feedback;
}

std::optional<CalibrationFeedbackData> XTProtocol::parseCalibrationFeedbackPacket(const QByteArray& data) {
    if (data.size() < 5) return std::nullopt;
    CalibrationFeedbackData feedback;
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data.constData());
    feedback.serialNumber = Utils::ByteUtils::readLittleEndian<uint32_t>(ptr); ptr += 4;
    feedback.state = *ptr++;
    int commandSize = data.size() - 5;
    if (commandSize > 0) {
        feedback.command = QByteArray(reinterpret_cast<const char*>(ptr), commandSize);
    }
    return feedback;
}

std::optional<RegisterWriteFeedbackData> XTProtocol::parseRegisterWriteFeedbackPacket(const QByteArray& data) {
    if (data.size() < 8) return std::nullopt;
    RegisterWriteFeedbackData feedback;
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data.constData());
    feedback.serialNumber = Utils::ByteUtils::readLittleEndian<uint32_t>(ptr); ptr += 4;
    feedback.state = *ptr++;
    feedback.dutSel = *ptr++;
    feedback.regAddr = *ptr++;
    feedback.length = *ptr++;
    int valuesSize = data.size() - 8;
    if (valuesSize > 0) {
        feedback.values = QByteArray(reinterpret_cast<const char*>(ptr), valuesSize);
    }
    return feedback;
}

std::optional<RegisterReadFeedbackData> XTProtocol::parseRegisterReadFeedbackPacket(const QByteArray& data) {
    if (data.size() < 8) return std::nullopt;
    RegisterReadFeedbackData feedback;
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data.constData());
    feedback.serialNumber = Utils::ByteUtils::readLittleEndian<uint32_t>(ptr); ptr += 4;
    feedback.state = *ptr++;
    feedback.dutSel = *ptr++;
    feedback.regAddr = *ptr++;
    feedback.length = *ptr++;
    int valuesSize = data.size() - 8;
    if (valuesSize > 0) {
        feedback.values = QByteArray(reinterpret_cast<const char*>(ptr), valuesSize);
    }
    return feedback;
}

std::optional<SetChipTypeFeedbackData> XTProtocol::parseSetChipTypeFeedbackPacket(const QByteArray& data) {
    const int expectedSize = 6;
    if (data.size() != expectedSize) return std::nullopt;
    SetChipTypeFeedbackData feedback;
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data.constData());
    feedback.serialNumber = Utils::ByteUtils::readLittleEndian<uint32_t>(ptr); ptr += 4;
    feedback.state = *ptr++;
    feedback.chipIndex = *ptr;
    return feedback;
}


void XTProtocol::parseDutGyroData(const uint8_t* data, DutGyroData& dut)
{
    int32_t gyro_x = Utils::ByteUtils::readLittleEndian<int32_t>(data); data += 4;
    int32_t gyro_y = Utils::ByteUtils::readLittleEndian<int32_t>(data); data += 4;
    int32_t gyro_z = Utils::ByteUtils::readLittleEndian<int32_t>(data); data += 4;
    int32_t gyro_acc_x = Utils::ByteUtils::readLittleEndian<int32_t>(data); data += 4;
    int32_t gyro_acc_y = Utils::ByteUtils::readLittleEndian<int32_t>(data); data += 4;
    int32_t gyro_acc_z = Utils::ByteUtils::readLittleEndian<int32_t>(data); data += 4;
    int32_t gyro_mix = Utils::ByteUtils::readLittleEndian<int32_t>(data); data += 4;
    int16_t gyro_temperature = Utils::ByteUtils::readLittleEndian<int16_t>(data);
    
    // 将0xFF值转换为-1进行显示
    dut.gyro_x = (gyro_x == 0xFFFFFFFF) ? static_cast<int32_t>(-1) : gyro_x;
    dut.gyro_y = (gyro_y == 0xFFFFFFFF) ? static_cast<int32_t>(-1) : gyro_y;
    dut.gyro_z = (gyro_z == 0xFFFFFFFF) ? static_cast<int32_t>(-1) : gyro_z;
    dut.gyro_acc_x = (gyro_acc_x == 0xFFFFFFFF) ? static_cast<int32_t>(-1) : gyro_acc_x;
    dut.gyro_acc_y = (gyro_acc_y == 0xFFFFFFFF) ? static_cast<int32_t>(-1) : gyro_acc_y;
    dut.gyro_acc_z = (gyro_acc_z == 0xFFFFFFFF) ? static_cast<int32_t>(-1) : gyro_acc_z;
    dut.gyro_mix = (gyro_mix == 0xFFFFFFFF) ? static_cast<int32_t>(-1) : gyro_mix;
    dut.gyro_temperature = (gyro_temperature == 0xFFFF) ? static_cast<int16_t>(-1) : gyro_temperature;
}

void XTProtocol::parseExternalGyroData(const uint8_t* data, ExternalGyroData& gyro)
{
    float gyro_x = *(float*)(data); data += 4;
    float gyro_y = *(float*)(data); data += 4;
    float gyro_z = *(float*)(data); data += 4;
    float gyro_acc_x = *(float*)(data); data += 4;
    float gyro_acc_y = *(float*)(data); data += 4;
    float gyro_acc_z = *(float*)(data); data += 4;
    float gyro_mix = *(float*)(data); data += 4;
    int16_t gyro_temperature = Utils::ByteUtils::readLittleEndian<int16_t>(data); data += 2;
    int16_t gyro_counter = Utils::ByteUtils::readLittleEndian<int16_t>(data);
    
    // 将0xFF值转换为-1进行显示
    gyro.gyro_x = gyro_x;
    gyro.gyro_y = gyro_y;
    gyro.gyro_z = gyro_z;
    gyro.gyro_acc_x = gyro_acc_x;
    gyro.gyro_acc_y = gyro_acc_y;
    gyro.gyro_acc_z = gyro_acc_z;
    gyro.gyro_mix = gyro_mix;
    gyro.gyro_temperature = (gyro_temperature == 0xFFFF) ? static_cast<int16_t>(-1) : gyro_temperature;
    gyro.gyro_counter = (gyro_counter == 0xFFFF) ? static_cast<int16_t>(-1) : gyro_counter;
}

template<typename T>
void XTProtocol::appendLittleEndian(QByteArray& data, T value) const
{
    for (size_t i = 0; i < sizeof(T); ++i) {
        data.append(static_cast<char>(value & 0xFF));
        value >>= 8;
    }
}

// 显式实例化模板
template void XTProtocol::appendLittleEndian<uint16_t>(QByteArray&, uint16_t) const;
template void XTProtocol::appendLittleEndian<uint32_t>(QByteArray&, uint32_t) const;

// --- 统一的格式化函数实现 ---
QString XTProtocol::formatPacketForDisplay(const QByteArray& rawPacket)
{
    if (rawPacket.size() < 8) {
        return "Error: Packet too short\n";
    }

    uint16_t cmdId = Utils::ByteUtils::readLittleEndian<uint16_t>(reinterpret_cast<const uint8_t*>(rawPacket.constData()) + 4);
    QByteArray cmdData = rawPacket.mid(8, rawPacket.size() - 9); // Exclude header and CRC

    QString details;
    QTextStream stream(&details);
    stream.setIntegerBase(10); // 默认十进制
    stream << "--- [RECV] Packet (CmdID: 0x" << QString::number(cmdId, 16).toUpper().rightJustified(4, '0') 
           << ", Size: " << rawPacket.size() << " bytes) ---\n";
    stream << "Raw: " << rawPacket.toHex(' ').toUpper() << "\n\n";
    stream << "Parsed Content:\n";

    switch (cmdId) {
        case CMD_TEST_FEEDBACK: {
            if (auto data = parseTestFeedbackPacket(cmdData)) {
                QDateTime timestamp = QDateTime::fromSecsSinceEpoch(data->timestamp);
                stream << "  Test State: " << data->testState << " (0x01: Testing, 0x00: Stopped, 0x02: Fault)\n";
                stream << "  Serial Number: " << data->serialNumber << "\n";
                stream << "  Timestamp: " << timestamp.toString("yyyy-MM-dd hh:mm:ss") << " (raw: " << data->timestamp << ")\n";
                stream << "  DUT Active: 0x" << QString::number(data->dutActive, 16).toUpper() << "\n";
                stream << "  Chip Index: " << data->chipIndex << "\n";
                
                // 输出所有DUT的数据
                for (int i = 0; i < 8; ++i) {
                    if ((data->dutActive >> i) & 0x01) {
                        stream << "  DUT " << (i + 1) << " Data:\n";
                        stream << "    Gyro (X,Y,Z): " << data->dutData[i].gyro_x << ", " 
                               << data->dutData[i].gyro_y << ", " << data->dutData[i].gyro_z << "\n";
                        stream << "    Acc (X,Y,Z): " << data->dutData[i].gyro_acc_x << ", " 
                               << data->dutData[i].gyro_acc_y << ", " << data->dutData[i].gyro_acc_z << "\n";
                        stream << "    Mix: " << data->dutData[i].gyro_mix << "\n";
                        stream << "    Temperature: " << data->dutData[i].gyro_temperature << " C\n";
                    }
                }
                
                // 输出外部陀螺仪数据
                if ((data->dutActive >> 15) & 0x01) {
                    stream << "  External Gyro Data:\n";
                    stream << "    Gyro (X,Y,Z): " << data->externalGyro.gyro_x << ", " 
                           << data->externalGyro.gyro_y << ", " << data->externalGyro.gyro_z << "\n";
                    stream << "    Acc (X,Y,Z): " << data->externalGyro.gyro_acc_x << ", " 
                           << data->externalGyro.gyro_acc_y << ", " << data->externalGyro.gyro_acc_z << "\n";
                    stream << "    Mix: " << data->externalGyro.gyro_mix << "\n";
                    stream << "    Temperature: " << data->externalGyro.gyro_temperature << " C\n";
                    stream << "    Counter: " << data->externalGyro.gyro_counter << " ms\n";
                }
            } else {
                stream << "  [Parse Error: Invalid Data Format]\n";
            }
            break;
        }
        case CMD_POWER_FEEDBACK: {
            if (auto data = parsePowerFeedbackPacket(cmdData)) {
                stream << "  Serial Number: " << data->serialNumber << "\n";
                stream << "  State: " << data->state << " (1: Success, 0: Fail)\n";
                stream << "  DUT Power State: 0b" << QString::number(data->dutPowerState, 2).rightJustified(16, '0') << "\n";
            } else {
                stream << "  [Parse Error: Invalid Data Format]\n";
            }
            break;
        }
        case CMD_VOLTAGE_FEEDBACK: {
            if (auto data = parseVoltageFeedbackPacket(cmdData)) {
                stream << "  Serial Number: " << data->serialNumber << "\n";
                stream << "  Main Voltage: " << data->mainVoltage << " mV\n";
                stream << "  Main Current: " << data->mainCurrent << " mA\n";
                for (int i = 0; i < 8; ++i) {
                     stream << "  DUT " << i + 1 << ": 5V:" << data->dutPower[i].voltage_5v << "mV "
                            << data->dutPower[i].current_5v << "mA, 3.3V:" 
                            << data->dutPower[i].voltage_3v3 << "mV "
                            << data->dutPower[i].current_3v3 << "mA\n";
                }
            } else {
                stream << "  [Parse Error: Invalid Data Format]\n";
            }
            break;
        }
        case CMD_FAULT_FEEDBACK: {
            if (auto data = parseFaultFeedbackPacket(cmdData)) {
                stream << "  Serial Number: " << data->serialNumber << "\n";
                stream << "  Fault State: 0x" << QString::number(data->faultState, 16).toUpper().rightJustified(8, '0');
                
                if (data->faultState == 0) {
                    stream << " (No Fault)\n";
                } else {
                    stream << "\n  Fault Details:\n";
                    // 这里可以根据实际的故障位定义添加具体说明
                    for (int bit = 0; bit < 32; ++bit) {
                        if (data->faultState & (1 << bit)) {
                            stream << "    - Bit " << bit << ": Fault Type " << bit << " (TBD)\n";
                        }
                    }
                }
            } else {
                stream << "  [Parse Error: Invalid Data Format]\n";
            }
            break;
        }
        case CMD_CALIBRATION_FEEDBACK: {
            if (auto data = parseCalibrationFeedbackPacket(cmdData)) {
                stream << "  Serial Number: " << data->serialNumber << "\n";
                stream << "  State: " << data->state << " (1: Success, 0: Fail)\n";
                stream << "  Command Echo: " << data->command.toHex(' ').toUpper() << "\n";
            } else {
                stream << "  [Parse Error: Invalid Data Format]\n";
            }
            break;
        }
        case CMD_REG_WRITE_FEEDBACK: {
             if (auto data = parseRegisterWriteFeedbackPacket(cmdData)) {
                stream << "  Serial Number: " << data->serialNumber << "\n";
                stream << "  State: " << data->state << " (1: Success, 0: Fail)\n";
                stream << "  DUT Select: 0x" << QString::number(data->dutSel, 16).toUpper() << "\n";
                stream << "  Register Addr: 0x" << QString::number(data->regAddr, 16).toUpper() << "\n";
                stream << "  Length: " << data->length << "\n";
                stream << "  Values Written Echo: " << data->values.toHex(' ').toUpper() << "\n";
            } else {
                stream << "  [Parse Error: Invalid Data Format]\n";
            }
            break;
        }
        case CMD_REG_READ_FEEDBACK: {
            if (auto data = parseRegisterReadFeedbackPacket(cmdData)) {
                stream << "  Serial Number: " << data->serialNumber << "\n";
                stream << "  State: " << data->state << " (1: Success, 0: Fail)\n";
                stream << "  DUT Select: 0x" << QString::number(data->dutSel, 16).toUpper() << "\n";
                stream << "  Register Addr: 0x" << QString::number(data->regAddr, 16).toUpper() << "\n";
                stream << "  Length: " << data->length << "\n";
                stream << "  Read Values: " << data->values.toHex(' ').toUpper() << "\n";
            } else {
                stream << "  [Parse Error: Invalid Data Format]\n";
            }
            break;
        }
        case CMD_SET_CHIP_TYPE_FEEDBACK: {
            if (auto data = parseSetChipTypeFeedbackPacket(cmdData)) {
                stream << "  Serial Number: " << data->serialNumber << "\n";
                stream << "  State: " << data->state << " (1: Success, 0: Fail)\n";
                stream << "  Chip Index: " << data->chipIndex << "\n";
            } else {
                stream << "  [Parse Error: Invalid Data Format]\n";
            }
            break;
        }
        default:
            stream << "  (Detailed parsing not implemented for this command)\n";
            break;
    }
    stream << "------------------------------------------\n";
    return details;
}

QString XTProtocol::formatSentPacketForDisplay(const QByteArray& rawPacket)
{
    if (rawPacket.size() < 9) { // Header (8) + CRC (1)
        return "Error: Sent packet too short\n";
    }

    uint16_t cmdId = Utils::ByteUtils::readLittleEndian<uint16_t>(reinterpret_cast<const uint8_t*>(rawPacket.constData()) + 4);
    QByteArray cmdData = rawPacket.mid(8, rawPacket.size() - 9); // Exclude header and CRC

    QString details;
    QTextStream stream(&details);
    stream.setIntegerBase(10); // 默认十进制
    stream << "--- [SENT] Packet (CmdID: 0x" << QString::number(cmdId, 16).toUpper().rightJustified(4, '0')
           << ", Size: " << rawPacket.size() << " bytes) ---\n";
    stream << "Raw: " << rawPacket.toHex(' ').toUpper() << "\n";
    stream << "Parsed Content:\n";

    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(cmdData.constData());

    switch (cmdId) {
        case CMD_START_STOP: {
            if (cmdData.size() == 7) {
                bool start = (*ptr == 0x01); ptr++;
                uint16_t dutActive = Utils::ByteUtils::readLittleEndian<uint16_t>(ptr); ptr += 2;
                uint32_t timestamp = Utils::ByteUtils::readLittleEndian<uint32_t>(ptr); ptr += 4;
                QDateTime dt = QDateTime::fromSecsSinceEpoch(timestamp);

                stream << "  Command: " << (start ? "Start Test" : "Stop Test") << "\n";
                stream << "  DUT Active Mask: 0b" << QString::number(dutActive, 2).rightJustified(16, '0') << "\n";
                stream << "  Timestamp: " << dt.toString("yyyy-MM-dd hh:mm:ss") << " (raw: " << timestamp << ")\n";
            } else {
                stream << "  [Parse Error: Invalid Data Length]\n";
            }
            break;
        }
        case CMD_POWER_CONTROL: {
            if (cmdData.size() == 3) {
                bool enable = (*ptr == 0x01); ptr++;
                uint16_t dutEnable = Utils::ByteUtils::readLittleEndian<uint16_t>(ptr); ptr += 2;
                stream << "  Command: " << (enable ? "Power ON" : "Power OFF") << "\n";
                stream << "  DUT Enable Mask: 0b" << QString::number(dutEnable, 2).rightJustified(16, '0') << "\n";
            } else {
                stream << "  [Parse Error: Invalid Data Length]\n";
            }
            break;
        }
        case CMD_VOLTAGE_QUERY: {
            stream << "  Command: Query Voltage Status\n";
            break;
        }
        case CMD_FAULT_QUERY: {
            stream << "  Command: Query Fault Status\n";
            break;
        }
        case CMD_CALIBRATION: {
            if (cmdData.size() >= 2) {
                uint8_t dutSel = *ptr++;
                uint8_t cmdSize = *ptr++;
                QByteArray command = cmdData.mid(2, cmdSize);
                stream << "  Command: Send Calibration\n";
                stream << "  DUT Select Mask: 0x" << QString::number(dutSel, 16).toUpper() << "\n";
                stream << "  Calib Command Size: " << cmdSize << "\n";
                stream << "  Calib Command: " << command.toHex(' ').toUpper() << "\n";
            } else {
                 stream << "  [Parse Error: Invalid Data Length]\n";
            }
            break;
        }
        case CMD_REG_WRITE: {
            if (cmdData.size() >= 3) {
                uint8_t dutSel = *ptr++;
                uint8_t regAddr = *ptr++;
                uint8_t dataSize = *ptr++;
                QByteArray data = cmdData.mid(3, dataSize);
                stream << "  Command: Write Register\n";
                stream << "  DUT Select Mask: 0x" << QString::number(dutSel, 16).toUpper() << "\n";
                stream << "  Register Address: 0x" << QString::number(regAddr, 16).toUpper() << "\n";
                stream << "  Data Size: " << dataSize << "\n";
                stream << "  Data: " << data.toHex(' ').toUpper() << "\n";
            } else {
                 stream << "  [Parse Error: Invalid Data Length]\n";
            }
            break;
        }
        case CMD_REG_READ: {
            if (cmdData.size() == 3) {
                uint8_t dutSel = *ptr++;
                uint8_t regAddr = *ptr++;
                uint8_t length = *ptr++;
                stream << "  Command: Read Register\n";
                stream << "  DUT Select Mask: 0x" << QString::number(dutSel, 16).toUpper() << "\n";
                stream << "  Register Address: 0x" << QString::number(regAddr, 16).toUpper() << "\n";
                stream << "  Length to Read: " << length << "\n";
            } else {
                 stream << "  [Parse Error: Invalid Data Length]\n";
            }
            break;
        }
        case CMD_SET_CHIP_TYPE: {
            if (cmdData.size() == 1) {
                uint8_t chipIndex = *ptr;
                stream << "  Command: Set Chip Type\n";
                stream << "  Chip Index: " << chipIndex << "\n";
            } else {
                stream << "  [Parse Error: Invalid Data Length]\n";
            }
            break;
        }
        default:
            stream << "  (Detailed parsing not implemented for this command)\n";
            break;
    }
    stream << "------------------------------------------\n";
    return details;
}

void XTProtocol::appendData(const QByteArray& data)
{
    if (m_framer) {
        m_framer->appendData(data);
    }
}

void XTProtocol::clearBuffer()
{
    if (m_framer) {
        m_framer->clearBuffer();
    }
}

} // namespace Protocols
} // namespace Domain 
