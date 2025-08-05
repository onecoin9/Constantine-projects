#ifndef DEVICEMANAGERDIALOG_H
#define DEVICEMANAGERDIALOG_H

#include <QDialog>
#include <memory>
#include <QMap>

QT_BEGIN_NAMESPACE
class QTableWidget;
class QPushButton;
class QGroupBox;
class QTabWidget;
class QStackedWidget;
class QComboBox;
class QTimer;
QT_END_NAMESPACE

namespace Core {
class CoreEngine;
}

namespace Domain {
class IDevice;
}

namespace Presentation {

class IDeviceControlWidget;

/**
 * @brief 设备管理对话框
 * 负责显示所有设备列表，管理设备连接/断开，并提供设备控制界面的入口
 */
class DeviceManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceManagerDialog(std::shared_ptr<Core::CoreEngine> coreEngine, 
                                QWidget *parent = nullptr);
    ~DeviceManagerDialog() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 设备列表操作
    void onRefreshDevices();
    void onDeviceSelectionChanged();
    void onConnectDevice();
    void onDisconnectDevice();
    void onTestDevice();   
    // 设备控制
    void onOpenDeviceControl();
    void onDeviceControlRequested(const QString &deviceId);
    
    // 定时更新
    void onUpdateTimer();

private:
    void setupUi();
    void connectSignals();
    void createDeviceListTab();
    void createDeviceControlTab();
    void updateDeviceList();
    void updateDeviceDetails();
    void updateButtonStates();
    
    // 设备控制界面管理
    void showDeviceControl(const QString &deviceId);
    void removeDeviceControl(const QString &deviceId);
    IDeviceControlWidget* getOrCreateControlWidget(std::shared_ptr<Domain::IDevice> device);
    
    // 核心引擎
    std::shared_ptr<Core::CoreEngine> m_coreEngine;
    
    // UI - 主界面
    QTabWidget *m_tabWidget;
    
    // UI - 设备列表标签页
    QTableWidget *m_deviceTable;
    QPushButton *m_connectButton;
    QPushButton *m_disconnectButton;
    QPushButton *m_testButton;
    QPushButton *m_refreshButton;
    QPushButton *m_openControlButton;
    QGroupBox *m_deviceDetailsGroup;
    
    // UI - 设备控制标签页
    QComboBox *m_deviceSelector;
    QStackedWidget *m_controlStack;
    
    // 设备控制界面缓存
    QMap<QString, IDeviceControlWidget*> m_controlWidgets;
    
    // 当前选中的设备ID
    QString m_selectedDeviceId;
    
    // 定时器
    QTimer *m_updateTimer;
};

} // namespace Presentation

#endif // DEVICEMANAGERDIALOG_H
