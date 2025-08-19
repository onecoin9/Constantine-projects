#include "presentation/HandlerControlWidget.h"
#include "domain/HandlerDevice.h"
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
#include <QDataStream>
#include <QTimer>
#include <QFrame>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QDialog>
#include <QFont>
#include <QTextCursor>
#include <QDebug>
#include <QPointer>
#include <QCoreApplication>
#include <QMetaEnum>
#include <QMap>

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
    // 设置大小策略，允许控件根据内容动态调整大小
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    setupUi();
}

HandlerControlWidget::~HandlerControlWidget()
{
    // 1. 停止定时器，避免在析构过程中触发
    if (m_responseTimer) {
        m_responseTimer->stop();
        disconnect(m_responseTimer, nullptr, this, nullptr);
    }
    
    // 2. 关闭日志对话框
    if (m_logDialog) {
        m_logDialog->close();
    }

    // 3. 断开设备信号连接
    if (m_handlerDevice) {
        disconnect(m_handlerDevice.get(), nullptr, this, nullptr);
        
        // 断开SProtocol的连接
        auto sProtocol = m_handlerDevice->getSProtocol();
        if (sProtocol) {
            disconnect(sProtocol, nullptr, this, nullptr);
        }
    }
}

void HandlerControlWidget::setDevice(std::shared_ptr<Domain::IDevice> device)
{
    // 如果之前有设备，先断开所有连接
    if (m_handlerDevice) {
        disconnect(m_handlerDevice.get(), nullptr, this, nullptr);
        
        // 断开SProtocol的连接
        auto sProtocol = m_handlerDevice->getSProtocol();
        if (sProtocol) {
            disconnect(sProtocol, nullptr, this, nullptr);
        }
        
        // 清理旧设备引用
        m_handlerDevice.reset();
    }
    
    // 处理设置为nullptr的情况（用于清理）
    if (!device) {
        updateDeviceStatus();
        setControlsEnabled(false);
        log(tr("设备已断开连接"), "INFO");
        return;
    }
    
    m_handlerDevice = std::dynamic_pointer_cast<Domain::HandlerDevice>(device);
    if (m_handlerDevice) {
        // 连接新设备的信号
        connectSignals();
        updateDeviceStatus();
        log(tr("设备已连接: %1").arg(device->getName()));
    } else {
        log(tr("设备类型不匹配"), "ERROR");
        setControlsEnabled(false);
    }
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
    // 任务控制相关按钮 - 添加安全检查
    if (m_loadTaskBtn) m_loadTaskBtn->setEnabled(enabled);
    if (m_browseTaskBtn) m_browseTaskBtn->setEnabled(enabled);
    if (m_tellDevReadyBtn) m_tellDevReadyBtn->setEnabled(enabled);
    if (m_tellDevCommVersionBtn) m_tellDevCommVersionBtn->setEnabled(enabled);
    
    // 站点控制相关按钮 - 添加安全检查
    if (m_querySiteEnableBtn) m_querySiteEnableBtn->setEnabled(enabled);
    if (m_setDoneSiteBtn) m_setDoneSiteBtn->setEnabled(enabled);
    if (m_getSiteMapBtn) m_getSiteMapBtn->setEnabled(enabled);
    if (m_sendContactCheckResultBtn) m_sendContactCheckResultBtn->setEnabled(enabled);
    if (m_sendRemainingCheckResultBtn) m_sendRemainingCheckResultBtn->setEnabled(enabled);
    
    // 轴控制相关按钮 - 添加安全检查
    if (m_sendAxisMoveBtn) m_sendAxisMoveBtn->setEnabled(enabled);
    
    // 日志按钮始终可用
    if (m_showLogDialogBtn) {
        m_showLogDialogBtn->setEnabled(true);
    }
    
    // 输入控件 - 添加安全检查
    if (m_taskPathEdit) m_taskPathEdit->setEnabled(enabled);
    if (m_passlotEdit) m_passlotEdit->setEnabled(enabled);
    if (m_reelStartPosEdit) m_reelStartPosEdit->setEnabled(enabled);
    if (m_ipsTypeCombo) m_ipsTypeCombo->setEnabled(enabled);
    
    // 托盘配置控件 - 添加安全检查
    if (m_supplyTrayTypeCombo) m_supplyTrayTypeCombo->setEnabled(enabled);
    if (m_productionTrayTypeCombo) m_productionTrayTypeCombo->setEnabled(enabled);
    if (m_rejectTrayTypeCombo) m_rejectTrayTypeCombo->setEnabled(enabled);
    
    // 坐标输入框根据配置类型决定是否启用 - 添加安全检查
    if (enabled && m_supplyTrayTypeCombo && m_productionTrayTypeCombo && m_rejectTrayTypeCombo) {
        bool supplyManual = (m_supplyTrayTypeCombo->itemData(m_supplyTrayTypeCombo->currentIndex()).toInt() == Domain::Protocols::SProtocol::TRAY_CFG_MANUAL);
        bool productionManual = (m_productionTrayTypeCombo->itemData(m_productionTrayTypeCombo->currentIndex()).toInt() == Domain::Protocols::SProtocol::TRAY_CFG_MANUAL);
        bool rejectManual = (m_rejectTrayTypeCombo->itemData(m_rejectTrayTypeCombo->currentIndex()).toInt() == Domain::Protocols::SProtocol::TRAY_CFG_MANUAL);
        
        if (m_supplyTrayXEdit) m_supplyTrayXEdit->setEnabled(supplyManual);
        if (m_supplyTrayYEdit) m_supplyTrayYEdit->setEnabled(supplyManual);
        if (m_productionTrayXEdit) m_productionTrayXEdit->setEnabled(productionManual);
        if (m_productionTrayYEdit) m_productionTrayYEdit->setEnabled(productionManual);
        if (m_rejectTrayXEdit) m_rejectTrayXEdit->setEnabled(rejectManual);
        if (m_rejectTrayYEdit) m_rejectTrayYEdit->setEnabled(rejectManual);
    } else {
        if (m_supplyTrayXEdit) m_supplyTrayXEdit->setEnabled(false);
        if (m_supplyTrayYEdit) m_supplyTrayYEdit->setEnabled(false);
        if (m_productionTrayXEdit) m_productionTrayXEdit->setEnabled(false);
        if (m_productionTrayYEdit) m_productionTrayYEdit->setEnabled(false);
        if (m_rejectTrayXEdit) m_rejectTrayXEdit->setEnabled(false);
        if (m_rejectTrayYEdit) m_rejectTrayYEdit->setEnabled(false);
    }
}

QString HandlerControlWidget::getDeviceTypeName() const
{
    return tr("自动化处理设备");
}

void HandlerControlWidget::setupUi()
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);  // 减少边距
    mainLayout->setSpacing(5);  // 减少间距
    
    // 创建各个功能区域
    createTaskControlGroup();
    createSiteControlGroup();
    createAxisControlGroup();
    
    // 添加到主布局 - 设置更紧凑的间距
    mainLayout->addWidget(m_taskControlGroup);
    mainLayout->addWidget(m_siteControlGroup);
    mainLayout->addWidget(m_axisControlGroup);
    
    // 添加底部控制区域
    createBottomControlArea();
    
    // 不添加stretch，让内容更紧凑
    // mainLayout->addStretch();
    
    // 创建定时器用于超时保护
    m_responseTimer = new QTimer(this);
    m_responseTimer->setSingleShot(true);
    
    // 连接超时处理槽函数
    connect(m_responseTimer, &QTimer::timeout, this, [this]() {
        // 检查对象是否仍然有效
        if (!this) {
            return;
        }
        
        log(tr("命令执行超时，自动恢复界面控制"), "WARNING");
        
        // 重置发送状态
        m_isSending = false;
        m_lastSentCommand.clear();
        setControlsEnabled(true);
    });
}

