#include "presentation/TestBoardControlWidget.h"
#include "domain/TestBoardDevice.h"
#include "infrastructure/ICommunicationChannel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QTableWidget>
#include <QTextEdit>
#include <QHeaderView>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QTimer>
#include <QComboBox>
#include <QMutex>
#include <QTextCursor>
#include "core/Logger.h"

namespace Presentation {

TestBoardControlWidget::TestBoardControlWidget(QWidget *parent)
    : IDeviceControlWidget(parent)
    , m_testBoard(nullptr)
    , m_updateTimer(new QTimer(this))
    , m_dataUpdateTimer(new QTimer(this))
    , m_logUpdateTimer(new QTimer(this))
    , m_isAcquiring(false)
    , m_currentChipIndex(1)  // 默认为A300
    , m_hasPendingData(false)
    , m_frameCounter(0)
    , m_enableRawFrameLogging(false)  // 默认禁用原始帧日志
    , m_logMutex(QMutex::Recursive)
{
    setupUi();
    
    // 初始化表格项缓存（移到setupUi之后）
    LOG_MODULE_DEBUG("TestBoardUI", "Initializing table items...");
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 10; ++j) {
            m_tableItems[i][j] = new QTableWidgetItem();
            m_dataTable->setItem(i, j, m_tableItems[i][j]);
        }
    }
    LOG_MODULE_DEBUG("TestBoardUI", "Table items initialized.");
    
    connectSignals();

    m_updateTimer->setInterval(1000); // 1s a time
    connect(m_updateTimer, &QTimer::timeout, this, &TestBoardControlWidget::onUpdateTimer);
    
    // 设置数据更新定时器（降低更新频率）
    m_dataUpdateTimer->setInterval(100); // 100ms更新一次，即10Hz
    connect(m_dataUpdateTimer, &QTimer::timeout, this, &TestBoardControlWidget::onDataUpdateTimeout);
    
    // 设置日志更新定时器
    m_logUpdateTimer->setInterval(200); // 200ms更新一次
    connect(m_logUpdateTimer, &QTimer::timeout, this, &TestBoardControlWidget::onLogUpdateTimeout);
    m_logUpdateTimer->start();

    setControlsEnabled(false);
}

TestBoardControlWidget::~TestBoardControlWidget()
{
    m_updateTimer->stop();
    m_dataUpdateTimer->stop();
    m_logUpdateTimer->stop();
    disconnectSignals();
    
    // 清理表格项缓存
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 10; ++j) {
            delete m_tableItems[i][j];
        }
    }
}

