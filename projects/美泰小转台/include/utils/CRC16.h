#ifndef CRC16_H
#define CRC16_H

#include <QByteArray>

namespace Utils {

/**
 * @brief CRC16-CCITT (XMODEM) 计算类
 */
class CRC16 {
public:
    /**
     * @brief 计算CRC16-CCITT (XMODEM)校验值
     * @param data 数据
     * @return CRC16校验值
     */
    static quint16 calculate(const QByteArray &data);
    
    /**
     * @brief 计算CRC16-CCITT (XMODEM)校验值
     * @param data 数据指针
     * @param length 数据长度
     * @return CRC16校验值
     */
    static quint16 calculate(const quint8 *data, int length);

private:
    static const quint16 CRC16_XMODEM_TABLE[256];
};

} // namespace Utils

#endif // CRC16_H 