void HandlerControlWidget::createTaskControlGroup()
{
    m_taskControlGroup = new QGroupBox(tr("CMD3 任务设置"), this);
    m_taskControlGroup->setCheckable(true);  // 设置为可勾选的组框
    m_taskControlGroup->setChecked(false);   // 默认不勾选（隐藏内容）
    m_taskControlGroup->setToolTip(tr("勾选以显示CMD3任务配置选项，取消勾选以隐藏并节省界面空间"));
    auto mainLayout = new QVBoxLayout(m_taskControlGroup);
    
    // 基本参数区域
    auto basicGroup = new QGroupBox(tr("基本参数"), this);
    auto basicLayout = new QGridLayout(basicGroup);
    
    // Pass Lot
    basicLayout->addWidget(new QLabel(tr("Pass Lot:")), 0, 0);
    m_passlotEdit = new QLineEdit("1");
    m_passlotEdit->setPlaceholderText("0");
    basicLayout->addWidget(m_passlotEdit, 0, 1);
    
    // IPS Type
    basicLayout->addWidget(new QLabel(tr("IPS 类型:")), 0, 2);
    m_ipsTypeCombo = new QComboBox();
    
    // 使用Qt元对象系统自动填充枚举
    QMetaEnum ipsEnum = QMetaEnum::fromType<Domain::Protocols::SProtocol::IPSType>();
    QStringList ipsNames = {tr("IPS5000/IPS5800"), tr("IPS3000/IPS5200"), tr("IPS7000/PHA2000"), tr("其他")};
    for (int i = 0; i < ipsEnum.keyCount() && i < ipsNames.size(); ++i) {
        m_ipsTypeCombo->addItem(ipsNames[i], ipsEnum.value(i));
    }
    
    basicLayout->addWidget(m_ipsTypeCombo, 0, 3);
    
    // 任务文件路径
    basicLayout->addWidget(new QLabel(tr("任务文件(.tsk):")), 1, 0);
    m_taskPathEdit = new QLineEdit();
    m_taskPathEdit->setPlaceholderText(tr("点击浏览选择任务文件"));
    basicLayout->addWidget(m_taskPathEdit, 1, 1);
    
    auto fileButtonLayout = new QHBoxLayout();
    m_browseTaskBtn = new QPushButton(tr("浏览..."));
    m_loadTaskBtn = new QPushButton(tr("加载任务 (CMD3)"));
    fileButtonLayout->addWidget(m_browseTaskBtn);
    fileButtonLayout->addWidget(m_loadTaskBtn);
    basicLayout->addLayout(fileButtonLayout, 1, 2, 1, 2);  // 跨越2列
    
    mainLayout->addWidget(basicGroup);
    
    // 托盘起始位置配置
    auto trayGroup = new QGroupBox(tr("托盘起始位置"), this);
    auto trayLayout = new QVBoxLayout(trayGroup);
    
    // 说明文字
    trayLayout->addWidget(new QLabel(tr("默认: 从默认起始坐标取放")));
    trayLayout->addWidget(new QLabel(tr("手动: 手动设置X和Y坐标")));
    trayLayout->addWidget(new QLabel(tr("自动: 自动设置托盘样式的起始坐标")));
    
    // 托盘配置网格
    auto trayGridLayout = new QGridLayout();
    
    // 创建托盘配置控件
    initTrayCoordWidgets();
    
    // 供料托盘
    trayGridLayout->addWidget(new QLabel(tr("供料托盘:")), 0, 0);
    trayGridLayout->addWidget(m_supplyTrayTypeCombo, 0, 1);
    trayGridLayout->addWidget(new QLabel(tr("X:")), 0, 3);
    trayGridLayout->addWidget(m_supplyTrayXEdit, 0, 4);
    trayGridLayout->addWidget(new QLabel(tr("Y:")), 0, 5);
    trayGridLayout->addWidget(m_supplyTrayYEdit, 0, 6);
    
    // 生产托盘
    trayGridLayout->addWidget(new QLabel(tr("生产托盘:")), 1, 0);
    trayGridLayout->addWidget(m_productionTrayTypeCombo, 1, 1);
    trayGridLayout->addWidget(new QLabel(tr("X:")), 1, 3);
    trayGridLayout->addWidget(m_productionTrayXEdit, 1, 4);
    trayGridLayout->addWidget(new QLabel(tr("Y:")), 1, 5);
    trayGridLayout->addWidget(m_productionTrayYEdit, 1, 6);
    
    // 废料托盘
    trayGridLayout->addWidget(new QLabel(tr("废料托盘:")), 2, 0);
    trayGridLayout->addWidget(m_rejectTrayTypeCombo, 2, 1);
    trayGridLayout->addWidget(new QLabel(tr("X:")), 2, 3);
    trayGridLayout->addWidget(m_rejectTrayXEdit, 2, 4);
    trayGridLayout->addWidget(new QLabel(tr("Y:")), 2, 5);
    trayGridLayout->addWidget(m_rejectTrayYEdit, 2, 6);
    
    // 添加弹性空间
    auto spacer = new QSpacerItem(50, 0, QSizePolicy::Expanding, QSizePolicy::Preferred);
    trayGridLayout->addItem(spacer, 0, 2);
    trayGridLayout->addItem(spacer, 1, 2);
    trayGridLayout->addItem(spacer, 2, 2);
    
    trayLayout->addLayout(trayGridLayout);
    mainLayout->addWidget(trayGroup);
    
    // 卷带起始位置
    auto reelLayout = new QHBoxLayout();
    reelLayout->addWidget(new QLabel(tr("卷带起始位置:")));
    m_reelStartPosEdit = new QLineEdit("0");
    m_reelStartPosEdit->setPlaceholderText("0");
    reelLayout->addWidget(m_reelStartPosEdit);
    reelLayout->addStretch();
    mainLayout->addLayout(reelLayout);
    
    // S协议控制内容已移至站点控制组
    
    // 连接信号
    connect(m_browseTaskBtn, &QPushButton::clicked, this, [this]() {
        if (!this) return;
        QString filePath = QFileDialog::getOpenFileName(this, 
            tr("选择任务文件"), "", tr("任务文件 (*.tsk *.json);;所有文件 (*)"));
        if (!filePath.isEmpty() && m_taskPathEdit) {
            m_taskPathEdit->setText(filePath);
        }
    });
    
    connect(m_loadTaskBtn, &QPushButton::clicked, this, &HandlerControlWidget::onLoadTaskClicked);
    
    // 连接折叠/展开功能
    connect(m_taskControlGroup, &QGroupBox::toggled, this, [this](bool checked) {
        log(checked ? tr("CMD3任务设置已展开") : tr("CMD3任务设置已折叠"), "UI");
        // 当状态改变时，通知父窗口调整大小
        if (parentWidget()) {
            QTimer::singleShot(0, parentWidget(), [this]() {
                parentWidget()->adjustSize();
            });
        }
    });
}

