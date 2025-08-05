#include "JsonRpcTestWidget.h"
#include <QApplication>
#include <QMessageBox>
#include <QDateTime>
#include <QJsonParseError>
#include <QJsonArray>
#include <QTreeWidgetItem>
#include <QScrollBar>
#include <QUuid>
#include <QFileDialog>
#include <QClipboard>
#include <QSplitter>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QRegularExpression>
#include <QBitArray>

JsonRpcTestWidget::JsonRpcTestWidget(QWidget *parent)
    : QWidget(parent)
    , m_client(nullptr)
    , m_statusTimer(nullptr)
    , m_isConnected(false)
    , m_requestCounter(0)
{
    // 初始化客户端
    m_client = new JsonRpcClient(this);
    
    // 连接客户端信号
    connect(m_client, &JsonRpcClient::connectionStateChanged,
            this, &JsonRpcTestWidget::onConnectionStateChanged);
    connect(m_client, &JsonRpcClient::notificationReceived,
            this, &JsonRpcTestWidget::onNotificationReceived);
    connect(m_client, &JsonRpcClient::errorOccurred,
            this, &JsonRpcTestWidget::onClientError);
    
    // 初始化状态定时器
    m_statusTimer = new QTimer(this);
    connect(m_statusTimer, &QTimer::timeout, this, &JsonRpcTestWidget::updateConnectionStatus);
    m_statusTimer->start(1000); // 每秒更新一次状态
    
    // 初始化方法配置
    initializeMethodConfigs();
    
    // 设置UI
    setupUI();
    
    // 初始状态
    enableMethodButtons(false);
    updateConnectionStatus();
    
    // 窗口设置
    setWindowTitle("JSON-RPC 客户端测试工具 v1.0");
    setMinimumSize(1200, 800);
    resize(1400, 900);
}

JsonRpcTestWidget::~JsonRpcTestWidget()
{
    if (m_client) {
        m_client->disconnectFromServer();
    }
}

void JsonRpcTestWidget::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // 创建主分割器
    auto* mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // 左侧：连接和方法
    auto* leftWidget = new QWidget;
    auto* leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setSpacing(10);
    
    setupConnectionGroup();
    setupMethodsGroup();
    
    leftLayout->addWidget(m_connectionGroup);
    leftLayout->addWidget(m_methodsGroup);
    leftLayout->addStretch();
    
    // 右侧：结果显示
    auto* rightWidget = new QWidget;
    auto* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setSpacing(10);
    
    setupResultsGroup();
    rightLayout->addWidget(m_resultsTabWidget);
    
    // 控制按钮区域
    auto* controlWidget = new QWidget;
    auto* controlLayout = new QHBoxLayout(controlWidget);
    
    m_clearLogsBtn = new QPushButton("清除日志");
    m_autoScrollBox = new QCheckBox("自动滚动");
    m_prettyJsonBox = new QCheckBox("格式化JSON");
    
    m_autoScrollBox->setChecked(true);
    m_prettyJsonBox->setChecked(true);
    
    controlLayout->addWidget(m_clearLogsBtn);
    controlLayout->addWidget(m_autoScrollBox);
    controlLayout->addWidget(m_prettyJsonBox);
    controlLayout->addStretch();
    
    rightLayout->addWidget(controlWidget);
    
    // 连接控制信号
    connect(m_clearLogsBtn, &QPushButton::clicked, this, &JsonRpcTestWidget::onClearLogsClicked);
    connect(m_autoScrollBox, &QCheckBox::toggled, this, &JsonRpcTestWidget::onAutoScrollToggled);
    
    // 设置分割器
    mainSplitter->addWidget(leftWidget);
    mainSplitter->addWidget(rightWidget);
    mainSplitter->setSizes({400, 800});
    
    mainLayout->addWidget(mainSplitter);
}

void JsonRpcTestWidget::setupConnectionGroup()
{
    m_connectionGroup = new QGroupBox("连接设置");
    auto* layout = new QGridLayout(m_connectionGroup);
    
    // 服务器地址
    layout->addWidget(new QLabel("服务器:"), 0, 0);
    m_hostEdit = new QLineEdit("127.0.0.1");
    layout->addWidget(m_hostEdit, 0, 1);
    
    // 端口
    layout->addWidget(new QLabel("端口:"), 1, 0);
    m_portSpinBox = new QSpinBox;
    m_portSpinBox->setRange(1, 65535);
    m_portSpinBox->setValue(8080);
    layout->addWidget(m_portSpinBox, 1, 1);
    
    // 连接按钮
    auto* buttonLayout = new QHBoxLayout;
    m_connectBtn = new QPushButton("连接");
    m_disconnectBtn = new QPushButton("断开");
    m_disconnectBtn->setEnabled(false);
    
    buttonLayout->addWidget(m_connectBtn);
    buttonLayout->addWidget(m_disconnectBtn);
    layout->addLayout(buttonLayout, 2, 0, 1, 2);
    
    // 连接状态
    m_statusLabel = new QLabel("未连接");
    m_statusProgress = new QProgressBar;
    m_statusProgress->setVisible(false);
    layout->addWidget(m_statusLabel, 3, 0, 1, 2);
    layout->addWidget(m_statusProgress, 4, 0, 1, 2);
    
    // 自动重连设置
    m_autoReconnectBox = new QCheckBox("自动重连");
    m_autoReconnectBox->setChecked(true);
    layout->addWidget(m_autoReconnectBox, 5, 0, 1, 2);
    
    layout->addWidget(new QLabel("重连间隔(秒):"), 6, 0);
    m_reconnectIntervalSpinBox = new QSpinBox;
    m_reconnectIntervalSpinBox->setRange(1, 60);
    m_reconnectIntervalSpinBox->setValue(5);
    layout->addWidget(m_reconnectIntervalSpinBox, 6, 1);
    
    // 连接信号
    connect(m_connectBtn, &QPushButton::clicked, this, &JsonRpcTestWidget::onConnectClicked);
    connect(m_disconnectBtn, &QPushButton::clicked, this, &JsonRpcTestWidget::onDisconnectClicked);
    connect(m_reconnectIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this](int value) { m_client->setReconnectInterval(value * 1000); });
}

