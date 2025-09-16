#ifndef BYTEUTILS_H
#define BYTEUTILS_H

#include <cstdint>
#include <QByteArray>

namespace Utils {

/**
 * @brief 提供静态的字节操作辅助函数
 */
class ByteUtils
{
public:
    /**
     * @brief 从小端字节数组中读取一个值
     * @tparam T 要读取的数据类型 (如 uint16_t, uint32_t)
     * @param data 指向字节数组的指针
     * @return 读取到的值
     */
    template<typename T>
    static T readLittleEndian(const uint8_t* data)
    {
        T value = 0;
        for (size_t i = 0; i < sizeof(T); ++i) {
            value |= static_cast<T>(data[i]) << (i * 8);
        }
        return value;
    }
};

} // namespace Utils

#endif // BYTEUTILS_H 