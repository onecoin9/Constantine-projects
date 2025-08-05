#include <QApplication>
#include <QIcon>
#include "ui/MainWindow.h"
#include "core/CoreEngine.h"
#include "core/Logger.h"
#include <memory>
#include <QFile>
#include <QStyleFactory>
#include <QDateTime>
#include <QtConcurrent/QtConcurrent> // 新增
#include <QFutureWatcher>           // 新增
// #include <QMetaType>                // 关键修复：新增头文件

#ifdef _WIN32
#include <windows.h>
#endif

// 关键修复：提前包含头文件
#include "infrastructure/SerialPortManager.h"
// #include "domain/protocols/IXTProtocol.h" // 关键修复：新增头文件

// 全局变量，用于信号槽连接
std::shared_ptr<Core::CoreEngine> g_coreEngine;

void setupStyleSheet(QApplication &app) {
    app.setStyle(QStyleFactory::create("Fusion"));
    QFile styleFile(":/styles/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(QLatin1String(styleFile.readAll()));
        styleFile.close();
    } else {
        qWarning("无法打开样式文件");
    }
}

int main(int argc, char *argv[])
{
    // 在所有Qt操作之前，设置Windows控制台编码为UTF-8，解决中文乱码问题
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    // setvbuf(stdout, nullptr, _IOFBF, 1024); // 可选：增加缓冲区
#endif

    QApplication app(argc, argv);
    
    // 注册逻辑已移至XTProtocol构造函数，此处不再需要
    // qRegisterMetaType<Domain::Protocols::PowerFeedbackData>("PowerFeedbackData");

    // 在QApplication创建之后初始化日志系统
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss-zzz");
    TesterFramework::Logger::getInstance().initialize(QString("TesterFramework_%1.log").arg(timestamp).toStdString());
    TesterFramework::Logger::getInstance().setPattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    app.setApplicationName("TesterFramework");
    app.setOrganizationName("YourCompany");
    app.setWindowIcon(QIcon(":/resources/icons/Acroview.ico"));
    
    setupStyleSheet(app);

    auto coreEngine = std::make_shared<Core::CoreEngine>();
    g_coreEngine = coreEngine; // 赋值给全局变量，供WaitForSignalStep使用

    // --- 异步初始化流程 ---

    // 1. 立即创建并显示主窗口
    Presentation::MainWindow mainWindow;
    mainWindow.setWindowTitle("测试系统框架 - Tester Framework v1.0");
    mainWindow.showMaximized(); // UI线程不会被阻塞，窗口会立刻显示
    // 关键修复：移除异步初始化，直接在主线程中执行
    if (coreEngine->initialize()) {
        LOG_INFO("CoreEngine initialization finished successfully.");
        mainWindow.setCoreEngine(coreEngine);
    } else {
        LOG_ERROR("CoreEngine initialization finished with a failure status.");
        // 可以在这里弹出一个错误对话框提示用户
    }

    return app.exec();
} 
