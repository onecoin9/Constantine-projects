#include "core/CoreEngine.h"
#include "services/DeviceManager.h"
#include "services/WorkflowManager.h"
#include "services/DutManager.h"
#include "domain/IDevice.h"
#include "domain/HandlerDevice.h"
#include "domain/TestBoardDevice.h"
#include "domain/BurnDevice.h"
#include "application/WorkflowContext.h"
#include "GlobalItem.h"
#include <QJsonDocument>
#include <QFile>
#include <QTimer>
#include "infrastructure/SerialPortManager.h"
#include "domain/Dut.h"
#include "services/SiteWorkflowRouter.h"
#include "domain/AsyncProcessDevice.h"
#include <QThread>
#include <QtConcurrent/QtConcurrent>
/*#ifdef Q_OS_WIN
#include <windows.h>
#include <locale.h>
#include <io.h>
#include <fcntl.h>
#endif*/


namespace Core {

CoreEngine::CoreEngine(QObject *parent)
    : QObject(parent)
    , m_status(EngineStatus::NotInitialized)
{
    // 日志系统已在 main() 中统一初始化，此处不再需要
}

CoreEngine::~CoreEngine()
{
    // 所有关闭逻辑都在main()函数中通过coreEngine->shutdown()显式调用。
    // 此处必须为空，以防止在静态对象销毁阶段再次调用shutdown()，
    // 从而访问已经被销毁的Logger或其他单例。
}

bool CoreEngine::initialize()
{
    m_status = EngineStatus::Initializing;
    emit engineStatusChanged(m_status);
    if (!initializeServices()) {
        m_status = EngineStatus::Error;
        emit engineStatusChanged(m_status);
        emit coreEngineInitializationFailed("服务初始化失败"); // 发出信号
        return false;
    }
    if (!loadConfiguration()) {
               m_status = EngineStatus::Error;
               emit engineStatusChanged(m_status);
               QString lastError = m_deviceManager->getLastError(); // 尝试获取更详细的错误
               emit coreEngineInitializationFailed("加载设备配置失败: " + lastError); // 发出信号
               return false;
    }
    connectAllDeviceSignals();

    m_status = EngineStatus::Ready;
    emit engineStatusChanged(m_status);
    LOG_INFO("核心引擎初始化完成");
    return true;
}

bool CoreEngine::shutdown()
{
    // 此方法已废弃，所有关闭逻辑依赖于操作系统对进程资源的自动回收，
    // 以避免静态对象析构顺序问题。
    return true;
}

CoreEngine::EngineStatus CoreEngine::getStatus() const
{
    return m_status;
}

bool CoreEngine::isReady() const
{
    return m_status == EngineStatus::Ready;
}

QJsonObject CoreEngine::manualStep(const QString &stepName, const QJsonObject &params)
{
    QJsonObject result;
    result["success"] = false;
    result["message"] = "手动步骤执行功能尚未实现";
    return result;
}

QJsonObject CoreEngine::getDeviceStatus(const QString &deviceId)
{
    QJsonObject status;
    
    if (!m_deviceManager) {
        status["error"] = "设备管理器未初始化";
        return status;
    }
    
    auto device = m_deviceManager->getDevice(deviceId);
    if (!device) {
        status["error"] = QString("设备 '%1' 未找到").arg(deviceId);
        return status;
    }
    
    status["deviceId"] = deviceId;
    status["status"] = static_cast<int>(device->getStatus());
    status["name"] = device->getName();
    status["type"] = static_cast<int>(device->getType());
    status["description"] = device->getDescription();
    
    return status;
}

QJsonObject CoreEngine::getAllDevicesStatus()
{
    QJsonObject allStatus;
    
    if (!m_deviceManager) {
        allStatus["error"] = "设备管理器未初始化";
        return allStatus;
    }
    
    QJsonObject devices;
    for (const QString &deviceId : m_deviceManager->getAllDeviceIds()) {
        devices[deviceId] = getDeviceStatus(deviceId);
    }
    
    allStatus["devices"] = devices;
    return allStatus;
}



QJsonObject CoreEngine::executeDeviceCommand(const QString &deviceId, const QString &command, const QJsonObject &params)
{
    if (!m_deviceManager) {
        QJsonObject error;
        error["success"] = false;
        error["error"] = "设备管理器未初始化";
        return error;
    }
    
    auto device = m_deviceManager->getDevice(deviceId);
    if (!device) {
        QJsonObject error;
        error["success"] = false;
        error["error"] = QString("设备 '%1' 未找到").arg(deviceId);
        return error;
    }
    
    return device->executeCommand(command, params);
}



std::shared_ptr<Services::DeviceManager> CoreEngine::getDeviceManager() const
{
    return m_deviceManager;
}

std::shared_ptr<Services::WorkflowManager> CoreEngine::getWorkflowManager() const
{
    return m_workflowManager;
}

std::shared_ptr<Services::LogService> CoreEngine::getLogService() const
{
    return m_logService;
}

std::shared_ptr<Services::ConfigurationManager> CoreEngine::getConfigurationManager() const
{
    return m_configManager;
}

bool CoreEngine::reloadDeviceConfiguration(const QString &configFilePath)
{
    LOG_MODULE_INFO("CoreEngine", QString("Reloading device configuration from: %1").arg(configFilePath).toStdString());
    LOG_INFO(QString("准备重新加载设备配置文件: %1").arg(configFilePath).toStdString());

    if (!m_deviceManager) {
        LOG_ERROR("设备管理器未初始化，无法加载设备配置。");
        return false;
    }

    // 重新加载设备配置流程

    QFile file(configFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR(QString("无法打开设备配置文件: %1 - %2").arg(configFilePath).arg(file.errorString()).toStdString());
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) {
        LOG_ERROR(QString("设备配置文件格式错误: %1").arg(configFilePath).toStdString());
        return false;
    }
    
    QJsonObject configRoot = doc.object();
    if (!configRoot.contains("devices") || !configRoot["devices"].isArray()) {
        LOG_ERROR(QString("设备配置文件缺少 'devices' 数组: %1").arg(configFilePath).toStdString());
        return false;
    }

    // 先清除现有设备，再加载新设备
    m_deviceManager->releaseAllDevices();
    LOG_INFO("已释放所有当前设备，准备加载新配置。");

    bool success = m_deviceManager->loadDevicesFromConfig(configRoot);

    if (success) {
        LOG_INFO(QString("设备配置已成功从 %1 加载并初始化。").arg(configFilePath).toStdString());
        // 触发一次全局设备状态更新，让UI刷新
        // 可以遍历所有新加载的设备并单独发送 deviceStatusUpdate，或者有一个更通用的信号
        for (const QString &deviceId : m_deviceManager->getAllDeviceIds()) {
            handleDeviceStatusChanged(deviceId, static_cast<int>(m_deviceManager->getDevice(deviceId)->getStatus()));
        }
    } else {
        LOG_ERROR(QString("从 %1 加载设备配置失败。").arg(configFilePath).toStdString());
    }
    
    return success;
}


void CoreEngine::handleWorkflowCompleted(const QString &instanceId)
{
    LOG_MODULE_INFO("CoreEngine", QString("Workflow instance '%1' completed.").arg(instanceId).toStdString());
    auto context = m_workflowManager->getWorkflowContext(instanceId);
    if (!context) {
        LOG_MODULE_ERROR("CoreEngine", QString("Cannot find context for completed instance '%1'.").arg(instanceId).toStdString());
        return;
    }

    // QString dutId = context->getData("dutId").toString();
    // if (dutId.isEmpty()) {
    //     LOG_MODULE_WARNING("CoreEngine", QString("No dutId found in context for instance '%1'.").arg(instanceId).toStdString());
    //     return;
    // }
    int siteEn = context->getData("dutMask").toInt();
    
    // 根据完成的工作流ID（模板名），决定下一步动作
    QString workflowId = context->getWorkflowId();
    if (workflowId.contains("activate")) {
        // 激活成功，通知Handler
        for (int i = 0; i < 32; ++i) {
            if ((siteEn >> i) & 1) {
                m_dutManager->setState(QString("%1-DUT%2").arg(context->getData("siteSn").toString()).arg(i + 1), Domain::Dut::State::ActivationPassed);
            }
        }
        // 这里可以调用handler设备，发送67码
        auto handler = std::dynamic_pointer_cast<Domain::HandlerDevice>(m_deviceManager->getDevice("handler-01"));
        if(handler) {
            handler->executeCommand("setDoneSite", {{"siteIdx", context->getData("siteId").toInt()}});
        }

    } else if (workflowId.contains("test")) {
        // 测试成功，通知Handler
        for (int i = 0; i < 32; ++i) {
            if ((siteEn >> i) & 1) {
                m_dutManager->setState(QString("%1-DUT%2").arg(context->getData("siteSn").toString()).arg(i + 1), Domain::Dut::State::TestingPassed);
            }
        }
        // m_dutManager->setState(dutId, Domain::Dut::State::TestingPassed);
    }
}

void CoreEngine::handleWorkflowFailed(const QString &instanceId, const QString &error)
{
    LOG_MODULE_ERROR("CoreEngine", QString("Workflow instance '%1' failed: %2").arg(instanceId).arg(error).toStdString());
    auto context = m_workflowManager->getWorkflowContext(instanceId);
    if (!context) return;
    
    QString dutId = context->getData("dutId").toString();
    if (dutId.isEmpty()) return;

    QString workflowId = context->getWorkflowId();
    if (workflowId.contains("activate")) {
        m_dutManager->setState(dutId, Domain::Dut::State::ActivationFailed);
    } else if (workflowId.contains("test")) {
        m_dutManager->setState(dutId, Domain::Dut::State::TestingFailed);
    }
}

void CoreEngine::handleDeviceStatusChanged(const QString &deviceId, int status)
{
    QJsonObject statusUpdate;
    statusUpdate["deviceId"] = deviceId;
    statusUpdate["status"] = status;
    emit deviceStatusUpdate(deviceId, statusUpdate);
}

bool CoreEngine::initializeServices()
{
    LOG_MODULE_DEBUG("CoreEngine", "Initializing services...");
    
    // 创建设备管理器
    m_deviceManager = std::make_shared<Services::DeviceManager>();
    
    // 创建Dut管理器
    m_dutManager = Services::DutManager::instance();

    // 让 DeviceManager 能回写 DUT 数据
    m_deviceManager->setDutManager(m_dutManager);

    // --------------------------------

    // 创建工作流管理器，并注入依赖
    m_workflowManager = std::make_shared<Services::WorkflowManager>(m_deviceManager, m_dutManager);
    
    // 连接工作流管理器信号
    connect(m_workflowManager.get(), &Services::WorkflowManager::workflowCompleted,
            this, &CoreEngine::handleWorkflowCompleted);
    connect(m_workflowManager.get(), &Services::WorkflowManager::workflowFailed,
            this, &CoreEngine::handleWorkflowFailed);
    
    // 连接DutManager的信号，使用lambda转换枚举为int
    // connect(m_dutManager.get(), &Services::DutManager::dutStateChanged,
    //         this, [this](const QString& dutId, Domain::Dut::State newState){
    //             this->onDutStateChanged(dutId, static_cast<int>(newState));
    //         });
    

    
    return true;
}

bool CoreEngine::loadConfiguration()
{
    //QString configPath = "config/device.json";//设备配置文件
    QString configPath = "config/device_config_with_handler.json";//设备配置文件
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR(QString("无法打开配置文件: %1").arg(configPath).toStdString());
        return false;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) {
        LOG_ERROR("配置文件格式错误");
        return false;
    }
    QString configPath2 = "config/site.json";//设备配置文件

