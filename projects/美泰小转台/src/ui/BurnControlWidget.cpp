#include "ui/BurnControlWidget.h"
#include "domain/BurnDevice.h"
#include "Ag06DoCustomProtocol.h"
#include "Ag06TrimDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTextCursor>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QMessageBox>
#include <QSplitter>
#include <QFrame>
#include <QFont>
#include <QDebug>
#include <QSharedMemory>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QProgressBar>
#include <QTabWidget>
#include <QScrollBar>
#include <QClipboard>
#include <QApplication>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QRegularExpression>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QCheckBox>
#include <QComboBox>

namespace Presentation {

    // 构造函数
    BurnControlWidget::BurnControlWidget(QWidget* parent)
        : IDeviceControlWidget(parent)
        , m_burnDevice(nullptr)
        , m_jsonRpcClient(nullptr)
        , m_isConnected(false)
        , m_requestCounter(0)
        , m_statusTimer(nullptr)
        , m_aprogStatusTimer(nullptr)
    {
        // 初始化定时器
        m_statusTimer = new QTimer(this);
        m_statusTimer->setInterval(1000);
        connect(m_statusTimer, &QTimer::timeout, this, &BurnControlWidget::updateConnectionStatus);

        m_aprogStatusTimer = new QTimer(this);
        m_aprogStatusTimer->setInterval(2000);
        connect(m_aprogStatusTimer, &QTimer::timeout, this, &BurnControlWidget::onAprogStatusTimerTimeout);

        setupUi();
    }

    // 析构函数
    BurnControlWidget::~BurnControlWidget()
    {
        if (m_burnDevice) {
            disconnect(m_burnDevice.get(), nullptr, this, nullptr);
        }
        if (m_jsonRpcClient) {
            disconnect(m_jsonRpcClient, nullptr, this, nullptr);
        }
    }

    // IDeviceControlWidget接口实现
    void BurnControlWidget::setDevice(std::shared_ptr<Domain::IDevice> device)
    {
        appendLog("setDevice 被调用", "DEBUG");

        if (m_burnDevice) {
            appendLog("断开之前的设备连接", "DEBUG");
            disconnect(m_burnDevice.get(), nullptr, this, nullptr);
        }

        if (!device) {
            m_burnDevice.reset();
            m_jsonRpcClient = nullptr;
            setControlsEnabled(false);
            if (m_aprogStatusTimer && m_aprogStatusTimer->isActive()) {
                m_aprogStatusTimer->stop();
            }
            appendLog("设备已断开连接", "WARNING");
            return;
        }

        appendLog(QString("尝试设置设备: %1, 类型: %2")
            .arg(device->getName())
            .arg(static_cast<int>(device->getType())), "DEBUG");

        m_burnDevice = std::dynamic_pointer_cast<Domain::BurnDevice>(device);
        if (m_burnDevice) {
            appendLog("设备类型转换成功", "DEBUG");

            // 获取JsonRpcClient实例
            m_jsonRpcClient = m_burnDevice->getJsonRpcClient();

            try {
                connectSignals();
                updateStatus();
                setControlsEnabled(true);
                appendLog(QString("设备已连接: %1").arg(device->getName()), "INFO");

                // 更新UI中的服务器配置
                if (m_hostEdit && m_portSpin) {
                    m_hostEdit->setText(m_burnDevice->getServerHost());
                    m_portSpin->setValue(m_burnDevice->getServerPort());
                    appendLog("服务器配置UI已更新", "DEBUG");
                }

                // 更新UI中的Aprog路径配置
                if (m_aprogPathEdit) {
                    QString configuredPath = m_burnDevice->getAprogPath();
                    if (!configuredPath.isEmpty()) {
                        m_aprogPathEdit->setText(configuredPath);
                        appendLog(QString("从配置加载Aprog路径: %1").arg(configuredPath), "INFO");
                    }
                }

                updateConnectionStatus();
                updateAprogStatus();

                // 启动定时器
                if (m_statusTimer && !m_statusTimer->isActive()) {
                    m_statusTimer->start();
                }
                if (m_aprogStatusTimer && !m_aprogStatusTimer->isActive()) {
                    m_aprogStatusTimer->start();
                }
            }
            catch (const std::exception& e) {
                appendLog(QString("设置设备时发生异常: %1").arg(e.what()), "ERROR");
                setControlsEnabled(false);
            }
        }
        else {
            appendLog(QString("设备类型不匹配 - 期望BurnDevice，实际类型: %1")
                .arg(static_cast<int>(device->getType())), "ERROR");
            setControlsEnabled(false);
        }
    }

    std::shared_ptr<Domain::IDevice> BurnControlWidget::getDevice() const
    {
        return m_burnDevice;
    }

    void BurnControlWidget::updateStatus()
    {
        updateConnectionStatus();
        updateAprogStatus();
    }

    void BurnControlWidget::setControlsEnabled(bool enabled)
    {
        // 连接管理
        if (m_connectBtn) m_connectBtn->setEnabled(!m_isConnected && enabled);
        if (m_disconnectBtn) m_disconnectBtn->setEnabled(m_isConnected && enabled);

        // 方法测试按钮
        enableMethodButtons(enabled && m_isConnected);

        // Aprog程序管理始终可用
        if (m_aprogPathEdit) m_aprogPathEdit->setEnabled(true);
        if (m_browseAprogBtn) m_browseAprogBtn->setEnabled(true);
        if (m_startAprogBtn) m_startAprogBtn->setEnabled(true);
        if (m_stopAprogBtn) m_stopAprogBtn->setEnabled(true);
    }

    QString BurnControlWidget::getDeviceTypeName() const
    {
        return tr("烧录设备控制");
    }

    // 设置UI
    void BurnControlWidget::setupUi()
    {
        auto* mainLayout = new QVBoxLayout(this);
        mainLayout->setSpacing(10);
        mainLayout->setContentsMargins(10, 10, 10, 10);

        // 创建主分割器
        auto* mainSplitter = new QSplitter(Qt::Horizontal, this);

        // 左侧：Aprog管理、连接和方法
        auto* leftWidget = new QWidget;
        auto* leftLayout = new QVBoxLayout(leftWidget);
        leftLayout->setSpacing(10);

        setupAprogGroup();
        setupConnectionGroup();
        setupMethodsGroup();

        leftLayout->addWidget(m_aprogGroup);
        leftLayout->addWidget(m_connectionGroup);
        leftLayout->addWidget(m_methodsGroup);
        leftLayout->addStretch();

        // 右侧：结果显示、站点管理和自定义JSON
        auto* rightWidget = new QWidget;
        auto* rightLayout = new QVBoxLayout(rightWidget);
        rightLayout->setSpacing(10);

        setupResultsGroup();
        // 站点管理由 DutMonitorWidget 负责
        setupCustomJsonGroup();
        setupAg06CustomUi();
        setupSiteJobGroup();

        rightLayout->addWidget(m_resultsTabWidget);
        rightLayout->addWidget(m_siteJobGroup);
        rightLayout->addWidget(m_customJsonGroup);
        rightLayout->addWidget(m_ag06Group);

        // 添加到主分割器
        mainSplitter->addWidget(leftWidget);
        mainSplitter->addWidget(rightWidget);
        mainSplitter->setStretchFactor(0, 1);
        mainSplitter->setStretchFactor(1, 2);

        mainLayout->addWidget(mainSplitter);

        // 初始状态
        setControlsEnabled(false);
        updateConnectionStatus();

        // 窗口设置
        setMinimumSize(1200, 800);
    }

