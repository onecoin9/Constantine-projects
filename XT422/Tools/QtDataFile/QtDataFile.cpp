#include "QtDataFile.h"
#include <QtWidgets>
#include <QtCore>
#include <QCryptographicHash>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QVBoxLayout>
#include "qhexedit.h"
#include "..\ParserPlugins\parser_global.h"

QtDataFile::QtDataFile(QWidget *parent)
    : QMainWindow(parent)
{
    output = new MyOutput();
    ui.setupUi(this);
    // ui component events
    QObject::connect(ui.tbBrowseHeap, &QToolButton::clicked, this, &QtDataFile::on_tbBrowseHeap_Clicked);
    QObject::connect(ui.tbBrowse, &QToolButton::clicked, this, &QtDataFile::on_tbBrowse_Clicked);
    QObject::connect(ui.pbHexEditor, &QPushButton::clicked, this, &QtDataFile::on_AddHexEditor);
    QObject::connect(ui.tbFileHash, &QToolButton::clicked, this, &QtDataFile::on_tbFileHash_Clicked);
    QObject::connect(ui.tbThreadHash, &QToolButton::clicked, this, &QtDataFile::on_tbThreadHash_Clicked);
    QObject::connect(ui.tbDetect, &QToolButton::clicked, this, &QtDataFile::on_tbDetect_Clicked);
    QObject::connect(ui.tbFileTransfer, &QToolButton::clicked, this, &QtDataFile::on_tbFileTransfer_Clicked);
    // software flow events
    QObject::connect(this, &QtDataFile::fileReadProgress, ui.progressHash, &QProgressBar::setValue);

    // Get app path and locate the test files located under ..\QC and ..\QC\DataFile folders
    // D:\PUI\AP9900PCSoftware\trunk\Tools\QtDataFile\Win32\Debug\QtWidgets.exe
    // D:\PUI\AP9900PCSoftware\trunk\Tools\QtDataFile\QC\Heap.bin
    // D:\PUI\AP9900PCSoftware\trunk\Tools\QtDataFile\QC\DatFile\motorola.s3
    QDir dir(QApplication::applicationDirPath());  
    if (dir.cdUp() && dir.cdUp()) // cd ..\..\QC
    {
        if (dir.cd("QC")) // D:\PUI\AP9900PCSoftware\trunk\Tools\QC
        {
            ui.eHeapFile->setText(dir.path() + "/Heap.bin"); // D:\PUI\AP9900PCSoftware\trunk\Tools\QC\Heap.bin
            if (dir.cd("DataFile"))
                ui.eFile->setText(dir.path() + "/motorola.s3"); // D:\PUI\AP9900PCSoftware\trunk\Tools\DataFile\motorola.s3
        }
    }
    dir = QApplication::applicationDirPath();
    if (dir.cd("parser"))
    {   
        // get all available parser dll filenames
        ui.cbParserNameList->clear();
        foreach(QFileInfo info, dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot))
        {
            // info.absoluteFilePath() = d:\test\parser\parserintelhex.dll
            // info.completeBaseName() = parserintelhex
            QPluginLoader pluginloader(info.absoluteFilePath());
            QObject* plugin = pluginloader.instance();
            if (plugin) // DLL 成功載入
            {
                // 通過qobjecct_cast<T> 正確的IID Pluging才會被取到parser中
                AbstractParser* parser = qobject_cast<AbstractParser*>(plugin);  
                if (parser)
                    parser->m_pOutput = output;
                if (parser)
                {
                    QString name = parser->getFormatName();
                    ui.cbParserNameList->addItem(name); // 放到ComboBox列表中
                }
            }
            else if (info.absoluteFilePath().contains(".dll"))
                qWarning() << "Failed to load plugin:" << pluginloader.errorString();
        }
    }
    // D:\PUI\AP9900PCSoftware\trunk\Tools\QtDataFile\Win32\Debug\parser\parserintelhex.dll
    QString fn = ui.eHeapFile->text();
    ui.pbHexEditor->setEnabled(!fn.isNull() && !fn.isEmpty() && QFile::exists(fn));
    ui.tbFileTransfer->setEnabled(!fn.isNull() && !fn.isEmpty() && QFile::exists(fn));
}

