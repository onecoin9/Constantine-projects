#ifndef SWITCHSTEP_H
#define SWITCHSTEP_H

#include "application/IWorkflowStep.h"
#include <QJsonObject>
#include <QList>
#include <memory>
#include <QVariant>

namespace Application {

class SwitchStep : public IWorkflowStep {
public:
    // 禁用拷贝构造和赋值操作，防止浅拷贝问题
    SwitchStep(const SwitchStep&) = delete;
    SwitchStep& operator=(const SwitchStep&) = delete;
    SwitchStep(const QJsonObject &config, QObject *parent = nullptr);
    ~SwitchStep() override;

    bool execute(std::shared_ptr<WorkflowContext> context) override;
    QString getName() const override;
    QString getDescription() const override;
    StepStatus getStatus() const override;
    QJsonObject getResult() const override;
    void setConfiguration(const QJsonObject &config) override;
    QJsonObject getConfiguration() const override;
    bool canExecute(std::shared_ptr<WorkflowContext> context) const override;
    
    QList<std::shared_ptr<IWorkflowStep>> getStepsToExecute(std::shared_ptr<WorkflowContext> context);

private:
    QJsonObject m_config;
    // The key is the value to match against, stored as a QVariant to preserve its original type.
    QMap<QVariant, QList<std::shared_ptr<IWorkflowStep>>> m_caseSteps;
    QList<std::shared_ptr<IWorkflowStep>> m_defaultSteps;
};

} // namespace Application

#endif // SWITCHSTEP_H 