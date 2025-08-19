#include "presentation/DeviceManagerDialog.h"
#include "presentation/IDeviceControlWidget.h"
#include "presentation/DeviceControlWidgetFactory.h"
#include "core/CoreEngine.h"
#include "services/DeviceManager.h"
#include "domain/IDevice.h"
#include "domain/HandlerDevice.h"
#include "infrastructure/ICommunicationChannel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QTabWidget>
#include <QStackedWidget>
#include <QScrollArea>
#include <QComboBox>
#include <QTimer>
#include <QHeaderView>
#include <QMessageBox>
#include <QLabel>
#include <QFormLayout>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QtAlgorithms>
#include <QDebug>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QEventLoop>

namespace Presentation {

DeviceManagerDialog::DeviceManagerDialog(std::shared_ptr<Core::CoreEngine> coreEngine, QWidget *parent)
    : QDialog(parent)
    , m_coreEngine(coreEngine)
    , m_updateTimer(new QTimer(this))
{
    setWindowTitle("设备管理器");
    
    // 设置更灵活的大小策略
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // 设置一个合理的初始大小，但不限制最大大小
    resize(1200, 800);  // 初始大小，用户可以调整
    setMinimumSize(800, 600);   // 设置一个更小的最小值，允许更大的灵活性

    setupUi();
    connectSignals();

    // m_updateTimer->setInterval(1000); // 1秒更新一次
    // m_updateTimer->start(); // [临时修复] 注释掉定时器，以避免在底层COM插件初始化失败时，持续访问损坏的对象导致崩溃。

    onRefreshDevices();
}

DeviceManagerDialog::~DeviceManagerDialog()
{
    // 断开所有控件与设备的连接
    // 这样可以防止设备在控件销毁后仍然发送信号
    for (auto it = m_controlWidgets.begin(); it != m_controlWidgets.end(); ++it) {
        IDeviceControlWidget* widget = it.value();
        
        if (widget) {
            // 断开设备与控件的所有连接
            auto device = widget->getDevice();
            
            if (device) {
                // 断开设备到控件的所有信号连接
                disconnect(device.get(), nullptr, widget, nullptr);
                
                // 如果是HandlerDevice，确保断开其内部协议的信号
                auto handlerDevice = std::dynamic_pointer_cast<Domain::HandlerDevice>(device);
                if (handlerDevice) {
                    auto sProtocol = handlerDevice->getSProtocol();
                    if (sProtocol) {
                        disconnect(sProtocol, nullptr, widget, nullptr);
                    }
                }
            }
            
            // 将设备从控件中清除，防止控件析构时再次访问设备
            widget->setDevice(nullptr);
        }
    }

    // [修复] 首先断开CoreEngine，防止任何异步回调访问
    m_coreEngine = nullptr;
    
    // 停止定时器，防止在析构过程中调用槽函数。
    if (m_updateTimer) {
        m_updateTimer->stop();
        // 断开定时器的所有连接，确保即使有待处理的事件也不会被执行
        disconnect(m_updateTimer, nullptr, this, nullptr);
    }
    
    // 断开所有信号连接，防止在析构过程中触发槽函数
    disconnect();
    
    // 清理映射关系
    m_controlWidgets.clear();
    
    // 让Qt的父子机制自动处理控件销毁
    // 不需要手动删除或使用deleteLater
}

void DeviceManagerDialog::setupUi()
{
    auto mainLayout = new QVBoxLayout(this);
    m_tabWidget = new QTabWidget(this);

    // 创建两个标签页
    createDeviceListTab();
    createDeviceControlTab();

    mainLayout->addWidget(m_tabWidget);
}

void DeviceManagerDialog::createDeviceListTab()
{
    auto deviceListPage = new QWidget();
    auto layout = new QVBoxLayout(deviceListPage);

    // 设备列表 - 使用deviceListPage作为父对象
    m_deviceTable = new QTableWidget(deviceListPage);
    m_deviceTable->setColumnCount(4);
    m_deviceTable->setHorizontalHeaderLabels({"ID", "类型", "状态", "描述"});
    m_deviceTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_deviceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_deviceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_deviceTable, 1);

    // 设备详情 - 使用deviceListPage作为父对象
    m_deviceDetailsGroup = new QGroupBox("设备详情", deviceListPage);
    m_deviceDetailsGroup->setLayout(new QFormLayout());
    layout->addWidget(m_deviceDetailsGroup);

    // 控制按钮 - 使用deviceListPage作为父对象
    auto buttonLayout = new QHBoxLayout();
    m_connectButton = new QPushButton("连接", deviceListPage);
    m_disconnectButton = new QPushButton("断开", deviceListPage);
    m_testButton = new QPushButton("测试", deviceListPage);
    m_refreshButton = new QPushButton("刷新列表", deviceListPage);
    m_openControlButton = new QPushButton("打开控制面板", deviceListPage);

