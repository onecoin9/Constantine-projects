#include "ui/HandlerControlWidget.h"
#include "domain/HandlerDevice.h"
#include "domain/protocols/ISProtocol.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QMessageBox>
#include <QCheckBox>
#include <QTimer>
#include <QFrame>
#include <QDialog>
#include <QFont>
#include <QTextCursor>
#include <QMenu>
#include <QAction>

namespace Presentation {

HandlerControlWidget::HandlerControlWidget(QWidget *parent)
    : IDeviceControlWidget(parent)
    , m_handlerDevice(nullptr)
    , m_logDialog(nullptr)
    , m_logTextEdit(nullptr)
    , m_clearLogBtn(nullptr)
    , m_responseTimer(nullptr)
    , m_showLogDialogBtn(nullptr)
    , m_isSending(false)
    , m_lastSentCommand("")
{
    setupUi();
}

HandlerControlWidget::~HandlerControlWidget()
{
    if (m_responseTimer) {
        m_responseTimer->stop();
    }
    
    if (m_logDialog) {
        m_logDialog->close();
    }

    if (m_handlerDevice) {
        disconnect(m_handlerDevice.get(), nullptr, this, nullptr);
        
        auto sProtocol = m_handlerDevice->getSProtocol();
        if (sProtocol) {
            disconnect(sProtocol, nullptr, this, nullptr);
        }
    }
}

void HandlerControlWidget::setDevice(std::shared_ptr<Domain::IDevice> device)
{
    log("setDevice 被调用", "DEBUG");
    
    if (m_handlerDevice) {
        disconnect(m_handlerDevice.get(), nullptr, this, nullptr);
        auto sProtocol = m_handlerDevice->getSProtocol();
        if (sProtocol) {
            disconnect(sProtocol, nullptr, this, nullptr);
        }
    }
    
    if (!device) {
        m_handlerDevice.reset();
        setControlsEnabled(false);
        log("设备已断开连接", "INFO");
        return;
    }
    
    log(QString("接收到设备: %1, 类型: %2").arg(device->getName()).arg(static_cast<int>(device->getType())), "DEBUG");
    log(QString("设备状态: %1").arg(static_cast<int>(device->getStatus())), "DEBUG");
    
    m_handlerDevice = std::dynamic_pointer_cast<Domain::HandlerDevice>(device);
    if (m_handlerDevice) {
        log("设备类型转换成功，开始连接信号", "DEBUG");
        connectSignals();
        updateDeviceStatus();
        
        // 检查SProtocol状态
        auto sProtocol = m_handlerDevice->getSProtocol();
        if (!sProtocol) {
            log("SProtocol 实例为空", "WARNING");
        }
        
        log(QString("设备已连接: %1，启用控件").arg(device->getName()), "INFO");
        setControlsEnabled(true);
        log("控件已启用", "DEBUG");
        
    } else {
        log(QString("设备类型不匹配 - 期望HandlerDevice，实际类型: %1").arg(static_cast<int>(device->getType())), "ERROR");
        setControlsEnabled(false);
    }

    // 更新PDU 67的站点下拉框
    m_pdu67SiteSelectCombo->clear();
    if (m_handlerDevice) {
        QJsonObject config = m_handlerDevice->getConfiguration();
        QJsonArray sites = config.value("siteConfiguration").toObject().value("sites").toArray();
        for (const QJsonValue& val : sites) {
            QJsonObject siteObj = val.toObject();
            if (siteObj.value("siteEnvInit").toBool(false)) {
                int siteIndex = siteObj["siteIndex"].toInt();
                QString siteAlias = siteObj["siteAlias"].toString(QString("Site %1").arg(siteIndex));
                m_pdu67SiteSelectCombo->addItem(QString("%1 (站点 %2)").arg(siteAlias).arg(siteIndex), QVariant::fromValue(siteObj));
            }
        }
    }
    // 手动触发一次更新
    onSiteSelectionChanged(m_pdu67SiteSelectCombo->currentIndex());
}

std::shared_ptr<Domain::IDevice> HandlerControlWidget::getDevice() const
{
    return m_handlerDevice;
}

void HandlerControlWidget::updateStatus()
{
    updateDeviceStatus();
}

void HandlerControlWidget::setControlsEnabled(bool enabled)
{
    log(QString("setControlsEnabled 被调用，参数: %1").arg(enabled ? "true" : "false"), "DEBUG");
    
    // 检查控件是否已创建
    int enabledCount = 0;
    int totalCount = 12; // 总控件数
    
    // 设置主要控制按钮
    if (m_loadTaskBtn) { m_loadTaskBtn->setEnabled(enabled); enabledCount++; }
    if (m_tellDevReadyBtn) { m_tellDevReadyBtn->setEnabled(enabled); enabledCount++; }
    if (m_tellDevCommVersionBtn) { m_tellDevCommVersionBtn->setEnabled(enabled); enabledCount++; }
    if (m_querySiteEnableBtn) { m_querySiteEnableBtn->setEnabled(enabled); enabledCount++; }
    if (m_setDoneSiteBtn) { m_setDoneSiteBtn->setEnabled(enabled); enabledCount++; }
    if (m_getSiteMapBtn) { m_getSiteMapBtn->setEnabled(enabled); enabledCount++; }
    if (m_sendContactCheckResultBtn) { m_sendContactCheckResultBtn->setEnabled(enabled); enabledCount++; }
    if (m_sendRemainingCheckResultBtn) { m_sendRemainingCheckResultBtn->setEnabled(enabled); enabledCount++; }
    if (m_sendAxisMoveBtn) { m_sendAxisMoveBtn->setEnabled(enabled); enabledCount++; }
    
    // 设置输入控件
    if (m_taskPathEdit) { m_taskPathEdit->setEnabled(enabled); enabledCount++; }
    if (m_checkResultEdit) { m_checkResultEdit->setEnabled(enabled); enabledCount++; }
    
    // 设置CMD3相关控件
    if (m_productionQuantitySpinBox) { m_productionQuantitySpinBox->setEnabled(enabled); enabledCount++; }
    if (m_supplyTrayCombo) { m_supplyTrayCombo->setEnabled(enabled); enabledCount++; }
    if (m_productionTrayCombo) { m_productionTrayCombo->setEnabled(enabled); enabledCount++; }
    if (m_rejectTrayCombo) { m_rejectTrayCombo->setEnabled(enabled); enabledCount++; }
    if (m_reelStartPosSpinBox) { m_reelStartPosSpinBox->setEnabled(enabled); enabledCount++; }
    
    // 托盘坐标输入框需要根据配置类型单独处理
    if (enabled) {
        if (m_supplyTrayCombo && m_supplyTrayXSpinBox && m_supplyTrayYSpinBox) {
            bool isManual = (m_supplyTrayCombo->currentData().toInt() == 1);
            m_supplyTrayXSpinBox->setEnabled(isManual);
            m_supplyTrayYSpinBox->setEnabled(isManual);
        }
        if (m_productionTrayCombo && m_productionTrayXSpinBox && m_productionTrayYSpinBox) {
            bool isManual = (m_productionTrayCombo->currentData().toInt() == 1);
            m_productionTrayXSpinBox->setEnabled(isManual);
            m_productionTrayYSpinBox->setEnabled(isManual);
        }
        if (m_rejectTrayCombo && m_rejectTrayXSpinBox && m_rejectTrayYSpinBox) {
            bool isManual = (m_rejectTrayCombo->currentData().toInt() == 1);
            m_rejectTrayXSpinBox->setEnabled(isManual);
            m_rejectTrayYSpinBox->setEnabled(isManual);
        }
    } else {
        if (m_supplyTrayXSpinBox) m_supplyTrayXSpinBox->setEnabled(false);
        if (m_supplyTrayYSpinBox) m_supplyTrayYSpinBox->setEnabled(false);
        if (m_productionTrayXSpinBox) m_productionTrayXSpinBox->setEnabled(false);
        if (m_productionTrayYSpinBox) m_productionTrayYSpinBox->setEnabled(false);
        if (m_rejectTrayXSpinBox) m_rejectTrayXSpinBox->setEnabled(false);
        if (m_rejectTrayYSpinBox) m_rejectTrayYSpinBox->setEnabled(false);
    }
    
    // 日志按钮始终可用
    if (m_showLogDialogBtn) {
        m_showLogDialogBtn->setEnabled(true);
    }
    
    log(QString("控件状态设置完成: %1/%2 个控件存在，状态设为: %3").arg(enabledCount).arg(totalCount).arg(enabled ? "启用" : "禁用"), "DEBUG");
}

QString HandlerControlWidget::getDeviceTypeName() const
{
    return tr("自动化处理设备");
}

void HandlerControlWidget::setupUi()
{
    auto mainLayout = new QVBoxLayout(this);
    
    // 创建功能区域
    createTaskControlGroup();
    createSiteControlGroup();
    createAxisControlGroup();
    createBottomControlArea();
    
    // 添加到主布局
    mainLayout->addWidget(m_taskControlGroup);
    mainLayout->addWidget(m_siteControlGroup);
    mainLayout->addWidget(m_axisControlGroup);
    
    // 创建定时器
    m_responseTimer = new QTimer(this);
    m_responseTimer->setSingleShot(true);
    
    connect(m_responseTimer, &QTimer::timeout, this, [this]() {
        log("命令执行超时", "WARNING");
        m_isSending = false;
        m_lastSentCommand.clear();
        setControlsEnabled(true);
    });
}

void HandlerControlWidget::createTaskControlGroup()
{
    m_taskControlGroup = new QGroupBox("CMD3 任务设置", this);
    auto layout = new QVBoxLayout(m_taskControlGroup);
    
    // 任务文件选择
    auto fileLayout = new QHBoxLayout();
    fileLayout->addWidget(new QLabel("任务文件:"));
    m_taskPathEdit = new QLineEdit();
    m_taskPathEdit->setPlaceholderText("选择任务文件");
    fileLayout->addWidget(m_taskPathEdit);
    
    auto browseBtn = new QPushButton("浏览...");
    fileLayout->addWidget(browseBtn);
    layout->addLayout(fileLayout);
    
    // 生产数量设置
    auto quantityLayout = new QHBoxLayout();
    quantityLayout->addWidget(new QLabel("生产数量:"));
    m_productionQuantitySpinBox = new QSpinBox();
    m_productionQuantitySpinBox->setRange(1, 999999);
    m_productionQuantitySpinBox->setValue(100);
    quantityLayout->addWidget(m_productionQuantitySpinBox);
    quantityLayout->addStretch();
    layout->addLayout(quantityLayout);
    
    // 托盘坐标配置
    auto trayGroup = new QGroupBox("托盘起始位置设置");
    auto trayLayout = new QGridLayout(trayGroup);
    
    // 供给托盘
    int row = 0;
    trayLayout->addWidget(new QLabel("供给托盘:"), row, 0);
    m_supplyTrayCombo = new QComboBox();
    m_supplyTrayCombo->addItem("默认", 0);
    m_supplyTrayCombo->addItem("手动", 1);
    m_supplyTrayCombo->addItem("自动", 2);
    trayLayout->addWidget(m_supplyTrayCombo, row, 1);
    
    trayLayout->addWidget(new QLabel("X:"), row, 2);
    m_supplyTrayXSpinBox = new QSpinBox();
    m_supplyTrayXSpinBox->setRange(-9999, 9999);
    m_supplyTrayXSpinBox->setEnabled(false);
    trayLayout->addWidget(m_supplyTrayXSpinBox, row, 3);
    
    trayLayout->addWidget(new QLabel("Y:"), row, 4);
    m_supplyTrayYSpinBox = new QSpinBox();
    m_supplyTrayYSpinBox->setRange(-9999, 9999);
    m_supplyTrayYSpinBox->setEnabled(false);
    trayLayout->addWidget(m_supplyTrayYSpinBox, row, 5);
    
    // 生产托盘
    row++;
    trayLayout->addWidget(new QLabel("生产托盘:"), row, 0);
    m_productionTrayCombo = new QComboBox();
    m_productionTrayCombo->addItem("默认", 0);
    m_productionTrayCombo->addItem("手动", 1);
    m_productionTrayCombo->addItem("自动", 2);
    trayLayout->addWidget(m_productionTrayCombo, row, 1);
    
    trayLayout->addWidget(new QLabel("X:"), row, 2);
    m_productionTrayXSpinBox = new QSpinBox();
    m_productionTrayXSpinBox->setRange(-9999, 9999);
    m_productionTrayXSpinBox->setEnabled(false);
    trayLayout->addWidget(m_productionTrayXSpinBox, row, 3);
    
    trayLayout->addWidget(new QLabel("Y:"), row, 4);
    m_productionTrayYSpinBox = new QSpinBox();
    m_productionTrayYSpinBox->setRange(-9999, 9999);
    m_productionTrayYSpinBox->setEnabled(false);
    trayLayout->addWidget(m_productionTrayYSpinBox, row, 5);
    
    // 拒料托盘
    row++;
    trayLayout->addWidget(new QLabel("拒料托盘:"), row, 0);
    m_rejectTrayCombo = new QComboBox();
    m_rejectTrayCombo->addItem("默认", 0);
    m_rejectTrayCombo->addItem("手动", 1);
    m_rejectTrayCombo->addItem("自动", 2);
    trayLayout->addWidget(m_rejectTrayCombo, row, 1);
    
    trayLayout->addWidget(new QLabel("X:"), row, 2);
    m_rejectTrayXSpinBox = new QSpinBox();
    m_rejectTrayXSpinBox->setRange(-9999, 9999);
    m_rejectTrayXSpinBox->setEnabled(false);
    trayLayout->addWidget(m_rejectTrayXSpinBox, row, 3);
    
    trayLayout->addWidget(new QLabel("Y:"), row, 4);
    m_rejectTrayYSpinBox = new QSpinBox();
    m_rejectTrayYSpinBox->setRange(-9999, 9999);
    m_rejectTrayYSpinBox->setEnabled(false);
    trayLayout->addWidget(m_rejectTrayYSpinBox, row, 5);
    
    layout->addWidget(trayGroup);
    
    // 卷带起始位置
    auto reelLayout = new QHBoxLayout();
    reelLayout->addWidget(new QLabel("卷带起始位置:"));
    m_reelStartPosSpinBox = new QSpinBox();
    m_reelStartPosSpinBox->setRange(1, 9999);
    m_reelStartPosSpinBox->setValue(1);
    reelLayout->addWidget(m_reelStartPosSpinBox);
    reelLayout->addStretch();
    layout->addLayout(reelLayout);
    
    // 加载按钮
    m_loadTaskBtn = new QPushButton("加载任务");
    layout->addWidget(m_loadTaskBtn);
    
    // 连接信号
    connect(browseBtn, &QPushButton::clicked, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(this, "选择任务文件", "", "任务文件 (*.tsk *.json)");
        if (!filePath.isEmpty()) {
            m_taskPathEdit->setText(filePath);
        }
    });
    
