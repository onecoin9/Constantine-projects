#include "application/ParseJsonFileStep.h"
#include "application/WorkflowContext.h"
#include "core/Logger.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

namespace Application {

ParseJsonFileStep::ParseJsonFileStep(const QJsonObject &config, QObject *parent) : IWorkflowStep(parent)
{
    setConfiguration(config);
}

ParseJsonFileStep::~ParseJsonFileStep() = default;

bool ParseJsonFileStep::execute(std::shared_ptr<WorkflowContext> context)
{
    m_status = StepStatus::Running;
    QJsonObject config = m_config["config"].toObject();
    QString filePath = config["filePath"].toString();
    QString contextKey = config["contextKey"].toString();

    if (filePath.isEmpty() || contextKey.isEmpty()) {
        LOG_MODULE_ERROR("ParseJsonFileStep", "'filePath' and 'contextKey' must be specified in config.");
        m_status = StepStatus::Failed;
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_MODULE_ERROR("ParseJsonFileStep", QString("Failed to open file: %1").arg(filePath).toStdString());
        m_status = StepStatus::Failed;
        return false;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull() || !doc.isObject()) {
        LOG_MODULE_ERROR("ParseJsonFileStep", QString("Failed to parse JSON from file: %1. Content might be invalid.").arg(filePath).toStdString());
        m_status = StepStatus::Failed;
        return false;
    }

    context->setJsonData(contextKey, doc.object());
    LOG_MODULE_INFO("ParseJsonFileStep", QString("Successfully parsed '%1' and stored it in context with key '%2'.").arg(filePath, contextKey).toStdString());
    
    m_status = StepStatus::Completed;
    m_result["success"] = true;
    m_result["filePath"] = filePath;
    m_result["contextKey"] = contextKey;
    return true;
}

QString ParseJsonFileStep::getName() const { return m_config.value("name").toString("ParseJsonFileStep"); }
QString ParseJsonFileStep::getDescription() const { return "Loads a JSON file and stores its content in the workflow context."; }
bool ParseJsonFileStep::canExecute(std::shared_ptr<WorkflowContext>) const { return true; }
void ParseJsonFileStep::setConfiguration(const QJsonObject& config) { m_config = config; }
QJsonObject ParseJsonFileStep::getConfiguration() const { return m_config; }
QJsonObject ParseJsonFileStep::getResult() const { return m_result; }
IWorkflowStep::StepStatus ParseJsonFileStep::getStatus() const { return m_status; }

} // namespace Application 