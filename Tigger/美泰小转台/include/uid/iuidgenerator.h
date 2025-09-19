#ifndef IUIDGENERATOR_H
#define IUIDGENERATOR_H

#include <string>
#include <memory>

// ITimeProvider 的前向声明
class ITimeProvider;

class IUIDGenerator {
public:
    virtual ~IUIDGenerator() = default;

    // 生成并返回 UID。
    // 可返回空字符串或在失败时抛出异常、
    // 尽管对于这个特定的 MTUIDGenerator 来说，生成是相当确定的。
    virtual std::string getUID() = 0;

    // 报告使用/处理生成的 UID 的结果。
    // 成功 "参数表示涉及 UID 的操作是否成功。
    // 该函数的返回值可指示是否成功记录了结果。
    virtual bool setUIDResult(const std::string& uid, bool success) = 0;
};

#endif // IUIDGENERATOR_H
