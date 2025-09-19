#include "C2000outParser.h"
#include <QtWidgets/QApplication>
#include <QProcess>

#define STRMSG_LEN	(256)


C2000outParser::C2000outParser()
{
    line.address = 0;
    line.tag = 0;
    line.cnt = 0;
    for (int i = 0; i < C_LineDataSize; i++)
        line.data[i] = 0;
}

char* C2000outParser::getVersion()
{
    return "1.0.0.0";
}

char* C2000outParser::getFormatName()
{
    return C_C2000OUT;
}

bool C2000outParser::ConfirmFormat(QString& filename)
{

    QFileInfo file(filename);
    if (file.suffix().compare("out"))
        return false;
    
    QString tmpHex = "%1/tools/thirdparty/tmp1234.hex";
    tmpHex = tmpHex.arg(QApplication::applicationDirPath());
    QString errStr;
    bool Ret = ExecHexC2000Tool(filename, tmpHex, errStr);
    if (Ret) {///生成HexParser成功
        return true;
    }
    QFile::remove(tmpHex);
    return false;
}


bool C2000outParser::ExecHexC2000Tool(const QString& strCurFile, const QString& OutFile, QString& errStr)
{
    QProcess mProcess;
    QStringList arguments;
    arguments << "-i"
        << "-romwidth" << "16"
        << strCurFile
        << "-o"
        << OutFile;

    QString program = "%1/tools/thirdparty/hex2000.exe";
    program = program.arg(QApplication::applicationDirPath());

    mProcess.start(program, arguments);

    if (!mProcess.waitForStarted()) {
        errStr = mProcess.errorString();
        return false;
    }

    if (!mProcess.waitForFinished()) {
        errStr = mProcess.errorString();
        return false;
    }

    QByteArray standardOutput = mProcess.readAllStandardOutput();
    QByteArray standardError = mProcess.readAllStandardError();

    qDebug() << "Standard output:" << standardOutput;
    qDebug() << "Standard error:" << standardError;

    int exitCode = mProcess.exitCode();
    if (exitCode != 0) {
        qDebug() << "Process exited with non-zero exit code:" << exitCode;
    }
    else {
        qDebug() << "Process exited successfully.";
    }

    mProcess.waitForFinished(10000);

    if (!QFile(OutFile).exists())
        errStr = "cxec hex2000.exe error";
    return QFile(OutFile).exists();

    //QProcess mProcess;
    //QStringList paramList;
    //paramList << "-i" << "-romwidth" << "16" << QString("\"%1\"").arg(strCurFile) << "-o" << QString("\"%1\"").arg(OutFile);

    //mProcess.start(QString("\"%1\"").arg(QString("%1/tools/thirdparty/hex2000.exe").arg(QApplication::applicationDirPath())), paramList);
    //bool errFlag = false;
    //QObject::connect(&mProcess, &QProcess::errorOccurred, [](QProcess::ProcessError error) {
    //    qDebug() << "Error occurred:" << error;
    //    });
    //QObject::connect(&mProcess, &QProcess::readyReadStandardOutput, [&]() {
    //        QString tmpStr = mProcess.readAllStandardOutput();
    //        if (STRMSG_LEN > tmpStr.length()) {
    //            if (tmpStr.contains("error")) {
    //                errFlag = true;
    //                errStr = tmpStr;
    //            }
    //        }
    //    });
    //// 连接 readyReadStandardError() 信号来接收标准错误输出的数据
    //QObject::connect(&mProcess, &QProcess::readyReadStandardError, [&]() {
    //        QString tmpStr = mProcess.readAllStandardOutput();
    //        if (STRMSG_LEN > tmpStr.length()) {
    //            if (tmpStr.contains("error")) {
    //                errFlag = true;
    //                errStr = tmpStr;
    //            }
    //        }
    //        qDebug() << "process recv: " << tmpStr;
    //    });

    //while (1) {
    //    if (!mProcess.waitForReadyRead(10000) || errFlag) {
    //        break;
    //    }
    //}

    //mProcess.waitForFinished();

    //return QFile(OutFile).exists();
}

int C2000outParser::TransferFile(QString& srcfn, QIODevice* dst)
{
    m_Dst = dst;
    QString tmpHex = "%1/tools/thirdparty/tmpHex.hex";
    tmpHex = tmpHex.arg(QApplication::applicationDirPath());
    
    QString errStr;
    QString i10_path = srcfn;
    i10_path = i10_path.replace(".out", ".i10");
    AbstractParser* pHexParser = NULL;
    if (ExecHexC2000Tool(srcfn, tmpHex, errStr)) {
        QPluginLoader pluginloader(QApplication::applicationDirPath() + "/parser/Hex.dll");
        QObject* plugin = pluginloader.instance();
        if (plugin) // DLL 成功載入
        {
            pHexParser = qobject_cast<AbstractParser*>(plugin);
            if (!pHexParser)
            {
                return -1;
            }
            pHexParser->m_pOutput = m_pOutput;
            QFile hexFile(tmpHex), i10File(i10_path);
            // 尝试打开文件，创建文件，如果它不存在
            if (!hexFile.open(QIODevice::ReadWrite | QIODevice::Append) || !i10File.open(QIODevice::ReadWrite | QIODevice::Append)) {

                return -1;
            }
            pHexParser->TransferFile(tmpHex, &i10File);

            pHexParser->TransferFile(i10_path, dst);
            hexFile.close();
            i10File.close();

            QFile::remove(tmpHex);
        }
        else
            qDebug() << pluginloader.errorString();
    }
    else
        return -1;
    if (pHexParser) {
        delete pHexParser;
    }

    return 0;
}
