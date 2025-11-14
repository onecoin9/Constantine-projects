#ifndef GLOBALCONFIGSTEP_H
#define GLOBALCONFIGSTEP_H

#include "application/IWorkflowStep.h"

namespace Application {

/**
 * @brief 全局配置步骤
 * 
 * 这个步骤用于设置工作流的全局参数，包括：
 * - cmd3TaskPath: 自动机配置文件路径
 * - taskFileName: 烧录软件配置文件路径
 * - batchNumber: 批次号
 * - productionQuantity: 生产数量
 * - activationParams: 激活参数
 * 
 * 这些参数会被注入到WorkflowContext中，供后续步骤使用
 */
class GlobalConfigStep : public IWorkflowStep
{
public:
    // 禁用拷贝构造和赋值操作，防止浅拷贝问题
    GlobalConfigStep(const GlobalConfigStep&) = delete;
    GlobalConfigStep& operator=(const GlobalConfigStep&) = delete;
    Q_OBJECT

public:
    explicit GlobalConfigStep(const QJsonObject& config, QObject* parent = nullptr);
    ~GlobalConfigStep() override;

    // IWorkflowStep interface
    bool execute(std::shared_ptr<WorkflowContext> context) override;
    QString getName() const override;
    QString getDescription() const override;
    StepStatus getStatus() const override;
    QJsonObject getResult() const override;
    void setConfiguration(const QJsonObject& config) override;
    QJsonObject getConfiguration() const override;
    bool canExecute(std::shared_ptr<WorkflowContext> context) const override;
    bool rollback(std::shared_ptr<WorkflowContext> context) override;

private:
    QJsonObject m_config;
    StepStatus m_status;
    QJsonObject m_result;
};

} // namespace Application

#endif // GLOBALCONFIGSTEP_H 