    // 托盘配置类型变化时启用/禁用坐标输入
    auto setupTrayCombo = [](QComboBox* combo, QSpinBox* xSpin, QSpinBox* ySpin) {
        connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [combo, xSpin, ySpin](int index) {
            bool isManual = (combo->itemData(index).toInt() == 1); // 1=手动
            xSpin->setEnabled(isManual);
            ySpin->setEnabled(isManual);
        });
    };
    
    setupTrayCombo(m_supplyTrayCombo, m_supplyTrayXSpinBox, m_supplyTrayYSpinBox);
    setupTrayCombo(m_productionTrayCombo, m_productionTrayXSpinBox, m_productionTrayYSpinBox);
    setupTrayCombo(m_rejectTrayCombo, m_rejectTrayXSpinBox, m_rejectTrayYSpinBox);
    
    connect(m_loadTaskBtn, &QPushButton::clicked, this, &HandlerControlWidget::onLoadTaskClicked);
}

void HandlerControlWidget::createSiteControlGroup()
{
    m_siteControlGroup = new QGroupBox("站点控制", this);
    auto layout = new QVBoxLayout(m_siteControlGroup);
    
    // 基本控制按钮
    auto btnLayout = new QHBoxLayout();
    m_querySiteEnableBtn = new QPushButton("查询站点使能");
    m_getSiteMapBtn = new QPushButton("获取站点映射");
    m_tellDevReadyBtn = new QPushButton("告知设备就绪 (PDU63)");
    
    btnLayout->addWidget(m_querySiteEnableBtn);
    btnLayout->addWidget(m_getSiteMapBtn);
    btnLayout->addWidget(m_tellDevReadyBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);
    
    // 版本控制
    auto versionLayout = new QHBoxLayout();
    versionLayout->addWidget(new QLabel("通信版本:"));
    m_commVersionSpinBox = new QSpinBox();
    m_commVersionSpinBox->setRange(1, 10);
    m_commVersionSpinBox->setValue(2);
    versionLayout->addWidget(m_commVersionSpinBox);
    
    m_tellDevCommVersionBtn = new QPushButton("告知Comm版本");
    versionLayout->addWidget(m_tellDevCommVersionBtn);
    versionLayout->addStretch();
    layout->addLayout(versionLayout);

    // -- PDU 0x67 结果设置 (新UI) --
    auto pdu67Layout = new QHBoxLayout();
    pdu67Layout->addWidget(new QLabel("烧录结果站点:"));

    m_pdu67SiteSelectCombo = new QComboBox();
    pdu67Layout->addWidget(m_pdu67SiteSelectCombo);

    m_setDoneSiteBtn = new QPushButton("告知烧录结果 (0x67)");
    pdu67Layout->addWidget(m_setDoneSiteBtn);
    pdu67Layout->addStretch();
    layout->addLayout(pdu67Layout);

    // 动态Socket结果区域
    m_socketResultWidget = new QWidget();
    m_socketResultLayout = new QGridLayout(m_socketResultWidget);
    m_socketResultLayout->setSpacing(10);
    m_socketResultLayout->setContentsMargins(15, 5, 15, 5);
    layout->addWidget(m_socketResultWidget);
    
    // IC检查结果 (旧的控件暂时保留)
    auto icLayout = new QHBoxLayout();
    icLayout->addWidget(new QLabel("AdapterCnt:"));
    m_adapterCntSpinBox = new QSpinBox();
    m_adapterCntSpinBox->setRange(1, 64);
    m_adapterCntSpinBox->setValue(8);
    icLayout->addWidget(m_adapterCntSpinBox);

    icLayout->addWidget(new QLabel("Result(Hex):"));
    m_checkResultEdit = new QLineEdit();
    m_checkResultEdit->setPlaceholderText("00 01 02 ...");
    icLayout->addWidget(m_checkResultEdit);

    m_sendContactCheckResultBtn = new QPushButton("发送接触检查结果");
    m_sendRemainingCheckResultBtn = new QPushButton("发送残料检查结果");
    icLayout->addWidget(m_sendContactCheckResultBtn);
    icLayout->addWidget(m_sendRemainingCheckResultBtn);
    layout->addLayout(icLayout);
    
    // 连接信号
    connect(m_pdu67SiteSelectCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HandlerControlWidget::onSiteSelectionChanged);
    connect(m_querySiteEnableBtn, &QPushButton::clicked, this, &HandlerControlWidget::onQuerySiteEnableClicked);
    connect(m_getSiteMapBtn, &QPushButton::clicked, this, &HandlerControlWidget::onGetSiteMapClicked);
    connect(m_tellDevReadyBtn, &QPushButton::clicked, this, &HandlerControlWidget::onTellDevReadyClicked);
    connect(m_tellDevCommVersionBtn, &QPushButton::clicked, this, &HandlerControlWidget::onTellDevCommVersionClicked);
    connect(m_setDoneSiteBtn, &QPushButton::clicked, this, &HandlerControlWidget::onSetDoneSiteClicked);
    connect(m_sendContactCheckResultBtn, &QPushButton::clicked, this, &HandlerControlWidget::onSendContactCheckResultClicked);
    connect(m_sendRemainingCheckResultBtn, &QPushButton::clicked, this, &HandlerControlWidget::onSendRemainingCheckResultClicked);
}

