#include "ui/MainWindow.h"
#include "core/CoreEngine.h" // 包含 CoreEngine.h
#include "services/WorkflowManager.h" // 新增
#include "services/DeviceManager.h" // 添加 DeviceManager.h
#include "services/DutManager.h" // 添加 DutManager.h
#include "application/WorkflowContext.h" // 新增
#include "ui/DeviceManagerDialog.h" // 包含 DeviceManagerDialog.h
#include "ui/DutMonitorWidget.h" // <-- 1. 包含新的头文件
#include <QTimer> // 添加 QTimer
#include "core/Logger.h"
#include "ui/LogConfigDialog.h"
#include "ui/LogDisplayWidget.h"
#include "ui/RecipeConfigDialog.h"
#include <QFile>
#include <QJsonDocument>
#include <QDateTime>

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
#include <QScrollArea>
#include <QSettings>
#include <QTabWidget>
#include <QFrame>
#include <QSpinBox>
#include <QSizePolicy>
#include <QIcon>
#include <QPixmap>
#include <QFormLayout> // <--- 新增
#include <QLineEdit> // <--- 新增

namespace Presentation {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_coreEngine(nullptr) // 初始化 m_coreEngine
    , m_isTestRunning(false)
    , m_displayTimer(new QTimer(this))
    , m_deviceManagerDialog(nullptr) // 初始化为 nullptr
    , m_isProcessRunning(false) // 初始化 m_isProcessRunning
    , m_dutMonitorWidget(nullptr) // 初始化为 nullptr
    , m_dutMonitorDock(nullptr) // 初始化为 nullptr
    , m_timeUpdateTimer(new QTimer(this))
    , m_settings("TesterFramework", "MainWindow")
{
    
    // 设置默认窗口大小为 1920x1080
    resize(1920, 1080);
    setMinimumSize(1200, 800); // 设置最小尺寸
    
    // 关键修复：必须先创建和设置中央控件，然后才能添加DockWidgets
    createDockWidgets(); 
    setupUi();
    createMenus();
    createStatusBar();
    // connectSignals(); // 推迟到 setCoreEngine 中调用

    // 初始UI状态：加载中
    m_statusLabel->setText("正在初始化核心引擎...");
    m_startButton->setEnabled(false);
    // 启动时间更新定时器用于状态栏等UI更新 - 优化：降低频率
    connect(m_timeUpdateTimer, &QTimer::timeout, this, &MainWindow::updateStatus);
    m_timeUpdateTimer->start(3000); // 每3秒更新一次，减少性能开销

    // 【问题修复】确保所有模块的日志都能在UI上显示，特别是BurnDevice
    TesterFramework::Logger::getInstance().setModuleLevel("BurnDevice", TesterFramework::LogLevel::Debug);
    TesterFramework::Logger::getInstance().setModuleLevel("TcpChannel", TesterFramework::LogLevel::Debug);

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
    
    // 引擎就绪，连接所有必要的信号
    connectSignals();
    
    if (m_coreEngine && !m_deviceManagerDialog) {
        m_deviceManagerDialog = new DeviceManagerDialog(m_coreEngine, this);
    }

    if (m_coreEngine && m_dutMonitorWidget) {
        m_dutMonitorWidget->setDutManager(m_coreEngine->getDutManager());
        
        // 自动加载默认的设备配置和站点信息
        QString defaultConfigPath = "config/device_config_with_handler.json";
        QFile configFile(defaultConfigPath);
        
        if (configFile.exists()) {
            
            auto dutManager = m_coreEngine->getDutManager();
            if (dutManager) {
                
                if (dutManager->loadSiteConfiguration(defaultConfigPath)) {
                    auto siteInfoMap = dutManager->getAllSiteInfo();
                    
                    // 简短的站点信息日志
                    for (auto it = siteInfoMap.begin(); it != siteInfoMap.end(); ++it) {
                       // const auto& siteInfo = it.value();
                        
                    }
                } else {
                    
                }
            } else {
                
            }
        } else {
            
        }
    } else {
        
    }

    // 更新UI状态为就绪
    m_statusLabel->setText("系统就绪");
    onEngineStatusChanged(static_cast<int>(m_coreEngine->getStatus())); // 根据引擎状态更新按钮

    // 启用依赖引擎的菜单项
    m_manualControlAction->setEnabled(true);
    m_deviceControlAction->setEnabled(true);
    m_acAutoSettingAction->setEnabled(true);
}

void MainWindow::setupUi()
{
    // 创建工具栏
    QToolBar *mainToolBar = addToolBar("主工具栏");
    mainToolBar->setMovable(false);
    
    // 添加应用程序Logo
    QLabel *logoLabel = new QLabel(this);
    QPixmap logoPixmap(":/resources/icons/Acroview.png");
    if (!logoPixmap.isNull()) {
        // 缩放图标到合适大小
        logoPixmap = logoPixmap.scaled(160, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        logoLabel->setPixmap(logoPixmap);
        logoLabel->setToolTip("Acroview 测试系统框架");
    } else {
        // 如果图标加载失败，使用文字版本
        logoLabel->setText("Acroview");
        logoLabel->setStyleSheet("font-weight: bold; color: #2c3e50; padding: 5px;");
        logoLabel->setToolTip("Acroview 测试系统框架 (图标加载失败)");
    }
    mainToolBar->addWidget(logoLabel);
    
    // 添加分隔符
    mainToolBar->addSeparator();
    
    m_startButton = new QPushButton("开始流程", this);
    QIcon startIcon(":/resources/icons/play.png");
    if (!startIcon.isNull()) m_startButton->setIcon(startIcon);
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::onStartProcess);
    mainToolBar->addWidget(m_startButton);
    
    m_pauseButton = new QPushButton("暂停流程", this);
    QIcon pauseIcon(":/resources/icons/pause.png");
    if (!pauseIcon.isNull()) m_pauseButton->setIcon(pauseIcon);
    m_pauseButton->setEnabled(false);
    connect(m_pauseButton, &QPushButton::clicked, this, &MainWindow::onPauseProcess);
    mainToolBar->addWidget(m_pauseButton);
    
    m_resumeButton = new QPushButton("继续流程", this);
    QIcon resumeIcon(":/resources/icons/play.png");
    if (!resumeIcon.isNull()) m_resumeButton->setIcon(resumeIcon);
    m_resumeButton->setEnabled(false);
    m_resumeButton->setVisible(false);
    connect(m_resumeButton, &QPushButton::clicked, this, &MainWindow::onResumeProcess);
    mainToolBar->addWidget(m_resumeButton);
    
    m_stopButton = new QPushButton("停止流程", this);
    QIcon stopIcon(":/resources/icons/stop.png");
    if (!stopIcon.isNull()) m_stopButton->setIcon(stopIcon);
    m_stopButton->setEnabled(false);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::onStopProcess);
    mainToolBar->addWidget(m_stopButton);
}

