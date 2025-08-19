#include "application/GlobalConfigStep.h"
#include "application/WorkflowContext.h"
#include "core/Logger.h"

namespace Application {

GlobalConfigStep::GlobalConfigStep(const QJsonObject& config, QObject* parent)
    : IWorkflowStep(parent)
    , m_status(StepStatus::NotStarted)
{
    setConfiguration(config);
}

GlobalConfigStep::~GlobalConfigStep()
{
}

bool GlobalConfigStep::execute(std::shared_ptr<WorkflowContext> context)
{
    m_status = StepStatus::Running;
    emit statusChanged(m_status);
    
    LOG_MODULE_INFO("GlobalConfigStep", "开始执行全局配置步骤...");
    
    try {
        // 从配置中获取全局参数
        QString cmd3TaskPath = m_config.value("cmd3TaskPath").toString();
        QString taskFileName = m_config.value("taskFileName").toString();
        QString batchNumber = m_config.value("batchNumber").toString();
        int productionQuantity = m_config.value("productionQuantity").toInt(8);
        QString activationParams = m_config.value("activationParams").toString();
        
        // 将全局参数注入到工作流上下文中
        context->setData("global.cmd3TaskPath", cmd3TaskPath);
        context->setData("global.taskFileName", taskFileName);
        context->setData("global.batchNumber", batchNumber);
        context->setData("global.productionQuantity", productionQuantity);
        context->setData("global.activationParams", activationParams);
        
        // 设置兼容性参数（供现有步骤使用）
        context->setData("device.handler-01.cmd3TaskPath", cmd3TaskPath);
        context->setData("burn.taskFileName", taskFileName);
        context->setData("production.batchNumber", batchNumber);
        context->setData("production.quantity", productionQuantity);
        
        // 记录配置信息
        LOG_MODULE_INFO("GlobalConfigStep", QString("全局配置参数已设置:").toStdString());
        LOG_MODULE_INFO("GlobalConfigStep", QString("  - 自动机配置文件: %1").arg(cmd3TaskPath).toStdString());
        LOG_MODULE_INFO("GlobalConfigStep", QString("  - 烧录配置文件: %1").arg(taskFileName).toStdString());
        LOG_MODULE_INFO("GlobalConfigStep", QString("  - 批次号: %1").arg(batchNumber).toStdString());
        LOG_MODULE_INFO("GlobalConfigStep", QString("  - 生产数量: %1").arg(productionQuantity).toStdString());
        LOG_MODULE_INFO("GlobalConfigStep", QString("  - 激活参数: %1").arg(activationParams).toStdString());
        
        // 准备结果数据
        m_result = QJsonObject();
        m_result["cmd3TaskPath"] = cmd3TaskPath;
        m_result["taskFileName"] = taskFileName;
        m_result["batchNumber"] = batchNumber;
        m_result["productionQuantity"] = productionQuantity;
        m_result["activationParams"] = activationParams;
        m_result["configuredAt"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        m_status = StepStatus::Completed;
        emit statusChanged(m_status);
        emit logMessage("全局配置步骤执行完成");
        
        return true;
        
    } catch (const std::exception& e) {
        QString errorMsg = QString("全局配置步骤执行失败: %1").arg(e.what());
        LOG_MODULE_ERROR("GlobalConfigStep", errorMsg.toStdString());
        
        m_status = StepStatus::Failed;
        emit statusChanged(m_status);
        emit errorOccurred(errorMsg);
        
        return false;
    }
}

QString GlobalConfigStep::getName() const
{
    return m_config.value("name").toString("全局配置参数");
}

QString GlobalConfigStep::getDescription() const
{
    return m_config.value("description").toString("设置工作流的全局配置参数");
}

GlobalConfigStep::StepStatus GlobalConfigStep::getStatus() const
{
    return m_status;
}

QJsonObject GlobalConfigStep::getResult() const
{
    return m_result;
}

void GlobalConfigStep::setConfiguration(const QJsonObject& config)
{
    m_config = config;
}

QJsonObject GlobalConfigStep::getConfiguration() const
{
    return m_config;
}

bool GlobalConfigStep::canExecute(std::shared_ptr<WorkflowContext> context) const
{
    Q_UNUSED(context);
    
    // 全局配置步骤总是可以执行
    return true;
}

bool GlobalConfigStep::rollback(std::shared_ptr<WorkflowContext> context)
{
    // 移除注入的全局参数
    if (context) {
        context->removeData("global.cmd3TaskPath");
        context->removeData("global.taskFileName");
        context->removeData("global.batchNumber");
        context->removeData("global.productionQuantity");
        context->removeData("global.activationParams");
        
        // 移除兼容性参数
        context->removeData("device.handler-01.cmd3TaskPath");
        context->removeData("burn.taskFileName");
        context->removeData("production.batchNumber");
        context->removeData("production.quantity");
        
        LOG_MODULE_INFO("GlobalConfigStep", "全局配置参数已回滚");
    }
    
    m_status = StepStatus::NotStarted;
    m_result = QJsonObject();
    
    return true;
}

} // namespace Application 