    // 连接信号槽
    void BurnControlWidget::connectSignals()
    {
        if (!m_burnDevice) {
            appendLog("设备指针为空，无法连接信号", "WARNING");
            return;
        }

        try {
            // 断开之前的连接
            disconnect(m_burnDevice.get(), nullptr, this, nullptr);

            // 连接Aprog进程相关信号
            connect(m_burnDevice.get(), &Domain::BurnDevice::aprogStarted,
                this, &BurnControlWidget::onAprogStarted);
            connect(m_burnDevice.get(), &Domain::BurnDevice::aprogFinished,
                this, &BurnControlWidget::onAprogFinished);
            connect(m_burnDevice.get(), &Domain::BurnDevice::aprogError,
                this, &BurnControlWidget::onAprogError);
            connect(m_burnDevice.get(), &Domain::BurnDevice::aprogOutputReceived,
                this, &BurnControlWidget::onAprogOutputReceived);

            // 连接JsonRpcClient信号
            if (m_jsonRpcClient) {
                disconnect(m_jsonRpcClient, nullptr, this, nullptr);

                connect(m_jsonRpcClient, &JsonRpcClient::connectionStateChanged,
                    this, &BurnControlWidget::onConnectionStateChanged);
                connect(m_jsonRpcClient, &JsonRpcClient::notificationReceived,
                    this, &BurnControlWidget::onNotificationReceived);
                connect(m_jsonRpcClient, &JsonRpcClient::errorOccurred,
                    this, &BurnControlWidget::onClientError);
            }

            appendLog("设备信号连接成功", "DEBUG");
        }
        catch (const std::exception& e) {
            appendLog(QString("连接设备信号时发生异常: %1").arg(e.what()), "ERROR");
        }
    }

    // 日志功能
    void BurnControlWidget::appendLog(const QString& message, const QString& level)
    {
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
        QString colorCode;

        if (level == "ERROR") {
            colorCode = "color: red; font-weight: bold;";
        }
        else if (level == "WARNING") {
            colorCode = "color: orange; font-weight: bold;";
        }
        else if (level == "SUCCESS") {
            colorCode = "color: green; font-weight: bold;";
        }
        else if (level == "DEBUG") {
            colorCode = "color: gray;";
        }
        else {
            colorCode = "color: blue;";
        }

        QString formattedMsg = QString("<span style='%1'>[%2][%3] %4</span><br>")
            .arg(colorCode)
            .arg(timestamp)
            .arg(level)
            .arg(message);

        // 添加到日志显示区域
        addLogEntry(level, message);

        // 同时输出到调试控制台
        qDebug() << QString("[%1][%2] %3").arg(timestamp).arg(level).arg(message);
    }

    // 检查Aprog.exe是否运行
    bool BurnControlWidget::isAprogRunningBySharedMemory() const
    {
        QSharedMemory sharedMemory("AcroView_SingleInstance_Guard");

        if (sharedMemory.attach()) {
            sharedMemory.detach();
            return true;
        }

        return false;
    }

    // 设置Aprog管理组
    void BurnControlWidget::setupAprogGroup()
    {
        m_aprogGroup = new QGroupBox("Aprog.exe 程序管理", this);
        auto* layout = new QVBoxLayout(m_aprogGroup);

        // Aprog路径
        auto* pathLayout = new QHBoxLayout;
        pathLayout->addWidget(new QLabel("程序路径:"));
        m_aprogPathEdit = new QLineEdit;
        m_aprogPathEdit->setPlaceholderText("将从设备配置加载Aprog.exe路径");
        m_browseAprogBtn = new QPushButton("浏览...");
        pathLayout->addWidget(m_aprogPathEdit);
        pathLayout->addWidget(m_browseAprogBtn);
        layout->addLayout(pathLayout);

        // 状态显示
        auto* statusLayout = new QHBoxLayout;
        statusLayout->addWidget(new QLabel("程序状态:"));
        m_aprogStatusLabel = new QLabel("未启动");
        m_aprogStatusLabel->setStyleSheet("color: gray; font-weight: bold;");
        statusLayout->addWidget(m_aprogStatusLabel);
        statusLayout->addStretch();
        layout->addLayout(statusLayout);

        // 控制按钮
        auto* controlLayout = new QHBoxLayout;
        m_startAprogBtn = new QPushButton("启动程序");
        m_stopAprogBtn = new QPushButton("停止程序");
        controlLayout->addWidget(m_startAprogBtn);
        controlLayout->addWidget(m_stopAprogBtn);
        controlLayout->addStretch();
        layout->addLayout(controlLayout);

        // 连接信号
        connect(m_aprogPathEdit, &QLineEdit::textChanged, this, &BurnControlWidget::onAprogPathChanged);
        connect(m_browseAprogBtn, &QPushButton::clicked, this, &BurnControlWidget::onBrowseAprogPathClicked);
        connect(m_startAprogBtn, &QPushButton::clicked, this, &BurnControlWidget::onStartAprogClicked);
        connect(m_stopAprogBtn, &QPushButton::clicked, this, &BurnControlWidget::onStopAprogClicked);
    }

    // 设置连接管理组
    void BurnControlWidget::setupConnectionGroup()
    {
        m_connectionGroup = new QGroupBox("服务器连接", this);
        auto* layout = new QVBoxLayout(m_connectionGroup);

        // 服务器配置
        auto* serverLayout = new QHBoxLayout;
        serverLayout->addWidget(new QLabel("服务器:"));

        m_hostEdit = new QLineEdit("127.0.0.1");
        m_hostEdit->setPlaceholderText("主机地址");
        serverLayout->addWidget(m_hostEdit);

        m_portSpin = new QSpinBox;
        m_portSpin->setRange(1, 65535);
        m_portSpin->setValue(12345);
        m_portSpin->setPrefix("端口: ");
        serverLayout->addWidget(m_portSpin);

        layout->addLayout(serverLayout);

        // 连接按钮
        auto* buttonLayout = new QHBoxLayout;
        m_connectBtn = new QPushButton("连接");
        m_disconnectBtn = new QPushButton("断开连接");
        m_disconnectBtn->setEnabled(false);

        buttonLayout->addWidget(m_connectBtn);
        buttonLayout->addWidget(m_disconnectBtn);
        buttonLayout->addStretch();

        layout->addLayout(buttonLayout);

        // 状态显示
        auto* statusLayout = new QHBoxLayout;
        m_statusLabel = new QLabel("未连接");
        m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
        m_progressBar = new QProgressBar;
        m_progressBar->setVisible(false);
        m_progressBar->setRange(0, 0);

        statusLayout->addWidget(new QLabel("状态:"));
        statusLayout->addWidget(m_statusLabel);
        statusLayout->addWidget(m_progressBar);
        statusLayout->addStretch();

        layout->addLayout(statusLayout);

        // 重连间隔
        auto* reconnectLayout = new QFormLayout;
        m_reconnectIntervalSpin = new QSpinBox;
        m_reconnectIntervalSpin->setRange(1, 60);
        m_reconnectIntervalSpin->setValue(5);
        m_reconnectIntervalSpin->setSuffix(" 秒");
        reconnectLayout->addRow("重连间隔:", m_reconnectIntervalSpin);

        layout->addLayout(reconnectLayout);

        // 连接信号
        connect(m_connectBtn, &QPushButton::clicked, this, &BurnControlWidget::onConnectClicked);
        connect(m_disconnectBtn, &QPushButton::clicked, this, &BurnControlWidget::onDisconnectClicked);
        connect(m_reconnectIntervalSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            [this](int value) {
                if (m_jsonRpcClient) {
                    m_jsonRpcClient->setReconnectInterval(value * 1000);
                }
            });
    }

