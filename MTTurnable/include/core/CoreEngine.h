#ifndef COREENGINE_H
#define COREENGINE_H

#include <QObject>
#include <QJsonObject>
#include <memory>
#include <QMap>
#include<Logger.h>
#include "domain/protocols/IXTProtocol.h" // 包含协议定义

namespace Services {
    class DeviceManager;
    class WorkflowManager;
    class DutManager;
    class LogService;
    class ConfigurationManager;
}

namespace Application {
    class IWorkflowStep;
    class WorkflowContext;
}

namespace Domain {
    class Dut; // 前向声明
    enum class Dut_State : int; // 前向声明枚举
}

namespace Core {

/**
 * @brief 核心引擎
 * 整个应用的总协调器，提供对外API
 */
// 职责划分与整体规划：
// 1. DutManager
//    • 仅做"集中存储 + 状态修改 API"，不主动改变业务。
//    • 提供 setState(dutId, newState) / setTestData(dutId, data) 等线程安全接口。
// 2. IWorkflowStep
//    • "业务原子单元"，是唯一知道"某条指令完成后要把哪个 DUT 状态改成什么"的层次。
//    • 在 execute(context) 内部：
//        a) 调 DeviceManager / 外部程序 执行动作；
//        b) 根据返回结果，通过 context->getDutManager()->setState(...) 或 setTestData(...) 更新 DUT。
// 3. WorkflowContext
//    • 只是"传输总线"，本身不做任何业务决策。
//    • 保存指针：DeviceManager、DutManager、WorkflowManager 以及临时数据。
// 4. WorkflowManager
//    • 负责"排程"——按顺序（或循环、并行）执行各 IWorkflowStep，处理暂停/恢复/取消。
//    • 绝不直接改 DUT 状态。
// 5. 外部程序（烧录、标定、Handler、AP8000V2 等）
//    • 皆按"设备"抽象，放入 DeviceManager；如是进程通信，可封装为 ProcessChannel + ProcessDevice。
//    • IWorkflowStep 通过 DeviceManager 取得句柄并调用。
// 所有"业务规则"都在 Step 内，可任意组合、复制、替换而不改框架代码
// 由 IWorkflowStep 触发 DutManager 的状态/数据更新。
// WorkflowContext 仅作"依赖传递"，WorkflowManager 仅作"排程驱动"
// 1.DutManager 永远保持"纯数据层"，对 UI / MES / 日志都可做统一订阅。
// 2.WorkflowManager 逻辑极简，后期要做并行、分支亦容易，toDo
// setState / setTestData 的触发路径
// IWorkflowStep::execute()
//    ├─ 调设备/外部程序（DeviceManager）
//    ├─ 解析结果
//    └─ context->getDutManager()->setState()/setTestData()
// UI 或 MES 只订阅 DutManager 的 dutStateChanged / testDataChanged 信号即可.
// TODOLIST:
//     处理多站点、多设备的思路
//     1. Site 概念
//        • 在 Dut 中增加 m_currentSite（如 "ActStation-1-DUT3"）。
//        • Handler 完成搬运后发事件（或 Step 调用）→ DutManager->setSite(dutId, newSite)。
//     2. 可选的"子工作流"
//        • 激活站与测试站可分别写成独立 workflow；
//        • 总流程可由一个"调度工作流"按事件（Dut 状态变化）启动子流程。
//     3. Device 的扩展
//        • 新增 AP8000V2：只需实现新 Device + 复用 Protocol/Channel。
//        • 新增进程调用：实现 ProcessChannel 与 ProcessDevice。
//     四、日志系统
//     1.在 CoreEngine 层集中收集 signal，写本地文件 + 转发到 MES；
//     2.IWorkflowStep 内部用 context->log()/logWarning()/logError()。
//     3.WorkflowManager 做并行
class CoreEngine : public QObject
{
    Q_OBJECT
public:
    enum class EngineStatus {
        NotInitialized,
        Initializing,
        Ready,
        Running,
        Error
    };

    explicit CoreEngine(QObject *parent = nullptr);
    ~CoreEngine();

    // 初始化
    bool initialize();
    bool shutdown();
    
    // 状态查询
    EngineStatus getStatus() const;
    bool isReady() const;
    
    // API - 自动流程控制 (Legacy - a a new dedicated workflow)
    // bool startAutoProcess(const QString &workflowConfigPath);
    // bool stopAutoProcess();
    // bool pauseAutoProcess();
    // bool resumeAutoProcess();
    
    // API - 手动控制
    QJsonObject manualStep(const QString &stepName, const QJsonObject &params);
    
    // API - 设备控制
    QJsonObject getDeviceStatus(const QString &deviceId);
    QJsonObject getAllDevicesStatus();
    QJsonObject executeDeviceCommand(const QString &deviceId, const QString &command, const QJsonObject &params);

    // API - 配置管理
    bool reloadDeviceConfiguration(const QString& configFilePath);
    
    // 获取管理器实例
    std::shared_ptr<Services::DeviceManager> getDeviceManager() const;
    std::shared_ptr<Services::WorkflowManager> getWorkflowManager() const;
    std::shared_ptr<Services::DutManager> getDutManager() const;
    std::shared_ptr<Services::LogService> getLogService() const;
    std::shared_ptr<Services::ConfigurationManager> getConfigurationManager() const;

signals:
    // 状态更新信号
    void engineStatusChanged(EngineStatus status);
    void coreEngineInitializationFailed(const QString& error); // 新增信号
    void processStarted(const QString &processName);
    void processCompleted(const QString &processName);
    void processFailed(const QString &processName, const QString &error);
    
    // 设备状态信号
    void deviceStatusUpdate(const QString &deviceId, const QJsonObject &status);
    
    // 测试结果信号
    void testResultAvailable(const QJsonObject &result);

    // 新增：全局业务事件信号，用于工作流同步
    void axisMoveCompleted(const QString& axisName, int siteIndex, bool success);

    // 新增: 用于 TestBoardDevice 和模拟流程的信号
    void powerStateChanged(const QString& deviceId, const Domain::Protocols::PowerFeedbackData &data);
    void testStateChanged(const QString& deviceId, bool isRunning);

    // 新增: 激活完成信号
    void activationCompleted(int siteIndex, const QJsonObject& result);

    // 新增: 校准完成信号
    void calibrationCompleted(const QString& dutId, const QJsonObject& result);

    // 新增: Handler放置芯片信号，用于主流程等待
    void chipPlaced(int siteIndex, uint32_t slotEn, const QString& siteSn);

private slots:
    void handleWorkflowCompleted(const QString &workflowId);
    void handleWorkflowFailed(const QString &workflowId, const QString &error);
    void handleDeviceStatusChanged(const QString &deviceId, int status);
    void onDutStateChanged(const QString& dutId, int newState);
    void onHandlerChipPlaced(int siteIndex, uint32_t slotEn, const QString& siteSn);
    void onHandlerAxisMovementCompleted(const QString& axisSelect, int siteIdx, int currentAngle, int result, const QString& errMsg);

private:
    bool initializeServices();
    //加载device.json,site.json,后续扩展
    bool loadConfiguration();

    void connectAllDeviceSignals();
    
private:
    EngineStatus m_status;
    // 服务实例
    std::shared_ptr<Services::DeviceManager> m_deviceManager;
    std::shared_ptr<Services::WorkflowManager> m_workflowManager;
    std::shared_ptr<Services::DutManager> m_dutManager;
    std::shared_ptr<Services::LogService> m_logService;
    std::shared_ptr<Services::ConfigurationManager> m_configManager;
    

};

} // namespace Core

#endif // COREENGINE_H
