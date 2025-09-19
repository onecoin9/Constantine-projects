#include "ui/TestBoardControlWidget.h"
#include "ui_TestBoardControlWidget.h" // 包含自动生成的头文件
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
#include <QProgressBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QSignalBlocker> // <--- 新增
#include "core/Logger.h"
#include "utils/ByteUtils.h" // 引入新的工具类

namespace Presentation {

// 移除本地的辅助函数
// static uint16_t readLittleEndian_uint16(const uint8_t* data)
// {
//     return static_cast<uint16_t>(data[0]) | (static_cast<uint16_t>(data[1]) << 8);
// }

TestBoardControlWidget::TestBoardControlWidget(QWidget *parent)
    : IDeviceControlWidget(parent)
    , ui(new Ui::TestBoardControlWidget) // 创建UI实例
    , m_testBoard(nullptr)
    , m_updateTimer(new QTimer(this))
    , m_dataUpdateTimer(new QTimer(this))
    , m_isAcquiring(false)
    , m_currentChipIndex(1)  // 默认为A300
    , m_hasPendingData(false)
    , m_frameCounter(0)
{
    ui->setupUi(this); // 加载UI定义
    setupUiLogic();     // 执行剩余的逻辑设置
    
    connectSignals();

    m_updateTimer->setInterval(1000); // 1s a time
    connect(m_updateTimer, &QTimer::timeout, this, &TestBoardControlWidget::onUpdateTimer);
    
    // 设置数据更新定时器（降低更新频率）
    m_dataUpdateTimer->setInterval(100); // 100ms更新一次，即10Hz
    connect(m_dataUpdateTimer, &QTimer::timeout, this, &TestBoardControlWidget::onDataUpdateTimeout);

    setControlsEnabled(false);
}

TestBoardControlWidget::~TestBoardControlWidget()
{
    m_updateTimer->stop();
    m_dataUpdateTimer->stop();
    disconnectSignals();
    
    // 清理表格项缓存
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 10; ++j) {
            delete m_tableItems[i][j];
        }
    }

    delete ui; // 释放UI
}

QString TestBoardControlWidget::getDeviceTypeName() const
{
    return "测试板控制";
}

void TestBoardControlWidget::setupUiLogic()
{
    // 将数组中的CheckBox和Label指针与UI中的控件对应起来
    // DUTs
    for(int i = 0; i < 8; ++i) {
        m_dutCheckBoxes[i] = new QCheckBox(QString("DUT %1").arg(i+1));
        m_powerStatusLabels[i] = new QLabel("OFF");
        m_powerStatusLabels[i]->setStyleSheet("color: red; font-weight: bold;");

        int row = i / 4;
        int col = i % 4;
        ui->dutsGridLayout->addWidget(m_dutCheckBoxes[i], row, col * 2);
        ui->dutsGridLayout->addWidget(m_powerStatusLabels[i], row, col * 2 + 1);
    }
    // Main Power Indicator
    m_powerStatusLabels[8] = ui->mainPowerStatusIndicator;
    
    // 初始化芯片类型下拉框
    ui->chipTypeComboBox->addItem("A300", 1);
    ui->chipTypeComboBox->addItem("G300", 2);
    ui->chipTypeComboBox->addItem("270", 3);
    
    // 初始化表格
    ui->dataTable->setRowCount(9);
    QStringList verticalLabels;
    for(int i=0; i<8; ++i) verticalLabels << QString("DUT %1").arg(i+1);
    verticalLabels << "External";
    ui->dataTable->setVerticalHeaderLabels(verticalLabels);

    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 10; ++j) {
            m_tableItems[i][j] = new QTableWidgetItem();
            ui->dataTable->setItem(i, j, m_tableItems[i][j]);
        }
    }
    

}