void HandlerControlWidget::createAxisControlGroup()
{
    m_axisControlGroup = new QGroupBox("轴控制", this);
    auto layout = new QHBoxLayout(m_axisControlGroup);
    
    layout->addWidget(new QLabel("轴选择:"));
    m_axisSelectCombo = new QComboBox();
    m_axisSelectCombo->addItems({"Axis_Pitch", "Axis_Roll", "Axis_Yaw", "Axis_X", "Axis_Y", "Axis_Z"});
    layout->addWidget(m_axisSelectCombo);
    
    layout->addWidget(new QLabel("站点索引:"));
    m_axisSiteIdxSpinBox = new QSpinBox();
    m_axisSiteIdxSpinBox->setRange(1, 64);
    layout->addWidget(m_axisSiteIdxSpinBox);
    
    layout->addWidget(new QLabel("目标角度:"));
    m_targetAngleSpinBox = new QSpinBox();
    m_targetAngleSpinBox->setRange(-360, 360);
    m_targetAngleSpinBox->setSuffix("°");
    layout->addWidget(m_targetAngleSpinBox);
    
    m_sendAxisMoveBtn = new QPushButton("发送轴移动");
    layout->addWidget(m_sendAxisMoveBtn);
    layout->addStretch();
    
    connect(m_sendAxisMoveBtn, &QPushButton::clicked, this, &HandlerControlWidget::onSendAxisMoveClicked);
}

