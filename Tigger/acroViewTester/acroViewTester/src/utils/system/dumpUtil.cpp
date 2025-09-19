#include "dumpUtil.h"

std::wstring DumpUtil::s_dumpPath;

void DumpUtil::InitDumpHandler(const std::wstring& dumpPath) {
    s_dumpPath = dumpPath;
    SetUnhandledExceptionFilter(UnhandledExceptionFilter);
}

LONG WINAPI DumpUtil::UnhandledExceptionFilter(EXCEPTION_POINTERS* pExceptionPointers) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    wchar_t szFileName[MAX_PATH];
    swprintf_s(szFileName, MAX_PATH, L"%s\\Crash_%04d%02d%02d_%02d%02d%02d.dmp",
        s_dumpPath.c_str(), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    HANDLE hFile = CreateFileW(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
        exceptionInfo.ThreadId = GetCurrentThreadId();
        exceptionInfo.ExceptionPointers = pExceptionPointers;
        exceptionInfo.ClientPointers = FALSE;

        MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            hFile,
            MiniDumpWithFullMemory,  // 使用完整内存转储
            &exceptionInfo,
            NULL,
            NULL);

        CloseHandle(hFile);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}