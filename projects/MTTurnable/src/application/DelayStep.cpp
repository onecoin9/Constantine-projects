#include "application/DelayStep.h"
#include "application/WorkflowContext.h"
#include <QThread> // For QThread::msleep
#include <QDebug>

namespace Application {

DelayStep::DelayStep(const QJsonObject &config, QObject *parent)
    : IWorkflowStep(parent) { // Call base constructor
    setConfiguration(config);
}

DelayStep::~DelayStep() {
    // Destructor implementation, if needed
}

bool DelayStep::execute(std::shared_ptr<WorkflowContext> context) {
    QJsonObject config = m_config.value("config").toObject();
    // 支持多种配置键名，优先使用delayMs，兼容durationMs
    int delayMs = config.value("delayMs").toInt(
        config.value("durationMs").toInt(
            config.value("duration").toInt(0)
        )
    );

    if (!context) {
        qWarning() << getName() << ": WorkflowContext is null.";
        return false;
    }
    context->log(getName() + ": Starting delay for " + QString::number(delayMs) + " ms.");

    if (delayMs > 0) {
        QThread::msleep(delayMs);
    } else {
        qWarning() << getName() << ": Warning - No valid delay configuration found. Expected 'delayMs', 'durationMs', or 'duration' in config.";
    }
    context->log(getName() + ": Delay finished.");
    return true;
}

QString DelayStep::getName() const {
    return m_config.value("name").toString("DelayStep");
}

QString DelayStep::getDescription() const {
    return m_config.value("description").toString("Pauses workflow execution for a specified duration.");
}

IWorkflowStep::StepStatus DelayStep::getStatus() const {
    return m_status;
}

QJsonObject DelayStep::getResult() const {
    QJsonObject config = m_config.value("config").toObject();
    // 支持多种配置键名，保持与execute方法一致
    int delayMs = config.value("delayMs").toInt(
        config.value("durationMs").toInt(
            config.value("duration").toInt(0)
        )
    );
    
    QJsonObject result;
    result["delayMs"] = delayMs;
    return result;
}

void DelayStep::setConfiguration(const QJsonObject &config) {
    m_config = config;
}

QJsonObject DelayStep::getConfiguration() const {
    return m_config;
}

bool DelayStep::canExecute(std::shared_ptr<WorkflowContext> context) const {
    return true;
}

} // namespace Application
