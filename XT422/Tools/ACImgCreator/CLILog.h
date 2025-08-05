#pragma once
#include <stdint.h>
#include <QFile>
#include "ILog.h"

class CLILog : public ILog
{
public:
    void Init();
    void Deinit();
    void PrintLog(int32_t Level, const char* fmt, ...);
    void PrintBuf(char* pHeader, char* pData, int Size);

    ~CLILog() {
        Deinit();
    }

private:
    QFile m_File;
};