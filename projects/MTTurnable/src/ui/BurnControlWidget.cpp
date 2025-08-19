#include "ui/BurnControlWidget.h"
#include "domain/BurnDevice.h"
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
#include <QFont>
#include <QDebug>

namespace Presentation {

BurnControlWidget::BurnControlWidget(QWidget *parent)
    : IDeviceControlWidget(parent)
    , m_burnDevice(nullptr)
    , m_isDeviceConnected(false)
{
    setupUi();
}

BurnControlWidget::~BurnControlWidget()
{
    if (m_burnDevice) {
        disconnect(m_burnDevice.get(), nullptr, this, nullptr);
    }
}

void BurnControlWidget::setDevice(std::shared_ptr<Domain::IDevice> device)
{
    appendLog("setDevice 被调用", "DEBUG");
    
    if (m_burnDevice) {
        appendLog("断开之前的设备连接", "DEBUG");
        disconnect(m_burnDevice.get(), nullptr, this, nullptr);
    }
    
    if (!device) {
        m_burnDevice.reset();
        setControlsEnabled(false);
        appendLog("设备已断开连接", "WARNING");
        return;
    }
    
    appendLog(QString("尝试设置设备: %1, 类型: %2")
             .arg(device->getName())
             .arg(static_cast<int>(device->getType())), "DEBUG");
    
    m_burnDevice = std::dynamic_pointer_cast<Domain::BurnDevice>(device);
    if (m_burnDevice) {
        appendLog("设备类型转换成功", "DEBUG");
        
        try {
            connectSignals();
            updateDeviceStatus();
            setControlsEnabled(true);
            appendLog(QString("设备已连接: %1").arg(device->getName()), "INFO");
            
            // 更新UI中的服务器配置
            if (m_serverHostEdit && m_serverPortSpin) {
                m_serverHostEdit->setText(m_burnDevice->getServerHost());
                m_serverPortSpin->setValue(m_burnDevice->getServerPort());
                appendLog("服务器配置UI已更新", "DEBUG");
            }
            
            // 更新UI中的Aprog路径配置
            if (m_aprogPathEdit) {
                QString configuredPath = m_burnDevice->getAprogPath();
                if (!configuredPath.isEmpty()) {
                    m_aprogPathEdit->setText(configuredPath);
                    appendLog(QString("从配置加载Aprog路径: %1").arg(configuredPath), "INFO");
                } else {
                    appendLog("配置中未找到Aprog路径，使用默认值", "WARNING");
                }
            }
            updateConnectionStatus();
            updateAprogStatus();
        } catch (const std::exception &e) {
            appendLog(QString("设置设备时发生异常: %1").arg(e.what()), "ERROR");
            setControlsEnabled(false);
        }
    } else {
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
    updateDeviceStatus();
    updateConnectionStatus();
}

void BurnControlWidget::setControlsEnabled(bool enabled)
{
    // 基本操作按钮
    if (m_siteScanConnectBtn) m_siteScanConnectBtn->setEnabled(enabled);
    if (m_getProjectInfoBtn) m_getProjectInfoBtn->setEnabled(enabled);
    if (m_getSKTInfoBtn) m_getSKTInfoBtn->setEnabled(enabled);
    if (m_doJobBtn) m_doJobBtn->setEnabled(enabled);
    
    // 项目管理
    if (m_projectPathEdit) m_projectPathEdit->setEnabled(enabled);
    if (m_browseProjectBtn) m_browseProjectBtn->setEnabled(enabled);
    if (m_loadProjectBtn) m_loadProjectBtn->setEnabled(enabled);
    if (m_projectInfoExtEdit) m_projectInfoExtEdit->setEnabled(enabled);
    if (m_getProjectInfoExtBtn) m_getProjectInfoExtBtn->setEnabled(enabled);
    
    // 高级操作
    if (m_strIPEdit) m_strIPEdit->setEnabled(enabled);
    if (m_nHopNumSpin) m_nHopNumSpin->setEnabled(enabled);
    if (m_portIDSpin) m_portIDSpin->setEnabled(enabled);
    if (m_cmdFlagSpin) m_cmdFlagSpin->setEnabled(enabled);
    if (m_cmdIDSpin) m_cmdIDSpin->setEnabled(enabled);
    if (m_sktEnEdit) m_sktEnEdit->setEnabled(enabled);
    if (m_bpuIDSpin) m_bpuIDSpin->setEnabled(enabled);
    if (m_docmdSeqJsonEdit) m_docmdSeqJsonEdit->setEnabled(enabled);
    if (m_doCustomBtn) m_doCustomBtn->setEnabled(enabled);
    
    // UUID操作
    if (m_uuidEdit) m_uuidEdit->setEnabled(enabled);
    if (m_uuidStrIPEdit) m_uuidStrIPEdit->setEnabled(enabled);
    if (m_uuidNHopNumSpin) m_uuidNHopNumSpin->setEnabled(enabled);
    if (m_uuidSktEnableEdit) m_uuidSktEnableEdit->setEnabled(enabled);
    if (m_sendUUIDBtn) m_sendUUIDBtn->setEnabled(enabled);
    
    // 服务器配置始终可用
    if (m_serverHostEdit) m_serverHostEdit->setEnabled(enabled);
    if (m_serverPortSpin) m_serverPortSpin->setEnabled(enabled);
    
    // Aprog程序管理始终可用
    if (m_aprogPathEdit) m_aprogPathEdit->setEnabled(true);
    if (m_browseAprogBtn) m_browseAprogBtn->setEnabled(true);
    if (m_startAprogBtn) m_startAprogBtn->setEnabled(true);
    if (m_stopAprogBtn) m_stopAprogBtn->setEnabled(true);
    
    // 日志控制始终可用
    if (m_clearLogBtn) m_clearLogBtn->setEnabled(true);
}

QString BurnControlWidget::getDeviceTypeName() const
{
    return tr("烧录设备控制");
}

void BurnControlWidget::setupUi()
{
    auto mainLayout = new QVBoxLayout(this);
    
    // 使用分割器来组织界面
    auto splitter = new QSplitter(Qt::Vertical, this);
    
    // 上半部分：控制面板
    auto controlWidget = new QWidget();
    auto controlLayout = new QVBoxLayout(controlWidget);
    
    createConnectionGroup();
    createAprogGroup();
    createBasicOperationsGroup();
    createAdvancedOperationsGroup();
    createLoggingGroup();
    
    controlLayout->addWidget(m_connectionGroup);
    controlLayout->addWidget(m_aprogGroup);
    controlLayout->addWidget(m_basicOpsGroup);
    controlLayout->addWidget(m_advancedOpsGroup);
    
    // 下半部分：日志
    auto logWidget = new QWidget();
    auto logLayout = new QVBoxLayout(logWidget);
    logLayout->addWidget(m_loggingGroup);
    
    splitter->addWidget(controlWidget);
    splitter->addWidget(logWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    
    mainLayout->addWidget(splitter);
    
    // 初始化UI状态
    setControlsEnabled(false);
    updateConnectionStatus();
    
    // 设置默认值
    initializeDefaultValues();
}

void BurnControlWidget::createConnectionGroup()
{
    m_connectionGroup = new QGroupBox("服务器配置", this);
    auto layout = new QFormLayout(m_connectionGroup);
    
    // 服务器配置
    m_serverHostEdit = new QLineEdit("127.0.0.1");
    m_serverPortSpin = new QSpinBox();
    m_serverPortSpin->setRange(1, 65535);
    m_serverPortSpin->setValue(12345);
    
    auto hostPortLayout = new QHBoxLayout();
    hostPortLayout->addWidget(m_serverHostEdit, 2);
    hostPortLayout->addWidget(new QLabel(":"));
    hostPortLayout->addWidget(m_serverPortSpin, 1);
    
    // 添加连接按钮
    auto connectBtn = new QPushButton("连接");
    connectBtn->setMaximumWidth(80);
    hostPortLayout->addWidget(connectBtn);
    
    layout->addRow("服务器地址:", hostPortLayout);
    
    // 连接状态
    m_connectionStatusLabel = new QLabel("长连接模式 - 未连接");
    m_connectionStatusLabel->setStyleSheet("color: red; font-weight: bold;");
    layout->addRow("连接状态:", m_connectionStatusLabel);
    
    // 连接信号
    connect(m_serverHostEdit, &QLineEdit::textChanged, this, &BurnControlWidget::onServerConfigChanged);
    connect(m_serverPortSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &BurnControlWidget::onServerConfigChanged);
    connect(connectBtn, &QPushButton::clicked, this, &BurnControlWidget::onConnectButtonClicked);
}

void BurnControlWidget::createBasicOperationsGroup()
{
    m_basicOpsGroup = new QGroupBox("基本操作", this);
    auto layout = new QVBoxLayout(m_basicOpsGroup);
    
    // 第一行：基本查询操作
    auto row1 = new QHBoxLayout();
    m_siteScanConnectBtn = new QPushButton("站点扫描和连接");
    m_getProjectInfoBtn = new QPushButton("获取项目信息");
    m_getSKTInfoBtn = new QPushButton("获取Socket信息");
    
    row1->addWidget(m_siteScanConnectBtn);
    row1->addWidget(m_getProjectInfoBtn);
    row1->addWidget(m_getSKTInfoBtn);
    row1->addStretch();
    
    // 第二行：项目管理
    m_projectGroup = new QGroupBox("项目管理");
    auto projectLayout = new QFormLayout(m_projectGroup);
    
    auto projectFileLayout = new QHBoxLayout();
    m_projectPathEdit = new QLineEdit();
    m_projectPathEdit->setPlaceholderText("选择项目文件路径");
    m_browseProjectBtn = new QPushButton("浏览...");
    m_loadProjectBtn = new QPushButton("加载项目");
    
    projectFileLayout->addWidget(m_projectPathEdit, 3);
    projectFileLayout->addWidget(m_browseProjectBtn, 1);
    projectFileLayout->addWidget(m_loadProjectBtn, 1);
    
    projectLayout->addRow("项目文件:", projectFileLayout);
    
    auto projectInfoExtLayout = new QHBoxLayout();
    m_projectInfoExtEdit = new QLineEdit("C:/Users/Administrator/Desktop/XT422/AIPE/Build/project/default.eapr");
    m_getProjectInfoExtBtn = new QPushButton("获取扩展项目信息");
    
    projectInfoExtLayout->addWidget(m_projectInfoExtEdit, 2);
    projectInfoExtLayout->addWidget(m_getProjectInfoExtBtn, 1);
    
    projectLayout->addRow("项目URL:", projectInfoExtLayout);
    
    // 第三行：执行操作
    auto row3 = new QHBoxLayout();
    m_doJobBtn = new QPushButton("执行作业");
    m_doJobBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    
    row3->addWidget(m_doJobBtn);
    row3->addStretch();
    
    layout->addLayout(row1);
    layout->addWidget(m_projectGroup);
    layout->addLayout(row3);
    
    // 连接信号
    connect(m_siteScanConnectBtn, &QPushButton::clicked, this, &BurnControlWidget::onSiteScanAndConnectClicked);
    connect(m_getProjectInfoBtn, &QPushButton::clicked, this, &BurnControlWidget::onGetProjectInfoClicked);
    connect(m_getSKTInfoBtn, &QPushButton::clicked, this, &BurnControlWidget::onGetSKTInfoClicked);
    connect(m_browseProjectBtn, &QPushButton::clicked, this, &BurnControlWidget::onBrowseProjectFileClicked);
    connect(m_loadProjectBtn, &QPushButton::clicked, this, &BurnControlWidget::onLoadProjectClicked);
    connect(m_getProjectInfoExtBtn, &QPushButton::clicked, this, &BurnControlWidget::onGetProjectInfoExtClicked);
    connect(m_doJobBtn, &QPushButton::clicked, this, &BurnControlWidget::onDoJobClicked);
}

void BurnControlWidget::createAdvancedOperationsGroup()
{
    m_advancedOpsGroup = new QGroupBox("高级操作", this);
    auto mainLayout = new QVBoxLayout(m_advancedOpsGroup);
    
    // DoCustom参数配置
    auto doCustomGroup = new QGroupBox("DoCustom 参数配置");
    auto doCustomLayout = new QGridLayout(doCustomGroup);
    
    // 参数输入控件
    m_strIPEdit = new QLineEdit("192.168.70.109");
    m_nHopNumSpin = new QSpinBox();
    m_nHopNumSpin->setRange(0, 999);
    m_nHopNumSpin->setValue(0);
    
    m_portIDSpin = new QSpinBox();
    m_portIDSpin->setRange(0, 999);
    m_portIDSpin->setValue(0);
    
    m_cmdFlagSpin = new QSpinBox();
    m_cmdFlagSpin->setRange(0, 999);
    m_cmdFlagSpin->setValue(0);
    
    m_cmdIDSpin = new QSpinBox();
    m_cmdIDSpin->setRange(0, 9999);
    m_cmdIDSpin->setValue(1047);
    
    m_sktEnEdit = new QLineEdit("0x0000FFFF");
    m_bpuIDSpin = new QSpinBox();
    m_bpuIDSpin->setRange(0, 999);
    m_bpuIDSpin->setValue(8);
    
    // 布局参数控件
    doCustomLayout->addWidget(new QLabel("目标IP:"), 0, 0);
    doCustomLayout->addWidget(m_strIPEdit, 0, 1);
    doCustomLayout->addWidget(new QLabel("Hop数:"), 0, 2);
    doCustomLayout->addWidget(m_nHopNumSpin, 0, 3);
    
    doCustomLayout->addWidget(new QLabel("端口ID:"), 1, 0);
    doCustomLayout->addWidget(m_portIDSpin, 1, 1);
    doCustomLayout->addWidget(new QLabel("命令标志:"), 1, 2);
    doCustomLayout->addWidget(m_cmdFlagSpin, 1, 3);
    
    doCustomLayout->addWidget(new QLabel("命令ID:"), 2, 0);
    doCustomLayout->addWidget(m_cmdIDSpin, 2, 1);
    doCustomLayout->addWidget(new QLabel("Socket使能:"), 2, 2);
    doCustomLayout->addWidget(m_sktEnEdit, 2, 3);
    
    doCustomLayout->addWidget(new QLabel("BPU ID:"), 3, 0);
    doCustomLayout->addWidget(m_bpuIDSpin, 3, 1);
    
    // JSON命令序列
    auto jsonLabel = new QLabel("命令序列JSON:");
    m_docmdSeqJsonEdit = new QTextEdit();
    m_docmdSeqJsonEdit->setMaximumHeight(120);
    
    // 默认JSON内容
    QJsonObject defaultCmd;
    defaultCmd["CmdID"] = "1806";
    defaultCmd["CmdRun"] = "Read";
    QJsonArray sequences;
    QJsonObject seqItem;
    seqItem["ID"] = "806";
    seqItem["Name"] = "Read";
    sequences.append(seqItem);
    defaultCmd["CmdSequences"] = sequences;
    defaultCmd["CmdSequencesGroupCnt"] = 1;
    
    QJsonDocument doc(defaultCmd);
    m_docmdSeqJsonEdit->setPlainText(doc.toJson(QJsonDocument::Indented));
    
    doCustomLayout->addWidget(jsonLabel, 4, 0, 1, 4);
    doCustomLayout->addWidget(m_docmdSeqJsonEdit, 5, 0, 1, 4);
    
    m_doCustomBtn = new QPushButton("执行DoCustom");
    m_doCustomBtn->setStyleSheet("QPushButton { background-color: #FF9800; color: white; font-weight: bold; }");
    doCustomLayout->addWidget(m_doCustomBtn, 6, 0, 1, 4);
    
    // UUID操作组
    m_uuidGroup = new QGroupBox("UUID 操作");
    auto uuidLayout = new QFormLayout(m_uuidGroup);
    
    m_uuidEdit = new QLineEdit("123");
    m_uuidStrIPEdit = new QLineEdit("192.168.70.109");
    m_uuidNHopNumSpin = new QSpinBox();
    m_uuidNHopNumSpin->setRange(0, 999);
    m_uuidNHopNumSpin->setValue(0);
    m_uuidSktEnableEdit = new QLineEdit("0x0000FFFF");
    
    uuidLayout->addRow("UUID:", m_uuidEdit);
    uuidLayout->addRow("目标IP:", m_uuidStrIPEdit);
    uuidLayout->addRow("Hop数:", m_uuidNHopNumSpin);
    uuidLayout->addRow("Socket使能:", m_uuidSktEnableEdit);
    
    m_sendUUIDBtn = new QPushButton("发送UUID");
    uuidLayout->addRow("", m_sendUUIDBtn);
    
    mainLayout->addWidget(doCustomGroup);
    mainLayout->addWidget(m_uuidGroup);
    
    // 连接信号
    connect(m_doCustomBtn, &QPushButton::clicked, this, &BurnControlWidget::onDoCustomClicked);
    connect(m_sendUUIDBtn, &QPushButton::clicked, this, &BurnControlWidget::onSendUUIDClicked);
}

void BurnControlWidget::createAprogGroup()
{
    m_aprogGroup = new QGroupBox("Aprog.exe 程序管理", this);
    auto layout = new QFormLayout(m_aprogGroup);
    
    // Aprog路径配置
    auto aprogPathLayout = new QHBoxLayout();
    m_aprogPathEdit = new QLineEdit();
    m_aprogPathEdit->setPlaceholderText("将从设备配置加载Aprog.exe路径");
    m_browseAprogBtn = new QPushButton("浏览...");
    m_browseAprogBtn->setMaximumWidth(80);
    
    aprogPathLayout->addWidget(m_aprogPathEdit, 3);
    aprogPathLayout->addWidget(m_browseAprogBtn, 1);
    
    layout->addRow("Aprog路径:", aprogPathLayout);
    
    // Aprog状态显示
    m_aprogStatusLabel = new QLabel("未启动");
    m_aprogStatusLabel->setStyleSheet("color: gray; font-weight: bold;");
    layout->addRow("程序状态:", m_aprogStatusLabel);
    
    // Aprog控制按钮
    auto buttonLayout = new QHBoxLayout();
    m_startAprogBtn = new QPushButton("启动程序");
    m_stopAprogBtn = new QPushButton("停止程序");
    m_startAprogBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    m_stopAprogBtn->setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; }");
    
    buttonLayout->addWidget(m_startAprogBtn);
    buttonLayout->addWidget(m_stopAprogBtn);
    buttonLayout->addStretch();
    
    layout->addRow("程序控制:", buttonLayout);
    
    // 连接信号
    connect(m_aprogPathEdit, &QLineEdit::textChanged, this, &BurnControlWidget::onAprogPathChanged);
    connect(m_browseAprogBtn, &QPushButton::clicked, this, &BurnControlWidget::onBrowseAprogPathClicked);
    connect(m_startAprogBtn, &QPushButton::clicked, this, &BurnControlWidget::onStartAprogClicked);
    connect(m_stopAprogBtn, &QPushButton::clicked, this, &BurnControlWidget::onStopAprogClicked);
}

void BurnControlWidget::createLoggingGroup()
{
    m_loggingGroup = new QGroupBox("操作日志", this);
    auto layout = new QVBoxLayout(m_loggingGroup);
    
    // 日志显示区域
    m_logTextEdit = new QTextEdit();
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setFont(QFont("Consolas", 9));
    
    // 控制按钮
    auto buttonLayout = new QHBoxLayout();
    m_clearLogBtn = new QPushButton("清除日志");
    buttonLayout->addWidget(m_clearLogBtn);
    buttonLayout->addStretch();
    
    layout->addWidget(m_logTextEdit);
    layout->addLayout(buttonLayout);
    
    // 连接信号
    connect(m_clearLogBtn, &QPushButton::clicked, this, &BurnControlWidget::clearLog);
    
    // 添加初始日志
    appendLog("烧录设备控制界面已初始化", "INFO");
}

void BurnControlWidget::connectSignals()
{
    if (!m_burnDevice) {
        appendLog("设备指针为空，无法连接信号", "WARNING");
        return;
    }
    
    try {
        // 断开之前的连接
        disconnect(m_burnDevice.get(), nullptr, this, nullptr);
        
        // 连接设备信号
        connect(m_burnDevice.get(), &Domain::BurnDevice::operationCompleted,
                this, &BurnControlWidget::onOperationCompleted);
        connect(m_burnDevice.get(), &Domain::BurnDevice::operationFailed,
                this, &BurnControlWidget::onOperationFailed);
        connect(m_burnDevice.get(), &Domain::IDevice::statusChanged,
                this, [this](Domain::IDevice::DeviceStatus status) {
                    Q_UNUSED(status)
                    updateDeviceStatus();
                });
        connect(m_burnDevice.get(), &Domain::BurnDevice::connected,
                this, &BurnControlWidget::onDeviceConnected);
        connect(m_burnDevice.get(), &Domain::BurnDevice::disconnected,
                this, &BurnControlWidget::onDeviceDisconnected);
        
        // 连接Aprog进程相关信号
        connect(m_burnDevice.get(), &Domain::BurnDevice::aprogStarted,
                this, &BurnControlWidget::onAprogStarted);
        connect(m_burnDevice.get(), &Domain::BurnDevice::aprogFinished,
                this, &BurnControlWidget::onAprogFinished);
        connect(m_burnDevice.get(), &Domain::BurnDevice::aprogError,
                this, &BurnControlWidget::onAprogError);
        connect(m_burnDevice.get(), &Domain::BurnDevice::aprogOutputReceived,
                this, &BurnControlWidget::onAprogOutputReceived);
        
        appendLog("设备信号连接成功", "DEBUG");
    } catch (const std::exception &e) {
        appendLog(QString("连接设备信号时发生异常: %1").arg(e.what()), "ERROR");
    }
}

void BurnControlWidget::initializeDefaultValues()
{
    // 设置一些默认值以便测试
    if (m_strIPEdit) m_strIPEdit->setText("192.168.70.109");
    if (m_uuidEdit) m_uuidEdit->setText("123");
    if (m_uuidStrIPEdit) m_uuidStrIPEdit->setText("192.168.70.109");
    if (m_projectInfoExtEdit) {
        m_projectInfoExtEdit->setText("C:/Users/Administrator/Desktop/XT422/AIPE/Build/project/default.eapr");
    }
    
    // Aprog路径由设备配置决定，不在这里设置默认值
    // 如果设备未连接或配置中没有aprogPath，则显示为空
    if (m_aprogPathEdit && !m_burnDevice) {
        m_aprogPathEdit->setPlaceholderText("请先连接设备以加载配置路径");
    }
}

// 按钮槽函数实现
void BurnControlWidget::onSiteScanAndConnectClicked()
{
    if (!m_burnDevice) {
        appendLog("设备未连接", "ERROR");
        return;
    }
    
    try {
        appendLog("===== 开始执行站点扫描和连接 =====", "INFO");
        appendLog(QString("当前设备状态: %1").arg(static_cast<int>(m_burnDevice->getStatus())), "DEBUG");
        appendLog(QString("设备连接状态: %1").arg(m_burnDevice->isConnected() ? "已连接" : "未连接"), "DEBUG");
        
        auto result = m_burnDevice->executeCommand("siteScanAndConnect", QJsonObject());
        
        appendLog(QString("executeCommand 返回结果: %1").arg(formatJsonForDisplay(result)), "DEBUG");
        
        if (result["success"].toBool()) {
            appendLog("站点扫描和连接请求已加入队列", "SUCCESS");
        } else {
            appendLog(QString("站点扫描和连接失败: %1").arg(result["error"].toString()), "ERROR");
        }
    } catch (const std::exception &e) {
        appendLog(QString("执行站点扫描时发生异常: %1").arg(e.what()), "ERROR");
    }
}

void BurnControlWidget::onLoadProjectClicked()
{
    if (!m_burnDevice) return;
    
    QString filePath = m_projectPathEdit->text().trimmed();
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择项目文件");
        return;
    }
    
    QFileInfo fileInfo(filePath);
    QString path = fileInfo.path();
    QString fileName = fileInfo.fileName();
    
    appendLog(QString("加载项目: %1/%2").arg(path).arg(fileName), "INFO");
    
    QJsonObject params;
    params["path"] = path;
    params["taskFileName"] = fileName;
    
    auto result = m_burnDevice->executeCommand("loadProject", params);
    
    if (result["success"].toBool()) {
        appendLog("项目加载请求已发送", "SUCCESS");
    } else {
        appendLog(QString("项目加载失败: %1").arg(result["error"].toString()), "ERROR");
    }
}

void BurnControlWidget::onDoJobClicked()
{
    if (!m_burnDevice) return;
    
    appendLog("执行作业...", "INFO");
    auto result = m_burnDevice->executeCommand("doJob", QJsonObject());
    
    if (result["success"].toBool()) {
        appendLog("作业执行请求已发送", "SUCCESS");
    } else {
        appendLog(QString("作业执行失败: %1").arg(result["error"].toString()), "ERROR");
    }
}

void BurnControlWidget::onGetProjectInfoClicked()
{
    if (!m_burnDevice) return;
    
    appendLog("获取项目信息...", "INFO");
    auto result = m_burnDevice->executeCommand("getProjectInfo", QJsonObject());
    
    if (result["success"].toBool()) {
        appendLog("项目信息查询请求已发送", "SUCCESS");
    } else {
        appendLog(QString("项目信息查询失败: %1").arg(result["error"].toString()), "ERROR");
    }
}

void BurnControlWidget::onDoCustomClicked()
{
    if (!m_burnDevice) return;
    
    // 构建DoCustom参数
    QJsonObject params = buildDoCustomParams();
    
    appendLog("执行DoCustom命令...", "INFO");
    appendLog(QString("参数: %1").arg(QString(QJsonDocument(params).toJson(QJsonDocument::Compact))), "DEBUG");
    
    auto result = m_burnDevice->executeCommand("doCustom", params);
    
    if (result["success"].toBool()) {
        appendLog("DoCustom请求已发送", "SUCCESS");
    } else {
        appendLog(QString("DoCustom执行失败: %1").arg(result["error"].toString()), "ERROR");
    }
}

void BurnControlWidget::onGetProjectInfoExtClicked()
{
    if (!m_burnDevice) return;
    
    QString projectUrl = m_projectInfoExtEdit->text().trimmed();
    if (projectUrl.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入项目URL");
        return;
    }
    
    appendLog(QString("获取扩展项目信息: %1").arg(projectUrl), "INFO");
    
    QJsonObject params;
    params["projectUrl"] = projectUrl;
    
    auto result = m_burnDevice->executeCommand("getProjectInfoExt", params);
    
    if (result["success"].toBool()) {
        appendLog("扩展项目信息查询请求已发送", "SUCCESS");
    } else {
        appendLog(QString("扩展项目信息查询失败: %1").arg(result["error"].toString()), "ERROR");
    }
}

void BurnControlWidget::onGetSKTInfoClicked()
{
    if (!m_burnDevice) return;
    
    appendLog("获取Socket信息...", "INFO");
    auto result = m_burnDevice->executeCommand("getSKTInfo", QJsonObject());
    
    if (result["success"].toBool()) {
        appendLog("Socket信息查询请求已发送", "SUCCESS");
    } else {
        appendLog(QString("Socket信息查询失败: %1").arg(result["error"].toString()), "ERROR");
    }
}

void BurnControlWidget::onSendUUIDClicked()
{
    if (!m_burnDevice) return;
    
    QString uuid = m_uuidEdit->text().trimmed();
    QString strIP = m_uuidStrIPEdit->text().trimmed();
    int nHopNum = m_uuidNHopNumSpin->value();
    
    // 解析sktEnable
    QString sktEnableStr = m_uuidSktEnableEdit->text().trimmed();
    bool ok;
    quint32 sktEnable = sktEnableStr.startsWith("0x") ? 
                       sktEnableStr.mid(2).toUInt(&ok, 16) : 
                       sktEnableStr.toUInt(&ok);
    
    if (!ok) {
        QMessageBox::warning(this, "警告", "Socket使能值格式错误");
        return;
    }
    
    appendLog(QString("发送UUID: %1 到 %2").arg(uuid).arg(strIP), "INFO");
    
    QJsonObject params;
    params["uuid"] = uuid;
    params["strIP"] = strIP;
    params["nHopNum"] = nHopNum;
    params["sktEnable"] = static_cast<double>(sktEnable);
    
    auto result = m_burnDevice->executeCommand("sendUUID", params);
    
    if (result["success"].toBool()) {
        appendLog("UUID发送请求已发送", "SUCCESS");
    } else {
        appendLog(QString("UUID发送失败: %1").arg(result["error"].toString()), "ERROR");
    }
}

// 设备信号槽实现
void BurnControlWidget::onOperationCompleted(const QString &operation, const QJsonObject &result)
{
    appendLog(QString("操作完成: %1").arg(operation), "SUCCESS");
    if (!result.isEmpty()) {
        appendLog(QString("结果: %1").arg(formatJsonForDisplay(result)), "INFO");
    }
    
    // 特殊处理连接操作
    if (operation == "connection") {
        updateConnectionStatus();
        updateDeviceStatus();
    }
}

void BurnControlWidget::onOperationFailed(const QString &operation, const QString &error)
{
    appendLog(QString("操作失败: %1 - %2").arg(operation).arg(error), "ERROR");
    
    // 特殊处理连接操作失败
    if (operation == "connection") {
        updateConnectionStatus();
        updateDeviceStatus();
    }
}

// 服务器配置相关槽函数
void BurnControlWidget::onServerConfigChanged()
{
    if (!m_burnDevice) return;
    
    QString host = m_serverHostEdit->text().trimmed();
    quint16 port = static_cast<quint16>(m_serverPortSpin->value());
    
    m_burnDevice->setServerInfo(host, port);
    appendLog(QString("服务器配置已更新: %1:%2").arg(host).arg(port), "INFO");
}

void BurnControlWidget::onBrowseProjectFileClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this,
        tr("选择项目文件"),
        "",
        tr("项目文件 (*.eapr *.json);;所有文件 (*.*)"));
    
    if (!filePath.isEmpty()) {
        m_projectPathEdit->setText(filePath);
        appendLog(QString("选择项目文件: %1").arg(filePath), "INFO");
    }
}