void TestBoardControlWidget::setupUi()
{
    auto mainLayout = new QVBoxLayout(this);

    // Top part: status and main controls
    auto topLayout = new QHBoxLayout();

    // Status Group
    auto statusGroup = new QGroupBox("设备状态");
    auto statusLayout = new QFormLayout(statusGroup);
    m_statusEdit = new QLineEdit();
    m_statusEdit->setReadOnly(true);
    m_serialNumberEdit = new QLineEdit();
    m_serialNumberEdit->setReadOnly(true);
    statusLayout->addRow("状态:", m_statusEdit);
    statusLayout->addRow("序列号:", m_serialNumberEdit);
    topLayout->addWidget(statusGroup, 1);

    // Main Control Group
    auto mainControlGroup = new QGroupBox("主控制");
    auto mainControlLayout = new QGridLayout(mainControlGroup);
    m_startButton = new QPushButton("开始采集");
    m_stopButton = new QPushButton("停止采集");
    m_powerOnButton = new QPushButton("DUT上电");
    m_powerOffButton = new QPushButton("DUT断电");
    m_queryVoltageButton = new QPushButton("查询电压");
    m_queryFaultButton = new QPushButton("查询故障");
    mainControlLayout->addWidget(m_startButton, 0, 0);
    mainControlLayout->addWidget(m_stopButton, 0, 1);
    mainControlLayout->addWidget(m_powerOnButton, 1, 0);
    mainControlLayout->addWidget(m_powerOffButton, 1, 1);
    mainControlLayout->addWidget(m_queryVoltageButton, 2, 0);
    mainControlLayout->addWidget(m_queryFaultButton, 2, 1);
    topLayout->addWidget(mainControlGroup, 1);

    mainLayout->addLayout(topLayout);

    // DUT Selection
    auto dutGroup = new QGroupBox("DUT选择");
    auto dutLayout = new QVBoxLayout(dutGroup);
    auto dutSelectionLayout = new QHBoxLayout();
    m_selectAllCheckBox = new QCheckBox("全选");
    dutSelectionLayout->addWidget(m_selectAllCheckBox);
    for(int i = 0; i < 8; ++i) {
        m_dutCheckBoxes[i] = new QCheckBox(QString("DUT %1").arg(i+1));
        dutSelectionLayout->addWidget(m_dutCheckBoxes[i]);
    }
    dutSelectionLayout->addStretch();
    dutLayout->addLayout(dutSelectionLayout);
    
    // 修复：使用中文引号避免嵌套字符串导致编译错误
    auto helpLabel = new QLabel(QString::fromUtf8("提示: 请在勾选需要监控的DUT后，再点击\"开始采集\"。"));
    helpLabel->setStyleSheet("color: gray;");
    dutLayout->addWidget(helpLabel);

    mainLayout->addWidget(dutGroup);

    // Advanced commands
    auto advancedGroup = new QGroupBox("高级/调试指令");
    auto advancedLayout = new QGridLayout(advancedGroup);

    // Set Chip Type
    advancedLayout->addWidget(new QLabel("芯片类型:"), 0, 0);
    m_chipTypeComboBox = new QComboBox();
    m_chipTypeComboBox->addItem("A300", 1);
    m_chipTypeComboBox->addItem("G300", 2);
    m_chipTypeComboBox->addItem("270", 3);
    advancedLayout->addWidget(m_chipTypeComboBox, 0, 1);
    m_setChipTypeButton = new QPushButton("设置芯片类型");
    advancedLayout->addWidget(m_setChipTypeButton, 0, 2);

    // Calibration
    advancedLayout->addWidget(new QLabel("标定指令(Hex):"), 1, 0);
    m_calibCmdEdit = new QLineEdit();
    m_calibCmdEdit->setPlaceholderText("例如: 0102030A");
    advancedLayout->addWidget(m_calibCmdEdit, 1, 1, 1, 3);
    m_calibSendButton = new QPushButton("发送标定指令");
    advancedLayout->addWidget(m_calibSendButton, 1, 4);

    // Register Write
    advancedLayout->addWidget(new QLabel("写寄存器:"), 2, 0);
    m_regWriteAddrEdit = new QLineEdit();
    m_regWriteAddrEdit->setPlaceholderText("地址(Hex)");
    advancedLayout->addWidget(m_regWriteAddrEdit, 2, 1);
    m_regWriteDataEdit = new QLineEdit();
    m_regWriteDataEdit->setPlaceholderText("数据(Hex)");
    advancedLayout->addWidget(m_regWriteDataEdit, 2, 2, 1, 2);
    m_regWriteSendButton = new QPushButton("写入");
    advancedLayout->addWidget(m_regWriteSendButton, 2, 4);
    
    // Register Read
    advancedLayout->addWidget(new QLabel("读寄存器:"), 3, 0);
    m_regReadAddrEdit = new QLineEdit();
    m_regReadAddrEdit->setPlaceholderText("地址(Hex)");
    advancedLayout->addWidget(m_regReadAddrEdit, 3, 1);
    m_regReadLengthEdit = new QLineEdit();
    m_regReadLengthEdit->setPlaceholderText("长度(Dec)");
    advancedLayout->addWidget(m_regReadLengthEdit, 3, 2);
    m_regReadSendButton = new QPushButton("读取");
    advancedLayout->addWidget(m_regReadSendButton, 3, 4);
    
    mainLayout->addWidget(advancedGroup);

    // Data Table
    m_dataTable = new QTableWidget();
    m_dataTable->setColumnCount(10); // DUT, ChipType, Gx, Gy, Gz, Ax, Ay, Az, Mix, Temp
    m_dataTable->setHorizontalHeaderLabels({"DUT", "芯片类型", "Gyro X (LSB)", "Gyro Y (LSB)", "Gyro Z (LSB)", "Acc X (LSB)", "Acc Y (LSB)", "Acc Z (LSB)", "Mix (LSB)", "Temp (°C)"});
    m_dataTable->setRowCount(9); // 8 DUTs + 1 External
    QStringList verticalLabels;
    for(int i=0; i<8; ++i) verticalLabels << QString("DUT %1").arg(i+1);
    verticalLabels << "External";
    m_dataTable->setVerticalHeaderLabels(verticalLabels);
    m_dataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_dataTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(m_dataTable, 4);

    // Log and Error
    auto bottomLayout = new QHBoxLayout();
    
    // 添加日志控制区域
    auto logControlLayout = new QVBoxLayout();
    m_enableRawFrameCheckBox = new QCheckBox("显示原始数据帧");
    m_enableRawFrameCheckBox->setChecked(false);
    connect(m_enableRawFrameCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        m_enableRawFrameLogging = checked;
        if (!checked) {
            QMutexLocker locker(&m_logMutex);
            m_logBuffer.append(QString("[%1] 原始数据帧日志已禁用（提升性能）")
                .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
        }
    });
    logControlLayout->addWidget(m_enableRawFrameCheckBox);
    
    m_logTextEdit = new QTextEdit();
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setPlaceholderText("指令日志和原始数据帧将在这里显示...");
    // 使用 QTextDocument 来限制内容大小
    m_logTextEdit->document()->setMaximumBlockCount(1000); // 限制最大行数，避免内存占用过大
    logControlLayout->addWidget(m_logTextEdit);
    
    bottomLayout->addLayout(logControlLayout, 2);

    m_errorLabel = new QLabel();
    m_errorLabel->setStyleSheet("QLabel { color: red; }");
    m_errorLabel->setWordWrap(true);
    bottomLayout->addWidget(m_errorLabel, 1);

    mainLayout->addLayout(bottomLayout);
}

