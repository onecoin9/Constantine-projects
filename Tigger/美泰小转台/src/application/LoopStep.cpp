#include "application/LoopStep.h"
#include "application/WorkflowContext.h"
#include "services/WorkflowManager.h"
#include "application/WorkflowUtils.h"
#include "core/Logger.h"
#include <QJsonArray>
#include <QJsonDocument>

namespace Application {

LoopStep::LoopStep(const QJsonObject &config, QObject *parent) : IWorkflowStep(parent) {
    setConfiguration(config);
}

LoopStep::~LoopStep() = default;

// LoopStep's execute method now does nothing, as the WorkflowManager handles the logic.
bool LoopStep::execute(std::shared_ptr<WorkflowContext> context) {
    m_status = StepStatus::Completed;
    return true;
}

int LoopStep::getLoopCount(std::shared_ptr<WorkflowContext> context) {
    LOG_MODULE_DEBUG("LoopStep", QString("Full m_config: %1").arg(QString(QJsonDocument(m_config).toJson(QJsonDocument::Compact))).toStdString());
    
    QJsonObject config = m_config["config"].toObject();
    LOG_MODULE_DEBUG("LoopStep", QString("Extracted config object: %1").arg(QString(QJsonDocument(config).toJson(QJsonDocument::Compact))).toStdString());
    
    if (config.contains("count")) {
        QString countStr = config["count"].toVariant().toString();
        LOG_MODULE_DEBUG("LoopStep", QString("Original count string: '%1'").arg(countStr).toStdString());
        
        countStr = WorkflowUtils::replacePlaceholders(countStr, context);
        LOG_MODULE_DEBUG("LoopStep", QString("After placeholder replacement: '%1'").arg(countStr).toStdString());
        
        bool ok;
        int count = countStr.toInt(&ok);
        LOG_MODULE_DEBUG("LoopStep", QString("Final count: %1, conversion success: %2").arg(count).arg(ok).toStdString());
        
        return ok ? count : 0;
    }
    LOG_MODULE_DEBUG("LoopStep", "No 'count' field found in config.");
    return 0;
}

const QList<std::shared_ptr<IWorkflowStep>>& LoopStep::getLoopSteps() const {
    return m_steps;
}

void LoopStep::setConfiguration(const QJsonObject& config) {
    m_config = config;
    // The parsing of sub-steps is deferred to `canExecute` where WorkflowManager is available.
}

bool LoopStep::canExecute(std::shared_ptr<WorkflowContext> context) const {
    if (m_steps.isEmpty()) {
        auto manager = context->getWorkflowManager();
        if (!manager) {
            LOG_MODULE_ERROR("LoopStep", "WorkflowManager not available for sub-step parsing.");
            return false;
        }

        LoopStep* nonConstThis = const_cast<LoopStep*>(this);
        QJsonObject config = m_config["config"].toObject();
        QJsonArray stepsConfig = config["steps"].toArray();
        
        LOG_MODULE_DEBUG("LoopStep", QString("Parsing %1 sub-steps...").arg(stepsConfig.size()).toStdString());
        
        for (const auto& val : stepsConfig) {
            auto step = manager->createStep(val.toObject());
            if (step) {
                nonConstThis->m_steps.append(step);
                LOG_MODULE_DEBUG("LoopStep", QString("Added sub-step: %1").arg(step->getName()).toStdString());
            } else {
                LOG_MODULE_ERROR("LoopStep", QString("Failed to create sub-step from config: %1").arg(QString(QJsonDocument(val.toObject()).toJson(QJsonDocument::Compact))).toStdString());
            }
        }
        
        LOG_MODULE_INFO("LoopStep", QString("Successfully parsed %1 sub-steps for loop execution.").arg(m_steps.size()).toStdString());
    }
    return true;
}

QString LoopStep::getName() const { return m_config.value("name").toString("LoopStep"); }
QString LoopStep::getDescription() const { return "Executes a list of sub-steps multiple times."; }
QJsonObject LoopStep::getConfiguration() const { return m_config; }
QJsonObject LoopStep::getResult() const { return m_result; }
IWorkflowStep::StepStatus LoopStep::getStatus() const { return m_status; }

} // namespace Application