// 辅助函数实现
void BurnControlWidget::updateConnectionStatus()
{
    if (!m_burnDevice) {
        m_connectionStatusLabel->setText("长连接模式 - 未设置设备");
        m_connectionStatusLabel->setStyleSheet("color: gray; font-weight: bold;");
        m_isDeviceConnected = false;
        return;
    }
    
    // 根据实际设备状态更新显示
    auto status = m_burnDevice->getStatus();
    QString statusText;
    QString styleSheet;
    
    switch (status) {
        case Domain::IDevice::DeviceStatus::Ready:
            statusText = "长连接模式 - 已连接";
            styleSheet = "color: green; font-weight: bold;";
            m_isDeviceConnected = true;
            break;
        case Domain::IDevice::DeviceStatus::Connected:
            statusText = "长连接模式 - 已连接";
            styleSheet = "color: green; font-weight: bold;";
            m_isDeviceConnected = true;
            break;
        case Domain::IDevice::DeviceStatus::Initializing:
            statusText = "长连接模式 - 正在连接...";
            styleSheet = "color: orange; font-weight: bold;";
            m_isDeviceConnected = false;
            break;
        case Domain::IDevice::DeviceStatus::Disconnected:
            statusText = "长连接模式 - 未连接";
            styleSheet = "color: red; font-weight: bold;";
            m_isDeviceConnected = false;
            break;
        case Domain::IDevice::DeviceStatus::Error:
            statusText = "长连接模式 - 连接错误";
            styleSheet = "color: red; font-weight: bold;";
            m_isDeviceConnected = false;
            break;
        default:
            statusText = "长连接模式 - 未知状态";
            styleSheet = "color: gray; font-weight: bold;";
            m_isDeviceConnected = false;
    }
    
    m_connectionStatusLabel->setText(statusText);
    m_connectionStatusLabel->setStyleSheet(styleSheet);
}