void TestBoardControlWidget::connectSignals()
{
    connect(m_startButton, &QPushButton::clicked, this, &TestBoardControlWidget::onStartAcquisition);
    connect(m_stopButton, &QPushButton::clicked, this, &TestBoardControlWidget::onStopAcquisition);
    connect(m_powerOnButton, &QPushButton::clicked, this, &TestBoardControlWidget::onDutPowerOn);
    connect(m_powerOffButton, &QPushButton::clicked, this, &TestBoardControlWidget::onDutPowerOff);
    connect(m_queryVoltageButton, &QPushButton::clicked, this, &TestBoardControlWidget::onQueryVoltage);
    connect(m_queryFaultButton, &QPushButton::clicked, this, &TestBoardControlWidget::onQueryFault);
    connect(m_selectAllCheckBox, &QCheckBox::stateChanged, this, &TestBoardControlWidget::onDutSelectionChanged);

    connect(m_setChipTypeButton, &QPushButton::clicked, this, &TestBoardControlWidget::onSetChipType);
    connect(m_calibSendButton, &QPushButton::clicked, this, &TestBoardControlWidget::onSendCalibration);
    connect(m_regWriteSendButton, &QPushButton::clicked, this, &TestBoardControlWidget::onWriteRegister);
    connect(m_regReadSendButton, &QPushButton::clicked, this, &TestBoardControlWidget::onReadRegister);
}

void TestBoardControlWidget::disconnectSignals()
{
    if(m_testBoard) {
        disconnect(m_testBoard.get(), nullptr, this, nullptr);
    }
}