void TestBoardControlWidget::connectSignals()
{
    // 关键修复：在连接前先断开，确保信号槽连接唯一
    disconnect(ui->startButton, &QPushButton::clicked, this, &TestBoardControlWidget::onStartAcquisition);
    connect(ui->startButton, &QPushButton::clicked, this, &TestBoardControlWidget::onStartAcquisition);

    disconnect(ui->stopButton, &QPushButton::clicked, this, &TestBoardControlWidget::onStopAcquisition);
    connect(ui->stopButton, &QPushButton::clicked, this, &TestBoardControlWidget::onStopAcquisition);

    disconnect(ui->powerOnButton, &QPushButton::clicked, this, &TestBoardControlWidget::onDutPowerOn);
    connect(ui->powerOnButton, &QPushButton::clicked, this, &TestBoardControlWidget::onDutPowerOn);

    disconnect(ui->powerOffButton, &QPushButton::clicked, this, &TestBoardControlWidget::onDutPowerOff);
    connect(ui->powerOffButton, &QPushButton::clicked, this, &TestBoardControlWidget::onDutPowerOff);

    disconnect(ui->queryVoltageButton, &QPushButton::clicked, this, &TestBoardControlWidget::onQueryVoltage);
    connect(ui->queryVoltageButton, &QPushButton::clicked, this, &TestBoardControlWidget::onQueryVoltage);

    disconnect(ui->queryFaultButton, &QPushButton::clicked, this, &TestBoardControlWidget::onQueryFault);
    connect(ui->queryFaultButton, &QPushButton::clicked, this, &TestBoardControlWidget::onQueryFault);

    disconnect(ui->selectAllCheckBox, &QCheckBox::stateChanged, this, &TestBoardControlWidget::onDutSelectionChanged);
    connect(ui->selectAllCheckBox, &QCheckBox::stateChanged, this, &TestBoardControlWidget::onDutSelectionChanged);

    disconnect(ui->mainPowerCheckBox, &QCheckBox::stateChanged, this, &TestBoardControlWidget::onDutSelectionChanged);
    connect(ui->mainPowerCheckBox, &QCheckBox::stateChanged, this, &TestBoardControlWidget::onDutSelectionChanged); // 连接主电源复选框

    disconnect(ui->setChipTypeButton, &QPushButton::clicked, this, &TestBoardControlWidget::onSetChipType);
    connect(ui->setChipTypeButton, &QPushButton::clicked, this, &TestBoardControlWidget::onSetChipType);

    disconnect(ui->calibSendButton, &QPushButton::clicked, this, &TestBoardControlWidget::onSendCalibration);
    connect(ui->calibSendButton, &QPushButton::clicked, this, &TestBoardControlWidget::onSendCalibration);

    disconnect(ui->regWriteSendButton, &QPushButton::clicked, this, &TestBoardControlWidget::onWriteRegister);
    connect(ui->regWriteSendButton, &QPushButton::clicked, this, &TestBoardControlWidget::onWriteRegister);

    disconnect(ui->regReadSendButton, &QPushButton::clicked, this, &TestBoardControlWidget::onReadRegister);
    connect(ui->regReadSendButton, &QPushButton::clicked, this, &TestBoardControlWidget::onReadRegister);

    disconnect(ui->parseBinFileButton, &QPushButton::clicked, this, &TestBoardControlWidget::onParseBinFile);
    connect(ui->parseBinFileButton, &QPushButton::clicked, this, &TestBoardControlWidget::onParseBinFile);
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
        // 获取设备名称
        m_deviceName = m_testBoard->getName();
        if (m_deviceName.isEmpty()) {
            m_deviceName = "TestBoard";
        }
        
        // 移除对废弃功能的调用
        // TesterFramework::Logger::getInstance().setCurrentDeviceContext(m_deviceName.toStdString());
        
        LOG_INFO("设备控制界面已连接");
        
        connect(m_testBoard.get(), &Domain::TestBoardDevice::testStateChanged, this, &TestBoardControlWidget::onTestStateChanged, Qt::QueuedConnection);
        connect(m_testBoard.get(), &Domain::TestBoardDevice::powerStateChanged, this, &TestBoardControlWidget::onPowerStateChanged, Qt::QueuedConnection);
        connect(m_testBoard.get(), &Domain::TestBoardDevice::voltageDataReceived, this, &TestBoardControlWidget::onVoltageDataReceived, Qt::QueuedConnection);
        connect(m_testBoard.get(), &Domain::TestBoardDevice::faultStatusReceived, this, &TestBoardControlWidget::onFaultStatusReceived, Qt::QueuedConnection);
        
        // 连接到设备发出的原始数据帧信号
        connect(m_testBoard.get(), &Domain::IDevice::rawFrameSent, this, &TestBoardControlWidget::onFrameSent, Qt::QueuedConnection);
        connect(m_testBoard.get(), &Domain::IDevice::rawFrameReceived, this, &TestBoardControlWidget::onFrameReceived, Qt::QueuedConnection);

        connect(m_testBoard.get(), &Domain::TestBoardDevice::dataError, this, &TestBoardControlWidget::onDataError, Qt::QueuedConnection);

        connect(m_testBoard.get(), &Domain::TestBoardDevice::calibrationResultReceived, this, &TestBoardControlWidget::onCalibrationResult, Qt::QueuedConnection);
        connect(m_testBoard.get(), &Domain::TestBoardDevice::registerWriteResultReceived, this, &TestBoardControlWidget::onRegisterWriteResult, Qt::QueuedConnection);
        connect(m_testBoard.get(), &Domain::TestBoardDevice::registerReadResultReceived, this, &TestBoardControlWidget::onRegisterReadResult, Qt::QueuedConnection);
        connect(m_testBoard.get(), &Domain::TestBoardDevice::chipTypeSetResultReceived, this, &TestBoardControlWidget::onChipTypeSetResult, Qt::QueuedConnection);

        // 正确设置初始UI状态
        updateStatus();
        setControlsEnabled(true); // 确保辅助控件在初始时是可用的
        m_updateTimer->start();
    } else {
        m_deviceName.clear();
        
        // 移除对废弃功能的调用
        // TesterFramework::Logger::getInstance().clearCurrentDeviceContext();
        
        setControlsEnabled(false);
        ui->statusEdit->clear();
        ui->serialNumberEdit->clear();
        ui->errorLabel->clear();
        ui->dataTable->clearContents();
        // Reset labels on disconnect
        ui->mainPowerStatusLabel->setText("未知");
        ui->mainPowerStatusLabel->setStyleSheet("");
        for(int i=0; i<9; ++i) {
            if(m_powerStatusLabels[i]) {
                m_powerStatusLabels[i]->setText("OFF");
                m_powerStatusLabels[i]->setStyleSheet("color: red; font-weight: bold;");
            }
        }
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
    ui->statusEdit->setText(statusText);
    
    // 这个函数现在只负责根据 m_isAcquiring 更新核心控制按钮
    ui->startButton->setEnabled(isConnected && !m_isAcquiring);
    ui->stopButton->setEnabled(isConnected && m_isAcquiring);
    
    // 辅助按钮的启用/禁用由 onTestStateChanged 统一管理
}

