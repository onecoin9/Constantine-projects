#ifndef DEVICECOMMANDSTEP_H
#define DEVICECOMMANDSTEP_H

#include "application/IWorkflowStep.h"
#include <QJsonObject>
#include <QString>

namespace Application {

class DeviceCommandStep : public IWorkflowStep {
public:
    // 禁用拷贝构造和赋值操作，防止浅拷贝问题
    DeviceCommandStep(const DeviceCommandStep&) = delete;
    DeviceCommandStep& operator=(const DeviceCommandStep&) = delete;
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
    QJsonObject completeParams(const QString& command, QJsonObject& params);

    // The only state a step should have is its configuration.
    QJsonObject m_config;
};

} // namespace Application

#endif // DEVICECOMMANDSTEP_H
