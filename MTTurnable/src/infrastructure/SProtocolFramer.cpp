#include "infrastructure/SProtocolFramer.h"
#include "core/Logger.h"
#include <QDataStream>
#include <QDebug>

namespace Infrastructure {

SProtocolFramer::SProtocolFramer(QObject *parent)
    : IProtocolFramer(parent)
    , m_nextPacketPos(-1)
    , m_nextPacketPflag(0)
    , m_nextPacketPdu(0)
    , m_nextPacketSize(0)
{
    // 初始化统计信息
    m_stats.packetsReceived = 0;
    m_stats.packetsSent = 0;
    m_stats.crcErrors = 0;
    m_stats.frameErrors = 0;
}

void SProtocolFramer::appendData(const QByteArray& data)
{
    QMutexLocker locker(&m_mutex);
    
    if (data.isEmpty()) {
        return;
    }
    
    m_buffer.append(data);
    processBuffer();
}

bool SProtocolFramer::hasCompletePacket() const
{
    QMutexLocker locker(&m_mutex);
    return m_nextPacketPos >= 0;
}

QByteArray SProtocolFramer::getNextPacket()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_nextPacketPos < 0 || m_nextPacketSize <= 0) {
        return QByteArray();
    }
    
    // 提取完整的包（不包含头部和CRC）
    QByteArray packet = m_buffer.mid(m_nextPacketPos + HEADER_SIZE, 
                                    m_nextPacketSize - HEADER_SIZE - CRC_SIZE);
    
    // 移除已处理的数据
    m_buffer.remove(0, m_nextPacketPos + m_nextPacketSize);
    
    // 重置状态
    m_nextPacketPos = -1;
    m_nextPacketSize = 0;
    
    // 更新统计
    m_stats.packetsReceived++;
    
    // 重新处理缓冲区
    processBuffer();
    
    emit packetReady(getNextPacketCmdId(), packet);
    
    return packet;
}

uint16_t SProtocolFramer::getNextPacketCmdId() const
{
    QMutexLocker locker(&m_mutex);
    
    // S协议使用 PFLAG + PDU 组合作为命令ID
    return (static_cast<uint16_t>(m_nextPacketPflag) << 8) | m_nextPacketPdu;
}

QByteArray SProtocolFramer::buildPacket(uint16_t cmdId, const QByteArray& cmdData)
{
    // S协议中，cmdId 的高8位是PFLAG，低8位是PDU
    uint16_t pflag = (cmdId >> 8) & 0xFFFF;
    uint8_t pdu = cmdId & 0xFF;
    
    return buildSPacket(pflag, pdu, cmdData);
}

QByteArray SProtocolFramer::buildSPacket(uint16_t pflag, uint8_t pdu, const QByteArray& data)
{
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    
    // 构建包头：PFLAG(2) + PDU(1) + LEN(1)
    stream << pflag;
    stream << pdu;
    stream << static_cast<uint8_t>(data.size());
    
    // 添加数据
    packet.append(data);
    
    // 计算并添加CRC
    uint8_t crc = calculateCRC(packet);
    packet.append(static_cast<char>(crc));
    
    // 更新统计
    m_stats.packetsSent++;
    
    return packet;
}

void SProtocolFramer::clearBuffer()
{
    QMutexLocker locker(&m_mutex);
    m_buffer.clear();
    m_nextPacketPos = -1;
    m_nextPacketSize = 0;
}

Domain::IProtocolFramer::Statistics SProtocolFramer::getStatistics() const
{
    QMutexLocker locker(&m_mutex);
    return m_stats;
}

void SProtocolFramer::processBuffer()
{
    // 如果已经有一个完整的包，不需要重新处理
    if (m_nextPacketPos >= 0) {
        return;
    }
    
    while (m_buffer.size() >= MIN_PACKET_SIZE) {
        int startPos = findPacketStart();
        if (startPos < 0) {
            // 没找到有效的包开始，清除所有数据
            m_buffer.clear();
            m_stats.frameErrors++;
            break;
        }
        
        // 移除无效数据
        if (startPos > 0) {
            m_buffer.remove(0, startPos);
        }
        
        // 检查是否有完整的包
        if (checkCompletePacket(0)) {
            break;  // 找到完整包，退出循环
        } else {
            // 数据不够，等待更多数据
            break;
        }
    }
}

