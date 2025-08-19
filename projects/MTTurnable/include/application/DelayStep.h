#ifndef DELAYSTEP_H
#define DELAYSTEP_H

#include "application/IWorkflowStep.h"
#include <QJsonObject>

namespace Application {

class WorkflowContext;

/**
 * @brief 延迟步骤
 * 在工作流中引入一个指定的延迟。
 */
class DelayStep : public IWorkflowStep {
public:
    explicit DelayStep(const QJsonObject &config, QObject *parent = nullptr);
    ~DelayStep() override;

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
    QJsonObject m_config;
    int m_delayMs; // Delay in milliseconds
};

} // namespace Application

#endif // DELAYSTEP_H
