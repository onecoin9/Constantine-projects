#include "ui/MainWindow.h"
#include "core/CoreEngine.h" // 包含 CoreEngine.h
#include "services/WorkflowManager.h" // 新增
#include "services/DeviceManager.h" // 添加 DeviceManager.h
#include "services/DutManager.h" // 添加 DutManager.h
#include "application/WorkflowContext.h" // 新增
#include "ui/DeviceManagerDialog.h" // 包含 DeviceManagerDialog.h
#include "ui/DutMonitorWidget.h" // <-- 1. 包含新的头文件
#include "domain/BurnDevice.h" // 添加 BurnDevice 头文件
#include "domain/HandlerDevice.h" // 添加 HandlerDevice 头文件
#include <QTimer> // 添加 QTimer
#include "core/Logger.h"
#include "ui/LogConfigDialog.h"
#include "ui/SimpleLogWidget.h"
#include "ui/RecipeConfigDialog.h"
#include "ui/RecipeStartDialog.h"
#include "services/SiteWorkflowRouter.h"
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
    , m_currentBurnDevice(nullptr) // 初始化BurnDevice指针
    , m_waitingForBurnConnection(false) // 初始化连接等待状态
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
        QString defaultConfigPath = "";
        QFile configFile(defaultConfigPath);
        
        if (configFile.exists()) {
            
            auto dutManager = m_coreEngine->getDutManager();
            if (dutManager) {
                
                if (dutManager->loadSiteConfiguration(defaultConfigPath)) {
                    auto siteInfoMap = dutManager->getAllSiteInfo();
                    
                    // 简短的站点信息日志
                    for (auto it = siteInfoMap.begin(); it != siteInfoMap.end(); ++it) {
                        const auto& siteInfo = it.value();
                        
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
}

void MainWindow::setupUi()
{
    // 创建工具栏
    QToolBar *mainToolBar = addToolBar("主工具栏");
    mainToolBar->setMovable(false);
    
    // 先添加按钮到左侧
    m_startButton = new QPushButton("开始测试", this);
    QIcon startIcon(":/resources/icons/play.png");
    if (!startIcon.isNull()) m_startButton->setIcon(startIcon);
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::onStartProcess);
    mainToolBar->addWidget(m_startButton);
    
    m_editButton = new QPushButton("编辑配方", this);
    QIcon editIcon(":/resources/icons/settings.png");
    if (!editIcon.isNull()) m_editButton->setIcon(editIcon);
    connect(m_editButton, &QPushButton::clicked, this, &MainWindow::onEditProcess);
    mainToolBar->addWidget(m_editButton);
    
    m_resumeButton = new QPushButton("手动计量", this);
    QIcon manualIcon(":/resources/icons/control.png");
    if (!manualIcon.isNull()) m_resumeButton->setIcon(manualIcon);
    connect(m_resumeButton, &QPushButton::clicked, this, &MainWindow::onResumeProcess);
    mainToolBar->addWidget(m_resumeButton);
    
    m_stopButton = new QPushButton("停止流程", this);
    QIcon stopIcon(":/resources/icons/stop.png");
    if (!stopIcon.isNull()) m_stopButton->setIcon(stopIcon);
    m_stopButton->setEnabled(false);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::onStopProcess);
    mainToolBar->addWidget(m_stopButton);
    
    // 添加分隔符
    mainToolBar->addSeparator();
    
    // 添加弹性空间，将Logo推到右边
    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mainToolBar->addWidget(spacer);
    
    // 添加应用程序Logo到右侧
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
}

void MainWindow::createMenus()
{
    // 文件菜单
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");
    
    QAction *configRecipeAction = fileMenu->addAction("配方配置(&R)...");
    QIcon settingsIcon(":/resources/icons/settings.png");
    if (!settingsIcon.isNull()) configRecipeAction->setIcon(settingsIcon);
    connect(configRecipeAction, &QAction::triggered, this, &MainWindow::onConfigRecipe);
    
    fileMenu->addSeparator();
    
    QAction *exitAction = fileMenu->addAction("退出(&X)");
    QIcon exitIcon(":/resources/icons/exit.png");
    if (!exitIcon.isNull()) exitAction->setIcon(exitIcon);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onExit);
    
    // 控制菜单
    QMenu *controlMenu = menuBar()->addMenu("控制(&C)");
    
    m_manualControlAction = controlMenu->addAction("手动计量(&M)...");
    QIcon controlIcon(":/resources/icons/control.png");
    if (!controlIcon.isNull()) m_manualControlAction->setIcon(controlIcon);
    connect(m_manualControlAction, &QAction::triggered, this, &MainWindow::onManualControl);
    m_manualControlAction->setEnabled(false); // 初始禁用

    m_deviceControlAction = controlMenu->addAction("设备控制(&D)...");
    QIcon deviceIcon(":/resources/icons/device.png");
    if (!deviceIcon.isNull()) m_deviceControlAction->setIcon(deviceIcon);
    connect(m_deviceControlAction, &QAction::triggered, this, &MainWindow::onDeviceControl);
    m_deviceControlAction->setEnabled(false); // 初始禁用

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
    m_tabWidget = new QTabWidget();
    setCentralWidget(m_tabWidget); // 将此TabWidget设为中央控件

    // 创建并添加DutMonitorWidget到第一个Tab
    m_dutMonitorWidget = new DutMonitorWidget(this);
    m_tabWidget->addTab(m_dutMonitorWidget, "控制面板");

    // 添加其他占位符Tab
    //m_tabWidget->addTab(new QWidget(), "控制面板");
    //m_tabWidget->addTab(new QWidget(), "数据统计");
    //m_tabWidget->addTab(new QWidget(), "告警信息");
    //m_tabWidget->addTab(new QWidget(), "测试计划");
    
    // 创建并添加系统日志Tab
    m_simpleLogWidget = new SimpleLogWidget(this);
    m_tabWidget->addTab(m_simpleLogWidget, "日志");
    
    //m_tabWidget->addTab(new QWidget(), "外设管理");

    // 左侧：生产信息面板
    QDockWidget *productionInfoDock = new QDockWidget("生产信息", this);
    QWidget *productionInfoWidget = new QWidget();
    productionInfoWidget->setObjectName("productionInfoPanel"); // <--- 设置对象名称
    QFormLayout *formLayout = new QFormLayout(productionInfoWidget);
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->addRow("工单:", new QLineEdit(""));
    formLayout->addRow("数量:", new QLineEdit(""));
    formLayout->addRow("良品:", new QLineEdit(""));
    formLayout->addRow("不良:", new QLineEdit(""));
    formLayout->addRow("OS Fail:", new QLineEdit(""));
    formLayout->addRow("Program Fail:", new QLineEdit(""));
    formLayout->addRow("OS通过率:", new QLineEdit(""));
    formLayout->addRow("Prog通过率:", new QLineEdit(""));
    formLayout->addRow("模式:", new QLineEdit("MES模式"));
    formLayout->addRow("自动:", new QLineEdit("自动模式"));
    formLayout->addRow("UPH:", new QLineEdit(""));
    formLayout->addRow("用户:", new QLineEdit(""));
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
    activeTasksDock->setVisible(false);

    // 顶部：生产状态
    QDockWidget *productionDock = new QDockWidget("生产状态", this);
    productionDock->setObjectName("productionStatusPanel"); // <--- 设置对象名称
    createProductionStatusWidget(productionDock);
    addDockWidget(Qt::TopDockWidgetArea, productionDock);
    productionDock->setVisible(false);

    resizeDocks({productionInfoDock, activeTasksDock}, {250, 400}, Qt::Horizontal);
    
    // 注意：系统日志现在在tabWidget的"系统日志"tab中，不再使用底部dock
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
                        onWorkflowEnded(instanceId);
                    });
            connect(workflowManager.get(), &Services::WorkflowManager::workflowCancelled, // 新增连接
                    this, &MainWindow::onWorkflowEnded);
            connect(workflowManager.get(), &Services::WorkflowManager::stepStarted,
                    this, &MainWindow::onStepStarted);
        }
    }
}