void MainWindow::createMenus()
{
    // 文件菜单
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");
    
    QAction *loadWorkflowAction = fileMenu->addAction("加载工作流(&L)...");
    QIcon openIcon(":/resources/icons/open.png");
    if (!openIcon.isNull()) loadWorkflowAction->setIcon(openIcon);
    loadWorkflowAction->setShortcut(QKeySequence::Open);
    connect(loadWorkflowAction, &QAction::triggered, this, &MainWindow::onLoadWorkflow);
    
    QAction *configRecipeAction = fileMenu->addAction("配方配置(&R)...");
    QIcon settingsIcon(":/resources/icons/settings.png");
    if (!settingsIcon.isNull()) configRecipeAction->setIcon(settingsIcon);
    connect(configRecipeAction, &QAction::triggered, this, &MainWindow::onConfigRecipe);
    
    QAction *loadDeviceConfigAction = fileMenu->addAction("加载设备配置(&D)...");
    if (!openIcon.isNull()) loadDeviceConfigAction->setIcon(openIcon);
    connect(loadDeviceConfigAction, &QAction::triggered, this, &MainWindow::onLoadDeviceConfig);
    
    fileMenu->addSeparator();
    
    QAction *exitAction = fileMenu->addAction("退出(&X)");
    QIcon exitIcon(":/resources/icons/exit.png");
    if (!exitIcon.isNull()) exitAction->setIcon(exitIcon);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onExit);
    
    // 控制菜单
    QMenu *controlMenu = menuBar()->addMenu("控制(&C)");
    
    m_manualControlAction = controlMenu->addAction("手动控制(&M)...");
    QIcon controlIcon(":/resources/icons/control.png");
    if (!controlIcon.isNull()) m_manualControlAction->setIcon(controlIcon);
    connect(m_manualControlAction, &QAction::triggered, this, &MainWindow::onManualControl);
    m_manualControlAction->setEnabled(false); // 初始禁用

    m_deviceControlAction = controlMenu->addAction("设备控制(&D)...");
    QIcon deviceIcon(":/resources/icons/device.png");
    if (!deviceIcon.isNull()) m_deviceControlAction->setIcon(deviceIcon);
    connect(m_deviceControlAction, &QAction::triggered, this, &MainWindow::onDeviceControl);
    m_deviceControlAction->setEnabled(false); // 初始禁用

    m_acAutoSettingAction = controlMenu->addAction("AC自动设置(&S)...");
    QIcon acSettingsIcon(":/resources/icons/settings.png");
    if (!acSettingsIcon.isNull()) m_acAutoSettingAction->setIcon(acSettingsIcon);
    connect(m_acAutoSettingAction, &QAction::triggered, this, &MainWindow::onAcAutoSetting);
    m_acAutoSettingAction->setEnabled(false); // 初始禁用
    
    // 日志菜单
    QMenu *logMenu = menuBar()->addMenu("日志(&L)");
    QAction *logConfigAction = logMenu->addAction("模块日志级别(&M)...");
    QIcon logSettingsIcon(":/resources/icons/settings.png");
    if (!logSettingsIcon.isNull()) logConfigAction->setIcon(logSettingsIcon);
    connect(logConfigAction, &QAction::triggered, this, &MainWindow::onLogConfig);

    // 帮助菜单
    QMenu *helpMenu = menuBar()->addMenu("帮助(&H)");
    
    QAction *aboutAction = helpMenu->addAction("关于(&A)...");
    QIcon aboutIcon(":/resources/icons/about.png");
    if (!aboutIcon.isNull()) aboutAction->setIcon(aboutIcon);
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
    // 中心区域：必须先设置中央控件
    QTabWidget *tabWidget = new QTabWidget();
    setCentralWidget(tabWidget); // 将此TabWidget设为中央控件

    // 创建并添加DutMonitorWidget到第一个Tab
    m_dutMonitorWidget = new DutMonitorWidget(this);
    tabWidget->addTab(m_dutMonitorWidget, "DUT 监控面板");

    // 添加其他占位符Tab
    tabWidget->addTab(new QWidget(), "控制面板");
    tabWidget->addTab(new QWidget(), "数据统计");
    tabWidget->addTab(new QWidget(), "告警信息");
    tabWidget->addTab(new QWidget(), "测试计划");
    tabWidget->addTab(new QWidget(), "系统日志"); // 这个可以和底部的日志dock合并或替换
    tabWidget->addTab(new QWidget(), "外设管理");

    // 左侧：生产信息面板
    QDockWidget *productionInfoDock = new QDockWidget("生产信息", this);
    QWidget *productionInfoWidget = new QWidget();
    productionInfoWidget->setObjectName("productionInfoPanel"); // <--- 设置对象名称
    QFormLayout *formLayout = new QFormLayout(productionInfoWidget);
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->addRow("工单:", new QLineEdit("ABCD123"));
    formLayout->addRow("数量:", new QLineEdit("1000"));
    formLayout->addRow("良品:", new QLineEdit("950"));
    formLayout->addRow("不良:", new QLineEdit("50"));
    formLayout->addRow("OS Fail:", new QLineEdit("10"));
    formLayout->addRow("Program Fail:", new QLineEdit("10"));
    formLayout->addRow("OS通过率:", new QLineEdit("80%"));
    formLayout->addRow("Prog通过率:", new QLineEdit("80%"));
    formLayout->addRow("模式:", new QLineEdit("MES模式"));
    formLayout->addRow("自动:", new QLineEdit("自动模式"));
    formLayout->addRow("UPH:", new QLineEdit("8000"));
    formLayout->addRow("用户:", new QLineEdit("admin"));
    productionInfoDock->setWidget(productionInfoWidget);
    addDockWidget(Qt::LeftDockWidgetArea, productionInfoDock);
    // 右侧：当前活动
    QDockWidget *activeTasksDock = new QDockWidget("当前活动", this);
    m_activeTasksTable = new QTableWidget(activeTasksDock);
    m_activeTasksTable->setColumnCount(3);
    m_activeTasksTable->setHorizontalHeaderLabels({"实例ID", "目标DUT", "当前步骤"});
    m_activeTasksTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_activeTasksTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    activeTasksDock->setWidget(m_activeTasksTable);
    addDockWidget(Qt::RightDockWidgetArea, activeTasksDock);

    // 顶部：生产状态
    QDockWidget *productionDock = new QDockWidget("生产状态", this);
    productionDock->setObjectName("productionStatusPanel"); // <--- 设置对象名称
    createProductionStatusWidget(productionDock);
    addDockWidget(Qt::TopDockWidgetArea, productionDock);

    // 底部：日志窗口
    m_logDock = new QDockWidget("实时日志", this);
    m_logDisplayWidget = new LogDisplayWidget(m_logDock);
    m_logDock->setWidget(m_logDisplayWidget);
    addDockWidget(Qt::BottomDockWidgetArea, m_logDock);
    resizeDocks({productionInfoDock, activeTasksDock}, {250, 400}, Qt::Horizontal);
    resizeDocks({m_logDock}, {200}, Qt::Vertical);
}

