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
#include <QMessageBox>
// #include <QMetaType>                // 关键修复：新增头文件
#include <ShlObj.h>
#include <Knownfolders.h>
#include <Dbghelp.h>
#include <combaseapi.h>
#include <thread>
#include <werapi.h>
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "wer.lib")


#ifdef _WIN32
#include <windows.h>
#endif

// 关键修复：提前包含头文件
#include "infrastructure/SerialPortManager.h"
// #include "domain/protocols/IXTProtocol.h" // 关键修复：新增头文件

// 全局变量，用于信号槽连接
std::shared_ptr<Core::CoreEngine> g_coreEngine;


void setupStyleSheet(QApplication& app) {
    app.setStyle(QStyleFactory::create("Fusion"));
    QFile styleFile(":/styles/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(QLatin1String(styleFile.readAll()));
        styleFile.close();
    }
    else {
        qWarning("无法打开样式文件");
    }
}


void DisableSystemCrashUI()
{
#ifdef _WIN32
    SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS);
#endif
}

//--------------------------------------------
// Dump 生成函数（Vectored Handler 版本）
//--------------------------------------------
LONG WINAPI EnhancedExceptionCapture(EXCEPTION_POINTERS* pException)
{

    DWORD code = pException->ExceptionRecord->ExceptionCode;
    if (code != 0xC0000005) {
        return EXCEPTION_CONTINUE_SEARCH; // 跳过，不生成dump
    }

    // 生成 dump 文件名
    SYSTEMTIME time;
    GetLocalTime(&time);
    WCHAR fileName[MAX_PATH];
    swprintf_s(fileName, L"dumps\\TesterFramework_%04d%02d%02d_%02d%02d%02d.dmp",
        time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

    HANDLE hFile = CreateFileW(fileName, GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        wchar_t msg[256];
        swprintf_s(msg, L"CreateFileW失败 错误码：%u", err);
        MessageBoxW(NULL, msg, L"Dump Error", MB_OK);
        return EXCEPTION_CONTINUE_SEARCH;
    }

    MINIDUMP_EXCEPTION_INFORMATION dumpInfo = { GetCurrentThreadId(), pException, TRUE };
    BOOL result = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
        MiniDumpWithFullMemory, &dumpInfo, nullptr, nullptr);
    CloseHandle(hFile);

    if (!result) {
        DWORD err = GetLastError();
        wchar_t msg[256];
        swprintf_s(msg, L"MiniDumpWriteDump失败 错误码：%u", err);
        MessageBoxW(NULL, msg, L"Dump Error", MB_OK);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}



int main(int argc, char* argv[])
{
#ifdef _WIN32
    DisableSystemCrashUI();
    // 注册全局异常钩子（覆盖一切线程）
    AddVectoredExceptionHandler(1, (PVECTORED_EXCEPTION_HANDLER)EnhancedExceptionCapture);
#endif

    // 在所有Qt操作之前，设置Windows控制台编码为UTF-8，解决中文乱码问题
#ifdef _WIN32
    SetConsoleOutputCP(936);
    SetConsoleCP(936);
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
    app.setOrganizationName("Acroview");
    app.setWindowIcon(QIcon(":/resources/icons/Acroview.ico"));
    setupStyleSheet(app);
    //QMessageBox::warning(nullptr, QStringLiteral("警告"), QStringLiteral("当前版本为内部测试版本，请勿用于生产"), QMessageBox::Ok);

    auto coreEngine = std::make_shared<Core::CoreEngine>();
    g_coreEngine = coreEngine; // 赋值给全局变量，供WaitForSignalStep使用





    // --- 异步初始化流程 ---

    // 1. 立即创建并显示主窗口
    int ret = 0;
    {
        QSettings settings("config/setting.ini", QSettings::IniFormat);
        // 尝试设置编码（兼容Qt5和Qt6）
        QTextCodec* codec = QTextCodec::codecForName("UTF-8");
        if (codec) {
            settings.setIniCodec(codec);
        }
        settings.beginGroup("System");
        QString version = settings.value("Version", "V0.0.0").toString();
        settings.endGroup();

        Presentation::MainWindow mainWindow;
        mainWindow.setWindowTitle("昂科测试系统 " + version);
        mainWindow.showMaximized(); // UI线程不会被阻塞，窗口会立刻显示
        // 关键修复：移除异步初始化，直接在主线程中执行
        if (coreEngine->initialize()) {
            LOG_INFO("CoreEngine initialization finished successfully.");
            mainWindow.setCoreEngine(coreEngine);
        }
        else {
            LOG_ERROR("CoreEngine initialization finished with a failure status.");
            // 可以在这里弹出一个错误对话框提示用户
        }


        ret = app.exec();
        // 作用域结束前销毁所有UI对象，防止其析构中继续写日志
    }

    // 确保核心引擎先停止并销毁，避免其析构期间写日志
    if (g_coreEngine) {
        g_coreEngine->shutdown();
        g_coreEngine.reset();
    }

    // 最后关闭日志系统，避免spdlog关闭后仍有对象写日志导致崩溃
    TesterFramework::Logger::getInstance().shutdown();

    return ret;
}
