#include "application/ProcessBinFileStep.h"
#include "application/WorkflowContext.h"
#include "domain/protocols/IXTProtocol.h"
#include "core/Logger.h"

#include <QObject>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDataStream>
#include <QRegularExpression>
#include <QFileInfo>
#include <QVariant>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace Application
{

ProcessBinFileStep::ProcessBinFileStep(const QJsonObject &config, QObject *parent)
    : IWorkflowStep(parent)
{
    setConfiguration(config);
}

ProcessBinFileStep::~ProcessBinFileStep() = default;

bool ProcessBinFileStep::execute(std::shared_ptr<WorkflowContext> context)
{
    m_status = IWorkflowStep::StepStatus::Running;
    emit statusChanged(m_status);
    
    QString statusMessage = "开始处理Bin文件...";
    emit logMessage(statusMessage);

    QString cachePath = m_config.value("config").toObject().value("sourceDirectory").toString("cache");
    QString outputRootPath = m_config.value("config").toObject().value("outputDirectory").toString("output");
    
    QDir cacheDir(cachePath);
    if (!cacheDir.exists()) {
        statusMessage = QString("源目录 %1 不存在").arg(cachePath);
        LOG_MODULE_ERROR("ProcessBinFileStep", statusMessage.toStdString());
        m_status = IWorkflowStep::StepStatus::Failed;
        emit statusChanged(m_status);
        emit errorOccurred(statusMessage);
        return false;
    }

    // 新增：创建history目录
    QDir historyDir = cacheDir;
    historyDir.cdUp();
    QString historyPath = historyDir.filePath("history");
    if (!QDir(historyPath).exists()) {
        if (!historyDir.mkpath(historyPath)) {
            statusMessage = QString("创建history目录 %1 失败").arg(historyPath);
            LOG_MODULE_ERROR("ProcessBinFileStep", statusMessage.toStdString());
            m_status = IWorkflowStep::StepStatus::Failed;
            emit statusChanged(m_status);
            emit errorOccurred(statusMessage);
            return false;
        }
    }

    QString timestampStr = context->getData("timestamp").toString();
    QFileInfoList binFiles = cacheDir.entryInfoList(QStringList() << "*.bin", QDir::Files, QDir::SortFlag::Name);


    if (binFiles.isEmpty()) {
        statusMessage = "在cache目录中未找到匹配的bin文件。";
        LOG_MODULE_WARNING("ProcessBinFileStep", statusMessage.toStdString());
        m_status = IWorkflowStep::StepStatus::Completed;
        emit statusChanged(m_status);
        return true;
    }
    
    int filesProcessed = 0;
    for (const QFileInfo &binFileInfo : binFiles) {

        QString binFilePath = binFileInfo.absoluteFilePath();
        QString baseName = binFileInfo.baseName();
        QString outputDirPath = QString("%1/%2").arg(outputRootPath).arg(baseName);
        
        QDir outputDir(outputDirPath);
        if (!outputDir.exists()) {
            if (!outputDir.mkpath(".")) {
                statusMessage = QString("创建目录 %1 失败").arg(outputDirPath);
                LOG_MODULE_ERROR("ProcessBinFileStep", statusMessage.toStdString());
                continue; 
            }
        }

        QFile binFile(binFilePath);
        if (!binFile.open(QIODevice::ReadOnly)) {
            statusMessage = QString("打开bin文件 %1 失败").arg(binFilePath);
            LOG_MODULE_ERROR("ProcessBinFileStep", statusMessage.toStdString());
            continue;
        }
        
        QDataStream in(&binFile);
        
        QVector<QTextStream*> dutStreams;
        QVector<QFile*> dutFiles;
        for (int i = 0; i < 9; ++i) {
            QString dutFileName;
            if(i < 8) {
                dutFileName = QString("%1/dut_%2.txt").arg(outputDirPath).arg(i + 1);
            } else {
                dutFileName = QString("%1/dut_99.txt").arg(outputDirPath);
            }
            QFile *outFile = new QFile(dutFileName);
            if (!outFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
                 statusMessage = QString("创建txt文件 %1 失败").arg(dutFileName);
                 LOG_MODULE_ERROR("ProcessBinFileStep", statusMessage.toStdString());
                 for(auto f : dutFiles) {
                     f->close();
                     delete f;
                 }
                 delete outFile;
                 dutFiles.clear();
                 break;
            }
            dutFiles.append(outFile);
            QTextStream* outStream = new QTextStream(outFile);
            *outStream << "gyro_x\tgyro_y\tgyro_z\tacc_x\tacc_y\tacc_z\ttemperature\tmix\n";
            dutStreams.append(outStream);
        }

        if(dutFiles.isEmpty()) continue;

        while (!in.atEnd()) {
             qint64 dataSize;
             in.readRawData(reinterpret_cast<char*>(&dataSize), sizeof(qint64));
             
             if (in.status() != QDataStream::Ok) break;

             QByteArray packet(dataSize, Qt::Uninitialized);
             int bytesRead = in.readRawData(packet.data(), dataSize);

             if (bytesRead < dataSize) break;
             
             QByteArray payload = packet.mid(8);
             
             if (payload.size() >= 284) { 
                 const char* rawData = payload.constData();
                 Domain::Protocols::TestFeedbackData frame;
                 memcpy(&frame, rawData, sizeof(Domain::Protocols::TestFeedbackData));

                 for (int i = 0; i < 8; ++i) {
                     if((frame.dutActive >> i) & 0x01) {
                        *dutStreams[i] << frame.dutData[i].gyro_x << "\t"
                                       << frame.dutData[i].gyro_y << "\t"
                                       << frame.dutData[i].gyro_z << "\t"
                                       << frame.dutData[i].gyro_acc_x << "\t"
                                       << frame.dutData[i].gyro_acc_y << "\t"
                                       << frame.dutData[i].gyro_acc_z << "\t"
                                       << frame.dutData[i].gyro_temperature << "\t"
                                       << frame.dutData[i].gyro_mix << "\n";
                     }
                 }

                 // 检查外部陀螺仪数据是否非零，作为写入的依据，这比依赖标志位更可靠
                 if (frame.externalGyro.gyro_x != 0 || frame.externalGyro.gyro_y != 0 || frame.externalGyro.gyro_z != 0)
                 {
                    *dutStreams[8] << frame.externalGyro.gyro_x << "\t"
                                   << frame.externalGyro.gyro_y << "\t"
                                   << frame.externalGyro.gyro_z << "\t"
                                   << frame.externalGyro.gyro_acc_x << "\t"
                                   << frame.externalGyro.gyro_acc_y << "\t"
                                   << frame.externalGyro.gyro_acc_z << "\t"
                                   << frame.externalGyro.gyro_temperature << "\t"
                                   << frame.externalGyro.gyro_mix << "\n";
                 }
             }
        }

        for (int i = 0; i < 9; ++i) {
            dutFiles[i]->close();
            delete dutStreams[i];
            delete dutFiles[i];
        }

        binFile.close();

        // 新增：移动已处理的文件到history目录
        QString historyFilePath = QDir(historyPath).filePath(binFileInfo.fileName());
        if (QFile::exists(historyFilePath)) {
            QFile::remove(historyFilePath);
        }
        if (!QFile::rename(binFilePath, historyFilePath)) {
            statusMessage = QString("移动文件 %1 到 %2 失败").arg(binFilePath).arg(historyFilePath);
            LOG_MODULE_ERROR("ProcessBinFileStep", statusMessage.toStdString());
        } else {
            LOG_MODULE_INFO("ProcessBinFileStep", QString("文件 %1 已处理并移动到history目录。").arg(binFileInfo.fileName()).toStdString());
        }

        filesProcessed++;
        emit progressUpdated(static_cast<int>((static_cast<double>(filesProcessed) / binFiles.size()) * 100));
    }
    
    statusMessage = "所有Bin文件处理完毕。";
    m_status = IWorkflowStep::StepStatus::Completed;
    emit statusChanged(m_status);
    emit logMessage(statusMessage);
    return true;
}

QString ProcessBinFileStep::getName() const
{
    return m_config.value("name").toString("ProcessBinFileStep");
}

QString ProcessBinFileStep::getDescription() const
{
    return m_config.value("description").toString("Processes binary log files and generates text files.");
}

void ProcessBinFileStep::setConfiguration(const QJsonObject& config)
{
    m_config = config;
}

QJsonObject ProcessBinFileStep::getConfiguration() const
{
    return m_config;
}

bool ProcessBinFileStep::canExecute(std::shared_ptr<WorkflowContext>) const
{
    return true;
}

QJsonObject ProcessBinFileStep::getResult() const
{
    return m_result;
}

IWorkflowStep::StepStatus ProcessBinFileStep::getStatus() const
{
    return m_status;
}

bool ProcessBinFileStep::rollback(std::shared_ptr<WorkflowContext>)
{
    // 对于此步骤，回滚操作可以是将生成的文件夹删除
    return true;
}


} // namespace Application 