    if(!m_dutManager->loadSiteConfiguration(configPath2)){
        LOG_MODULE_WARNING("CoreEngine", "loadSiteConfiguration failed!");
    };
    // 加载设备配置
    if (m_deviceManager) {
        if (!m_deviceManager->loadDevicesFromConfig(doc.object())) {
            LOG_ERROR("加载设备配置失败");
            return false;
        }  
        // 检查是否需要自动初始化设备
        QJsonObject globalSettings = doc.object()["globalSettings"].toObject();
        bool autoInitOnStartup = globalSettings["autoInitOnStartup"].toBool(false);
        //根据device.json中的参数设置是否自动初始化设备，todo，是否启用
        if (autoInitOnStartup) {
            LOG_MODULE_INFO("CoreEngine", "Auto-initializing all devices on startup...");
            
            // 延迟初始化Handler设备，避免与VAuto端口冲突
            QTimer::singleShot(2000, this, [this]() {
                LOG_MODULE_INFO("CoreEngine", "Delayed initialization of Handler devices...");
                auto deviceIds = m_deviceManager->getAllDeviceIds();
                for (const QString& deviceId : deviceIds) {
                    auto device = m_deviceManager->getDevice(deviceId);
                    if (device && device->getType() == Domain::IDevice::DeviceType::Handler) {
                        if (!m_deviceManager->isDeviceInitialized(deviceId)) {
                            LOG_MODULE_INFO("CoreEngine", QString("Initializing Handler device: %1").arg(deviceId).toStdString());
                            m_deviceManager->initializeDevice(deviceId);
                        }
                    }
                }
            });
            
            // 立即初始化非Handler设备
            auto deviceIds = m_deviceManager->getAllDeviceIds();
            for (const QString& deviceId : deviceIds) {
                auto device = m_deviceManager->getDevice(deviceId);
                if (device && device->getType() != Domain::IDevice::DeviceType::Handler) {
                    if (!m_deviceManager->initializeDevice(deviceId)) {
                        LOG_MODULE_WARNING("CoreEngine", QString("Failed to initialize device: %1").arg(deviceId).toStdString());
                    }
                }
            }
        }
    }
    
