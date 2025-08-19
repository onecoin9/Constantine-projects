#include "presentation/MainWindow.h"
#include "core/CoreEngine.h" // 包含 CoreEngine.h
#include "services/WorkflowManager.h" // 新增
#include "services/DeviceManager.h" // 添加 DeviceManager.h
#include "application/WorkflowContext.h" // 新增
#include "presentation/DeviceManagerDialog.h" // 包含 DeviceManagerDialog.h
#include "presentation/DeviceControlDialog.h" // 添加 DeviceControlDialog.h
#include "presentation/DutMonitorWidget.h" // <-- 1. 包含新的头文件
#include "presentation/ACAutomaticSetting.h"
#include "ui/widgets/TestSite.h"  // 添加 TestSite 头文件
#include "form.h"
#include <QDebug> // 添加 QDebug 用于调试
#include <QTimer> // 添加 QTimer
#include "core/Logger.h"
#include "presentation/LogConfigDialog.h"

// Qt Widgets
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTreeWidget>
#include <QTableWidget>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QDateTime>
#include <QScrollArea>
#include <QSettings>
#include <QTabWidget>
#include <QFrame>
#include <QSpinBox>
#include <QSizePolicy>
#include <QIcon>
#include <QPixmap>

namespace Presentation {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_coreEngine(nullptr) // 初始化 m_coreEngine
    // , m_serialManager(std::make_unique<SerialManager>(this)) // 注释掉或者确保 SerialManager 已定义并包含头文件
    , m_isPortOpen(false)
    , m_isTestRunning(false)
    , m_displayTimer(new QTimer(this))
    , m_deviceManagerDialog(nullptr) // 初始化为 nullptr
    , m_isProcessRunning(false) // 初始化 m_isProcessRunning
    // , m_currentWorkflowPath() // QString 默认构造为空，无需显式初始化
    , m_dutMonitorWidget(nullptr) // 初始化为 nullptr
    , m_testSiteScrollArea(nullptr)
    , m_timeUpdateTimer(new QTimer(this))
    , m_settings("TesterFramework", "MainWindow")
    , m_testSiteControlDock(nullptr)
    , m_rowSpinBox(nullptr)
    , m_colSpinBox(nullptr)
    , m_totalSitesLabel(nullptr)
    , m_blockRowSpinBox(nullptr)
    , m_blockColSpinBox(nullptr)
{
    setupUi();
    createMenus();
    createStatusBar();
    createDockWidgets();
    createTestSiteControlDock();
    connectSignals();
    setupTestSites();
    
    // 启动时间更新定时器
    connect(m_timeUpdateTimer, &QTimer::timeout, this, &MainWindow::updateTime);
    m_timeUpdateTimer->start(1000); // 每秒更新一次

    // 注册 Logger sink，将日志发送到 UI
    TesterFramework::Logger::getInstance().addSink([this](TesterFramework::LogLevel lvl, const std::string &msg){
        int uiLevel = 3; // default debug/trace
        switch (lvl) {
            case TesterFramework::LogLevel::Info: uiLevel = 0; break;
            case TesterFramework::LogLevel::Warning: uiLevel = 1; break;
            case TesterFramework::LogLevel::Error:
            case TesterFramework::LogLevel::Critical: uiLevel = 2; break;
            default: uiLevel = 3; break;
        }

        QString qMsg = QString::fromStdString(msg);
        QMetaObject::invokeMethod(this, [this, uiLevel, qMsg](){
            appendLog(qMsg, uiLevel);
        }, Qt::QueuedConnection);
    });
}

MainWindow::~MainWindow()
{
    // 如果对话框正在显示，先关闭它
    if (m_deviceManagerDialog) {
        m_deviceManagerDialog->close();
        // 不需要手动删除，因为它的父对象是this，Qt会自动处理
    }
}

void MainWindow::setCoreEngine(std::shared_ptr<Core::CoreEngine> engine)
{
    m_coreEngine = engine;
    
    // 重新建立信号连接，因为在构造函数中调用 connectSignals() 时 m_coreEngine 还是 nullptr
    connectSignals();
    
    // 如果 DeviceManagerDialog 在构造函数中未正确初始化（因为 m_coreEngine 为空）
    // 可以在这里创建或重新创建它
    if (m_coreEngine && !m_deviceManagerDialog) {
        m_deviceManagerDialog = new DeviceManagerDialog(m_coreEngine, this);
    }

    // <-- 3. 在这里设置 DutManager
    if (m_coreEngine && m_dutMonitorWidget) {
        m_dutMonitorWidget->setDutManager(m_coreEngine->getDutManager());
    }
}

void MainWindow::setupUi()
{
    // 创建工具栏
    QToolBar *mainToolBar = addToolBar("主工具栏");
    mainToolBar->setMovable(false);
    
    // 添加应用程序Logo
    QLabel *logoLabel = new QLabel(this);
    QPixmap logoPixmap(":/resources/icons/Acroview.png");
    // 缩放logo到合适大小（64x64像素）
    logoPixmap = logoPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    logoLabel->setPixmap(logoPixmap);
    logoLabel->setToolTip("Acroview 测试系统框架");
    mainToolBar->addWidget(logoLabel);
    
    // 添加分隔符
    mainToolBar->addSeparator();
    
    m_startButton = new QPushButton("开始流程", this);
    m_startButton->setIcon(QIcon(":/resources/icons/play.png"));
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::onStartProcess);
    mainToolBar->addWidget(m_startButton);
    
