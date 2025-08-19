#include "application/SetDutStateStep.h"
#include "application/WorkflowContext.h"
#include "services/DutManager.h"
#include "domain/Dut.h"
#include "core/Logger.h"
#include "application/WorkflowUtils.h"
#include <QMetaEnum>

namespace Application {

SetDutStateStep::SetDutStateStep(const QJsonObject &config, QObject *parent) : IWorkflowStep(parent)
{
    setConfiguration(config);
}

SetDutStateStep::~SetDutStateStep() = default;

bool SetDutStateStep::execute(std::shared_ptr<WorkflowContext> context)
{
    m_status = StepStatus::Running;

    QJsonObject config = m_config.value("config").toObject();
    QString dutId = WorkflowUtils::replacePlaceholders(config["dutId"].toString(), context);
    QString stateStr = WorkflowUtils::replacePlaceholders(config["state"].toString(), context);

    if (dutId.isEmpty()) {
        LOG_MODULE_ERROR("SetDutStateStep", "DUT ID is not specified or resolved.");
        m_status = StepStatus::Failed;
        return false;
    }

    auto dutManager = context->getDutManager();
    if (!dutManager) {
        LOG_MODULE_ERROR("SetDutStateStep", "DutManager is not available in the context.");
        m_status = StepStatus::Failed;
        return false;
    }

    // Convert string to Dut::State enum
    bool ok;
    Domain::Dut::State newState = static_cast<Domain::Dut::State>(stateStr.toInt(&ok));
    if (!ok) {
        // Fallback for string-based state names, requires a map or if-else chain
        QMetaEnum metaEnum = QMetaEnum::fromType<Domain::Dut::State>();
        newState = static_cast<Domain::Dut::State>(metaEnum.keyToValue(stateStr.toUtf8()));
        if(metaEnum.keyToValue(stateStr.toUtf8()) == -1)
        {
             LOG_MODULE_ERROR("SetDutStateStep", QString("Invalid state string: %1").arg(stateStr).toStdString());
             m_status = StepStatus::Failed;
             return false;
        }
    }
    
    LOG_MODULE_INFO("SetDutStateStep", QString("Setting state of DUT '%1' to '%2'").arg(dutId).arg(stateStr).toStdString());
    if (dutManager->setState(dutId, newState)) {
        m_status = StepStatus::Completed;
        return true;
    } else {
        LOG_MODULE_ERROR("SetDutStateStep", QString("Failed to set state for DUT '%1'.").arg(dutId).toStdString());
        m_status = StepStatus::Failed;
        return false;
    }
}

QString SetDutStateStep::getName() const { return m_config["name"].toString("SetDutStateStep"); }
QString SetDutStateStep::getDescription() const { return "Sets the state of a specified DUT."; }
IWorkflowStep::StepStatus SetDutStateStep::getStatus() const { return m_status; }
QJsonObject SetDutStateStep::getResult() const { return m_result; }
void SetDutStateStep::setConfiguration(const QJsonObject &config) { m_config = config; }
QJsonObject SetDutStateStep::getConfiguration() const { return m_config; }
bool SetDutStateStep::canExecute(std::shared_ptr<WorkflowContext> context) const { return context && context->getDutManager(); }

} // namespace Application 