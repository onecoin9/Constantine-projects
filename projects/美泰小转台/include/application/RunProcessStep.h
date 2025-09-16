#ifndef APPLICATION_RUNPROCESSSTEP_H
#define APPLICATION_RUNPROCESSSTEP_H

#include "application/IWorkflowStep.h"
#include <QProcess>
#include <QJsonObject>

namespace Application {

/**
 * @brief RunProcessStep
 * 运行外部可执行程序（exe），并根据标准输出/错误输出或退出码判定成功与否。
 *
 * 配置示例：
 * {
 *   "id": "run_chip_test",
 *   "type": "RunProcess",
 *   "name": "运行ChipTest_all并判定结果",
 *   "config": {
 *     "executable": "C:/Users/pc/Downloads/turnable/ChipTest_all.exe",
 *     "args": ["${global.archiveBaseDir}"],
 *     "workingDirectory": "C:/Users/pc/Downloads/turnable",
 *     "timeoutMs": 600000,
 *     "successPatterns": ["OK"],
 *     "failurePatterns": ["FAILED"],
 *     "expectedExitCode": 0,
 *     "exitCodeAsSuccess": false
 *   }
 * }
 */
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


