#pragma once
#include <winsock2.h>
#include <windows.h>
#include <DbgHelp.h>
#include <string>

class DumpUtil {
public:
    static void InitDumpHandler(const std::wstring& dumpPath);
    static LONG WINAPI UnhandledExceptionFilter(EXCEPTION_POINTERS* pExceptionPointers);

private:
    static std::wstring s_dumpPath;
};