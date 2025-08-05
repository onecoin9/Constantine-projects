#ifndef ICUSTOMDLLHANDLER_H
#define ICUSTOMDLLHANDLER_H

#include <QByteArray>
#include <QString>

class ICustomDllHandler
{
public:
    virtual ~ICustomDllHandler() {}
    // 处理消息，返回处理是否成功
    virtual bool ProcessMessage(const QString& strIPHop, const uint16_t BPUID, const uint8_t comNum,const QByteArray &message) = 0;
    // 返回该处理器的唯一标识符
    virtual QString handlerId() const = 0;
};

#endif // ICUSTOMDLLHANDLER_H 