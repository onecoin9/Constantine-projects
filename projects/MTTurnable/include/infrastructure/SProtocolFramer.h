#ifndef SPROTOCOLFRAMER_H
#define SPROTOCOLFRAMER_H

#include "domain/IProtocolFramer.h"
#include <QMutex>
#include <QDateTime>

namespace Infrastructure {

/**
 * @brief S协议帧处理器实现
 * 处理S协议的数据包组帧和拆帧
 */
class SProtocolFramer : public Domain::IProtocolFramer
{
    Q_OBJECT
public:
    explicit SProtocolFramer(QObject *parent = nullptr);
    ~SProtocolFramer() override = default;
    
    // IProtocolFramer接口实现
    void appendData(const QByteArray& data) override;
    bool hasCompletePacket() const override;
    QByteArray getNextPacket() override;
    uint16_t getNextPacketCmdId() const override;
    QByteArray buildPacket(uint16_t cmdId, const QByteArray& cmdData) override;
    void clearBuffer() override;
    Domain::IProtocolFramer::Statistics getStatistics() const override;
    
    // S协议特有的方法
    QByteArray buildSPacket(uint16_t pflag, uint8_t pdu, const QByteArray& data);
    bool parseSPacket(const QByteArray& packet, uint16_t& pflag, uint8_t& pdu, QByteArray& data);

private:
    void processBuffer();
    
    // S协议常量
    static constexpr size_t HEADER_SIZE = 4;          // PFLAG(2) + PDU(1) + LEN(1)
    static constexpr size_t CRC_SIZE = 1;
    static constexpr size_t MIN_PACKET_SIZE = HEADER_SIZE + CRC_SIZE;
    
    // 包头结构
    struct SPacketHeader {
        uint16_t pflag;
        uint8_t pdu;
        uint8_t dataLen;
    };
    
    // 内部方法
    int findPacketStart() const;
    bool checkCompletePacket(int startPos) const;
    bool validatePacket(const QByteArray& packet) const;
    uint8_t calculateCRC(const QByteArray& data) const;
    SPacketHeader parseHeader(const QByteArray& data) const;
    
    // 字节序转换
    template<typename T>
    void appendBigEndian(QByteArray& data, T value) const;
    
    template<typename T>
    T readBigEndian(const uint8_t* data) const;
    
    // 数据成员
    mutable QMutex m_mutex;
    mutable QByteArray m_buffer;
    mutable Domain::IProtocolFramer::Statistics m_stats;
    
    // 缓存的包信息
    mutable int m_nextPacketPos = -1;
    mutable uint16_t m_nextPacketPflag = 0;
    mutable uint8_t m_nextPacketPdu = 0;
    mutable int m_nextPacketSize = 0;
};

} // namespace Infrastructure

#endif // SPROTOCOLFRAMER_H 