void TestBoardControlWidget::setDevice(std::shared_ptr<Domain::IDevice> device)
{
    m_updateTimer->stop();
    disconnectSignals();

    m_testBoard = std::dynamic_pointer_cast<Domain::TestBoardDevice>(device);

    if (m_testBoard) {
        connect(m_testBoard.get(), &Domain::TestBoardDevice::testDataReceived, this, &TestBoardControlWidget::onTestDataReceived);
        connect(m_testBoard.get(), &Domain::TestBoardDevice::testStateChanged, this, &TestBoardControlWidget::onTestStateChanged);
        connect(m_testBoard.get(), &Domain::TestBoardDevice::powerStateChanged, this, &TestBoardControlWidget::onPowerStateChanged);
        connect(m_testBoard.get(), &Domain::TestBoardDevice::voltageDataReceived, this, &TestBoardControlWidget::onVoltageDataReceived);
        connect(m_testBoard.get(), &Domain::TestBoardDevice::faultStatusReceived, this, &TestBoardControlWidget::onFaultStatusReceived);
        
        // 连接到设备发出的原始数据帧信号
        connect(m_testBoard.get(), &Domain::IDevice::rawFrameSent, this, &TestBoardControlWidget::onFrameSent);
        connect(m_testBoard.get(), &Domain::IDevice::rawFrameReceived, this, &TestBoardControlWidget::onFrameReceived);

        connect(m_testBoard.get(), &Domain::TestBoardDevice::dataError, this, &TestBoardControlWidget::onDataError);

        connect(m_testBoard.get(), &Domain::TestBoardDevice::calibrationResultReceived, this, &TestBoardControlWidget::onCalibrationResult);
        connect(m_testBoard.get(), &Domain::TestBoardDevice::registerWriteResultReceived, this, &TestBoardControlWidget::onRegisterWriteResult);
        connect(m_testBoard.get(), &Domain::TestBoardDevice::registerReadResultReceived, this, &TestBoardControlWidget::onRegisterReadResult);
        connect(m_testBoard.get(), &Domain::TestBoardDevice::chipTypeSetResultReceived, this, &TestBoardControlWidget::onChipTypeSetResult);

        updateStatus();
        m_updateTimer->start();
    } else {
        setControlsEnabled(false);
        m_statusEdit->clear();
        m_serialNumberEdit->clear();
        m_errorLabel->clear();
        m_logTextEdit->clear();
        m_dataTable->clearContents();
    }
}

std::shared_ptr<Domain::IDevice> TestBoardControlWidget::getDevice() const
{
    return m_testBoard;
}

void TestBoardControlWidget::updateStatus()
{
    if (!m_testBoard) return;

    auto status = m_testBoard->getStatus();
    QString statusText;
    bool isConnected = false;
    switch(status) {
        case Domain::IDevice::DeviceStatus::Ready: statusText = "就绪"; isConnected = true; break;
        case Domain::IDevice::DeviceStatus::Connected: statusText = "已连接"; isConnected = true; break;
        case Domain::IDevice::DeviceStatus::Busy: statusText = "采集中..."; isConnected = true; break;
        case Domain::IDevice::DeviceStatus::Error: statusText = "错误"; isConnected = true; break;
        default: statusText = "未连接"; break;
    }
    m_statusEdit->setText(statusText);
    setControlsEnabled(isConnected && !m_isAcquiring);
    m_startButton->setEnabled(isConnected && !m_isAcquiring);
    m_stopButton->setEnabled(isConnected && m_isAcquiring);
}

void TestBoardControlWidget::setControlsEnabled(bool enabled)
{
    m_powerOnButton->setEnabled(enabled);
    m_powerOffButton->setEnabled(enabled);
    m_queryVoltageButton->setEnabled(enabled);
    m_queryFaultButton->setEnabled(enabled);
    m_selectAllCheckBox->setEnabled(enabled);
    for(auto checkbox : m_dutCheckBoxes) {
        checkbox->setEnabled(enabled);
    }
    m_chipTypeComboBox->setEnabled(enabled);
    m_setChipTypeButton->setEnabled(enabled);
    m_calibCmdEdit->setEnabled(enabled);
    m_calibSendButton->setEnabled(enabled);
    m_regWriteAddrEdit->setEnabled(enabled);
    m_regWriteDataEdit->setEnabled(enabled);
    m_regWriteSendButton->setEnabled(enabled);
    m_regReadAddrEdit->setEnabled(enabled);
    m_regReadLengthEdit->setEnabled(enabled);
    m_regReadSendButton->setEnabled(enabled);
}