void JsonRpcTestWidget::setupMethodsGroup()
{
    m_methodsGroup = new QGroupBox("RPC方法测试");
    auto* layout = new QVBoxLayout(m_methodsGroup);
    
    // 方法选择下拉框
    m_methodComboBox = new QComboBox;
    layout->addWidget(new QLabel("选择方法:"));
    layout->addWidget(m_methodComboBox);
    
    // 参数表单容器
    m_parameterFormWidget = new QWidget;
    m_parameterFormLayout = new QFormLayout(m_parameterFormWidget);
    
    auto* scrollArea = new QScrollArea;
    scrollArea->setWidget(m_parameterFormWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setMaximumHeight(300);
    layout->addWidget(scrollArea);
    
    // 添加方法到下拉框并创建按钮
    for (const auto& config : m_methodConfigs) {
        m_methodComboBox->addItem(config.description, config.name);
        
        auto* button = new QPushButton(config.description);
        button->setToolTip(QString("调用 %1 方法").arg(config.name));
        layout->addWidget(button);
        
        m_methodButtons[config.name] = button;
        
        // 连接按钮信号
        if (config.name == "LoadProject") {
            connect(button, &QPushButton::clicked, this, &JsonRpcTestWidget::onLoadProjectClicked);
        } else if (config.name == "SiteScanAndConnect") {
            connect(button, &QPushButton::clicked, this, &JsonRpcTestWidget::onSiteScanAndConnectClicked);
        } else if (config.name == "GetProjectInfo") {
            connect(button, &QPushButton::clicked, this, &JsonRpcTestWidget::onGetProjectInfoClicked);
        } else if (config.name == "GetProjectInfoExt") {
            connect(button, &QPushButton::clicked, this, &JsonRpcTestWidget::onGetProjectInfoExtClicked);
        } else if (config.name == "GetSKTInfo") {
            connect(button, &QPushButton::clicked, this, &JsonRpcTestWidget::onGetSKTInfoClicked);
        } else if (config.name == "SendUUID") {
            connect(button, &QPushButton::clicked, this, &JsonRpcTestWidget::onSendUUIDClicked);
        } else if (config.name == "DoJob") {
            connect(button, &QPushButton::clicked, this, &JsonRpcTestWidget::onDoJobClicked);
        } else if (config.name == "DoCustom") {
            connect(button, &QPushButton::clicked, this, &JsonRpcTestWidget::onDoCustomClicked);
        }
    }
    
    // 连接下拉框变化信号
    connect(m_methodComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int index) {
                QString methodName = m_methodComboBox->itemData(index).toString();
                updateMethodForm(methodName);
            });
    
    // 初始化第一个方法的表单
    if (!m_methodConfigs.isEmpty()) {
        updateMethodForm(m_methodConfigs.first().name);
    }
}

void JsonRpcTestWidget::setupResultsGroup()
{
    m_resultsTabWidget = new QTabWidget;
    
    // 响应结果标签页
    m_responseTextEdit = new QTextEdit;
    m_responseTextEdit->setReadOnly(true);
    m_responseTextEdit->setFont(QFont("Consolas", 9));
    m_resultsTabWidget->addTab(m_responseTextEdit, "响应结果");
    
    // 结构化显示标签页
    m_responseTreeWidget = new QTreeWidget;
    m_responseTreeWidget->setHeaderLabels({"键", "值", "类型"});
    m_responseTreeWidget->header()->setStretchLastSection(false);
    m_responseTreeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_responseTreeWidget->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_responseTreeWidget->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_resultsTabWidget->addTab(m_responseTreeWidget, "结构化视图");
    
    // 通知消息标签页
    m_notificationTextEdit = new QTextEdit;
    m_notificationTextEdit->setReadOnly(true);
    m_notificationTextEdit->setFont(QFont("Consolas", 9));
    m_resultsTabWidget->addTab(m_notificationTextEdit, "通知消息");
    
    // 日志标签页
    m_logTextEdit = new QTextEdit;
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setFont(QFont("Consolas", 9));
    m_resultsTabWidget->addTab(m_logTextEdit, "详细日志");
    
    // 站点作业管理标签页
    setupSiteJobGroup();
    m_resultsTabWidget->addTab(m_siteJobGroup, "站点作业管理");
    
    // 自定义JSON编辑标签页
    setupCustomJsonGroup();
    m_resultsTabWidget->addTab(m_customJsonGroup, "自定义JSON编辑");
}