    // 设置方法测试组
    void BurnControlWidget::setupMethodsGroup()
    {
        m_methodsGroup = new QGroupBox("RPC方法测试", this);
        auto* layout = new QVBoxLayout(m_methodsGroup);

        // 项目文件选择
        auto* projectLayout = new QGridLayout;
        projectLayout->addWidget(new QLabel("项目路径:"), 0, 0);
        m_projectPathEdit = new QLineEdit;
        m_projectPathEdit->setPlaceholderText("选择项目文件夹路径");
        m_browseProjectBtn = new QPushButton("浏览...");
        projectLayout->addWidget(m_projectPathEdit, 0, 1);
        projectLayout->addWidget(m_browseProjectBtn, 0, 2);

        projectLayout->addWidget(new QLabel("任务文件:"), 1, 0);
        m_taskFileEdit = new QLineEdit;
        m_taskFileEdit->setPlaceholderText("任务文件名 (如: ag06.actask)");
        m_browseTaskBtn = new QPushButton("浏览...");
        projectLayout->addWidget(m_taskFileEdit, 1, 1);
        projectLayout->addWidget(m_browseTaskBtn, 1, 2);

        layout->addLayout(projectLayout);

        // 方法按钮
        auto* methodBtnLayout = new QGridLayout;
        m_loadProjectBtn = new QPushButton("加载项目");
        m_siteScanBtn = new QPushButton("站点扫描");
        m_doJobBtn = new QPushButton("执行作业");
        m_doCustomBtn = new QPushButton("自定义命令");

        methodBtnLayout->addWidget(m_loadProjectBtn, 0, 0);
        methodBtnLayout->addWidget(m_siteScanBtn, 0, 1);
        methodBtnLayout->addWidget(m_doJobBtn, 1, 0);
        methodBtnLayout->addWidget(m_doCustomBtn, 1, 1);

        layout->addLayout(methodBtnLayout);

        // 连接信号
        connect(m_browseProjectBtn, &QPushButton::clicked, [this]() {
            QString dir = QFileDialog::getExistingDirectory(this, "选择项目文件夹");
            if (!dir.isEmpty()) {
                m_projectPathEdit->setText(dir);
            }
            });

        connect(m_browseTaskBtn, &QPushButton::clicked, [this]() {
            QString file = QFileDialog::getOpenFileName(this, "选择任务文件", "",
                "任务文件 (*.actask);;所有文件 (*.*)");
            if (!file.isEmpty()) {
                QFileInfo fi(file);
                m_taskFileEdit->setText(fi.fileName());
            }
            });

        connect(m_loadProjectBtn, &QPushButton::clicked, this, &BurnControlWidget::onLoadProjectClicked);
        connect(m_siteScanBtn, &QPushButton::clicked, this, &BurnControlWidget::onSiteScanAndConnectClicked);
        connect(m_doJobBtn, &QPushButton::clicked, this, &BurnControlWidget::onDoJobClicked);
        connect(m_doCustomBtn, &QPushButton::clicked, this, &BurnControlWidget::onDoCustomClicked);
    }

    // 设置站点作业区
    void BurnControlWidget::setupSiteJobGroup()
    {
        m_siteJobGroup = new QGroupBox("站点作业", this);
        auto* siteGroupLayout = new QVBoxLayout(m_siteJobGroup);

        // 站点选择
        auto* siteSelLayout = new QHBoxLayout;
        siteSelLayout->addWidget(new QLabel("站点:"));
        m_siteComboBox = new QComboBox;
        connect(m_siteComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BurnControlWidget::onSiteSelectionChanged);
        siteSelLayout->addWidget(m_siteComboBox);
        siteSelLayout->addStretch();
        siteGroupLayout->addLayout(siteSelLayout);

        // 站点信息
        m_siteInfoLabel = new QLabel("请选择站点");
        siteGroupLayout->addWidget(m_siteInfoLabel);

        // Socket 选择（16座）
        m_socketWidget = new QWidget(m_siteJobGroup);
        m_socketLayout = new QGridLayout(m_socketWidget);
        for (int i = 0; i < 16; ++i) {
            m_socketCheckBoxes[i] = new QCheckBox(QString::number(i), m_socketWidget);
            connect(m_socketCheckBoxes[i], &QCheckBox::toggled, this, [this, i](bool checked) {
                onSocketToggled(i, checked);
                });
            int row = i / 8, col = i % 8;
            m_socketLayout->addWidget(m_socketCheckBoxes[i], row, col);
        }
        siteGroupLayout->addWidget(m_socketWidget);

        // 命令序列 + 执行
        auto* cmdLayout = new QHBoxLayout;
        cmdLayout->addWidget(new QLabel("命令序列:"));
        m_cmdSequenceComboBox = new QComboBox;
        connect(m_cmdSequenceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BurnControlWidget::onCmdSequenceSelectionChanged);
        cmdLayout->addWidget(m_cmdSequenceComboBox);

        m_executeJobBtn = new QPushButton("执行作业");
        connect(m_executeJobBtn, &QPushButton::clicked, this, &BurnControlWidget::onExecuteJobClicked);
        cmdLayout->addWidget(m_executeJobBtn);
        cmdLayout->addStretch();
        siteGroupLayout->addLayout(cmdLayout);

        // 已选座显示
        m_selectedSocketsLabel = new QLabel("已选择: 无");
        siteGroupLayout->addWidget(m_selectedSocketsLabel);
    }