    return true;
}

Services::DutManager* CoreEngine::getDutManager() const
{
    return m_dutManager;
}

void CoreEngine::onDutStateChanged(const QString &dutId, int newStateInt)
{
    Domain::Dut::State newState = static_cast<Domain::Dut::State>(newStateInt);

    LOG_MODULE_INFO("CoreEngine", QString("State changed for DUT '%1' to '%2'")
        .arg(dutId).arg(static_cast<int>(newState)).toStdString());

    auto dut = m_dutManager->getDut(dutId);
    if (!dut) {
        LOG_MODULE_ERROR("CoreEngine", QString("Received state change for non-existent DUT: %1").arg(dutId).toStdString());
        return;
    }

    // This function is now purely for logging and reacting to state changes.
    // It NO LONGER triggers major workflows. Workflow triggering is handled
    // by higher-level events like onHandlerChipPlaced.
    LOG_MODULE_DEBUG("CoreEngine", QString("State for DUT %1 is now '%2'. This is an informational event.")
        .arg(dutId).arg(static_cast<int>(newState)).toStdString());
}

void CoreEngine::connectAllDeviceSignals()
{
    // 假设handler的deviceId是"handler_1"
    if (!m_deviceManager) return;
    
    for (const auto& deviceId : m_deviceManager->getAllDeviceIds()) {
        auto device = m_deviceManager->getDevice(deviceId);
        if (!device) continue;

        if (auto handler = std::dynamic_pointer_cast<Domain::HandlerDevice>(device)) {
            LOG_MODULE_INFO("CoreEngine", QString("Connecting to HandlerDevice '%1' signals...").arg(deviceId).toStdString());
            connect(handler.get(), &Domain::HandlerDevice::chipPlaced, this, &CoreEngine::onHandlerChipPlaced);
            connect(handler.get(), &Domain::HandlerDevice::axisMovementCompleted, this, &CoreEngine::onHandlerAxisMovementCompleted);
            connect(handler.get(), &Domain::HandlerDevice::testResultSent, this, [this](int siteIndex, const QByteArray& resultData) {
                // 解析结果数据，判断是否成功
                // 根据协议，0x01表示Bin1(PASS)，其他值表示失败
                bool hasSuccess = false;
                for (int i = 0; i < resultData.size(); ++i) {
                    if (static_cast<uint8_t>(resultData[i]) == 0x01) {
                        hasSuccess = true;
                        break;
                    }
                }
                
                // 更新DutManager中的站点测试结果
                if (m_dutManager) {
                    m_dutManager->updateSiteTestResult(siteIndex, hasSuccess);
                }
                
                LOG_MODULE_INFO("CoreEngine", QString("Test result for site %1: %2")
                               .arg(siteIndex).arg(hasSuccess ? "SUCCESS" : "FAIL").toStdString());
            });
        } else if (auto testBoard = std::dynamic_pointer_cast<Domain::TestBoardDevice>(device)) {
            LOG_MODULE_INFO("CoreEngine", QString("Connecting to TestBoardDevice '%1' signals...").arg(deviceId).toStdString());
            // 断开旧连接，防止重复
            disconnect(testBoard.get(), &Domain::TestBoardDevice::powerStateChanged, this, nullptr);
            disconnect(testBoard.get(), &Domain::TestBoardDevice::testStateChanged, this, nullptr);

            connect(testBoard.get(), &Domain::TestBoardDevice::powerStateChanged, this, [this, deviceId](const Domain::Protocols::PowerFeedbackData &data){
                LOG_MODULE_DEBUG("CoreEngine", QString("Relaying powerStateChanged from %1").arg(deviceId).toStdString());
                emit this->powerStateChanged(deviceId, data);
            });
            connect(testBoard.get(), &Domain::TestBoardDevice::testStateChanged, this, [this, deviceId](bool isRunning){
                LOG_MODULE_DEBUG("CoreEngine", QString("Relaying testStateChanged from %1").arg(deviceId).toStdString());
                emit this->testStateChanged(deviceId, isRunning);
            });
        } else if (auto asyncDevice = std::dynamic_pointer_cast<Domain::AsyncProcessDevice>(device)) {
            LOG_MODULE_INFO("CoreEngine", QString("Connecting to AsyncProcessDevice '%1' signals...").arg(deviceId).toStdString());
            
            connect(asyncDevice.get(), &Domain::AsyncProcessDevice::activationCompleted, 
                    this, [this](int siteIndex, const QJsonObject& result){
                        LOG_MODULE_DEBUG("CoreEngine", QString("Relaying activationCompleted for site %1").arg(siteIndex).toStdString());
                        emit this->activationCompleted(siteIndex, result);
                    });
            
            connect(asyncDevice.get(), &Domain::AsyncProcessDevice::calibrationCompleted,
                    this, [this](const QString& dutId, const QJsonObject& result){
                        LOG_MODULE_DEBUG("CoreEngine", QString("Relaying calibrationCompleted for DUT %1").arg(dutId).toStdString());
                        emit this->calibrationCompleted(dutId, result);
                    });
        } else if (auto burnIn = std::dynamic_pointer_cast<Domain::BurnDevice>(device)) {
            LOG_MODULE_INFO("CoreEngine", QString("Connecting to BurnInDevice '%1' signals...").arg(deviceId).toStdString());
            connect(burnIn.get(), &Domain::BurnDevice::doJobCompleted,
                    this, [this](const QJsonObject& result){
                        LOG_MODULE_DEBUG("CoreEngine", QString("Relaying doJobCompleted.").toStdString());
                        emit this->doJobCompleted(result);
                    });
        }
    }
}