uint16_t TestBoardControlWidget::getSelectedDuts() const
{
    uint16_t selectionLow = 0;
    for (int i = 0; i < 8; ++i) {
        if (m_dutCheckBoxes[i]->isChecked()) {
            selectionLow |= (1 << (7 - i));   // DUT1→bit7 … DUT8→bit0
        }
    }

    uint16_t selectionHigh = static_cast<uint16_t>(selectionLow << 8);

    // 调试输出 16 进制掩码值（高位在前，宽 4 位，不足补 0）
    LOG_MODULE_DEBUG("TestBoardUI", QString("[getSelectedDuts] selection = 0x high%1 low%2")
                            .arg(QString::number(selectionHigh,16).toUpper().rightJustified(4,'0'))
                            .arg(QString::number(selectionLow,16).toUpper().rightJustified(4,'0')).toStdString());

    return selectionHigh;
}

void TestBoardControlWidget::executeDeviceCommand(const QString& command, const QJsonObject& params)
{
    if (!m_testBoard) return;

    setControlsEnabled(false);
    m_startButton->setEnabled(false);
    m_stopButton->setEnabled(false);

    {
        QMutexLocker locker(&m_logMutex);
        m_logBuffer.append(QString("[%1] > EXEC: %2, PARAMS: %3")
            .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
            .arg(command)
            .arg(QString(QJsonDocument(params).toJson(QJsonDocument::Compact))));
    }

    auto watcher = new QFutureWatcher<QJsonObject>(this);
    connect(watcher, &QFutureWatcher<QJsonObject>::finished, this, [this, watcher, command]() {
        QJsonObject result = watcher->result();
        if (!result.value("success").toBool()) {
            QString error = result.value("error").toString("Unknown error");
            onDataError(error);
            QMutexLocker locker(&m_logMutex);
            m_logBuffer.append(QString("[%1] < FAIL: %2. Error: %3")
                .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
                .arg(command)
                .arg(error));
        } else {
            QMutexLocker locker(&m_logMutex);
            m_logBuffer.append(QString("[%1] < OK: %2")
                .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
                .arg(command));
        }
        watcher->deleteLater();
        updateStatus(); // 命令完成后，根据最新状态更新UI
    });

    auto future = QtConcurrent::run([board = m_testBoard, command, params]() {
        // 在后台线程执行可能阻塞的命令
        return board->executeCommand(command, params);
    });
    watcher->setFuture(future);
}

void TestBoardControlWidget::onStartAcquisition()
{
    if (!m_testBoard) return;
    QJsonObject params;
    params["dutActive"] = getSelectedDuts();
    executeDeviceCommand("startTest", params);
}

void TestBoardControlWidget::onStopAcquisition()
{
    if (!m_testBoard) return;
    executeDeviceCommand("stopTest", {});
}

void TestBoardControlWidget::onDutPowerOn()
{
    if (!m_testBoard) return;
    QJsonObject params;
    params["enable"] = true;
    params["dutEnable"] = getSelectedDuts();
    executeDeviceCommand("controlDutPower", params);
}

void TestBoardControlWidget::onDutPowerOff()
{
    if (!m_testBoard) return;
    QJsonObject params;
    params["enable"] = false;
    params["dutEnable"] = getSelectedDuts();
    executeDeviceCommand("controlDutPower", params);
}

void TestBoardControlWidget::onQueryVoltage()
{
    if (!m_testBoard) return;
    executeDeviceCommand("queryvoltagestatus", {});
}

void TestBoardControlWidget::onQueryFault()
{
    if (!m_testBoard) return;
    executeDeviceCommand("queryfaultstatus", {});
}