    buttonLayout->addWidget(m_refreshButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_connectButton);
    buttonLayout->addWidget(m_disconnectButton);
    buttonLayout->addWidget(m_testButton);
    buttonLayout->addWidget(m_openControlButton);
    layout->addLayout(buttonLayout);

    m_tabWidget->addTab(deviceListPage, "设备列表");
}

void DeviceManagerDialog::createDeviceControlTab()
{
    auto deviceControlPage = new QWidget();
    auto layout = new QVBoxLayout(deviceControlPage);

    // 设备选择器
    auto selectorLayout = new QHBoxLayout();
    selectorLayout->addWidget(new QLabel("选择设备:", deviceControlPage));
    m_deviceSelector = new QComboBox(deviceControlPage);
    selectorLayout->addWidget(m_deviceSelector, 1);
    layout->addLayout(selectorLayout);

    // 控制界面堆栈 - 使用deviceControlPage作为父对象，添加滚动支持
    m_controlStack = new QStackedWidget(deviceControlPage);
    
    // 为控制界面添加滚动区域支持
    auto scrollArea = new QScrollArea(deviceControlPage);
    scrollArea->setWidget(m_controlStack);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    layout->addWidget(scrollArea, 1);

    m_tabWidget->addTab(deviceControlPage, "设备控制");
}

void DeviceManagerDialog::connectSignals()
{
    // 设备列表操作
    connect(m_refreshButton, &QPushButton::clicked, this, &DeviceManagerDialog::onRefreshDevices);
    connect(m_deviceTable, &QTableWidget::itemSelectionChanged, this, &DeviceManagerDialog::onDeviceSelectionChanged);
    connect(m_connectButton, &QPushButton::clicked, this, &DeviceManagerDialog::onConnectDevice);
    connect(m_disconnectButton, &QPushButton::clicked, this, &DeviceManagerDialog::onDisconnectDevice);
    connect(m_testButton, &QPushButton::clicked, this, &DeviceManagerDialog::onTestDevice);
    connect(m_openControlButton, &QPushButton::clicked, this, &DeviceManagerDialog::onOpenDeviceControl);
    
    // 定时器
    connect(m_updateTimer, &QTimer::timeout, this, &DeviceManagerDialog::onUpdateTimer);
    
    // 设备控制
    connect(m_deviceSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){
        if (index >= 0) {
            QString deviceId = m_deviceSelector->itemData(index).toString();
            showDeviceControl(deviceId);
        }
    });
}

void DeviceManagerDialog::onRefreshDevices()
{
    updateDeviceList();
    updateButtonStates();
}

void DeviceManagerDialog::updateDeviceList()
{
    auto deviceManager = m_coreEngine->getDeviceManager();
    if (!deviceManager) return;

    QString previouslySelectedId = m_selectedDeviceId;

    m_deviceTable->blockSignals(true);

    auto deviceIds = deviceManager->getAllDeviceIds();
    m_deviceTable->setRowCount(0);
    m_deviceTable->setRowCount(deviceIds.size());

    int rowToSelect = -1;
    int row = 0;
    for (const auto& deviceId : deviceIds) {
        auto device = deviceManager->getDevice(deviceId);
        if (!device) continue;

        if (deviceId == previouslySelectedId) {
            rowToSelect = row;
        }

        m_deviceTable->setItem(row, 0, new QTableWidgetItem(deviceId));

        QString typeStr;
        switch(device->getType()) {
            case Domain::IDevice::DeviceType::Handler: typeStr = "Handler"; break;
            case Domain::IDevice::DeviceType::Turntable: typeStr = "转台"; break;
            case Domain::IDevice::DeviceType::TestBoard: typeStr = "测试板"; break;
            case Domain::IDevice::DeviceType::Unknown: typeStr = "未知"; break;
        }
        m_deviceTable->setItem(row, 1, new QTableWidgetItem(typeStr));
        
        QString statusStr;
        switch(device->getStatus()) {
            case Domain::IDevice::DeviceStatus::Disconnected: statusStr = "未连接"; break;
            case Domain::IDevice::DeviceStatus::Connected: statusStr = "已连接 (VAuto)"; break;
            case Domain::IDevice::DeviceStatus::Initializing: statusStr = "初始化中"; break;
            case Domain::IDevice::DeviceStatus::Ready: statusStr = "就绪 (等待VAuto)"; break;
            case Domain::IDevice::DeviceStatus::Busy: statusStr = "忙碌"; break;
            case Domain::IDevice::DeviceStatus::Error: statusStr = "错误"; break;
        }
        m_deviceTable->setItem(row, 2, new QTableWidgetItem(statusStr));
        m_deviceTable->setItem(row, 3, new QTableWidgetItem(device->getDescription()));
        row++;
    }

    if (rowToSelect != -1) {
        m_deviceTable->selectRow(rowToSelect);
    } else {
        m_selectedDeviceId.clear();
    }
    
    m_deviceTable->blockSignals(false);
}