void QtDataFile::on_tbBrowseHeap_Clicked() 
{
    QString path = ui.eHeapFile->text();
    if ((!path.isEmpty()) && (!path.isNull()))
    {
        QFileInfo fi(path);
        path = fi.filePath();
    }
    else
        path = QDir::currentPath();
    QString fn = QFileDialog::getOpenFileName(
        this,
        tr("Open Binary File as Heap"),
        path,
        tr("Binary files (*.bin);;All files (*.*)")
    ); // Please note, path seperator is '/', but windows native seperator is '\'
    if (!fn.isNull()) // Empty string will still assign
        ui.eHeapFile->setText(QDir::toNativeSeparators(fn));
    fn = ui.eHeapFile->text();
    ui.pbHexEditor->setEnabled(!fn.isNull() && !fn.isEmpty() && QFile::exists(fn));
    ui.tbFileTransfer->setEnabled(!fn.isNull() && !fn.isEmpty() && QFile::exists(fn));
}

void QtDataFile::on_tbBrowse_Clicked()
{
    QString path = ui.eFile->text();
    if ((!path.isEmpty()) && (!path.isNull()))
    {
        QFileInfo fi(path);
        path = fi.filePath();
    }
    else
        path = QDir::currentPath();
    QString fn = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        path,
        tr("All files (*.*)")
    );
    if (!fn.isNull()) // Empty string will still assign
        ui.eFile->setText(QDir::toNativeSeparators(fn));
}

void QtDataFile::on_AddHexEditor()
{
    QHexEdit* he = new QHexEdit(NULL);
    if (he != NULL) 
        QObject::connect(he, &QHexEdit::close, this, &QtDataFile::on_RemoveHexEditor);
    QString fn = ui.eHeapFile->text();
    QFile fp; 
    fp.setFileName(fn);
    if (!he->setData(fp)) {
        QMessageBox::warning(this, tr("QHexEdit"), tr("Cannot read file %1:\n%2.").arg(fn).arg(fp.errorString()));
        return;
    }
    else
        he->show();
}

void QtDataFile::on_RemoveHexEditor()
{ 
    QHexEdit* he = qobject_cast<QHexEdit*>(sender());
    if (he)
        delete he; // How about the QFile? memory leak?
}

//QCryptographicHash::
//  Algorithm hashAlgorithm
//  Md4	0	Generate an MD4 hash sum
//  Md5	1	Generate an MD5 hash sum
//  Sha1	2	Generate an SHA - 1 hash sum
//  Sha224	3	Generate an SHA - 224 hash sum(SHA - 2).Introduced in Qt 5.0
//  Sha256	4	Generate an SHA - 256 hash sum(SHA - 2).Introduced in Qt 5.0
//  Sha384	5	Generate an SHA - 384 hash sum(SHA - 2).Introduced in Qt 5.0
//  Sha512	6	Generate an SHA - 512 hash sum(SHA - 2).Introduced in Qt 5.0
//  Keccak_224	7	Generate a Keccak - 224 hash sum.Introduced in Qt 5.9.2
//  Keccak_256	8	Generate a Keccak - 256 hash sum.Introduced in Qt 5.9.2
//  Keccak_384	9	Generate a Keccak - 384 hash sum.Introduced in Qt 5.9.2
//  Keccak_512	10	Generate a Keccak - 512 hash sum.Introduced in Qt 5.9.2
//  Sha3_224	11 RealSha3_224	Generate an SHA3 - 224 hash sum.Introduced in Qt 5.1
//  Sha3_256	12 RealSha3_256	Generate an SHA3 - 256 hash sum.Introduced in Qt 5.1
//  Sha3_384	13 RealSha3_384	Generate an SHA3 - 384 hash sum.Introduced in Qt 5.1
//  Sha3_512	14 RealSha3_512	Generate an SHA3 - 512 hash sum.Introduced in Qt 5.1
QString QtDataFile::fileChecksum(const QString& fileName, QCryptographicHash::Algorithm hashAlgorithm)
{
    QFile sourceFile(fileName);
    qint64 readSoFar = 0;
    qint64 fileSize = sourceFile.size();
    qint64 cnt = fileSize;
    const qint64 bufferSize = 10240;
    int percentProgress = -1;

    if (sourceFile.open(QIODevice::ReadOnly | QFile::ExistingOnly) && (fileSize > 0))
    {
        char buffer[bufferSize];
        int bytesRead;
        int readSize = qMin(cnt, bufferSize);
        int p;

        QCryptographicHash hash(hashAlgorithm);
        while ((readSize > 0) && ((bytesRead = sourceFile.read(buffer, readSize)) > 0))
        {
            cnt -= bytesRead;
            hash.addData(buffer, bytesRead);
            readSize = qMin(cnt, bufferSize);

            readSoFar += bytesRead;
            p = readSoFar * 100 / fileSize;
            if (percentProgress != p)
                emit fileReadProgress(percentProgress = p);
        }

        sourceFile.close();
        return QString(hash.result().toHex());
    }
    else
        return QString(); // Clear ui.eHashedit
}

