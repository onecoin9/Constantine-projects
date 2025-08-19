#ifndef MTUIDGENERATOR_H
#define MTUIDGENERATOR_H
#include <atomic>
#include <QDebug>         // 用于在 setUIDResult 中记录日志 (For logging in setUIDResult)
#include <QFile>          // 用于文件操作
#include <QTextStream>    // 用于文本流操作
#include <QDate>          // 在辅助方法签名中使用
#include "iuidgenerator.h"
#include "itimeprovider.h"
class MTUIDGenerator : public IUIDGenerator {
public:
    // 构造函数接受一个时间提供者用于依赖注入
    // (Constructor takes a time provider for dependency injection)
    explicit MTUIDGenerator(std::shared_ptr<ITimeProvider> timeProvider);

    std::string getUID() override;
    bool setUIDResult(const std::string& uid, bool success) override;

private:
    std::shared_ptr<ITimeProvider> timeProvider_;
    uint16_t currentSerialNumber_; // "流水号" 0-16383 (Serial number 0-16383)
    const QString persistenceFilename_ = "uid_state.txt"; // 持久化文件名

    // 辅助函数，将年份转换为4位表示形式
    // (Helper to convert year to the 4-bit representation)
    uint8_t encodeYear(int year) const;

    // 持久化相关的辅助函数
    bool loadState(std::string& lastUID, QDate& lastDate);
    void saveState(const std::string& currentUID, const QDate& currentDate);
    uint16_t extractSerialFromUID(const std::string& uidStr) const;
};

#endif // MTUIDGENERATOR_H