void BurnControlWidget::updateDeviceStatus()
{
    if (!m_burnDevice) return;
    
    auto status = m_burnDevice->getStatus();
    QString statusText;
    QString statusLevel = "STATUS";
    
    switch (status) {
        case Domain::IDevice::DeviceStatus::Disconnected:
            statusText = "未连接";
            statusLevel = "WARNING";
            break;
        case Domain::IDevice::DeviceStatus::Connected:
            statusText = "已连接";
            statusLevel = "SUCCESS";
            break;
        case Domain::IDevice::DeviceStatus::Ready:
            statusText = "就绪";
            statusLevel = "SUCCESS";
            break;
        case Domain::IDevice::DeviceStatus::Busy:
            statusText = "忙碌";
            statusLevel = "INFO";
            break;
        case Domain::IDevice::DeviceStatus::Error:
            statusText = "错误 - 请检查服务器连接";
            statusLevel = "ERROR";
            break;
        case Domain::IDevice::DeviceStatus::Initializing:
            statusText = "正在初始化...";
            statusLevel = "INFO";
            break;
        default:
            statusText = "未知";
            statusLevel = "WARNING";
    }
    
    // 在日志中显示状态变化
    static QString lastStatus;
    if (statusText != lastStatus) {
        appendLog(QString("设备状态: %1").arg(statusText), statusLevel);
        lastStatus = statusText;
        
        // 更新连接状态显示
        if (m_connectionStatusLabel) {
            QString connectionText = QString("长连接模式 - %1").arg(statusText);
            QString styleSheet;
            
            switch (status) {
                case Domain::IDevice::DeviceStatus::Ready:
                case Domain::IDevice::DeviceStatus::Connected:
                    styleSheet = "color: green; font-weight: bold;";
                    break;
                case Domain::IDevice::DeviceStatus::Error:
                case Domain::IDevice::DeviceStatus::Disconnected:
                    styleSheet = "color: red; font-weight: bold;";
                    break;
                case Domain::IDevice::DeviceStatus::Initializing:
                    styleSheet = "color: orange; font-weight: bold;";
                    break;
                default:
                    styleSheet = "color: blue; font-weight: bold;";
            }
            
            m_connectionStatusLabel->setText(connectionText);
            m_connectionStatusLabel->setStyleSheet(styleSheet);
        }
    }
}