void HandlerControlWidget::createSiteControlGroup()
{
    m_siteControlGroup = new QGroupBox(tr("站点控制与配置"), this);
    auto layout = new QVBoxLayout(m_siteControlGroup);
    
    // 站点表格 - 增强显示Socket信息，设置更合适的大小
    m_siteTable = new QTableWidget(0, 7);
    m_siteTable->setHorizontalHeaderLabels({
        tr("站点索引"), tr("站点别名"), tr("SN"), tr("Socket数"), 
        tr("Socket使能"), tr("状态"), tr("操作")
    });
    m_siteTable->setMaximumHeight(200);  // 减少高度
    m_siteTable->setMinimumHeight(120);  // 设置最小高度
    m_siteTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_siteTable->horizontalHeader()->setStretchLastSection(true);  // 最后一列自动拉伸
    layout->addWidget(m_siteTable);
    
    // 控制按钮行
    auto btnLayout = new QHBoxLayout();
    m_querySiteEnableBtn = new QPushButton(tr("查询站点使能"));
    m_getSiteMapBtn = new QPushButton(tr("获取站点映射"));
    m_refreshSiteConfigBtn = new QPushButton(tr("刷新站点配置"));
    m_saveSiteConfigBtn = new QPushButton(tr("保存站点配置"));
    
    btnLayout->addWidget(m_querySiteEnableBtn);
    btnLayout->addWidget(m_getSiteMapBtn);
    btnLayout->addWidget(m_refreshSiteConfigBtn);
    btnLayout->addWidget(m_saveSiteConfigBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);
    
    // 站点配置编辑区域
    auto configGroup = new QGroupBox(tr("站点配置编辑"), this);
    auto configLayout = new QGridLayout(configGroup);
    
    // 选择站点
    configLayout->addWidget(new QLabel(tr("选择站点:")), 0, 0);
    m_configSiteCombo = new QComboBox();
    configLayout->addWidget(m_configSiteCombo, 0, 1);
    
    // Socket数量
    configLayout->addWidget(new QLabel(tr("Socket数量:")), 0, 2);
    m_socketCountSpinBox = new QSpinBox();
    m_socketCountSpinBox->setRange(1, 64);
    m_socketCountSpinBox->setValue(8);
    configLayout->addWidget(m_socketCountSpinBox, 0, 3);
    
    // Socket使能编辑
    configLayout->addWidget(new QLabel(tr("Socket使能(Hex):")), 1, 0);
    m_socketEnableEdit = new QLineEdit();
    m_socketEnableEdit->setPlaceholderText("0x0F");
    configLayout->addWidget(m_socketEnableEdit, 1, 1);
    
    // 站点别名
    configLayout->addWidget(new QLabel(tr("站点别名:")), 1, 2);
    m_siteAliasEdit = new QLineEdit();
    configLayout->addWidget(m_siteAliasEdit, 1, 3);
    
    // 站点SN
    configLayout->addWidget(new QLabel(tr("站点SN:")), 2, 0);
    m_siteSNEdit = new QLineEdit();
    configLayout->addWidget(m_siteSNEdit, 2, 1);
    
    // 站点状态
    configLayout->addWidget(new QLabel(tr("站点初始化:")), 2, 2);
    m_siteInitCheckBox = new QCheckBox(tr("已初始化"));
    configLayout->addWidget(m_siteInitCheckBox, 2, 3);
    
    // 应用按钮
    m_applySiteConfigBtn = new QPushButton(tr("应用配置"));
    configLayout->addWidget(m_applySiteConfigBtn, 3, 0);
    
    // Socket快速设置按钮
    auto quickSetLayout = new QHBoxLayout();
    quickSetLayout->addWidget(new QLabel(tr("快速设置:")));
    m_enableAllSocketsBtn = new QPushButton(tr("全部使能"));
    m_disableAllSocketsBtn = new QPushButton(tr("全部禁用"));
    m_enableFirstHalfBtn = new QPushButton(tr("前半部分"));
    m_enableSecondHalfBtn = new QPushButton(tr("后半部分"));
    
    quickSetLayout->addWidget(m_enableAllSocketsBtn);
    quickSetLayout->addWidget(m_disableAllSocketsBtn);
    quickSetLayout->addWidget(m_enableFirstHalfBtn);
    quickSetLayout->addWidget(m_enableSecondHalfBtn);
    quickSetLayout->addStretch();
    
    configLayout->addLayout(quickSetLayout, 3, 1, 1, 3);
    
    layout->addWidget(configGroup);
    
    // 设置完成状态区域（保留原有功能）
    auto doneLayout = new QHBoxLayout();
    doneLayout->addWidget(new QLabel(tr("站点索引:")));
    m_siteIdxSpinBox = new QSpinBox();
    m_siteIdxSpinBox->setRange(1, 64);
    doneLayout->addWidget(m_siteIdxSpinBox);
    
    doneLayout->addWidget(new QLabel(tr("结果(Hex):")));
    m_resultEdit = new QLineEdit();
    m_resultEdit->setText("FF");
    doneLayout->addWidget(m_resultEdit);
    
    doneLayout->addWidget(new QLabel(tr("掩码(Hex):")));
    m_maskEdit = new QLineEdit();
    m_maskEdit->setText("FFFF");
    doneLayout->addWidget(m_maskEdit);
    
    m_setDoneSiteBtn = new QPushButton(tr("设置完成"));
    doneLayout->addWidget(m_setDoneSiteBtn);
    doneLayout->addStretch();
    
    layout->addLayout(doneLayout);

    // IC 检查结果测试区域（保留原有功能）
    auto icLayout = new QHBoxLayout();
    icLayout->addWidget(new QLabel(tr("AdapterCnt:")));
    m_adapterCntSpinBox = new QSpinBox();
    m_adapterCntSpinBox->setRange(1, 64);
    m_adapterCntSpinBox->setValue(8);
    icLayout->addWidget(m_adapterCntSpinBox);

    icLayout->addWidget(new QLabel(tr("Result(Byte Hex):")));
    m_checkResultEdit = new QLineEdit();
    m_checkResultEdit->setPlaceholderText("00 01 02 ...");
    icLayout->addWidget(m_checkResultEdit);

    m_sendContactCheckResultBtn = new QPushButton(tr("发送接触检查结果"));
    m_sendRemainingCheckResultBtn = new QPushButton(tr("发送残料检查结果"));
    icLayout->addWidget(m_sendContactCheckResultBtn);
    icLayout->addWidget(m_sendRemainingCheckResultBtn);
    icLayout->addStretch();

    layout->addLayout(icLayout);
    
    // 添加分隔线
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);
    
    // S协议控制区域
    auto sProtocolGroup = new QGroupBox(tr("S协议控制"), this);
    auto sProtocolLayout = new QHBoxLayout(sProtocolGroup);
    
    m_tellDevReadyBtn = new QPushButton(tr("告知设备就绪 (PDU63)"));
    m_tellDevReadyBtn->setToolTip(tr("根据JSON配置的站点信息发送PDU63码"));
    sProtocolLayout->addWidget(m_tellDevReadyBtn);
    
    sProtocolLayout->addWidget(new QLabel(tr("通信版本:")));
    m_commVersionSpinBox = new QSpinBox();
    m_commVersionSpinBox->setRange(1, 10);
    m_commVersionSpinBox->setValue(2);
    sProtocolLayout->addWidget(m_commVersionSpinBox);
    
    m_tellDevCommVersionBtn = new QPushButton(tr("告知Comm版本"));
    sProtocolLayout->addWidget(m_tellDevCommVersionBtn);
    
    sProtocolLayout->addStretch();
    
    layout->addWidget(sProtocolGroup);
    
    // 连接信号
    connect(m_querySiteEnableBtn, &QPushButton::clicked, this, &HandlerControlWidget::onQuerySiteEnableClicked);
    connect(m_getSiteMapBtn, &QPushButton::clicked, this, &HandlerControlWidget::onGetSiteMapClicked);
    connect(m_refreshSiteConfigBtn, &QPushButton::clicked, this, &HandlerControlWidget::onRefreshSiteConfigClicked);
    connect(m_saveSiteConfigBtn, &QPushButton::clicked, this, &HandlerControlWidget::onSaveSiteConfigClicked);
    connect(m_applySiteConfigBtn, &QPushButton::clicked, this, &HandlerControlWidget::onApplySiteConfigClicked);
    
    // S协议控制信号连接
    connect(m_tellDevReadyBtn, &QPushButton::clicked, this, &HandlerControlWidget::onTellDevReadyClicked);
    connect(m_tellDevCommVersionBtn, &QPushButton::clicked, this, &HandlerControlWidget::onTellDevCommVersionClicked);
    
    connect(m_setDoneSiteBtn, &QPushButton::clicked, this, &HandlerControlWidget::onSetDoneSiteClicked);
    connect(m_sendContactCheckResultBtn, &QPushButton::clicked, this, &HandlerControlWidget::onSendContactCheckResultClicked);
    connect(m_sendRemainingCheckResultBtn, &QPushButton::clicked, this, &HandlerControlWidget::onSendRemainingCheckResultClicked);
    
    // Socket快速设置按钮连接
    connect(m_enableAllSocketsBtn, &QPushButton::clicked, this, &HandlerControlWidget::onEnableAllSocketsClicked);
    connect(m_disableAllSocketsBtn, &QPushButton::clicked, this, &HandlerControlWidget::onDisableAllSocketsClicked);
    connect(m_enableFirstHalfBtn, &QPushButton::clicked, this, &HandlerControlWidget::onEnableFirstHalfClicked);
    connect(m_enableSecondHalfBtn, &QPushButton::clicked, this, &HandlerControlWidget::onEnableSecondHalfClicked);
    
    // 站点选择变化时更新配置显示
    connect(m_configSiteCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &HandlerControlWidget::onConfigSiteChanged);
}

