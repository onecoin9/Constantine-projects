#include "application/PostProcessStep.h"
#include "application/WorkflowContext.h"
#include "core/Logger.h"
#include "GlobalItem.h"

#include <QDir>
#include <QDateTime>

namespace Application
{

/**
 * @brief PostProcessStep 构造函数
 * @param config JSON配置对象
 * @param parent 父对象
 *
 * 这个类实现了一个通用的后处理工作流步骤，其主要功能有两个：
 * 1.  参数设置：从JSON配置的 "setParameters" 字段中读取键值对，
 *     并将其设置到工作流的上下文中。这允许动态地为后续流程配置参数，
 *     例如设置当前测试的芯片类型、文件路径等。
 *
 * 2.  文件归档：处理 "archive" 字段中的配置，执行文件操作。
 *     - sourceDirectory: 指定要处理的源文件夹（例如 "output"）。
 *     - archiveRoot: 指定归档操作的根目录。
 *     - archiveBy: 指定用于创建第一级子目录的参数名（例如 "chipType"），
 *       该参数的值会从工作流上下文中获取。
 *     - testType: 测试类型（例如 "calibration"），用于创建测试类型-站点ID文件夹。
 *     - siteId: 站点ID（例如 "B01"），与testType组合创建文件夹。
 *     - operation: "copy" 或 "move"，决定是复制还是剪切文件。
 *     
 * 文件夹结构: archiveRoot/chipType/timestamp/testType-siteId/文件名文件夹/文件名
 *
 * 完整JSON配置示例:
 * {
 *   "id": "post_process",
 *   "name": "后处理与归档",
 *   "type": "PostProcess",
 *   "config": {
 *     "setParameters": {
 *       "chipType": "270",
 *       "somePath": "C:/new/path/for/something.json"
 *     },
 *     "archive": {
 *       "sourceDirectory": "output",
 *       "archiveRoot": "D:/TestArchive",
 *       "archiveBy": "chipType",
 *       "testType": "calibration",
 *       "siteId": "B01",
 *       "operation": "move"
 *     }
 *   }
 * }
 */
PostProcessStep::PostProcessStep(const QJsonObject &config, QObject *parent)
    : IWorkflowStep(parent)
{
    setConfiguration(config);
}

PostProcessStep::~PostProcessStep() = default;

bool PostProcessStep::execute(std::shared_ptr<WorkflowContext> context)
{
    m_status = IWorkflowStep::StepStatus::Running;
    emit statusChanged(m_status);

    QJsonObject config = m_config.value("config").toObject();

    // 1. 处理参数设置
    if (config.contains("setParameters")) {
        QJsonObject paramsToSet = config.value("setParameters").toObject();
        for (const QString &key : paramsToSet.keys()) {
            context->setData(key, paramsToSet.value(key).toVariant());
            LOG_MODULE_INFO("PostProcessStep", QString("设置参数 '%1' = '%2'").arg(key).arg(paramsToSet.value(key).toString()).toStdString());
        }
    }

    // 2. 处理文件归档
    if (config.contains("archive")) {
        GlobalItem::getInstance().setInt("binIndex", 1);

        QJsonObject archiveConfig = config.value("archive").toObject();
        QString sourcePath = archiveConfig.value("sourceDirectory").toString("output");
        QString archiveRoot = archiveConfig.value("archiveRoot").toString();
        QString archiveBy = archiveConfig.value("archiveBy").toString();
        QString operation = archiveConfig.value("operation").toString("copy");

        if (archiveRoot.isEmpty() || archiveBy.isEmpty()) {
            m_status = IWorkflowStep::StepStatus::Failed;
            emit statusChanged(m_status);
            emit errorOccurred("归档失败: 'archiveRoot' 或 'archiveBy' 未在配置中指定。");
            return false;
        }

        QString archiveByValue = context->getData(archiveBy).toString();
        if (archiveByValue.isEmpty()) {
            m_status = IWorkflowStep::StepStatus::Failed;
            emit statusChanged(m_status);
            emit errorOccurred(QString("归档失败: 上下文中未找到参数 '%1' 的值。").arg(archiveBy));
            return false;
        }
        
        QDir sourceDir(sourcePath);
        if (!sourceDir.exists()) {
             LOG_MODULE_WARNING("PostProcessStep", QString("源目录 '%1' 不存在，跳过归档。").arg(sourcePath).toStdString());
             m_status = IWorkflowStep::StepStatus::Completed;
             emit statusChanged(m_status);
             return true;
        }

        // 获取testType和siteId参数
        QString testType = archiveConfig.value("testType").toString("test");
        QString siteId = archiveConfig.value("siteId").toString("A01");
        
        // 创建目标路径: archiveRoot/chipType/timestamp/testType-siteId
        QString timestampDirName = QDateTime::currentDateTime().toString("yyyyMMdd_HH_mm_ss");
        QString testTypeSiteDir = QString("%1-%2").arg(testType).arg(siteId);
        QDir baseDestDir(QString("%1/%2/%3/%4").arg(archiveRoot).arg(archiveByValue).arg(timestampDirName).arg(testTypeSiteDir));

        if (!baseDestDir.mkpath(".")) {
            m_status = IWorkflowStep::StepStatus::Failed;
            emit statusChanged(m_status);
            emit errorOccurred(QString("归档失败: 无法创建目标目录 '%1'。").arg(baseDestDir.absolutePath()));
            return false;
        }

        // 保存到全局变量：testType-siteId 这一层目录
        GlobalItem::getInstance().setString("archiveBaseDir", baseDestDir.absolutePath());

        bool success = true;
        if (operation.toLower() == "move") {
            QFileInfoList entries = sourceDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
            for(const QFileInfo& entry : entries) {
                QString newPath = baseDestDir.filePath(entry.fileName());
                if (!QFile::rename(entry.absoluteFilePath(), newPath)) {
                    success = false;
                    emit errorOccurred(QString("移动失败: 从 '%1' 到 '%2'").arg(entry.absoluteFilePath()).arg(newPath));
                    break;
                }
            }
        } else { // copy
            success = copyDirectory(sourceDir, baseDestDir, true);
        }

        if(success) {
             LOG_MODULE_INFO("PostProcessStep", QString("成功归档到 '%1'").arg(baseDestDir.absolutePath()).toStdString());
        } else {
             m_status = IWorkflowStep::StepStatus::Failed;
             emit statusChanged(m_status);
             emit errorOccurred("归档操作中发生错误。");
             return false;
        }
    }

    m_status = IWorkflowStep::StepStatus::Completed;
    emit statusChanged(m_status);
    return true;
}

// 递归复制目录
bool PostProcessStep::copyDirectory(const QDir& fromDir, const QDir& toDir, bool recursively)
{
    if (!toDir.mkpath(toDir.absolutePath()))
        return false;

    QFileInfoList entries = fromDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);

    for (const QFileInfo &entry : entries) {
        QString newPath = toDir.filePath(entry.fileName());
        if (entry.isDir() && recursively) {
            if (!copyDirectory(QDir(entry.filePath()), QDir(newPath), true))
                return false;
        } else if (entry.isFile()) {
            if (!QFile::copy(entry.filePath(), newPath)) {
                return false;
            }
        }
    }
    return true;
}

QString PostProcessStep::getName() const
{
    return m_config.value("name").toString("PostProcessStep");
}

QString PostProcessStep::getDescription() const
{
    return m_config.value("description").toString("执行参数设置和文件归档。");
}

void PostProcessStep::setConfiguration(const QJsonObject& config)
{
    m_config = config;
}

QJsonObject PostProcessStep::getConfiguration() const
{
    return m_config;
}

bool PostProcessStep::canExecute(std::shared_ptr<WorkflowContext>) const
{
    return true;
}

QJsonObject PostProcessStep::getResult() const
{
    return m_result;
}

IWorkflowStep::StepStatus PostProcessStep::getStatus() const
{
    return m_status;
}

bool PostProcessStep::rollback(std::shared_ptr<WorkflowContext>)
{
    return true;
}

} // namespace Application 