void BurnControlWidget::appendLog(const QString &message, const QString &level)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString colorCode;
    
    if (level == "ERROR") {
        colorCode = "color: red; font-weight: bold;";
    } else if (level == "WARNING") {
        colorCode = "color: orange; font-weight: bold;";
    } else if (level == "SUCCESS") {
        colorCode = "color: green; font-weight: bold;";
    } else if (level == "DEBUG") {
        colorCode = "color: gray;";
    } else {
        colorCode = "color: blue;";
    }
    
    QString formattedMsg = QString("<span style='%1'>[%2][%3] %4</span><br>")
                          .arg(colorCode)
                          .arg(timestamp)
                          .arg(level)
                          .arg(message);
    
    if (m_logTextEdit) {
        m_logTextEdit->moveCursor(QTextCursor::End);
        m_logTextEdit->insertHtml(formattedMsg);
        m_logTextEdit->ensureCursorVisible();
    }
    
    // 同时输出到调试控制台
    qDebug() << QString("[%1][%2] %3").arg(timestamp).arg(level).arg(message);
}

void BurnControlWidget::clearLog()
{
    if (m_logTextEdit) {
        m_logTextEdit->clear();
    }
    m_logBuffer.clear();
    appendLog("日志已清除", "INFO");
}

QString BurnControlWidget::formatJsonForDisplay(const QJsonValue &value)
{
    QJsonDocument doc;
    if (value.isObject()) {
        doc = QJsonDocument(value.toObject());
    } else if (value.isArray()) {
        doc = QJsonDocument(value.toArray());
    } else {
        return value.toVariant().toString();
    }
    
    return doc.toJson(QJsonDocument::Compact);
}

