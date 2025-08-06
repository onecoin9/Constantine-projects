#include <QApplication>
#include <QMessageBox>
#include "acroViewTester.h"
#include "ProjectSelectDialog.h"
#include "logManager.h"
#include "dumpUtil.h"
#include "../acroViewTester/src/utils/db/sqlite3.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <winsock2.h>
#include <windows.h>
#include <winnt.h>
#include <Dbghelp.h>
#include <QString>
#include <QDebug>

#pragma comment(lib, "Dbghelp.lib")

namespace {
    // 生成当前时间字符串
    std::wstring GetCurrentTimeString() {
        const int TIMESTRLEN = 32;
        WCHAR timeStr[TIMESTRLEN];
        SYSTEMTIME time;
        GetLocalTime(&time);
        swprintf_s(timeStr, TIMESTRLEN, L"%4d_%02d_%02d_%02d_%02d_%02d",
            time.wYear, time.wMonth, time.wDay,
            time.wHour, time.wMinute, time.wSecond);
        return timeStr;
    }

    // 创建dump文件
    void CreateDumpFile(EXCEPTION_POINTERS* pException, const std::wstring& filename) {
        HANDLE hDumpFile = CreateFile(filename.c_str(),
            GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, nullptr);

        if (hDumpFile != INVALID_HANDLE_VALUE) {
            MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
            dumpInfo.ExceptionPointers = pException;
            dumpInfo.ThreadId = GetCurrentThreadId();
            dumpInfo.ClientPointers = TRUE;

            MiniDumpWriteDump(GetCurrentProcess(),
                GetCurrentProcessId(),
                hDumpFile,
                MiniDumpNormal,
                &dumpInfo,
                nullptr,
                nullptr);

            CloseHandle(hDumpFile);
        }
    }

    // 初始化SQLite数据库
    bool InitializeDatabase() {
        sqlite3* sqldb = nullptr;
        int res = sqlite3_open("data/user.db", &sqldb);

        if (res == SQLITE_OK) {
            qDebug() << "Database opened successfully";
            sqlite3_close(sqldb);
            return true;
        }

        qDebug() << "Failed to open database:";
        qDebug() << "Error code:" << sqlite3_errcode(sqldb);
        qDebug() << "Error message:" << sqlite3_errmsg(sqldb);
        return false;
    }
}

// 异常捕获处理函数
LONG WINAPI ExceptionCapture(EXCEPTION_POINTERS* pException) {
    std::wstring timeStr = GetCurrentTimeString();
    std::wstring dumpFilename = L"acroview_" + timeStr + L".dmp";

    CreateDumpFile(pException, dumpFilename);

    return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // 设置应用程序的基本信息，有助于 QSettings 和日志目录等的管理
    QCoreApplication::setOrganizationName("AcroView");
    QCoreApplication::setApplicationName("acroViewTester");
    
    // 显示项目选择对话框
    ProjectSelectDialog projectDialog;
    if (projectDialog.exec() != QDialog::Accepted) {
        // 用户取消了项目选择，退出应用程序
        return 0;
    }
    
    // 获取选择的项目信息
    QString projectPath = projectDialog.getSelectedProjectPath();
    QString projectName = projectDialog.getSelectedProjectName();
    
    // 确保项目配置文件存在
    QFileInfo configFile(projectPath);
    if (!configFile.exists() || !configFile.isFile()) {
        QMessageBox::critical(nullptr, "错误", "所选项目配置文件不存在！");
        return 1;
    }
    
    // 记录启动信息
    LOG_INFO("应用程序启动，选择了项目：%s，配置文件路径：%s", 
             qPrintable(projectName), qPrintable(projectPath));
    
    // 注册异常捕获
    SetUnhandledExceptionFilter(ExceptionCapture);
    DumpUtil::InitDumpHandler(L"./dumps");

    // 初始化数据库
    if (!InitializeDatabase()) {
        qDebug() << "Database initialization failed";
        // 可以决定是否继续运行程序
    }

    // 创建主应用程序窗口，并传入项目配置路径
    acroViewTester w(nullptr, projectPath, projectName);
    w.show();
    
    return a.exec();
}