void QtDataFile::on_tbFileHash_Clicked()
{
    ui.tbFileHash->setEnabled(false);
    QApplication::processEvents();

    QString fn = ui.eFile->text();
    ui.eFilesum->setText(fileChecksum(fn, static_cast<QCryptographicHash::Algorithm>(ui.cbHashAlgo->currentIndex())));
    ui.tbFileHash->setEnabled(true);
}

void QtDataFile::on_tbThreadHash_Clicked()
{ 
    ui.tbThreadHash->setEnabled(false);
    ui.progressHash->setValue(0);
    QApplication::processEvents();
    QString fn = ui.eHeapFile->text();

    // 請注意 這樣的Thread的用法有問題 有專家推薦要用QObject moveToThread的方法
    HashThread *thr = new HashThread(this);
    thr->filename = fn;
    thr->hashAlgoIndex = ui.cbHashAlgo->currentIndex();
    QObject::connect(thr, &HashThread::progressChanged, ui.progressHash, &QProgressBar::setValue);
    QObject::connect(thr, &HashThread::updateHash, ui.eFilesum, &QLineEdit::setText);
    QObject::connect(thr, &HashThread::finished, this, &QtDataFile::on_threadDone);

    thr->start(); // how to link to the Thread Done
}

void QtDataFile::on_tbDetect_Clicked()
{
    ui.tbDetect->setEnabled(false);
    QString fn = ui.eFile->text();
    bool found = false;

    QDir dir = QApplication::applicationDirPath();
    if (dir.cd("parser"))
    {
        // get all available parser dll filenames
        foreach(QFileInfo info, dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot))
        {
            // info.absoluteFilePath() = d:\test\parser\parserintelhex.dll
            // info.completeBaseName() = parserintelhex
            QPluginLoader pluginloader(info.absoluteFilePath());
            QObject* plugin = pluginloader.instance();
            if (plugin)
            {
                AbstractParser* parser = qobject_cast<AbstractParser*>(plugin);
                if (parser)
                    parser->m_pOutput = output;
                if (parser && parser->ConfirmFormat(fn))
                {
                    QString name = parser->getFormatName();
                    ui.cbParserNameList->setCurrentIndex(ui.cbParserNameList->findText(name, Qt::MatchExactly));
                    found = true;
                    break;
                }
            }
        }
    }
    if (!found)
    {
        ui.cbParserNameList->setCurrentIndex(-1);
    }

    ui.tbDetect->setEnabled(true);
}

void QtDataFile::on_threadDone()
{
    HashThread* thr = qobject_cast<HashThread*>(sender());
    QObject::disconnect(thr, &HashThread::progressChanged, ui.progressHash, &QProgressBar::setValue);
    QObject::disconnect(thr, &HashThread::updateHash, ui.eFilesum, &QLineEdit::setText);
    QObject::disconnect(thr, &HashThread::finished, this, &QtDataFile::on_threadDone);
    delete thr;

    ui.tbThreadHash->setEnabled(true);
}

// https://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/ 中介紹
// 使用QThread衍生子類, 實現自己的run來執行是錯誤的做法, 該在QObject衍生自己的類與程式, 用moveToThread推送給QThread的Instance來運算
// 永遠不應該在QObject 類別的建構函式中指派堆物件（使用new）
HashThread::HashThread(QObject *parent) : QThread(parent)
{
    hashAlgoIndex = 0; // new HashThread之後對instance
}