/**
 * @brief 配置配方文件
 * 使用异步方式执行文件对话框操作，避免触发KVirtualFolder::Initialize导致死锁
 */
void MainWindow::onConfigRecipe()
{
    // 使用QTimer::singleShot将文件对话框操作延迟到下一个事件循环
    //QTimer::singleShot(0, this, [this]() {
        try {
        
            // 创建并显示配方配置对话框
            RecipeConfigDialog configDialog(this);
            
            if (configDialog.exec() == QDialog::Accepted) {
                // 用户点击了保存，配方文件已经被更新
                QJsonObject configParams = configDialog.getConfigParameters();
                   
                //LOG_MODULE_INFO("MainWindow", QString("配方配置已更新: %1").arg(recipeFilePath).toStdString());
                LOG_MODULE_INFO("MainWindow", QString("  - 自动机配置: %1").arg(configParams["cmd3TaskPath"].toString()).toStdString());
                LOG_MODULE_INFO("MainWindow", QString("  - 烧录任务: %1").arg(configParams["taskFileName"].toString()).toStdString());
                LOG_MODULE_INFO("MainWindow", QString("  - 批次号: %1").arg(configParams["batchNumber"].toString()).toStdString());
                LOG_MODULE_INFO("MainWindow", QString("  - 生产数量: %1").arg(configParams["productionQuantity"].toInt()).toStdString());

            }
        } catch (const std::exception &e) {
            QMessageBox::critical(this, tr("错误"), QString("文件选择异常: %1").arg(e.what()));
        } catch (...) {
            QMessageBox::critical(this, tr("错误"), tr("文件选择发生未知异常"));
        }
    //});
}