    m_pauseButton = new QPushButton("暂停流程", this);
    m_pauseButton->setIcon(QIcon(":/resources/icons/pause.png"));
    m_pauseButton->setEnabled(false);
    connect(m_pauseButton, &QPushButton::clicked, this, &MainWindow::onPauseProcess);
    mainToolBar->addWidget(m_pauseButton);
    
    m_resumeButton = new QPushButton("继续流程", this);
    m_resumeButton->setIcon(QIcon(":/resources/icons/play.png"));
    m_resumeButton->setEnabled(false);
    m_resumeButton->setVisible(false);
    connect(m_resumeButton, &QPushButton::clicked, this, &MainWindow::onResumeProcess);
    mainToolBar->addWidget(m_resumeButton);
    
    m_stopButton = new QPushButton("停止流程", this);
    m_stopButton->setIcon(QIcon(":/resources/icons/stop.png"));
    m_stopButton->setEnabled(false);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::onStopProcess);
    mainToolBar->addWidget(m_stopButton);

    // 模拟器快速启动按钮
    m_simButton = new QPushButton("开始模拟", this);
    m_simButton->setIcon(QIcon(":/resources/icons/simulation.png"));
    connect(m_simButton, &QPushButton::clicked, this, &MainWindow::onStartSimulation);
    mainToolBar->addWidget(m_simButton);

    // 将DutMonitorWidget设置为主窗口的中央部件
    m_dutMonitorWidget = new DutMonitorWidget(this);
    setCentralWidget(m_dutMonitorWidget);
}

void MainWindow::createMenus()
{
    // 文件菜单
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");
    
    QAction *loadWorkflowAction = fileMenu->addAction("加载工作流(&L)...");
    loadWorkflowAction->setIcon(QIcon(":/resources/icons/open.png"));
    loadWorkflowAction->setShortcut(QKeySequence::Open);
    connect(loadWorkflowAction, &QAction::triggered, this, &MainWindow::onLoadWorkflow);
    
    QAction *loadDeviceConfigAction = fileMenu->addAction("加载设备配置(&D)...");
    loadDeviceConfigAction->setIcon(QIcon(":/resources/icons/open.png"));
    connect(loadDeviceConfigAction, &QAction::triggered, this, &MainWindow::onLoadDeviceConfig);
    
    fileMenu->addSeparator();
    
    QAction *exitAction = fileMenu->addAction("退出(&X)");
    exitAction->setIcon(QIcon(":/resources/icons/exit.png"));
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onExit);
    
    // 控制菜单
    QMenu *controlMenu = menuBar()->addMenu("控制(&C)");
    
    QAction *manualControlAction = controlMenu->addAction("手动控制(&M)...");
    manualControlAction->setIcon(QIcon(":/resources/icons/control.png"));
    connect(manualControlAction, &QAction::triggered, this, &MainWindow::onManualControl);
    
    QAction *deviceControlAction = controlMenu->addAction("设备控制(&D)...");
    deviceControlAction->setIcon(QIcon(":/resources/icons/device.png"));
    connect(deviceControlAction, &QAction::triggered, this, &MainWindow::onDeviceControl);

    QAction *acAutoSettingAction = controlMenu->addAction("AC自动设置(&S)...");
    acAutoSettingAction->setIcon(QIcon(":/resources/icons/settings.png"));
    connect(acAutoSettingAction, &QAction::triggered, this, &MainWindow::onAcAutoSetting);
    
    // 日志菜单
    QMenu *logMenu = menuBar()->addMenu("日志(&L)");
    QAction *logConfigAction = logMenu->addAction("模块日志级别(&M)...");
    logConfigAction->setIcon(QIcon(":/resources/icons/settings.png"));
    connect(logConfigAction, &QAction::triggered, this, &MainWindow::onLogConfig);

    // 帮助菜单
    QMenu *helpMenu = menuBar()->addMenu("帮助(&H)");
    
    QAction *aboutAction = helpMenu->addAction("关于(&A)...");
    aboutAction->setIcon(QIcon(":/resources/icons/about.png"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::createStatusBar()
{
    m_statusLabel = new QLabel("就绪", this);
    statusBar()->addWidget(m_statusLabel);

    m_deviceStatusLabel = new QLabel("设备: 0/0", this);
    statusBar()->addPermanentWidget(m_deviceStatusLabel); // 放置在右侧
    
    // 定期更新状态
    QTimer *statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, &MainWindow::updateStatus);
    statusTimer->start(1000); // 每秒更新一次
}

void MainWindow::createDockWidgets()
{
    // 创建新的"当前活动"停靠窗口
    QDockWidget *activeTasksDock = new QDockWidget("当前活动", this);
    m_activeTasksTable = new QTableWidget(activeTasksDock);
    m_activeTasksTable->setColumnCount(3);
    m_activeTasksTable->setHorizontalHeaderLabels({"实例ID", "目标DUT", "当前步骤"});
    m_activeTasksTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_activeTasksTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    activeTasksDock->setWidget(m_activeTasksTable);
    addDockWidget(Qt::RightDockWidgetArea, activeTasksDock);

    // 创建生产状态停靠窗口
    QDockWidget *productionDock = new QDockWidget("生产状态", this);
    createProductionStatusWidget(productionDock);
    addDockWidget(Qt::TopDockWidgetArea, productionDock);

    // 日志窗口作为可停靠的Dock
    m_logDock = new QDockWidget("实时日志", this);
    m_logView = new QPlainTextEdit(m_logDock);
    m_logView->setReadOnly(true);
    m_logView->setMaximumBlockCount(2000); // 增加日志行数
    m_logDock->setWidget(m_logView);
    addDockWidget(Qt::BottomDockWidgetArea, m_logDock);
    
    // 默认给窗口合适的尺寸
    resizeDocks({m_logDock}, {200}, Qt::Vertical);
    resizeDocks({activeTasksDock}, {400}, Qt::Horizontal);
    resizeDocks({productionDock}, {150}, Qt::Vertical);
}

void MainWindow::connectSignals()
{
    if (!m_coreEngine) return;
    
    // 连接核心引擎信号
    connect(m_coreEngine.get(), &Core::CoreEngine::engineStatusChanged,
            this, [this](Core::CoreEngine::EngineStatus status) {
                onEngineStatusChanged(static_cast<int>(status));
            });
    connect(m_coreEngine.get(), &Core::CoreEngine::processStarted,
            this, &MainWindow::onProcessStarted);
    connect(m_coreEngine.get(), &Core::CoreEngine::processCompleted,
            this, &MainWindow::onProcessCompleted);
    connect(m_coreEngine.get(), &Core::CoreEngine::processFailed,
            this, &MainWindow::onProcessFailed);
    connect(m_coreEngine.get(), &Core::CoreEngine::deviceStatusUpdate,
            this, &MainWindow::onDeviceStatusUpdate);
    connect(m_coreEngine.get(), &Core::CoreEngine::testResultAvailable,
            this, &MainWindow::onTestResultAvailable);
    
    // 获取工作流管理器并连接信号
    if (m_coreEngine) { // 检查 m_coreEngine 是否有效
        auto workflowManager = m_coreEngine->getWorkflowManager();
        if (workflowManager) {
            connect(workflowManager.get(), &Services::WorkflowManager::workflowStarted,
                    this, &MainWindow::onWorkflowStarted);
            connect(workflowManager.get(), &Services::WorkflowManager::workflowCompleted,
                    this, &MainWindow::onWorkflowEnded);
            connect(workflowManager.get(), &Services::WorkflowManager::workflowFailed,
                    this, [this](const QString& instanceId, const QString& error){
                        onWorkflowEnded(instanceId);
                    });
            connect(workflowManager.get(), &Services::WorkflowManager::stepStarted,
                    this, &MainWindow::onStepStarted);
        }
    }
}

void MainWindow::onLoadWorkflow()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("选择工作流配置文件"), 
        "config",
        tr("JSON文件 (*.json)"));
    
    if (!fileName.isEmpty()) {
        m_currentWorkflowPath = fileName;
        appendLog(QString("已加载工作流配置: %1").arg(fileName), 0);
    }
}

