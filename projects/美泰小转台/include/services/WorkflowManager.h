#ifndef WORKFLOWMANAGER_H
#define WORKFLOWMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QVector>
#include <QList>
#include <memory>
#include <QMap>
#include <stack>
#include <QSharedPointer>
#include <QDateTime>
#include <QMutex>

namespace Application {
    class IWorkflowStep;
    class WorkflowContext;
}

namespace Services {
    class DeviceManager;
    class DutManager;


/**
 * @brief 工作流管理器 (WorkflowManager)
 * @class WorkflowManager
 * 
 * @details
 * WorkflowManager 是整个自动化框架的流程控制核心。它的主要职责是：
 * 1.  **加载和管理工作流模板**：从JSON文件中加载工作流的定义，并将其解析为一系列可执行的步骤 (IWorkflowStep)。
 * 2.  **实例化和执行工作流**：当外部事件 (如 CoreEngine 的信号) 触发时，根据模板创建一个独立的工作流实例，并为其分配一个唯一的上下文 (WorkflowContext)。
 * 3.  **驱动步骤执行**：以递归的方式执行工作流中的每一个步骤。它能够处理复杂的流程控制，如：
 *     - **顺序执行**: 依次执行列表中的步骤。
 *     - **条件分支 (ConditionalStep)**: 根据条件评估结果，选择性地执行 'then' 或 'else' 子流程。
 *     - **循环 (LoopStep)**: 重复执行一组子步骤，直到满足循环次数或退出条件。
 * 4.  **并发管理**: 能够同时管理和执行多个来自不同触发源的工作流实例，每个实例都有其独立的状态和数据上下文。
 * 5.  **提供同步执行接口**: 为嵌套步骤 (如 ConditionalStep) 提供同步执行其子步骤列表的能力。
 * 
 * @note
 * 本管理器采用异步启动 (`startWorkflow`) + 同步执行 (`executeStepList`) 的模型。
 * `startWorkflow` 会在一个新的线程中启动工作流，以避免阻塞主线程。
 * `executeStepList` 是一个阻塞的、递归的函数，负责驱动整个或部分工作流的执行。
 */
class WorkflowManager : public QObject, public std::enable_shared_from_this<WorkflowManager>
{
    Q_OBJECT
public:
    /**
     * @brief 工作流实例的运行状态
     */
    enum class WorkflowStatus {
        NotStarted, ///< 尚未开始
        Running,    ///< 正在运行
        Paused,     ///< 已暂停
        Completed,  ///< 成功完成
        Failed,     ///< 执行失败
        Cancelled   ///< 已取消
    };

    /**
     * @brief 构造函数
     * @param deviceManager 设备管理器实例
     * @param dutManager DUT管理器实例
     * @param parent 父QObject
     */
    explicit WorkflowManager(
        std::shared_ptr<Services::DeviceManager> deviceManager,
        Services::DutManager* dutManager,
        QObject *parent = nullptr
    );

    /**
     * @brief 析构函数
     */
    ~WorkflowManager();

    // --- 工作流模板管理 ---

    /**
     * @brief 从JSON配置加载一个工作流模板
     * @param workflowId 模板的唯一ID (通常是文件路径)
     * @param workflowConfig JSON配置对象
     * @return bool 是否加载成功
     */
    bool loadWorkflow(const QString &workflowId, const QJsonObject &workflowConfig);

    /**
     * @brief 卸载一个已加载的工作流模板
     * @param workflowId 要卸载的模板ID
     * @return bool 是否卸载成功
     */
    bool unloadWorkflow(const QString &workflowId);

    /**
     * @brief 获取所有已加载工作流模板的ID列表
     * @return QStringList 模板ID列表
     */
    QStringList getLoadedWorkflows() const;

    /**
     * @brief 获取所有正在运行的工作流实例的ID列表
     * @return QStringList 实例ID列表
     */
    QStringList getRunningInstanceIds() const;
    
    // --- 工作流实例执行与控制 ---

    /**
     * @brief 异步启动一个工作流实例
     * 
     * 该函数会立即返回一个实例ID，并在一个新线程中开始执行工作流。
     * @param workflowId 要实例化的工作流模板ID
     * @param context (可选) 外部传入的初始上下文，若为nullptr则创建一个新的
     * @return QString 启动的实例ID，若失败则为空字符串
     */
    QString startWorkflow(const QString &workflowId, std::shared_ptr<Application::WorkflowContext> context = nullptr);

    /**
     * @brief 同步执行一个工作流
     * 
     * 此函数会阻塞，直到工作流执行完成或失败。主要用于嵌套步骤的实现。
     * @param workflowId 要执行的工作流模板ID
     * @param context 工作流上下文
     * @return bool 是否执行成功
     */
    bool executeWorkflowSync(const QString &workflowId, std::shared_ptr<Application::WorkflowContext> context);

    /**
     * @brief 暂停一个正在运行的工作流实例
     * @param instanceId 实例ID
     * @return bool 是否成功发送暂停指令
     */
    bool pauseWorkflow(const QString &instanceId);

    /**
     * @brief 恢复一个已暂停的工作流实例
     * @param instanceId 实例ID
     * @return bool 是否成功恢复
     */
    bool resumeWorkflow(const QString &instanceId);

    /**
     * @brief 停止(取消)一个正在运行或暂停的工作流实例
     * @param instanceId 实例ID
     * @return bool 是否成功发送停止指令
     */
    bool stopWorkflow(const QString &instanceId);
    