void JsonRpcTestWidget::initializeMethodConfigs()
{
    // LoadProject
    MethodConfig loadProject;
    loadProject.name = "LoadProject";
    loadProject.description = "加载项目";
    loadProject.parameters = QStringList({"path", "taskFileName"});
    loadProject.parameterTypes["path"] = "string";
    loadProject.parameterTypes["taskFileName"] = "string";
    loadProject.defaultValues["path"] = "C:/projects/test_project";
    loadProject.defaultValues["taskFileName"] = "task.json";
    m_methodConfigs.append(loadProject);
    
    // SiteScanAndConnect
    MethodConfig siteScan;
    siteScan.name = "SiteScanAndConnect";
    siteScan.description = "站点扫描连接";
    siteScan.parameters = QStringList();
    m_methodConfigs.append(siteScan);
    
    // GetProjectInfo
    MethodConfig getProjectInfo;
    getProjectInfo.name = "GetProjectInfo";
    getProjectInfo.description = "获取项目信息";
    getProjectInfo.parameters = QStringList();
    m_methodConfigs.append(getProjectInfo);
    
    // GetProjectInfoExt
    MethodConfig getProjectInfoExt;
    getProjectInfoExt.name = "GetProjectInfoExt";
    getProjectInfoExt.description = "获取项目扩展信息";
    getProjectInfoExt.parameters = QStringList();
    m_methodConfigs.append(getProjectInfoExt);
    
    // GetSKTInfo
    MethodConfig getSKTInfo;
    getSKTInfo.name = "GetSKTInfo";
    getSKTInfo.description = "获取SKT信息";
    getSKTInfo.parameters = QStringList();
    m_methodConfigs.append(getSKTInfo);
    
    // SendUUID
    MethodConfig sendUUID;
    sendUUID.name = "SendUUID";
    sendUUID.description = "发送UUID";
    sendUUID.parameters = QStringList({"uuid", "strIP", "nHopNum", "sktEnable"});
    sendUUID.parameterTypes["uuid"] = "string";
    sendUUID.parameterTypes["strIP"] = "string";
    sendUUID.parameterTypes["nHopNum"] = "number";
    sendUUID.parameterTypes["sktEnable"] = "number";
    sendUUID.defaultValues["uuid"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    sendUUID.defaultValues["strIP"] = "192.168.1.100";
    sendUUID.defaultValues["nHopNum"] = 1;
    sendUUID.defaultValues["sktEnable"] = 255;
    m_methodConfigs.append(sendUUID);
    
    // DoJob
    MethodConfig doJob;
    doJob.name = "DoJob";
    doJob.description = "执行作业";
    doJob.parameters = QStringList({"strIP", "sktEn", "docmdSeqJson"});
    doJob.parameterTypes["strIP"] = "string";
    doJob.parameterTypes["sktEn"] = "number";
    doJob.parameterTypes["docmdSeqJson"] = "json";
    doJob.defaultValues["strIP"] = "192.168.1.100";
    doJob.defaultValues["sktEn"] = 255;
    doJob.defaultValues["docmdSeqJson"] = "{\"operation\":\"program\",\"data\":\"test_data\"}";
    m_methodConfigs.append(doJob);
    
    // DoCustom
    MethodConfig doCustom;
    doCustom.name = "DoCustom";
    doCustom.description = "自定义命令";
    doCustom.parameters = QStringList({"customParams"});
    doCustom.parameterTypes["customParams"] = "json";
    doCustom.defaultValues["customParams"] = "{\"command\":\"test\",\"value\":123}";
    m_methodConfigs.append(doCustom);
}

// === 连接管理实现 ===

void JsonRpcTestWidget::onConnectClicked()
{
    QString host = m_hostEdit->text().trimmed();
    if (host.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入服务器地址");
        return;
    }
    
    int port = m_portSpinBox->value();
    bool autoReconnect = m_autoReconnectBox->isChecked();
    
    addLogEntry("INFO", QString("正在连接到服务器 %1:%2").arg(host).arg(port));
    
    m_client->setReconnectInterval(m_reconnectIntervalSpinBox->value() * 1000);
    m_client->connectToServer(host, port, autoReconnect);
    
    m_connectBtn->setEnabled(false);
    m_statusProgress->setVisible(true);
    m_statusProgress->setRange(0, 0); // 无限进度条
}

void JsonRpcTestWidget::onDisconnectClicked()
{
    addLogEntry("INFO", "断开服务器连接");
    m_client->disconnectFromServer();
}

void JsonRpcTestWidget::onConnectionStateChanged(JsonRpcClient::ConnectionState state)
{
    QString stateText;
    QString styleSheet;
    
    switch (state) {
    case JsonRpcClient::Disconnected:
        stateText = "未连接";
        styleSheet = "color: red;";
        m_isConnected = false;
        m_connectBtn->setEnabled(true);
        m_disconnectBtn->setEnabled(false);
        m_statusProgress->setVisible(false);
        break;
        
    case JsonRpcClient::Connecting:
        stateText = "连接中...";
        styleSheet = "color: orange;";
        m_isConnected = false;
        m_connectBtn->setEnabled(false);
        m_disconnectBtn->setEnabled(false);
        m_statusProgress->setVisible(true);
        break;
        
    case JsonRpcClient::Connected:
        stateText = "已连接";
        styleSheet = "color: green; font-weight: bold;";
        m_isConnected = true;
        m_connectBtn->setEnabled(false);
        m_disconnectBtn->setEnabled(true);
        m_statusProgress->setVisible(false);
        addLogEntry("SUCCESS", "成功连接到服务器");
        break;
        
    case JsonRpcClient::Reconnecting:
        stateText = "重连中...";
        styleSheet = "color: blue;";
        m_isConnected = false;
        m_connectBtn->setEnabled(false);
        m_disconnectBtn->setEnabled(true);
        m_statusProgress->setVisible(true);
        break;
    }
    
    m_statusLabel->setText(stateText);
    m_statusLabel->setStyleSheet(styleSheet);
    enableMethodButtons(m_isConnected);
}

void JsonRpcTestWidget::onClientError(const QString& error)
{
    addLogEntry("ERROR", "客户端错误: " + error);
    QMessageBox::warning(this, "客户端错误", error);
}

// === RPC方法实现 ===

void JsonRpcTestWidget::onLoadProjectClicked()
{
    QJsonObject params = getParametersFromForm("LoadProject");
    callRpcMethod("LoadProject", params, "加载项目");
}

void JsonRpcTestWidget::onSiteScanAndConnectClicked()
{
    callRpcMethod("SiteScanAndConnect", QJsonObject(), "站点扫描连接");
}

void JsonRpcTestWidget::onGetProjectInfoClicked()
{
    callRpcMethod("GetProjectInfo", QJsonObject(), "获取项目信息");
}

void JsonRpcTestWidget::onGetProjectInfoExtClicked()
{
    callRpcMethod("GetProjectInfoExt", QJsonObject(), "获取项目扩展信息");
}

void JsonRpcTestWidget::onGetSKTInfoClicked()
{
    callRpcMethod("GetSKTInfo", QJsonObject(), "获取SKT信息");
}

void JsonRpcTestWidget::onSendUUIDClicked()
{
    QJsonObject params = getParametersFromForm("SendUUID");
    callRpcMethod("SendUUID", params, "发送UUID");
}

void JsonRpcTestWidget::onDoJobClicked()
{
    QJsonObject params = getParametersFromForm("DoJob");
    callRpcMethod("DoJob", params, "执行作业");
}

void JsonRpcTestWidget::onDoCustomClicked()
{
    QJsonObject params = getParametersFromForm("DoCustom");
    callRpcMethod("DoCustom", params, "自定义命令");
}

// === 工具方法实现 ===

void JsonRpcTestWidget::callRpcMethod(const QString& methodName, const QJsonObject& params, const QString& description)
{
    if (!m_isConnected) {
        QMessageBox::warning(this, "错误", "请先连接到服务器");
        return;
    }
    
    m_requestCounter++;
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString requestInfo = QString("[请求 #%1] %2").arg(m_requestCounter).arg(description);
    
    // 构造完整的JSON-RPC请求数据包（模拟JsonRpcClient中的逻辑）
    QJsonObject request;
    request["jsonrpc"] = "2.0";
    request["method"] = methodName;
    request["params"] = params;
    request["id"] = QString::number(m_requestCounter);
    
    QJsonDocument requestDoc(request);
    QByteArray jsonData = requestDoc.toJson(QJsonDocument::Compact);
    
    // 计算数据包大小（32字节头部 + JSON载荷）
    const int HEADER_LENGTH = 32;
    int totalSize = HEADER_LENGTH + jsonData.size();
    
    // 同时在控制台打印请求日志（与JsonRpcClient保持一致）
    qDebug().noquote() << QString("[%1] REQUEST: %2").arg(timestamp).arg(requestInfo);
    if (!params.isEmpty()) {
        QJsonDocument paramDoc(params);
        qDebug().noquote() << QString("请求参数: %1").arg(QString::fromUtf8(paramDoc.toJson(QJsonDocument::Compact)));
    }
    qDebug().noquote() << QString("发送数据包: %1").arg(QString::fromUtf8(jsonData));
    qDebug().noquote() << QString("协议头部: Magic=0x4150524F, Version=1, PayloadLength=%1, TotalSize=%2")
                          .arg(jsonData.size()).arg(totalSize);
    qDebug().noquote() << QString("数据包已发送: 总大小=%1字节").arg(totalSize);
    qDebug().noquote() << QString("-").repeated(80);
    
    // 构造详细的日志信息用于GUI显示
    QJsonObject detailData = params;
    detailData["_jsonrpc_packet"] = QString::fromUtf8(jsonData);
    detailData["_protocol_info"] = QString("Magic=0x4150524F, Version=1, PayloadLength=%1, TotalSize=%2")
                                   .arg(jsonData.size()).arg(totalSize);
    detailData["_packet_size"] = totalSize;
    
    addLogEntry("REQUEST", requestInfo, detailData);
    
    // 创建回调函数
    auto callback = [this, methodName, requestInfo](bool success, const QJsonObject& result, const QString& error) {
        onRpcResponse(methodName, success, result, error);
    };
    
    // 调用相应的RPC方法
    if (methodName == "LoadProject") {
        m_client->loadProject(params["path"].toString(), params["taskFileName"].toString(), callback);
    } else if (methodName == "SiteScanAndConnect") {
        m_client->siteScanAndConnect(callback);
    } else if (methodName == "GetProjectInfo") {
        m_client->getProjectInfo(callback);
    } else if (methodName == "GetProjectInfoExt") {
        m_client->getProjectInfoExt(callback);
    } else if (methodName == "GetSKTInfo") {
        m_client->getSKTInfo(callback);
    } else if (methodName == "SendUUID") {
        m_client->sendUUID(params["uuid"].toString(), params["strIP"].toString(),
                          params["nHopNum"].toInt(), params["sktEnable"].toInt(), callback);
    } else if (methodName == "DoJob") {
        QJsonObject cmdSeq;
        if (params.contains("docmdSeqJson")) {
            QString jsonStr = params["docmdSeqJson"].toString();
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);
            if (error.error == QJsonParseError::NoError) {
                cmdSeq = doc.object();
            }
        }
        m_client->doJob(params["strIP"].toString(), params["sktEn"].toInt(), cmdSeq, callback);
    } else if (methodName == "DoCustom") {
        QJsonObject customParams;
        if (params.contains("customParams")) {
            QString jsonStr = params["customParams"].toString();
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);
            if (error.error == QJsonParseError::NoError) {
                customParams = doc.object();
            }
        }
        m_client->doCustom(customParams, callback);
    }
}

