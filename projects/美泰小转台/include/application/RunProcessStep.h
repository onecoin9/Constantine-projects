#ifndef APPLICATION_RUNPROCESSSTEP_H
#define APPLICATION_RUNPROCESSSTEP_H

#include "application/IWorkflowStep.h"
#include <QProcess>
#include <QJsonObject>

namespace Application {

class RunProcessStep : public IWorkflowStep {
    Q_OBJECT
public:
    explicit RunProcessStep(const QJsonObject& config, QObject* parent = nullptr);
    ~RunProcessStep() override;

    // IWorkflowStep
    QString getName() const override;
    QString getDescription() const override;
    void setConfiguration(const QJsonObject& config) override;
    QJsonObject getConfiguration() const override;
    bool canExecute(std::shared_ptr<WorkflowContext> context) const override;
    bool execute(std::shared_ptr<WorkflowContext> context) override;
    bool rollback(std::shared_ptr<WorkflowContext> context) override;
    QJsonObject getResult() const override;
    StepStatus getStatus() const override;

private:
    bool matchesAnyPattern(const QString& text, const QStringList& patterns, Qt::CaseSensitivity cs) const;

private:
    QJsonObject m_config;
    QJsonObject m_result;
    StepStatus m_status { StepStatus::NotStarted };
};

} // namespace Application

#endif // APPLICATION_RUNPROCESSSTEP_H


