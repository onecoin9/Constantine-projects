#include "infrastructure/BinLogger.h"
#include "GlobalItem.h"
#include "core/Logger.h"
#include <QDir>
#include <QMutexLocker>
#include <QFileInfo>

namespace Infrastructure {

BinLogger::BinLogger(QObject *parent)
    : QObject(parent)
{
}

BinLogger::~BinLogger()
{
    if (m_logFile.isOpen()) {
        stopLogging();
    }
}

bool BinLogger::startLogging(const QString &directory)
{
    return startLogging(directory, QString());
}

bool BinLogger::startLogging(const QString &directory, const QString &customFileName)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_logFile.isOpen()) {
        LOG_MODULE_INFO("BinLogger", QString("Stopping current logging session before starting new one. Current file: %1").arg(m_currentFilePath).toStdString());
        stopLoggingInternal();
    }

    m_logDirectory = directory;
    QDir dir(m_logDirectory);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            LOG_MODULE_ERROR("BinLogger", QString("Failed to create cache directory: %1").arg(m_logDirectory).toStdString());
            return false;
        }
    }

    m_startTime = QDateTime::currentDateTime();
    
    QString tempBaseName = customFileName;
    if (tempBaseName.isEmpty()) {
        tempBaseName = m_startTime.toString("yyyy-MM-dd_HH-mm-ss-zzz");
    }
    
    m_currentFilePath = QString("%1/%2_temp.bin")
        .arg(m_logDirectory)
        .arg(tempBaseName);

    m_logFile.setFileName(m_currentFilePath);

    if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        LOG_MODULE_ERROR("BinLogger", QString("Failed to open bin log file: %1. Error: %2")
            .arg(m_currentFilePath).arg(m_logFile.errorString()).toStdString());
        return false;
    }

    m_logStream.setDevice(&m_logFile);
    LOG_MODULE_INFO("BinLogger", QString("Started logging raw data to: %1").arg(m_currentFilePath).toStdString());
    return true;
}

void BinLogger::stopLogging()
{
    QMutexLocker locker(&m_mutex);
    stopLoggingInternal();
}

void BinLogger::stopLoggingInternal()
{
    if (!m_logFile.isOpen()) {
        return;
    }

    m_logFile.close();

    QFileInfo fileInfo(m_currentFilePath);
    QString baseName = fileInfo.baseName();
    
    QString customPart = baseName;
    if (customPart.endsWith("_temp")) {
        customPart.chop(5); // removes "_temp"
    }

    QDateTime endTime = QDateTime::currentDateTime();
    qint64 diffSecs = m_startTime.secsTo(endTime);  // 秒差


    int binIndex = GlobalItem::getInstance().getInt("binIndex", 1);
    GlobalItem::getInstance().setInt("binIndex", binIndex + 1);

    QString finalFilePath = QString("%1/%4_%2_%3S.bin")
        .arg(m_logDirectory)
        .arg(customPart)
        .arg(diffSecs)
        .arg(binIndex);

    if (QFile::rename(m_currentFilePath, finalFilePath)) {
        LOG_MODULE_INFO("BinLogger", QString("Stopped logging. Final log file: %1").arg(finalFilePath).toStdString());
    } else {
        LOG_MODULE_ERROR("BinLogger", QString("Failed to rename log file from %1 to %2")
            .arg(m_currentFilePath).arg(finalFilePath).toStdString());
    }
    
    m_currentFilePath.clear();
}

void BinLogger::onRawPacketReceived(const QByteArray& packet)
{
    QMutexLocker locker(&m_mutex);
    if (m_logFile.isOpen()) {
        // 为了性能，直接写入QByteArray
        // Qt的int64足以表示长度
        qint64 dataSize = packet.size();
        m_logStream.writeRawData(reinterpret_cast<const char*>(&dataSize), sizeof(qint64));
        m_logStream.writeRawData(packet.constData(), dataSize);
        
        // 新增：强制将缓冲区数据写入文件
        m_logFile.flush();
    }
}

} // namespace Infrastructure 