QJsonObject JsonRpcTestWidget::getParametersFromForm(const QString& method)
{
    QJsonObject params;
    
    // 查找方法配置
    const MethodConfig* config = nullptr;
    for (const auto& cfg : m_methodConfigs) {
        if (cfg.name == method) {
            config = &cfg;
            break;
        }
    }
    
    if (!config) return params;
    
    // 从表单获取参数值
    for (int i = 0; i < m_parameterFormLayout->rowCount(); ++i) {
        QLayoutItem* labelItem = m_parameterFormLayout->itemAt(i, QFormLayout::LabelRole);
        QLayoutItem* fieldItem = m_parameterFormLayout->itemAt(i, QFormLayout::FieldRole);
        
        if (!labelItem || !fieldItem) continue;
        
        QLabel* label = qobject_cast<QLabel*>(labelItem->widget());
        if (!label) continue;
        
        QString paramName = label->text().replace(":", "");
        QWidget* fieldWidget = fieldItem->widget();
        
        if (auto* lineEdit = qobject_cast<QLineEdit*>(fieldWidget)) {
            QString value = lineEdit->text();
            QString type = config->parameterTypes.value(paramName, "string");
            
            if (type == "number") {
                params[paramName] = value.toInt();
            } else {
                params[paramName] = value;
            }
        } else if (auto* spinBox = qobject_cast<QSpinBox*>(fieldWidget)) {
            params[paramName] = spinBox->value();
        } else if (auto* textEdit = qobject_cast<QTextEdit*>(fieldWidget)) {
            params[paramName] = textEdit->toPlainText();
        }
    }
    
    return params;
}

void JsonRpcTestWidget::updateMethodForm(const QString& method)
{
    // 清除现有表单
    QLayoutItem* item;
    while ((item = m_parameterFormLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    // 查找方法配置
    const MethodConfig* config = nullptr;
    for (const auto& cfg : m_methodConfigs) {
        if (cfg.name == method) {
            config = &cfg;
            break;
        }
    }
    
    if (!config || config->parameters.isEmpty()) {
        m_parameterFormLayout->addRow(new QLabel("此方法无需参数"));
        return;
    }
    
    // 创建参数输入控件
    for (const QString& param : config->parameters) {
        QString type = config->parameterTypes.value(param, "string");
        QVariant defaultValue = config->defaultValues.value(param);
        
        QWidget* inputWidget = nullptr;
        
        if (type == "number") {
            auto* spinBox = new QSpinBox;
            spinBox->setRange(-999999, 999999);
            spinBox->setValue(defaultValue.toInt());
            inputWidget = spinBox;
        } else if (type == "json") {
            auto* textEdit = new QTextEdit;
            textEdit->setMaximumHeight(80);
            textEdit->setPlainText(defaultValue.toString());
            inputWidget = textEdit;
        } else {
            auto* lineEdit = new QLineEdit;
            lineEdit->setText(defaultValue.toString());
            inputWidget = lineEdit;
        }
        
        m_parameterFormLayout->addRow(param + ":", inputWidget);
    }
}

void JsonRpcTestWidget::onRpcResponse(const QString& method, bool success, const QJsonObject& result, const QString& error)
{
    if (success) {
        addLogEntry("RESPONSE", QString("方法 %1 调用成功").arg(method), result);
        displayJsonData(QString("%1 - 成功响应").arg(method), result);
    } else {
        addLogEntry("ERROR", QString("方法 %1 调用失败: %2").arg(method).arg(error));
        QMessageBox::warning(this, "RPC调用失败", QString("方法: %1\n错误: %2").arg(method).arg(error));
    }
}

void JsonRpcTestWidget::onNotificationReceived(const QString& method, const QJsonObject& params)
{
    addLogEntry("NOTIFICATION", QString("收到服务器通知: %1").arg(method), params);
    
    QString notificationText = QString("[%1] 通知: %2\n%3\n\n")
                              .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                              .arg(method)
                              .arg(formatJsonString(params));
    
    m_notificationTextEdit->append(notificationText);
    
    if (m_autoScrollBox->isChecked()) {
        m_notificationTextEdit->moveCursor(QTextCursor::End);
    }
    
    // 解析站点相关通知
    parseSiteNotification(method, params);
    
    // 处理自定义命令结果通知
    if (method == "setDoCustomResult") {
        QString resultText = params["result"].toBool() ? "成功" : "失败";
        QString logMessage = QString("自定义命令执行%1").arg(resultText);
        addLogEntry(params["result"].toBool() ? "SUCCESS" : "ERROR", logMessage, params);
    }
}

void JsonRpcTestWidget::addLogEntry(const QString& type, const QString& message, const QJsonObject& data)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString logEntry;
    
    // 根据类型设置不同的日志格式
    if (type == "REQUEST") {
        logEntry = QString("[%1] REQUEST: %2").arg(timestamp).arg(message);
    } else {
        logEntry = QString("[%1] %2: %3").arg(timestamp).arg(type).arg(message);
    }
    
    if (!data.isEmpty()) {
        // 检查是否包含特殊的数据包信息字段
        if (data.contains("_jsonrpc_packet")) {
            // 这是一个请求数据包，显示详细信息
            QString packetInfo = data["_jsonrpc_packet"].toString();
            QString protocolInfo = data["_protocol_info"].toString();
            int packetSize = data["_packet_size"].toInt();
            
            logEntry += "\n发送数据包: " + packetInfo;
            logEntry += "\n协议头部: " + protocolInfo;
            logEntry += QString("\n数据包已发送: 总大小=%1字节").arg(packetSize);
            
            // 如果还有其他参数（去除特殊字段）
            QJsonObject normalParams = data;
            normalParams.remove("_jsonrpc_packet");
            normalParams.remove("_protocol_info");
            normalParams.remove("_packet_size");
            
            if (!normalParams.isEmpty()) {
                logEntry += "\n请求参数: " + formatJsonString(normalParams);
            }
        } else {
            // 普通数据显示
            logEntry += "\n参数/数据: " + formatJsonString(data);
        }
    }
    
    logEntry += "\n" + QString("-").repeated(80) + "\n";
    
    m_logTextEdit->append(logEntry);
    
    if (m_autoScrollBox->isChecked()) {
        m_logTextEdit->moveCursor(QTextCursor::End);
    }
}

void JsonRpcTestWidget::displayJsonData(const QString& title, const QJsonObject& data)
{
    // 更新文本显示
    QString formattedJson = m_prettyJsonBox->isChecked() ? 
                           QJsonDocument(data).toJson(QJsonDocument::Indented) :
                           QJsonDocument(data).toJson(QJsonDocument::Compact);
    
    QString displayText = QString("=== %1 ===\n时间: %2\n\n%3\n\n")
                         .arg(title)
                         .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                         .arg(formattedJson);
    
    m_responseTextEdit->append(displayText);
    
    // 更新树形显示
    auto* rootItem = new QTreeWidgetItem(m_responseTreeWidget);
    rootItem->setText(0, title);
    rootItem->setText(1, "");
    rootItem->setText(2, "Object");
    
    addJsonToTree(rootItem, data);
    m_responseTreeWidget->expandAll();
    
    if (m_autoScrollBox->isChecked()) {
        m_responseTextEdit->moveCursor(QTextCursor::End);
        m_responseTreeWidget->scrollToBottom();
    }
}

void JsonRpcTestWidget::addJsonToTree(QTreeWidgetItem* parent, const QJsonObject& obj)
{
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        auto* item = new QTreeWidgetItem(parent);
        item->setText(0, it.key());
        
        QJsonValue value = it.value();
        if (value.isObject()) {
            item->setText(1, "");
            item->setText(2, "Object");
            addJsonToTree(item, value.toObject());
        } else if (value.isArray()) {
            item->setText(1, QString("[%1 items]").arg(value.toArray().size()));
            item->setText(2, "Array");
            addJsonArrayToTree(item, value.toArray());
        } else {
            item->setText(1, value.toVariant().toString());
            item->setText(2, value.type() == QJsonValue::String ? "String" :
                              value.type() == QJsonValue::Double ? "Number" :
                              value.type() == QJsonValue::Bool ? "Boolean" : "Other");
        }
    }
}

