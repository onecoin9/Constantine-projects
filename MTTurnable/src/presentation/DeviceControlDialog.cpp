#include "presentation/DeviceControlDialog.h"
#include "core/CoreEngine.h"
#include "core/Logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QGroupBox>
#include <QJsonDocument>

namespace Presentation {

DeviceControlDialog::DeviceControlDialog(std::shared_ptr<Core::CoreEngine> coreEngine, QWidget *parent)
    : QDialog(parent)
    , m_coreEngine(coreEngine)
{
    setupUi();
    setWindowTitle(tr("设备手动控制"));
}

DeviceControlDialog::~DeviceControlDialog()
{
}

void DeviceControlDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Turntable Control Group
    QGroupBox *turntableGroup = new QGroupBox(tr("转台控制"));
    QFormLayout *turntableLayout = new QFormLayout();

    m_turntableIdInput = new QLineEdit("turntable_1");
    m_turntableAngleInput = new QLineEdit("0");
    m_turntableMoveButton = new QPushButton(tr("移动到角度"));

    turntableLayout->addRow(new QLabel(tr("转台ID:")), m_turntableIdInput);
    turntableLayout->addRow(new QLabel(tr("目标角度:")), m_turntableAngleInput);
    turntableLayout->addRow(m_turntableMoveButton);
    turntableGroup->setLayout(turntableLayout);
    mainLayout->addWidget(turntableGroup);

    // Test Board Control Group
    QGroupBox *testBoardGroup = new QGroupBox(tr("测试板控制"));
    QFormLayout *testBoardLayout = new QFormLayout();

    m_testBoardIdInput = new QLineEdit("testboard_1");
    m_startAcqButton = new QPushButton(tr("开始采集"));
    m_stopAcqButton = new QPushButton(tr("停止采集"));

    testBoardLayout->addRow(new QLabel(tr("测试板ID:")), m_testBoardIdInput);
    QHBoxLayout* acqButtonsLayout = new QHBoxLayout();
    acqButtonsLayout->addWidget(m_startAcqButton);
    acqButtonsLayout->addWidget(m_stopAcqButton);
    testBoardLayout->addRow(acqButtonsLayout);
    testBoardGroup->setLayout(testBoardLayout);
    mainLayout->addWidget(testBoardGroup);

    // Connections
    connect(m_turntableMoveButton, &QPushButton::clicked, this, &DeviceControlDialog::onMoveTurntableClicked);
    connect(m_startAcqButton, &QPushButton::clicked, this, &DeviceControlDialog::onStartAcquisitionClicked);
    connect(m_stopAcqButton, &QPushButton::clicked, this, &DeviceControlDialog::onStopAcquisitionClicked);

    setLayout(mainLayout);
}

void DeviceControlDialog::onMoveTurntableClicked()
{
    if (!m_coreEngine) {
        QMessageBox::warning(this, tr("错误"), tr("核心引擎未初始化"));
        return;
    }
    QString deviceId = m_turntableIdInput->text();
    bool ok;
    double angle = m_turntableAngleInput->text().toDouble(&ok);
    if (!ok) {
        QMessageBox::warning(this, tr("错误"), tr("无效的角度值"));
        return;
    }

    QJsonObject args;
    args["angle"] = angle;
    auto res = m_coreEngine->executeDeviceCommand(deviceId, "moveToPosition", args);
    LOG_MODULE_INFO("DeviceControl", QString("moveToPosition result: %1")
                    .arg(QString(QJsonDocument(res).toJson(QJsonDocument::Compact))).toStdString());
}

void DeviceControlDialog::onStartAcquisitionClicked()
{
    if (!m_coreEngine) {
        QMessageBox::warning(this, tr("错误"), tr("核心引擎未初始化"));
        return;
    }
    QString deviceId = m_testBoardIdInput->text();
    {
        auto res = m_coreEngine->executeDeviceCommand(deviceId, "startDataAcquisition", QJsonObject{});
        LOG_MODULE_INFO("DeviceControl", QString("startDataAcquisition result: %1")
                        .arg(QString(QJsonDocument(res).toJson(QJsonDocument::Compact))).toStdString());
    }
}

void DeviceControlDialog::onStopAcquisitionClicked()
{
    if (!m_coreEngine) {
        QMessageBox::warning(this, tr("错误"), tr("核心引擎未初始化"));
        return;
    }
    QString deviceId = m_testBoardIdInput->text();
    {
        auto res = m_coreEngine->executeDeviceCommand(deviceId, "stopDataAcquisition", QJsonObject{});
        LOG_MODULE_INFO("DeviceControl", QString("stopDataAcquisition result: %1")
                        .arg(QString(QJsonDocument(res).toJson(QJsonDocument::Compact))).toStdString());
    }
}

} // namespace Presentation
