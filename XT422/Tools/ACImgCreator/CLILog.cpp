#include "CLILog.h"
#include <QCoreApplication>
#include <QTime>
#include <QDate>

#define LOGBUFSIZE (1024)
char strErrLevel[][2] =
{
    "D",
    "N",
    "W",
    "E"
};

void CLILog::Init()
{
    bool Ret = false;
    QString strErr;
    QString strAppPath = QCoreApplication::applicationDirPath();
    QTime CurTime = QTime::currentTime();
    QDate CurDate = QDate::currentDate();
    QString strTime = CurTime.toString("hhmmss");
    QString strDate = CurDate.toString("yyyyMMdd");
    QString strFileName = strAppPath + "\\log\\" + strDate + "-" + strTime + ".log";
    m_File.setFileName(strFileName);
    m_File.setPermissions(QFileDevice::ReadOther);
    Ret = m_File.open(QIODevice::ReadWrite);
    if (Ret == false) {
        strErr = m_File.errorString();
    }
    return;
}
void CLILog::Deinit()
{
    if (m_File.isOpen()) {
        m_File.close();
    }
}


void CLILog::PrintLog(int32_t Level, const char* fmt, ...)
{
    QString strTime;
    QString strLog;
    char* strLevel = strErrLevel[Level];
    QTime CurTime = QTime::currentTime();
    QDate CurDate = QDate::currentDate();
    char TmpBuf[LOGBUFSIZE];
    strTime = CurTime.toString("hh:mm:ss.zzz");
    QString strDate = CurDate.toString("yyyyMMdd");
    if (Level < m_LogLevel) {
        return;
    }
    memset(TmpBuf, 0, LOGBUFSIZE);
    va_list args;
    va_start(args, fmt);
    vsprintf(TmpBuf, fmt, args);
    va_end(args);

    if (m_bTimeHeadEn) {
        strLog = QString::asprintf("[%s-%s - %s] %s", strDate.toUtf8().toStdString().c_str(), strTime.toUtf8().toStdString().c_str(), strLevel, TmpBuf);
    }
    else {
        strLog = QString::asprintf("%s", TmpBuf);
    }
    printf("%s", strLog.toUtf8().toStdString().c_str());
    if (m_File.isOpen()) {
        m_File.write((char*)strLog.toUtf8().toStdString().c_str(), strLog.size());
        m_File.flush();
    }
}
void CLILog::PrintBuf(char* pHeader, char* pData, int Size)
{
    int i = 0;
    QString strLog;
    if (pHeader != NULL) {
        strLog = QString::asprintf("%s ===Bytes:%d\r\n", pHeader, Size);
        printf("%s", strLog.toUtf8().toStdString().c_str());
        if (m_File.isOpen()) {
            m_File.write((char*)strLog.toUtf8().toStdString().c_str(), strLog.size());
        }
    }
    strLog = "";
    for (i = 0; i < Size; ++i) {
        if (i % 16 == 0 && i != 0) {
            strLog += "\r\n";
            printf("%s", strLog.toUtf8().toStdString().c_str());
            if (m_File.isOpen()) {
                m_File.write((char*)strLog.toUtf8().toStdString().c_str(), strLog.size());
            }
            strLog = "";
        }
        strLog += QString::asprintf("%02X ", (uint8_t)pData[i]);
    }
    strLog += "\r\n";
    printf("%s", strLog.toUtf8().toStdString().c_str());
    if (m_File.isOpen()) {
        m_File.write((char*)strLog.toUtf8().toStdString().c_str(), strLog.size());
    }
    m_File.flush();
}