void MainWindow::onLoadDeviceConfig()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("选择设备配置文件"),
        "config", 
        tr("JSON文件 (*.json)"));
    
    if (!fileName.isEmpty()) {
        appendLog(QString("已加载设备配置: %1").arg(fileName), 0);
        if (m_coreEngine) {
            if (m_coreEngine->reloadDeviceConfiguration(fileName)) {
                appendLog(QString("核心引擎已重新加载设备配置: %1").arg(fileName), 0);
                // updateDeviceTree(); // m_deviceTree 已被移除，此调用不再需要
            } else {
                appendLog(QString("核心引擎重新加载设备配置失败: %1").arg(fileName), 2); // 2 for ERROR level
                QMessageBox::warning(this, tr("加载失败"), tr("无法重新加载设备配置文件。"));
            }
        } else {
            appendLog("核心引擎未初始化，无法重新加载设备配置。", 1); // 1 for WARNING level
        }
    }
}

void MainWindow::onExit()
{
    close();
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, tr("关于"),
        tr("测试系统框架 v1.0\n\n"
           "基于Qt/C++的模块化测试系统\n"
           "支持多种设备接入和灵活的工作流配置"));
}

void MainWindow::onStartProcess()
{
    if (!m_coreEngine || !m_coreEngine->isReady()) {
        QMessageBox::warning(this, tr("警告"), tr("系统未就绪，无法启动测试"));
        return;
    }
    
    // The old startAutoProcess is deprecated.
    // The new architecture uses a dedicated workflow for system initialization.
    if (m_coreEngine->startSystemInitialization()) {
        m_isProcessRunning = true;
        m_startButton->setEnabled(false);
        m_pauseButton->setEnabled(true);
        m_stopButton->setEnabled(true);
    } else {
        QMessageBox::critical(this, tr("错误"), tr("启动系统初始化流程失败，请检查日志。"));
    }
}

