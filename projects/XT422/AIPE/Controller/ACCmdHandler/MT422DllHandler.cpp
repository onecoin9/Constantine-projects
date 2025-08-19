#include "MT422DllHandler.h"
#include "MT422SerialHandler.h"
#include "CustomMessageHandler.h"
#include <QDebug>
#include <QThread>

static MT422SerialHandler *g_serialHandler = nullptr;

MT422DllHandler::MT422DllHandler(QObject* parent)
{
    if (!g_serialHandler) {
        g_serialHandler = &MT422SerialHandler::instance();
    }
}

MT422DllHandler::~MT422DllHandler()
{
    // 不在这里删除 g_serialHandler，因为可能还有其他 MT422DllHandler 实例在使用
}

bool MT422DllHandler::ProcessMessage(const QString& strIPHop, const uint16_t BPUID, const uint8_t comNum,const QByteArray &message)
{
    // 获取从上一阶段传递的traceId
    quint64 traceId = 0;
    QVariant traceVar = QThread::currentThread()->property("currentTraceId");
    if (traceVar.isValid()) {
        traceId = traceVar.toULongLong();
    }
    if (!g_serialHandler) {
        return false;
    }
    
    // 继续传递traceId到下一阶段
    g_serialHandler->setProperty("currentTraceId", QVariant::fromValue(traceId));
    
    // 发送消息
    if (!g_serialHandler->sendMessage(strIPHop, BPUID, comNum,message)) {
        return false;
    }
    return true;
}

QString MT422DllHandler::handlerId() const
{
    return "MT422";
}

ICustomDllHandler* CreateDllHandler()
{
    return new MT422DllHandler();
}
