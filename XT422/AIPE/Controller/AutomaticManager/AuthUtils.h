#pragma once
#include <string>
#include <memory>
#include <QObject>
#include <QTcpSocket>

class AuthUtils : public QObject {
    Q_OBJECT

public:
    static AuthUtils& Instance() {
        static AuthUtils instance;
        return instance;
    }

    bool StartExternalProcess();
    bool IsProcessRunning();
    bool StopExternalProcess();
    // 添加常量定义
    static constexpr const char* LICENSE_SERVER_EXE = "ACAutoLicServer.exe";

private:
    AuthUtils(QObject* parent = nullptr) : QObject(parent) {}
    ~AuthUtils() = default;
    AuthUtils(const AuthUtils&) = delete;
    AuthUtils& operator=(const AuthUtils&) = delete;
};