void MainWindow::onStopProcess()
{
    // This is now more complex as we can have multiple running workflow instances.
    // For now, this button might stop ALL workflows, or we need a UI to select one.
    // To fix compilation, the direct call is removed.
    // TODO: Implement instance-specific controls.
    QMessageBox::information(this, tr("功能调整"), tr("停止流程的功能已调整，请在未来的版本中通过任务列表进行实例控制。"));
    
    // As a temporary measure, let's just reset the button states
    m_isProcessRunning = false;
    m_startButton->setEnabled(true);
    m_pauseButton->setEnabled(false);
    m_resumeButton->setEnabled(false);
    m_stopButton->setEnabled(false);
}

void MainWindow::onPauseProcess()
{
    QMessageBox::information(this, tr("功能调整"), tr("暂停流程的功能已调整。"));
    // To fix compilation, the direct call is removed.
}

void MainWindow::onResumeProcess()
{
    QMessageBox::information(this, tr("功能调整"), tr("恢复流程的功能已调整。"));
    // To fix compilation, the direct call is removed.
}

void MainWindow::onManualControl()
{
    // TODO: 实现手动控制对话框
    QMessageBox::information(this, tr("手动控制"), tr("手动控制功能尚未实现"));
}

void MainWindow::onDeviceControl()
{
    // 统一使用onDeviceManager的实现
    onDeviceManager();
}

void MainWindow::onAcAutoSetting()
{
    ACAutomaticSetting dialog(this);
    dialog.exec();
    // Form *dialog = new Form();
    // dialog->show();
}

void MainWindow::onEngineStatusChanged(int status)
{
    Core::CoreEngine::EngineStatus engineStatus = static_cast<Core::CoreEngine::EngineStatus>(status);
    QString statusText;
    
    switch (engineStatus) {
        case Core::CoreEngine::EngineStatus::NotInitialized:
            statusText = "未初始化";
            m_startButton->setEnabled(false);
            break;
        case Core::CoreEngine::EngineStatus::Initializing:
            statusText = "正在初始化...";
            m_startButton->setEnabled(false);
            break;
        case Core::CoreEngine::EngineStatus::Ready:
            statusText = "就绪";
            m_startButton->setEnabled(true);
            break;
        case Core::CoreEngine::EngineStatus::Running:
            statusText = "运行中";
            m_startButton->setEnabled(false);
            break;
        case Core::CoreEngine::EngineStatus::Error:
            statusText = "错误";
            m_startButton->setEnabled(true);
            break;
    }
    
    m_statusLabel->setText(statusText);
}

void MainWindow::onProcessStarted(const QString &processName)
{
    appendLog(QString("流程已启动: %1").arg(processName), 0);
}

void MainWindow::onProcessCompleted(const QString &processName)
{
    appendLog(QString("流程已完成: %1").arg(processName), 0);
    QMessageBox::information(this, tr("完成"), tr("测试流程 '%1' 已完成").arg(processName));
    
    m_isProcessRunning = false;
    m_startButton->setEnabled(true);
    m_pauseButton->setEnabled(false);
    m_stopButton->setEnabled(false);
}

void MainWindow::onProcessFailed(const QString &processName, const QString &error)
{
    appendLog(QString("流程失败: %1 - %2").arg(processName).arg(error), 2);
    QMessageBox::critical(this, tr("错误"), tr("测试流程 '%1' 失败:\n%2").arg(processName).arg(error));
    
    m_isProcessRunning = false;
    m_startButton->setEnabled(true);
    m_pauseButton->setEnabled(false);
    m_stopButton->setEnabled(false);
}

void MainWindow::onDeviceStatusUpdate(const QString &deviceId, const QJsonObject &status)
{
    // updateDeviceTree() 已被移除，因为 m_deviceTree 不再存在
}

void MainWindow::onTestResultAvailable(const QJsonObject &result)
{
    // TODO: 更新测试结果表格
    appendLog("收到测试结果", 0);
}

void MainWindow::onStepStarted(const QString &instanceId, int stepIndex, const QString &stepName)
{
    // 更新"当前活动"面板
    for (int row = 0; row < m_activeTasksTable->rowCount(); ++row) {
        if (m_activeTasksTable->item(row, 0)->text() == instanceId) {
            m_activeTasksTable->item(row, 2)->setText(stepName);
            break;
        }
    }
    appendLog(QString("实例 '%1' -> 步骤开始: [%2] %3").arg(instanceId).arg(stepIndex + 1).arg(stepName), 0);
}

void MainWindow::onWorkflowStarted(const QString& instanceId)
{
    auto context = m_coreEngine->getWorkflowManager()->getWorkflowContext(instanceId);
    if (!context) return;
    
    QVariant dutIdVar = context->getData("dutId");
    QString dutId = dutIdVar.isValid() ? dutIdVar.toString() : "N/A";

    int newRow = m_activeTasksTable->rowCount();
    m_activeTasksTable->insertRow(newRow);
    m_activeTasksTable->setItem(newRow, 0, new QTableWidgetItem(instanceId));
    m_activeTasksTable->setItem(newRow, 1, new QTableWidgetItem(dutId));
    m_activeTasksTable->setItem(newRow, 2, new QTableWidgetItem("Initializing..."));
}