    // 设置结果显示组
    void BurnControlWidget::setupResultsGroup()
    {
        m_resultsTabWidget = new QTabWidget(this);

        // 日志标签页
        auto* logTab = new QWidget;
        auto* logLayout = new QVBoxLayout(logTab);

        m_logTextEdit = new QTextEdit;
        m_logTextEdit->setReadOnly(true);
        m_logTextEdit->setFont(QFont("Consolas", 9));
        logLayout->addWidget(m_logTextEdit);

        // 控制按钮
        auto* controlLayout = new QHBoxLayout;
        m_clearLogsBtn = new QPushButton("清除日志");
        m_autoScrollBox = new QCheckBox("自动滚动");
        m_prettyJsonBox = new QCheckBox("格式化JSON");

        m_autoScrollBox->setChecked(true);
        m_prettyJsonBox->setChecked(true);

        controlLayout->addWidget(m_clearLogsBtn);
        controlLayout->addWidget(m_autoScrollBox);
        controlLayout->addWidget(m_prettyJsonBox);
        controlLayout->addStretch();

        logLayout->addLayout(controlLayout);

        m_resultsTabWidget->addTab(logTab, "日志");

        // 响应标签页
        auto* responseTab = new QWidget;
        auto* responseLayout = new QVBoxLayout(responseTab);

        m_responseTreeWidget = new QTreeWidget;
        m_responseTreeWidget->setHeaderLabels({ "时间", "方法", "状态", "数据" });
        m_responseTreeWidget->setRootIsDecorated(true);
        responseLayout->addWidget(m_responseTreeWidget);

        m_resultsTabWidget->addTab(responseTab, "响应");

        // 通知标签页
        auto* notificationTab = new QWidget;
        auto* notificationLayout = new QVBoxLayout(notificationTab);

        m_notificationTreeWidget = new QTreeWidget;
        m_notificationTreeWidget->setHeaderLabels({ "时间", "方法", "参数" });
        m_notificationTreeWidget->setRootIsDecorated(true);
        notificationLayout->addWidget(m_notificationTreeWidget);

        m_resultsTabWidget->addTab(notificationTab, "通知");

        // 连接信号
        connect(m_clearLogsBtn, &QPushButton::clicked, this, &BurnControlWidget::onClearLogsClicked);
        connect(m_autoScrollBox, &QCheckBox::toggled, this, &BurnControlWidget::onAutoScrollToggled);
    }

    // 移除站点管理UI，改由 DutMonitorWidget 承担

    // 设置自定义JSON编辑组
    void BurnControlWidget::setupCustomJsonGroup()
    {
        m_customJsonGroup = new QGroupBox("自定义JSON编辑", this);
        auto* layout = new QVBoxLayout(m_customJsonGroup);

        // JSON编辑器
        m_customJsonEdit = new QTextEdit;
        m_customJsonEdit->setPlaceholderText("在此输入自定义JSON参数...");
        m_customJsonEdit->setMaximumHeight(150);
        m_customJsonEdit->setFont(QFont("Consolas", 9));
        layout->addWidget(m_customJsonEdit);

        // 按钮布局
        auto* buttonLayout = new QHBoxLayout;
        m_formatJsonBtn = new QPushButton("格式化");
        m_clearJsonBtn = new QPushButton("清除");
        m_executeCustomBtn = new QPushButton("执行自定义命令");

        buttonLayout->addWidget(m_formatJsonBtn);
        buttonLayout->addWidget(m_clearJsonBtn);
        buttonLayout->addStretch();
        buttonLayout->addWidget(m_executeCustomBtn);

        layout->addLayout(buttonLayout);

        // 状态标签
        m_jsonStatusLabel = new QLabel("JSON状态: 未输入");
        m_jsonStatusLabel->setStyleSheet("QLabel { color: gray; }");
        layout->addWidget(m_jsonStatusLabel);

        // 连接信号
        connect(m_customJsonEdit, &QTextEdit::textChanged,
            this, &BurnControlWidget::onCustomJsonTextChanged);
        connect(m_formatJsonBtn, &QPushButton::clicked,
            this, &BurnControlWidget::onFormatJsonClicked);
        connect(m_clearJsonBtn, &QPushButton::clicked,
            this, &BurnControlWidget::onClearJsonClicked);
        connect(m_executeCustomBtn, &QPushButton::clicked,
            this, &BurnControlWidget::onExecuteCustomClicked);
    }

    // 设置AG06自定义UI
    void BurnControlWidget::setupAg06CustomUi()
    {
        m_ag06Group = new QGroupBox("AG06 DoCustom 协议", this);
        auto* layout = new QVBoxLayout(m_ag06Group);

        // UID发送
        auto* uidLayout = new QHBoxLayout;
        uidLayout->addWidget(new QLabel("UID (8字符):"));
        m_ag06UidEdit = new QLineEdit;
        m_ag06UidEdit->setPlaceholderText("例如: B8200000");
        m_ag06UidEdit->setMaxLength(8);
        m_ag06SendUidBtn = new QPushButton("发送 UID (0x10)");
        uidLayout->addWidget(m_ag06UidEdit);
        uidLayout->addWidget(m_ag06SendUidBtn);
        layout->addLayout(uidLayout);

        // Trim JSON编辑
        layout->addWidget(new QLabel("Trim 参数 JSON:"));
        m_ag06TrimJsonEdit = new QTextEdit;
        m_ag06TrimJsonEdit->setPlaceholderText("输入包含 t1_trim_en/t1_trim_regs/... 的 JSON");
        m_ag06TrimJsonEdit->setMaximumHeight(100);
        layout->addWidget(m_ag06TrimJsonEdit);

        // Trim发送按钮
        auto* trimBtnLayout = new QHBoxLayout;
        m_ag06SendTrimBtn = new QPushButton("发送 Trim (0x11)");
        m_openTrimDialogBtn = new QPushButton("打开Trim编辑器");
        trimBtnLayout->addWidget(m_ag06SendTrimBtn);
        trimBtnLayout->addWidget(m_openTrimDialogBtn);
        trimBtnLayout->addStretch();
        layout->addLayout(trimBtnLayout);

        // 状态显示
        m_ag06Status = new QLabel("状态: 就绪");
        m_ag06Status->setStyleSheet("QLabel { color: blue; }");
        layout->addWidget(m_ag06Status);

        // 连接信号
        connect(m_ag06SendUidBtn, &QPushButton::clicked,
            this, &BurnControlWidget::ag06SendUid);
        connect(m_ag06SendTrimBtn, &QPushButton::clicked,
            this, &BurnControlWidget::ag06SendJsonTrim);
        connect(m_openTrimDialogBtn, &QPushButton::clicked,
            this, &BurnControlWidget::openAg06TrimDialog);
    }

    // Aprog相关槽函数实现
    void BurnControlWidget::onBrowseAprogPathClicked()
    {
        QTimer::singleShot(0, this, [this]() {
            QString filePath = QFileDialog::getOpenFileName(this,
                tr("选择Aprog.exe程序"),
                "",
                tr("可执行文件 (*.exe);;所有文件 (*.*)"));

            if (!filePath.isEmpty()) {
                m_aprogPathEdit->setText(filePath);
                appendLog(QString("选择Aprog程序: %1").arg(filePath), "INFO");

                if (m_burnDevice) {
                    m_burnDevice->setAprogPath(filePath);
                }
            }
            });
    }

