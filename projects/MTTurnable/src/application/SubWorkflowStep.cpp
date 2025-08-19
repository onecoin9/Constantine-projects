#include "application/SubWorkflowStep.h"
#include "application/WorkflowContext.h"
#include "services/WorkflowManager.h"
#include "application/WorkflowUtils.h"
#include "core/Logger.h"
#include <QFile>
#include <QJsonDocument>

namespace Application {

SubWorkflowStep::SubWorkflowStep(const QJsonObject &config, QObject *parent)
    : IWorkflowStep(parent)
{
    setConfiguration(config);
}

SubWorkflowStep::~SubWorkflowStep() = default;

bool SubWorkflowStep::execute(std::shared_ptr<WorkflowContext> context)
{
    m_status = StepStatus::Running;
    if (!context) {
        LOG_MODULE_ERROR("SubWorkflowStep", "Context is null");
        m_status = StepStatus::Failed;
        return false;
    }
    bool ok = executeSubWorkflowSync(context);
    m_status = ok ? StepStatus::Completed : StepStatus::Failed;
    return ok;
}

bool SubWorkflowStep::executeSubWorkflowSync(std::shared_ptr<WorkflowContext> parentCtx)
{
    auto manager = parentCtx->getWorkflowManager();
    if (!manager) {
        LOG_MODULE_ERROR("SubWorkflowStep", "WorkflowManager not available in context");
        return false;
    }
    QJsonObject cfg = m_config.value("config").toObject();
    QString templateId = cfg.value("template").toString();
    if (templateId.isEmpty()) {
        LOG_MODULE_ERROR("SubWorkflowStep", "'template' missing in config");
        return false;
    }
    
    // 检查模板是否已加载，如果没有则尝试从文件加载
    if (!manager->getLoadedWorkflows().contains(templateId)) {
        LOG_MODULE_INFO("SubWorkflowStep", QString("Template '%1' not loaded, attempting to load from file").arg(templateId).toStdString());
        
        QFile file(templateId);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            LOG_MODULE_ERROR("SubWorkflowStep", QString("Cannot open template file: %1").arg(templateId).toStdString());
            return false;
        }
        
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        
        if (doc.isNull() || !doc.isObject()) {
            LOG_MODULE_ERROR("SubWorkflowStep", QString("Invalid JSON in template file: %1").arg(templateId).toStdString());
            return false;
        }
        
        if (!manager->loadWorkflow(templateId, doc.object())) {
            LOG_MODULE_ERROR("SubWorkflowStep", QString("Failed to load template: %1").arg(templateId).toStdString());
            return false;
        }
        
        LOG_MODULE_INFO("SubWorkflowStep", QString("Successfully loaded template: %1").arg(templateId).toStdString());
    }
    
    // 创建或继承上下文
    std::shared_ptr<WorkflowContext> subCtx;
    bool inherit = cfg.value("inheritContext").toBool(true);
    if (inherit) {
        subCtx = parentCtx;
    } else {
        subCtx = std::make_shared<WorkflowContext>();
        subCtx->setDeviceManager(parentCtx->getDeviceManager());
        subCtx->setDutManager(parentCtx->getDutManager());
        subCtx->setWorkflowManager(parentCtx->getWorkflowManager());
    }
    // 参数映射，占位符解析
    QJsonObject mapping = cfg.value("paramMapping").toObject();
    for (auto it = mapping.begin(); it != mapping.end(); ++it) {
        QString key = it.key();
        QVariant value = WorkflowUtils::replacePlaceholders(it.value().toString(), parentCtx);
        
        // 处理嵌套键（如 parameters.axisName）
        if (key.contains('.')) {
            QStringList parts = key.split('.');
            if (parts.size() == 2 && parts[0] == "parameters") {
                // 特殊处理parameters对象
                QJsonObject params;
                if (subCtx->hasData("parameters")) {
                    QVariant existingParams = subCtx->getData("parameters");
                    if (existingParams.canConvert<QJsonObject>()) {
                        params = existingParams.value<QJsonObject>();
                    }
                }
                params[parts[1]] = QJsonValue::fromVariant(value);
                subCtx->setData("parameters", params);
            } else {
                // 对于其他嵌套键，仍然使用平面存储
                subCtx->setData(key, value);
            }
        } else {
            subCtx->setData(key, value);
        }
    }
    
    bool success = manager->executeWorkflowSync(templateId, subCtx);
    m_result["success"] = success;
    return success;
}

bool SubWorkflowStep::canExecute(std::shared_ptr<WorkflowContext>) const { return true; }
QString SubWorkflowStep::getName() const { return m_config.value("name").toString("SubWorkflowStep"); }
QString SubWorkflowStep::getDescription() const { return "Executes a sub workflow"; }
QJsonObject SubWorkflowStep::getConfiguration() const { return m_config; }
QJsonObject SubWorkflowStep::getResult() const { return m_result; }
IWorkflowStep::StepStatus SubWorkflowStep::getStatus() const { return m_status; }
void SubWorkflowStep::setConfiguration(const QJsonObject &config) { m_config = config; }

} // namespace Application 