void TestBoardControlWidget::onDutSelectionChanged()
{
    bool all_checked = m_selectAllCheckBox->isChecked();
    for(auto checkbox : m_dutCheckBoxes) {
        checkbox->setChecked(all_checked);
    }
}

void TestBoardControlWidget::onUpdateTimer()
{
    updateStatus();
}

void TestBoardControlWidget::onTestDataReceived(const Domain::Protocols::TestFeedbackData &data)
{
    LOG_MODULE_DEBUG("TestBoardUI", QString("[onTestDataReceived] serial:%1 chip:%2 dutActive:0x%3 acquiring:%4")
            .arg(data.serialNumber)
            .arg(data.chipIndex)
            .arg(QString::number(data.dutActive,16))
            .arg(m_isAcquiring).toStdString());
    
    m_serialNumberEdit->setText(QString::number(data.serialNumber));
    m_currentChipIndex = data.chipIndex;  // 更新当前芯片类型
    
    // 缓存数据，等待定时器批量更新
    m_pendingData = data;
    m_hasPendingData = true;
    
    // 如果是第一次接收数据，启动数据更新定时器
    if (!m_dataUpdateTimer->isActive()) {
        LOG_MODULE_DEBUG("TestBoardUI", "Starting data update timer");
        m_dataUpdateTimer->start();
    }
}

void TestBoardControlWidget::onTestStateChanged(bool isRunning)
{
    m_isAcquiring = isRunning;
    updateStatus();
    
    // 根据采集状态控制数据更新定时器
    if (!isRunning) {
        m_dataUpdateTimer->stop();
        m_hasPendingData = false;
    }
    
    QMutexLocker locker(&m_logMutex);
    m_logBuffer.append(QString("[%1] 测试状态: %2")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
        .arg(isRunning ? "运行中" : "已停止"));
}

void TestBoardControlWidget::onPowerStateChanged(bool powerOn, uint16_t dutPowerState)
{
    QStringList dutStatusList;
    for (int i = 0; i < 8; ++i) {
        bool dutIsOn = (dutPowerState & (1 << i)) != 0;
        dutStatusList.append(QString("DUT%1: %2").arg(i + 1).arg(dutIsOn ? "ON" : "OFF"));
    }

    QMutexLocker locker(&m_logMutex);
    m_logBuffer.append(QString("[%1] 主电源状态: %2, 原始掩码: 0x%3")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
        .arg(powerOn ? "ON" : "OFF")
        .arg(QString::number(dutPowerState, 16).toUpper().rightJustified(4, '0')));
        
    m_logBuffer.append(QString("[%1] 各DUT电源详情: %2")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
        .arg(dutStatusList.join(", ")));
}

void TestBoardControlWidget::onVoltageDataReceived(const QJsonObject &voltageData)
{
    QString voltageString = QJsonDocument(voltageData).toJson(QJsonDocument::Compact);
    QMutexLocker locker(&m_logMutex);
    m_logBuffer.append(QString("[%1] 电压数据: %2")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
        .arg(voltageString));
}

void TestBoardControlWidget::onFaultStatusReceived(uint32_t faultState)
{
    QMutexLocker locker(&m_logMutex);
    m_logBuffer.append(QString("[%1] 故障码: 0x%2")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
        .arg(QString::number(faultState, 16)));
}

void TestBoardControlWidget::onDataError(const QString &error)
{
    m_errorLabel->setText(error);
    QMutexLocker locker(&m_logMutex);
    m_logBuffer.append(QString("[%1] 错误: %2")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
        .arg(error));
    emit errorOccurred(error);
}