void DeviceManagerDialog::onDeviceSelectionChanged()
{
    auto selectedItems = m_deviceTable->selectedItems();
    if (selectedItems.isEmpty()) {
        m_selectedDeviceId.clear();
    } else {
        m_selectedDeviceId = m_deviceTable->item(selectedItems.first()->row(), 0)->text();
    }
    updateDeviceDetails();
    updateButtonStates();
}

void DeviceManagerDialog::updateDeviceDetails()
{
    // [修复] 采用更安全、原子化的方式更新UI，防止因定时器重入导致崩溃。
    // 1. 禁用并隐藏，防止在更新期间发生重绘或用户交互。
    m_deviceDetailsGroup->setEnabled(false);
    m_deviceDetailsGroup->hide();

    // 2. 安全地删除旧布局及其所有子控件。
    QLayout* oldLayout = m_deviceDetailsGroup->layout();
    if (oldLayout) {
        // 创建一个临时QWidget来接管旧布局的所有权，
        // 然后在其析构时自动、安全地销毁所有子控件。
        QWidget tempWidgetOwner;
        tempWidgetOwner.setLayout(oldLayout);
    }

    // 3. 创建一个全新的布局。
    auto newLayout = new QFormLayout();
    m_deviceDetailsGroup->setLayout(newLayout);

    if (m_selectedDeviceId.isEmpty()) {
        m_deviceDetailsGroup->show();
        m_deviceDetailsGroup->setEnabled(true);
        return;
    }

    auto device = m_coreEngine->getDeviceManager()->getDevice(m_selectedDeviceId);
    if (!device) {
        m_deviceDetailsGroup->show();
        m_deviceDetailsGroup->setEnabled(true);
        return;
    }

    // 获取当前行索引
    int currentRow = m_deviceTable->currentRow();
    if (currentRow < 0 || currentRow >= m_deviceTable->rowCount()) {
        m_deviceDetailsGroup->show();
        m_deviceDetailsGroup->setEnabled(true);
        return; // 无效的行索引
    }
    
    // 4. 用新内容填充新布局。
    newLayout->addRow("ID:", new QLabel(m_selectedDeviceId));
    newLayout->addRow("名称:", new QLabel(device->getName()));
    
    // 安全获取表格项
    auto typeItem = m_deviceTable->item(currentRow, 1);
    auto statusItem = m_deviceTable->item(currentRow, 2);
    
    if (typeItem) {
        newLayout->addRow("类型:", new QLabel(typeItem->text()));
    }
    if (statusItem) {
        newLayout->addRow("状态:", new QLabel(statusItem->text()));
    }
    
    newLayout->addRow("描述:", new QLabel(device->getDescription()));
    
    auto channel = device->getCommunicationChannel();
    if(channel) {
        auto params = channel->getParameters();
        for(auto it = params.constBegin(); it != params.constEnd(); ++it) {
            newLayout->addRow(it.key() + ":", new QLabel(it.value().toString()));
        }
    }

    // 5. 重新启用并显示。
    m_deviceDetailsGroup->setEnabled(true);
    m_deviceDetailsGroup->show();
}

void DeviceManagerDialog::updateButtonStates()
{
    if (m_selectedDeviceId.isEmpty()) {
        m_connectButton->setEnabled(false);
        m_disconnectButton->setEnabled(false);
        m_testButton->setEnabled(false);
        m_openControlButton->setEnabled(false);
        return;
    }

    auto device = m_coreEngine->getDeviceManager()->getDevice(m_selectedDeviceId);
    if (!device) return;

    bool isInitialized = m_coreEngine->getDeviceManager()->isDeviceInitialized(m_selectedDeviceId);

    m_connectButton->setEnabled(!isInitialized);
    m_disconnectButton->setEnabled(isInitialized);
    m_testButton->setEnabled(isInitialized);
    
    bool isControlSupported = DeviceControlWidgetFactory::isSupported(device->getType());
    m_openControlButton->setEnabled(isInitialized && isControlSupported);
}

void DeviceManagerDialog::onConnectDevice()
{
    if (m_selectedDeviceId.isEmpty()) return;
    m_coreEngine->getDeviceManager()->initializeDevice(m_selectedDeviceId);
    updateDeviceList();
    updateButtonStates();
}

void DeviceManagerDialog::onDisconnectDevice()
{
    if (m_selectedDeviceId.isEmpty()) return;
    m_coreEngine->getDeviceManager()->releaseDevice(m_selectedDeviceId);
    updateDeviceList();
    updateButtonStates();
}

