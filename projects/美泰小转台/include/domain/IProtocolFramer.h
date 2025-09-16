#ifndef IPROTOCOLFRAMER_H
#define IPROTOCOLFRAMER_H

#include <QObject>
#include <QByteArray>

namespace Domain {

/**
 * @brief 协议帧处理器接口
 * 负责数据包的组帧和拆帧，处理包边界、CRC校验等
 */
class IProtocolFramer : public QObject
{
    Q_OBJECT
public:
    explicit IProtocolFramer(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IProtocolFramer() = default;
    
    /**
     * @brief 添加接收到的原始数据
     * @param data 原始数据
     */
    virtual void appendData(const QByteArray& data) = 0;
    
    /**
     * @brief 检查是否有完整的数据包
     * @return true 如果有完整包
     */
    virtual bool hasCompletePacket() const = 0;
    
    /**
     * @brief 获取下一个完整的数据包
     * @return 数据包内容（不含包头和CRC）
     */
    virtual QByteArray getNextPacket() = 0;
    
    /**
     * @brief 获取下一个数据包的命令ID
     * @return 命令ID
     */
    virtual uint16_t getNextPacketCmdId() const = 0;
    
    /**
     * @brief 构建完整的数据包
     * @param cmdId 命令ID
     * @param cmdData 命令数据
     * @return 完整的数据包（含包头和CRC）
     */
    virtual QByteArray buildPacket(uint16_t cmdId, const QByteArray& cmdData) = 0;
    
    /**
     * @brief 清空接收缓冲区
     */
    virtual void clearBuffer() = 0;
    
    /**
     * @brief 获取统计信息
     */
    struct Statistics {
        uint32_t packetsReceived = 0;
        uint32_t packetsSent = 0;
        uint32_t crcErrors = 0;
        uint32_t frameErrors = 0;
    };
    virtual Statistics getStatistics() const = 0;

signals:
    /**
     * @brief 当有完整的数据包准备好时发出
     * @param cmdId 命令ID
     * @param cmdData 命令数据（不含包头和CRC）
     */
    void packetReady(uint16_t cmdId, const QByteArray& cmdData);
    void rawPacketReady(const QByteArray& rawPacket); // 新增：广播原始数据包的信号
    
    /**
     * @brief 帧错误信号
     * @param error 错误描述
     */
    void frameError(const QString& error);
};

} // namespace Domain

#endif // IPROTOCOLFRAMER_H 