void MainWindow::onWorkflowEnded(const QString& instanceId)
{
    for (int row = 0; row < m_activeTasksTable->rowCount(); ++row) {
        if (m_activeTasksTable->item(row, 0)->text() == instanceId) {
            m_activeTasksTable->removeRow(row);
            break;
        }
    }
}

void MainWindow::updateStatus()
{
    if (!m_coreEngine) return;
    
    // 更新设备连接状态
    QJsonObject allStatus = m_coreEngine->getAllDevicesStatus();
    if (allStatus.contains("devices")) {
        QJsonObject devices = allStatus["devices"].toObject();
        int total = devices.size();
        int connected = 0;
        for (const auto& deviceStatusVal : devices) {
            QJsonObject deviceStatus = deviceStatusVal.toObject();
            if (deviceStatus["status"].toInt() != 0) { // 0 is Disconnected
                connected++;
            }
        }
        m_deviceStatusLabel->setText(QString("设备: %1/%2").arg(connected).arg(total));
    }
}

void MainWindow::appendLog(const QString &message, int level)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString levelText;
    QString color;
    
    switch (level) {
        case 0: // INFO
            levelText = "INFO";
            color = "black";
            break;
        case 1: // WARNING
            levelText = "WARN";
            color = "orange";
            break;
        case 2: // ERROR
            levelText = "ERROR";
            color = "red";
            break;
        default:
            levelText = "DEBUG";
            color = "gray";
    }
    
    QString formattedMessage = QString("<span style='color: gray'>[%1]</span> "
                                     "<span style='color: %2'>[%3]</span> %4")
                                     .arg(timestamp)
                                     .arg(color)
                                     .arg(levelText)
                                     .arg(message);
    
    m_logView->appendHtml(formattedMessage);
}

void MainWindow::onDeviceManager() {
    if (!m_deviceManagerDialog) {
        // 尝试再次创建，以防 setCoreEngine 在 setupUi 之后被调用
        if (m_coreEngine) {
            m_deviceManagerDialog = new DeviceManagerDialog(m_coreEngine, this);
            // 连接关闭信号，在对话框关闭时更新日志
            connect(m_deviceManagerDialog, &QDialog::finished, this, [this]() {
                appendLog("设备配置已更新。", 0);
            });
        } else {
            LOG_MODULE_WARNING("MainWindow", "Cannot open DeviceManagerDialog: CoreEngine is not available.");
            QMessageBox::warning(this, tr("错误"), tr("核心引擎未初始化，无法打开设备管理器。"));
            return;
        }
    }
    
    // 使用show()而不是exec()，避免模态对话框可能的问题
    m_deviceManagerDialog->show();
    m_deviceManagerDialog->raise();
    m_deviceManagerDialog->activateWindow();
}

void MainWindow::onLogConfig()
{
    LogConfigDialog dlg(this);
    // 初始复选框状态
    connect(&dlg, &LogConfigDialog::logViewVisibilityChanged, this, [this](bool visible){
        if (m_logDock) m_logDock->setVisible(visible);
    });
    dlg.exec();
}

void MainWindow::onStartSimulation()
{
    if (!m_coreEngine || !m_coreEngine->isReady()) {
        QMessageBox::warning(this, tr("警告"), tr("系统未就绪，无法启动模拟"));
        return;
    }
    
    if (m_coreEngine->startSimulation()) {
        appendLog("开始1000芯片生产模拟", 0);
        m_simButton->setEnabled(false);
        
        // 重置生产状态显示
        m_totalChipsLabel->setText("目标数量: 1000");
        m_processedChipsLabel->setText("已处理: 0");
        m_passedChipsLabel->setText("合格: 0");
        m_failedChipsLabel->setText("不合格: 0");
        m_productionProgress->setMaximum(1000);
        m_productionProgress->setValue(0);
        m_currentThroughputLabel->setText("吞吐率: 0 chips/min");
        m_sitesStatusLabel->setText("站点状态: 等待中...");
        
        // 启动状态更新定时器
        QTimer *productionTimer = new QTimer(this);
        connect(productionTimer, &QTimer::timeout, this, &MainWindow::updateProductionStatus);
        productionTimer->start(2000); // 每2秒更新一次
        
    } else {
        QMessageBox::critical(this, tr("错误"), tr("启动模拟失败，请检查日志"));
    }
}