void TestBoardControlWidget::updateDataTable(const Domain::Protocols::TestFeedbackData &data)
{
    LOG_MODULE_DEBUG("TestBoardUI", QString("[updateDataTable] dutActive:0x%1").arg(QString::number(data.dutActive,16)).toStdString());
    
    // 获取芯片类型名称
    QString chipName;
    switch(m_currentChipIndex) {
        case 1: chipName = "A300"; break;
        case 2: chipName = "G300"; break;
        case 3: chipName = "270"; break;
        default: chipName = QString("Unknown(%1)").arg(m_currentChipIndex); break;
    }

    for (int i = 0; i < 8; ++i) {
        if ((data.dutActive >> i) & 0x01) {
            const auto& dutData = data.dutData[i];
            LOG_MODULE_DEBUG("TestBoardUI", QString("[updateDataTable] Updating DUT %1 data").arg(i).toStdString());
            m_tableItems[i][0]->setText("Active");
            m_tableItems[i][1]->setText(chipName);
            m_tableItems[i][2]->setText(QString::number(dutData.gyro_x));
            m_tableItems[i][3]->setText(QString::number(dutData.gyro_y));
            m_tableItems[i][4]->setText(QString::number(dutData.gyro_z));
            m_tableItems[i][5]->setText(QString::number(dutData.gyro_acc_x));
            m_tableItems[i][6]->setText(QString::number(dutData.gyro_acc_y));
            m_tableItems[i][7]->setText(QString::number(dutData.gyro_acc_z));
            m_tableItems[i][8]->setText(QString::number(dutData.gyro_mix));
            m_tableItems[i][9]->setText(QString::number(dutData.gyro_temperature / 100.0, 'f', 2));
        } else {
            for (int j = 0; j < 10; ++j) {
                m_tableItems[i][j]->setText("N/A");
            }
        }
    }

    const auto& extData = data.externalGyro;
    LOG_MODULE_DEBUG("TestBoardUI", "[updateDataTable] Updating External gyro data");
    m_tableItems[8][0]->setText("Active");
    m_tableItems[8][1]->setText(chipName);
    m_tableItems[8][2]->setText(QString::number(extData.gyro_x));
    m_tableItems[8][3]->setText(QString::number(extData.gyro_y));
    m_tableItems[8][4]->setText(QString::number(extData.gyro_z));
    m_tableItems[8][5]->setText(QString::number(extData.gyro_acc_x));
    m_tableItems[8][6]->setText(QString::number(extData.gyro_acc_y));
    m_tableItems[8][7]->setText(QString::number(extData.gyro_acc_z));
    m_tableItems[8][8]->setText(QString::number(extData.gyro_mix));
    m_tableItems[8][9]->setText(QString::number(extData.gyro_temperature / 100.0, 'f', 2));
}

void TestBoardControlWidget::onFrameSent(const QByteArray &frame)
{
    if (m_enableRawFrameLogging) {
        QMutexLocker locker(&m_logMutex);
        m_logBuffer.append(QString("[%1] > SENT: %2")
            .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
            .arg(QString(frame.toHex(' ').toUpper())));
    }
}

void TestBoardControlWidget::onFrameReceived(const QByteArray &frame)
{
    m_frameCounter++;
    
    if (m_enableRawFrameLogging) {
        QMutexLocker locker(&m_logMutex);
        // 限制缓冲区大小
        if (m_logBuffer.size() > 100) {
            m_logBuffer.removeFirst();
        }
        
        m_logBuffer.append(QString("[%1] < RECV: %2 bytes")
            .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
            .arg(frame.size()));
        m_logBuffer.append(QString("[%1] < RECV: %2")
            .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
            .arg(QString(frame.toHex(' ').toUpper())));
    } else if (m_frameCounter % 100 == 0) {
        // 即使禁用了原始帧日志，也定期显示统计信息
        QMutexLocker locker(&m_logMutex);
        m_logBuffer.append(QString("[%1] 已接收 %2 帧数据")
            .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
            .arg(m_frameCounter));
    }
}

void TestBoardControlWidget::onSendCalibration()
{
    if (!m_testBoard) return;
    QJsonObject params;
    params["dutSel"] = getSelectedDuts();
    params["command"] = m_calibCmdEdit->text();
    executeDeviceCommand("sendCalibration", params);
}

