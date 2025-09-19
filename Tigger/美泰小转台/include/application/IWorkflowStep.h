#ifndef IWORKFLOWSTEP_H
#define IWORKFLOWSTEP_H

#include <QObject>
#include <QJsonObject>
#include <QString>
#include <memory>

namespace Application {

// 前向声明
class WorkflowContext;

/**
 * @brief 工作流步骤接口
 * 定义单个操作步骤的接口
 */
class IWorkflowStep : public QObject
{
    Q_OBJECT
public:
    enum class StepStatus {
        NotStarted,
        Running,
        Completed,
        Failed,
        Skipped
    };

    explicit IWorkflowStep(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IWorkflowStep() = default;

    // 执行步骤
    virtual bool execute(std::shared_ptr<WorkflowContext> context) = 0;
    
    // 步骤信息
    virtual QString getName() const = 0;
    virtual QString getDescription() const = 0;
    virtual StepStatus getStatus() const = 0;
    virtual QJsonObject getResult() const = 0;
    
    // 配置
    virtual void setConfiguration(const QJsonObject &config) = 0;
    virtual QJsonObject getConfiguration() const = 0;
    
    // 验证步骤是否可以执行
    virtual bool canExecute(std::shared_ptr<WorkflowContext> context) const = 0;
    
    // 步骤回滚（可选）
    virtual bool rollback(std::shared_ptr<WorkflowContext> context) { return true; }

signals:
    void statusChanged(StepStatus status);
    void progressUpdated(int percentage);
    void logMessage(const QString &message);
    void errorOccurred(const QString &error);

protected:
    StepStatus m_status = StepStatus::NotStarted;
    QJsonObject m_result;
    QJsonObject m_config;
};

} // namespace Application

#endif // IWORKFLOWSTEP_H 