// [已删除] 原始帧调试功能

void HandlerControlWidget::createBottomControlArea()
{
    auto bottomLayout = new QHBoxLayout();
    
    m_showLogDialogBtn = new QPushButton("显示调试日志");
    m_showLogDialogBtn->setCheckable(true);
    bottomLayout->addWidget(m_showLogDialogBtn);
    bottomLayout->addStretch();
    
    auto mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (mainLayout) {
        mainLayout->addLayout(bottomLayout);
    }
    
    connect(m_showLogDialogBtn, &QPushButton::toggled, this, [this](bool checked) {
        if (checked) {
            if (!m_logDialog) {
                createLogDialog();
            }
            if (m_logDialog) {
                m_logDialog->show();
            }
        } else {
            if (m_logDialog) {
                m_logDialog->hide();
            }
        }
    });
}

void HandlerControlWidget::createLogDialog()
{
    if (m_logDialog) {
        return; // 已经创建过了
    }
    
    m_logDialog = new QDialog(this);
    m_logDialog->setWindowTitle(tr("调试日志"));
    m_logDialog->setModal(false); // 非模态窗口
    m_logDialog->resize(800, 400);
    
    auto layout = new QVBoxLayout(m_logDialog);
    
    m_logTextEdit = new QTextEdit();
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setFont(QFont("Consolas", 9)); // 使用等宽字体便于阅读
    
    // 将缓存的日志添加到日志编辑器中
    if (!m_logCache.isEmpty() && m_logTextEdit) {
        try {
            m_logTextEdit->setPlainText(m_logCache.join("\n"));
            // 滚动到底部显示最新日志
            m_logTextEdit->moveCursor(QTextCursor::End);
        } catch (const std::exception& e) {
            log(QString("Exception when setting log text: %1").arg(e.what()), "ERROR");
        }
    }
    
    layout->addWidget(m_logTextEdit);
    
    // 底部按钮区域
    auto btnLayout = new QHBoxLayout();
    m_clearLogBtn = new QPushButton(tr("清除日志"));
    auto closeBtn = new QPushButton(tr("关闭"));
    
    btnLayout->addWidget(m_clearLogBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    
    layout->addLayout(btnLayout);
    
    // 连接信号
    connect(m_clearLogBtn, &QPushButton::clicked, this, [this]() {
        if (!this) {
            return;
        }
        
        // 清除缓存和显示
        m_logCache.clear();
        if (m_logTextEdit) {
            m_logTextEdit->clear();
        }
        log(tr("日志已清除"), "INFO");
    });
    
    connect(closeBtn, &QPushButton::clicked, this, [this]() {
        if (!this || !m_logDialog) {
            return;
        }
        m_logDialog->hide();
        if (m_showLogDialogBtn) {
            m_showLogDialogBtn->setChecked(false);
        }
    });
    
    // 当对话框关闭时，取消按钮的选中状态
    connect(m_logDialog, &QDialog::finished, this, [this]() {
        if (!this || !m_showLogDialogBtn) {
            return;
        }
        m_showLogDialogBtn->setChecked(false);
    });
}

void HandlerControlWidget::connectSignals()
{
    if (!m_handlerDevice) return;
    
    disconnect(m_handlerDevice.get(), nullptr, this, nullptr);
    
    // 连接设备信号
    connect(m_handlerDevice.get(), &Domain::HandlerDevice::chipPlaced,
            this, &HandlerControlWidget::onChipPlaced);
    connect(m_handlerDevice.get(), &Domain::HandlerDevice::contactCheckRequested,
            this, &HandlerControlWidget::onContactCheckRequested);
    connect(m_handlerDevice.get(), &Domain::HandlerDevice::remainingCheckRequested,
            this, &HandlerControlWidget::onRemainingCheckRequested);
    connect(m_handlerDevice.get(), &Domain::HandlerDevice::axisMovementRequested,
            this, &HandlerControlWidget::onAxisMovementRequested);
    connect(m_handlerDevice.get(), &Domain::HandlerDevice::axisMovementCompleted,
            this, &HandlerControlWidget::onAxisMovementCompleted);
    connect(m_handlerDevice.get(), &Domain::HandlerDevice::rawFrameReceived,
            this, &HandlerControlWidget::onRawFrameReceived);
    connect(m_handlerDevice.get(), &Domain::HandlerDevice::debugMessage,
            this, &HandlerControlWidget::onDebugMessage);
    connect(m_handlerDevice.get(), &Domain::IDevice::errorOccurred, 
            this, &HandlerControlWidget::onCommandError);
    connect(m_handlerDevice.get(), &Domain::IDevice::commandFinished, 
            this, &HandlerControlWidget::onCommandFinished);
}

void HandlerControlWidget::onLoadTaskClicked()
{
    if (!m_handlerDevice) return;
    
    QString taskPath = m_taskPathEdit->text();
    if (taskPath.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择任务文件");
        return;
    }
    
    if (!QFile::exists(taskPath)) {
        QMessageBox::warning(this, "错误", "任务文件不存在");
        return;
    }
    
    // 构建CMD3参数
    QJsonObject cmdParams;
    cmdParams["command"] = taskPath;
    cmdParams["productionQuantity"] = m_productionQuantitySpinBox->value();
    
    // 供给托盘配置
    QJsonObject supplyTray;
    supplyTray["config"] = m_supplyTrayCombo->currentData().toInt();
    supplyTray["x"] = m_supplyTrayXSpinBox->value();
    supplyTray["y"] = m_supplyTrayYSpinBox->value();
    cmdParams["supplyTray"] = supplyTray;
    
    // 生产托盘配置
    QJsonObject productionTray;
    productionTray["config"] = m_productionTrayCombo->currentData().toInt();
    productionTray["x"] = m_productionTrayXSpinBox->value();
    productionTray["y"] = m_productionTrayYSpinBox->value();
    cmdParams["productionTray"] = productionTray;
    
    // 拒料托盘配置
    QJsonObject rejectTray;
    rejectTray["config"] = m_rejectTrayCombo->currentData().toInt();
    rejectTray["x"] = m_rejectTrayXSpinBox->value();
    rejectTray["y"] = m_rejectTrayYSpinBox->value();
    cmdParams["rejectTray"] = rejectTray;
    
    // 卷带起始位置
    cmdParams["reelStartPosition"] = m_reelStartPosSpinBox->value();
    
    log(QString("正在发送CMD3任务: 生产数量=%1, 任务文件=%2").arg(m_productionQuantitySpinBox->value()).arg(taskPath), "INFO");
    
    m_handlerDevice->executeCommand("sendCmd3Command", cmdParams);
}

void HandlerControlWidget::onTellDevReadyClicked()
{
    if (!m_handlerDevice) return;
    
    QJsonObject readyInfo = buildReadyInfo();
    QJsonObject params;
    params["readyInfo"] = readyInfo;
    
    log("正在发送PDU63告知设备就绪...", "INFO");
    QJsonObject result = m_handlerDevice->executeCommand("tellDevReady", params);
    if (result.value("success").toBool()) {
        log(tr("PDU63 '告知设备就绪' 命令已发送。"), "SUCCESS");
    } else {
        log(tr("PDU63 '告知设备就绪' 命令发送失败。请检查底层日志获取详细错误。"), "ERROR");
    }
}

void HandlerControlWidget::onQuerySiteEnableClicked()
{
    if (!m_handlerDevice) return;
    
    log(tr("正在查询站点使能..."), "INFO");
    // 移除 setControlsEnabled(false); - 不再禁用所有控件
    m_handlerDevice->executeCommand("querySiteEnable", QJsonObject());
}

void HandlerControlWidget::onSetDoneSiteClicked()
{
    if (!m_handlerDevice || m_pdu67SiteSelectCombo->currentIndex() < 0) return;
    
    QJsonObject siteObj = m_pdu67SiteSelectCombo->currentData().toJsonObject();
    int siteIdx = siteObj["siteIndex"].toInt();
    int socketCount = siteObj["socketCount"].toInt(16);

    log(QString("准备发送PDU 0x67: 站点=%1, Socket数量=%2").arg(siteIdx).arg(socketCount), "DEBUG");

    QByteArray resultData(socketCount, 0x00); // 初始化为0
    for (int i = 0; i < m_socketResultControls.size(); ++i) {
        auto pair = m_socketResultControls[i];
        QCheckBox* checkBox = pair.first;
        QSpinBox* spinBox = pair.second;
        if (checkBox && spinBox && checkBox->isChecked()) {
            resultData[i] = static_cast<char>(spinBox->value());
        } else {
            resultData[i] = static_cast<char>(0x00); // 00. 无 Socket 无 IC 或未启用
        }
    }

    log(tr("构建PDU 0x67结果数据: %1").arg(QString(resultData.toHex(' ').toUpper())), "DEBUG");

    QJsonObject params;
    params["siteIdx"] = siteIdx;
    params["result"] = QString(resultData.toHex());
    
    log(tr("正在设置站点完成状态..."), "INFO");
    QJsonObject result = m_handlerDevice->executeCommand("setDoneSite", params);
    
    if (!result["success"].toBool()) {
        QString error = result["error"].toString();
        log(tr("命令执行失败: %1").arg(error), "ERROR");
    } else {
        log(tr("PDU 0x67 命令已成功发送"), "SUCCESS");
    }
}

void HandlerControlWidget::onGetSiteMapClicked()
{
    if (!m_handlerDevice) return;
    
    log(tr("正在获取站点映射..."), "INFO");
    //setControlsEnabled(false);
    m_handlerDevice->executeCommand("getSiteMap", QJsonObject());
}

void HandlerControlWidget::onSendAxisMoveClicked()
{
    if (!m_handlerDevice) return;
    
    if (m_isSending) {
        log("正在发送命令中，请等待...", "WARNING");
        return;
    }
    
    QJsonObject params;
    params["axisSelect"] = m_axisSelectCombo->currentText();
    params["siteIdx"] = m_axisSiteIdxSpinBox->value();
    params["targetAngle"] = m_targetAngleSpinBox->value();
    
    log("正在发送轴移动命令...", "INFO");
    
    m_isSending = true;
    m_sendAxisMoveBtn->setEnabled(false);
    
    // 添加超时保护
    QTimer::singleShot(5000, this, [this]() {
        if (m_isSending) {
            log("轴移动命令超时", "WARNING");
        m_isSending = false;
        m_sendAxisMoveBtn->setEnabled(true);
        }
    });
    
    m_handlerDevice->executeCommand("sendAxisMove", params);
}

// [已删除] 原始帧相关方法

void HandlerControlWidget::onChipPlaced(int siteIndex, uint32_t slotEn, const QString &siteSn)
{
    log(tr("芯片放置事件: 站点%1, 槽位0x%2, SN:%3").arg(siteIndex).arg(slotEn, 4, 16, QChar('0')).arg(siteSn), "EVENT");
    
    // 使用异步调用避免在信号处理中阻塞
    QTimer::singleShot(0, this, [this]() {
        if (this) {  // 确保对象仍然有效
            updateSiteTable();
        }
    });
}

void HandlerControlWidget::onContactCheckRequested(int siteIdx, const QByteArray &sktEn)
{
    log(tr("接触检查请求: 站点%1, 数据:%2").arg(siteIdx).arg(formatHexData(sktEn)), "EVENT");
}

void HandlerControlWidget::onRemainingCheckRequested(int siteIdx, const QByteArray &sktEn)
{
    log(tr("残料检查请求: 站点%1, 数据:%2").arg(siteIdx).arg(formatHexData(sktEn)), "EVENT");
}

// 已删除 onProgramResultReceived 方法

void HandlerControlWidget::onAxisMovementRequested(const QString &axisSelect, int siteIdx, int targetAngle)
{
    log(tr("轴移动请求: %1, 站点%2, 目标角度%3°").arg(axisSelect).arg(siteIdx).arg(targetAngle), "EVENT");
}

void HandlerControlWidget::onAxisMovementCompleted(const QString &axisSelect, int siteIdx, int currentAngle, int result, const QString &errMsg)
{
    log(tr("轴移动完成: %1, 站点%2, 当前角度%3°, 结果:%4").arg(axisSelect).arg(siteIdx).arg(currentAngle).arg(result), "EVENT");
    if (!errMsg.isEmpty()) {
        log(tr("错误信息: %1").arg(errMsg), "ERROR");
    }
}

void HandlerControlWidget::onSiteSelectionChanged(int index)
{
    // 清理旧的控件
    qDeleteAll(m_socketResultWidget->findChildren<QWidget*>("", Qt::FindDirectChildrenOnly));
    m_socketResultControls.clear();

    if (index < 0) return;

    QJsonObject siteObj = m_pdu67SiteSelectCombo->itemData(index).toJsonObject();
    int socketCount = siteObj.value("socketCount").toInt(16);
    log(tr("站点 %1 被选中, Socket数量: %2").arg(m_pdu67SiteSelectCombo->itemText(index)).arg(socketCount), "DEBUG");
    
    const int cols = 8; // 每行最多显示8个
    for (int i = 0; i < socketCount; ++i) {
        int row = i / cols;
        int col = i % cols * 3;

        // CheckBox for enabling
        QCheckBox* checkBox = new QCheckBox(QString("SKT%1").arg(i + 1, 2, 10, QChar('0')));
        checkBox->setChecked(true); // 默认选中
        m_socketResultLayout->addWidget(checkBox, row, col);

        // SpinBox for Bin value
        QSpinBox* spinBox = new QSpinBox();
        spinBox->setRange(1, 255);
        spinBox->setValue(1); // Default to PASS
        spinBox->setPrefix("Bin:");
        m_socketResultLayout->addWidget(spinBox, row, col + 1, 1, 2);

        m_socketResultControls.append({checkBox, spinBox});
    }
}

// 已删除 onRawFrameSent 方法

void HandlerControlWidget::onRawFrameReceived(const QByteArray &frame)
{
    QString hexData = formatHexData(frame);
    log(tr("RX ← 自动化设备: %1").arg(hexData), "RAW");
}

void HandlerControlWidget::onDebugMessage(const QString &message)
{
    log(message, "DEBUG");
}

void HandlerControlWidget::updateSiteTable()
{
    log(tr("站点信息已更新"), "INFO");
}

void HandlerControlWidget::updateDeviceStatus()
{
    if (!m_handlerDevice) return;
    
    auto status = m_handlerDevice->getStatus();
    QString statusText;
    switch (status) {
        case Domain::IDevice::DeviceStatus::Disconnected:
            statusText = tr("未连接");
            break;
        case Domain::IDevice::DeviceStatus::Connected:
            statusText = tr("已连接");
            break;
        case Domain::IDevice::DeviceStatus::Ready:
            statusText = tr("就绪");
            break;
        case Domain::IDevice::DeviceStatus::Busy:
            statusText = tr("忙碌");
            break;
        case Domain::IDevice::DeviceStatus::Error:
            statusText = tr("错误");
            break;
        default:
            statusText = tr("未知");
    }
    
    log(tr("设备状态: %1").arg(statusText), "STATUS");
}

void HandlerControlWidget::log(const QString &message, const QString &category)
{
    // 基本保护，防止在析构过程中调用
    if (!this) {
        log(QString("log() called with null this pointer!"), "CRITICAL");
        return;
    }
    
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString formattedMsg = QString("[%1][%2] %3").arg(timestamp).arg(category).arg(message);
    
    // 将日志添加到缓存中
    try {
        m_logCache.append(formattedMsg);
        
        // 限制缓存大小，避免内存溢出（保留最新的1000条）
        if (m_logCache.size() > 1000) {
            m_logCache.removeFirst();
        }
    } catch (const std::exception& e) {
        log(QString(tr("Exception in log() when accessing m_logCache: %1")).arg(e.what()), "ERROR");
        return;
    }
    
    // 如果日志对话框已创建，立即显示日志
    if (m_logTextEdit) {
        try {
            m_logTextEdit->append(formattedMsg);
        } catch (const std::exception& e) {
            log(QString(tr("Exception in log() when appending to m_logTextEdit: %1")).arg(e.what()), "ERROR");
        }
    }
}

QString HandlerControlWidget::formatHexData(const QByteArray &data)
{
    QString result;
    for (int i = 0; i < data.size(); ++i) {
        if (i > 0) result += " ";
        result += QString("%1").arg(static_cast<uint8_t>(data[i]), 2, 16, QChar('0')).toUpper();
    }
    return result;
}

QString HandlerControlWidget::getPduDescription(uint8_t pdu)
{
    switch (pdu) {
        // StdMes → AutoApp
        case 0x63: return "告知初始化情况";
        case 0x64: return "允许开始测试/流程控制";
        case 0x67: return "告知芯片烧录结果";
        case 0x61: return "告知软件版本信息";
        case 0x68: return "告知接触检查结果";
        case 0x65: return "告知残料检查结果";
        
        // AutoApp → StdMes
        case 0xE6: return "告知芯片放置";
        case 0xE1: return "请求软件版本";
        case 0xE8: return "请求接触检查";
        case 0xE5: return "请求残料检查";
        case 0xE4: return "告知站点使能";
        
        default: return QString("未知(0x%1)").arg(pdu, 2, 16, QChar('0'));
    }
}

QJsonObject HandlerControlWidget::buildReadyInfo()
{
    log(tr("开始构建设备就绪信息..."), "DEBUG");
    
    // 简化配置：只发送站点9（vauto特殊站点），避免vauto崩溃
    // 这样确保vauto的特殊站点功能能正常工作，且不会因数据过多而崩溃
        QJsonObject projInfo;
        projInfo["AdapterNum"] = 4;
    projInfo["SKTCnt"] = 4;  // 每个站点最大4个Socket（与vauto config.ini一致）
    projInfo["SiteCnt"] = 1;  // 只发送1个站点（站点9）
        
        QJsonArray siteReady;
    
    // 只包含站点9: 转台测试站（特殊站点）
    QJsonObject site9;
    site9["SiteSN"] = "Site09";
    site9["SiteAlias"] = "转台测试站";
    site9["SiteEnvRdy"] = 1;
    site9["SKTRdy"] = "0x0F";  // 4个Socket全部使能（与vauto special_site_max_skt=4一致）
    site9["SocketCount"] = 4;   // 与projInfo["SKTCnt"]保持一致
    siteReady.append(site9);
    
    log(tr("简化配置：只添加站点9(vauto特殊站点), Socket使能=0x0F"), "CONFIG");
        
        QJsonObject readyInfo;
        readyInfo["ProjInfo"] = projInfo;
        readyInfo["SiteReady"] = siteReady;
    
    log(tr("构建完成：简化配置，只包含vauto特殊站点9"), "CONFIG");
    log(tr("项目信息: AdapterNum=%1, SKTCnt=%2, SiteCnt=%3").arg(projInfo["AdapterNum"].toInt()).arg(projInfo["SKTCnt"].toInt()).arg(projInfo["SiteCnt"].toInt()), "CONFIG");
    
    return readyInfo;
}

void HandlerControlWidget::onTellDevCommVersionClicked()
{
    if (!m_handlerDevice || !m_commVersionSpinBox) return;
    
    QJsonObject params;
    params["version"] = m_commVersionSpinBox->value();
    
    log(tr("正在告知通信版本..."), "INFO");
    QJsonObject result = m_handlerDevice->executeCommand("tellDevCommVersion", params);
    if (result.value("success").toBool()) {
        log(tr("'告知通信版本' 命令已发送。"), "SUCCESS");
    } else {
        log(tr("'告知通信版本' 命令发送失败。请检查底层日志获取详细错误。"), "ERROR");
    }
}

void HandlerControlWidget::onSendContactCheckResultClicked()
{
    if (!m_handlerDevice || !m_checkResultEdit || m_pdu67SiteSelectCombo->currentIndex() < 0 || !m_adapterCntSpinBox) return;
    
    // 解析结果字节串
    QStringList byteStrs = m_checkResultEdit->text().simplified().split(' ', Qt::SkipEmptyParts);
    QJsonArray resultArray;
    for(const QString &b : byteStrs){
        bool ok=false; int val=b.toInt(&ok,16);
        if(ok) resultArray.append(val&0xFF);
    }
    
    QJsonObject siteObj = m_pdu67SiteSelectCombo->currentData().toJsonObject();
    int siteIdx = siteObj["siteIndex"].toInt();

    QJsonObject params;
    params["siteIdx"] = siteIdx;
    params["adapterCnt"] = m_adapterCntSpinBox->value();
    params["result"] = resultArray;
    
    auto result = m_handlerDevice->executeCommand("contactCheckResult", params);
    if (result["success"].toBool()) {
        log(tr("发送接触检查结果成功"), "SUCCESS");
    } else {
        log(tr("发送接触检查结果失败: %1").arg(result["error"].toString()), "ERROR");
    }
}

void HandlerControlWidget::onSendRemainingCheckResultClicked()
{
    if (!m_handlerDevice || !m_checkResultEdit || m_pdu67SiteSelectCombo->currentIndex() < 0 || !m_adapterCntSpinBox) return;
    
    QStringList byteStrs = m_checkResultEdit->text().simplified().split(' ', Qt::SkipEmptyParts);
    QJsonArray resultArray;
    for(const QString &b : byteStrs){
        bool ok=false; int val=b.toInt(&ok,16);
        if(ok) resultArray.append(val&0xFF);
    }
    
    QJsonObject siteObj = m_pdu67SiteSelectCombo->currentData().toJsonObject();
    int siteIdx = siteObj["siteIndex"].toInt();
    
    QJsonObject params;
    params["siteIdx"] = siteIdx;
    params["adapterCnt"] = m_adapterCntSpinBox->value();
    params["result"] = resultArray;
    
    auto result = m_handlerDevice->executeCommand("remainingCheckResult", params);
    if (result["success"].toBool()) {
        log(tr("发送残料检查结果成功"), "SUCCESS");
    } else {
        log(tr("发送残料检查结果失败: %1").arg(result["error"].toString()), "ERROR");
    }
}



// [新增] 处理命令完成的槽函数
void HandlerControlWidget::onCommandFinished(const QJsonObject& result)
{
    // 添加安全检查
    if (!this || !m_handlerDevice) {
        return;
    }
    
    QString command = result.value("command").toString();
    
    // 停止超时定时器
    if (m_responseTimer && m_responseTimer->isActive()) {
        m_responseTimer->stop();
        log(tr("收到响应，停止超时定时器"), "DEBUG");
    }
    
    // 重置发送状态
    bool wasSnding = m_isSending;
    if (m_isSending) {
        m_isSending = false;
        m_lastSentCommand.clear();
        log(tr("命令执行完成，重置发送状态"), "DEBUG");
    }
    
    // 只有在实际发送状态下才处理响应（避免重复处理）
    if (wasSnding || command == "getSiteMap") {
        if (command == "getSiteMap") {
            updateSiteTable();
            log("获取站点映射成功", "SUCCESS");
        } else if (command == "sendAxisMove") {
            log("轴移动命令已成功发送并被接受", "SUCCESS");
        } else {
            log(QString("命令 '%1' 执行完成").arg(command), "SUCCESS");
        }
        
        m_sendAxisMoveBtn->setEnabled(true);
    }
}

// [新增] 处理命令错误的槽函数
void HandlerControlWidget::onCommandError(const QString& errorMsg)
{
    // 添加安全检查
    if (!this) {
        return;
    }
    
    // 停止超时定时器
    if (m_responseTimer && m_responseTimer->isActive()) {
        m_responseTimer->stop();
        log(tr("命令错误，停止超时定时器"), "DEBUG");
    }
    
    log(tr("命令失败: %1").arg(errorMsg), "ERROR");
    
    // 重置发送状态
    if (m_isSending) {
        m_isSending = false;
        m_lastSentCommand.clear();
        log(tr("命令失败，重置发送状态"), "DEBUG");
    }
    
    // 只恢复轴移动按钮，不再全局恢复所有控件
    m_sendAxisMoveBtn->setEnabled(true);
    
    // 添加调试信息，帮助诊断问题
    log(tr("轴移动按钮已恢复"), "DEBUG");
}

// 已删除托盘配置相关代码

// 已删除托盘坐标获取相关代码

// 已删除站点配置相关代码

// 已删除所有站点配置和Socket使能相关的方法

// 已删除所有站点配置辅助方法

void HandlerControlWidget::logFrame(const QString& direction, const QByteArray& frame)
{
    // 尝试将数据帧解析为JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(frame, &parseError);

    // 如果是有效的JSON对象，则以紧凑的字符串格式记录
    if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
        QString jsonString = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
        log(QString("%1: %2").arg(direction).arg(jsonString), "JSON");
    } else {
        // 如果不是JSON，则回退到十六进制格式，用于记录二进制协议
        QString hexData = formatHexData(frame);
        log(QString("%1: %2").arg(direction).arg(hexData), "RAW");
    }
}

} // namespace Presentation