int SProtocolFramer::findPacketStart() const
{
    // S协议通过PFLAG来识别包开始
    // PFLAG应该是 0x5341 (SA) 或 0x4153 (AS)
    
    for (int i = 0; i <= m_buffer.size() - HEADER_SIZE; ++i) {
        uint16_t pflag = readBigEndian<uint16_t>(reinterpret_cast<const uint8_t*>(m_buffer.data() + i));
        
        // 检查是否是有效的PFLAG
        if (pflag == 0x5341 || pflag == 0x4153) {
            // 读取PDU和长度
            if (i + HEADER_SIZE <= m_buffer.size()) {
                uint8_t pdu = static_cast<uint8_t>(m_buffer[i + 2]);
                uint8_t dataLen = static_cast<uint8_t>(m_buffer[i + 3]);
                
                // 检查PDU是否有效
                if ((pdu >= 0x61 && pdu <= 0x68) || (pdu >= 0xE1 && pdu <= 0xE8)) {
                    // 检查包长度是否合理
                    int totalLen = HEADER_SIZE + dataLen + CRC_SIZE;
                    if (totalLen >= MIN_PACKET_SIZE && totalLen <= 1024) {
                        return i;
                    }
                }
            }
        }
    }
    
    return -1;
}

bool SProtocolFramer::checkCompletePacket(int startPos) const
{
    if (startPos + HEADER_SIZE > m_buffer.size()) {
        return false;
    }
    
    // 读取包头信息
    uint8_t dataLen = static_cast<uint8_t>(m_buffer[startPos + 3]);
    int totalLen = HEADER_SIZE + dataLen + CRC_SIZE;
    
    if (startPos + totalLen > m_buffer.size()) {
        return false;  // 数据不够
    }
    
    // 验证CRC
    QByteArray packetData = m_buffer.mid(startPos, totalLen);
    if (!validatePacket(packetData)) {
        return false;
    }
    
    // 保存包信息
    m_nextPacketPos = startPos;
    m_nextPacketSize = totalLen;
    
    // 解析包头
    SPacketHeader header = parseHeader(packetData);
    m_nextPacketPflag = header.pflag;
    m_nextPacketPdu = header.pdu;
    
    return true;
}

bool SProtocolFramer::validatePacket(const QByteArray& packet) const
{
    if (packet.size() < MIN_PACKET_SIZE) {
        return false;
    }
    
    // 验证CRC
    uint8_t expectedCrc = calculateCRC(packet.left(packet.size() - 1));
    uint8_t actualCrc = static_cast<uint8_t>(packet[packet.size() - 1]);
    
    return expectedCrc == actualCrc;
}

uint8_t SProtocolFramer::calculateCRC(const QByteArray& data) const
{
    uint8_t crc = 0;
    for (int i = 0; i < data.size(); ++i) {
        crc += static_cast<uint8_t>(data[i]);
    }
    return crc;
}

SProtocolFramer::SPacketHeader SProtocolFramer::parseHeader(const QByteArray& data) const
{
    SPacketHeader header;
    
    if (data.size() >= HEADER_SIZE) {
        header.pflag = readBigEndian<uint16_t>(reinterpret_cast<const uint8_t*>(data.data()));
        header.pdu = static_cast<uint8_t>(data[2]);
        header.dataLen = static_cast<uint8_t>(data[3]);
    }
    
    return header;
}

template<typename T>
T SProtocolFramer::readBigEndian(const uint8_t* data) const
{
    T value = 0;
    
    if constexpr (sizeof(T) == 1) {
        value = static_cast<T>(data[0]);
    } else if constexpr (sizeof(T) == 2) {
        value = static_cast<T>((data[0] << 8) | data[1]);
    } else if constexpr (sizeof(T) == 4) {
        value = static_cast<T>((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]);
    }
    
    return value;
}

} // namespace Infrastructure 