    void BurnControlWidget::onAprogPathChanged()
    {
        if (!m_burnDevice) return;

        QString path = m_aprogPathEdit->text().trimmed();
        m_burnDevice->setAprogPath(path);
    }

    void BurnControlWidget::onStartAprogClicked()
    {
        if (!m_burnDevice) {
            appendLog("设备未连接", "ERROR");
            return;
        }

        appendLog("正在启动Aprog.exe程序...", "INFO");
        m_burnDevice->startAprog();
    }

    void BurnControlWidget::onStopAprogClicked()
    {
        if (!m_burnDevice) {
            appendLog("设备未连接", "ERROR");
            return;
        }

        appendLog("正在停止Aprog.exe程序...", "INFO");
        m_burnDevice->stopAprog();
    }

    void BurnControlWidget::onAprogStarted()
    {
        appendLog("Aprog.exe程序已启动", "SUCCESS");
        updateAprogStatus();
    }

    void BurnControlWidget::onAprogFinished(int exitCode, QProcess::ExitStatus exitStatus)
    {
        QString statusStr = (exitStatus == QProcess::NormalExit) ? "正常退出" : "崩溃退出";
        appendLog(QString("Aprog.exe程序已结束: %1 (退出码: %2)").arg(statusStr).arg(exitCode),
            exitStatus == QProcess::NormalExit ? "INFO" : "WARNING");
        updateAprogStatus();
    }

    void BurnControlWidget::onAprogError(QProcess::ProcessError error)
    {
        QString errorStr;
        switch (error) {
        case QProcess::FailedToStart:
            errorStr = "启动失败";
            break;
        case QProcess::Crashed:
            errorStr = "程序崩溃";
            break;
        case QProcess::Timedout:
            errorStr = "操作超时";
            break;
        case QProcess::WriteError:
            errorStr = "写入错误";
            break;
        case QProcess::ReadError:
            errorStr = "读取错误";
            break;
        case QProcess::UnknownError:
        default:
            errorStr = "未知错误";
            break;
        }

        appendLog(QString("Aprog.exe程序错误: %1").arg(errorStr), "ERROR");
        updateAprogStatus();
    }

    void BurnControlWidget::onAprogOutputReceived(const QString& output)
    {
        appendLog(QString("Aprog输出: %1").arg(output), "DEBUG");
    }

    void BurnControlWidget::onAprogStatusTimerTimeout()
    {
        updateAprogStatus();
    }

    void BurnControlWidget::updateAprogStatus()
    {
        if (!m_aprogStatusLabel) return;

        QString statusText;
        QString styleSheet;

        bool isRunningBySharedMemory = isAprogRunningBySharedMemory();
        bool isRunningByDevice = m_burnDevice ? m_burnDevice->isAprogRunning() : false;
        bool isActuallyRunning = isRunningBySharedMemory || isRunningByDevice;

        if (isActuallyRunning) {
            if (isRunningBySharedMemory) {
                statusText = "运行中 (共享内存检测)";
            }
            else {
                statusText = "运行中 (进程检测)";
            }
            styleSheet = "color: green; font-weight: bold;";
        }
        else {
            statusText = "未启动";
            styleSheet = "color: gray; font-weight: bold;";
        }

        m_aprogStatusLabel->setText(statusText);
        m_aprogStatusLabel->setStyleSheet(styleSheet);
    }

    // 连接管理槽函数
    void BurnControlWidget::onConnectClicked()
    {
        if (!m_jsonRpcClient) {
            appendLog("JsonRpcClient未初始化", "ERROR");
            return;
        }

        QString host = m_hostEdit->text().trimmed();
        quint16 port = static_cast<quint16>(m_portSpin->value());

        if (host.isEmpty()) {
            appendLog("请输入服务器地址", "WARNING");
            return;
        }

        m_progressBar->setVisible(true);
        appendLog(QString("正在连接到 %1:%2...").arg(host).arg(port), "INFO");

        m_jsonRpcClient->connectToServer(host, port, true);
    }

    void BurnControlWidget::onDisconnectClicked()
    {
        if (!m_jsonRpcClient) return;

        appendLog("正在断开连接...", "INFO");
        m_jsonRpcClient->disconnectFromServer();
    }

    void BurnControlWidget::onConnectionStateChanged(JsonRpcClient::ConnectionState state)
    {
        switch (state) {
        case JsonRpcClient::Disconnected:
            m_statusLabel->setText("未连接");
            m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
            m_progressBar->setVisible(false);
            m_isConnected = false;
            appendLog("已断开连接", "WARNING");
            break;

        case JsonRpcClient::Connecting:
            m_statusLabel->setText("正在连接...");
            m_statusLabel->setStyleSheet("color: orange; font-weight: bold;");
            m_progressBar->setVisible(true);
            break;

        case JsonRpcClient::Connected:
            m_statusLabel->setText("已连接");
            m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
            m_progressBar->setVisible(false);
            m_isConnected = true;
            appendLog("连接成功", "SUCCESS");
            break;

        case JsonRpcClient::Reconnecting:
            m_statusLabel->setText("正在重连...");
            m_statusLabel->setStyleSheet("color: orange; font-weight: bold;");
            m_progressBar->setVisible(true);
            appendLog("正在尝试重新连接...", "INFO");
            break;
        }

        updateConnectionStatus();
        setControlsEnabled(m_burnDevice != nullptr);
    }

    void BurnControlWidget::onClientError(const QString& error)
    {
        appendLog(QString("客户端错误: %1").arg(error), "ERROR");
    }

    void BurnControlWidget::updateConnectionStatus()
    {
        m_connectBtn->setEnabled(!m_isConnected);
        m_disconnectBtn->setEnabled(m_isConnected);
        enableMethodButtons(m_isConnected);

        if (m_siteJobGroup) {
            m_siteJobGroup->setEnabled(m_isConnected && !m_siteList.isEmpty());
        }
    }

    // RPC方法槽函数
    void BurnControlWidget::onLoadProjectClicked()
    {
        if (!m_jsonRpcClient) return;

        QString projectPath = m_projectPathEdit->text().trimmed();
        QString taskFile = m_taskFileEdit->text().trimmed();

        if (projectPath.isEmpty() || taskFile.isEmpty()) {
            appendLog("请先选择项目路径和任务文件", "WARNING");
            return;
        }

        appendLog(QString("加载项目: %1 / %2").arg(projectPath).arg(taskFile), "INFO");
        m_requestCounter++;

        m_jsonRpcClient->loadProject(projectPath, taskFile,
            [this](bool success, const QJsonObject& result, const QString& error) {
                onRpcResponse("LoadProject", success, result, error);
            });
    }