void TestBoardControlWidget::setControlsEnabled(bool enabled)
{
    // 这个函数现在负责所有"非核心"的辅助控件
    ui->powerOnButton->setEnabled(enabled);
    ui->powerOffButton->setEnabled(enabled);
    ui->queryVoltageButton->setEnabled(enabled);
    ui->queryFaultButton->setEnabled(enabled);
    ui->selectAllCheckBox->setEnabled(enabled);
    for(auto checkbox : m_dutCheckBoxes) {
        checkbox->setEnabled(enabled);
    }
    ui->chipTypeComboBox->setEnabled(enabled);
    ui->setChipTypeButton->setEnabled(enabled);
    ui->calibCmdEdit->setEnabled(enabled);
    ui->calibSendButton->setEnabled(enabled);
    ui->regWriteAddrEdit->setEnabled(enabled);
    ui->regWriteDataEdit->setEnabled(enabled);
    ui->regWriteSendButton->setEnabled(enabled);
    ui->regReadAddrEdit->setEnabled(enabled);
    ui->regReadLengthEdit->setEnabled(enabled);
    ui->regReadSendButton->setEnabled(enabled);
    ui->mainPowerCheckBox->setEnabled(enabled); // 确保主电源复选框也被控制
}

uint16_t TestBoardControlWidget::getSelectedDuts() const
{
    uint16_t selection = 0;
    // 处理DUT 1-8 (对应 bit 0-7)
    for (int i = 0; i < 8; ++i) {
        if (m_dutCheckBoxes[i]->isChecked()) {
            selection |= (1 << i);
        }
    }

    // 处理主电源 (313), 对应 bit 15
    if (ui->mainPowerCheckBox->isChecked()) {
        selection |= (1 << 15);
    }
    
    LOG_DEBUG(QString("DUT选择掩码: 0x%1")
             .arg(QString::number(selection, 16).toUpper().rightJustified(4, '0')).toStdString());

    return selection;
}