void MainWindow::connectSignals()
{
    if (!m_coreEngine) return;
    
    // 连接核心引擎信号
    connect(m_coreEngine.get(), &Core::CoreEngine::engineStatusChanged,
            this, [this](Core::CoreEngine::EngineStatus status) {
                onEngineStatusChanged(static_cast<int>(status));
            });
    connect(m_coreEngine.get(), &Core::CoreEngine::coreEngineInitializationFailed,
            this, &MainWindow::onCoreEngineInitializationFailed);
    
    connect(m_coreEngine.get(), &Core::CoreEngine::processStarted,
            this, &MainWindow::onProcessStarted);
    connect(m_coreEngine.get(), &Core::CoreEngine::processCompleted,
            this, &MainWindow::onProcessCompleted);
    connect(m_coreEngine.get(), &Core::CoreEngine::processFailed,
            this, &MainWindow::onProcessFailed);
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
                Q_UNUSED(error);
                        onWorkflowEnded(instanceId);
                    });
            connect(workflowManager.get(), &Services::WorkflowManager::workflowCancelled, // 新增连接
                    this, &MainWindow::onWorkflowEnded);
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
        LOG_MODULE_INFO("MainWindow", QString("已加载工作流配置: %1").arg(fileName).toStdString());
    }
}

void MainWindow::onLoadDeviceConfig()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("选择设备配置文件"),
        "config", 
        tr("JSON文件 (*.json)"));
    
    if (!fileName.isEmpty()) {
        LOG_MODULE_INFO("MainWindow", QString("已加载设备配置: %1").arg(fileName).toStdString());
        if (m_coreEngine) {
            if (m_coreEngine->reloadDeviceConfiguration(fileName)) {
                LOG_MODULE_INFO("MainWindow", QString("核心引擎已重新加载设备配置: %1").arg(fileName).toStdString());
                
                // 同时加载站点配置信息
                auto dutManager = m_coreEngine->getDutManager();
                if (dutManager && dutManager->loadSiteConfiguration(fileName)) {
                    LOG_MODULE_INFO("MainWindow", QString("已从配置文件加载站点信息: %1").arg(fileName).toStdString());
                    
                    // 获取站点信息统计
                    auto siteInfoMap = dutManager->getAllSiteInfo();
                    LOG_MODULE_INFO("MainWindow", QString("成功加载 %1 个站点配置").arg(siteInfoMap.size()).toStdString());
                    
                    // 输出站点信息摘要
                    for (auto it = siteInfoMap.begin(); it != siteInfoMap.end(); ++it) {
                        const auto& siteInfo = it.value();
                        LOG_MODULE_INFO("MainWindow", QString("站点 %1: %2 (%3 sockets, enable=0x%4)")
                                  .arg(siteInfo.siteAlias)
                                  .arg(siteInfo.description)
                                  .arg(siteInfo.socketCount)
                                  .arg(siteInfo.socketEnable, 0, 16).toStdString());
                    }
                } else {
                    LOG_MODULE_WARNING("MainWindow", QString("加载站点配置失败: %1").arg(fileName).toStdString());
                }
            } else {
                LOG_MODULE_ERROR("MainWindow", QString("核心引擎重新加载设备配置失败: %1").arg(fileName).toStdString());
                QMessageBox::warning(this, tr("加载失败"), tr("无法重新加载设备配置文件。"));
            }
        } else {
            LOG_MODULE_WARNING("MainWindow", "核心引擎未初始化，无法重新加载设备配置。");
        }
    }
}

