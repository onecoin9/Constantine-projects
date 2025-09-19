#ifndef BINLOGGER_H
#define BINLOGGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDataStream>
#include <QDateTime>
#include <QMutex>

namespace Infrastructure {

/**
 * @brief 负责将原始数据包写入二进制文件的类
 */
class BinLogger : public QObject
{
    Q_OBJECT
public:
    explicit BinLogger(QObject *parent = nullptr);
    ~BinLogger();

public slots:
    /**
     * @brief 开始记录日志
     * @param directory 日志文件存放的目录
     * @return 如果成功打开文件并开始记录，则返回true
     */
    bool startLogging(const QString& directory);
    
    // 开始记录到指定目录，使用自定义文件名
    bool startLogging(const QString& directory, const QString& customFileName);

    /**
     * @brief 停止记录日志
     * 会将文件重命名为包含起止时间的格式
     */
    void stopLogging();

    /**
     * @brief 接收并写入一个原始数据包
     * @param packet 要写入的原始数据包
     */
    void onRawPacketReceived(const QByteArray& packet);

private:
    void stopLoggingInternal();
    
    QFile m_logFile;
    QDataStream m_logStream;
    QMutex m_mutex;

    QString m_logDirectory;
    QString m_currentFilePath;
    QDateTime m_startTime;
};

} // namespace Infrastructure

#endif // BINLOGGER_H 