/**
 * @brief 配置配方文件
 * 使用异步方式执行文件对话框操作，避免触发KVirtualFolder::Initialize导致死锁
 */
void MainWindow::onSelectRecipe()
{
    // 使用QTimer::singleShot将文件对话框操作延迟到下一个事件循环
    //QTimer::singleShot(0, this, [this]() {
    try {

        // 创建并显示配方配置对话框
        RecipeStartDialog startDialog(this);
        //startDialog.setRecipeFilePath(recipeFilePath);

        if (startDialog.exec() == QDialog::Accepted) {
            // 用户点击了保存，配方文件已经被更新
            QJsonObject configParams = startDialog.getConfigParameters();

            // 更新当前工作流路径
            m_currentWorkflowPath = "config/recipes/turntable_test_safe.json";
            m_currentRouterPath = configParams["routerFile"].toString();

            // 设置taskFileName到BurnDevice配置
            if (m_coreEngine) {
                auto deviceManager = m_coreEngine->getDeviceManager();
                if (deviceManager) {
                    auto burnDevice = deviceManager->getBurnDevice();
                    if (burnDevice) {
                        auto burnDevicePtr = std::dynamic_pointer_cast<Domain::BurnDevice>(burnDevice);
                        if (burnDevicePtr) {
                            // 获取当前配置并更新taskFileName
                            QJsonObject burnConfig = burnDevicePtr->getConfiguration();
                            burnConfig["taskFileName"] = configParams["taskFileName"].toString();
                            burnDevicePtr->setConfiguration(burnConfig);

                            LOG_MODULE_INFO("MainWindow", QString("已设置BurnDevice ACTaskFile: %1")
                                .arg(configParams["taskFileName"].toString()).toStdString());
                        }
                    }

                    auto handleDevice = deviceManager->getHandlerDevice();
                    if (handleDevice) {
                        auto handleDevicePtr = std::dynamic_pointer_cast<Domain::HandlerDevice>(handleDevice);
                        if (handleDevicePtr) {
                            QJsonObject handleConfig = handleDevicePtr->getConfiguration();
                            // 根据配置文件turntable_test_safe.json需要加载cmd3TaskPath
                            handleConfig["cmd3TaskPath"] = configParams["cmd3TaskPath"].toString();
                            handleConfig["productionQuantity"] = configParams["productionQuantity"].toInt();
                            handleDevicePtr->setConfiguration(handleConfig);

                            LOG_MODULE_INFO("MainWindow", QString("已设置HandlerDevice cmd3TaskPath: %1 productionQuantity: %2")
                                .arg(configParams["cmd3TaskPath"].toString()).arg(configParams["productionQuantity"].toInt()).toStdString());
                        }
                    }
                }
            }

            //LOG_MODULE_INFO("MainWindow", QString("配方配置已更新: %1").arg(recipeFilePath).toStdString());
            LOG_MODULE_INFO("MainWindow", QString("  - 自动机配置: %1").arg(configParams["cmd3TaskPath"].toString()).toStdString());
            LOG_MODULE_INFO("MainWindow", QString("  - 烧录任务: %1").arg(configParams["taskFileName"].toString()).toStdString());
            LOG_MODULE_INFO("MainWindow", QString("  - 批次号: %1").arg(configParams["batchNumber"].toString()).toStdString());
            LOG_MODULE_INFO("MainWindow", QString("  - 生产数量: %1").arg(configParams["productionQuantity"].toInt()).toStdString());
            LOG_MODULE_INFO("MainWindow", QString("  - 参数1: %1").arg(configParams["param1"].toString()).toStdString());
            LOG_MODULE_INFO("MainWindow", QString("  - 参数2: %1").arg(configParams["param2"].toString()).toStdString());
            LOG_MODULE_INFO("MainWindow", QString("  - 参数3: %1").arg(configParams["param3"].toString()).toStdString());

            //QMessageBox::information(this, tr("配置成功"), 
            //    tr("配方配置已更新并保存到文件。\n下次启动工作流时将使用新的配置参数。"));
        }
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, tr("错误"), QString("文件选择异常: %1").arg(e.what()));
    }
    catch (...) {
        QMessageBox::critical(this, tr("错误"), tr("文件选择发生未知异常"));
    }
    //});
}

