#ifndef PROCESSBINFILESTEP_H
#define PROCESSBINFILESTEP_H

#include "application/IWorkflowStep.h"
#include <memory>

namespace Application
{
class WorkflowContext;

class ProcessBinFileStep : public IWorkflowStep
{
public:
    // 禁用拷贝构造和赋值操作，防止浅拷贝问题
    ProcessBinFileStep(const ProcessBinFileStep&) = delete;
    ProcessBinFileStep& operator=(const ProcessBinFileStep&) = delete;
    Q_OBJECT

public:
    explicit ProcessBinFileStep(const QJsonObject &config, QObject *parent = nullptr);
    ~ProcessBinFileStep() override;

    // IWorkflowStep interface
    bool execute(std::shared_ptr<WorkflowContext> context) override;
    
    QString getName() const override;
    QString getDescription() const override;
    StepStatus getStatus() const override;
    QJsonObject getResult() const override;
    void setConfiguration(const QJsonObject &config) override;
    QJsonObject getConfiguration() const override;
    bool canExecute(std::shared_ptr<WorkflowContext> context) const override;
    bool rollback(std::shared_ptr<WorkflowContext> context) override;

private:
    QJsonObject m_config;
    bool m_stopped = false;
    StepStatus m_status = StepStatus::NotStarted;
    QJsonObject m_result;
};

} // namespace Application

#endif // PROCESSBINFILESTEP_H 