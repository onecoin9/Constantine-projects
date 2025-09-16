#ifndef CONDITIONALSTEP_H
#define CONDITIONALSTEP_H

#include "application/IWorkflowStep.h"
#include <QJsonObject>
#include <QList>
#include <memory>

namespace Application {

/**
 * @brief 根据上下文中的值，有条件地执行一个子工作流。
 *
 * JSON配置示例:
 * {
 *   "id": "check_and_branch",
 *   "name": "检查并分支",
 *   "type": "Conditional",
 *   "config": {
 *     "condition": {
 *       "source": "%some_step_result.success%",
 *       "operator": "equals",
 *       "value": true
 *     },
 *     "on_true": "recipes/path_if_true.json",
 *     "on_false": "recipes/path_if_false.json" // 可选
 *   }
 * }
 */
class ConditionalStep : public IWorkflowStep {
public:
    ConditionalStep(const QJsonObject &config, QObject *parent = nullptr);
    ~ConditionalStep() override;

    bool execute(std::shared_ptr<WorkflowContext> context) override;
    QString getName() const override;
    QString getDescription() const override;
    bool canExecute(std::shared_ptr<WorkflowContext> context) const override;
    void setConfiguration(const QJsonObject& config) override;
    QJsonObject getConfiguration() const override;
    QJsonObject getResult() const override;
    StepStatus getStatus() const override;
    
    // New methods to expose sub-steps to the manager
    bool evaluate(std::shared_ptr<WorkflowContext> context);
    QList<std::shared_ptr<IWorkflowStep>> getStepsToExecute() const;

private:
    bool evaluateCondition(std::shared_ptr<WorkflowContext> context);

    QJsonObject m_config;
    QList<std::shared_ptr<IWorkflowStep>> m_thenSteps;
    QList<std::shared_ptr<IWorkflowStep>> m_elseSteps;
    bool m_lastEvaluationResult = false;
};

} // namespace Application

#endif // CONDITIONALSTEP_H 