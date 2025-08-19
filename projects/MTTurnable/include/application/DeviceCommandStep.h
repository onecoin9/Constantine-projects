#ifndef DEVICECOMMANDSTEP_H
#define DEVICECOMMANDSTEP_H

#include "application/IWorkflowStep.h"
#include <QJsonObject>
#include <QString>

namespace Application {

class DeviceCommandStep : public IWorkflowStep {
public:
    DeviceCommandStep(const QJsonObject &config, QObject *parent = nullptr);
    ~DeviceCommandStep() override;

    // IWorkflowStep interface implementation
    bool execute(std::shared_ptr<WorkflowContext> context) override;
    QString getName() const override;
    QString getDescription() const override;
    StepStatus getStatus() const override;
    QJsonObject getResult() const override;
    void setConfiguration(const QJsonObject &config) override;
    QJsonObject getConfiguration() const override;
    bool canExecute(std::shared_ptr<WorkflowContext> context) const override;

private:
    // The only state a step should have is its configuration.
    QJsonObject m_config;
};

} // namespace Application

#endif // DEVICECOMMANDSTEP_H