QJsonObject BurnControlWidget::buildDoCustomParams()
{
    QJsonObject params;
    
    // 基本参数
    params["strIP"] = m_strIPEdit->text().trimmed();
    params["nHopNum"] = m_nHopNumSpin->value();
    params["portID"] = m_portIDSpin->value();
    params["cmdFlag"] = m_cmdFlagSpin->value();
    params["cmdID"] = m_cmdIDSpin->value();
    params["bpuID"] = m_bpuIDSpin->value();
    
    // 解析Socket使能值
    QString sktEnStr = m_sktEnEdit->text().trimmed();
    bool ok;
    quint32 sktEn = sktEnStr.startsWith("0x") ? 
                   sktEnStr.mid(2).toUInt(&ok, 16) : 
                   sktEnStr.toUInt(&ok);
    if (!ok) {
        sktEn = 0x0000FFFF; // 默认值
    }
    params["sktEn"] = static_cast<double>(sktEn);
    
    // 解析JSON命令序列
    QString jsonText = m_docmdSeqJsonEdit->toPlainText().trimmed();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonText.toUtf8(), &parseError);
    
    if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
        params["docmdSeqJson"] = doc.object();
    } else {
        // 使用默认JSON
        QJsonObject defaultCmd;
        defaultCmd["CmdID"] = "1806";
        defaultCmd["CmdRun"] = "Read";
        QJsonArray sequences;
        QJsonObject seqItem;
        seqItem["ID"] = "806";
        seqItem["Name"] = "Read";
        sequences.append(seqItem);
        defaultCmd["CmdSequences"] = sequences;
        defaultCmd["CmdSequencesGroupCnt"] = 1;
        params["docmdSeqJson"] = defaultCmd;
        
        appendLog("JSON解析失败，使用默认命令序列", "WARNING");
    }
    
    return params;
}