void MainWindow::onConfigRecipe()
{
    // 首先选择配方文件
    QString recipeFilePath = QFileDialog::getOpenFileName(this,
        tr("选择配方文件"),
        "config/recipes",
        tr("JSON文件 (*.json)"));
    
    if (recipeFilePath.isEmpty()) {
        return;
    }
    
    // 创建并显示配方配置对话框
    RecipeConfigDialog configDialog(this);
    configDialog.setRecipeFilePath(recipeFilePath);
    
    if (configDialog.exec() == QDialog::Accepted) {
        // 用户点击了保存，配方文件已经被更新
        QJsonObject configParams = configDialog.getConfigParameters();
        
        // 更新当前工作流路径
        m_currentWorkflowPath = recipeFilePath;
        
        LOG_MODULE_INFO("MainWindow", QString("配方配置已更新: %1").arg(recipeFilePath).toStdString());
        LOG_MODULE_INFO("MainWindow", QString("  - 自动机配置: %1").arg(configParams["cmd3TaskPath"].toString()).toStdString());
        LOG_MODULE_INFO("MainWindow", QString("  - 烧录配置: %1").arg(configParams["taskFileName"].toString()).toStdString());
        LOG_MODULE_INFO("MainWindow", QString("  - 批次号: %1").arg(configParams["batchNumber"].toString()).toStdString());
        LOG_MODULE_INFO("MainWindow", QString("  - 生产数量: %1").arg(configParams["productionQuantity"].toInt()).toStdString());
        
        QMessageBox::information(this, tr("配置成功"), 
            tr("配方配置已更新并保存到文件。\n下次启动工作流时将使用新的配置参数。"));
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
    
    // 启动转台测试工作流（使用安全版本）
    //const QString workflowPath = "config/recipes/turntable_test_safe2.json";
    const QString workflowPath = m_currentWorkflowPath;
    // 检查工作流模板是否已加载
    auto workflowManager = m_coreEngine->getWorkflowManager();
    if (!workflowManager->getLoadedWorkflows().contains(workflowPath)) {
        LOG_MODULE_INFO("MainWindow", "Loading turntable test workflow template...");
        QFile file(workflowPath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, tr("错误"), tr("无法打开工作流文件: %1").arg(workflowPath));
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();

        if (doc.isNull() || !doc.isObject()) {
            QMessageBox::critical(this, tr("错误"), tr("工作流文件格式错误: %1").arg(workflowPath));
            return;
        }

        if (!workflowManager->loadWorkflow(workflowPath, doc.object())) {
            QMessageBox::critical(this, tr("错误"), tr("加载工作流模板失败: %1").arg(workflowPath));
            return;
        }
    }
    
    // 创建工作流上下文
    auto context = std::make_shared<Application::WorkflowContext>();
    context->setData("timestamp", QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
    context->setData("startTime", QDateTime::currentDateTime().toString(Qt::ISODate));
    
    // 启动工作流
    QString instanceId = workflowManager->startWorkflow(workflowPath, context);

    if (instanceId.isEmpty()) {
        QMessageBox::critical(this, tr("错误"), tr("启动转台测试工作流失败，请检查日志。"));
    } else {
        m_isProcessRunning = true;
        m_startButton->setEnabled(false);
        m_pauseButton->setEnabled(true);
        m_stopButton->setEnabled(true);
        
        LOG_MODULE_INFO("MainWindow", QString("已启动转台测试工作流，实例ID: %1").arg(instanceId).toStdString());
        
        // 重置生产状态显示
        m_totalChipsLabel->setText("目标数量: 8");
        m_processedChipsLabel->setText("已处理: 0");
        m_passedChipsLabel->setText("合格: 0");
        m_failedChipsLabel->setText("不合格: 0");
        m_productionProgress->setMaximum(8);
        m_productionProgress->setValue(0);
        m_currentThroughputLabel->setText("吞吐率: 0 chips/min");
        m_sitesStatusLabel->setText("站点状态: 启动中...");
    }
}

void MainWindow::onStopProcess()
{
    if (!m_coreEngine) return;
    auto workflowManager = m_coreEngine->getWorkflowManager();
    if (!workflowManager) return;

    QStringList runningInstances = workflowManager->getRunningInstanceIds();
    if (runningInstances.isEmpty()) {
        LOG_MODULE_INFO("MainWindow", "没有正在运行的工作流需要停止。");
        updateUiForProcessState(); // 确保UI状态正确
        return;
    }

    LOG_MODULE_INFO("MainWindow", QString("准备停止 %1 个正在运行的工作流...").arg(runningInstances.size()).toStdString());
    for (const QString& instanceId : runningInstances) {
        workflowManager->stopWorkflow(instanceId);
    }
    // UI状态的更新将由 onWorkflowEnded 触发
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
    LOG_MODULE_INFO("MainWindow", QString("流程已启动: %1").arg(processName).toStdString());
}

void MainWindow::onProcessCompleted(const QString &processName)
{
    LOG_MODULE_INFO("MainWindow", QString("流程已完成: %1").arg(processName).toStdString());
    QMessageBox::information(this, tr("完成"), tr("测试流程 '%1' 已完成").arg(processName));
    
    m_isProcessRunning = false;
    m_startButton->setEnabled(true);
    m_pauseButton->setEnabled(false);
    m_stopButton->setEnabled(false);
}

void MainWindow::onProcessFailed(const QString &processName, const QString &error)
{
    LOG_MODULE_ERROR("MainWindow", QString("流程失败: %1 - %2").arg(processName).arg(error).toStdString());
    QMessageBox::critical(this, tr("错误"), tr("测试流程 '%1' 失败:\n%2").arg(processName).arg(error));
    
    m_isProcessRunning = false;
    m_startButton->setEnabled(true);
    m_pauseButton->setEnabled(false);
    m_stopButton->setEnabled(false);
}

void MainWindow::onTestResultAvailable(const QJsonObject &result)
{
    Q_UNUSED(result);
    // TODO: 更新测试结果表格
    // appendLog("收到测试结果", 0); // 注释掉这行高频日志，解决UI卡顿问题
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
    LOG_MODULE_INFO("MainWindow", QString("实例 '%1' -> 步骤开始: [%2] %3").arg(instanceId).arg(stepIndex + 1).arg(stepName).toStdString());
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
    // 延迟检查UI状态，确保在所有信号处理后执行
    QTimer::singleShot(100, this, &MainWindow::updateUiForProcessState);
}

void MainWindow::updateUiForProcessState()
{
    if (!m_coreEngine) return;
    auto workflowManager = m_coreEngine->getWorkflowManager();
    if (!workflowManager) return;

    if (workflowManager->getRunningInstanceIds().isEmpty()) {
        m_isProcessRunning = false;
        m_startButton->setEnabled(true);
        m_pauseButton->setEnabled(false);
        m_resumeButton->setEnabled(false);
        m_stopButton->setEnabled(false);
        LOG_MODULE_INFO("MainWindow", "所有工作流已结束，UI状态已重置。");
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

// appendLog 函数已被移除，现在使用 LogDisplayWidget

void MainWindow::onDeviceManager() {
    if (!m_coreEngine) {
        LOG_MODULE_WARNING("MainWindow", "Cannot open DeviceManagerDialog: CoreEngine is not available or failed to initialize.");
        QMessageBox::warning(this, tr("核心引擎不可用"), 
            tr("核心引擎正在初始化或初始化失败，请稍后重试或检查日志。"));
        return;
    }
    
    if (!m_deviceManagerDialog) {
        m_deviceManagerDialog = new DeviceManagerDialog(m_coreEngine, this);
        // 连接关闭信号，在对话框关闭时更新日志
        connect(m_deviceManagerDialog, &QDialog::finished, this, [this]() {
            LOG_MODULE_INFO("MainWindow", "设备配置已更新。");
        });
    }
    
    // 使用show()而不是exec()，避免模态对话框可能的问题
    m_deviceManagerDialog->show();
    m_deviceManagerDialog->raise();
    m_deviceManagerDialog->activateWindow();
}

void MainWindow::onCoreEngineInitializationFailed(const QString& error)
{
    m_statusLabel->setText("核心引擎初始化失败!");
    m_statusLabel->setStyleSheet("color: red;");
    QMessageBox::critical(this, tr("初始化失败"), 
        tr("核心引擎未能成功初始化，应用程序功能受限。\n\n"
           "错误: %1\n\n"
           "请检查日志文件获取详细信息，或检查硬件连接后重启程序。").arg(error));
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

void MainWindow::createProductionStatusWidget(QDockWidget *parent)
{
    QWidget *statusWidget = new QWidget(parent);
    // statusWidget->setObjectName("productionStatusPanel"); // <--- 设置对象名称
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
    
}
} // namespace Presentation
