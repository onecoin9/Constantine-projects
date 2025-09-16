#include "services/WorkflowManager.h"
#include "services/DeviceManager.h"
#include "services/DutManager.h"
#include "application/IWorkflowStep.h"
#include "application/WorkflowContext.h"
#include "application/DeviceCommandStep.h"
#include "application/LoopStep.h"
#include "application/DelayStep.h"
#include "application/WaitForSignalStep.h"
#include "application/ParseJsonFileStep.h"
#include "application/ConditionalStep.h"
#include "application/SwitchStep.h"
#include "application/SubWorkflowStep.h"
#include "application/ProcessBinFileStep.h"
#include "application/PostProcessStep.h"
#include "application/RunProcessStep.h"
#include "application/GlobalConfigStep.h"
#include "core/Logger.h"

#include <QJsonArray>
#include <QThread>
#include <QMetaObject>
#include <QMutexLocker>

namespace Services {

WorkflowManager::WorkflowManager(
    std::shared_ptr<DeviceManager> deviceManager,
    DutManager* dutManager,
    QObject *parent)
    : QObject(parent)
    , m_deviceManager(deviceManager)
    , m_dutManager(dutManager)
{
    LOG_MODULE_DEBUG("WorkflowManager", "WorkflowManager constructed.");
}

WorkflowManager::~WorkflowManager()
{
    // 停止所有正在运行的工作流
    for (const auto &instanceId : m_runningInstances.keys()) {
        stopWorkflow(instanceId);
    }
}

bool WorkflowManager::loadWorkflow(const QString &workflowId, const QJsonObject &workflowConfig)
{
    QMutexLocker locker(&m_mutex);
    if (workflowId.isEmpty() || workflowConfig.isEmpty()) {
        LOG_MODULE_ERROR("WorkflowManager", "加载工作流失败：ID或配置为空。");
        return false;
    }
    
    if (m_loadedWorkflows.contains(workflowId)) {
        LOG_MODULE_WARNING("WorkflowManager", QString("工作流模板 '%1' 已加载，无需重复加载。").arg(workflowId).toStdString());
        return true; 
    }
    
    WorkflowInstance workflow;
    workflow.workflowId = workflowId;
    workflow.name = workflowConfig["name"].toString("未命名工作流");
    workflow.status = WorkflowStatus::NotStarted;
    workflow.currentStepIndex = -1;
    
    // 加载步骤
    QJsonArray stepsConfig = workflowConfig["steps"].toArray();
    for (const QJsonValue &stepValue : stepsConfig) {
        QJsonObject stepConfig = stepValue.toObject();
        auto step = createStep(stepConfig);
        
        if (!step) {
            LOG_MODULE_ERROR("WorkflowManager", QString("在模板 '%1' 中创建步骤 '%2' 失败。").arg(workflowId).arg(stepConfig["name"].toString()).toStdString());
            return false;
        }
        
        workflow.steps.append(step);
    }
    
    if (workflow.steps.isEmpty()) {
        LOG_MODULE_WARNING("WorkflowManager", QString("工作流模板 '%1' 不包含任何步骤。").arg(workflowId).toStdString());
        return false;
    }
    
    m_loadedWorkflows[workflowId] = workflow;
    LOG_MODULE_INFO("WorkflowManager", QString("工作流模板 '%1' 加载成功，包含 %2 个步骤。").arg(workflowId).arg(workflow.steps.size()).toStdString());
    
    return true;
}

bool WorkflowManager::unloadWorkflow(const QString &workflowId)
{
    QMutexLocker locker(&m_mutex);
    if (!m_loadedWorkflows.contains(workflowId)) {
        LOG_MODULE_WARNING("WorkflowManager", QString("工作流模板 '%1' 不存在，无法卸载。").arg(workflowId).toStdString());
        return false;
    }
    
    // 检查是否有基于此模板的实例在运行
    for(const auto& instance : m_runningInstances) {
        if (instance.workflowId == workflowId) {
            LOG_MODULE_ERROR("WorkflowManager", QString("无法卸载模板 '%1'，因为实例 '%2' 正在运行。").arg(workflowId).arg(instance.instanceId).toStdString());
        return false;
        }
    }
    
    m_loadedWorkflows.remove(workflowId);
    LOG_MODULE_INFO("WorkflowManager", QString("工作流模板 '%1' 已卸载。").arg(workflowId).toStdString());
    
    return true;
}

QStringList WorkflowManager::getLoadedWorkflows() const
{
    QMutexLocker locker(&m_mutex);
    return m_loadedWorkflows.keys();
}

QStringList WorkflowManager::getRunningInstanceIds() const
{
    QMutexLocker locker(&m_mutex);
    return m_runningInstances.keys();
}

QString WorkflowManager::startWorkflow(const QString &workflowId, std::shared_ptr<Application::WorkflowContext> context)
{
    QMutexLocker locker(&m_mutex);
    if (!m_loadedWorkflows.contains(workflowId)) {
        LOG_MODULE_ERROR("WorkflowManager", QString("启动工作流失败：模板 '%1' 未加载。").arg(workflowId).toStdString());
        return QString();
    }
    
    // 从模板创建一个新的实例
    WorkflowInstance instance = m_loadedWorkflows[workflowId];

    // 如果未提供上下文，则创建一个新的
    if (!context) {
        context = std::make_shared<Application::WorkflowContext>();
    }
    // 注入框架核心服务的依赖到上下文中，供所有步骤使用
    context->setWorkflowId(workflowId);
    context->setWorkflowName(instance.name);
    context->setDeviceManager(m_deviceManager);
    context->setDutManager(m_dutManager);
    context->setWorkflowManager(shared_from_this());
    instance.context = context;
    
    // 生成一个人类可读且唯一的实例ID
    QString dutId = context->getData("dutId").toString();
    QString baseName = dutId.isEmpty() ? workflowId : QString("%1_on_%2").arg(workflowId).arg(dutId);
    instance.instanceId = QString("%1_%2").arg(baseName).arg(QDateTime::currentMSecsSinceEpoch());

    if (m_runningInstances.contains(instance.instanceId)) {
        LOG_MODULE_ERROR("WorkflowManager", QString("启动工作流失败：实例ID '%1' 已存在。").arg(instance.instanceId).toStdString());
        return QString();
    }
    
    instance.status = WorkflowStatus::Running;
    instance.currentStepIndex = 0;
    
    m_runningInstances.insert(instance.instanceId, instance);
    
    emit workflowStarted(instance.instanceId);
    LOG_MODULE_INFO("WorkflowManager", QString("在新线程中启动工作流实例: '%1' (模板: '%2')").arg(instance.instanceId).arg(workflowId).toStdString());

    // 使用QThread在新线程中执行工作流，避免阻塞主线程
    QThread::create([this, instanceId = instance.instanceId]() {
        executeWorkflow(instanceId);
    })->start();
    
    return instance.instanceId;
}

bool WorkflowManager::executeWorkflowSync(const QString &workflowId, std::shared_ptr<Application::WorkflowContext> context)
{
    if (!m_loadedWorkflows.contains(workflowId)) {
        LOG_MODULE_ERROR("WorkflowManager", QString("同步执行失败: 工作流模板 '%1' 未找到。").arg(workflowId).toStdString());
        return false;
    }

    // 从模板创建一个临时实例用于本次同步执行
    WorkflowInstance instance = m_loadedWorkflows[workflowId];
    instance.context = context;
    instance.status = WorkflowStatus::Running;
    instance.currentStepIndex = 0;
    
    LOG_MODULE_INFO("WorkflowManager", QString("开始同步执行工作流 '%1'").arg(workflowId).toStdString());

    // 直接调用步骤列表执行函数
    bool success = executeStepList(instance, instance.steps);

    LOG_MODULE_INFO("WorkflowManager", QString("工作流 '%1' 同步执行完成，结果: %2。").arg(workflowId).arg(success ? "成功" : "失败").toStdString());
    return success;
}

bool WorkflowManager::pauseWorkflow(const QString &instanceId)
{
    if (!m_runningInstances.contains(instanceId)) {
        return false;
    }
    
    WorkflowInstance &instance = m_runningInstances[instanceId];
    
    if (instance.status == WorkflowStatus::Running) {
        instance.status = WorkflowStatus::Paused;
        LOG_MODULE_INFO("WorkflowManager", QString("工作流实例 '%1' 已暂停。").arg(instanceId).toStdString());
        return true;
    }
    
    return false;
}

bool WorkflowManager::resumeWorkflow(const QString &instanceId)
{
    if (!m_runningInstances.contains(instanceId)) {
        return false;
    }
    
    WorkflowInstance &instance = m_runningInstances[instanceId];
    
    if (instance.status == WorkflowStatus::Paused) {
        instance.status = WorkflowStatus::Running;
        
        // 恢复执行时，需要重新在一个线程中启动执行循环
        // 注意：这里的实现假设暂停前的状态可以无缝衔接。
        QThread::create([this, instanceId]() {
            executeWorkflow(instanceId);
        })->start();
        
        LOG_MODULE_INFO("WorkflowManager", QString("工作流实例 '%1' 已恢复。").arg(instanceId).toStdString());
        return true;
    }
    
    return false;
}

bool WorkflowManager::stopWorkflow(const QString &instanceId)
{
    if (!m_runningInstances.contains(instanceId)) {
        return false;
    }
    
    WorkflowInstance &instance = m_runningInstances[instanceId];
    
    if (instance.status == WorkflowStatus::Running || instance.status == WorkflowStatus::Paused) {
        instance.status = WorkflowStatus::Cancelled;
        emit workflowCancelled(instance.instanceId);
        LOG_MODULE_INFO("WorkflowManager", QString("工作流实例 '%1' 已被取消。").arg(instanceId).toStdString());
        // 实际的清理将在 executeStepList 循环检查到状态变化后进行
        return true;
    }
    
    return false;
}

WorkflowManager::WorkflowStatus WorkflowManager::getWorkflowStatus(const QString &instanceId) const
{
    if (!m_runningInstances.contains(instanceId)) {
        // 如果不在运行实例中，可能已经完成或失败，这里可以考虑查询历史记录
        return WorkflowStatus::NotStarted;
    }
    
    return m_runningInstances[instanceId].status;
}

int WorkflowManager::getCurrentStepIndex(const QString &instanceId) const
{
    if (!m_runningInstances.contains(instanceId)) {
        return -1;
    }
    
    return m_runningInstances[instanceId].currentStepIndex;
}

int WorkflowManager::getTotalSteps(const QString &instanceId) const
{
    if (!m_runningInstances.contains(instanceId)) {
        return 0;
    }
    
    const auto& instance = m_runningInstances[instanceId];
    if (m_loadedWorkflows.contains(instance.workflowId)) {
        return m_loadedWorkflows[instance.workflowId].steps.size();
    }
    return 0;
}

QString WorkflowManager::getStepName(const QString &workflowId, int stepIndex) const
{
    if (!m_loadedWorkflows.contains(workflowId)) {
        return QString();
    }
    
    const WorkflowInstance &workflow = m_loadedWorkflows[workflowId];
    if (stepIndex < 0 || stepIndex >= workflow.steps.size()) {
        return QString();
    }
    
    auto step = workflow.steps[stepIndex];
    if (step) {
        return step->getName();
    }
    
    return QString("步骤 %1").arg(stepIndex + 1);
}

std::shared_ptr<Application::WorkflowContext> WorkflowManager::getWorkflowContext(const QString &instanceId) const
{
    if (!m_runningInstances.contains(instanceId)) {
        return nullptr;
    }
    
    return m_runningInstances[instanceId].context;
}

std::shared_ptr<Application::IWorkflowStep> WorkflowManager::createStep(const QJsonObject &stepConfig)
{
    QString stepType = stepConfig["type"].toString();
    std::shared_ptr<Application::IWorkflowStep> step = nullptr;

    LOG_MODULE_DEBUG("WorkflowManager", QString("正在创建步骤，类型: '%1', 名称: '%2'").arg(stepType).arg(stepConfig["name"].toString()).toStdString());

    if (stepType == "DeviceCommand") {
        step = std::make_shared<Application::DeviceCommandStep>(stepConfig);
    } else if (stepType == "Loop") {
        step = std::make_shared<Application::LoopStep>(stepConfig);
    } else if (stepType == "Delay") {
        step = std::make_shared<Application::DelayStep>(stepConfig);
    } else if (stepType == "WaitForSignal") {
        step = std::make_shared<Application::WaitForSignalStep>(stepConfig);
    } else if (stepType == "ParseJsonFile") {
        step = std::make_shared<Application::ParseJsonFileStep>(stepConfig);
    } else if (stepType == "Conditional") {
        step = std::make_shared<Application::ConditionalStep>(stepConfig);
    }else if (stepType == "Switch") {
        step = std::make_shared<Application::SwitchStep>(stepConfig);
    } else if (stepType == "SubWorkflow") {
        step = std::make_shared<Application::SubWorkflowStep>(stepConfig);
    } else if (stepType == "ProcessBinFile") {
        step = std::make_shared<Application::ProcessBinFileStep>(stepConfig);
    } else if (stepType == "PostProcess") {
        step = std::make_shared<Application::PostProcessStep>(stepConfig);
    } else if (stepType == "RunProcess") {
        step = std::make_shared<Application::RunProcessStep>(stepConfig);
    } else if (stepType == "GlobalConfig") {
        step = std::make_shared<Application::GlobalConfigStep>(stepConfig);
    } else {
        LOG_MODULE_ERROR("WorkflowManager", QString("未知的步骤类型: '%1' (来自步骤 '%2')").arg(stepType).arg(stepConfig["name"].toString()).toStdString());
        return nullptr;
    }

    if (!step) {
        LOG_MODULE_ERROR("WorkflowManager", QString("创建步骤对象失败，类型: '%1', 名称: '%2'").arg(stepType).arg(stepConfig["name"].toString()).toStdString());
    }

    return step;
}

void WorkflowManager::executeWorkflow(const QString &instanceId)
{
    WorkflowInstance *instance = nullptr;
    
    // 临界区：获取实例指针
    {
        QMutexLocker locker(&m_mutex);
        if (!m_runningInstances.contains(instanceId)) {
            LOG_MODULE_ERROR("WorkflowManager", QString("executeWorkflow: 实例 '%1' 未找到。").arg(instanceId).toStdString());
            return;
        }
        
        // 获取实例指针，但不持有锁执行工作流
        instance = &m_runningInstances[instanceId];
    } // 锁在这里释放
    
    // 核心执行逻辑现在委托给可递归的 executeStepList
    // 重要：在没有持有锁的情况下执行，避免死锁
    bool success = executeStepList(*instance, instance->steps);

    // 临界区：更新最终状态
    {
        QMutexLocker locker(&m_mutex);
        // 重新验证实例是否仍然存在（可能在执行过程中被取消）
        if (!m_runningInstances.contains(instanceId)) {
            LOG_MODULE_WARNING("WorkflowManager", QString("executeWorkflow: 实例 '%1' 在执行过程中被移除。").arg(instanceId).toStdString());
            return;
        }
        
        // 更新实例状态
        instance = &m_runningInstances[instanceId];
        
        if (instance->status == WorkflowStatus::Cancelled) {
            // 如果是被取消的，即使上一步返回true，最终状态也应是Cancelled
            LOG_MODULE_INFO("WorkflowManager", QString("工作流 '%1' 已被取消，执行终止。").arg(instanceId).toStdString());
        } else if (success) {
            instance->status = WorkflowStatus::Completed;
            emit progressUpdated(instance->instanceId, 100);
            emit workflowCompleted(instance->instanceId);
            LOG_MODULE_INFO("WorkflowManager", QString("工作流实例 '%1' 已成功完成。").arg(instanceId).toStdString());
        } else {
            // executeStepList 已经在失败点发射了 stepFailed / workflowFailed 信号，这里仍需更新状态与日志。
            instance->status = WorkflowStatus::Failed;
            LOG_MODULE_ERROR("WorkflowManager", QString("工作流实例 '%1' 因步骤执行失败而终止。").arg(instanceId).toStdString());
        }
    } // 锁在这里释放
    
    // 延迟移除实例：
    // 若直接在此线程（工作线程）中移除，框架会立刻删除上下文，
    // 而 workflowCompleted/workflowFailed 等信号通常通过跨线程的 QueuedConnection
    // 投递到主线程。主线程中的槽（如 CoreEngine::handleWorkflowCompleted）
    // 在收到信号时若尝试取上下文，就会发现已经被删除，导致 "Cannot find context" 的错误。
    // 因此，这里改为在对象所属线程（通常为主线程）排队执行移除操作，
    // 保证移除动作发生在所有槽函数处理完毕之后。
    QMetaObject::invokeMethod(this, [this, instanceId]() {
        QMutexLocker locker(&m_mutex);
        m_runningInstances.remove(instanceId);
        LOG_MODULE_INFO("WorkflowManager", QString("工作流实例 '%1' 执行完毕并已移除。").arg(instanceId).toStdString());
    }, Qt::QueuedConnection);
}

bool WorkflowManager::executeStepList(WorkflowInstance& instance, const QList<std::shared_ptr<Application::IWorkflowStep>>& steps)
{
    for (int i = 0; i < steps.size(); ++i) {
        // 更新顶层步骤索引，用于外部查询进度
        // 注意: 这是一个简化处理，在深度嵌套时可能不完全精确反映子步骤进度，但能表示顶层进度
        if (steps == instance.steps) { // 只更新顶层列表的索引
            instance.currentStepIndex = i;
        }

        auto step = steps[i];
        if (!step) {
            emit workflowFailed(instance.instanceId, "工作流执行失败：遇到一个空的步骤对象。");
            return false;
        }

        QString stepName = step->getName();
        QString stepType = step->metaObject()->className();
        LOG_MODULE_INFO("WorkflowManager", QString("实例 '%1' -> 步骤开始: [%2] %3 (Type: %4)").arg(instance.instanceId).arg(i+1).arg(stepName).arg(stepType).toStdString());
        
        emit stepStarted(instance.instanceId, i, stepName);

        // --- 核心调度逻辑：识别并特殊处理结构化步骤 ---
        
        // 1. 处理条件步骤 (ConditionalStep)
        if (auto conditionalStep = std::dynamic_pointer_cast<Application::ConditionalStep>(step)) {
            LOG_MODULE_INFO("WorkflowManager", QString("Processing ConditionalStep: %1").arg(stepName).toStdString());
            
            // 确保子步骤已解析
            if (!conditionalStep->canExecute(instance.context)) {
                QString errorMsg = QString("ConditionalStep '%1' canExecute failed").arg(stepName);
                LOG_MODULE_ERROR("WorkflowManager", errorMsg.toStdString());
                emit stepFailed(instance.instanceId, i, stepName, errorMsg);
                emit workflowFailed(instance.instanceId, errorMsg);
                return false;
            }
            
            if (conditionalStep->evaluate(instance.context)) {
                // 条件为真，递归执行 'then' 子步骤列表
                auto subSteps = conditionalStep->getStepsToExecute();
                LOG_MODULE_INFO("WorkflowManager", QString("ConditionalStep evaluated TRUE, executing %1 'then' steps").arg(subSteps.size()).toStdString());
                if (!executeStepList(instance, subSteps)) {
                    return false; // 如果子流程失败，则整个流程失败
                }
            } else {
                // 条件为假，递归执行 'else' 子步骤列表 (如果存在)
                auto subSteps = conditionalStep->getStepsToExecute();
                LOG_MODULE_INFO("WorkflowManager", QString("ConditionalStep evaluated FALSE, executing %1 'else' steps").arg(subSteps.size()).toStdString());
                if (!executeStepList(instance, subSteps)) {
                    return false;
                }
            }
            emit stepCompleted(instance.instanceId, i, stepName);
            continue; // 继续执行当前列表的下一个步骤
        }

        // 2. Handle SwitchStep
        if (auto switchStep = std::dynamic_pointer_cast<Application::SwitchStep>(step)) {
            if (!executeStepList(instance, switchStep->getStepsToExecute(instance.context))) {
                return false; // Propagate failure
            }
            emit stepCompleted(instance.instanceId, i, stepName);
            continue;
        }

        // 3. 处理循环步骤 (LoopStep)
        if (auto loopStep = std::dynamic_pointer_cast<Application::LoopStep>(step)) {
            // 在执行循环前，必须先调用canExecute来解析子步骤！
            if (!loopStep->canExecute(instance.context)) {
                QString errorMsg = QString("无法初始化LoopStep '%1'的子步骤。").arg(stepName);
                emit stepFailed(instance.instanceId, i, stepName, errorMsg);
                emit workflowFailed(instance.instanceId, errorMsg);
                return false;
            }

            int loopCount = loopStep->getLoopCount(instance.context);
            const auto& loopSteps = loopStep->getLoopSteps();
            LOG_MODULE_INFO("WorkflowManager", QString("Entering LoopStep '%1', executing %2 time(s) with %3 sub-steps.")
                .arg(stepName).arg(loopCount).arg(loopSteps.size()).toStdString());
            for (int j = 0; j < loopCount; ++j) {
                // 为循环设置上下文变量，方便子步骤引用
                instance.context->setData("loop.index", j);
                instance.context->setData("loop.count", loopCount);
                instance.context->setData("loop.isFirst", j == 0);
                instance.context->setData("loop.isLast", j == loopCount - 1);
                
                LOG_MODULE_INFO("WorkflowManager", QString("Loop '%1': Iteration %2/%3").arg(stepName).arg(j + 1).arg(loopCount).toStdString());
                if (!executeStepList(instance, loopSteps)) {
                    return false; // 如果任何一次循环失败，则整个循环步骤失败
                }
                // 检查在两次循环之间是否收到了取消或暂停信号
                {
                    QMutexLocker locker(&m_mutex);
                    // 重新查找实例，因为可能在循环过程中被修改
                    if (!m_runningInstances.contains(instance.instanceId)) {
                        LOG_MODULE_INFO("WorkflowManager", QString("Loop '%1': 实例已被移除，终止循环").arg(stepName).toStdString());
                        return false;
                    }
                    if (m_runningInstances[instance.instanceId].status != WorkflowStatus::Running) {
                        LOG_MODULE_INFO("WorkflowManager", QString("Loop '%1': 状态不再是Running，终止循环").arg(stepName).toStdString());
                        break;
                    }
                }
            }
            emit stepCompleted(instance.instanceId, i, stepName);
            continue;
        }
        
        // --- 执行常规的、非结构化的步骤 ---
        bool success = false;
        int maxRetries = step->getConfiguration().value("retries").toInt(0);
        int retryDelayMs = step->getConfiguration().value("retryDelayMs").toInt(0);

        for (int j = 0; j <= maxRetries; ++j) {
            if (j > 0) {
                LOG_MODULE_WARNING("WorkflowManager", QString("步骤 '%1' (实例: %2) 第 %3 次执行失败。正在重试 (%4/%5)...")
                    .arg(stepName).arg(instance.instanceId).arg(j).arg(j).arg(maxRetries).toStdString());
                if (retryDelayMs > 0) QThread::msleep(retryDelayMs);
            }
            
            // 检查步骤是否可执行
            if (step->canExecute(instance.context)) {
                success = step->execute(instance.context);
            } else {
                 LOG_MODULE_WARNING("WorkflowManager", QString("步骤 '%1' 前置条件不满足，跳过执行。").arg(stepName).toStdString());
                 success = false; // canExecute返回false通常意味着失败或跳过，这里定义为失败
            }

            if (success) break; // 成功则退出重试循环
            
            // 如果在重试间隔中工作流被外部取消，则立即停止重试
            {
                QMutexLocker locker(&m_mutex);
                // 重新查找实例，因为可能在重试过程中被修改
                if (!m_runningInstances.contains(instance.instanceId)) {
                    LOG_MODULE_INFO("WorkflowManager", QString("因工作流实例被移除，停止重试步骤 '%1'。").arg(stepName).toStdString());
                    return false;
                }
                if (m_runningInstances[instance.instanceId].status != WorkflowStatus::Running) {
                    LOG_MODULE_INFO("WorkflowManager", QString("因工作流被取消，停止重试步骤 '%1'。").arg(stepName).toStdString());
                    return false; 
                }
            }
        }

        if (success) {
            emit stepCompleted(instance.instanceId, i, stepName);
        } else {
            // 步骤执行失败，并已用尽重试次数
            QString errorMsg = QString("步骤 '%1' 执行失败。").arg(stepName);
            emit stepFailed(instance.instanceId, i, stepName, errorMsg);
            emit workflowFailed(instance.instanceId, errorMsg); // 整个工作流也标记为失败
            return false; // 终止整个工作流
        }

        // --- 步骤间检查：处理暂停和取消 ---
        // 只有在实例有有效ID时才进行状态检查（排除同步执行的临时实例）
        if (!instance.instanceId.isEmpty()) {
            while (true) {
                QMutexLocker locker(&m_mutex);
                // 重新查找实例，因为可能在步骤执行过程中被修改
                if (!m_runningInstances.contains(instance.instanceId)) {
                    LOG_MODULE_INFO("WorkflowManager", QString("步骤 '%1' 后检测到实例被移除。").arg(stepName).toStdString());
                    return false;
                }
                auto currentStatus = m_runningInstances[instance.instanceId].status;
                if (currentStatus != WorkflowStatus::Paused) break;
                // 释放锁并等待
                locker.unlock();
                QThread::msleep(100);
            }
            {
                QMutexLocker locker(&m_mutex);
                // 重新查找实例，因为可能在等待过程中被修改
                if (!m_runningInstances.contains(instance.instanceId)) {
                    LOG_MODULE_INFO("WorkflowManager", QString("步骤 '%1' 后检测到实例被移除。").arg(stepName).toStdString());
                    return false;
                }
                if (m_runningInstances[instance.instanceId].status == WorkflowStatus::Cancelled) {
                    LOG_MODULE_INFO("WorkflowManager", QString("在步骤 '%1' 后检测到取消信号。").arg(stepName).toStdString());
                    return false; // 工作流被取消
                }
            }
        }
    }
    return true; // 此列表中的所有步骤都已成功执行
}
}

