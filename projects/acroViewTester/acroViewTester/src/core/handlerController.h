#pragma once
#ifndef HANDLERCONTROLLER_H
#define HANDLERCONTROLLER_H

#include <string>
#include "tcphandler.h"

class HandlerController {
public:
    HandlerController();
    ~HandlerController();

    // 查询协议版本号
    void getVersion();

    // 设定使能
    void setEnable();

    // 开始批量
    void lotStart();

    // 结束批量
    void lotEnd();

    // 结束测试
    void endTest();

    // 获取报警信息
    void sendAlarm();

private:
    bool m_isEnabled;
    bool m_isBatchRunning;
    TcpHandler tcphanlder;
    std::string m_protocolVersion;
};

#endif // HANDLERCONTROLLER_H 