void MainWindow::createProductionStatusWidget(QDockWidget *parent)
{
    QWidget *statusWidget = new QWidget(parent);
    QVBoxLayout *layout = new QVBoxLayout(statusWidget);
    
    // 创建生产进度条
    m_productionProgress = new QProgressBar(statusWidget);
    m_productionProgress->setMinimum(0);
    m_productionProgress->setMaximum(1000);
    m_productionProgress->setValue(0);
    m_productionProgress->setTextVisible(true);
    m_productionProgress->setFormat("进度: %v/%m (%p%)");
    layout->addWidget(m_productionProgress);
    
    // 创建统计标签
    QHBoxLayout *statsLayout = new QHBoxLayout();
    
    m_totalChipsLabel = new QLabel("目标数量: 1000", statusWidget);
    m_processedChipsLabel = new QLabel("已处理: 0", statusWidget);
    m_passedChipsLabel = new QLabel("合格: 0", statusWidget);
    m_failedChipsLabel = new QLabel("不合格: 0", statusWidget);
    
    statsLayout->addWidget(m_totalChipsLabel);
    statsLayout->addWidget(m_processedChipsLabel);
    statsLayout->addWidget(m_passedChipsLabel);
    statsLayout->addWidget(m_failedChipsLabel);
    
    layout->addLayout(statsLayout);
    
    // 创建性能指标
    QHBoxLayout *perfLayout = new QHBoxLayout();
    
    m_currentThroughputLabel = new QLabel("吞吐率: 0 chips/min", statusWidget);
    m_sitesStatusLabel = new QLabel("站点状态: 未启动", statusWidget);
    
    perfLayout->addWidget(m_currentThroughputLabel);
    perfLayout->addWidget(m_sitesStatusLabel);
    
    layout->addLayout(perfLayout);
    
    parent->setWidget(statusWidget);
}

void MainWindow::updateProductionStatus()
{
    if (!m_coreEngine) return;
    
    // 从DutManager获取统计信息
    auto dutManager = m_coreEngine->getDutManager();
    if (!dutManager) return;
    
    // 这里应该从DutManager获取实际的统计数据
    // 由于DutManager可能没有这些统计方法，我们使用模拟数据
    static int lastProcessed = 0;
    static qint64 lastUpdateTime = QDateTime::currentMSecsSinceEpoch();
    
    // 模拟获取数据
    int totalChips = 1000;
    int processed = lastProcessed + (rand() % 5); // 模拟处理进度
    int passed = processed * 0.9; // 90%合格率
    int failed = processed - passed;
    
    if (processed > totalChips) processed = totalChips;
    if (passed > processed) passed = processed;
    
    // 计算吞吐率
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    double elapsedMinutes = (currentTime - lastUpdateTime) / 60000.0;
    double throughput = elapsedMinutes > 0 ? (processed - lastProcessed) / elapsedMinutes : 0;
    
    // 更新显示
    m_productionProgress->setValue(processed);
    m_processedChipsLabel->setText(QString("已处理: %1").arg(processed));
    m_passedChipsLabel->setText(QString("合格: %1").arg(passed));
    m_failedChipsLabel->setText(QString("不合格: %1").arg(failed));
    m_currentThroughputLabel->setText(QString("吞吐率: %1 chips/min").arg(throughput, 0, 'f', 1));
    
    // 获取站点状态
    QJsonObject allStatus = m_coreEngine->getAllDevicesStatus();
    int activeSites = 0;
    if (allStatus.contains("devices")) {
        QJsonObject devices = allStatus["devices"].toObject();
        for (const auto& deviceStatusVal : devices) {
            QJsonObject deviceStatus = deviceStatusVal.toObject();
            if (deviceStatus["status"].toInt() == 2) { // 2 is Busy
                activeSites++;
            }
        }
    }
    
    m_sitesStatusLabel->setText(QString("活跃站点: %1/8").arg(activeSites));
    
    lastProcessed = processed;
    lastUpdateTime = currentTime;
    
    // 如果完成了所有芯片，重启模拟按钮
    if (processed >= totalChips) {
        m_simButton->setEnabled(true);
        appendLog("生产模拟完成！", 0);
    }
}

// =================== TestSite 相关方法实现 ===================

void MainWindow::setupTestSites()
{
    int siteRows = m_settings.value("Grid/SiteRows", 2).toInt();
    int siteCols = m_settings.value("Grid/SiteCols", 4).toInt();

    // 清理现有的测试站点
    qDeleteAll(m_testSites);
    m_testSites.clear();

    // 如果滚动区域还不存在，将在createTestSiteControlDock中创建
    if (!m_testSiteScrollArea) {
        return; // 等待dock创建时再初始化
    }

    // 创建内容容器
    QWidget* scrollAreaContent = new QWidget();
    QGridLayout* gridLayout = new QGridLayout(scrollAreaContent);
    gridLayout->setSpacing(8); // 增加TestSite之间的间距
    gridLayout->setContentsMargins(10, 10, 10, 10); // 增加边距
    gridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft); // 左上对齐

    // 获取block布局设置
    int blockRows = m_settings.value("Grid/BlockRows", 3).toInt();
    int blockCols = m_settings.value("Grid/BlockCols", 8).toInt();
    
    // 创建测试站点
    for (int i = 0; i < siteRows * siteCols; ++i) {
        TestSite* site = new TestSite(i + 1, this);
        
        // 设置TestSite内部的block布局
        site->setBlockLayout(blockRows, blockCols);
        
        // 连接信号
        connect(site, &TestSite::startClicked, this, &MainWindow::onTestSiteStartClicked);
        connect(site, &TestSite::siteInfoChanged, this, &MainWindow::onSiteInfoChanged);
        connect(site, &TestSite::adapterEnableChanged, this, &MainWindow::onAdapterEnableChanged);
        connect(site, &TestSite::deviceCommandRequested, this, &MainWindow::onDeviceCommandRequested);
        
        // 如果有可用设备，为站点分配设备
        if (m_coreEngine) {
            auto deviceManager = m_coreEngine->getDeviceManager();
            if (deviceManager) {
                QStringList deviceIds = deviceManager->getAllDeviceIds();
                if (i < deviceIds.size()) {
                    auto device = deviceManager->getDevice(deviceIds[i]);
                    if (device) {
                        site->setDevice(device);
                    }
                }
            }
        }
        
        // 移除固定高度，让TestSite根据内容自动调整
        // site->setFixedHeight(120);  // 注释掉固定高度
        site->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        
        m_testSites.append(site);
        gridLayout->addWidget(site, i / siteCols, i % siteCols);
    }

    scrollAreaContent->setLayout(gridLayout);
    m_testSiteScrollArea->setWidget(scrollAreaContent);
    
    appendLog(QString("已创建 %1 个测试站点").arg(m_testSites.size()), 0);
}

