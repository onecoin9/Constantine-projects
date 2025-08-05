#ifndef XTPROTOCOLFRAMER_H
#define XTPROTOCOLFRAMER_H

#include "domain/IProtocolFramer.h"
#include <QByteArray>
#include <QMutex>
#include <QDateTime>

namespace Protocols {

/**
 * @brief XT协议帧处理器实现
 * 处理XT小转台协议的数据包组帧和拆帧
 */
class XTProtocolFramer : public Domain::IProtocolFramer
{
    Q_OBJECT
public:
    explicit XTProtocolFramer(QObject *parent = nullptr);
    ~XTProtocolFramer() override = default;

    // IProtocolFramer接口实现
    void appendData(const QByteArray& data) override;
    bool hasCompletePacket() const override;
    QByteArray getNextPacket() override;
    uint16_t getNextPacketCmdId() const override;
    QByteArray buildPacket(uint16_t cmdId, const QByteArray& cmdData) override;
    void clearBuffer() override;
    Statistics getStatistics() const override;

private slots:
    void processBuffer();

private:
    // 协议常量
    static constexpr uint32_t CMD_FLAG = 0x58544B5A;  // "XTKZ"
    static constexpr size_t HEADER_SIZE = 8;          // 4+2+2
    static constexpr size_t CRC_SIZE = 1;
    static constexpr size_t MIN_PACKET_SIZE = HEADER_SIZE + CRC_SIZE;

    // 包头结构
    struct PacketHeader {
        uint32_t cmdFlag;
        uint16_t cmdId;
        uint16_t cmdDataSize;
    };

    // 内部方法
    int findPacketStart() const;
    bool checkCompletePacket(int startPos) const;
    bool validatePacket(const QByteArray& packet) const;
    uint8_t calculateCRC(const QByteArray& data) const;
    PacketHeader parseHeader(const QByteArray& data) const;

    // 字节序转换
    template<typename T>
    void appendLittleEndian(QByteArray& data, T value) const;

    template<typename T>
    T readLittleEndian(const uint8_t* data) const;

    // 数据成员
    mutable QMutex m_mutex;
    mutable QByteArray m_buffer;
    mutable Statistics m_stats;
    qint64 m_logPacketCounter; // 新增：用于日志节流的计数器

    // 缓存的包信息
    mutable int m_nextPacketPos = -1;
    mutable uint16_t m_nextPacketCmdId = 0;
    mutable int m_nextPacketSize = 0;
};

} // namespace Protocols

#endif // XTPROTOCOLFRAMER_H