void TestBoardControlWidget::executeDeviceCommand(const QString& command, const QJsonObject& params)
{
    if (!m_testBoard) return;

    // 关键修复：命令发出后，禁用所有辅助按钮防止重入
    setControlsEnabled(false);

    LOG_INFO(QString("执行命令: %1, 参数: %2")
             .arg(command)
             .arg(QString(QJsonDocument(params).toJson(QJsonDocument::Compact))).toStdString());

    auto watcher = new QFutureWatcher<QJsonObject>(this);
    connect(watcher, &QFutureWatcher<QJsonObject>::finished, this, [this, watcher, command]() {
        QJsonObject result = watcher->result();
        
        // 关键修复：无论成功或失败，都在命令结束后恢复UI
        updateStatus(); // 根据最新状态更新UI
        setControlsEnabled(!m_isAcquiring); // 如果不在采集中，则启用所有控件

        if (!result.value("success").toBool()) {
            QString error = result.value("error").toString("Unknown error");
            onDataError(error);
            
            LOG_ERROR(QString("命令执行失败: %1, 错误: %2").arg(command).arg(error).toStdString());
        } else {
            // 命令成功发送，我们什么都不做，等待设备状态信号来更新UI
            LOG_INFO(QString("命令执行成功: %1").arg(command).toStdString());
        }
        watcher->deleteLater();
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
    bool all_checked = ui->selectAllCheckBox->isChecked();
    for(auto checkbox : m_dutCheckBoxes) {
        checkbox->setChecked(all_checked);
    }
    ui->mainPowerCheckBox->setChecked(all_checked); // 确保主电源复选框也同步
}

void TestBoardControlWidget::onUpdateTimer()
{
    updateStatus();
}

void TestBoardControlWidget::onDataUpdateTimeout()
{
    if (!m_testBoard || !m_isAcquiring) {
        return;
    }

    // 从设备获取最新的缓存数据
    auto data = m_testBoard->getLatestData();
    
    // 用这份最新数据更新UI
    ui->serialNumberEdit->setText(QString::number(data.serialNumber));
    m_currentChipIndex = data.chipIndex;
    updateDataTable(data);
}

void TestBoardControlWidget::onTestStateChanged(bool isRunning)
{
    m_isAcquiring = isRunning;
    
    // 状态信号是唯一可信源，用它来统一更新整个UI
    updateStatus(); 
    setControlsEnabled(!isRunning); // 采集中禁用辅助按钮，停止后启用

    // 根据采集状态控制数据更新定时器
    if (!isRunning) {
        m_dataUpdateTimer->stop();
    } else {
        // 开始采集时，确保定时器在运行
        if (!m_dataUpdateTimer->isActive()) {
            m_dataUpdateTimer->start();
        }
    }
    
    LOG_INFO(QString("[%1] 测试状态: %2").arg(m_deviceName).arg(isRunning ? "运行中" : "已停止").toStdString());
}

void TestBoardControlWidget::onPowerStateChanged(const Domain::Protocols::PowerFeedbackData &data)
{
    // 强制调试日志：确认方法被调用
    LOG_INFO(QString("DEBUG: onPowerStateChanged called - State: 0x%1, DUT Mask: 0x%2")
        .arg(QString::number(data.state, 16))
        .arg(QString::number(data.dutPowerState, 16).toUpper().rightJustified(4, '0')).toStdString());
    
    uint16_t dutPowerState = data.dutPowerState;

    // 根据反馈的16位掩码，更新所有9个复选框和状态标签
    // 1. 更新主电源 (bit 15)
    bool mainPowerIsOn = (dutPowerState & 0x8000) != 0;
    {
        // 关键修复：临时阻塞信号，避免触发onDutSelectionChanged
        const QSignalBlocker blocker(ui->mainPowerCheckBox);
        ui->mainPowerCheckBox->setChecked(mainPowerIsOn);
    }
    ui->mainPowerStatusLabel->setText(mainPowerIsOn ? "ON" : "OFF");
    ui->mainPowerStatusLabel->setStyleSheet(mainPowerIsOn ? "color: green;" : "color: red;");
    m_powerStatusLabels[8]->setText(mainPowerIsOn ? "ON" : "OFF");
    m_powerStatusLabels[8]->setStyleSheet(mainPowerIsOn ? "color: green; font-weight: bold;" : "color: red; font-weight: bold;");

    // 2. 更新 8个 DUT (bit 0-7)
    QStringList dutStatusList;
    for (int i = 0; i < 8; ++i) {
        bool dutIsOn = (dutPowerState & (1 << i)) != 0;
        {
            // 关键修复：同样阻塞DUT复选框的信号
            const QSignalBlocker blocker(m_dutCheckBoxes[i]);
            m_dutCheckBoxes[i]->setChecked(dutIsOn);
        }
        m_powerStatusLabels[i]->setText(dutIsOn ? "ON" : "OFF");
        m_powerStatusLabels[i]->setStyleSheet(dutIsOn ? "color: green; font-weight: bold;" : "color: red; font-weight: bold;");
        dutStatusList.append(QString("DUT%1: %2").arg(i + 1).arg(dutIsOn ? "ON" : "OFF"));
    }

    LOG_INFO(QString("反馈状态--主电源(313): %1, DUTs: 0x%2")
        .arg(mainPowerIsOn ? "ON" : "OFF")
        .arg(QString::number(dutPowerState, 16).toUpper().rightJustified(4, '0')).toStdString());
    
    LOG_INFO(QString("各DUT电源详情: %1").arg(dutStatusList.join(", ")).toStdString());

    // 强制UI刷新
    update();
}

void TestBoardControlWidget::onVoltageDataReceived(const QJsonObject &voltageData)
{
    QString voltageString = QJsonDocument(voltageData).toJson(QJsonDocument::Compact);
    LOG_DEBUG(QString("电压数据: %1").arg(voltageString).toStdString());
}

void TestBoardControlWidget::onFaultStatusReceived(uint32_t faultState)
{
    LOG_DEBUG(QString("故障码: 0x%1").arg(QString::number(faultState, 16)).toStdString());
    if (m_testBoard && m_testBoard->getProtocol()) {
         LOG_INFO(QString("Received Fault Feedback: State=0x%1")
            .arg(QString::number(faultState, 16).toUpper().rightJustified(8, '0')).toStdString());
    }
}

void TestBoardControlWidget::onDataError(const QString &error)
{
    ui->errorLabel->setText(error);
    LOG_ERROR(QString("错误: %1").arg(error).toStdString());
    emit errorOccurred(error);
}

void TestBoardControlWidget::updateDataTable(const Domain::Protocols::TestFeedbackData &data)
{
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
            m_tableItems[i][0]->setText("Active");
            m_tableItems[i][1]->setText(chipName);
            m_tableItems[i][2]->setText(QString::number(dutData.gyro_x));
            m_tableItems[i][3]->setText(QString::number(dutData.gyro_y));
            m_tableItems[i][4]->setText(QString::number(dutData.gyro_z));
            m_tableItems[i][5]->setText(QString::number(dutData.gyro_acc_x));
            m_tableItems[i][6]->setText(QString::number(dutData.gyro_acc_y));
            m_tableItems[i][7]->setText(QString::number(dutData.gyro_acc_z));
            m_tableItems[i][8]->setText(QString::number(dutData.gyro_mix));
            m_tableItems[i][9]->setText(QString::number(dutData.gyro_temperature, 'f', 2));
        } else {
            for (int j = 0; j < 10; ++j) {
                m_tableItems[i][j]->setText("N/A");
            }
        }
    }

    const auto& extData = data.externalGyro;
    m_tableItems[8][0]->setText("Active");
    m_tableItems[8][1]->setText(chipName);
    m_tableItems[8][2]->setText(QString::number(extData.gyro_x));
    m_tableItems[8][3]->setText(QString::number(extData.gyro_y));
    m_tableItems[8][4]->setText(QString::number(extData.gyro_z));
    m_tableItems[8][5]->setText(QString::number(extData.gyro_acc_x));
    m_tableItems[8][6]->setText(QString::number(extData.gyro_acc_y));
    m_tableItems[8][7]->setText(QString::number(extData.gyro_acc_z));
    m_tableItems[8][8]->setText(QString::number(extData.gyro_mix));
    m_tableItems[8][9]->setText(QString::number(extData.gyro_temperature, 'f', 2));
}

