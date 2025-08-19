#include "acroViewTester.h"

void acroViewTester::initializeUdpInterface()
{
    ui.udpThermalModeComboBox->clear(); 
    ui.udpThermalModeComboBox->addItems({ "hold", "ramp", "none" });

    connect(m_udpCommunicator, &UdpCommunicator::deviceControlResponseReceived, this, &acroViewTester::handleUdpDeviceControlResponse);
    connect(m_udpCommunicator, &UdpCommunicator::functionTestResponseReceived, this, &acroViewTester::handleUdpFunctionTestResponse);
    connect(m_udpCommunicator, &UdpCommunicator::sensorDataReceived, this, &acroViewTester::handleUdpSensorDataReceived);
    connect(m_udpCommunicator, &UdpCommunicator::unknownPacketReceived, this, &acroViewTester::handleUdpUnknownPacketReceived);
    connect(m_udpCommunicator, &UdpCommunicator::bindError, this, &acroViewTester::handleUdpBindError);
    connect(m_udpCommunicator, &UdpCommunicator::sendError, this, &acroViewTester::handleUdpSendError);

    logToUdpInterface("UDP 通信模块已初始化。请在UDP通信接口标签页配置服务器。");
}

void acroViewTester::logToUdpInterface(const QString& message)
{
    if (ui.udpLogTextEdit) {
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        ui.udpLogTextEdit->append(QString("[%1] %2").arg(timestamp, message));
    }
    else {
        qDebug() << "[UDP Log - UI Element Missing]:" << message;
    }
}

void acroViewTester::on_udpApplyConfigButton_clicked()
{
    QString serverIp = ui.udpServerIpLineEdit->text().trimmed();
    bool portOk;
    quint16 serverPort = ui.udpServerPortLineEdit->text().toUShort(&portOk);
    if (serverIp.isEmpty() || !QHostAddress().setAddress(serverIp) || !portOk || serverPort == 0) {
        QMessageBox::warning(this, "UDP配置错误", "无效的服务器IP或端口。");
        logToUdpInterface("错误: 输入了无效的UDP服务器IP或端口。");
        return;
    }

    bool localPortOk;
    quint16 localPort = ui.udpLocalPortLineEdit->text().toUShort(&localPortOk);
    if (!localPortOk) {
        QMessageBox::warning(this, "UDP配置错误", "无效的本地绑定端口。");
        logToUdpInterface("错误: 输入了无效的本地UDP绑定端口。");
        return;
    }

    m_udpCommunicator->setTargetServer(QHostAddress(serverIp), serverPort);
    logToUdpInterface(QString("UDP目标服务器已设置为 %1:%2").arg(serverIp).arg(serverPort));

    if (!m_udpCommunicator->bind(localPort)) {
        logToUdpInterface(QString("UDP绑定本地端口 %1 失败。详情请查看控制台输出。")
            .arg(localPort == 0 ? "自动" : QString::number(localPort)));
        // bindError信号会被 handleUdpBindError 槽函数捕获
    }
    else {
        quint16 boundPort = m_udpCommunicator->getLocalPort(); // 使用新添加的方法
        logToUdpInterface(QString("UDP成功绑定到本地端口 %1。").arg(boundPort));
        if (ui.statusBar) { // 检查 statusBar 是否存在
            ui.statusBar->showMessage(QString("UDP已连接到 %1:%2 | 本地监听端口 %3")
                .arg(serverIp).arg(serverPort).arg(boundPort), 5000);
        }
    }
}

void acroViewTester::on_udpSendDeviceControlButton_clicked()
{
    if (m_udpCommunicator->getTargetServerAddress().isNull() || m_udpCommunicator->getTargetServerPort() == 0) {
        QMessageBox::warning(this, "UDP未配置", "请先在UDP通信接口标签页应用服务器设置。");
        logToUdpInterface("错误: 尝试发送设备控制命令前未配置UDP服务器。");
        return;
    }

    QString mode = ui.udpThermalModeComboBox->currentText();
    bool tempOk, timeOk;
    double targetTemp = ui.udpTargetTempLineEdit->text().toDouble(&tempOk);
    int stableTime = ui.udpStableTimeLineEdit->text().toInt(&timeOk);

    if (!tempOk || !timeOk) {
        QMessageBox::warning(this, "UDP输入错误", "设备控制命令的目标温度或稳定时间无效。");
        logToUdpInterface("错误: 设备控制命令的输入参数无效。");
        return;
    }

    logToUdpInterface(QString("发送UDP设备控制命令: 模式=%1, 温度=%2, 时间=%3")
        .arg(mode).arg(targetTemp).arg(stableTime));
    double rampRate = 0.0; // 默认斜率值，也可以从UI获取（如果添加了相应控件）
    m_udpCommunicator->sendDeviceControl(mode, targetTemp, stableTime, rampRate);
}

