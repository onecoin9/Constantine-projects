#include "application/WaitForSignalStep.h"
#include "application/WorkflowContext.h"
#include "core/CoreEngine.h"
#include "core/Logger.h"
#include "services/DutManager.h"
#include "services/DeviceManager.h"
#include "domain/HandlerDevice.h"
#include <QEventLoop>
#include <QTimer>
#include <QVariant>
#include <QRegularExpression>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

// 这是一个临时的全局变量，用于获取CoreEngine实例。
extern std::shared_ptr<Core::CoreEngine> g_coreEngine;

namespace Application {

namespace { // Anonymous namespace for helper functions

QVariant getPlaceholderValue(const QString& placeholder,
                               const std::shared_ptr<Application::WorkflowContext>& context,
                               const QVariantMap& signalArgs)
{
    QString key = placeholder.trimmed();
    if (key.startsWith("data.")) {
        return signalArgs.value(key.mid(5));
    }
    if (key.startsWith("context.")) {
        return context->getData(key.mid(8));
    }
    // 新增：支持从DutManager查询站点状态
    if (key.startsWith("site.")) {
        QString siteQuery = key.mid(5); // 去掉"site."前缀
        // 解析格式：site.<siteIndex>.<property>
        QStringList parts = siteQuery.split('.');
        if (parts.size() >= 2) {
            bool ok;
            int siteIndex = parts[0].toInt(&ok);
            if (ok) {
                auto dutManager = context->getDutManager();
                if (dutManager) {
                    QVariantMap siteStatus = dutManager->getSiteStatus(siteIndex);
                    if (parts.size() == 1) {
                        return siteStatus; // 返回整个状态对象
                    } else {
                        return siteStatus.value(parts[1]); // 返回特定属性
                    }
                }
            }
        }
    }
    // 新增：支持从DutManager查询DUT状态
    if (key.startsWith("dut.")) {
        QString dutQuery = key.mid(4); // 去掉"dut."前缀
        QStringList parts = dutQuery.split('.');
        if (!parts.isEmpty()) {
            QString dutId = parts[0];
            auto dutManager = context->getDutManager();
            if (dutManager) {
                auto dut = dutManager->getDut(dutId);
                if (dut) {
                    if (parts.size() == 1) {
                        // 返回DUT的状态
                        return static_cast<int>(dut->getState());
                    } else if (parts[1] == "state") {
                        return static_cast<int>(dut->getState());
                    } else if (parts[1] == "site") {
                        return dut->getCurrentSite();
                    }
                }
            }
        }
    }
    // Default to context for backward compatibility
    return context->getData(key);
}

bool evaluateCondition(const QString& condition,
                       const std::shared_ptr<Application::WorkflowContext>& context,
                       const QVariantMap& signalArgs)
{
    if (condition.isEmpty()) {
        return true; // No condition always evaluates to true
    }

    LOG_MODULE_DEBUG("WaitForSignalStep", QString("Evaluating condition: '%1'").arg(condition).toStdString());

    // 支持逻辑操作符 && 和 ||  
    // 重要：在进行占位符替换之前检查逻辑操作符
    if (condition.contains("&&")) {
        QStringList parts = condition.split("&&");
        for (const QString& part : parts) {
            if (!evaluateCondition(part.trimmed(), context, signalArgs)) {
                return false; // 所有部分都必须为true
            }
        }
        return true;
    }
    
    if (condition.contains("||")) {
        QStringList parts = condition.split("||");
        for (const QString& part : parts) {
            if (evaluateCondition(part.trimmed(), context, signalArgs)) {
                return true; // 任意一个部分为true即可
            }
        }
        return false;
    }

    // 进行占位符替换
    QString processedCondition = condition;
    QRegularExpression re("\\$\\{([^}]+)\\}");
    auto it = re.globalMatch(processedCondition);
    while(it.hasNext()){
        auto match = it.next();
        QString placeholder = match.captured(1);
        QVariant value = getPlaceholderValue(placeholder, context, signalArgs);
        // Important: Convert bool to lower case "true"/"false" for string comparison
        QString valueStr = (value.type() == QVariant::Bool) ? value.toString().toLower() : value.toString();
        LOG_MODULE_DEBUG("WaitForSignalStep", QString("Placeholder '%1' resolved to '%2'").arg(placeholder).arg(valueStr).toStdString());
        processedCondition.replace(match.captured(0), valueStr);
    }
    
    LOG_MODULE_DEBUG("WaitForSignalStep", QString("Processed condition: '%1'").arg(processedCondition).toStdString());
    
    // 增强的条件评估器，支持更多操作符
    QString op;
    if (processedCondition.contains("<=")) op = "<=";
    else if (processedCondition.contains(">=")) op = ">=";
    else if (processedCondition.contains("==")) op = "==";
    else if (processedCondition.contains("!=")) op = "!=";
    else if (processedCondition.contains("<")) op = "<";
    else if (processedCondition.contains(">")) op = ">";
    else return processedCondition.trimmed().toLower() == "true"; // For conditions like "${data.success}"

    if (op.isEmpty()) return false;

    QStringList parts = processedCondition.split(op);
    if (parts.size() != 2) return false;

    QString left = parts[0].trimmed();
    QString right = parts[1].trimmed();
    
    // Remove quotes for string literal comparison
    if ((right.startsWith("'") && right.endsWith("'")) || (right.startsWith("\"") && right.endsWith("\""))) {
        right = right.mid(1, right.length() - 2);
    }

    // 尝试转换为数字进行比较
    bool leftOk, rightOk;
    double leftNum = left.toDouble(&leftOk);
    double rightNum = right.toDouble(&rightOk);
    
    if (leftOk && rightOk) {
        // 数字比较
        LOG_MODULE_DEBUG("WaitForSignalStep", QString("Numeric comparison: %1 %2 %3").arg(leftNum).arg(op).arg(rightNum).toStdString());
        if (op == "==") return leftNum == rightNum;
        if (op == "!=") return leftNum != rightNum;
        if (op == "<") return leftNum < rightNum;
        if (op == ">") return leftNum > rightNum;
        if (op == "<=") return leftNum <= rightNum;
        if (op == ">=") return leftNum >= rightNum;
    } else {
        // 字符串比较
        LOG_MODULE_DEBUG("WaitForSignalStep", QString("String comparison: '%1' %2 '%3'").arg(left).arg(op).arg(right).toStdString());
        if (op == "==") return left.compare(right, Qt::CaseInsensitive) == 0;
        if (op == "!=") return left.compare(right, Qt::CaseInsensitive) != 0;
        // 字符串不支持大小比较
        if (op == "<" || op == ">" || op == "<=" || op == ">=") return false;
    }
    
    return false;
}

} // end anonymous namespace

WaitForSignalStep::WaitForSignalStep(const QJsonObject &config, QObject *parent) 
    : IWorkflowStep(parent)
    , m_timer(new QTimer(this)) // 在构造函数中初始化
    , m_mutex(new QMutex())
    , m_waitCondition(new QWaitCondition())
    , m_signalReceived(false)
    , m_success(false)
{
    setConfiguration(config);
    m_timer->setSingleShot(true);
}

WaitForSignalStep::~WaitForSignalStep() 
{
    delete m_mutex;
    delete m_waitCondition;
}

bool WaitForSignalStep::execute(std::shared_ptr<WorkflowContext> context)
{
    m_status = StepStatus::Running;
    
    if (!g_coreEngine) {
        LOG_MODULE_ERROR("WaitForSignalStep", "Global CoreEngine instance is not available.");
        m_status = StepStatus::Failed;
        return false;
    }

    QJsonObject config = m_config["config"].toObject();
    QString signalName = config["signal"].toString();
    int timeoutMs = config["timeoutMs"].toInt(30000); // 默认30秒超时
    QString condition = config["condition"].toString();

    // 重置状态
    m_signalReceived = false;
    m_success = false;
    
    // 设置超时定时器，使用Qt::QueuedConnection确保在主线程中执行
    QMetaObject::invokeMethod(m_timer, "start", Qt::QueuedConnection, Q_ARG(int, timeoutMs));
    
    // 连接超时信号，使用Qt::QueuedConnection
    connect(m_timer, &QTimer::timeout, this, [this]() {
        QMutexLocker locker(m_mutex);
        if (!m_signalReceived) {
            LOG_MODULE_ERROR("WaitForSignalStep", "Signal wait timed out");
            m_signalReceived = true;
            m_success = false;
            m_waitCondition->wakeAll();
        }
    }, Qt::QueuedConnection);

    QMetaObject::Connection conn;

    if (signalName == "axisMoveCompleted") {
        conn = connect(g_coreEngine.get(), &Core::CoreEngine::axisMoveCompleted, 
            this, [this, condition, context](const QString& axisName, int siteIndex, bool result) {
                QVariantMap args;
                args["axisName"] = axisName;
                args["siteIndex"] = siteIndex;
                args["success"] = result;
                if (evaluateCondition(condition, context, args)) {
                    QMutexLocker locker(m_mutex);
                    if (!m_signalReceived) {
                        m_success = result;
                        QString storeResultIn = m_config["config"].toObject().value("storeResultIn").toString();
                        if(!storeResultIn.isEmpty()){
                            context->setData(storeResultIn, args);
                        }
                        m_signalReceived = true;
                        m_waitCondition->wakeAll();
                    }
                }
            }, Qt::QueuedConnection);

    } 
    else if (signalName == "powerStateChanged") {
        conn = connect(g_coreEngine.get(), &Core::CoreEngine::powerStateChanged,
            this, [this, condition, context](const QString& deviceId, const Domain::Protocols::PowerFeedbackData &data) {
                bool mainPowerOn = (data.dutPowerState & 0x8000) != 0;
                quint16 dutMask = data.dutPowerState;

                QVariantMap args;
                args["deviceId"] = deviceId;
                args["mainPowerOn"] = mainPowerOn;
                args["dutMask"] = dutMask;
                args["success"] = (data.state == 1); // 假设state=1表示成功
                if (evaluateCondition(condition, context, args)) {
                    QMutexLocker locker(m_mutex);
                    if (!m_signalReceived) {
                        m_success = (data.state == 1);
                        QString storeResultIn = m_config["config"].toObject().value("storeResultIn").toString();
                        if(!storeResultIn.isEmpty()){
                            context->setData(storeResultIn, args);
                        }
                        m_signalReceived = true;
                        m_waitCondition->wakeAll();
                    }
                }
            }, Qt::QueuedConnection);

    } 
    else if (signalName == "testStateChanged") {
        conn = connect(g_coreEngine.get(), &Core::CoreEngine::testStateChanged,
            this, [this, condition, context](const QString& deviceId, bool isRunning) {
                QVariantMap args;
                args["deviceId"] = deviceId;
                args["isRunning"] = isRunning;
                args["success"] = true;
                if (evaluateCondition(condition, context, args)) {
                    QMutexLocker locker(m_mutex);
                    if (!m_signalReceived) {
                        m_success = true;
                        QString storeResultIn = m_config["config"].toObject().value("storeResultIn").toString();
                        if(!storeResultIn.isEmpty()){
                            context->setData(storeResultIn, args);
                        }
                        m_signalReceived = true;
                        m_waitCondition->wakeAll();
                    }
                }
            }, Qt::QueuedConnection);
    } 
    else if (signalName == "activationCompleted") {
        conn = connect(g_coreEngine.get(), &Core::CoreEngine::activationCompleted,
            this, [this, condition, context](int siteIndex, const QJsonObject& result) {
                QVariantMap args;
                args["siteIndex"] = siteIndex;
                args["method"] = result.value("method").toString();
                args["success"] = result.value("success").toBool();
                args["data"] = result.value("data").toVariant();
                if (evaluateCondition(condition, context, args)) {
                    QMutexLocker locker(m_mutex);
                    if (!m_signalReceived) {
                        m_success = result.value("success").toBool();
                        QString storeResultIn = m_config["config"].toObject().value("storeResultIn").toString();
                        if(!storeResultIn.isEmpty()){
                            context->setData(storeResultIn, QVariant::fromValue(result));
                        }
                        m_signalReceived = true;
                        m_waitCondition->wakeAll();
                    }
                }
            }, Qt::QueuedConnection);
    } 
    else if (signalName == "calibrationCompleted") {
        conn = connect(g_coreEngine.get(), &Core::CoreEngine::calibrationCompleted,
            this, [this, condition, context](const QString& dutId, const QJsonObject& result) {
                QVariantMap args;
                args["dutId"] = dutId;
                args["success"] = result.value("success").toBool();
                args["algorithm"] = result.value("algorithm").toString();
                args["data"] = result.value("data").toVariant();
                if (evaluateCondition(condition, context, args)) {
                    QMutexLocker locker(m_mutex);
                    if (!m_signalReceived) {
                        m_success = result.value("success").toBool();
                        QString storeResultIn = m_config["config"].toObject().value("storeResultIn").toString();
                        if(!storeResultIn.isEmpty()){
                            context->setData(storeResultIn, QVariant::fromValue(result));
                        }
                        m_signalReceived = true;
                        m_waitCondition->wakeAll();
                    }
                }
            }, Qt::QueuedConnection);
    } 
    else if (signalName == "chipPlaced") {
        LOG_MODULE_INFO("WaitForSignalStep", QString("Setting up chipPlaced signal listener with condition: '%1'").arg(condition).toStdString());
        
        if (!g_coreEngine) {
            LOG_MODULE_ERROR("WaitForSignalStep", "g_coreEngine is null! Cannot connect to chipPlaced signal");
            m_status = StepStatus::Failed;
            return false;
        }
        
        LOG_MODULE_INFO("WaitForSignalStep", QString("g_coreEngine is valid: %1").arg(reinterpret_cast<quintptr>(g_coreEngine.get()), 0, 16).toStdString());
        
        conn = connect(g_coreEngine.get(), &Core::CoreEngine::chipPlaced,
            this, [this, condition, context](int siteIndex, uint32_t slotEn, const QString& siteSn) {
                LOG_MODULE_INFO("WaitForSignalStep", QString("Received chipPlaced signal: Site=%1, SN=%2, SlotEn=0x%3")
                               .arg(siteIndex).arg(siteSn).arg(slotEn, 0, 16).toStdString());
                
                QVariantMap args;
                args["siteIndex"] = siteIndex;
                args["slotEn"] = slotEn;
                args["siteSn"] = siteSn;
                args["success"] = true; // chipPlaced signal is an event, assume success
                
                LOG_MODULE_INFO("WaitForSignalStep", QString("Evaluating condition '%1' with siteIndex=%2")
                               .arg(condition).arg(siteIndex).toStdString());
                
                if (evaluateCondition(condition, context, args)) {
                    LOG_MODULE_INFO("WaitForSignalStep", "Condition evaluated to TRUE, accepting signal");
                    QMutexLocker locker(m_mutex);
                    if (!m_signalReceived) {
                        m_success = true;
                        QString storeResultIn = m_config["config"].toObject().value("storeResultIn").toString();
                        if(!storeResultIn.isEmpty()){
                            context->setData(storeResultIn, args);
                            LOG_MODULE_INFO("WaitForSignalStep", QString("Stored result in context key: %1").arg(storeResultIn).toStdString());
                        }
                        m_signalReceived = true;
                        m_waitCondition->wakeAll();
                        LOG_MODULE_INFO("WaitForSignalStep", "Signal successfully processed, waking up waiting thread");
                    } else {
                        LOG_MODULE_WARNING("WaitForSignalStep", "Signal already received, ignoring duplicate");
                    }
                } else {
                    LOG_MODULE_INFO("WaitForSignalStep", "Condition evaluated to FALSE, ignoring signal");
                }
            }, Qt::DirectConnection);
            
        if (!conn) {
            LOG_MODULE_ERROR("WaitForSignalStep", "Failed to connect to chipPlaced signal!");
            m_status = StepStatus::Failed;
            return false;
        }
        
        LOG_MODULE_INFO("WaitForSignalStep", "Successfully connected to chipPlaced signal with DirectConnection");
    } 
    else if (signalName == "axisMovementCompleted") {
        LOG_MODULE_INFO("WaitForSignalStep", QString("Setting up axisMovementCompleted signal listener with condition: '%1'").arg(condition).toStdString());
        
        if (!g_coreEngine) {
            LOG_MODULE_ERROR("WaitForSignalStep", "g_coreEngine is null! Cannot connect to axisMovementCompleted signal");
            m_status = StepStatus::Failed;
            return false;
        }
        
        // 连接到HandlerDevice的axisMovementCompleted信号
        auto deviceManager = g_coreEngine->getDeviceManager();
        if (!deviceManager) {
            LOG_MODULE_ERROR("WaitForSignalStep", "DeviceManager is null!");
            m_status = StepStatus::Failed;
            return false;
        }
        
        auto handlerDevice = std::dynamic_pointer_cast<Domain::HandlerDevice>(deviceManager->getDevice("handler-01"));
        if (!handlerDevice) {
            LOG_MODULE_ERROR("WaitForSignalStep", "HandlerDevice 'handler-01' not found!");
            m_status = StepStatus::Failed;
            return false;
        }
        
        conn = connect(handlerDevice.get(), &Domain::HandlerDevice::axisMovementCompleted,
            this, [this, condition, context](const QString& axisSelect, int siteIdx, int currentAngle, int result, const QString& errMsg) {
                LOG_MODULE_INFO("WaitForSignalStep", QString("Received axisMovementCompleted signal: Axis=%1, Site=%2, Angle=%3, Result=%4")
                               .arg(axisSelect).arg(siteIdx).arg(currentAngle).arg(result).toStdString());
                
                QVariantMap args;
                args["axisSelect"] = axisSelect;
                args["siteIdx"] = siteIdx;
                args["currentAngle"] = currentAngle;
                args["result"] = result;
                args["errMsg"] = errMsg;
                args["success"] = (result == 0); // result为0表示成功
                
                // 如果没有条件或条件为空，则接受所有信号
                if (condition.isEmpty() || evaluateCondition(condition, context, args)) {
                    LOG_MODULE_INFO("WaitForSignalStep", "Condition evaluated to TRUE, accepting signal");
                    QMutexLocker locker(m_mutex);
                    if (!m_signalReceived) {
                        m_success = (result == 0);
                        QString storeResultIn = m_config["config"].toObject().value("storeResultIn").toString();
                        if(!storeResultIn.isEmpty()){
                            context->setData(storeResultIn, args);
                            LOG_MODULE_INFO("WaitForSignalStep", QString("Stored result in context key: %1").arg(storeResultIn).toStdString());
                        }
                        m_signalReceived = true;
                        m_waitCondition->wakeAll();
                    }
                } else {
                    LOG_MODULE_INFO("WaitForSignalStep", QString("Condition evaluated to FALSE, ignoring signal. Condition: '%1', Args: axisSelect=%2, currentAngle=%3")
                                   .arg(condition).arg(axisSelect).arg(currentAngle).toStdString());
                }
            }, Qt::DirectConnection);
            
        if (!conn) {
            LOG_MODULE_ERROR("WaitForSignalStep", "Failed to connect to axisMovementCompleted signal!");
            m_status = StepStatus::Failed;
            return false;
        }
        
        LOG_MODULE_INFO("WaitForSignalStep", "Successfully connected to axisMovementCompleted signal");
    } 
    else if (signalName == "doJobCompleted") {

        conn = connect(g_coreEngine.get(), &Core::CoreEngine::doJobCompleted,
            this, [this, condition, context](const QJsonObject& result) {
                QString strIp = result.value("strIp").toString();
                if (strIp == context->getData("siteIp").toString()) {
                    QMutexLocker locker(m_mutex);
                    if (!m_signalReceived) {
                        LOG_MODULE_INFO("WaitForSignalStep", QString("doJobCompleted result json:%1, siteIp:%2").arg(QString(QJsonDocument(result).toJson(QJsonDocument::Compact))).arg(context->getData("siteIp").toString()).toStdString());
                        m_success = true;
                        QString storeResultIn = m_config["config"].toObject().value("storeResultIn").toString();
                        if(!storeResultIn.isEmpty()){
                            context->setData(storeResultIn, QVariant::fromValue(result));
                        }
                        m_signalReceived = true;
                        m_waitCondition->wakeAll();
                    }
                }
            }, Qt::DirectConnection);
    }
    else {
        LOG_MODULE_ERROR("WaitForSignalStep", QString("Unsupported signal to wait for: '%1'").arg(signalName).toStdString());
        m_status = StepStatus::Failed;
        return false;
    }

    // 使用QMutex和QWaitCondition替代QEventLoop，避免跨线程事件循环冲突
    QMutexLocker locker(m_mutex);
    while (!m_signalReceived) {
        // 等待信号或超时，最多等待100ms然后检查状态
        m_waitCondition->wait(m_mutex, 100);
    }
    
    // 清理
    disconnect(conn);
    if (m_timer->isActive()) {
        QMetaObject::invokeMethod(m_timer, "stop", Qt::QueuedConnection);
    }

    if (!m_success) {
        LOG_MODULE_ERROR("WaitForSignalStep", QString("Wait for signal '%1' failed or timed out.").arg(signalName).toStdString());
        m_status = StepStatus::Failed;
        return false;
    }

    m_status = StepStatus::Completed;
    return true;
}

QString WaitForSignalStep::getName() const { return m_config["name"].toString(); }
QString WaitForSignalStep::getDescription() const { return "Waits for a specific signal from the CoreEngine."; }
bool WaitForSignalStep::canExecute(std::shared_ptr<WorkflowContext>) const { return true; }
void WaitForSignalStep::setConfiguration(const QJsonObject& config) { m_config = config; }
QJsonObject WaitForSignalStep::getConfiguration() const { return m_config; }
QJsonObject WaitForSignalStep::getResult() const { return m_result; }
IWorkflowStep::StepStatus WaitForSignalStep::getStatus() const { return m_status; }

} 