void JsonRpcTestWidget::addJsonArrayToTree(QTreeWidgetItem* parent, const QJsonArray& array)
{
    for (int i = 0; i < array.size(); ++i) {
        auto* item = new QTreeWidgetItem(parent);
        item->setText(0, QString("[%1]").arg(i));
        
        QJsonValue value = array[i];
        if (value.isObject()) {
            item->setText(1, "");
            item->setText(2, "Object");
            addJsonToTree(item, value.toObject());
        } else if (value.isArray()) {
            item->setText(1, QString("[%1 items]").arg(value.toArray().size()));
            item->setText(2, "Array");
            addJsonArrayToTree(item, value.toArray());
        } else {
            item->setText(1, value.toVariant().toString());
            item->setText(2, value.type() == QJsonValue::String ? "String" :
                              value.type() == QJsonValue::Double ? "Number" :
                              value.type() == QJsonValue::Bool ? "Boolean" : "Other");
        }
    }
}

QString JsonRpcTestWidget::formatJsonString(const QJsonObject& json)
{
    return m_prettyJsonBox->isChecked() ? 
           QJsonDocument(json).toJson(QJsonDocument::Indented) :
           QJsonDocument(json).toJson(QJsonDocument::Compact);
}

void JsonRpcTestWidget::enableMethodButtons(bool enabled)
{
    for (auto* button : m_methodButtons) {
        button->setEnabled(enabled);
    }
    m_methodComboBox->setEnabled(enabled);
    
    // 同时更新站点作业管理界面的状态
    updateExecuteButtonState();
}

void JsonRpcTestWidget::onClearLogsClicked()
{
    m_responseTextEdit->clear();
    m_notificationTextEdit->clear();
    m_logTextEdit->clear();
    m_responseTreeWidget->clear();
    m_requestCounter = 0;
    
    addLogEntry("INFO", "日志已清除");
}

void JsonRpcTestWidget::onAutoScrollToggled(bool enabled)
{
    Q_UNUSED(enabled)
    // 自动滚动状态已更新，在后续的日志追加中会使用
}

void JsonRpcTestWidget::updateConnectionStatus()
{
    // 定期更新连接状态（如果需要的话）
}

// === 站点作业管理界面设置 ===

void JsonRpcTestWidget::setupSiteJobGroup()
{
    m_siteJobGroup = new QGroupBox("站点作业管理");
    auto* layout = new QVBoxLayout(m_siteJobGroup);
    
    // 站点选择
    layout->addWidget(new QLabel("选择站点:"));
    m_siteComboBox = new QComboBox;
    m_siteComboBox->setEnabled(false); // 初始禁用，需要加载项目后才能使用
    layout->addWidget(m_siteComboBox);
    
    // 站点信息显示
    m_siteInfoLabel = new QLabel("请先进行站点扫描并加载项目");
    m_siteInfoLabel->setWordWrap(true);
    m_siteInfoLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 5px; border: 1px solid #ccc; }");
    layout->addWidget(m_siteInfoLabel);
    
    // 座子选择标题
    layout->addWidget(new QLabel("座子选择 (BPU使能):"));
    
    // 座子网格
    m_socketWidget = new QWidget;
    m_socketLayout = new QGridLayout(m_socketWidget);
    m_socketLayout->setSpacing(2);
    
    // 创建16个座子复选框 (4x4网格)
    for (int i = 0; i < 16; ++i) {
        m_socketCheckBoxes[i] = new QCheckBox(QString("S%1").arg(i));
        m_socketCheckBoxes[i]->setEnabled(false);
        
        // 连接信号
        connect(m_socketCheckBoxes[i], &QCheckBox::toggled, [this, i](bool checked) {
            onSocketToggled(i, checked);
        });
        
        // 按4x4排列
        int row = i / 4;
        int col = i % 4;
        m_socketLayout->addWidget(m_socketCheckBoxes[i], row, col);
    }
    
    layout->addWidget(m_socketWidget);
    
    // 选中座子显示
    m_selectedSocketsLabel = new QLabel("未选择座子");
    m_selectedSocketsLabel->setStyleSheet("QLabel { color: blue; font-weight: bold; }");
    layout->addWidget(m_selectedSocketsLabel);
    
    // 命令序列选择
    layout->addWidget(new QLabel("选择命令序列:"));
    m_cmdSequenceComboBox = new QComboBox;
    m_cmdSequenceComboBox->setEnabled(false);
    layout->addWidget(m_cmdSequenceComboBox);
    
    // 执行作业按钮
    m_executeJobBtn = new QPushButton("执行作业");
    m_executeJobBtn->setEnabled(false);
    m_executeJobBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 8px; }");
    layout->addWidget(m_executeJobBtn);
    
    // 连接信号
    connect(m_siteComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &JsonRpcTestWidget::onSiteSelectionChanged);
    connect(m_cmdSequenceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &JsonRpcTestWidget::onCmdSequenceSelectionChanged);
    connect(m_executeJobBtn, &QPushButton::clicked, this, &JsonRpcTestWidget::onExecuteJobClicked);
}

// === 自定义JSON编辑界面设置 ===

void JsonRpcTestWidget::setupCustomJsonGroup()
{
    m_customJsonGroup = new QGroupBox("自定义JSON编辑与发送");
    auto* layout = new QVBoxLayout(m_customJsonGroup);
    
    // 说明标签
    auto* infoLabel = new QLabel("编辑自定义JSON数据，通过DoCustom命令发送。请先在\"站点作业管理\"标签页中选择站点和座子：");
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("QLabel { color: #2196F3; font-weight: bold; padding: 8px; background-color: #E3F2FD; border-radius: 4px; }");
    layout->addWidget(infoLabel);
    
    // JSON编辑器
    m_customJsonEdit = new QTextEdit;
    m_customJsonEdit->setMaximumHeight(150);
    m_customJsonEdit->setPlaceholderText(
        "请输入JSON数据，例如：\n"
        "{\n"
        "  \"command\": \"test\",\n" 
        "  \"data\": [1, 2, 3],\n"
        "  \"options\": {\n"
        "    \"verbose\": true\n"
        "  }\n"
        "}"
    );
    m_customJsonEdit->setFont(QFont("Consolas", 9));
    layout->addWidget(m_customJsonEdit);
    
    // 状态显示
    m_customStatusLabel = new QLabel("请先在\"站点作业管理\"标签页中选择站点和座子");
    m_customStatusLabel->setWordWrap(true);
    m_customStatusLabel->setStyleSheet("QLabel { color: #666; font-style: italic; padding: 5px; }");
    layout->addWidget(m_customStatusLabel);
    
    // 按钮行
    auto* buttonLayout = new QHBoxLayout;
    
    // 格式化按钮
    m_formatJsonBtn = new QPushButton("格式化JSON");
    m_formatJsonBtn->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 5px 10px; }");
    buttonLayout->addWidget(m_formatJsonBtn);
    
    // 清空按钮
    m_clearJsonBtn = new QPushButton("清空");
    m_clearJsonBtn->setStyleSheet("QPushButton { background-color: #FF9800; color: white; padding: 5px 10px; }");
    buttonLayout->addWidget(m_clearJsonBtn);
    
    buttonLayout->addStretch();
    
    // 执行按钮
    m_executeCustomBtn = new QPushButton("执行自定义命令");
    m_executeCustomBtn->setEnabled(false);
    m_executeCustomBtn->setStyleSheet("QPushButton { background-color: #9C27B0; color: white; font-weight: bold; padding: 8px 15px; }");
    buttonLayout->addWidget(m_executeCustomBtn);
    
    layout->addLayout(buttonLayout);
    
    // 连接信号
    connect(m_customJsonEdit, &QTextEdit::textChanged, this, &JsonRpcTestWidget::onCustomJsonTextChanged);
    connect(m_formatJsonBtn, &QPushButton::clicked, this, &JsonRpcTestWidget::onFormatJsonClicked);
    connect(m_clearJsonBtn, &QPushButton::clicked, this, &JsonRpcTestWidget::onClearJsonClicked);
    connect(m_executeCustomBtn, &QPushButton::clicked, this, &JsonRpcTestWidget::onExecuteCustomClicked);
}

