#include "measureDialog.h"
#include "ui_measureDialog.h"

MeasurementDialog::MeasurementDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::MeasurementDialog) {
    ui->setupUi(this);

    // 设置样式表
    setStyleSheet(
        "QDialog {"
        "    background-color: #ffffff;"  // 白色背景
        "    color: #000000;"  // 黑色文字
        "}"
        "QGroupBox {"
        "    border: 1px solid #000000;"  // 黑色边框
        "    margin-top: 10px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    subcontrol-position: top left;"
        "    padding: 0 3px;"
        "    background-color: #ffffff;"  // 白色背景
        "    color: #000000;"  // 黑色文字
        "}"
        "QLabel {"
        "    color: #000000;"  // 黑色文字
        "}"
        "QLineEdit {"
        "    border: 1px solid #000000;"  // 黑色边框
        "    background-color: #f0f0f0;"  // 浅灰色背景
        "    color: #000000;"  // 黑色文字
        "}"
        "QPushButton {"
        "    background-color: #000000;"  // 黑色背景
        "    color: #ffffff;"  // 白色文字
        "    border: none;"
        "    padding: 5px 10px;"
        "    border-radius: 5px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #333333;"  // 深灰色背景
        "}"
        "QComboBox {"
        "    border: 1px solid #000000;"  // 黑色边框
        "    background-color: #f0f0f0;"  // 浅灰色背景
        "    color: #000000;"  // 黑色文字
        "}"
    );
    initCheckbox();
    connect(ui->checkBoxPinContinuity, &QCheckBox::toggled, this, &MeasurementDialog::handlePinContinuityTest);
    connect(ui->checkBoxPowerOnCurrent, &QCheckBox::toggled, this, &MeasurementDialog::handlePowerOnCurrentTest);
    connect(ui->checkBoxFrequencyScreening, &QCheckBox::toggled, this, &MeasurementDialog::handleFrequencyScreening);
    connect(ui->checkBoxScaleCalibration, &QCheckBox::toggled, this, &MeasurementDialog::handleScaleCalibration);
    connect(ui->checkBoxFunctionalSafetyCalibration, &QCheckBox::toggled, this, &MeasurementDialog::handleFunctionalSafetyCalibration);
    connect(ui->checkBoxWriteOTP, &QCheckBox::toggled, this, &MeasurementDialog::handleWriteOTP);
    connect(ui->checkBoxReadPhaseInfo, &QCheckBox::toggled, this, &MeasurementDialog::handlePSI5Test);
    connect(ui->checkBoxPinContinuity, &QCheckBox::toggled, this, &MeasurementDialog::updateCheckBoxStatus);
    connect(ui->checkBoxPowerOnCurrent, &QCheckBox::toggled, this, &MeasurementDialog::updateCheckBoxStatus);
    connect(ui->checkBoxFrequencyScreening, &QCheckBox::toggled, this, &MeasurementDialog::updateCheckBoxStatus);
    connect(ui->checkBoxScaleCalibration, &QCheckBox::toggled, this, &MeasurementDialog::updateCheckBoxStatus);
    connect(ui->checkBoxFunctionalSafetyCalibration, &QCheckBox::toggled, this, &MeasurementDialog::updateCheckBoxStatus);
    connect(ui->checkBoxWriteOTP, &QCheckBox::toggled, this, &MeasurementDialog::updateCheckBoxStatus);
    connect(ui->checkBoxReadPhaseInfo, &QCheckBox::toggled, this, &MeasurementDialog::updateCheckBoxStatus);
    connect(ui->checkBoxReadZeroNoise, &QCheckBox::toggled, this, &MeasurementDialog::updateCheckBoxStatus);
    connect(ui->checkBoxReadManchesterCodeCurrent, &QCheckBox::toggled, this, &MeasurementDialog::updateCheckBoxStatus);
    connect(ui->checkBoxMeasureBitTime, &QCheckBox::toggled, this, &MeasurementDialog::updateCheckBoxStatus);
    connect(ui->checkBoxMeasureTDelay, &QCheckBox::toggled, this, &MeasurementDialog::updateCheckBoxStatus);
    connect(ui->checkBoxSyncPulseVDD, &QCheckBox::toggled, this, &MeasurementDialog::updateCheckBoxStatus);
    connect(ui->checkBoxAsynchronousMode, &QCheckBox::toggled, this, &MeasurementDialog::updateCheckBoxStatus);
    connect(ui->checkBoxDualTimeSlotMode, &QCheckBox::toggled, this, &MeasurementDialog::updateCheckBoxStatus);
}
    // Add more connections as needed