void CoreEngine::onHandlerChipPlaced(int siteIndex, uint32_t slotEn, const QString& siteSn)
{

    LOG_MODULE_INFO("CoreEngine", QString("Event from Handler: Chip placed at site %1 (SN: %2), slot mask: 0x%3")
        .arg(siteIndex).arg(siteSn).arg(QString::number(slotEn, 16)).toStdString());

    // 更新DutManager中的站点状态
    if (m_dutManager) {
        m_dutManager->updateSiteChipPlacement(siteIndex, static_cast<quint64>(slotEn), siteSn);
    }

    // 首先立即发射chipPlaced信号，确保WaitForSignalStep能接收到
    // 这样无论路由是否成功，等待该信号的工作流都能继续执行
    emit chipPlaced(siteIndex, slotEn, siteSn);

    if (siteIndex == TURNABLE_SITE_INDEX) {
        QByteArray dataArr(MAX_SOCKET_NUM, 0x00);
        for (int i = 0; i < MAX_SOCKET_NUM; i++) {
            if ((slotEn & 0xFF) & (1 << i)) {
                dataArr[i] = 1;
            }
        }
        Services::DutManager::instance()->updateSiteChipStatusByIndex(TURNABLE_SITE_INDEX, dataArr);
        LOG_MODULE_INFO("CoreEngine", QString("Update turnable site chip status:%1")
            .arg(dataArr.toHex().constData()).toStdString());
    }

    // 使用QtConcurrent::run将路由相关的处理过程移到后台线程
    QtConcurrent::run([this, siteIndex, slotEn, siteSn]() {
        // 使用路由器决定启动工作流（可选功能）
        auto routeOpt = Services::SiteWorkflowRouter::instance().match(siteSn);
        if(!routeOpt){
            LOG_MODULE_INFO("CoreEngine", QString("No workflow route matched for site SN: %1 (using direct workflow control)").arg(siteSn).toStdString());
            return; // 路由失败不影响信号发射，工作流通过WaitForSignalStep直接控制
        }
        
        LOG_MODULE_INFO("CoreEngine", QString("Starting route-based workflow '%1' for site %2").arg(routeOpt->workflow).arg(siteSn).toStdString());
        QString workflowToStart = routeOpt->workflow;

        Domain::Dut::State initialDutState = Domain::Dut::State::Unknown;
        if(workflowToStart.contains("activate", Qt::CaseInsensitive))
            initialDutState = Domain::Dut::State::AtActivationStation;
        else if(workflowToStart.contains("test", Qt::CaseInsensitive))
            initialDutState = Domain::Dut::State::AtTestStation;

       // 注册 DUT
        for (int i = 0; i < 32; ++i) {
            if ((slotEn >> i) & 1) {
                QString dutId = QString("%1-DUT%2").arg(siteSn).arg(i + 1);
                m_dutManager->registerDut(dutId);
                m_dutManager->setSite(dutId, siteSn);
                m_dutManager->setState(dutId, initialDutState);
            }
        }

        if (!m_workflowManager->getLoadedWorkflows().contains(workflowToStart)) {
            QFile file(workflowToStart);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                LOG_MODULE_INFO("CoreEngine", QString("无法打开工作流文件: %1").arg(workflowToStart).toStdString());
                return;
            }

            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            file.close();

            if (doc.isNull() || !doc.isObject()) {
                LOG_MODULE_INFO("CoreEngine", QString("工作流文件格式错误: %1").arg(workflowToStart).toStdString());
                return;
            }

            if (!m_workflowManager->loadWorkflow(workflowToStart, doc.object())) {
                LOG_MODULE_INFO("CoreEngine", QString("加载工作流模板失败: %1").arg(workflowToStart).toStdString());
                return;
            }
        }

        // 保存ip
        auto siteInfo = m_dutManager->getSiteInfoByIndex(siteIndex);

        auto context = std::make_shared<Application::WorkflowContext>();
        context->setData("siteId", siteIndex);
        context->setData("siteSn", siteSn);
        context->setData("dutMask", slotEn);
        context->setData("siteIp", siteInfo.ip);

        QString instanceId = m_workflowManager->startWorkflow(workflowToStart, context);
        if(instanceId.isEmpty()){
            LOG_MODULE_ERROR("CoreEngine", QString("Failed to start workflow for site '%1'.").arg(siteSn).toStdString());
        } else {
            LOG_MODULE_INFO("CoreEngine", QString("Route-based workflow instance '%1' started for site %2").arg(instanceId).arg(siteSn).toStdString());
        }
    });
}

void CoreEngine::onHandlerAxisMovementCompleted(const QString& axisSelect, int siteIdx, int currentAngle, int result, const QString& errMsg)
{
    LOG_MODULE_INFO("CoreEngine", QString("Event from Handler: Axis '%1' movement completed at site %2. Result: %3, Err: %4")
        .arg(axisSelect).arg(siteIdx).arg(result).arg(errMsg).toStdString());
    
    // 这个信号可以用来解除在工作流步骤中的等待，或更新某个特定DUT的状态。
    // 例如，可以发射一个全局信号，让等待的步骤继续执行。
    emit axisMoveCompleted(axisSelect, siteIdx, result == 0);
}

} // namespace Core