void TestBoardControlWidget::onFrameSent(const QByteArray &frame)
{
    if (!m_testBoard || !m_testBoard->getProtocol()) return;

    // 对于所有发送的命令，都使用详细的格式化函数
    QString formattedLog = m_testBoard->getProtocol()->formatSentPacketForDisplay(frame);
    
    LOG_INFO(formattedLog.toStdString());
}

void TestBoardControlWidget::onFrameReceived(const QByteArray &frame)
{
    if (!m_testBoard || !m_testBoard->getProtocol() || frame.size() < 8) return;

    // 从原始包中解析出CmdID来判断是否是高频测试数据
    const uint8_t* raw_data = reinterpret_cast<const uint8_t*>(frame.constData());
    uint16_t cmdId = Utils::ByteUtils::readLittleEndian<uint16_t>(raw_data + 4);

    if (cmdId == 0x8001) { // 0x8001是测试反馈命令ID
        m_frameCounter++;
        // 对于高频测试数据，完全跳过日志处理
        return;
    } else {
        // 对于所有其他命令，都打印详细的解析日志
        QString formattedLog = m_testBoard->getProtocol()->formatPacketForDisplay(frame);
        LOG_INFO(formattedLog.toStdString());
    }
}

void TestBoardControlWidget::onSendCalibration()
{
    if (!m_testBoard) return;
    QJsonObject params;
    params["dutSel"] = getSelectedDuts();
    params["command"] = ui->calibCmdEdit->text();
    executeDeviceCommand("sendCalibration", params);
}