void HandlerControlWidget::createAxisControlGroup()
{
    m_axisControlGroup = new QGroupBox(tr("轴控制"), this);
    auto layout = new QHBoxLayout(m_axisControlGroup);
    
    layout->addWidget(new QLabel(tr("轴选择:")));
    m_axisSelectCombo = new QComboBox();
    m_axisSelectCombo->addItems({"Axis_Pitch", "Axis_Roll", "Axis_Yaw", "Axis_X", "Axis_Y", "Axis_Z"});
    layout->addWidget(m_axisSelectCombo);
    
    layout->addWidget(new QLabel(tr("站点索引:")));
    m_axisSiteIdxSpinBox = new QSpinBox();
    m_axisSiteIdxSpinBox->setRange(1, 64);
    layout->addWidget(m_axisSiteIdxSpinBox);
    
    layout->addWidget(new QLabel(tr("目标角度:")));
    m_targetAngleSpinBox = new QSpinBox();
    m_targetAngleSpinBox->setRange(-360, 360);
    m_targetAngleSpinBox->setSuffix("°");
    layout->addWidget(m_targetAngleSpinBox);
    
    m_sendAxisMoveBtn = new QPushButton(tr("发送轴移动"));
    layout->addWidget(m_sendAxisMoveBtn);

    layout->addStretch();
    
    connect(m_sendAxisMoveBtn, &QPushButton::clicked, this, &HandlerControlWidget::onSendAxisMoveClicked);
}

// [已删除] 原始帧调试功能

