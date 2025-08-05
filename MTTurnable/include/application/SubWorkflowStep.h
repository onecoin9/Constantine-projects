#pragma once

#include "application/IWorkflowStep.h"
#include <memory>
#include <QObject>
#include <QJsonObject>

namespace Application {

class SubWorkflowStep : public IWorkflowStep {
    Q_OBJECT
public:
    explicit SubWorkflowStep(const QJsonObject &config, QObject *parent = nullptr);
    ~SubWorkflowStep() override;

    // IWorkflowStep interface
    bool execute(std::shared_ptr<WorkflowContext> context) override;
    bool canExecute(std::shared_ptr<WorkflowContext> context) const override;

    QString getName() const override;
    QString getDescription() const override;
    QJsonObject getConfiguration() const override;
    QJsonObject getResult() const override;
    StepStatus getStatus() const override;

    void setConfiguration(const QJsonObject &config) override;

private:
    bool executeSubWorkflowSync(std::shared_ptr<WorkflowContext> parentCtx);

    mutable QJsonObject m_config;
    QJsonObject m_result;
    StepStatus m_status { StepStatus::NotStarted };
};

} // namespace Application 