void TestBoardControlWidget::onWriteRegister()
{
    if (!m_testBoard) return;
    bool ok;
    int regAddr = ui->regWriteAddrEdit->text().toInt(&ok, 16);
    if (!ok) {
        onDataError("无效的寄存器地址 (必须是16进制).");
        return;
    }

    QJsonObject params;
    params["dutSel"] = getSelectedDuts();
    params["regAddr"] = regAddr;
    params["data"] = ui->regWriteDataEdit->text();
    executeDeviceCommand("writeRegister", params);
}

void TestBoardControlWidget::onReadRegister()
{
    if (!m_testBoard) return;
    bool ok_addr, ok_len;
    int regAddr = ui->regReadAddrEdit->text().toInt(&ok_addr, 16);
    int length = ui->regReadLengthEdit->text().toInt(&ok_len, 10);

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
    LOG_DEBUG(QString("标定反馈: %1").arg(resultString).toStdString());
}

void TestBoardControlWidget::onRegisterWriteResult(const QJsonObject &result)
{
    QString resultString = QJsonDocument(result).toJson(QJsonDocument::Compact);
    LOG_DEBUG(QString("寄存器写入反馈: %1").arg(resultString).toStdString());
}

void TestBoardControlWidget::onRegisterReadResult(const QJsonObject &result)
{
    QString resultString = QJsonDocument(result).toJson(QJsonDocument::Compact);
    LOG_DEBUG(QString("寄存器读取反馈: %1").arg(resultString).toStdString());
    LOG_INFO(QString("Received Register Read Result: %1").arg(resultString).toStdString());
}

void TestBoardControlWidget::onSetChipType()
{
    if (!m_testBoard) return;
    QJsonObject params;
    params["chipIndex"] = ui->chipTypeComboBox->currentData().toInt();
    executeDeviceCommand("setChipType", params);
}

void TestBoardControlWidget::onChipTypeSetResult(const QJsonObject &result)
{
    QString resultString = QJsonDocument(result).toJson(QJsonDocument::Compact);
    LOG_DEBUG(QString("设置芯片类型反馈: %1").arg(resultString).toStdString());
    LOG_INFO(QString("Received Chip Type Set Result: %1").arg(resultString).toStdString());
    
    if (result["state"].toInt() == 1) {
        m_currentChipIndex = static_cast<uint8_t>(result["chipIndex"].toInt());
        LOG_INFO(QString("芯片类型已成功设置为: %1").arg(result["chipName"].toString()).toStdString());
    }
}