    // --- 状态查询 ---

    /**
     * @brief 获取指定工作流实例的当前状态
     * @param instanceId 实例ID
     * @return WorkflowStatus 状态枚举
     */
    WorkflowStatus getWorkflowStatus(const QString &instanceId) const;

    /**
     * @brief 获取实例当前执行到的顶层步骤索引
     * @param instanceId 实例ID
     * @return int 步骤索引, 若实例不存在则为-1
     */
    int getCurrentStepIndex(const QString &instanceId) const;

    /**
     * @brief 获取实例的顶层步骤总数
     * @param instanceId 实例ID
     * @return int 步骤总数
     */
    int getTotalSteps(const QString &instanceId) const;

    /**
     * @brief 获取指定模板中某一步骤的名称
     * @param workflowId 模板ID
     * @param stepIndex 步骤索引
     * @return QString 步骤名称
     */
    QString getStepName(const QString &workflowId, int stepIndex) const;
    
    // --- 上下文与步骤工厂 ---

    /**
     * @brief 获取指定工作流实例的上下文对象
     * @param instanceId 实例ID
     * @return std::shared_ptr<Application::WorkflowContext> 上下文指针，若实例不存在则为nullptr
     */
    std::shared_ptr<Application::WorkflowContext> getWorkflowContext(const QString &instanceId) const;
    
    /**
     * @brief 根据JSON配置创建具体的步骤实例 (工厂方法)
     * @param stepConfig 单个步骤的JSON配置
     * @return std::shared_ptr<Application::IWorkflowStep> 步骤实例指针，若类型未知则为nullptr
     */
    std::shared_ptr<Application::IWorkflowStep> createStep(const QJsonObject &stepConfig);

signals:
    // --- 对外信号 ---

    /**
     * @brief 当一个工作流实例开始时发射
     * @param instanceId 实例ID
     */
    void workflowStarted(const QString &instanceId);

    /**
     * @brief 当一个工作流实例成功完成时发射
     * @param instanceId 实例ID
     */
    void workflowCompleted(const QString &instanceId);

    /**
     * @brief 当一个工作流实例执行失败时发射
     * @param instanceId 实例ID
     * @param error 错误信息
     */
    void workflowFailed(const QString &instanceId, const QString &error);

    /**
     * @brief 当一个工作流实例被取消时发射
     * @param instanceId 实例ID
     */
    void workflowCancelled(const QString &instanceId);

    /**
     * @brief 当一个步骤开始执行时发射
     * @param instanceId 实例ID
     * @param stepIndex 步骤在当前列表中的索引
     * @param stepName 步骤名称
     */
    void stepStarted(const QString &instanceId, int stepIndex, const QString &stepName);

    /**
     * @brief 当一个步骤成功完成时发射
     * @param instanceId 实例ID
     * @param stepIndex 步骤索引
     * @param stepName 步骤名称
     */
    void stepCompleted(const QString &instanceId, int stepIndex, const QString &stepName);

    /**
     * @brief 当一个步骤执行失败时发射
     * @param instanceId 实例ID
     * @param stepIndex 步骤索引
     * @param stepName 步骤名称
     * @param error 错误信息
     */
    void stepFailed(const QString &instanceId, int stepIndex, const QString &stepName, const QString &error);

    /**
     * @brief 顶层进度的更新信号
     * @param instanceId 实例ID
     * @param percentage 完成百分比 (0-100)
     */
    void progressUpdated(const QString &instanceId, int percentage);

private:
    /**
     * @brief 工作流实例的内部表示
     */
    struct WorkflowInstance {
        QString instanceId; ///< 唯一的实例ID
        QString workflowId; ///< 模板ID
        QString name;       ///< 工作流名称
        QList<std::shared_ptr<Application::IWorkflowStep>> steps; ///< 顶层步骤列表
        std::shared_ptr<Application::WorkflowContext> context; ///< 数据上下文
        WorkflowStatus status; ///< 当前状态
        int currentStepIndex;  ///< 当前执行到的顶层步骤索引
    };

    /**
     * @brief 异步执行工作流的入口函数 (在单独线程中运行)
     * @param instanceId 要执行的实例ID
     */
    void executeWorkflow(const QString &instanceId);

    /**
     * @brief 递归地执行一个步骤列表
     * 
     * 这是流程控制的核心。它会遍历步骤列表，并特殊处理ConditionalStep和LoopStep，
     * 通过递归调用自身来执行子步骤。
     * @param instance 工作流实例的引用
     * @param steps 要执行的步骤列表
     * @return bool 列表中的所有步骤是否都成功完成
     */
    bool executeStepList(WorkflowInstance& instance, const QList<std::shared_ptr<Application::IWorkflowStep>>& steps);
    
private:
    std::shared_ptr<Services::DeviceManager> m_deviceManager; ///< 设备管理器依赖
    Services::DutManager* m_dutManager;       ///< DUT管理器依赖
    QMap<QString, WorkflowInstance> m_runningInstances;       ///< 正在运行的实例集合
    QMap<QString, WorkflowInstance> m_loadedWorkflows;        ///< 已加载的模板集合
    mutable QMutex m_mutex; // <-- 2. 添加互斥锁
};
} // namespace Services

#endif // WORKFLOWMANAGER_H 