// 新增的槽函数实现
void BurnControlWidget::onConnectButtonClicked()
{
    if (!m_burnDevice) {
        appendLog("设备未设置", "ERROR");
        return;
    }
    
    // 更新服务器配置
    QString host = m_serverHostEdit->text().trimmed();
    quint16 port = static_cast<quint16>(m_serverPortSpin->value());
    m_burnDevice->setServerInfo(host, port);
    
    appendLog(QString("尝试连接到 %1:%2").arg(host).arg(port), "INFO");
    
    // 请求连接
    m_burnDevice->requestConnection();
}

void BurnControlWidget::onDeviceConnected()
{
    appendLog("设备已连接", "SUCCESS");
    updateConnectionStatus();
    setControlsEnabled(true);
}

void BurnControlWidget::onDeviceDisconnected()
{
    appendLog("设备已断开连接", "WARNING");
    updateConnectionStatus();
    setControlsEnabled(false);
}

// Aprog路径管理相关槽函数实现
void BurnControlWidget::onBrowseAprogPathClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this,
        tr("选择Aprog.exe程序"),
        "",
        tr("可执行文件 (*.exe);;所有文件 (*.*)"));
    
    if (!filePath.isEmpty()) {
        m_aprogPathEdit->setText(filePath);
        appendLog(QString("选择Aprog程序: %1").arg(filePath), "INFO");
        
        // 自动更新设备配置
        if (m_burnDevice) {
            m_burnDevice->setAprogPath(filePath);
        }
    }
}

void BurnControlWidget::onAprogPathChanged()
{
    if (!m_burnDevice) return;
    
    QString path = m_aprogPathEdit->text().trimmed();
    m_burnDevice->setAprogPath(path);
    appendLog(QString("Aprog路径已更新: %1").arg(path), "INFO");
    appendLog("提示: Aprog路径已在运行时更新，如需永久保存请修改配置文件", "INFO");
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

// Aprog进程信号槽实现
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

void BurnControlWidget::onAprogOutputReceived(const QString &output)
{
    appendLog(QString("Aprog输出: %1").arg(output), "DEBUG");
}

// 更新Aprog状态显示
void BurnControlWidget::updateAprogStatus()
{
    if (!m_burnDevice || !m_aprogStatusLabel) return;
    
    QString statusText;
    QString styleSheet;
    
    if (m_burnDevice->isAprogRunning()) {
        statusText = "运行中";
        styleSheet = "color: green; font-weight: bold;";
    } else {
        statusText = "未启动";
        styleSheet = "color: gray; font-weight: bold;";
    }
    
    m_aprogStatusLabel->setText(statusText);
    m_aprogStatusLabel->setStyleSheet(styleSheet);
}

} // namespace Presentation 