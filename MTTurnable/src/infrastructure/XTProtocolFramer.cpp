#include "infrastructure/XTProtocolFramer.h"
#include <QMutexLocker>
#include <QDebug>
#include <cstring>
#include <QDateTime>
#include <QThread>

namespace Infrastructure {

XTProtocolFramer::XTProtocolFramer(QObject *parent)
    : IProtocolFramer(parent)
{
    m_nextPacketPos = -1;
    m_nextPacketCmdId = 0;
    m_nextPacketSize = 0;
}

void XTProtocolFramer::appendData(const QByteArray& data)
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "[FRAMER" << QThread::currentThreadId() << "] appendData: Received" << data.size() << "bytes.";
    {
        QMutexLocker locker(&m_mutex);
        m_buffer.append(data);
    } // Mutex is unlocked here
    
    // Process buffer outside the lock to avoid holding it during signal emission
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
        return QByteArray();
    }
    
    // Extract packet data
    QByteArray packet = m_buffer.mid(m_nextPacketPos, m_nextPacketSize);
    
    // From buffer, remove the data that has been processed.
    m_buffer.remove(0, m_nextPacketPos + m_nextPacketSize);
    m_nextPacketPos = -1;
    
    // Validate CRC on the extracted packet
    if (!validatePacket(packet)) {
        m_stats.crcErrors++;
        emit frameError("CRC校验失败");
        return QByteArray();
    }
    
    // Extract command data (skip header and CRC)
    return packet.mid(HEADER_SIZE, packet.size() - HEADER_SIZE - CRC_SIZE);
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
    while (true) {
        uint16_t cmdId = 0;
        QByteArray cmdData;

        { // --- Locked Scope ---
            QMutexLocker locker(&m_mutex);
            if (!hasCompletePacket()) {
                break; // No more complete packets
            }
            cmdId = getNextPacketCmdId();
            cmdData = getNextPacket();
        } // --- Mutex is unlocked here ---

        // Process the packet outside the lock
        if (!cmdData.isEmpty() || cmdId != 0) {
            m_stats.packetsReceived++;
            qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "[FRAMER" << QThread::currentThreadId() << "] processBuffer: Emitting packetReady for cmdId" << Qt::hex << cmdId;
            emit packetReady(cmdId, cmdData);
            qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "[FRAMER" << QThread::currentThreadId() << "] processBuffer: emit packetReady finished.";
        }
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

IProtocolFramer::Statistics XTProtocolFramer::getStatistics() const
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
            return i;
        }
    }
    
    // 没找到有效包头，清理无用数据
    if (m_buffer.size() > static_cast<int>(HEADER_SIZE)) {
        m_buffer.remove(0, m_buffer.size() - HEADER_SIZE + 1);
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
    
    // 计算总包长度
    int totalSize = HEADER_SIZE + header.cmdDataSize + CRC_SIZE;
    
    // 检查是否有足够的数据
    if (startPos + totalSize > m_buffer.size()) {
        return false;
    }
    
    // 缓存包信息
    m_nextPacketPos = startPos;
    m_nextPacketCmdId = header.cmdId;
    m_nextPacketSize = totalSize;
    
    return true;
}

bool XTProtocolFramer::validatePacket(const QByteArray& packet) const
{
    if (packet.size() < static_cast<int>(MIN_PACKET_SIZE)) {
        return false;
    }
    
    // 计算CRC（不包括最后一个字节的CRC）
    uint8_t calculatedCrc = calculateCRC(packet.left(packet.size() - 1));
    uint8_t receivedCrc = static_cast<uint8_t>(packet.at(packet.size() - 1));
    
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

} // namespace Infrastructure 