void TestBoardControlWidget::onParseBinFile()
{
    // 使用异步方式调用文件对话框，避免在主线程中可能触发的死锁问题
    QTimer::singleShot(0, this, [this]() {
        try {
            QString binFilePath = QFileDialog::getOpenFileName(this, "选择要解析的Bin文件", "cache", "Binary files (*.bin)");

            if (binFilePath.isEmpty()) {
                LOG_INFO("用户取消了Bin文件选择");
                return;
            }

            LOG_INFO(QString("选择的Bin文件路径: %1").arg(binFilePath).toStdString());
            
            // 继续执行文件解析逻辑
            parseBinFileInternal(binFilePath);
        } catch (const std::exception& e) {
            LOG_ERROR(QString("选择Bin文件时发生异常: %1").arg(e.what()).toStdString());
            QMessageBox::critical(this, "错误", QString("选择文件时发生错误: %1").arg(e.what()));
        } catch (...) {
            LOG_ERROR("选择Bin文件时发生未知异常");
            QMessageBox::critical(this, "错误", "选择文件时发生未知错误");
        }
    });
}

void TestBoardControlWidget::parseBinFileInternal(const QString& binFilePath)
{

    QString txtFilePath = binFilePath;
    txtFilePath.replace(".bin", ".txt");

    ui->progressBar->setVisible(true);
    ui->progressBar->setValue(0);
    setControlsEnabled(false); // 禁用其他控件

    auto watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, txtFilePath]() {
        bool success = watcher->result();
        ui->progressBar->setVisible(false);
        setControlsEnabled(true);
        if (success) {
            QMessageBox::information(this, "成功", QString("文件解析成功！已保存至:\n%1").arg(txtFilePath));
        } else {
            QMessageBox::critical(this, "失败", "文件解析失败，请查看日志获取详情。");
        }
        watcher->deleteLater();
    });

    // 使用信号槽更新进度条，确保线程安全
    connect(this, &TestBoardControlWidget::parseProgressUpdated, ui->progressBar, &QProgressBar::setValue, Qt::QueuedConnection);

    // 在后台线程执行解析
    auto future = QtConcurrent::run([this, binFilePath, txtFilePath]() -> bool {
        QFile binFile(binFilePath);
        if (!binFile.open(QIODevice::ReadOnly)) {
            LOG_ERROR(QString("无法打开bin文件: %1").arg(binFilePath).toStdString());
            return false;
        }

        QFile txtFile(txtFilePath);
        if (!txtFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            LOG_ERROR(QString("无法创建txt文件: %1").arg(txtFilePath).toStdString());
            return false;
        }

        QDataStream binStream(&binFile);
        QTextStream txtStream(&txtFile);
        qint64 totalSize = binFile.size();
        qint64 readSize = 0;

        // 获取协议对象以调用格式化方法
        // 注意：m_testBoard和其m_protocol可能在不同线程，但我们的format函数是const的，所以是安全的
        auto protocol = m_testBoard ? m_testBoard->getProtocol() : nullptr;
        if (!protocol) {
            LOG_ERROR("无法获取协议对象，无法解析文件");
            return false;
        }

        while (!binStream.atEnd()) {
            qint64 packetSize;
            if (binStream.readRawData(reinterpret_cast<char*>(&packetSize), sizeof(qint64)) != sizeof(qint64)) {
                break; // 文件末尾或读取错误
            }

            QByteArray packet(packetSize, Qt::Uninitialized);
            if (binStream.readRawData(packet.data(), packetSize) != packetSize) {
                break; // 文件末尾或读取错误
            }

            // --- 调用统一的格式化函数 ---
            // 直接传递原始包，让Protocol层自己解析
            QString formattedDetails = protocol->formatPacketForDisplay(packet);
            txtStream << formattedDetails;

            readSize += sizeof(qint64) + packetSize;
            int progress = static_cast<int>((readSize * 100) / totalSize);
            emit parseProgressUpdated(progress);
        }
        
        emit parseProgressUpdated(100);
        return true;
    });

    watcher->setFuture(future);
}

} // namespace Presentation