MeasurementDialog::~MeasurementDialog() {
    delete ui;
}   

void MeasurementDialog::initCheckbox()
{
    ui->checkBoxPinContinuity->setChecked(true);
    ui->checkBoxPowerOnCurrent->setChecked(true);
    ui->checkBoxFrequencyScreening->setChecked(true);
    ui->checkBoxScaleCalibration->setChecked(true);
    ui->checkBoxFunctionalSafetyCalibration->setChecked(true);
    ui->checkBoxWriteOTP->setChecked(true);
    ui->checkBoxReadPhaseInfo->setChecked(true);
    ui->checkBoxReadZeroNoise->setChecked(true);
    ui->checkBoxReadManchesterCodeCurrent->setChecked(true);
    ui->checkBoxMeasureBitTime->setChecked(true);
    ui->checkBoxMeasureTDelay->setChecked(true);
    ui->checkBoxSyncPulseVDD->setChecked(true);
    ui->checkBoxAsynchronousMode->setChecked(true);
    ui->checkBoxDualTimeSlotMode->setChecked(true);
}

CheckBoxStatus MeasurementDialog::collectCheckBoxStatus()
{
    status.pinContinuityTest = ui->checkBoxPinContinuity->isChecked();
    status.powerOnCurrentTest = ui->checkBoxPowerOnCurrent->isChecked();
    status.frequencyScreening = ui->checkBoxFrequencyScreening->isChecked();
    status.scaleCalibration = ui->checkBoxScaleCalibration->isChecked();
    status.functionalSafetyCalibration = ui->checkBoxFunctionalSafetyCalibration->isChecked();
    status.writeOTP = ui->checkBoxWriteOTP->isChecked();
    status.readPhaseInfo = ui->checkBoxReadPhaseInfo->isChecked();
    status.readZeroNoise = ui->checkBoxReadZeroNoise->isChecked();
    status.readManchesterCodeCurrent = ui->checkBoxReadManchesterCodeCurrent->isChecked();
    status.measureBitTime = ui->checkBoxMeasureBitTime->isChecked();
    status.measureTDelay = ui->checkBoxMeasureTDelay->isChecked();
    status.syncPulseVDD = ui->checkBoxSyncPulseVDD->isChecked();
    status.asynchronousMode = ui->checkBoxAsynchronousMode->isChecked();
    status.dualTimeSlotMode = ui->checkBoxDualTimeSlotMode->isChecked();
    return status;
}


void MeasurementDialog::updateCheckBoxStatus()
{
    CheckBoxStatus status = collectCheckBoxStatus();
    // 在这里处理更新后的状态，例如发送给其他类
    // otherClass->receiveCheckBoxStatus(status);
}

void MeasurementDialog::handlePinContinuityTest()
{
    // 处理管脚通断测试的逻辑
}

void MeasurementDialog::handlePowerOnCurrentTest()
{
    // 处理上电电流测试的逻辑
}

void MeasurementDialog::handleFrequencyScreening()
{
    // 处理频率筛选的逻辑
}

void MeasurementDialog::handleScaleCalibration()
{
    // 处理标度标定的逻辑
}

void MeasurementDialog::handleFunctionalSafetyCalibration()
{
    // 处理功能安全标定的逻辑
}

void MeasurementDialog::handleWriteOTP()
{
    // 处理烧写OTP的逻辑
}

void MeasurementDialog::handlePSI5Test()
{
    // 处理PSI5下测试的逻辑
}