void MainWindow::updateTime()
{
    QString currentTime = QDateTime::currentDateTime().toString("hh:mm:ss");
    for (TestSite* site : m_testSites) {
        if (site) {
            site->updateTime(currentTime);
        }
    }
}

void MainWindow::createTestSiteControlDock()
{
    // 创建TestSite作为右侧dock窗口
    m_testSiteControlDock = new QDockWidget("测试站点控制", this);
    QWidget *dockWidget = new QWidget();
    
    QVBoxLayout *centralLayout = new QVBoxLayout(dockWidget);
    centralLayout->setContentsMargins(5, 5, 5, 5);
    
    // 创建标题和控制区域
    QHBoxLayout *headerLayout = new QHBoxLayout();
    
    QLabel *titleLabel = new QLabel("测试站点控制", this);
    titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333;");
    headerLayout->addWidget(titleLabel);
    
    headerLayout->addStretch(); // 添加弹性空间
    
    // 添加行列设置控件
    QLabel *rowLabel = new QLabel("行数:", this);
    m_rowSpinBox = new QSpinBox(this);
    m_rowSpinBox->setMinimum(1);
    m_rowSpinBox->setMaximum(10);
    m_rowSpinBox->setValue(m_settings.value("Grid/SiteRows", 2).toInt());
    m_rowSpinBox->setToolTip("设置测试站点的行数");
    
    QLabel *colLabel = new QLabel("列数:", this);
    m_colSpinBox = new QSpinBox(this);
    m_colSpinBox->setMinimum(1);
    m_colSpinBox->setMaximum(10);
    m_colSpinBox->setValue(m_settings.value("Grid/SiteCols", 4).toInt());
    m_colSpinBox->setToolTip("设置测试站点的列数");
    
    // 添加分割线
    QFrame *separator1 = new QFrame();
    separator1->setFrameShape(QFrame::VLine);
    separator1->setFrameShadow(QFrame::Sunken);
    
    // 添加TestSite内部block布局设置
    QLabel *blockRowLabel = new QLabel("Block行:", this);
    m_blockRowSpinBox = new QSpinBox(this);
    m_blockRowSpinBox->setMinimum(1);
    m_blockRowSpinBox->setMaximum(10);
    m_blockRowSpinBox->setValue(m_settings.value("Grid/BlockRows", 3).toInt());
    m_blockRowSpinBox->setToolTip("设置每个测试站点内部socket的行数");
    
    QLabel *blockColLabel = new QLabel("Block列:", this);
    m_blockColSpinBox = new QSpinBox(this);
    m_blockColSpinBox->setMinimum(1);
    m_blockColSpinBox->setMaximum(20);
    m_blockColSpinBox->setValue(m_settings.value("Grid/BlockCols", 8).toInt());
    m_blockColSpinBox->setToolTip("设置每个测试站点内部socket的列数");
    
    QPushButton *applyButton = new QPushButton("应用", this);
    applyButton->setToolTip("应用新的行列设置");
    
    // 设置控件样式
    QString spinBoxStyle = "QSpinBox { "
                          "padding: 3px; "
                          "border: 1px solid #ccc; "
                          "border-radius: 3px; "
                          "min-width: 50px; "
                          "}";
    
    QString buttonStyle = "QPushButton { "
                         "padding: 5px 15px; "
                         "margin: 2px; "
                         "border: 1px solid #ccc; "
                         "border-radius: 3px; "
                         "background-color: #4CAF50; "
                         "color: white; "
                         "font-weight: bold; "
                         "} "
                         "QPushButton:hover { "
                         "background-color: #45a049; "
                         "} "
                         "QPushButton:pressed { "
                         "background-color: #3d8b40; "
                         "}";
    
    m_rowSpinBox->setStyleSheet(spinBoxStyle);
    m_colSpinBox->setStyleSheet(spinBoxStyle);
    m_blockRowSpinBox->setStyleSheet(spinBoxStyle);
    m_blockColSpinBox->setStyleSheet(spinBoxStyle);
    applyButton->setStyleSheet(buttonStyle);
    
    // 连接应用按钮
    connect(applyButton, &QPushButton::clicked, this, [this]() {
        int rows = m_rowSpinBox->value();
        int cols = m_colSpinBox->value();
        int blockRows = m_blockRowSpinBox->value();
        int blockCols = m_blockColSpinBox->value();
        
        // 保存设置
        m_settings.setValue("Grid/SiteRows", rows);
        m_settings.setValue("Grid/SiteCols", cols);
        m_settings.setValue("Grid/BlockRows", blockRows);
        m_settings.setValue("Grid/BlockCols", blockCols);
        
        // 重新创建站点
        setupTestSites();
        
        appendLog(QString("已更新布局: 站点 %1行x%2列，每站点Block %3行x%4列")
                  .arg(rows).arg(cols).arg(blockRows).arg(blockCols), 0);
    });
    
    // 添加站点总数显示
    m_totalSitesLabel = new QLabel(this);
    updateTotalSitesLabel();
    
    // 连接数值变化信号到总数更新
    connect(m_rowSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updateTotalSitesLabel);
    connect(m_colSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updateTotalSitesLabel);
    connect(m_blockRowSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updateTotalSitesLabel);
    connect(m_blockColSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updateTotalSitesLabel);
    
    headerLayout->addWidget(rowLabel);
    headerLayout->addWidget(m_rowSpinBox);
    headerLayout->addWidget(colLabel);
    headerLayout->addWidget(m_colSpinBox);
    headerLayout->addWidget(separator1);
    headerLayout->addWidget(blockRowLabel);
    headerLayout->addWidget(m_blockRowSpinBox);
    headerLayout->addWidget(blockColLabel);
    headerLayout->addWidget(m_blockColSpinBox);
    headerLayout->addWidget(m_totalSitesLabel);
    headerLayout->addWidget(applyButton);
    
    centralLayout->addLayout(headerLayout);
    
    // 创建分割线
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("color: #ccc;");
    centralLayout->addWidget(line);
    
    // 创建滚动区域用于TestSite
    m_testSiteScrollArea = new QScrollArea(this);
    m_testSiteScrollArea->setWidgetResizable(true);
    m_testSiteScrollArea->setStyleSheet("QScrollArea { "
                                       "border: 1px solid #ccc; "
                                       "border-radius: 5px; "
                                       "background-color: #fafafa; "
                                       "}");
    
    centralLayout->addWidget(m_testSiteScrollArea, 1); // 给滚动区域最大的空间权重
    
    dockWidget->setLayout(centralLayout);
    m_testSiteControlDock->setWidget(dockWidget);
    
    // 将TestSite dock添加到右侧
    addDockWidget(Qt::RightDockWidgetArea, m_testSiteControlDock);
    
    // 现在可以初始化TestSite了
    setupTestSites();
}