void HashThread::run()
{
    QFile sourceFile(filename);
    qint64 readSoFar = 0;
    qint64 fileSize = sourceFile.size();
    qint64 cnt = fileSize;
    const qint64 bufferSize = 10240;
    int percentProgress = -1;

    emit updateHash(QString("")); // Clear ui.eHashedit

    if (sourceFile.open(QIODevice::ReadOnly) && (fileSize > 0))
    {
        char buffer[bufferSize];
        int bytesRead;
        int readSize = qMin(cnt, bufferSize);
        int p;

        QCryptographicHash hash(static_cast<QCryptographicHash::Algorithm>(hashAlgoIndex));
        while (readSize > 0 && (bytesRead = sourceFile.read(buffer, readSize)) > 0)
        {
            cnt -= bytesRead;
            hash.addData(buffer, bytesRead);
            readSize = qMin(cnt, bufferSize);

            readSoFar += bytesRead;
            p = readSoFar * 100 / fileSize;
            if (percentProgress != p)
                emit progressChanged(percentProgress = p);
        }

        sourceFile.close();
        emit updateHash(hash.result().toHex()); // update sum
    }
}

void QtDataFile::on_tbFileTransfer_Clicked()
{


    ui.tbFileTransfer->setEnabled(false);
    ui.tbAbort->setEnabled(true);
    ui.lineErrorRpt->setText(QString(""));
    bool AutoDetect = ui.cbAutoDetect->isChecked();
    QString FormatName;
    QString dstfn = ui.eHeapFile->text(); // dst
    QString srcfn = ui.eFile->text();
    bool found = false;
    if (ui.cbParserNameList->currentIndex() >= 0)
        FormatName = ui.cbParserNameList->currentText();
    else
        AutoDetect = true;
    ui.progressTransfer->setValue(0);

    QDir dir = QApplication::applicationDirPath();
    if (dir.cd("parser"))
    {
        // get all available parser dll filenames
        foreach(QFileInfo info, dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot))
        {
            // info.absoluteFilePath() = d:\test\parser\parserintelhex.dll
            // info.completeBaseName() = parserintelhex
            QPluginLoader pluginloader(info.absoluteFilePath());
            QObject* plugin = pluginloader.instance();
            if (plugin)
            {
                AbstractParser* parser = qobject_cast<AbstractParser*>(plugin);
                if (parser)
                    parser->m_pOutput = output;
                if (parser)
                    if ((AutoDetect && parser->ConfirmFormat(srcfn)) || (!AutoDetect) && (FormatName.compare(parser->getFormatName() == 0)))
                    {
                        if (AutoDetect)
                        {
                            FormatName = parser->getFormatName();
                            ui.cbParserNameList->setCurrentIndex(ui.cbParserNameList->findText(FormatName, Qt::MatchExactly));
                        }
                        found = true;
                        QObject::connect(parser, &AbstractParser::Progress, ui.progressTransfer, &QProgressBar::setValue);
                        QObject::connect(parser, &AbstractParser::ErrorRpt, this, &QtDataFile::TransferErrorRpt);
                        QObject::connect(ui.tbAbort, &QToolButton::clicked, parser, &AbstractParser::UserAbort);
                        QFile dst(dstfn);
                        dst.open(QIODevice::ReadWrite);
                        if (dst.isOpen())
                        {
                            parser->TransferFile(srcfn, &dst);
                            dst.close();
                        }
                        QObject::disconnect(ui.tbAbort, &QToolButton::clicked, parser, &AbstractParser::UserAbort);
                        QObject::disconnect(parser, &AbstractParser::ErrorRpt, this, &QtDataFile::TransferErrorRpt);
                        QObject::disconnect(parser, &AbstractParser::Progress, ui.progressTransfer, &QProgressBar::setValue);
                        break;
                    }
            }
        }
    }
    if (!found)
    {
        QMessageBox::warning(this, tr("FileTransfer"), tr("Format Parser not found!"));
    }

    ui.tbAbort->setEnabled(false);
    ui.tbFileTransfer->setEnabled(true);
}

void QtDataFile::TransferErrorRpt(int error, QString msg)
{
    // update to ui.lineErrorRpt
    ui.lineErrorRpt->setText(QString("Error = %1, %2").arg(error).arg(msg));
}