// === 站点数据解析和管理 ===

void JsonRpcTestWidget::parseSiteNotification(const QString& method, const QJsonObject& params)
{
    if (method == "setSiteReuslt") {
        // 解析站点扫描结果
        SiteInfo siteInfo;
        siteInfo.alias = params["SiteAlias"].toString();
        siteInfo.macAddr = params["MacAddr"].toString();
        siteInfo.firmwareVersion = params["FirmwareVersion"].toString();
        siteInfo.siteDetails = params;
        
        // 从UDPPort获取IP（这里可能需要根据实际情况调整）
        QString udpPort = params["UDPPort"].toString();
        // 暂时使用占位符IP，实际IP会在其他通知中获取
        siteInfo.ip = "待确定";
        
        // 创建或更新站点数据
        SiteData siteData;
        siteData.siteInfo = siteInfo;
        
        // 暂时以alias作为key，实际应该用IP
        m_siteDataMap[siteInfo.alias] = siteData;
        
        qDebug() << "解析站点信息:" << siteInfo.alias << siteInfo.firmwareVersion;
        
    } else if (method == "setLoadProjectResult") {
        // 解析项目加载结果
        if (params["result"].toBool()) {
            QJsonObject data = params["data"].toObject();
            QString ip = data["strIp"].toString();
            
            QJsonObject proInfo = params["proInfo"].toObject();
            
            ProjectInfo projectInfo;
            projectInfo.projectUrl = proInfo["pro_url"].toString();
            projectInfo.cmdSequenceArray = proInfo["doCmdSequenceArray"].toArray();
            projectInfo.chipData = proInfo["pro_chipdata"].toObject();
            
            // 更新或创建对应IP的站点数据
            if (m_siteDataMap.contains(ip)) {
                m_siteDataMap[ip].projectInfo = projectInfo;
                m_siteDataMap[ip].hasProject = true;
                m_siteDataMap[ip].siteInfo.ip = ip; // 更新IP
            } else {
                // 如果没有对应的站点信息，创建新的
                SiteData siteData;
                siteData.siteInfo.ip = ip;
                siteData.siteInfo.alias = QString("Site_%1").arg(ip);
                siteData.projectInfo = projectInfo;
                siteData.hasProject = true;
                m_siteDataMap[ip] = siteData;
            }
            
            qDebug() << "解析项目信息，IP:" << ip << "命令数量:" << projectInfo.cmdSequenceArray.size();
        }
        
    } else if (method == "setSKTEnResult") {
        // 解析BPU使能结果
        QJsonObject data = params["data"].toObject();
        QJsonArray results = data["results"].toArray();
        
        for (const auto& resultValue : results) {
            QString resultStr = resultValue.toString();
            QString ip;
            quint32 bpuEn;
            
            extractIPFromBPUResult(resultStr, ip, bpuEn);
            
            if (!ip.isEmpty()) {
                BPUInfo bpuInfo;
                bpuInfo.ip = ip;
                bpuInfo.bpuEn = bpuEn;
                bpuInfo.socketStates = convertBPUEnToBitArray(bpuEn);
                
                // 更新对应IP的站点数据
                if (m_siteDataMap.contains(ip)) {
                    m_siteDataMap[ip].bpuInfo = bpuInfo;
                    m_siteDataMap[ip].hasBPU = true;
                } else {
                    // 创建新的站点数据
                    SiteData siteData;
                    siteData.siteInfo.ip = ip;
                    siteData.siteInfo.alias = QString("Site_%1").arg(ip);
                    siteData.bpuInfo = bpuInfo;
                    siteData.hasBPU = true;
                    m_siteDataMap[ip] = siteData;
                }
                
                qDebug() << "解析BPU信息，IP:" << ip << "BPUEn:" << bpuEn << "二进制:" << formatBPUEnBinary(bpuEn);
            }
        }
    }
    
    // 更新界面显示
    updateSiteInfoDisplay();
}

// === 辅助方法实现 ===

QBitArray JsonRpcTestWidget::convertBPUEnToBitArray(quint32 bpuEn)
{
    QBitArray bitArray(16);
    for (int i = 0; i < 16; ++i) {
        bitArray.setBit(i, (bpuEn >> i) & 1);
    }
    return bitArray;
}

quint32 JsonRpcTestWidget::convertBitArrayToBPUEn(const QBitArray& bitArray)
{
    quint32 bpuEn = 0;
    for (int i = 0; i < qMin(16, bitArray.size()); ++i) {
        if (bitArray.testBit(i)) {
            bpuEn |= (1u << i);
        }
    }
    return bpuEn;
}

QString JsonRpcTestWidget::formatBPUEnBinary(quint32 bpuEn)
{
    QString binary = QString::number(bpuEn, 2).rightJustified(16, '0');
    // 从右往左，每4位添加一个空格便于阅读
    QString formatted;
    for (int i = 0; i < 16; ++i) {
        if (i > 0 && i % 4 == 0) {
            formatted = " " + formatted;
        }
        formatted = binary[15-i] + formatted;
    }
    return formatted;
}

void JsonRpcTestWidget::extractIPFromBPUResult(const QString& result, QString& ip, quint32& bpuEn)
{
    // 解析格式如："BPUEn:3855 recvIP:192.168.31.30 nHop:0"
    QRegularExpression regex(R"(BPUEn:(\d+)\s+recvIP:([0-9.]+))");
    QRegularExpressionMatch match = regex.match(result);
    
    if (match.hasMatch()) {
        bpuEn = match.captured(1).toUInt();
        ip = match.captured(2);
    } else {
        ip.clear();
        bpuEn = 0;
    }
}

// === 界面更新方法 ===

void JsonRpcTestWidget::updateSiteInfoDisplay()
{
    // 更新站点下拉框
    m_siteComboBox->clear();
    
    for (auto it = m_siteDataMap.begin(); it != m_siteDataMap.end(); ++it) {
        const SiteData& siteData = it.value();
        QString displayText;
        
        if (!siteData.siteInfo.alias.isEmpty()) {
            displayText = QString("%1 (%2)").arg(siteData.siteInfo.alias).arg(siteData.siteInfo.ip);
        } else {
            displayText = siteData.siteInfo.ip;
        }
        
        // 只显示有项目信息的站点
        if (siteData.hasProject) {
            m_siteComboBox->addItem(displayText, siteData.siteInfo.ip);
            m_siteComboBox->setEnabled(true);
        }
    }
    
    if (m_siteComboBox->count() == 0) {
        m_siteInfoLabel->setText("请先进行站点扫描并加载项目");
        m_siteComboBox->setEnabled(false);
    } else {
        // 自动选择第一个站点
        onSiteSelectionChanged();
    }
}

