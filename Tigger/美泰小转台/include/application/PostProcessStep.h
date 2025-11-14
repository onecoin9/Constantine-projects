#ifndef POSTPROCESSSTEP_H
#define POSTPROCESSSTEP_H

#include "application/IWorkflowStep.h"
#include <memory>
#include <QDir>
namespace Application
{
class WorkflowContext;

class PostProcessStep : public IWorkflowStep
{
public:
    // 禁用拷贝构造和赋值操作，防止浅拷贝问题
    PostProcessStep(const PostProcessStep&) = delete;
    PostProcessStep& operator=(const PostProcessStep&) = delete;
    Q_OBJECT

public:
    explicit PostProcessStep(const QJsonObject &config, QObject *parent = nullptr);
    ~PostProcessStep() override;

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
    bool copyDirectory(const QDir& fromDir, const QDir& toDir, bool recursively);

    QJsonObject m_config;
    StepStatus m_status = StepStatus::NotStarted;
    QJsonObject m_result;
};

} // namespace Application

#endif // POSTPROCESSSTEP_H 