    void BurnControlWidget::onSiteScanAndConnectClicked()
    {
        if (!m_jsonRpcClient) return;

        appendLog("开始扫描站点...", "INFO");
        m_requestCounter++;

        // 清空之前的站点列表
        m_siteList.clear();
        m_siteComboBox->clear();

        m_jsonRpcClient->siteScanAndConnect("",
            [this](bool success, const QJsonObject& result, const QString& error) {
                onRpcResponse("SiteScanAndConnect", success, result, error);
            });
    }

    void BurnControlWidget::onDoJobClicked()
    {
        if (!m_jsonRpcClient) return;

        appendLog("执行DoJob...", "INFO");
        m_requestCounter++;

        // 这里可以添加DoJob的具体实现
        QJsonObject cmdSequence; // 需要根据实际情况构建

        m_jsonRpcClient->doJob("192.168.1.100", 0xFFFF, cmdSequence,
            [this](bool success, const QJsonObject& result, const QString& error) {
                onRpcResponse("DoJob", success, result, error);
            });
    }

    void BurnControlWidget::onDoCustomClicked()
    {
        if (!m_jsonRpcClient) return;

        QString jsonText = m_customJsonEdit->toPlainText().trimmed();
        if (jsonText.isEmpty()) {
            appendLog("请先输入自定义JSON参数", "WARNING");
            return;
        }

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(jsonText.toUtf8(), &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            appendLog(QString("JSON解析错误: %1").arg(parseError.errorString()), "ERROR");
            return;
        }

        if (!doc.isObject()) {
            appendLog("JSON必须是对象格式", "ERROR");
            return;
        }

        appendLog("执行自定义命令...", "INFO");
        m_requestCounter++;

        m_jsonRpcClient->doCustom(doc.object(),
            [this](bool success, const QJsonObject& result, const QString& error) {
                onRpcResponse("DoCustom", success, result, error);
            });
    }

    // 结果处理
    void BurnControlWidget::onNotificationReceived(const QString& method, const QJsonObject& params)
    {
        addLogEntry("NOTIFY", QString("收到通知: %1").arg(method), params);

        // 添加到通知树
        auto* item = new QTreeWidgetItem(m_notificationTreeWidget);
        item->setText(0, QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));
        item->setText(1, method);
        item->setText(2, QString::fromUtf8(QJsonDocument(params).toJson(QJsonDocument::Compact)));

        // 解析特定的通知
        parseSiteNotification(method, params);

