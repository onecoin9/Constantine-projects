#ifndef MT422DLLHANDLER_H
#define MT422DLLHANDLER_H

#include "ICustomDllHandler.h"
#include <QObject>
#include <QByteArray>
#include <QString>

class MT422DllHandler : public ICustomDllHandler
{
public:
    explicit MT422DllHandler(QObject* parent = nullptr);
    ~MT422DllHandler() override;

    // ICustomDllHandler interface
    bool ProcessMessage(const QString& strIPHop, const uint16_t BPUID, const uint8_t comNum,const QByteArray &message) override;
    QString handlerId() const override;

private:
    static const quint32 MT422_TAG_ID = 0x4D543432; // "MT422" in ASCII
};

#endif // MT422DLLHANDLER_H 