void acroViewTester::on_udpSendDataAcqConfigButton_clicked()
{
    if (m_udpCommunicator->getTargetServerAddress().isNull() || m_udpCommunicator->getTargetServerPort() == 0) {
        QMessageBox::warning(this, "UDP未配置", "请先在UDP通信接口标签页应用服务器设置。");
        logToUdpInterface("错误: 尝试发送数据采集配置前未配置UDP服务器。");
        return;
    }

    bool enabled = ui.udpEnableAcqCheckBox->isChecked();
    bool durationOk, intervalOk;
    int duration = ui.udpDurationLineEdit->text().toInt(&durationOk);
    int interval = ui.udpIntervalLineEdit->text().toInt(&intervalOk);

    if (!durationOk || !intervalOk || duration < 0 || interval <= 0) {
        QMessageBox::warning(this, "UDP输入错误", "数据采集配置的持续时间或间隔无效。");
        logToUdpInterface("错误: 数据采集配置的输入参数无效。");
        return;
    }
    logToUdpInterface(QString("发送UDP数据采集配置: 使能=%1, 持续时间=%2s, 间隔=%3ms")
        .arg(enabled ? "是" : "否").arg(duration).arg(interval));
    m_udpCommunicator->sendDataAcquisitionConfig(enabled, duration, interval);
}

void acroViewTester::handleUdpDeviceControlResponse(int code, int timestamp, const QHostAddress& senderAddress, quint16 senderPort)
{
    QString log = QString("UDP设备控制响应 from %1:%2 - Code: %3, Timestamp: %4")
        .arg(senderAddress.toString()).arg(senderPort)
        .arg(code).arg(timestamp);
    logToUdpInterface(log);
    if (ui.statusBar) {
        ui.statusBar->showMessage(QString("UDP设备控制响应: Code %1").arg(code), 3000);
    }
}

void acroViewTester::handleUdpFunctionTestResponse(int code, int timestamp, const QHostAddress& senderAddress, quint16 senderPort)
{
    QString log = QString("UDP功能测试响应 from %1:%2 - Code: %3, Timestamp: %4")
        .arg(senderAddress.toString()).arg(senderPort)
        .arg(code).arg(timestamp);
    logToUdpInterface(log);
    // 根据需要更新UI或状态
}

void acroViewTester::handleUdpSensorDataReceived(const CommandBuilder::SensorData& sensorData, const QHostAddress& senderAddress, quint16 senderPort)
{
    QString log = QString("UDP传感器数据 from %1:%2 - Temp: %3, Voltage: %4, Current: %5")
        .arg(senderAddress.toString()).arg(senderPort)
        .arg(sensorData.temperature)
        .arg(sensorData.voltage)
        .arg(sensorData.current);
    // 根据需要添加其他 sensorData 字段
    logToUdpInterface(log);
    if (ui.statusBar) {
        ui.statusBar->showMessage(QString("UDP传感器数据: Temp %1°C").arg(sensorData.temperature), 3000);
    }
}

void acroViewTester::handleUdpUnknownPacketReceived(quint16 cmdID, const QJsonObject& jsonData, const QHostAddress& senderAddress, quint16 senderPort)
{
    QString jsonDataStr = QJsonDocument(jsonData).toJson(QJsonDocument::Indented);
    if (jsonDataStr.length() > 200) { // 避免日志过长
        jsonDataStr = jsonDataStr.left(200) + "...";
    }
    QString log = QString("UDP未知包 from %1:%2 - CmdID: 0x%3, Data: %4")
        .arg(senderAddress.toString()).arg(senderPort)
        .arg(QString::number(cmdID, 16).toUpper())
        .arg(jsonDataStr);
    logToUdpInterface(log);
}

void acroViewTester::handleUdpBindError(const QString& errorString)
{
    QString log = QString("UDP绑定错误: %1").arg(errorString);
    logToUdpInterface(log);
    QMessageBox::critical(this, "UDP绑定错误", "UDP套接字绑定失败: " + errorString);
}

void acroViewTester::handleUdpSendError(const QString& errorString)
{
    QString log = QString("UDP发送错误: %1").arg(errorString);
    logToUdpInterface(log);
    // 可以选择性地显示一个非致命错误消息或状态栏更新
}