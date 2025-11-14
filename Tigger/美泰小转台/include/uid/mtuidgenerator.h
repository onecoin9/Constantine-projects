#ifndef MTUIDGENERATOR_H
#define MTUIDGENERATOR_H
#include <atomic>
#include <QDebug>         // 用于在 setUIDResult 中记录日志 (For logging in setUIDResult)
#include <QFile>          // 用于文件操作
#include <QTextStream>    // 用于文本流操作
#include <QDate>          // 在辅助方法签名中使用
#include <mutex>          // 用于线程安全
#include "iuidgenerator.h"
#include "itimeprovider.h"

// 前置声明清理函数，用于程序退出时释放资源
void cleanupMTUIDGenerator();

class MTUIDGenerator : public IUIDGenerator {
public:
    // 删除拷贝构造函数和赋值操作符以防止复制
    MTUIDGenerator(const MTUIDGenerator&) = delete;
    MTUIDGenerator& operator=(const MTUIDGenerator&) = delete;
    
    // 获取单例实例的静态方法
    static MTUIDGenerator& getInstance();
    
    // 实现接口方法
    std::string getUID() override;
    bool setUIDResult(const std::string& uid, bool success) override;

    // 声明清理函数为友元，使其可以访问私有成员
    friend void cleanupMTUIDGenerator();

private:
    // 私有构造函数，防止外部创建实例
    explicit MTUIDGenerator(std::shared_ptr<ITimeProvider> timeProvider);
    
    // 单例实例和互斥锁
    static std::mutex instanceMutex_;
    static MTUIDGenerator* instance_;

    static std::string sLastUID;
    static QDate sLastDate;
    
    std::shared_ptr<ITimeProvider> timeProvider_;
    std::mutex dataMutex_; // 保护数据访问的互斥锁
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
