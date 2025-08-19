#pragma once

#include <QtWidgets/QMainWindow>
#include <QtCore>
#include <QDir>
#include <qfile.h>
#include <QCryptographicHash>
#include "..\ParserPlugins\parser_global.h"
#include "ui_QtDataFile.h"

// https://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/ 中介紹
// 使用QThread衍生子類, 實現自己的run來執行是錯誤的做法, 
// 該在QObject衍生自己的類與程式, 用moveToThread推送給QThread的Instance來運算
// 永遠不應該在QObject 類別的建構函式中指派堆物件（使用new)
class HashThread : public QThread
{
    Q_OBJECT
public:
    explicit HashThread(QObject *partnt = 0);
    void run();
    int hashAlgoIndex;
    QString filename;

signals:
    void progressChanged(int percent);
    void updateHash(QString hash);
};



class MyOutput : public IOutput
{
public:
    MyOutput() {};
    virtual ~MyOutput() {};

    //分级别记录日志
    virtual int32_t Log(int32_t iLevel, const char* strOutput)
    {
        qDebug() << "log : " << strOutput;
        return 0;
    }

    //打印日志
    virtual int32_t Log(const char* strOutput)
    {
        qDebug() << "log : " << strOutput;
        return 0;
    }

    //打印警告
    virtual int32_t Warning(const char* strOutput)
    {
        qDebug() << "Warning : " << strOutput;
        return 0;
    }

    //打印错误
    virtual int32_t Error(const char* strOutput)
    {
        qDebug() << "Error : " << strOutput;
        return 0;
    }

    //弹出消息框显示信息
    virtual int32_t Message(const char* strOutput)
    {
        qDebug() << "Message : " << strOutput;
        return 0;
    }

};




class QtDataFile : public QMainWindow
{
    Q_OBJECT
public:
    QtDataFile(QWidget *parent = Q_NULLPTR);

public slots:
    void on_tbBrowseHeap_Clicked();
    void on_tbBrowse_Clicked();
    void on_AddHexEditor();
    void on_RemoveHexEditor();
    void on_tbFileHash_Clicked();
    void on_tbThreadHash_Clicked();
    void on_tbDetect_Clicked();
    void on_threadDone();
    void on_tbFileTransfer_Clicked();

public slots:
    void TransferErrorRpt(int, QString);

signals:
    void fileReadProgress(int percent);

private:
    Ui::QtDataFileClass ui;
    MyOutput* output;
    QString fileChecksum(const QString& fileName, QCryptographicHash::Algorithm hashAlgorithm);
};
