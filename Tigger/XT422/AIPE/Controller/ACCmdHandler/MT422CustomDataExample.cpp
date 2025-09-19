// MT422CustomDataExample.cpp
// 这是一个示例文件，展示如何使用 MT422SerialHandler 的 assembleCustomData 辅助函数

#include "MT422SerialHandler.h"
#include <QDebug>
#include <QByteArray>

void demonstrateCustomDataAssembly()
{
    // 获取单例实例
    MT422SerialHandler& handler = MT422SerialHandler::instance();
    
    // 示例1：组装简单的自定义数据包
    uint8_t cmdId = 0x05;
    QByteArray payload;
    payload.append(0x01);  // 示例数据
    payload.append(0x02);
    payload.append(0x03);
    payload.append(0x04);
    
    uint16_t dataLength = static_cast<uint16_t>(payload.size());
    
    // 注意：assembleCustomData 是私有函数，这里只是展示数据格式
    // 实际使用时通过 slotTaskDownLoadStatus2 函数调用
    
    qDebug() << "Custom data format:";
    qDebug() << "Command ID (1 byte):" << QString("0x%1").arg(cmdId, 2, 16, QChar('0')).toUpper();
    qDebug() << "Data Length (2 bytes, little-endian):" 
             << QString("0x%1 0x%2").arg(dataLength & 0xFF, 2, 16, QChar('0'))
                                    .arg((dataLength >> 8) & 0xFF, 2, 16, QChar('0')).toUpper();
    qDebug() << "Payload Data:" << payload.toHex().toUpper();
}

void demonstrateBaudrateDataAssembly()
{
    qDebug() << "\n=== Baudrate Data Assembly Example ===";
    
    // 模拟 jsonConfigBaudrate = 921600 的情况
    uint32_t baudrate = 921600;
    
    // 按小端格式组装波特率数据
    QByteArray baudrateData;
    baudrateData.append(static_cast<char>(baudrate & 0xFF));         // 低字节: 0x00
    baudrateData.append(static_cast<char>((baudrate >> 8) & 0xFF));  // 次低字节: 0x10
    baudrateData.append(static_cast<char>((baudrate >> 16) & 0xFF)); // 次高字节: 0x0E
    baudrateData.append(static_cast<char>((baudrate >> 24) & 0xFF)); // 高字节: 0x00
    
    qDebug() << "Original baudrate:" << baudrate;
    qDebug() << "Little-endian bytes:" << baudrateData.toHex().toUpper();
    
    // 验证：从小端字节重新组装回原值
    uint32_t reconstructed = static_cast<uint8_t>(baudrateData[0]) |
                           (static_cast<uint8_t>(baudrateData[1]) << 8) |
                           (static_cast<uint8_t>(baudrateData[2]) << 16) |
                           (static_cast<uint8_t>(baudrateData[3]) << 24);
    
    qDebug() << "Reconstructed baudrate:" << reconstructed;
    qDebug() << "Match:" << (reconstructed == baudrate ? "YES" : "NO");
}

void demonstrateCompletePacketFormat()
{
    qDebug() << "\n=== Complete Packet Format Example ===";
    
    uint8_t cmdId = 0x05;
    uint32_t baudrate = 921600;
    
    // 组装载荷数据（波特率的小端表示）
    QByteArray payloadData;
    payloadData.append(static_cast<char>(baudrate & 0xFF));
    payloadData.append(static_cast<char>((baudrate >> 8) & 0xFF));
    payloadData.append(static_cast<char>((baudrate >> 16) & 0xFF));
    payloadData.append(static_cast<char>((baudrate >> 24) & 0xFF));
    
    uint16_t dataLength = static_cast<uint16_t>(payloadData.size());
    
    // 手动组装完整数据包（模拟 assembleCustomData 函数的行为）
    QByteArray completePacket;
    
    // 1. 命令ID (1字节)
    completePacket.append(static_cast<char>(cmdId));
    
    // 2. 数据长度 (2字节，小端格式)
    completePacket.append(static_cast<char>(dataLength & 0xFF));
    completePacket.append(static_cast<char>((dataLength >> 8) & 0xFF));
    
    // 3. 载荷数据
    completePacket.append(payloadData);
    
    qDebug() << "Complete packet structure:";
    qDebug() << "  Command ID:" << QString("0x%1").arg(static_cast<uint8_t>(completePacket[0]), 2, 16, QChar('0')).toUpper();
    qDebug() << "  Data Length:" << QString("0x%1%2").arg(static_cast<uint8_t>(completePacket[2]), 2, 16, QChar('0'))
                                                      .arg(static_cast<uint8_t>(completePacket[1]), 2, 16, QChar('0')).toUpper()
             << "(" << dataLength << "bytes)";
    qDebug() << "  Payload:" << payloadData.toHex().toUpper();
    qDebug() << "  Complete packet:" << completePacket.toHex().toUpper();
    qDebug() << "  Total size:" << completePacket.size() << "bytes";
}

// 主函数示例
int main()
{
    demonstrateCustomDataAssembly();
    demonstrateBaudrateDataAssembly();
    demonstrateCompletePacketFormat();
    
    return 0;
}

/*
预期输出示例：

Custom data format:
Command ID (1 byte): 0x05
Data Length (2 bytes, little-endian): 0x04 0x00
Payload Data: 01020304

=== Baudrate Data Assembly Example ===
Original baudrate: 921600
Little-endian bytes: 00100E00
Reconstructed baudrate: 921600
Match: YES

=== Complete Packet Format Example ===
Complete packet structure:
  Command ID: 0x05
  Data Length: 0x0004 (4 bytes)
  Payload: 00100E00
  Complete packet: 050400100E00
  Total size: 7 bytes

数据包格式说明：
- 字节0: 命令ID (0x05)
- 字节1-2: 数据长度，小端格式 (0x04 0x00 = 4字节)
- 字节3-6: 载荷数据，波特率921600的小端表示 (0x00 0x10 0x0E 0x00)
*/ 