void MainWindow::onExit()
{
    close();
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, tr("关于"),
        tr("昂科测试系统 v1.0\n\n"
           "小转台测试系统\n"));
}

void MainWindow::onStartProcess()
{
    if (!m_coreEngine || !m_coreEngine->isReady()) {
        QMessageBox::warning(this, tr("警告"), tr("系统未就绪，无法启动测试"));
        return;
    }

    onSelectRecipe();

    if (m_currentWorkflowPath.isEmpty()){
        //QMessageBox::warning(this, tr("警告"), tr("WorkFlowPath没有指定，请检查"));
        return;
    }
    
    auto deviceManager = m_coreEngine->getDeviceManager();
    if (!deviceManager) return;

    // 通过getBurnDevice获取Burn设备
    auto burnDevice = deviceManager->getBurnDevice();
    if (!burnDevice) {
        QMessageBox::warning(this, tr("警告"), tr("未找到Burn设备"));
        return;
    }
    
    // 转换为BurnDevice类型并启动Aprog
    auto burnDevicePtr = std::dynamic_pointer_cast<Domain::BurnDevice>(burnDevice);
    if (burnDevicePtr) {
        // 保存BurnDevice引用和待启动的工作流路径
        m_currentBurnDevice = burnDevicePtr;
        m_pendingWorkflowPath = m_currentWorkflowPath;
        m_waitingForBurnConnection = true;

        Services::SiteWorkflowRouter::instance().loadFromFile(m_currentRouterPath);
        
        // 连接BurnDevice状态变更信号
        connect(burnDevicePtr.get(), &Domain::BurnDevice::statusChanged, 
                this, &MainWindow::onBurnDeviceConnectionChanged);
        
        // 启动Aprog
        burnDevicePtr->startAprog();
        LOG_MODULE_INFO("MainWindow", "启动Aprog程序，等待连接建立...");
        
        // 更新UI状态，显示正在连接
        m_statusLabel->setText("正在启动Aprog并建立连接...");
        m_startButton->setEnabled(false);

        connect(burnDevicePtr.get(), &Domain::BurnDevice::aprogStarted, this, [this]() {
            m_statusLabel->setText("程序启动成功，正在执行测试任务...");
        }, Qt::QueuedConnection);

    } else {
        QMessageBox::warning(this, tr("警告"), tr("BurnDevice类型转换失败"));
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

void MainWindow::onEditProcess()
{
    try {

        // 创建并显示配方配置对话框
        RecipeConfigDialog configDialog(this);

        if (configDialog.exec() == QDialog::Accepted) {
            // 用户点击了保存，配方文件已经被更新
            QJsonObject configParams = configDialog.getConfigParameters();

            LOG_MODULE_INFO("MainWindow", QString("  - 自动机配置: %1").arg(configParams["cmd3TaskPath"].toString()).toStdString());
            LOG_MODULE_INFO("MainWindow", QString("  - 烧录任务: %1").arg(configParams["taskFileName"].toString()).toStdString());
            LOG_MODULE_INFO("MainWindow", QString("  - 批次号: %1").arg(configParams["batchNumber"].toString()).toStdString());
            LOG_MODULE_INFO("MainWindow", QString("  - 生产数量: %1").arg(configParams["productionQuantity"].toInt()).toStdString());

        }
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, tr("错误"), QString("文件选择异常: %1").arg(e.what()));
    }
    catch (...) {
        QMessageBox::critical(this, tr("错误"), tr("文件选择发生未知异常"));
    }

}

void MainWindow::onResumeProcess()
{
    QMessageBox::information(this, tr("功能调整"), tr("手动讲量的功能开发中。"));
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
    m_editButton->setEnabled(false);
    m_stopButton->setEnabled(false);
}

void MainWindow::onProcessFailed(const QString &processName, const QString &error)
{
    LOG_MODULE_ERROR("MainWindow", QString("流程失败: %1 - %2").arg(processName).arg(error).toStdString());
    QMessageBox::critical(this, tr("错误"), tr("测试流程 '%1' 失败:\n%2").arg(processName).arg(error));
    
    m_isProcessRunning = false;
    m_startButton->setEnabled(true);
    m_editButton->setEnabled(false);
    m_stopButton->setEnabled(false);
}

void MainWindow::onTestResultAvailable(const QJsonObject &result)
{
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
    // LOG_MODULE_INFO("MainWindow", QString("实例 '%1' -> 步骤开始: [%2] %3").arg(instanceId).arg(stepIndex + 1).arg(stepName).toStdString());
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
        m_editButton->setEnabled(false);
        m_resumeButton->setEnabled(false);
        m_stopButton->setEnabled(false);
        LOG_MODULE_INFO("MainWindow", "所有工作流已结束，UI状态已重置。");
        m_statusLabel->setText("测试任务已结束");
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

void MainWindow::onBurnDeviceConnectionChanged(Domain::IDevice::DeviceStatus status)
{
    if (!m_waitingForBurnConnection || !m_currentBurnDevice) {
        return;
    }
    
    // 将状态枚举转换为可读字符串
    QString statusText;
    switch (status) {
        case Domain::IDevice::DeviceStatus::Disconnected:
            statusText = "Disconnected";
            break;
        case Domain::IDevice::DeviceStatus::Connected:
            statusText = "Connected";
            break;
        case Domain::IDevice::DeviceStatus::Initializing:
            statusText = "Initializing";
            break;
        case Domain::IDevice::DeviceStatus::Ready:
            statusText = "Ready";
            break;
        case Domain::IDevice::DeviceStatus::Scanned:
            statusText = "Scanned";
            break;
        case Domain::IDevice::DeviceStatus::Loaded:
            statusText = "Loaded";
            break;
        case Domain::IDevice::DeviceStatus::Busy:
            statusText = "Busy";
            break;
        case Domain::IDevice::DeviceStatus::Error:
            statusText = "Error";
            break;
        default:
            statusText = QString("Unknown(%1)").arg(static_cast<int>(status));
            break;
    }
    
    LOG_MODULE_INFO("MainWindow", QString("BurnDevice状态变更: %1").arg(statusText).toStdString());

    // 检查BurnDevice状态
    if (status == Domain::IDevice::DeviceStatus::Ready) {
        LOG_MODULE_INFO("MainWindow", "BurnDevice已就绪，发送站点扫描命令");
        QJsonObject cmdParams;
        m_currentBurnDevice->executeCommand("SiteScanAndConnect", cmdParams);
    }
    else if (status == Domain::IDevice::DeviceStatus::Scanned) {
        LOG_MODULE_INFO("MainWindow", "BurnDevice已完成站点扫描，开始加载工程");
        QJsonObject cmdParams;
        m_currentBurnDevice->executeCommand("LoadProject", cmdParams);
    }
    else if (status == Domain::IDevice::DeviceStatus::Loaded) {
        LOG_MODULE_INFO("MainWindow", "BurnDevice已加载工程，开始启动工作流");

        // 重置等待状态
        m_waitingForBurnConnection = false;

        // 启动工作流
        if (!m_pendingWorkflowPath.isEmpty()) {
            startWorkflowAfterConnection(m_pendingWorkflowPath);
            m_pendingWorkflowPath.clear();
        }

        // 断开连接状态变更信号
        if (m_currentBurnDevice) {
            disconnect(m_currentBurnDevice.get(), &Domain::BurnDevice::statusChanged,
                this, &MainWindow::onBurnDeviceConnectionChanged);
        }
    }
}

void MainWindow::startWorkflowAfterConnection(const QString& workflowPath)
{
    auto workflowManager = m_coreEngine->getWorkflowManager();
    if (!workflowManager) {
        LOG_MODULE_ERROR("MainWindow", "WorkflowManager不可用");
        return;
    }

    // 检查工作流模板是否已加载
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
        m_editButton->setEnabled(true);
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

} // namespace Presentation
