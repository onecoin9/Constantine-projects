#ifndef LOOPSTEP_H
#define LOOPSTEP_H

#include "application/IWorkflowStep.h"
#include <QJsonObject>
#include <QList>
#include <memory>

// Forward declaration
namespace Services {
class WorkflowManager;
}

namespace Application {

class LoopStep : public IWorkflowStep {
public:
    // 禁用拷贝构造和赋值操作，防止浅拷贝问题
    LoopStep(const LoopStep&) = delete;
    LoopStep& operator=(const LoopStep&) = delete;
    LoopStep(const QJsonObject &config, QObject *parent = nullptr);
    ~LoopStep() override;

    bool execute(std::shared_ptr<WorkflowContext> context) override;
    QString getName() const override;
    QString getDescription() const override;
    StepStatus getStatus() const override;
    QJsonObject getResult() const override;
    void setConfiguration(const QJsonObject &config) override;
    QJsonObject getConfiguration() const override;
    bool canExecute(std::shared_ptr<WorkflowContext> context) const override;

    // New methods/members for manager-driven execution
    int getLoopCount(std::shared_ptr<WorkflowContext> context);
    const QList<std::shared_ptr<IWorkflowStep>>& getLoopSteps() const;

private:
    QJsonObject m_config;
    QString m_name;
    int m_loopCount;
    int m_currentIteration;
    QList<QJsonObject> m_subStepsConfig;
    QList<std::shared_ptr<IWorkflowStep>> m_subSteps;
    QList<std::shared_ptr<IWorkflowStep>> m_steps;
};

} // namespace Application

#endif // LOOPSTEP_H