void TestBoardControlWidget::onWriteRegister()
{
    if (!m_testBoard) return;
    bool ok;
    int regAddr = m_regWriteAddrEdit->text().toInt(&ok, 16);
    if (!ok) {
        onDataError("无效的寄存器地址 (必须是16进制).");
        return;
    }

    QJsonObject params;
    params["dutSel"] = getSelectedDuts();
    params["regAddr"] = regAddr;
    params["data"] = m_regWriteDataEdit->text();
    executeDeviceCommand("writeRegister", params);
}

void TestBoardControlWidget::onReadRegister()
{
    if (!m_testBoard) return;
    bool ok_addr, ok_len;
    int regAddr = m_regReadAddrEdit->text().toInt(&ok_addr, 16);
    int length = m_regReadLengthEdit->text().toInt(&ok_len, 10);

    if (!ok_addr) {
        onDataError("无效的寄存器地址 (必须是16进制).");
        return;
    }
    if (!ok_len || length <= 0 || length > 255) {
        onDataError("无效的长度 (必须是 1-255).");
        return;
    }

    QJsonObject params;
    params["dutSel"] = getSelectedDuts();
    params["regAddr"] = regAddr;
    params["length"] = length;
    executeDeviceCommand("readRegister", params);
}

void TestBoardControlWidget::onCalibrationResult(const QJsonObject &result)
{
    QString resultString = QJsonDocument(result).toJson(QJsonDocument::Compact);
    QMutexLocker locker(&m_logMutex);
    m_logBuffer.append(QString("[%1] 标定反馈: %2")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
        .arg(resultString));
}

void TestBoardControlWidget::onRegisterWriteResult(const QJsonObject &result)
{
    QString resultString = QJsonDocument(result).toJson(QJsonDocument::Compact);
    QMutexLocker locker(&m_logMutex);
    m_logBuffer.append(QString("[%1] 寄存器写入反馈: %2")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
        .arg(resultString));
}

void TestBoardControlWidget::onRegisterReadResult(const QJsonObject &result)
{
    QString resultString = QJsonDocument(result).toJson(QJsonDocument::Compact);
    QMutexLocker locker(&m_logMutex);
    m_logBuffer.append(QString("[%1] 寄存器读取反馈: %2")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
        .arg(resultString));
}

void TestBoardControlWidget::onSetChipType()
{
    if (!m_testBoard) return;
    QJsonObject params;
    params["chipIndex"] = m_chipTypeComboBox->currentData().toInt();
    executeDeviceCommand("setChipType", params);
}

void TestBoardControlWidget::onChipTypeSetResult(const QJsonObject &result)
{
    QString resultString = QJsonDocument(result).toJson(QJsonDocument::Compact);
    QMutexLocker locker(&m_logMutex);
    m_logBuffer.append(QString("[%1] 设置芯片类型反馈: %2")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
        .arg(resultString));
    
    if (result["state"].toInt() == 1) {
        m_currentChipIndex = static_cast<uint8_t>(result["chipIndex"].toInt());
        m_logBuffer.append(QString("[%1] 芯片类型已成功设置为: %2")
            .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
            .arg(result["chipName"].toString()));
    }
}

void TestBoardControlWidget::onDataUpdateTimeout()
{
    LOG_MODULE_DEBUG("TestBoardUI", QString("[onDataUpdateTimeout] hasPendingData:%1").arg(m_hasPendingData).toStdString());
    if (m_hasPendingData) {
        updateDataTable(m_pendingData);
        m_hasPendingData = false;
    }
}

void TestBoardControlWidget::onLogUpdateTimeout()
{
    QMutexLocker locker(&m_logMutex);
    if (!m_logBuffer.isEmpty()) {
        // 批量更新日志，最多更新10条
        int count = qMin(m_logBuffer.size(), 10);
        for (int i = 0; i < count; ++i) {
            m_logTextEdit->append(m_logBuffer.takeFirst());
        }
        
        // 自动滚动到底部
        QTextCursor cursor = m_logTextEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        m_logTextEdit->setTextCursor(cursor);
    }
}

} // namespace Presentation
