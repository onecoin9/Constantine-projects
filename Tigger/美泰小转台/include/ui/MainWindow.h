#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <QJsonObject>
#include <QTimer> // 添加 QTimer 头文件
#include <QVector>
#include <QScrollArea>
#include <QSettings>
#include <QTabWidget>
#include <QFrame>
#include <QSpinBox>
#include <QSizePolicy>
#include <QIcon>
#include <QPixmap>
#include <QFormLayout> // <--- 新增
#include <QLineEdit> // <--- 新增
#include <QAction> // <--- 新增
#include "ui/DeviceManagerDialog.h" // 确保路径正确
#include "ui/DatabaseWidget.h" // 添加数据库界面
#include "ui/ChipQueryWidget.h"
#include "services/WorkflowManager.h"
#include "domain/IDevice.h"
QT_BEGIN_NAMESPACE
class QTextEdit;
class QTreeWidget;
class QTableWidget;
class QProgressBar;
class QLabel;
class QPushButton;
class QDockWidget;
class QGridLayout;
class QSpinBox;
QT_END_NAMESPACE

namespace Core {
    class CoreEngine;
}

namespace Domain {
    class BurnDevice;
    class IDevice;
}

namespace Presentation {

class DeviceManagerDialog; // 前向声明
class DutMonitorWidget; // 前向声明
class SimpleLogWidget; // 前向声明
class DatabaseWidget; // 前向声明

/**
 * @brief 主窗口类
 * 提供用户界面交互
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setCoreEngine(std::shared_ptr<Core::CoreEngine> engine);
    void onCoreEngineInitializationFailed(const QString& error); // 新增

private slots:
    // 菜单栏动作
    void onConfigRecipe();
    void onSelectRecipe();
    void onExit();
    void onAbout();
    // 工具栏动作
    void onStartProcess();
    void onStopProcess();
    void onEditProcess();
    void onResumeProcess();    
    // 手动控制
    void onManualControl();
    void onDeviceControl();
    void onDeviceManager(); // 添加 onDeviceManager 声明
    
    // 核心引擎信号
    void onEngineStatusChanged(int status);
    void onProcessStarted(const QString &processName);
    void onProcessCompleted(const QString &processName);
    void onProcessFailed(const QString &processName, const QString &error);
    void onTestResultAvailable(const QJsonObject &result);
    
    // 工作流信号
    void onWorkflowStarted(const QString& instanceId);
    void onWorkflowEnded(const QString& instanceId);
    void updateUiForProcessState(); // 新增槽函数声明
    void onStepStarted(const QString &workflowId, int stepIndex, const QString &stepName); 
    
    // BurnDevice连接状态处理
    void onBurnDeviceConnectionChanged(Domain::IDevice::DeviceStatus status);

    // 定时器
    void updateStatus();
private:
    void setupUi();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createDockWidgets();
    void connectSignals();
    void updateWorkflowView();
    // appendLog 已移除，使用 LogDisplayWidget
    void createProductionStatusWidget(QDockWidget *parent);
    void updateProductionStatus();
    
    // BurnDevice连接后启动工作流
    void startWorkflowAfterConnection(const QString& workflowPath);
    
private:
    std::shared_ptr<Core::CoreEngine> m_coreEngine;
    
    // UI组件
    DutMonitorWidget *m_dutMonitorWidget; // <-- 现在是核心组件
    ChipQueryWidget* m_chipQueryWidget;
    QDockWidget *m_dutMonitorDock; // DUT监控面板的dock容器
    QTableWidget *m_activeTasksTable; // 新增：显示当前活动的工作流实例
    Presentation::SimpleLogWidget *m_simpleLogWidget; // 系统日志控件
    Presentation::DatabaseWidget *m_databaseWidget; // 数据库管理控件
    QTabWidget *m_tabWidget; // 中央tab控件
    QLabel *m_statusLabel;
    QLabel *m_deviceStatusLabel; // 新增：用于显示设备连接概览
    
    // 工具栏按钮
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    QPushButton *m_editButton;
    QPushButton *m_resumeButton;
    
    // 菜单项
    QAction* m_manualControlAction;
    QAction* m_deviceControlAction;

    bool m_isTestRunning;
    QTimer *m_displayTimer;
    DeviceManagerDialog *m_deviceManagerDialog;
    
    // 新增成员变量
    QString m_currentWorkflowPath; // 用于存储当前加载的工作流文件路径
    QString m_currentRouterPath; // 用于存储当前加载的Router文件路径
    bool m_isProcessRunning; // 用于跟踪测试流程是否正在运行
    
    // BurnDevice连接等待相关
    std::shared_ptr<Domain::BurnDevice> m_currentBurnDevice; // 当前使用的BurnDevice
    bool m_waitingForBurnConnection; // 是否正在等待BurnDevice连接
    QString m_pendingWorkflowPath; // 等待连接完成后要启动的工作流路径
    
    // 生产状态相关
    QProgressBar *m_productionProgress;
    QLabel *m_totalChipsLabel;
    QLabel *m_processedChipsLabel;
    QLabel *m_passedChipsLabel;
    QLabel *m_failedChipsLabel;
    QLabel *m_currentThroughputLabel;
    QLabel *m_sitesStatusLabel;
    
    // 设置相关
    QTimer *m_timeUpdateTimer;
    QSettings m_settings;
};

} // namespace Presentation

#endif // MAINWINDOW_H
