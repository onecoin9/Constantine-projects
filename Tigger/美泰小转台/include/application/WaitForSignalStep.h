#ifndef WAITFORSIGNALSTEP_H
#define WAITFORSIGNALSTEP_H

#include "application/IWorkflowStep.h"
#include <QTimer>

// 前向声明
class QMutex;
class QWaitCondition;

namespace Application {

/**
 * @brief 等待一个来自CoreEngine的特定信号。
 *
 * JSON配置示例:
 * {
 *   "type": "WaitForSignal",
 *   "config": {
 *     "signal": "axisMoveCompleted", // 要等待的信号名称
 *     "timeoutMs": 10000,           // 超时时间
 *     "filter": {                   // 用于匹配信号参数的过滤器
 *       "arg0_axisName": "%AXIS_NAME%",
 *       "arg1_siteIndex": "%SITE_INDEX%"
 *     }
 *   }
 * }
 */
class WaitForSignalStep : public IWorkflowStep {
    Q_OBJECT

public:
    WaitForSignalStep(const QJsonObject &config, QObject *parent = nullptr);
    ~WaitForSignalStep() override;
    
    // 禁用拷贝构造和赋值操作，防止浅拷贝问题
    WaitForSignalStep(const WaitForSignalStep&) = delete;
    WaitForSignalStep& operator=(const WaitForSignalStep&) = delete;

    bool execute(std::shared_ptr<WorkflowContext> context) override;
    QString getName() const override;
    QString getDescription() const override;
    bool canExecute(std::shared_ptr<WorkflowContext> context) const override;
    void setConfiguration(const QJsonObject& config) override;
    QJsonObject getConfiguration() const override;
    QJsonObject getResult() const override;
    StepStatus getStatus() const override;

private:
    QTimer* m_timer;
    QMutex* m_mutex;
    QWaitCondition* m_waitCondition;
    bool m_signalReceived;
    bool m_success;
};

} // namespace Application

#endif // WAITFORSIGNALSTEP_H 