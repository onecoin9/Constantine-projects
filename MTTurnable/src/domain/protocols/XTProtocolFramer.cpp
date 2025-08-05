#include "domain/protocols/XTProtocolFramer.h"
#include <QMutexLocker>
#include <QDateTime>
#include <QThread>
#include "utils/CRC16.h"
#include "core/Logger.h"

namespace Protocols {

// 定义日志节流的速率
static constexpr int LOG_THROTTLE_RATE = 500; // 每500个包记录一次日志

XTProtocolFramer::XTProtocolFramer(QObject *parent)
    : Domain::IProtocolFramer(parent)
    , m_logPacketCounter(0) // 初始化计数器
{
    m_nextPacketPos = -1;
    m_nextPacketCmdId = 0;
    m_nextPacketSize = 0;
}

void XTProtocolFramer::appendData(const QByteArray& data)
{
    // 移除调试日志以提升高频数据处理性能
    m_buffer.append(data);
    processBuffer();
}

bool XTProtocolFramer::hasCompletePacket() const
{
    // This function must be called within a locked section
    if (m_buffer.size() < static_cast<int>(MIN_PACKET_SIZE)) {
        return false;
    }

    // Find packet header
    int pos = findPacketStart();
    if (pos < 0) {
        return false;
    }

    // Check for a complete packet
    return checkCompletePacket(pos);
}

QByteArray XTProtocolFramer::getNextPacket()
{
    // This function must be called within a locked section
    if (m_nextPacketPos < 0 || m_nextPacketSize <= 0) {
        // LOG_MODULE_TRACE("XTProtocolFramer", "Invalid packet position or size");
        return QByteArray();
    }
    
    // Extract packet data
    QByteArray packet = m_buffer.mid(m_nextPacketPos, m_nextPacketSize);
    
    // LOG_MODULE_TRACE("XTProtocolFramer", QString("Extracted packet size: %1 cmdId: 0x%2")
    //     .arg(static_cast<int>(packet.size())).arg(QString::number(m_nextPacketCmdId, 16)).toStdString());
    
    // CRC验证
    bool enableCrcCheck = true;

    // Validate CRC on the extracted packet
    if (enableCrcCheck && !validatePacket(packet)) {
        m_stats.crcErrors++;
        LOG_ERROR("CRC validation failed!");
        emit frameError("CRC校验失败");

        // CRC失败时，跳过当前包头，寻找下一个有效包头
        // 删除从当前位置到下一个可能的包头位置
        int skipBytes = 1;

        // 在剩余数据中查找下一个包头
        for (int i = m_nextPacketPos + 1; i < m_buffer.size() - static_cast<int>(HEADER_SIZE); ++i) {
            uint32_t flag = readLittleEndian<uint32_t>(
                reinterpret_cast<const uint8_t*>(m_buffer.constData() + i));
            if (flag == CMD_FLAG) {
                skipBytes = i - m_nextPacketPos;
                LOG_TRACE(QString("Found next potential packet header at offset %1").arg(skipBytes).toStdString());
                break;
            }
        }

        // 如果没找到下一个包头，至少跳过当前包头
        if (skipBytes == 1) {
            skipBytes = HEADER_SIZE;
        }

        m_buffer.remove(0, skipBytes);
        m_nextPacketPos = -1;

        LOG_TRACE(QString("CRC failed, removed %1 bytes and will search for next valid packet").arg(skipBytes).toStdString());
        return QByteArray();
    }

    if (!enableCrcCheck) {
        LOG_WARNING("CRC check disabled for testing!");
    }

    // CRC验证通过，删除已处理的数据
    m_buffer.remove(0, m_nextPacketPos + m_nextPacketSize);
    m_nextPacketPos = -1;
    
    // Extract command data (skip header and CRC)
    QByteArray cmdData = packet.mid(HEADER_SIZE, packet.size() - HEADER_SIZE - CRC_SIZE);
    // LOG_MODULE_TRACE("XTProtocolFramer", QString("Returning cmdData size: %1").arg(static_cast<int>(cmdData.size())).toStdString());
    
    return cmdData;
}

uint16_t XTProtocolFramer::getNextPacketCmdId() const
{
    // This function must be called within a locked section.
    // It's assumed checkCompletePacket has already been called and set the member variables.
    return m_nextPacketCmdId;
}

QByteArray XTProtocolFramer::buildPacket(uint16_t cmdId, const QByteArray& cmdData)
{
    QByteArray packet;
    packet.reserve(HEADER_SIZE + cmdData.size() + CRC_SIZE);

    // 构建包头
    appendLittleEndian(packet, CMD_FLAG);
    appendLittleEndian(packet, cmdId);
    appendLittleEndian(packet, static_cast<uint16_t>(cmdData.size()));

    // 添加数据
    packet.append(cmdData);

    // 计算并添加CRC
    uint8_t crc = calculateCRC(packet);
    packet.append(static_cast<char>(crc));

    m_stats.packetsSent++;

    return packet;
}

void XTProtocolFramer::processBuffer()
{
    // 移除 while 循环，每次调用只处理一个包
    uint16_t cmdId = 0;
    QByteArray cmdData;
    QByteArray rawPacket;

    { // 锁定作用域
        QMutexLocker locker(&m_mutex);
        if (!hasCompletePacket()) {
            return; // 没有完整的包，直接返回
        }
        cmdId = getNextPacketCmdId();
        rawPacket = m_buffer.mid(m_nextPacketPos, m_nextPacketSize);
        cmdData = getNextPacket();
    } // 解锁

    // 在锁外发射信号
    if (!cmdData.isEmpty()) {
        m_stats.packetsReceived++;
        
        // 移除调试日志，特别是对高频数据
        emit rawPacketReady(rawPacket);
        emit packetReady(cmdId, cmdData);
    } else if (cmdId != 0) {
        LOG_WARNING(QString("cmdId 0x%1 but cmdData is empty, packet may be corrupted")
            .arg(QString::number(cmdId, 16)).toStdString());
    }

    // 检查是否还有数据，并调度下一次处理
    bool hasMoreData;
    {
        QMutexLocker locker(&m_mutex);
        hasMoreData = !m_buffer.isEmpty() && hasCompletePacket();
    }

    if (hasMoreData) {
        // 通过元对象系统调用自己，将任务放入事件队列，而不是阻塞地循环
        QMetaObject::invokeMethod(this, "processBuffer", Qt::QueuedConnection);
    }
}

void XTProtocolFramer::clearBuffer()
{
    QMutexLocker locker(&m_mutex);
    m_buffer.clear();
    m_nextPacketPos = -1;
    m_nextPacketCmdId = 0;
    m_nextPacketSize = 0;
}

Domain::IProtocolFramer::Statistics XTProtocolFramer::getStatistics() const
{
    QMutexLocker locker(&m_mutex);
    return m_stats;
}

int XTProtocolFramer::findPacketStart() const
{
    // 查找CMD_FLAG
    for (int i = 0; i <= m_buffer.size() - static_cast<int>(MIN_PACKET_SIZE); ++i) {
        uint32_t flag = readLittleEndian<uint32_t>(
            reinterpret_cast<const uint8_t*>(m_buffer.constData() + i));

        if (flag == CMD_FLAG) {
            // 找到可能的包头，进一步验证
            if (i + static_cast<int>(HEADER_SIZE) <= m_buffer.size()) {
                // 解析包头
                PacketHeader header = parseHeader(m_buffer.mid(i, HEADER_SIZE));

                // 验证cmdId是否在有效范围内
                if ((header.cmdId >= 0x01 && header.cmdId <= 0x08) ||
                    (header.cmdId >= 0x8001 && header.cmdId <= 0x8008)) {
                    // 验证数据长度是否合理
                    if (header.cmdDataSize <= 1024) { // 假设最大数据长度为1024
                        return i;
                    } else {
                        LOG_TRACE(QString("Invalid cmdDataSize: %1 at position %2, skipping...")
                            .arg(header.cmdDataSize).arg(i).toStdString());
                    }
                } else {
                    LOG_TRACE(QString("Invalid cmdId: 0x%1 at position %2, skipping...")
                        .arg(QString::number(header.cmdId, 16)).arg(i).toStdString());
                }
            }
        }
    }

    // 没找到有效包头，只清理确定无用的数据
    // 保留最后的HEADER_SIZE-1字节，因为包头可能跨越当前缓冲区边界
    if (m_buffer.size() > static_cast<int>(HEADER_SIZE * 2)) {
        int removeBytes = m_buffer.size() - HEADER_SIZE + 1;
        LOG_TRACE(QString("No valid packet found, removing %1 bytes from buffer").arg(removeBytes).toStdString());
        m_buffer.remove(0, removeBytes);
    }

    return -1;
}

bool XTProtocolFramer::checkCompletePacket(int startPos) const
{
    if (startPos < 0 || startPos + static_cast<int>(HEADER_SIZE) > m_buffer.size()) {
        return false;
    }
    
    // 解析包头
    PacketHeader header = parseHeader(m_buffer.mid(startPos, HEADER_SIZE));
    
    // LOG_MODULE_TRACE("XTProtocolFramer", QString("Header - cmdId: 0x%1 cmdDataSize: %2")
    //     .arg(QString::number(header.cmdId, 16)).arg(static_cast<int>(header.cmdDataSize)).toStdString());
    
    // 计算总包长度
    int totalSize = HEADER_SIZE + header.cmdDataSize + CRC_SIZE;
    
    // 检查是否有足够的数据
    if (startPos + totalSize > m_buffer.size()) {
        // LOG_MODULE_TRACE("XTProtocolFramer", QString("Not enough data. Need: %1 Have: %2")
        //     .arg(static_cast<int>(totalSize)).arg(static_cast<int>(m_buffer.size() - startPos)).toStdString());
        return false;
    }
    
    // QByteArray fullPacket = m_buffer.mid(startPos, totalSize);
    // LOG_MODULE_TRACE("XTProtocolFramer", QString("Packet end (last 20 bytes): %1")
    //     .arg(QString(fullPacket.right(20).toHex(' '))).toStdString());
    // LOG_MODULE_TRACE("XTProtocolFramer", QString("Expected CRC position: %1 CRC byte: 0x%2")
    //     .arg(static_cast<int>(totalSize - 1)).arg(QString::number(static_cast<uint8_t>(fullPacket.at(totalSize - 1)), 16)).toStdString());
    
    // 缓存包信息
    m_nextPacketPos = startPos;
    m_nextPacketCmdId = header.cmdId;
    m_nextPacketSize = totalSize;
    
    return true;
}

bool XTProtocolFramer::validatePacket(const QByteArray& packet) const
{
    if (packet.size() < HEADER_SIZE + CRC_SIZE) {
        return false;
    }

    // 计算CRC（不包括最后一个字节的CRC）
    uint8_t calculatedCrc = calculateCRC(packet.left(packet.size() - 1));
    uint8_t receivedCrc = static_cast<uint8_t>(packet.at(packet.size() - 1));

    // 添加调试信息
    if (calculatedCrc != receivedCrc) {
        LOG_ERROR(QString("CRC mismatch! Calculated: 0x%1 Received: 0x%2 Packet size: %3 First 16 bytes: %4")
            .arg(QString::number(calculatedCrc, 16))
            .arg(QString::number(receivedCrc, 16))
            .arg(packet.size())
            .arg(QString(packet.left(16).toHex(' '))).toStdString());

        // 手动计算CRC以验证
        uint8_t manualCrc = 0;
        for (int i = 0; i < packet.size() - 1; ++i) {
            manualCrc += static_cast<uint8_t>(packet.at(i));
        }
        LOG_TRACE(QString("Manual CRC calculation: 0x%1").arg(QString::number(manualCrc, 16)).toStdString());

    }

    return calculatedCrc == receivedCrc;
}

uint8_t XTProtocolFramer::calculateCRC(const QByteArray& data) const
{
    uint8_t crc = 0;
    for (int i = 0; i < data.size(); ++i) {
        crc += static_cast<uint8_t>(data.at(i));
    }
    return crc;
}

XTProtocolFramer::PacketHeader XTProtocolFramer::parseHeader(const QByteArray& data) const
{
    PacketHeader header;
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data.constData());

    header.cmdFlag = readLittleEndian<uint32_t>(ptr);
    header.cmdId = readLittleEndian<uint16_t>(ptr + 4);
    header.cmdDataSize = readLittleEndian<uint16_t>(ptr + 6);

    return header;
}

template<typename T>
void XTProtocolFramer::appendLittleEndian(QByteArray& data, T value) const
{
    for (size_t i = 0; i < sizeof(T); ++i) {
        data.append(static_cast<char>(value & 0xFF));
        value >>= 8;
    }
}

template<typename T>
T XTProtocolFramer::readLittleEndian(const uint8_t* data) const
{
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        value |= static_cast<T>(data[i]) << (i * 8);
    }
    return value;
}

// 显式实例化模板
template void XTProtocolFramer::appendLittleEndian<uint16_t>(QByteArray&, uint16_t) const;
template void XTProtocolFramer::appendLittleEndian<uint32_t>(QByteArray&, uint32_t) const;
template uint16_t XTProtocolFramer::readLittleEndian<uint16_t>(const uint8_t*) const;
template uint32_t XTProtocolFramer::readLittleEndian<uint32_t>(const uint8_t*) const;

} // namespace Protocols
