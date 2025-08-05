#include "CLITest.h"
#include <QDebug>
#include <QTime>
#include <QCoreApplication>
#include "Version.h"
#include <QFile>
#include <QDataStream>

#define STR_SOFT_VERSTION  ("V1.39")

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

#define LOGBUFSIZE (2048)
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
    QString strFileName = strAppPath+"\\log\\"+ strDate + "-" + strTime +".txt";
    m_File.setFileName(strFileName);
    m_File.setPermissions(QFileDevice::ReadOther);
    Ret=m_File.open(QIODevice::ReadWrite);
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
        strLog=QString::asprintf("[%s-%s-%s - %s] %s",STR_SOFT_VERSTION, strDate.toUtf8().toStdString().c_str(),strTime.toUtf8().toStdString().c_str(), strLevel, TmpBuf);
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
void CLILog::PrintBuf(char*pHeader, char* pData, int Size)
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
        if (i % 16 == 0 && i!=0) {
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

class CCLIApp 
{
public:
    CLILog Log;
    CAppModels AppModels;
    CCLIView AppView;
    CAppCtrls AppCtrls;
};

CCLIApp g_theApp;

class CAbsTest {
public:
    virtual void Test1() = 0;
    virtual void Test2() = 0;
    char D1;
    uint32_t D2;
    char  Data[12];
};

int32_t FuncTest()
{
   /* QHash<int,QString> Test;
    Test.insert(1, "aaa");
    Test.insert(2, "bbb");
    Test.insert(3, "ccc");
    Test.insert(4, "ddd");

    QString strTmp = "0x10";
    QString strTmp1 = "0x100000000000";
    uint32_t Data = strTmp.toInt(NULL,16);
    uint64_t Data1 = strTmp1.toULongLong(NULL, 16);
    QHash<int, QString>::iterator Itr = Test.begin();
    while (Itr != Test.end()) {
        printf("String:%s\r\n", Itr.value().toStdString().c_str());
        Itr++;
    }
    return 0;*/
    uint8_t Data[] = { 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08 };
    uint32_t Checksum = 0;
    CCrc32Std Crc32Std;
    Crc32Std.CalcSubRoutine(Data,8);
    Crc32Std.GetChecksum((uint8_t*)&Checksum,4);
    printf("ChksumCRC32:0x%08X\r\n", Checksum);

    QString str = QString::number(Checksum, 16).toUpper().insert(0,"0x");
    printf("ChksumCRC32:%s\r\n", str.toStdString().c_str());

    printf("tCmdCplPacketGetCRC32:%d\r\n", (int)sizeof(tCmdCplPacketGetCRC32));
    QString str11 = "12345678";
    QByteArray ByteArray = QByteArray::fromHex(str11.toStdString().c_str());

    QByteArray ByteArray1;
    QDataStream DataStream(&ByteArray1,QIODevice::ReadWrite);
    
    quint32  RetCode=0x3445, DataLen=0x5678;
    qint32  BytesWrite;
    BytesWrite=DataStream.writeRawData((char*)Data, 8);
    DataStream <<  RetCode << DataLen;
    DataStream.device()->seek(0);
    bool isEnd=DataStream.atEnd();
    DataStream.setByteOrder(QDataStream::LittleEndian);
    DataStream >> RetCode >> DataLen;
    QDataStream::Status status=DataStream.status();

    double Speed = 0;
    uint32_t Size = 0x3FFF5000;
    int ms = 6000;
    Speed = (double)Size / (double)ms * 1000/(double)(1024*1024);
    printf("Speed:%.2lf\r\n", Speed);

    printf("Sizeof Class :%d\r\n", sizeof(CAbsTest));
    return 0;
}

int32_t CLITest()
{
	int32_t Ret = 0;
    CCLIApp* ptheApp = &g_theApp;
    uint8_t TmpData[] = { 0x01,0x02,0x03,0x04 };
    QString strAppPath = QCoreApplication::applicationDirPath();
    //FuncTest();

    ptheApp->AppModels.m_AppConfigModel.LoadConfig(strAppPath+"/AppConfig.ini");
    ptheApp->Log.Init();
    ptheApp->Log.SetLogLevel(ptheApp->AppModels.m_AppConfigModel.LogLevel());
    ptheApp->Log.PrintLog(LOGLEVEL_N, "============Test Net Comm Start=============\r\n");
    ptheApp->Log.PrintLog(LOGLEVEL_N, "Version:%s, Date:%s\r\n", STR_SOFT_VERSTION,SOFT_DATE);
    ptheApp->AppView.AttachAppModel(&ptheApp->AppModels);
    ptheApp->AppView.AttachILog(&ptheApp->Log);

    ptheApp->AppCtrls.AttachAppModels(&ptheApp->AppModels);
    ptheApp->AppCtrls.AttachILog(&ptheApp->Log);
    
    ptheApp->AppCtrls.RunCtrls();

    //AppCtrls.StopCtrls();


    //Log.PrintLog(0, "============Test Net Comm End =================\r\n");

	return Ret;
}