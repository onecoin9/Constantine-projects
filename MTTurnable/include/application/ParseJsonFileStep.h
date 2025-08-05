#ifndef PARSEJSONFILESTEP_H
#define PARSEJSONFILESTEP_H

#include "application/IWorkflowStep.h"

namespace Application {

/**
 * @brief 从文件系统加载一个JSON文件，并将其内容存入工作流上下文中。
 *
 * JSON配置示例:
 * {
 *   "id": "load_test_params",
 *   "name": "加载测试参数",
 *   "type": "ParseJsonFile",
 *   "config": {
 *     "filePath": "config/parameters/test_A300.json",
 *     "contextKey": "currentTestParams"
 *   }
 * }
 */
class ParseJsonFileStep : public IWorkflowStep {
public:
    ParseJsonFileStep(const QJsonObject &config, QObject *parent = nullptr);
    ~ParseJsonFileStep() override;

    bool execute(std::shared_ptr<WorkflowContext> context) override;
    QString getName() const override;
    QString getDescription() const override;
    bool canExecute(std::shared_ptr<WorkflowContext> context) const override;
    void setConfiguration(const QJsonObject& config) override;
    QJsonObject getConfiguration() const override;
    QJsonObject getResult() const override;
    StepStatus getStatus() const override;
};

} // namespace Application

#endif // PARSEJSONFILESTEP_H 