void JsonRpcTestWidget::updateSocketDisplay()
{
    QString currentIP = m_siteComboBox->currentData().toString();
    if (currentIP.isEmpty() || !m_siteDataMap.contains(currentIP)) {
        // 禁用所有座子
        for (int i = 0; i < 16; ++i) {
            m_socketCheckBoxes[i]->setEnabled(false);
            m_socketCheckBoxes[i]->setChecked(false);
        }
        m_selectedSocketsLabel->setText("无BPU信息");
        return;
    }
    
    const SiteData& siteData = m_siteDataMap[currentIP];
    if (!siteData.hasBPU) {
        // 禁用所有座子
        for (int i = 0; i < 16; ++i) {
            m_socketCheckBoxes[i]->setEnabled(false);
            m_socketCheckBoxes[i]->setChecked(false);
        }
        m_selectedSocketsLabel->setText("无BPU信息");
        return;
    }
    
    // 根据BPU使能状态设置座子
    const QBitArray& socketStates = siteData.bpuInfo.socketStates;
    for (int i = 0; i < 16; ++i) {
        bool enabled = socketStates.testBit(i);
        m_socketCheckBoxes[i]->setEnabled(enabled);
        m_socketCheckBoxes[i]->setChecked(false); // 默认不选中
        
        // 设置样式
        if (enabled) {
            m_socketCheckBoxes[i]->setStyleSheet("QCheckBox { color: green; font-weight: bold; }");
            m_socketCheckBoxes[i]->setToolTip(QString("座子 %1 可用").arg(i));
        } else {
            m_socketCheckBoxes[i]->setStyleSheet("QCheckBox { color: gray; }");
            m_socketCheckBoxes[i]->setToolTip(QString("座子 %1 不可用").arg(i));
        }
    }
    
    // 显示BPU信息
    QString bpuInfo = QString("BPU使能: %1 (二进制: %2)")
                      .arg(siteData.bpuInfo.bpuEn)
                      .arg(formatBPUEnBinary(siteData.bpuInfo.bpuEn));
    m_selectedSocketsLabel->setText(bpuInfo);
}

void JsonRpcTestWidget::updateCmdSequenceDisplay()
{
    QString currentIP = m_siteComboBox->currentData().toString();
    if (currentIP.isEmpty() || !m_siteDataMap.contains(currentIP)) {
        m_cmdSequenceComboBox->clear();
        m_cmdSequenceComboBox->setEnabled(false);
        return;
    }
    
    const SiteData& siteData = m_siteDataMap[currentIP];
    if (!siteData.hasProject) {
        m_cmdSequenceComboBox->clear();
        m_cmdSequenceComboBox->setEnabled(false);
        return;
    }
    
    // 填充命令序列下拉框
    m_cmdSequenceComboBox->clear();
    const QJsonArray& cmdArray = siteData.projectInfo.cmdSequenceArray;
    
    for (const auto& cmdValue : cmdArray) {
        QJsonObject cmdObj = cmdValue.toObject();
        QString cmdRun = cmdObj["CmdRun"].toString();
        QString cmdID = cmdObj["CmdID"].toString();
        int groupCount = cmdObj["CmdSequencesGroupCnt"].toInt();
        
        QString displayText = QString("%1 (ID:%2, 步骤:%3)").arg(cmdRun).arg(cmdID).arg(groupCount);
        m_cmdSequenceComboBox->addItem(displayText, cmdValue);
    }
    
    m_cmdSequenceComboBox->setEnabled(true);
}

// === 事件处理方法 ===

void JsonRpcTestWidget::onSiteSelectionChanged()
{
    QString currentIP = m_siteComboBox->currentData().toString();
    if (currentIP.isEmpty() || !m_siteDataMap.contains(currentIP)) {
        m_siteInfoLabel->setText("无效的站点选择");
        return;
    }
    
    const SiteData& siteData = m_siteDataMap[currentIP];
    
    // 更新站点信息显示
    QString infoText;
    infoText += QString("站点: %1\n").arg(siteData.siteInfo.alias);
    infoText += QString("IP地址: %1\n").arg(siteData.siteInfo.ip);
    infoText += QString("MAC地址: %1\n").arg(siteData.siteInfo.macAddr);
    infoText += QString("固件版本: %1\n").arg(siteData.siteInfo.firmwareVersion);
    
    if (siteData.hasProject) {
        infoText += QString("项目: %1\n").arg(siteData.projectInfo.projectUrl);
        infoText += QString("命令数量: %1").arg(siteData.projectInfo.cmdSequenceArray.size());
    }
    
    m_siteInfoLabel->setText(infoText);
    
    // 更新座子和命令序列显示
    updateSocketDisplay();
    updateCmdSequenceDisplay();
    
    // 更新执行按钮状态
    updateExecuteButtonState();
}

void JsonRpcTestWidget::onSocketToggled(int socketIndex, bool enabled)
{
    Q_UNUSED(enabled)
    
    // 更新选中座子显示
    QStringList selectedSockets;
    quint32 selectedBPU = 0;
    
    for (int i = 0; i < 16; ++i) {
        if (m_socketCheckBoxes[i]->isChecked()) {
            selectedSockets << QString("S%1").arg(i);
            selectedBPU |= (1u << i);
        }
    }
    
    if (selectedSockets.isEmpty()) {
        m_selectedSocketsLabel->setText("未选择座子");
    } else {
        QString text = QString("已选择座子: %1 (BPU: %2)")
                       .arg(selectedSockets.join(", "))
                       .arg(selectedBPU);
        m_selectedSocketsLabel->setText(text);
    }
    
    // 更新执行按钮状态
    updateExecuteButtonState();
}

void JsonRpcTestWidget::onCmdSequenceSelectionChanged()
{
    // 更新执行按钮状态
    updateExecuteButtonState();
}

void JsonRpcTestWidget::updateExecuteButtonState()
{
    // 安全检查：确保UI组件已初始化
    if (!m_executeJobBtn || !m_siteComboBox || !m_cmdSequenceComboBox) {
        return;
    }
    
    bool hasSelectedSockets = false;
    for (int i = 0; i < 16; ++i) {
        if (m_socketCheckBoxes[i] && m_socketCheckBoxes[i]->isChecked()) {
            hasSelectedSockets = true;
            break;
        }
    }
    
    bool hasSelectedCmd = m_cmdSequenceComboBox->currentIndex() >= 0;
    bool hasValidSite = !m_siteComboBox->currentData().toString().isEmpty();
    
    // 更新标准作业执行按钮
    m_executeJobBtn->setEnabled(hasSelectedSockets && hasSelectedCmd && hasValidSite && m_isConnected);
    
    // 更新自定义命令执行按钮（不需要命令序列，只需要站点和座子）
    if (m_executeCustomBtn && m_customJsonEdit && m_customStatusLabel) {
        bool hasCustomText = !m_customJsonEdit->toPlainText().trimmed().isEmpty();
        m_executeCustomBtn->setEnabled(hasSelectedSockets && hasCustomText && hasValidSite && m_isConnected);
        
        // 更新状态显示
        QString statusText;
        if (!hasValidSite) {
            statusText = "❌ 请先选择站点";
        } else if (!hasSelectedSockets) {
            statusText = "❌ 请选择至少一个座子";
        } else if (!hasCustomText) {
            statusText = "❌ 请输入JSON数据";
        } else if (!m_isConnected) {
            statusText = "❌ 请先连接到服务器";
        } else {
            QString currentIP = m_siteComboBox->currentData().toString();
            QStringList selectedSockets;
            for (int i = 0; i < 16; ++i) {
                if (m_socketCheckBoxes[i] && m_socketCheckBoxes[i]->isChecked()) {
                    selectedSockets << QString("S%1").arg(i);
                }
            }
            statusText = QString("✅ 准备就绪 - 站点: %1, 座子: %2")
                        .arg(currentIP)
                        .arg(selectedSockets.join(", "));
        }
        m_customStatusLabel->setText(statusText);
    }
}

