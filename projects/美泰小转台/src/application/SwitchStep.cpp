#include "application/SwitchStep.h"
#include "application/WorkflowContext.h"
#include "services/WorkflowManager.h"
#include "application/WorkflowUtils.h"
#include <QJsonArray>
#include <QVariant>
#include <QJsonDocument>
#include "core/Logger.h"

namespace Application {

SwitchStep::SwitchStep(const QJsonObject &config, QObject *parent) : IWorkflowStep(parent) {
    // Pass the full step object, not just the 'config' sub-object
    setConfiguration(config);
}

SwitchStep::~SwitchStep() = default;

// The execute method is a no-op, control is passed to the manager.
bool SwitchStep::execute(std::shared_ptr<WorkflowContext> context) {
    m_status = StepStatus::Completed;
    return true;
}

QList<std::shared_ptr<IWorkflowStep>> SwitchStep::getStepsToExecute(std::shared_ptr<WorkflowContext> context)
{
    // 若尚未解析子步骤列表，先调用 canExecute() 触发解析。
    if (m_caseSteps.isEmpty() && m_defaultSteps.isEmpty()) {
        // canExecute 会在内部构建 m_caseSteps / m_defaultSteps。
        // 这里忽略返回值，因为解析失败在后续也会走默认分支。
        canExecute(context);
    }

    // The "config" property from the JSON is where our settings are.
    QJsonObject config = m_config["config"].toObject();

    QString sourceKey = config["source"].toString(); // e.g., "${context.power_on_result.success}"
    if(sourceKey.isEmpty()){
        LOG_MODULE_ERROR("SwitchStep", "'source' key is missing in config.");
        return m_defaultSteps;
    }
    
    // Extract the actual key from the placeholder syntax, e.g., "context.power_on_result.success"
    QString placeholderKey = sourceKey;
    if (placeholderKey.startsWith("${") && placeholderKey.endsWith("}")) {
        placeholderKey = placeholderKey.mid(2, placeholderKey.length() - 3);
    }

    QVariant sourceValue = WorkflowUtils::resolvePlaceholder(placeholderKey, context);
    
    LOG_MODULE_INFO("SwitchStep", QString("Evaluating switch on source '%1', resolved value: '%2' (Type: %3)").arg(sourceKey).arg(sourceValue.toString()).arg(sourceValue.typeName()).toStdString());

    if(m_caseSteps.contains(sourceValue)){
        LOG_MODULE_INFO("SwitchStep", QString("Match found for value '%1'. Executing corresponding case.").arg(sourceValue.toString()).toStdString());
        return m_caseSteps.value(sourceValue);
    } else {
        LOG_MODULE_INFO("SwitchStep", QString("No specific case matched for value '%1'. Executing default case.").arg(sourceValue.toString()).toStdString());
        return m_defaultSteps;
    }
}


void SwitchStep::setConfiguration(const QJsonObject& config) {
    m_config = config;
}

bool SwitchStep::canExecute(std::shared_ptr<WorkflowContext> context) const {
    if (m_caseSteps.isEmpty() && m_defaultSteps.isEmpty()) {
        auto manager = context->getWorkflowManager();
        if (!manager) {
            LOG_MODULE_ERROR("SwitchStep", "Cannot create sub-steps because WorkflowManager is not available.");
            return false;
        }

        SwitchStep* nonConstThis = const_cast<SwitchStep*>(this);
        QJsonObject config = m_config["config"].toObject();
        QJsonArray cases = config["cases"].toArray();

        for (const auto& caseVal : cases) {
            QJsonObject caseObj = caseVal.toObject();
            QJsonArray stepsConfig = caseObj["steps"].toArray();
            QList<std::shared_ptr<IWorkflowStep>> stepList;

            for (const auto& stepVal : stepsConfig) {
                auto step = manager->createStep(stepVal.toObject());
                if (step) {
                    stepList.append(step);
                }
            }
            
            if(caseObj.contains("equals")){
                 nonConstThis->m_caseSteps.insert(caseObj["equals"].toVariant(), stepList);
            } else if(caseObj.contains("default")){
                 nonConstThis->m_defaultSteps = stepList;
            }
        }
    }
    return true;
}

QString SwitchStep::getName() const { return m_config.value("name").toString("SwitchStep"); }
QString SwitchStep::getDescription() const { return "Executes a specific set of sub-steps based on a source value."; }
QJsonObject SwitchStep::getConfiguration() const { return m_config; }
QJsonObject SwitchStep::getResult() const { return m_result; }
IWorkflowStep::StepStatus SwitchStep::getStatus() const { return m_status; }

} // namespace Application 