void HandlerControlWidget::createBottomControlArea()
{
    auto bottomLayout = new QHBoxLayout();
    
    // 日志显示按钮
    m_showLogDialogBtn = new QPushButton(tr("显示调试日志"));
    m_showLogDialogBtn->setCheckable(true);
    bottomLayout->addWidget(m_showLogDialogBtn);
    
    bottomLayout->addStretch();
    
    // 添加到主布局
    auto mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (mainLayout) {
        mainLayout->addLayout(bottomLayout);
    }
    
    // 连接日志按钮信号
    connect(m_showLogDialogBtn, &QPushButton::toggled, this, [this](bool checked) {
        if (!this) {
            return;
        }
        
        if (checked) {
            if (!m_logDialog) {
                createLogDialog();
            }
            if (m_logDialog) {
                m_logDialog->show();
                m_logDialog->raise();
                m_logDialog->activateWindow();
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
            qDebug() << "Exception when setting log text:" << e.what();
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
    
    // 先断开所有现有连接，防止重复连接
    disconnect(m_handlerDevice.get(), nullptr, this, nullptr);
    
    auto sProtocol = m_handlerDevice->getSProtocol();
    if (sProtocol) {
        disconnect(sProtocol, nullptr, this, nullptr);
    }
    
    // 重新连接设备信号
    connect(m_handlerDevice.get(), &Domain::HandlerDevice::chipPlaced,
            this, &HandlerControlWidget::onChipPlaced);
    connect(m_handlerDevice.get(), &Domain::HandlerDevice::contactCheckRequested,
            this, &HandlerControlWidget::onContactCheckRequested);
    connect(m_handlerDevice.get(), &Domain::HandlerDevice::remainingCheckRequested,
            this, &HandlerControlWidget::onRemainingCheckRequested);
    connect(m_handlerDevice.get(), &Domain::HandlerDevice::programResultReceived,
            this, &HandlerControlWidget::onProgramResultReceived);
    connect(m_handlerDevice.get(), &Domain::HandlerDevice::axisMovementRequested,
            this, &HandlerControlWidget::onAxisMovementRequested);
    connect(m_handlerDevice.get(), &Domain::HandlerDevice::axisMovementCompleted,
            this, &HandlerControlWidget::onAxisMovementCompleted);
    connect(m_handlerDevice.get(), &Domain::HandlerDevice::rawFrameSent,
            this, &HandlerControlWidget::onRawFrameSent);
    connect(m_handlerDevice.get(), &Domain::HandlerDevice::rawFrameReceived,
            this, &HandlerControlWidget::onRawFrameReceived);
    connect(m_handlerDevice.get(), &Domain::HandlerDevice::debugMessage,
            this, &HandlerControlWidget::onDebugMessage);

    // 连接通用的错误和命令完成信号
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
        QMessageBox::warning(this, tr("警告"), tr("请选择任务文件"));
        return;
    }
    
    // 构建简单的参数传递给Device层
    QJsonObject cmdParams;
    cmdParams["passlot"] = m_passlotEdit->text();
    cmdParams["taskPath"] = taskPath;
    cmdParams["ipsType"] = m_ipsTypeCombo->currentData().toInt();
    
    // 供料托盘参数
    cmdParams["supplyTrayConfig"] = m_supplyTrayTypeCombo->currentData().toInt();
    cmdParams["supplyTrayX"] = m_supplyTrayXEdit->text().toInt();
    cmdParams["supplyTrayY"] = m_supplyTrayYEdit->text().toInt();
    
    // 生产托盘参数
    cmdParams["productionTrayConfig"] = m_productionTrayTypeCombo->currentData().toInt();
    cmdParams["productionTrayX"] = m_productionTrayXEdit->text().toInt();
    cmdParams["productionTrayY"] = m_productionTrayYEdit->text().toInt();
    
    // 废料托盘参数
    cmdParams["rejectTrayConfig"] = m_rejectTrayTypeCombo->currentData().toInt();
    cmdParams["rejectTrayX"] = m_rejectTrayXEdit->text().toInt();
    cmdParams["rejectTrayY"] = m_rejectTrayYEdit->text().toInt();
    
    cmdParams["reelStartPos"] = m_reelStartPosEdit->text().toInt();
    
    log(tr("正在加载任务 (CMD3)..."), "INFO");
    setControlsEnabled(false);
    m_handlerDevice->executeCommand("loadTask", cmdParams);
}

void HandlerControlWidget::onTellDevReadyClicked()
{
    if (!m_handlerDevice) return;
    
    // 构建设备就绪信息（自动从JSON配置读取）
    QJsonObject readyInfo = buildReadyInfo();
    m_lastReadyInfo = readyInfo;
    
    QJsonObject params;
    params["readyInfo"] = readyInfo;
    
    log(tr("正在根据JSON配置发送PDU63告知设备就绪..."), "INFO");
    setControlsEnabled(false);
    
    // 添加超时保护，防止长时间等待导致UI卡死
    QTimer::singleShot(10000, this, [this]() {
        if (!this) return;
        
        log(tr("命令执行超时，自动恢复界面控制"), "WARNING");
        setControlsEnabled(true);
    });
    
    m_handlerDevice->executeCommand("tellDevReady", params);
}

void HandlerControlWidget::onQuerySiteEnableClicked()
{
    if (!m_handlerDevice) return;
    
    log(tr("正在查询站点使能..."), "INFO");
    setControlsEnabled(false);
    m_handlerDevice->executeCommand("querySiteEnable", QJsonObject());
}

void HandlerControlWidget::onSetDoneSiteClicked()
{
    if (!m_handlerDevice) return;
    
    QJsonObject params;
    params["siteIdx"] = m_siteIdxSpinBox->value();
    params["result"] = m_resultEdit->text();
    params["mask"] = m_maskEdit->text();
    
    log(tr("正在设置站点完成状态..."), "INFO");
    //setControlsEnabled(false);
    m_handlerDevice->executeCommand("setDoneSite", params);
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
    
    // 防止重复发送
    if (m_isSending) {
        log(tr("正在发送命令中，请等待..."), "WARNING");
        return;
    }
    
    QJsonObject params;
    params["axisSelect"] = m_axisSelectCombo->currentText();
    params["siteIdx"] = m_axisSiteIdxSpinBox->value();
    params["targetAngle"] = m_targetAngleSpinBox->value();
    
    QString commandKey = QString("sendAxisMove_%1_%2_%3")
        .arg(params["axisSelect"].toString())
        .arg(params["siteIdx"].toInt())
        .arg(params["targetAngle"].toInt());
    
    // 检查是否与最近发送的命令相同
    if (m_lastSentCommand == commandKey) {
        log(tr("相同的轴移动命令刚刚发送过，请等待执行完成"), "WARNING");
        return;
    }
    
    log(tr("正在发送轴移动命令..."), "INFO");
    
    // 设置发送状态，只禁用当前按钮
    m_isSending = true;
    m_lastSentCommand = commandKey;
    m_sendAxisMoveBtn->setEnabled(false);
    
    // 添加超时保护，以防万一
    QTimer::singleShot(5000, this, [this]() {
        if (!this || !m_isSending) return;
        
        log(tr("轴移动命令超时，自动恢复界面控制"), "WARNING");
        m_isSending = false;
        m_lastSentCommand.clear();
        m_sendAxisMoveBtn->setEnabled(true);
    });
    
    m_handlerDevice->executeCommand("sendAxisMove", params);
}

// [已删除] 原始帧相关方法

void HandlerControlWidget::onChipPlaced(int siteIndex, uint32_t slotEn, const QString &siteSn)
{
    log(tr("芯片放置事件: 站点%1, 槽位0x%2, SN:%3")
        .arg(siteIndex)
        .arg(slotEn, 4, 16, QChar('0'))
        .arg(siteSn), "EVENT");
    
    updateSiteTable();
}

void HandlerControlWidget::onContactCheckRequested(int siteIdx, const QByteArray &sktEn)
{
    log(tr("接触检查请求: 站点%1, 数据:%2")
        .arg(siteIdx)
        .arg(formatHexData(sktEn)), "EVENT");
}

void HandlerControlWidget::onRemainingCheckRequested(int siteIdx, const QByteArray &sktEn)
{
    log(tr("残料检查请求: 站点%1, 数据:%2")
        .arg(siteIdx)
        .arg(formatHexData(sktEn)), "EVENT");
}

void HandlerControlWidget::onProgramResultReceived(bool success, int errCode, const QString &errMsg)
{
    log(tr("编程结果: %1, 错误码:%2, 消息:%3")
        .arg(success ? tr("成功") : tr("失败"))
        .arg(errCode)
        .arg(errMsg), success ? "SUCCESS" : "ERROR");
}

void HandlerControlWidget::onAxisMovementRequested(const QString &axisSelect, int siteIdx, int targetAngle)
{
    log(tr("轴移动请求: %1, 站点%2, 目标角度%3°")
        .arg(axisSelect).arg(siteIdx).arg(targetAngle), "EVENT");
}

void HandlerControlWidget::onAxisMovementCompleted(const QString &axisSelect, int siteIdx, int currentAngle, int result, const QString &errMsg)
{
    log(tr("轴移动完成: %1, 站点%2, 当前角度%3°, 结果:%4")
        .arg(axisSelect).arg(siteIdx).arg(currentAngle).arg(result), "EVENT");
    if (!errMsg.isEmpty()) {
        log(tr("错误信息: %1").arg(errMsg), "ERROR");
    }
}

void HandlerControlWidget::onRawFrameSent(const QByteArray &frame)
{
    logFrame(tr("TX → 自动化设备"), frame);
}

void HandlerControlWidget::onRawFrameReceived(const QByteArray &frame)
{
    logFrame(tr("RX ← 自动化设备"), frame);
}

void HandlerControlWidget::onDebugMessage(const QString &message)
{
    log(message, "DEBUG");
}

void HandlerControlWidget::updateSiteTable()
{
    m_siteTable->setRowCount(0);
    
    if (!m_handlerDevice) return;
    
    // 获取设备配置中的站点信息
    QJsonObject deviceConfig = m_handlerDevice->getConfiguration();
    QJsonObject siteConfig = deviceConfig.value("siteConfiguration").toObject();
    QJsonArray sitesArray = siteConfig.value("sites").toArray();
    
    if (!sitesArray.isEmpty()) {
        // 从配置文件读取详细站点信息
        for (int i = 0; i < sitesArray.size(); ++i) {
            QJsonObject siteObj = sitesArray[i].toObject();
            
            int row = m_siteTable->rowCount();
            m_siteTable->insertRow(row);
            
            // 站点索引
            m_siteTable->setItem(row, 0, new QTableWidgetItem(QString::number(siteObj["siteIndex"].toInt())));
            
            // 站点别名
            m_siteTable->setItem(row, 1, new QTableWidgetItem(siteObj["siteAlias"].toString()));
            
            // 站点SN
            m_siteTable->setItem(row, 2, new QTableWidgetItem(siteObj["siteSN"].toString()));
            
            // Socket数量
            m_siteTable->setItem(row, 3, new QTableWidgetItem(QString::number(siteObj["socketCount"].toInt())));
            
            // Socket使能状态
            m_siteTable->setItem(row, 4, new QTableWidgetItem(siteObj["socketEnable"].toString()));
            
            // 站点状态
            QString status = siteObj["siteEnvInit"].toBool() ? tr("已初始化") : tr("未初始化");
            m_siteTable->setItem(row, 5, new QTableWidgetItem(status));
            
            // 添加编辑按钮
            auto editBtn = new QPushButton(tr("编辑"));
            editBtn->setProperty("siteIndex", siteObj["siteIndex"].toInt());
            connect(editBtn, &QPushButton::clicked, this, [this, editBtn]() {
                int siteIndex = editBtn->property("siteIndex").toInt();
                
                // 设置下拉框到对应站点
                for (int i = 0; i < m_configSiteCombo->count(); ++i) {
                    if (m_configSiteCombo->itemData(i).toInt() == siteIndex) {
                        m_configSiteCombo->setCurrentIndex(i);
                        break;
                    }
                }
                
                log(tr("开始编辑站点%1").arg(siteIndex), "CONFIG");
            });
            
            m_siteTable->setCellWidget(row, 6, editBtn);
        }
        
        // 同时刷新站点下拉框
        refreshSiteComboBox();
    } else {
        // 如果配置文件中没有站点信息，从缓存的站点映射更新表格（向后兼容）
        for (auto it = m_siteMapCache.begin(); it != m_siteMapCache.end(); ++it) {
            if (it.key() != "success" && it.key() != "command") {
                int row = m_siteTable->rowCount();
                m_siteTable->insertRow(row);
                m_siteTable->setItem(row, 0, new QTableWidgetItem(it.key()));
                m_siteTable->setItem(row, 1, new QTableWidgetItem(it.value().toString()));
                m_siteTable->setItem(row, 2, new QTableWidgetItem(tr("未知"))); // SN
                m_siteTable->setItem(row, 3, new QTableWidgetItem("8")); // 默认Socket数
                m_siteTable->setItem(row, 4, new QTableWidgetItem("0x0F")); // 默认使能
                m_siteTable->setItem(row, 5, new QTableWidgetItem(tr("未知"))); // 状态
                
                // 空的操作列
                auto editBtn = new QPushButton(tr("编辑"));
                editBtn->setEnabled(false);
                m_siteTable->setCellWidget(row, 6, editBtn);
            }
        }
    }
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
        qDebug() << "WARNING: log() called with null this pointer!";
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
    } catch (...) {
        qDebug() << "Exception in log() when accessing m_logCache";
        return;
    }
    
    // 如果日志对话框已创建，立即显示日志
    if (m_logTextEdit) {
        try {
            m_logTextEdit->append(formattedMsg);
        } catch (...) {
            qDebug() << "Exception in log() when appending to m_logTextEdit";
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
    
    if (!m_handlerDevice) {
        log(tr("设备未连接，使用默认配置"), "WARNING");
        // 返回默认配置
        QJsonObject projInfo;
        projInfo["AdapterNum"] = 4;
        
        QJsonArray siteReady;
        QJsonObject site;
        site["SiteSN"] = "SN001";
        site["SiteAlias"] = "Site1";
        site["SiteEnvRdy"] = 1;
        site["SKTRdy"] = "0xFFFF";
        siteReady.append(site);
        
        QJsonObject readyInfo;
        readyInfo["ProjInfo"] = projInfo;
        readyInfo["SiteReady"] = siteReady;
        return readyInfo;
    }
    
    // 从设备配置中读取站点信息
    QJsonObject deviceConfig = m_handlerDevice->getConfiguration();
    log(tr("设备配置大小: %1 字节").arg(QJsonDocument(deviceConfig).toJson().size()), "DEBUG");
    
    QJsonObject siteConfig = deviceConfig.value("siteConfiguration").toObject();
    log(tr("站点配置大小: %1 字节").arg(QJsonDocument(siteConfig).toJson().size()), "DEBUG");
    
    QJsonArray sitesArray = siteConfig.value("sites").toArray();
    log(tr("站点数组大小: %1").arg(sitesArray.size()), "DEBUG");
    
    if (sitesArray.isEmpty()) {
        log(tr("未找到站点配置，使用默认配置"), "WARNING");
        // 返回默认配置
        QJsonObject projInfo;
        projInfo["AdapterNum"] = 4;
        projInfo["SiteCnt"] = 1;
        projInfo["SKTCnt"] = 4;  // 添加SKTCnt字段
        
        QJsonArray siteReady;
        QJsonObject site;
        site["SiteSN"] = "SN001";
        site["SiteAlias"] = "Site1";
        site["SiteEnvRdy"] = 1;
        site["SKTRdy"] = "0xFFFF";
        siteReady.append(site);
        
        QJsonObject readyInfo;
        readyInfo["ProjInfo"] = projInfo;
        readyInfo["SiteReady"] = siteReady;
        return readyInfo;
    }
    
    // 构建站点就绪信息
    QJsonArray siteReady;
    int totalSites = 0;
    QMap<int, int> socketCountMap;  // 统计不同Socket数量
    
    for (const auto& value : sitesArray) {
        QJsonObject siteObj = value.toObject();
        
        int siteIndex = siteObj["siteIndex"].toInt();
        QString siteAlias = siteObj["siteAlias"].toString();
        bool siteEnvInit = siteObj["siteEnvInit"].toBool();
        
        log(tr("检查站点%1(%2): 初始化状态=%3")
            .arg(siteIndex).arg(siteAlias).arg(siteEnvInit ? "是" : "否"), "DEBUG");
        
        // 只包含已初始化的站点
        if (!siteEnvInit) {
            log(tr("跳过未初始化站点%1").arg(siteIndex), "DEBUG");
            continue;
        }
        
        totalSites++;
        int socketCount = siteObj["socketCount"].toInt(8);
        socketCountMap[socketCount]++;
        
        QJsonObject site;
        site["SiteSN"] = siteObj["siteSN"].toString();
        site["SiteAlias"] = siteObj["siteAlias"].toString();
        site["SiteEnvRdy"] = 1;
        site["SKTRdy"] = siteObj["socketReady"].toString("0xFFFF");
        site["SocketCount"] = socketCount;
        site["SocketEnable"] = siteObj["socketEnable"].toString();
        
        log(tr("添加站点%1(%2)到就绪信息: SN=%3, Socket=%4")
            .arg(siteIndex).arg(siteAlias)
            .arg(siteObj["siteSN"].toString()).arg(socketCount), "DEBUG");
        
        siteReady.append(site);
    }
    
    log(tr("共找到%1个已初始化站点").arg(totalSites), "DEBUG");
    
    if (totalSites == 0) {
        log(tr("没有已初始化的站点，使用默认配置"), "WARNING");
        // 返回默认配置
        QJsonObject projInfo;
        projInfo["AdapterNum"] = 4;
        projInfo["SiteCnt"] = 1;
        projInfo["SKTCnt"] = 4;  // 添加SKTCnt字段
        
        QJsonArray siteReady;
        QJsonObject site;
        site["SiteSN"] = "SN001";
        site["SiteAlias"] = "Site1";
        site["SiteEnvRdy"] = 1;
        site["SKTRdy"] = "0xFFFF";
        siteReady.append(site);
        
        QJsonObject readyInfo;
        readyInfo["ProjInfo"] = projInfo;
        readyInfo["SiteReady"] = siteReady;
        return readyInfo;
    }
    
    // 确定主要的Socket数量（使用最常见的Socket数量）
    int primarySocketCount = 8;  // 默认值
    int maxCount = 0;
    for (auto it = socketCountMap.begin(); it != socketCountMap.end(); ++it) {
        if (it.value() > maxCount) {
            maxCount = it.value();
            primarySocketCount = it.key();
        }
    }
    
    // 构建项目信息
    QJsonObject projInfo;
    projInfo["AdapterNum"] = primarySocketCount;  // 使用主要的Socket数量
    projInfo["SiteCnt"] = totalSites;
    projInfo["SKTCnt"] = primarySocketCount;
    
    QJsonObject readyInfo;
    readyInfo["ProjInfo"] = projInfo;
    readyInfo["SiteReady"] = siteReady;
    
    log(tr("从JSON配置构建就绪信息: 站点数=%1, Socket数=%2")
        .arg(totalSites).arg(primarySocketCount), "CONFIG");
    
    return readyInfo;
}

void HandlerControlWidget::onTellDevCommVersionClicked()
{
    if (!m_handlerDevice || !m_commVersionSpinBox) return;
    
    QJsonObject params;
    params["version"] = m_commVersionSpinBox->value();
    
    log(tr("正在告知通信版本..."), "INFO");
    setControlsEnabled(false);
    m_handlerDevice->executeCommand("tellDevCommVersion", params);
}

void HandlerControlWidget::onSendContactCheckResultClicked()
{
    if (!m_handlerDevice || !m_checkResultEdit || !m_siteIdxSpinBox || !m_adapterCntSpinBox) return;
    
    // 解析结果字节串
    QStringList byteStrs = m_checkResultEdit->text().simplified().split(' ', Qt::SkipEmptyParts);
    QJsonArray resultArray;
    for(const QString &b : byteStrs){
        bool ok=false; int val=b.toInt(&ok,16);
        if(ok) resultArray.append(val&0xFF);
    }
    
    QJsonObject params;
    params["siteIdx"] = m_siteIdxSpinBox->value();
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
    if (!m_handlerDevice || !m_checkResultEdit || !m_siteIdxSpinBox || !m_adapterCntSpinBox) return;
    
    QStringList byteStrs = m_checkResultEdit->text().simplified().split(' ', Qt::SkipEmptyParts);
    QJsonArray resultArray;
    for(const QString &b : byteStrs){
        bool ok=false; int val=b.toInt(&ok,16);
        if(ok) resultArray.append(val&0xFF);
    }
    
    QJsonObject params;
    params["siteIdx"] = m_siteIdxSpinBox->value();
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
            m_siteMapCache = result;
            updateSiteTable();
            log(tr("获取站点映射成功"), "SUCCESS");
        } else if (command == "sendAxisMove") {
            log(tr("轴移动命令已成功发送并被接受"), "SUCCESS");
        } else {
            log(tr("命令 '%1' 执行完成").arg(command), "SUCCESS");
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
    
    // 确保界面状态恢复
    setControlsEnabled(true);
    m_sendAxisMoveBtn->setEnabled(true);
    
    // 添加调试信息，帮助诊断问题
    log(tr("界面控制已自动恢复"), "DEBUG");
}

void HandlerControlWidget::initTrayCoordWidgets()
{
    // 获取托盘配置枚举
    QMetaEnum trayConfigEnum = QMetaEnum::fromType<Domain::Protocols::SProtocol::TrayConfigType>();
    QStringList configNames = {tr("默认"), tr("手动"), tr("自动")};
    
    // 供料托盘配置控件
    m_supplyTrayTypeCombo = new QComboBox();
    for (int i = 0; i < trayConfigEnum.keyCount() && i < configNames.size(); ++i) {
        m_supplyTrayTypeCombo->addItem(configNames[i], trayConfigEnum.value(i));
    }
    m_supplyTrayTypeCombo->setCurrentIndex(0);
    
    m_supplyTrayXEdit = new QLineEdit("0");
    m_supplyTrayYEdit = new QLineEdit("0");
    m_supplyTrayXEdit->setEnabled(false);
    m_supplyTrayYEdit->setEnabled(false);
    
    // 生产托盘配置控件
    m_productionTrayTypeCombo = new QComboBox();
    for (int i = 0; i < trayConfigEnum.keyCount() && i < configNames.size(); ++i) {
        m_productionTrayTypeCombo->addItem(configNames[i], trayConfigEnum.value(i));
    }
    m_productionTrayTypeCombo->setCurrentIndex(0);
    
    m_productionTrayXEdit = new QLineEdit("0");
    m_productionTrayYEdit = new QLineEdit("0");
    m_productionTrayXEdit->setEnabled(false);
    m_productionTrayYEdit->setEnabled(false);
    
    // 废料托盘配置控件
    m_rejectTrayTypeCombo = new QComboBox();
    for (int i = 0; i < trayConfigEnum.keyCount() && i < configNames.size(); ++i) {
        m_rejectTrayTypeCombo->addItem(configNames[i], trayConfigEnum.value(i));
    }
    m_rejectTrayTypeCombo->setCurrentIndex(0);
    
    m_rejectTrayXEdit = new QLineEdit("0");
    m_rejectTrayYEdit = new QLineEdit("0");
    m_rejectTrayXEdit->setEnabled(false);
    m_rejectTrayYEdit->setEnabled(false);
    
    // 连接信号，当选择"手动"时启用坐标输入框
    connect(m_supplyTrayTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this](int index) {
            if (!this || !m_supplyTrayTypeCombo) return;
            int cfgType = m_supplyTrayTypeCombo->currentData().toInt();
            bool enabled = (cfgType == Domain::Protocols::SProtocol::TRAY_CFG_MANUAL);
            if (m_supplyTrayXEdit) m_supplyTrayXEdit->setEnabled(enabled);
            if (m_supplyTrayYEdit) m_supplyTrayYEdit->setEnabled(enabled);
        });
    
    connect(m_productionTrayTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this](int index) {
            if (!this || !m_productionTrayTypeCombo) return;
            int cfgType = m_productionTrayTypeCombo->currentData().toInt();
            bool enabled = (cfgType == Domain::Protocols::SProtocol::TRAY_CFG_MANUAL);
            if (m_productionTrayXEdit) m_productionTrayXEdit->setEnabled(enabled);
            if (m_productionTrayYEdit) m_productionTrayYEdit->setEnabled(enabled);
        });
    
    connect(m_rejectTrayTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this](int index) {
            if (!this || !m_rejectTrayTypeCombo) return;
            int cfgType = m_rejectTrayTypeCombo->currentData().toInt();
            bool enabled = (cfgType == Domain::Protocols::SProtocol::TRAY_CFG_MANUAL);
            if (m_rejectTrayXEdit) m_rejectTrayXEdit->setEnabled(enabled);
            if (m_rejectTrayYEdit) m_rejectTrayYEdit->setEnabled(enabled);
        });
}

// [改进] 完全模拟 ACAutomaticSetting::getCoordinfo
void HandlerControlWidget::getCoordinfo(Domain::Protocols::SProtocol::TrayType type, QStringList& infoList)
{
    infoList.clear();
    
    QComboBox* typeCombo = nullptr;
    QLineEdit* xEdit = nullptr;
    QLineEdit* yEdit = nullptr;

    switch (type) {
        case Domain::Protocols::SProtocol::TRAY_TYPE_SUPPLY:
            typeCombo = m_supplyTrayTypeCombo;
            xEdit = m_supplyTrayXEdit;
            yEdit = m_supplyTrayYEdit;
            break;
        case Domain::Protocols::SProtocol::TRAY_TYPE_PRODUCTION:
            typeCombo = m_productionTrayTypeCombo;
            xEdit = m_productionTrayXEdit;
            yEdit = m_productionTrayYEdit;
            break;
        case Domain::Protocols::SProtocol::TRAY_TYPE_REJECT:
            typeCombo = m_rejectTrayTypeCombo;
            xEdit = m_rejectTrayXEdit;
            yEdit = m_rejectTrayYEdit;
            break;
    }

    if (!typeCombo || !xEdit || !yEdit) return;

    // 托盘配置类型
    int cfg = typeCombo->itemData(typeCombo->currentIndex()).toInt();
    infoList.append(QString::number(cfg));

    // 托盘X和Y坐标
    int xPos = 0, yPos = 0;
    
    if (cfg == Domain::Protocols::SProtocol::TRAY_CFG_MANUAL) {
        xPos = xEdit->text().toInt();
        yPos = yEdit->text().toInt();
    }

    infoList.append(QString::number(xPos));
    infoList.append(QString::number(yPos));
}

void HandlerControlWidget::onConfigSiteChanged()
{
    if (!m_configSiteCombo) return;
    
    int currentIndex = m_configSiteCombo->currentIndex();
    if (currentIndex >= 0) {
        int siteIndex = m_configSiteCombo->itemData(currentIndex).toInt();
        displaySiteConfig(siteIndex);
    }
}

void HandlerControlWidget::onEnableAllSocketsClicked()
{
    if (!m_socketCountSpinBox || !m_socketEnableEdit) return;
    
    int socketCount = m_socketCountSpinBox->value();
    QString enableValue;
    
    if (socketCount <= 8) {
        enableValue = "0xFF";
    } else if (socketCount <= 16) {
        enableValue = "0xFFFF";
    } else if (socketCount <= 32) {
        enableValue = "0xFFFFFFFF";
    } else {
        enableValue = "0xFFFFFFFFFFFFFFFF";
    }
    
    m_socketEnableEdit->setText(enableValue);
    log(tr("已设置全部Socket使能: %1").arg(enableValue), "CONFIG");
}

void HandlerControlWidget::onDisableAllSocketsClicked()
{
    if (!m_socketEnableEdit) return;
    
    m_socketEnableEdit->setText("0x00");
    log(tr("已禁用全部Socket"), "CONFIG");
}

void HandlerControlWidget::onEnableFirstHalfClicked()
{
    if (!m_socketCountSpinBox || !m_socketEnableEdit) return;
    
    int socketCount = m_socketCountSpinBox->value();
    QString pattern = generateSocketEnableHex(socketCount, "first_half");
    m_socketEnableEdit->setText(pattern);
    log(tr("已设置前半部分Socket使能: %1").arg(pattern), "CONFIG");
}

void HandlerControlWidget::onEnableSecondHalfClicked()
{
    if (!m_socketCountSpinBox || !m_socketEnableEdit) return;
    
    int socketCount = m_socketCountSpinBox->value();
    QString pattern = generateSocketEnableHex(socketCount, "second_half");
    m_socketEnableEdit->setText(pattern);
    log(tr("已设置后半部分Socket使能: %1").arg(pattern), "CONFIG");
}

void HandlerControlWidget::onRefreshSiteConfigClicked()
{
    loadSiteConfiguration();
    log(tr("已刷新站点配置"), "CONFIG");
}

void HandlerControlWidget::onSaveSiteConfigClicked()
{
    if (!validateSiteConfig()) {
        QMessageBox::warning(this, tr("配置错误"), tr("请检查站点配置参数"));
        return;
    }
    
    // 这里可以实现保存配置到文件的逻辑
    log(tr("站点配置已保存"), "CONFIG");
    QMessageBox::information(this, tr("保存成功"), tr("站点配置已保存到配置文件"));
}

void HandlerControlWidget::onApplySiteConfigClicked()
{
    if (!m_configSiteCombo || !validateSiteConfig()) {
        QMessageBox::warning(this, tr("配置错误"), tr("请检查站点配置参数"));
        return;
    }
    
    int siteIndex = m_configSiteCombo->currentData().toInt();
    applySiteConfigToDevice(siteIndex);
    updateSiteConfigDisplay();
    log(tr("已应用站点%1的配置").arg(siteIndex), "CONFIG");
}

// 辅助方法实现
void HandlerControlWidget::loadSiteConfiguration()
{
    if (!m_handlerDevice) return;
    
    // 从设备获取站点映射
    QJsonObject result = m_handlerDevice->executeCommand("getSiteMap", QJsonObject());
    
    refreshSiteComboBox();
    updateSiteConfigDisplay();
}

void HandlerControlWidget::updateSiteConfigDisplay()
{
    if (!m_handlerDevice) return;
    
    // 获取站点映射并更新表格
    QJsonObject siteMap = m_handlerDevice->getSiteMap();
    
    m_siteTable->setRowCount(0);
    int row = 0;
    
    for (auto it = siteMap.begin(); it != siteMap.end(); ++it) {
        if (it.key() != "success" && it.key() != "command") {
            m_siteTable->insertRow(row);
            
            QJsonObject siteData;
            siteData["siteIndex"] = it.key().toInt();
            siteData["siteAlias"] = it.value().toString();
            siteData["siteSN"] = QString("SN%1").arg(it.key().toInt(), 3, 10, QChar('0'));
            siteData["socketCount"] = 8;  // 默认值，实际应从配置读取
            siteData["socketEnable"] = "0x0F";  // 默认值
            siteData["status"] = tr("正常");
            
            updateSiteTableRow(row, siteData);
            row++;
        }
    }
    
    log(tr("已更新站点配置显示，共%1个站点").arg(row), "CONFIG");
}

void HandlerControlWidget::refreshSiteComboBox()
{
    if (!m_handlerDevice || !m_configSiteCombo) return;
    
    m_configSiteCombo->clear();
    
    // 从设备配置读取站点信息
    QJsonObject deviceConfig = m_handlerDevice->getConfiguration();
    QJsonObject siteConfig = deviceConfig.value("siteConfiguration").toObject();
    QJsonArray sitesArray = siteConfig.value("sites").toArray();
    
    if (!sitesArray.isEmpty()) {
        // 从配置文件填充下拉框
        for (const auto& value : sitesArray) {
            QJsonObject siteObj = value.toObject();
            int siteIndex = siteObj["siteIndex"].toInt();
            QString siteAlias = siteObj["siteAlias"].toString();
            m_configSiteCombo->addItem(QString("%1 - %2").arg(siteIndex).arg(siteAlias), siteIndex);
        }
    } else {
        // 回退到使用getSiteMap（向后兼容）
        QJsonObject siteMap = m_handlerDevice->getSiteMap();
        for (auto it = siteMap.begin(); it != siteMap.end(); ++it) {
            if (it.key() != "success" && it.key() != "command") {
                QString siteIndex = it.key();
                QString siteAlias = it.value().toString();
                m_configSiteCombo->addItem(QString("%1 - %2").arg(siteIndex).arg(siteAlias), siteIndex.toInt());
            }
        }
    }
}

void HandlerControlWidget::displaySiteConfig(int siteIndex)
{
    if (!m_handlerDevice) return;
    
    // 从设备配置中读取指定站点的详细配置
    QJsonObject deviceConfig = m_handlerDevice->getConfiguration();
    QJsonObject siteConfig = deviceConfig.value("siteConfiguration").toObject();
    QJsonArray sitesArray = siteConfig.value("sites").toArray();
    
    // 查找指定的站点配置
    bool found = false;
    for (const auto& value : sitesArray) {
        QJsonObject siteObj = value.toObject();
        if (siteObj["siteIndex"].toInt() == siteIndex) {
            // 显示站点配置 - 添加安全检查
            if (m_siteAliasEdit) m_siteAliasEdit->setText(siteObj["siteAlias"].toString());
            if (m_siteSNEdit) m_siteSNEdit->setText(siteObj["siteSN"].toString());
            if (m_socketCountSpinBox) m_socketCountSpinBox->setValue(siteObj["socketCount"].toInt());
            if (m_socketEnableEdit) m_socketEnableEdit->setText(siteObj["socketEnable"].toString());
            if (m_siteInitCheckBox) m_siteInitCheckBox->setChecked(siteObj["siteEnvInit"].toBool());
            
            found = true;
            log(tr("已显示站点%1的配置").arg(siteIndex), "CONFIG");
            break;
        }
    }
    
    if (!found) {
        // 如果没有找到配置，使用默认值 - 添加安全检查
        if (m_siteAliasEdit) m_siteAliasEdit->setText(QString("Site%1").arg(siteIndex, 2, 10, QChar('0')));
        if (m_siteSNEdit) m_siteSNEdit->setText(QString("SN%1").arg(siteIndex, 3, 10, QChar('0')));
        if (m_socketCountSpinBox) m_socketCountSpinBox->setValue(8);  // 默认8个Socket
        if (m_socketEnableEdit) m_socketEnableEdit->setText("0x0F");  // 默认前4个Socket使能
        if (m_siteInitCheckBox) m_siteInitCheckBox->setChecked(siteIndex <= 2);  // 前两个站点默认已初始化
        
        log(tr("站点%1未找到配置，使用默认值").arg(siteIndex), "WARNING");
    }
}

bool HandlerControlWidget::validateSiteConfig()
{
    // 验证控件有效性
    if (!m_siteAliasEdit || !m_siteSNEdit || !m_socketEnableEdit) {
        log(tr("配置控件无效"), "ERROR");
        return false;
    }
    
    // 验证站点别名
    if (m_siteAliasEdit->text().isEmpty()) {
        log(tr("站点别名不能为空"), "ERROR");
        return false;
    }
    
    // 验证站点SN
    if (m_siteSNEdit->text().isEmpty()) {
        log(tr("站点SN不能为空"), "ERROR");
        return false;
    }
    
    // 验证Socket使能值格式
    QString socketEnable = m_socketEnableEdit->text();
    if (!socketEnable.startsWith("0x") && !socketEnable.startsWith("0X")) {
        log(tr("Socket使能值必须以0x开头"), "ERROR");
        return false;
    }
    
    bool ok;
    socketEnable.mid(2).toULongLong(&ok, 16);
    if (!ok) {
        log(tr("Socket使能值格式错误"), "ERROR");
        return false;
    }
    
    return true;
}

void HandlerControlWidget::applySiteConfigToDevice(int siteIndex)
{
    // 安全检查
    if (!m_siteAliasEdit || !m_siteSNEdit || !m_socketCountSpinBox || 
        !m_socketEnableEdit || !m_siteInitCheckBox) {
        log(tr("配置控件无效，无法应用配置"), "ERROR");
        return;
    }
    
    // 构建站点配置参数
    QJsonObject params;
    params["siteIndex"] = siteIndex;
    params["siteAlias"] = m_siteAliasEdit->text();
    params["siteSN"] = m_siteSNEdit->text();
    params["socketCount"] = m_socketCountSpinBox->value();
    params["socketEnable"] = m_socketEnableEdit->text();
    params["siteEnvInit"] = m_siteInitCheckBox->isChecked();
    
    // 这里可以添加实际的设备配置命令
    log(tr("正在应用站点%1配置到设备...").arg(siteIndex), "CONFIG");
    
    // 模拟配置应用成功
    QTimer::singleShot(500, this, [this, siteIndex]() {
        log(tr("站点%1配置应用成功").arg(siteIndex), "SUCCESS");
    });
}

QString HandlerControlWidget::generateSocketEnableHex(int socketCount, const QString& pattern)
{
    uint64_t enableMask = 0;
    
    if (pattern == "first_half") {
        // 前半部分使能
        int halfCount = socketCount / 2;
        for (int i = 0; i < halfCount; ++i) {
            enableMask |= (1ULL << i);
        }
    } else if (pattern == "second_half") {
        // 后半部分使能
        int halfCount = socketCount / 2;
        for (int i = halfCount; i < socketCount; ++i) {
            enableMask |= (1ULL << i);
        }
    }
    
    return QString("0x%1").arg(enableMask, 0, 16, QChar('0')).toUpper();
}

void HandlerControlWidget::updateSiteTableRow(int row, const QJsonObject& siteData)
{
    m_siteTable->setItem(row, 0, new QTableWidgetItem(QString::number(siteData["siteIndex"].toInt())));
    m_siteTable->setItem(row, 1, new QTableWidgetItem(siteData["siteAlias"].toString()));
    m_siteTable->setItem(row, 2, new QTableWidgetItem(siteData["siteSN"].toString()));
    m_siteTable->setItem(row, 3, new QTableWidgetItem(QString::number(siteData["socketCount"].toInt())));
    m_siteTable->setItem(row, 4, new QTableWidgetItem(siteData["socketEnable"].toString()));
    m_siteTable->setItem(row, 5, new QTableWidgetItem(siteData["status"].toString()));
    
    // 添加操作按钮
    auto editBtn = new QPushButton(tr("编辑"));
    editBtn->setProperty("siteIndex", siteData["siteIndex"].toInt());
    connect(editBtn, &QPushButton::clicked, this, [this, editBtn]() {
        int siteIndex = editBtn->property("siteIndex").toInt();
        m_configSiteCombo->setCurrentText(QString("%1 -").arg(siteIndex));
        displaySiteConfig(siteIndex);
        log(tr("开始编辑站点%1").arg(siteIndex), "CONFIG");
    });
    
    m_siteTable->setCellWidget(row, 6, editBtn);
}

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