        // 如果是项目信息结果，尝试解析命令序列，并更新 ComboBox
        // 兼容多种服务端格式：
        // 1) {"result": {"proInfo": {"doCmdSequenceArray": [...]}, "data": {"strIp":...}}}
        // 2) 直接携带 {"proInfo": {...}} / {"data": {...}}
        if (method == "setProjectInfo" || method == "GetProjectInfo" || method == "GetProjectInfoExt") {
            QJsonObject root = params;
            if (root.contains("result") && root.value("result").isObject()) {
                root = root.value("result").toObject();
            }
            QJsonObject proInfo = root.value("proInfo").toObject();
            QJsonObject dataObj = root.value("data").toObject();
            // 更新站点IP（如果提供）
            if (dataObj.contains("strIp")) {
                if (m_currentSiteInfo.isEmpty()) m_currentSiteInfo = QJsonObject();
                m_currentSiteInfo["ip"] = dataObj.value("strIp");
                updateSiteInfoDisplay();
            }
            if (!proInfo.isEmpty()) {
                // 清空旧的命令序列
                m_cmdSequences.clear();
                m_cmdSequenceComboBox->clear();

                // 直接解析 doCmdSequenceArray
                QJsonArray arr = proInfo.value("doCmdSequenceArray").toArray();
                for (const auto& v : arr) {
                    QJsonObject entry = v.toObject();
                    QString cmdRun = entry.value("CmdRun").toString();
                    QJsonArray seqs = entry.value("CmdSequences").toArray();
                    if (!seqs.isEmpty()) {
                        // 用 CmdRun 作为分组名
                        QJsonObject seqGroup;
                        seqGroup["CmdRun"] = cmdRun;
                        seqGroup["CmdSequences"] = seqs;
                        seqGroup["CmdSequencesGroupCnt"] = seqs.size();
                        QString key = cmdRun.isEmpty() ? QString::number(m_cmdSequences.size()) : cmdRun;
                        m_cmdSequences[key] = seqGroup;
                        m_cmdSequenceComboBox->addItem(key);
                    }
                }
                // 回退：递归方式挖掘
                if (m_cmdSequences.isEmpty()) {
                    extractCommandSequences(proInfo);
                }
                updateExecuteButtonState();
            }
        }
    }

    void BurnControlWidget::onRpcResponse(const QString& method, bool success,
        const QJsonObject& result, const QString& error)
    {
        QString status = success ? "成功" : "失败";
        QString message = success ?
            QString("方法 %1 执行成功").arg(method) :
            QString("方法 %1 执行失败: %2").arg(method).arg(error);

        addLogEntry(success ? "SUCCESS" : "ERROR", message, result);

        // 添加到响应树
        auto* item = new QTreeWidgetItem(m_responseTreeWidget);
        item->setText(0, QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));
        item->setText(1, method);
        item->setText(2, status);

        if (success) {
            item->setText(3, QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Compact)));

            // 处理特定的响应
            if (method == "LoadProject") {
                parseProjectInfo(result);
            }
            else if (method == "GetSKTInfo") {
                parseSKTInfo(result);
            }
        }
        else {
            item->setText(3, error);
        }
    }

    // 日志管理
    void BurnControlWidget::addLogEntry(const QString& type, const QString& message,
        const QJsonObject& data)
    {
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        QString prefix = QString("[%1] [%2]").arg(timestamp).arg(type);

        QString logText = QString("%1 %2").arg(prefix).arg(message);

        if (!data.isEmpty() && m_prettyJsonBox && m_prettyJsonBox->isChecked()) {
            QJsonDocument doc(data);
            logText += "\n" + QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
        }

        m_logTextEdit->append(logText);

        if (m_autoScrollBox && m_autoScrollBox->isChecked()) {
            m_logTextEdit->moveCursor(QTextCursor::End);
        }
    }

    void BurnControlWidget::onClearLogsClicked()
    {
        m_logTextEdit->clear();
        m_responseTreeWidget->clear();
        m_notificationTreeWidget->clear();
        m_requestCounter = 0;
    }

    void BurnControlWidget::onAutoScrollToggled(bool enabled)
    {
        if (enabled) {
            m_logTextEdit->moveCursor(QTextCursor::End);
        }
    }

    // 辅助方法
    void BurnControlWidget::enableMethodButtons(bool enabled)
    {
        m_loadProjectBtn->setEnabled(enabled);
        m_siteScanBtn->setEnabled(true);
        m_doJobBtn->setEnabled(enabled);
        m_doCustomBtn->setEnabled(enabled);
        m_executeCustomBtn->setEnabled(enabled);
        m_ag06SendUidBtn->setEnabled(enabled);
        m_ag06SendTrimBtn->setEnabled(enabled);
    }

    void BurnControlWidget::displayJsonData(const QString& title, const QJsonObject& data)
    {
        if (data.isEmpty()) return;

        QString formatted = m_prettyJsonBox->isChecked() ?
            QString::fromUtf8(QJsonDocument(data).toJson(QJsonDocument::Indented)) :
            QString::fromUtf8(QJsonDocument(data).toJson(QJsonDocument::Compact));

        addLogEntry("DATA", title, data);
    }

    // 站点管理
    void BurnControlWidget::parseSiteNotification(const QString& method, const QJsonObject& params)
    {
        if (method == "setSiteResult") {
            // 解析站点信息
            QJsonObject siteInfo;
            siteInfo["siteAlias"] = params.value("SiteAlias").toString();
            siteInfo["mac"] = params.value("MacAddr").toString();
            siteInfo["ip"] = params.value("ip").toString();

            // 添加到站点列表
            m_siteList.append(siteInfo);
            m_siteComboBox->addItem(QString("%1 (%2)")
                .arg(siteInfo["siteAlias"].toString())
                .arg(siteInfo["ip"].toString()));

            if (m_siteList.size() == 1) {
                m_siteComboBox->setCurrentIndex(0);
            }

            m_siteJobGroup->setEnabled(true);
        }

        // 服务器会通过 setLoadProjectResult 回传项目信息（JsonRpcTestWidget 同步逻辑）
        if (method == "setLoadProjectResult") {
            parseLoadProjectResult(params);
        }
    }

    void BurnControlWidget::parseLoadProjectResult(const QJsonObject& params)
    {
        // 与 JsonRpcTestWidget 行为保持一致：
        // 期望 params 内包含：
        // - data.strIp
        // - proInfo.doCmdSequenceArray
        QJsonObject data = params.value("data").toObject();
        QJsonObject proInfo = params.value("proInfo").toObject();

        if (data.contains("strIp")) {
            if (m_currentSiteInfo.isEmpty()) m_currentSiteInfo = QJsonObject();
            m_currentSiteInfo["ip"] = data.value("strIp");
            updateSiteInfoDisplay();
        }

        QJsonArray seqArray = proInfo.value("doCmdSequenceArray").toArray();
        if (!seqArray.isEmpty()) {
            m_cmdSequences.clear();
            m_cmdSequenceComboBox->clear();

            for (const auto& v : seqArray) {
                QJsonObject entry = v.toObject();
                QString group = entry.value("CmdRun").toString();
                QJsonArray seqs = entry.value("CmdSequences").toArray();
                if (seqs.isEmpty()) continue;

                QJsonObject groupObj;
                groupObj["CmdRun"] = group;
                groupObj["CmdSequences"] = seqs;
                QString key = group.isEmpty() ? QString::number(m_cmdSequences.size()) : group;
                m_cmdSequences[key] = groupObj;
                m_cmdSequenceComboBox->addItem(key);
            }
            updateExecuteButtonState();
        }
    }

    void BurnControlWidget::onSiteSelectionChanged()
    {
        int index = m_siteComboBox->currentIndex();
        if (index < 0 || index >= m_siteList.size()) return;

        m_currentSiteInfo = m_siteList[index];
        updateSiteInfoDisplay();
        updateSocketDisplay();
    }

    void BurnControlWidget::updateSiteInfoDisplay()
    {
        if (m_currentSiteInfo.isEmpty()) {
            m_siteInfoLabel->setText("请选择站点");
            return;
        }

        QString info = QString("站点: %1\nMAC: %2\nIP: %3")
            .arg(m_currentSiteInfo["siteAlias"].toString())
            .arg(m_currentSiteInfo["mac"].toString())
            .arg(m_currentSiteInfo["ip"].toString());

        m_siteInfoLabel->setText(info);
    }

    void BurnControlWidget::updateSocketDisplay()
    {
        // 根据当前站点信息更新socket显示
        for (int i = 0; i < 16; ++i) {
            m_socketCheckBoxes[i]->setEnabled(true);
        }
    }

    void BurnControlWidget::onSocketToggled(int socketIndex, bool enabled)
    {
        Q_UNUSED(socketIndex)
            Q_UNUSED(enabled)

            // 更新已选择的socket显示
            QStringList selected;
        int count = 0;
        quint32 bpuEn = 0;

        for (int i = 0; i < 16; ++i) {
            if (m_socketCheckBoxes[i]->isChecked()) {
                selected << QString::number(i);
                count++;
                bpuEn |= (1 << i);
            }
        }

        if (count == 0) {
            m_selectedSocketsLabel->setText("已选择: 无");
        }
        else {
            m_selectedSocketsLabel->setText(QString("已选择: %1 (BPUEn: 0x%2)")
                .arg(selected.join(", "))
                .arg(bpuEn, 4, 16, QChar('0')));
        }

        updateExecuteButtonState();
    }

    void BurnControlWidget::onCmdSequenceSelectionChanged()
    {
        updateExecuteButtonState();
    }

    void BurnControlWidget::updateExecuteButtonState()
    {
        bool hasSelectedSocket = false;
        for (int i = 0; i < 16; ++i) {
            if (m_socketCheckBoxes[i]->isChecked()) {
                hasSelectedSocket = true;
                break;
            }
        }

        bool hasCmdSequence = m_cmdSequenceComboBox->currentIndex() >= 0;
        m_executeJobBtn->setEnabled(hasSelectedSocket && hasCmdSequence);
    }

    void BurnControlWidget::onExecuteJobClicked()
    {
        if (!m_jsonRpcClient || m_currentSiteInfo.isEmpty()) return;

        QString ip = m_currentSiteInfo["ip"].toString();
        quint32 sktEn = 0;

        for (int i = 0; i < 16; ++i) {
            if (m_socketCheckBoxes[i]->isChecked()) {
                sktEn |= (1 << i);
            }
        }

        QString cmdSeqName = m_cmdSequenceComboBox->currentText();
        QJsonObject cmdSeq = m_cmdSequences[cmdSeqName];

        appendLog(QString("执行作业 - IP: %1, Socket: 0x%2, 命令: %3")
            .arg(ip).arg(sktEn, 4, 16, QChar('0')).arg(cmdSeqName), "INFO");

        m_jsonRpcClient->doJob(ip, sktEn, cmdSeq,
            [this](bool success, const QJsonObject& result, const QString& error) {
                onRpcResponse("DoJob", success, result, error);
            });
    }

    // 解析方法
    void BurnControlWidget::parseProjectInfo(const QJsonObject& result)
    {
        m_currentProjectInfo = result;

        // 提取命令序列
        m_cmdSequences.clear();
        m_cmdSequenceComboBox->clear();

        extractCommandSequences(result);

        if (!m_cmdSequences.isEmpty()) {
            m_cmdSequenceComboBox->setCurrentIndex(0);
        }
    }

    void BurnControlWidget::parseSKTInfo(const QJsonObject& result)
    {
        m_currentSKTInfo = result;
        // 根据SKT信息更新socket显示
        updateSocketDisplay();
    }

    void BurnControlWidget::extractCommandSequences(const QJsonObject& jsonObject, const QString& parentKey)
    {
        for (auto it = jsonObject.begin(); it != jsonObject.end(); ++it) {
            QString key = parentKey.isEmpty() ? it.key() : parentKey + "." + it.key();

            if (it.value().isObject()) {
                QJsonObject obj = it.value().toObject();

                // 检查是否是命令序列
                if (obj.contains("commands") || obj.contains("sequence")) {
                    m_cmdSequences[key] = obj;
                    m_cmdSequenceComboBox->addItem(key);
                }
                else {
                    // 递归搜索
                    extractCommandSequences(obj, key);
                }
            }
        }
    }

    // 自定义JSON编辑
    void BurnControlWidget::onCustomJsonTextChanged()
    {
        QString text = m_customJsonEdit->toPlainText().trimmed();

        if (text.isEmpty()) {
            m_jsonStatusLabel->setText("JSON状态: 未输入");
            m_jsonStatusLabel->setStyleSheet("QLabel { color: gray; }");
            return;
        }

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &error);

        if (error.error == QJsonParseError::NoError) {
            m_jsonStatusLabel->setText("JSON状态: 有效");
            m_jsonStatusLabel->setStyleSheet("QLabel { color: green; }");
        }
        else {
            m_jsonStatusLabel->setText(QString("JSON状态: 错误 - %1").arg(error.errorString()));
            m_jsonStatusLabel->setStyleSheet("QLabel { color: red; }");
        }
    }

    void BurnControlWidget::onFormatJsonClicked()
    {
        QString text = m_customJsonEdit->toPlainText().trimmed();
        if (text.isEmpty()) return;

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &error);

        if (error.error == QJsonParseError::NoError) {
            m_customJsonEdit->setPlainText(QString::fromUtf8(doc.toJson(QJsonDocument::Indented)));
        }
    }

    void BurnControlWidget::onClearJsonClicked()
    {
        m_customJsonEdit->clear();
    }

    void BurnControlWidget::onExecuteCustomClicked()
    {
        onDoCustomClicked();
    }

    // AG06协议实现
    void BurnControlWidget::ag06SendUid()
    {
        if (!m_jsonRpcClient || m_currentSiteInfo.isEmpty()) {
            appendLog("请先连接服务器并选择站点", "WARNING");
            return;
        }

        QString uid = m_ag06UidEdit->text().trimmed();
        if (uid.length() != 8) {
            appendLog("UID必须为8个字符", "ERROR");
            return;
        }

        // 使用Ag06DoCustomProtocol发送UID
        Ag06DoCustomProtocol protocol;
        protocol.setClient(m_jsonRpcClient);

        QString ip = m_currentSiteInfo["ip"].toString();
        quint32 sktEn = 0xFFFF; // 默认所有socket

        protocol.sendUid(ip, sktEn);
        appendLog(QString("发送UID: %1 到 %2").arg(uid).arg(ip), "INFO");
    }

    void BurnControlWidget::ag06SendJsonTrim()
    {
        if (!m_jsonRpcClient || m_currentSiteInfo.isEmpty()) {
            appendLog("请先连接服务器并选择站点", "WARNING");
            return;
        }

        QString trimJson = m_ag06TrimJsonEdit->toPlainText().trimmed();
        if (trimJson.isEmpty()) {
            appendLog("请输入Trim JSON数据", "WARNING");
            return;
        }

        // 使用Ag06DoCustomProtocol发送Trim
        Ag06DoCustomProtocol protocol;
        protocol.setClient(m_jsonRpcClient);

        QString ip = m_currentSiteInfo["ip"].toString();
        quint32 sktEn = 0xFFFF;

        protocol.sendTrimFromJson(trimJson, ip, sktEn);
        appendLog("发送Trim配置", "INFO");
    }

    void BurnControlWidget::openAg06TrimDialog()
    {
        Ag06TrimDialog dlg(this);
        // 导入当前自定义区的 JSON 文本
        connect(&dlg, &Ag06TrimDialog::sendUidRequested, this, [this](const QString& uid) {
            if (uid.isEmpty()) return;
            if (!m_jsonRpcClient || m_currentSiteInfo.isEmpty()) return;
            Ag06DoCustomProtocol proto; proto.setClient(m_jsonRpcClient);
            QString ip = m_currentSiteInfo.value("ip").toString();
            quint32 sktEn = 0xFFFF;
            proto.sendUid(ip, sktEn);
            appendLog(QString("发送UID: %1 -> %2").arg(uid).arg(ip), "INFO");
            });
        connect(&dlg, &Ag06TrimDialog::sendTrimJsonRequested, this, [this](const QString& json) {
            if (!m_jsonRpcClient || m_currentSiteInfo.isEmpty()) return;
            Ag06DoCustomProtocol proto; proto.setClient(m_jsonRpcClient);
            QString ip = m_currentSiteInfo.value("ip").toString();
            quint32 sktEn = 0xFFFF;
            proto.sendTrimFromJson(json, ip, sktEn);
            appendLog("发送Trim JSON", "INFO");
            });
        connect(&dlg, &Ag06TrimDialog::sendTrimStructRequested, this, [this](const xt_trim_t& trim) {
            if (!m_jsonRpcClient || m_currentSiteInfo.isEmpty()) return;
            Ag06DoCustomProtocol proto; proto.setClient(m_jsonRpcClient);
            QString ip = m_currentSiteInfo.value("ip").toString();
            quint32 sktEn = 0xFFFF;
            proto.sendTrim(trim, ip, sktEn);
            appendLog("发送Trim结构体", "INFO");
            });
        dlg.exec();
    }

    // 其他辅助函数
    QString BurnControlWidget::formatBPUEnBinary(quint32 bpuEn)
    {
        QString binary;
        for (int i = 15; i >= 0; --i) {
            binary += (bpuEn & (1 << i)) ? "1" : "0";
            if (i % 4 == 0 && i > 0) binary += " ";
        }
        return binary;
    }

    QBitArray BurnControlWidget::convertBPUEnToBitArray(quint32 bpuEn)
    {
        QBitArray bits(16);
        for (int i = 0; i < 16; ++i) {
            bits[i] = (bpuEn & (1 << i)) != 0;
        }
        return bits;
    }

    // 空实现的函数（需要时可以扩展）
    void BurnControlWidget::setupAg06TrimEditorUi() {}
    void BurnControlWidget::updateCmdSequenceDisplay() {}
    void BurnControlWidget::parseAndUpdateUI(const QString& taskContent) { Q_UNUSED(taskContent) }
    QJsonObject BurnControlWidget::buildTrimJsonFromUi() { return QJsonObject(); }
    void BurnControlWidget::setupRegBitRow(QGridLayout* grid, int row, const QString& title,
        RegBitWidgets& w)
    {
        Q_UNUSED(grid)
            Q_UNUSED(row)
            Q_UNUSED(title)
            Q_UNUSED(w)
    }

} // namespace Presentation
