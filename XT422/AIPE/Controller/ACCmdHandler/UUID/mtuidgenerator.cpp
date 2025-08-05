#include "mtuidgenerator.h"
#include <QString>
#include <QDate>
#include <stdexcept>
#include <QFile>
#include <QTextStream>
#include <QDir> // 包含 QDir 以便将来可能使用 AppDataLocation

MTUIDGenerator::MTUIDGenerator(std::shared_ptr<ITimeProvider> timeProvider)
    : timeProvider_(timeProvider), currentSerialNumber_(0) { // 初始化时间提供者和当前序列号
    if (!timeProvider_) {
        // 回退或抛出，取决于设计选择。
        // 为了稳健起见，如果是 nullptr，可以默认为 RealTimeProvider。
        // 在这里，我们假设总是给出一个有效的提供程序。
        qWarning("时间提供者为空，使用 RealTimeProvider 作为备选"); // 时间提供者为空，使用 RealTimeProvider 作为备选
        timeProvider_ = std::make_shared<RealTimeProvider>();
    }
}

uint8_t MTUIDGenerator::encodeYear(int year) const {
    // 年: 0-9, 从24年开始计算,
    // (Year: 0-9, calculated starting from year '24)
    // 4: 2024年
    // 5: 2025年
    // ...
    // 9: 2029年
    // 0: 2030年
    // 1: 2031年
    // ...
    // 这意味着 (年份 - 2020) % 10 的逻辑
    if (year < 2024) {
        // (或者作为错误处理，或者限制为一个默认值。)
        // (为简单起见，我们将2024年之前的年份映射为 '4' (2024年的代码)。)
        // (这个行为应该由需求明确定义。)
        qWarning("年份 %d 早于 2024，使用 2024 年的代码。", year); // 年份 %d 早于 2024，使用 2024 年的代码。
        return 4; // 返回2024年的代码
    }
    return static_cast<uint8_t>((year - 2020) % 10); // 计算并返回年份代码
}

uint16_t MTUIDGenerator::extractSerialFromUID(const std::string& uidStr) const {
    bool ok;
    uint32_t numericUID = QString::fromStdString(uidStr).toUInt(&ok, 16);
    if (!ok) {
        qWarning() << "无法从UID字符串解析序列号:" << QString::fromStdString(uidStr);
        return 0; // 或抛出异常
    }
    return static_cast<uint16_t>(numericUID & 0x3FFF); // 低14位是序列号
}

bool MTUIDGenerator::loadState(std::string& lastUID, QDate& lastDate) {
    QFile file(persistenceFilename_);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream in(&file);
    if (in.atEnd()) { file.close(); return false; }
    QString dateStr = in.readLine();
    if (in.atEnd()) { file.close(); return false; }
    QString uidStr = in.readLine();
    file.close();

    lastDate = QDate::fromString(dateStr, "yyyy-MM-dd");
    if (!lastDate.isValid()) {
        qWarning() << "持久化文件中日期格式无效:" << dateStr;
        return false;
    }
    lastUID = uidStr.toStdString();
    if (lastUID.empty() || lastUID.length() != 8) { // 基本的UID格式检查
        qWarning() << "持久化文件中UID格式无效:" << uidStr;
        return false;
    }
    return true;
}

void MTUIDGenerator::saveState(const std::string& currentUID, const QDate& currentDate) {
    QFile file(persistenceFilename_);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "无法打开持久化文件进行写入:" << persistenceFilename_ << file.errorString();
        return;
    }
    QTextStream out(&file);
    out << currentDate.toString("yyyy-MM-dd") << "\n";
    out << QString::fromStdString(currentUID) << "\n";
    file.close();
}

std::string MTUIDGenerator::getUID() {
    QDate currentDate = timeProvider_->getCurrentDate(); // 获取当前日期
    uint16_t serialToUse = 0; // 默认为0（新的一天或无持久化状态）

    std::string lastUIDStr;
    QDate lastUIDDate;

    if (loadState(lastUIDStr, lastUIDDate)) {
        if (lastUIDDate.isValid() && lastUIDDate == currentDate) {
            uint16_t lastSerial = extractSerialFromUID(lastUIDStr);
            serialToUse = lastSerial + 1;
            if (serialToUse > 16383) { // 序列号回绕
                serialToUse = 0;
            }
        }
        // 如果日期不同，serialToUse 保持为 0 (新的一天，序列号从0开始)
    }
    // 如果 loadState 失败, serialToUse 保持为 0 (视为新序列)

    currentSerialNumber_ = serialToUse; // 设置当前UID将使用的序列号

    uint32_t uid = 0; // 初始化UID
    // Bit 31: 默认, 不允许修改 (Default 1, not modifiable)
    uint32_t bit31_val = 1; // 第31位：固定为1
    // Bit 30: 默认写0. 分选使用位, 0: NG, 1:OK.
    // 本次初始化配置文件时只写0, 后期单分选功能时根据此位状态分选芯片
    uint32_t bit30_val = 0; // 第30位：默认为0
    // Bits 29-27: 设备类型 (Device Type), default 000
    uint32_t deviceType_val = 0; // 假设默认为000，可以配置 (Assuming default 000, can be made configurable)
    // Bits 26-23: 年 (Year)
    uint32_t year_val = encodeYear(currentDate.year()); // 编码年份
    // Bits 22-19: 月 (Month), 1-12
    uint32_t month_val = static_cast<uint32_t>(currentDate.month()); // 获取月份
    // Bits 18-14: 日 (Day), 1-31
    uint32_t day_val = static_cast<uint32_t>(currentDate.day()); // 获取日期
    // Bits 13-0: 流水号 (Serial Number), 0-16383
    uint32_t serial_val = currentSerialNumber_; // 使用已确定的序列号

    // 组装UID
    uid |= (bit31_val & 0x1) << 31;      // 设置第31位
    uid |= (bit30_val & 0x1) << 30;      // 设置第30位
    uid |= (deviceType_val & 0x7) << 27; // 设置设备类型 (3位，0-7)
    uid |= (year_val & 0xF) << 23;       // 设置年份 (4位，0-9，实际范围0-15)
    uid |= (month_val & 0xF) << 19;      // 设置月份 (4位，1-12，实际范围0-15)
    uid |= (day_val & 0x1F) << 14;       // 设置日期 (5位，1-31，实际范围0-31)
    uid |= (serial_val & 0x3FFF) << 0;   // 设置序列号 (14位，0-16383)

    // (转换为十六进制字符串，8个字符，不足补零，大写)
    QString hexUid = QString("%1").arg(uid, 8, 16, QChar('0')).toUpper();
    std::string newUIDstdString = hexUid.toStdString();

    saveState(newUIDstdString, currentDate); // 持久化新生成的UID和日期

    return newUIDstdString; // 返回标准字符串格式的UID
}

bool MTUIDGenerator::setUIDResult(const std::string& uid, bool success) {
    // 该函数用于报告使用 UID 的结果。
    // 在这个例子中，我们只记录它。
    // 在实际系统中，这可能会更新数据库、记录到持久化存储中、
    // 甚至会影响未来 UID 的生成（例如，如果 UID 失败，可能会使用不同的序列重试。）
// ...existing code...

// 返回 true 表示该函数已处理/记录结果。
    return true;
}