void JsonRpcTestWidget::onExecuteJobClicked()
{
    QString currentIP = m_siteComboBox->currentData().toString();
    if (currentIP.isEmpty()) {
        QMessageBox::warning(this, "错误", "请选择有效的站点");
        return;
    }
    
    // 获取选中的座子
    quint32 selectedBPU = 0;
    for (int i = 0; i < 16; ++i) {
        if (m_socketCheckBoxes[i]->isChecked()) {
            selectedBPU |= (1u << i);
        }
    }
    
    if (selectedBPU == 0) {
        QMessageBox::warning(this, "错误", "请至少选择一个座子");
        return;
    }
    
    // 获取选中的命令序列
    QJsonValue cmdSequenceValue = m_cmdSequenceComboBox->currentData().toJsonValue();
    if (!cmdSequenceValue.isObject()) {
        QMessageBox::warning(this, "错误", "请选择有效的命令序列");
        return;
    }
    
    QJsonObject cmdSequence = cmdSequenceValue.toObject();
    
    // 构造doJob参数
    QJsonObject params;
    params["strIP"] = currentIP;
    params["sktEn"] = static_cast<qint64>(selectedBPU);
    params["docmdSeqJson"] = cmdSequence;
    
    // 发送doJob命令
    QString description = QString("执行作业 - 站点:%1, 座子:%2, 命令:%3")
                          .arg(currentIP)
                          .arg(selectedBPU)
                          .arg(cmdSequence["CmdRun"].toString());
    
    callRpcMethod("DoJob", params, description);
    
    // 显示执行信息
    QString message = QString("已发送作业到站点 %1\n座子: %2\n命令: %3")
                      .arg(currentIP)
                      .arg(formatBPUEnBinary(selectedBPU))
                      .arg(cmdSequence["CmdRun"].toString());
    
    QMessageBox::information(this, "作业执行", message);
}

// === 自定义JSON编辑事件处理 ===

void JsonRpcTestWidget::onCustomJsonTextChanged()
{
    // 通过updateExecuteButtonState统一检查状态
    updateExecuteButtonState();
}

void JsonRpcTestWidget::onFormatJsonClicked()
{
    QString jsonText = m_customJsonEdit->toPlainText().trimmed();
    if (jsonText.isEmpty()) {
        QMessageBox::information(this, "提示", "请先输入JSON数据");
        return;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonText.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        QMessageBox::warning(this, "JSON格式错误", 
                           QString("JSON解析失败：\n%1\n位置：%2")
                           .arg(error.errorString())
                           .arg(error.offset));
        return;
    }
    
    // 格式化JSON并设置回编辑器
    QString formattedJson = doc.toJson(QJsonDocument::Indented);
    m_customJsonEdit->setPlainText(formattedJson);
    
    // 显示成功信息
    QMessageBox::information(this, "格式化完成", "JSON格式化成功！");
}

void JsonRpcTestWidget::onClearJsonClicked()
{
    if (!m_customJsonEdit->toPlainText().trimmed().isEmpty()) {
        auto reply = QMessageBox::question(this, "确认清空", 
                                         "确定要清空JSON编辑器的内容吗？",
                                         QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            m_customJsonEdit->clear();
        }
    }
}

void JsonRpcTestWidget::onExecuteCustomClicked()
{
    QString jsonText = m_customJsonEdit->toPlainText().trimmed();
    QString currentIP = m_siteComboBox->currentData().toString();
    
    if (jsonText.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入JSON数据");
        return;
    }
    
    if (currentIP.isEmpty()) {
        QMessageBox::warning(this, "错误", "请选择有效的站点");
        return;
    }
    
    // 验证JSON格式
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonText.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        QMessageBox::warning(this, "JSON格式错误", 
                           QString("JSON解析失败：\n%1\n位置：%2")
                           .arg(error.errorString())
                           .arg(error.offset));
        return;
    }
    
    // 获取选中的座子
    quint32 selectedBPU = 0;
    for (int i = 0; i < 16; ++i) {
        if (m_socketCheckBoxes[i] && m_socketCheckBoxes[i]->isChecked()) {
            selectedBPU |= (1u << i);
        }
    }
    
    if (selectedBPU == 0) {
        QMessageBox::warning(this, "错误", "请至少选择一个座子");
        return;
    }
    
    // 构造DoCustom参数（保持与doJob一致的参数结构）
    QJsonObject params;
    params["strIP"] = currentIP;
    params["sktEn"] = static_cast<qint64>(selectedBPU);
    params["doCustomData"] = doc.object().isEmpty() ? doc.array().isEmpty() ? 
                           QJsonValue(jsonText) : QJsonValue(doc.array()) : QJsonValue(doc.object());
    
    // 如果JSON是基本类型（字符串、数字等），直接使用原始值
    if (!doc.isObject() && !doc.isArray()) {
        if (doc.isNull()) {
            params["doCustomData"] = QJsonValue::Null;
        } else {
            // 尝试解析为基本类型
            bool ok;
            double num = jsonText.toDouble(&ok);
            if (ok) {
                params["doCustomData"] = num;
            } else if (jsonText.toLower() == "true") {
                params["doCustomData"] = true;
            } else if (jsonText.toLower() == "false") {
                params["doCustomData"] = false;
            } else if (jsonText.toLower() == "null") {
                params["doCustomData"] = QJsonValue::Null;
            } else {
                // 作为字符串处理（去掉引号）
                QString cleanText = jsonText;
                if (cleanText.startsWith('"') && cleanText.endsWith('"')) {
                    cleanText = cleanText.mid(1, cleanText.length() - 2);
                }
                params["doCustomData"] = cleanText;
            }
        }
    }
    
    // 发送DoCustom命令
    QString description = QString("执行自定义命令 - 站点:%1, 座子:%2")
                          .arg(currentIP)
                          .arg(selectedBPU);
    
    m_client->doCustom(params, [this, currentIP, selectedBPU, jsonText](bool success, const QJsonObject& result, const QString& errorMessage) {
        if (success) {
            addLogEntry("SUCCESS", "自定义命令执行成功", result);
            
            QString message = QString("自定义命令已发送到站点 %1\n座子: %2\n数据: %3")
                              .arg(currentIP)
                              .arg(formatBPUEnBinary(selectedBPU))
                              .arg(jsonText.left(100) + (jsonText.length() > 100 ? "..." : ""));
                              
            QMessageBox::information(this, "命令执行", message);
        } else {
            addLogEntry("ERROR", "自定义命令执行失败", QJsonObject{{"error", errorMessage}});
            QMessageBox::critical(this, "执行失败", QString("自定义命令执行失败：\n%1").arg(errorMessage));
        }
    });
    
    // 记录发送的请求
    addLogEntry("REQUEST", QString("自定义命令发送 - %1").arg(description), params);
} 
