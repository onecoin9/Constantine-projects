#include "AuthUtils.h"
//#include "AngkLogger.h"
#include <QDebug>
#include <QCoreApplication>
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <thread>
#include <fstream>


bool AuthUtils::StartExternalProcess() {
    const std::string appDir = QCoreApplication::applicationDirPath().toStdString();
    const std::string licExePath = appDir + "/Lic/Tools/";
    const std::string licFilePath = appDir + "/Lic/lic.aclic";
    // 构造命令脚本内容
    const std::string command = "cd /d \"" + licExePath + "\" && " + LICENSE_SERVER_EXE+" -i " + licFilePath;

    // 创建唯一的批处理文件名
    std::string batchFile = std::string(licExePath) + "/start_license_server.bat";
    if (std::ifstream(batchFile)) {
        std::remove(batchFile.c_str());
    }
    
    // 写入批处理文件
    std::ofstream batch(batchFile);
    if (batch.is_open()) {
        batch << "@echo off\n";
        batch << command << "\n";
        batch.close();
    } else {
        //ALOG_ERROR("Failed to create batch file: %s", "CU", "--", batchFile.c_str());
        return false;
    }

    // 将路径转换为宽字符
    int bufferSize = MultiByteToWideChar(CP_UTF8, 0, batchFile.c_str(), -1, NULL, 0);
    std::wstring wBatchFile(bufferSize, 0);
    MultiByteToWideChar(CP_UTF8, 0, batchFile.c_str(), -1, &wBatchFile[0], bufferSize);

    // 设置启动信息
    //STARTUPINFOW si;
    //PROCESS_INFORMATION pi;

    //ZeroMemory(&si, sizeof(si));
    //si.cb = sizeof(si);
    //ZeroMemory(&pi, sizeof(pi));
    // 设置窗口为最小化
    //si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    //si.wShowWindow = SW_HIDE;
    //bool success = true;
    //success = CreateProcessW(
    //    NULL,                                // 应用程序名
    //    const_cast<LPWSTR>(wBatchFile.c_str()), // 命令行
    //    NULL,                                // 进程安全属性
    //    NULL,                                // 线程安全属性
    //    TRUE,                               // 句柄继承标记
    //    NULL,                    // 创建标记：不显示命令行窗口
    //    NULL,                                // 环境块
    //    NULL,                                // 当前目录
    //    &si,                                 // 启动信息
    //    &pi                                  // 进程信息
    //);

    SHELLEXECUTEINFO ShExecInfo = { 0 };
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = _T("open");
    ShExecInfo.lpFile = wBatchFile.c_str();
    ShExecInfo.lpParameters = NULL;
    ShExecInfo.lpDirectory = NULL;
    ShExecInfo.nShow = SW_HIDE;
    ShExecInfo.hInstApp = NULL;
    int success = ShellExecuteEx(&ShExecInfo);
    if (success) {
        // 等待进程初始化完成
        //ALOG_INFO("Process started successfully", "CU", "--");
    } else {
        //ALOG_ERROR("Failed to start process", "CU", "--");
    }

    // 延迟一小段时间后删除批处理文件
    std::thread([batchFile]() {
        Sleep(2000);  // 等待2秒
        DeleteFileA(batchFile.c_str());
    }).detach();

    return success != 0;
}

bool AuthUtils::IsProcessRunning() {
    const std::string process = LICENSE_SERVER_EXE;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(processEntry);

    if (!Process32First(snapshot, &processEntry)) {
        CloseHandle(snapshot);
        return false;
    }

    do {
        wchar_t wProcessName[260];
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, wProcessName, process.c_str(), 260);
        if (_wcsicmp(processEntry.szExeFile, wProcessName) == 0) {
            CloseHandle(snapshot);
            return true;
        }
    } while (Process32Next(snapshot, &processEntry));

    CloseHandle(snapshot);
    return false;
}

bool AuthUtils::StopExternalProcess() {
    const std::string process = LICENSE_SERVER_EXE;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(processEntry);

    if (!Process32First(snapshot, &processEntry)) {
        CloseHandle(snapshot);
        return false;
    }

    bool success = false;
    do {
        wchar_t processNameW[260];
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, processNameW, process.c_str(), 260);
        if (_wcsicmp(processEntry.szExeFile, processNameW) == 0) {
            HANDLE processHandle = OpenProcess(PROCESS_TERMINATE, FALSE, processEntry.th32ProcessID);
            if (processHandle != NULL) {
                success = TerminateProcess(processHandle, 0);
                CloseHandle(processHandle);
            }
            break;
        }
    } while (Process32Next(snapshot, &processEntry));

    CloseHandle(snapshot);
    return success;
}