void DeviceManagerDialog::onTestDevice()
{
    if (m_selectedDeviceId.isEmpty()) return;

    auto device = m_coreEngine->getDeviceManager()->getDevice(m_selectedDeviceId);
    if (!device) return;

    m_testButton->setEnabled(false);
    QMessageBox::information(this, "设备测试", "设备自检正在进行，请稍候...");

    auto future = QtConcurrent::run([device]() {
        // 在后台线程运行自检
        return device->selfTest();
    });

    auto watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher]() {
        bool testResult = watcher->result();
        QMessageBox::information(this, "设备测试结果", testResult ? "设备自检通过。" : "设备自检失败。");
        watcher->deleteLater();
        updateButtonStates();
    });
    watcher->setFuture(future);
}

void DeviceManagerDialog::onOpenDeviceControl()
{
    if (m_selectedDeviceId.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择一个设备");
        return;
    }
    
    onDeviceControlRequested(m_selectedDeviceId);
}

void DeviceManagerDialog::onDeviceControlRequested(const QString &deviceId)
{
    auto device = m_coreEngine->getDeviceManager()->getDevice(deviceId);
    if (!device || !DeviceControlWidgetFactory::isSupported(device->getType())) {
        QMessageBox::warning(this, "错误", "该设备不支持控制面板或设备不存在。");
        return;
    }

    // 切换到设备控制标签页
    m_tabWidget->setCurrentWidget(m_tabWidget->widget(1));
    
    // 如果设备不在选择器中，添加它
    int index = m_deviceSelector->findData(deviceId);
    if (index == -1) {
        m_deviceSelector->addItem(QString("%1 (%2)").arg(device->getName()).arg(deviceId), QVariant(deviceId));
        index = m_deviceSelector->count() - 1;
    }
    
    // 切换选择器
    m_deviceSelector->setCurrentIndex(index);
    
    // 显示对应的控制界面
    showDeviceControl(deviceId);
}

void DeviceManagerDialog::showDeviceControl(const QString &deviceId)
{
    // 如果已经创建了控件，直接显示
    if (m_controlWidgets.contains(deviceId)) {
        m_controlStack->setCurrentWidget(m_controlWidgets[deviceId]);
        return;
    }

    // 否则，创建新的控制界面
    auto device = m_coreEngine->getDeviceManager()->getDevice(deviceId);
    if (!device) return;

    auto controlWidget = getOrCreateControlWidget(device);
    if (!controlWidget) return;

    m_controlStack->setCurrentWidget(controlWidget);
}

IDeviceControlWidget* DeviceManagerDialog::getOrCreateControlWidget(std::shared_ptr<Domain::IDevice> device)
{
    if (!device) return nullptr;

    auto deviceManager = m_coreEngine->getDeviceManager();
    if(!deviceManager) return nullptr;

    // 通过遍历设备列表找到正确的deviceId，而不是通过可能不唯一的设备名称
    QString deviceId;
    auto allDeviceIds = deviceManager->getAllDeviceIds();
    for (const auto& id : allDeviceIds) {
        if (deviceManager->getDevice(id) == device) {
            deviceId = id;
            break;
        }
    }

    if (deviceId.isEmpty()) {
        // 如果设备来自管理器，这理论上不应该发生
        return nullptr;
    }

    if (m_controlWidgets.contains(deviceId)) {
        return m_controlWidgets[deviceId];
    }

    auto widget = DeviceControlWidgetFactory::createControlWidget(device->getType(), this);
    if (!widget) {
        return nullptr;
    }

    widget->setDevice(device);
    IDeviceControlWidget* widgetPtr = widget.release(); // 转移所有权

    m_controlWidgets[deviceId] = widgetPtr;
    m_controlStack->addWidget(widgetPtr);

    return widgetPtr;
}

void DeviceManagerDialog::onUpdateTimer()
{
    // [修复] 添加安全检查，防止在析构过程中访问已失效的对象
    if (!m_coreEngine) {
        return; // CoreEngine已经失效，停止更新
    }
    
    auto deviceManager = m_coreEngine->getDeviceManager();
    if (!deviceManager) {
        return; // DeviceManager已经失效，停止更新
    }
    
    // 只在设备列表页面可见时才更新
    if (m_tabWidget && m_tabWidget->currentIndex() == 0) {
        updateDeviceList();
        updateDeviceDetails();
        updateButtonStates();
    }
}

void DeviceManagerDialog::closeEvent(QCloseEvent *event)
{
    // 停止定时器，避免在关闭过程中更新UI
    if (m_updateTimer) {
        m_updateTimer->stop();
    }
    
    // 调用父类的closeEvent
    QDialog::closeEvent(event);
}

} // namespace Presentation
