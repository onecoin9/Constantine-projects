#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <QJsonObject>
#include <QTimer> // 添加 QTimer 头文件
#include <QVector>
#include <QScrollArea>
#include <QSettings>
#include "presentation/DeviceManagerDialog.h" // 确保路径正确
#include "services/WorkflowManager.h"
QT_BEGIN_NAMESPACE
class QTextEdit;
class QPlainTextEdit;
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

// 包含 TestSite 头文件
#include "ui/widgets/TestSite.h"

namespace Presentation {

class DeviceManagerDialog; // 前向声明
class DutMonitorWidget; // 前向声明

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

private slots:
    // 菜单栏动作
    void onLoadWorkflow();
    void onLoadDeviceConfig();
    void onExit();
    void onAbout();
    
    // 工具栏动作
    void onStartProcess();
    void onStopProcess();
    void onPauseProcess();
    void onResumeProcess();
    void onStartSimulation();
    
    // 手动控制
    void onManualControl();
    void onDeviceControl();
    void onDeviceManager(); // 添加 onDeviceManager 声明
    void onAcAutoSetting();
    
    // 核心引擎信号
    void onEngineStatusChanged(int status);
    void onProcessStarted(const QString &processName);
    void onProcessCompleted(const QString &processName);
    void onProcessFailed(const QString &processName, const QString &error);
    void onDeviceStatusUpdate(const QString &deviceId, const QJsonObject &status);
    void onLogConfig();
    void onTestResultAvailable(const QJsonObject &result);
    
    // 工作流信号
    void onWorkflowStarted(const QString& instanceId);
    void onWorkflowEnded(const QString& instanceId);
    void onStepStarted(const QString &workflowId, int stepIndex, const QString &stepName);
    
    // 定时器
    void updateStatus();
    
    // TestSite 相关槽函数
    void onTestSiteStartClicked();
    void onSiteInfoChanged(int siteNumber, const TestSite::SiteInfo& siteInfo);
    void onAdapterEnableChanged(int siteNumber, quint32 adapterEnable);
    void onDeviceCommandRequested(const QString& deviceId, const QString& command, const QJsonObject& params);
    
private:
    void setupUi();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createDockWidgets();
    void connectSignals();
    
    // TestSite 相关方法
    void setupTestSites();
    void updateTime();
    void createTestSiteControlDock();
    void updateTotalSitesLabel();
    
    void updateDeviceTree();
    void updateWorkflowView();
    void appendLog(const QString &message, int level);
    void createProductionStatusWidget(QDockWidget *parent);
    void updateProductionStatus();
    
private:
    std::shared_ptr<Core::CoreEngine> m_coreEngine;
    
    // UI组件
    DutMonitorWidget *m_dutMonitorWidget; // <-- 现在是核心组件
    QTableWidget *m_activeTasksTable; // 新增：显示当前活动的工作流实例
    QPlainTextEdit *m_logView;
    QDockWidget *m_logDock;
    QLabel *m_statusLabel;
    QLabel *m_deviceStatusLabel; // 新增：用于显示设备连接概览
    
    // 工具栏按钮
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    QPushButton *m_pauseButton;
    QPushButton *m_resumeButton;
    QPushButton *m_simButton;
    
    // 添加缺失的成员变量声明
    // std::unique_ptr<SerialManager> m_serialManager; // 如果 SerialManager 是自定义类
    bool m_isPortOpen;
    bool m_isTestRunning;
    QTimer *m_displayTimer;
    DeviceManagerDialog *m_deviceManagerDialog;
    
    // 新增成员变量
    QString m_currentWorkflowPath; // 用于存储当前加载的工作流文件路径
    bool m_isProcessRunning; // 用于跟踪测试流程是否正在运行
    
    // 生产状态相关
    QProgressBar *m_productionProgress;
    QLabel *m_totalChipsLabel;
    QLabel *m_processedChipsLabel;
    QLabel *m_passedChipsLabel;
    QLabel *m_failedChipsLabel;
    QLabel *m_currentThroughputLabel;
    QLabel *m_sitesStatusLabel;
    
    // TestSite 相关成员变量
    QVector<TestSite*> m_testSites;
    QScrollArea *m_testSiteScrollArea;
    QTimer *m_timeUpdateTimer;
    QSettings m_settings;
    
    // TestSite 控制相关
    QDockWidget *m_testSiteControlDock;
    QSpinBox *m_rowSpinBox;
    QSpinBox *m_colSpinBox;
    QLabel *m_totalSitesLabel;
    QSpinBox *m_blockRowSpinBox;
    QSpinBox *m_blockColSpinBox;
};

} // namespace Presentation

#endif // MAINWINDOW_H
