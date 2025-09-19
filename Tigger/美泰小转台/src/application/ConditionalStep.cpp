#include "application/ConditionalStep.h"
#include "application/WorkflowContext.h"
#include "services/WorkflowManager.h"
#include "core/Logger.h"
#include "application/WorkflowUtils.h"
#include <QJsonDocument>
#include <QJsonArray>

namespace Application {

ConditionalStep::ConditionalStep(const QJsonObject &config, QObject *parent) : IWorkflowStep(parent) {
    setConfiguration(config);
}

ConditionalStep::~ConditionalStep() = default;

// The primary execute method for a ConditionalStep now simply returns true.
// The real logic is handled by WorkflowManager which will call evaluate() and getStepsToExecute().
bool ConditionalStep::execute(std::shared_ptr<WorkflowContext> context) {
    m_status = StepStatus::Completed;
    return true;
}

bool ConditionalStep::evaluate(std::shared_ptr<WorkflowContext> context) {
    m_lastEvaluationResult = evaluateCondition(context);
    LOG_MODULE_INFO("ConditionalStep", QString("Condition for '%1' evaluated to %2.").arg(getName()).arg(m_lastEvaluationResult ? "TRUE" : "FALSE").toStdString());
    return m_lastEvaluationResult;
}

QList<std::shared_ptr<IWorkflowStep>> ConditionalStep::getStepsToExecute() const {
    QList<std::shared_ptr<IWorkflowStep>> steps = m_lastEvaluationResult ? m_thenSteps : m_elseSteps;
    LOG_MODULE_INFO("ConditionalStep", QString("getStepsToExecute: evaluation=%1, returning %2 steps (%3)")
                   .arg(m_lastEvaluationResult ? "TRUE" : "FALSE")
                   .arg(steps.size())
                   .arg(m_lastEvaluationResult ? "then" : "else").toStdString());
    
    for (int i = 0; i < steps.size(); ++i) {
        LOG_MODULE_INFO("ConditionalStep", QString("  Step %1: %2").arg(i).arg(steps[i]->getName()).toStdString());
    }
    
    return steps;
}


bool ConditionalStep::evaluateCondition(std::shared_ptr<WorkflowContext> context) {
    QJsonObject config = m_config["config"].toObject();
    QString conditionStr = config["condition"].toString();
    if (conditionStr.isEmpty()) {
        LOG_MODULE_WARNING("ConditionalStep", "No 'condition' string found in config. Defaulting to FALSE.");
        return false;
    }

    QString processedCondition = WorkflowUtils::replacePlaceholders(conditionStr, context);
    
    QString op;
    if (processedCondition.contains("==")) op = "==";
    else if (processedCondition.contains("!=")) op = "!=";
    else return processedCondition.trimmed().toLower() == "true";

    if (op.isEmpty()) return false;

    QStringList parts = processedCondition.split(op);
    if (parts.size() != 2) return false;

    QString left = parts[0].trimmed();
    QString right = parts[1].trimmed();
    
    if ((right.startsWith("'") && right.endsWith("'")) || (right.startsWith("\"") && right.endsWith("\""))) {
        right = right.mid(1, right.length() - 2);
    }

    if (op == "==") return left.compare(right, Qt::CaseInsensitive) == 0;
    if (op == "!=") return left.compare(right, Qt::CaseInsensitive) != 0;
    
    return false;
}

void ConditionalStep::setConfiguration(const QJsonObject& config) { 
    m_config = config; 
}

bool ConditionalStep::canExecute(std::shared_ptr<WorkflowContext> context) const { 
    if(m_thenSteps.isEmpty() && m_elseSteps.isEmpty()){
        auto manager = context->getWorkflowManager();
        if(!manager) {
            LOG_MODULE_ERROR("ConditionalStep", "WorkflowManager not available");
            return false;
        }

        ConditionalStep* nonConstThis = const_cast<ConditionalStep*>(this);

        QJsonObject configObj = m_config["config"].toObject();
        LOG_MODULE_INFO("ConditionalStep", QString("Parsing sub-steps from config: %1")
                       .arg(QString(QJsonDocument(configObj).toJson(QJsonDocument::Compact))).toStdString());

        QJsonArray thenStepsConfig = configObj["then"].toArray();
        LOG_MODULE_INFO("ConditionalStep", QString("Found %1 'then' steps to parse")
                       .arg(thenStepsConfig.size()).toStdString());
        
        for(const auto& val : thenStepsConfig){
            auto step = manager->createStep(val.toObject());
            if(step) {
                nonConstThis->m_thenSteps.append(step);
                LOG_MODULE_INFO("ConditionalStep", QString("Added 'then' step: %1").arg(step->getName()).toStdString());
            } else {
                LOG_MODULE_ERROR("ConditionalStep", QString("Failed to create 'then' step from config: %1")
                               .arg(QString(QJsonDocument(val.toObject()).toJson(QJsonDocument::Compact))).toStdString());
            }
        }

        QJsonArray elseStepsConfig = configObj["else"].toArray();
        LOG_MODULE_INFO("ConditionalStep", QString("Found %1 'else' steps to parse")
                       .arg(elseStepsConfig.size()).toStdString());
        
        for(const auto& val : elseStepsConfig){
            auto step = manager->createStep(val.toObject());
            if(step) {
                nonConstThis->m_elseSteps.append(step);
                LOG_MODULE_INFO("ConditionalStep", QString("Added 'else' step: %1").arg(step->getName()).toStdString());
            } else {
                LOG_MODULE_ERROR("ConditionalStep", QString("Failed to create 'else' step from config: %1")
                               .arg(QString(QJsonDocument(val.toObject()).toJson(QJsonDocument::Compact))).toStdString());
            }
        }
        
        LOG_MODULE_INFO("ConditionalStep", QString("ConditionalStep parsing complete: %1 'then' steps, %2 'else' steps")
                       .arg(nonConstThis->m_thenSteps.size()).arg(nonConstThis->m_elseSteps.size()).toStdString());
    }
    return true;
}


QString ConditionalStep::getName() const { return m_config.value("name").toString("ConditionalStep"); }
QString ConditionalStep::getDescription() const { return "Conditionally executes a set of sub-steps."; }
QJsonObject ConditionalStep::getConfiguration() const { return m_config; }
QJsonObject ConditionalStep::getResult() const { return m_result; }
IWorkflowStep::StepStatus ConditionalStep::getStatus() const { return m_status; }

} 