// =================== TestSite 相关槽函数实现 ===================

void MainWindow::onTestSiteStartClicked()
{
    TestSite* site = qobject_cast<TestSite*>(sender());
    if (!site) return;
    
    appendLog(QString("测试站点 %1 开始测试").arg(site->property("siteNumber").toInt()), 0);
    
    // 如果站点有关联的设备，可以执行相应的测试流程
    auto device = site->getDevice();
    if (device && m_coreEngine) {
        // 执行设备命令或启动工作流
        QString deviceId = site->getDeviceId();
        if (!deviceId.isEmpty()) {
            // 这里可以启动特定的测试工作流
            appendLog(QString("为设备 %1 启动测试流程").arg(deviceId), 0);
        }
    }
}

void MainWindow::onSiteInfoChanged(int siteNumber, const TestSite::SiteInfo& siteInfo)
{
    appendLog(QString("站点 %1 信息更新: %2").arg(siteNumber).arg(siteInfo.deviceAlias), 0);
}

void MainWindow::onAdapterEnableChanged(int siteNumber, quint32 adapterEnable)
{
    appendLog(QString("站点 %1 适配器使能状态: 0x%2")
              .arg(siteNumber)
              .arg(adapterEnable, 8, 16, QChar('0')), 0);
}

void MainWindow::onDeviceCommandRequested(const QString& deviceId, const QString& command, const QJsonObject& params)
{
    if (!m_coreEngine) {
        appendLog("核心引擎未初始化，无法执行设备命令", 2);
        return;
    }
    
    QJsonObject result = m_coreEngine->executeDeviceCommand(deviceId, command, params);
    bool success = result.value("success").toBool();
    
    if (success) {
        appendLog(QString("设备命令执行成功: %1 - %2").arg(deviceId).arg(command), 0);
    } else {
        QString error = result.value("error").toString();
        appendLog(QString("设备命令执行失败: %1 - %2: %3").arg(deviceId).arg(command).arg(error), 2);
    }
}

void MainWindow::updateTotalSitesLabel()
{
    int rows = m_rowSpinBox->value();
    int cols = m_colSpinBox->value();
    int blockRows = m_blockRowSpinBox ? m_blockRowSpinBox->value() : 3;
    int blockCols = m_blockColSpinBox ? m_blockColSpinBox->value() : 8;
    
    m_totalSitesLabel->setText(QString("站点: %1 (每站点 %2x%3=%4 sockets)")
                              .arg(rows * cols)
                              .arg(blockRows).arg(blockCols)
                              .arg(blockRows * blockCols));
}

} // namespace Presentation
