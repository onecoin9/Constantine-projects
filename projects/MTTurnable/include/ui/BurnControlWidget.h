#ifndef BURNCONTROLWIDGET_H
#define BURNCONTROLWIDGET_H

#include "ui/IDeviceControlWidget.h"
#include <QJsonObject>
#include <QJsonValue>
#include <QAbstractSocket>
#include <memory>
#include <QProcess>
QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
class QComboBox;
class QSpinBox;
class QTextEdit;
class QGroupBox;
class QLabel;
class QFormLayout;
class QVBoxLayout;
class QHBoxLayout;
class QTabWidget;
class QGridLayout;
class QTableWidget;
class QCheckBox;
class QPlainTextEdit;
QT_END_NAMESPACE

namespace Domain {
    class BurnDevice;
}

namespace Presentation {

/**
 * @brief 烧录设备控制界面
 * 提供jsonrpcclient mainwindow的所有功能
 */
class BurnControlWidget : public IDeviceControlWidget
{
    Q_OBJECT
    
public:
    explicit BurnControlWidget(QWidget *parent = nullptr);
    ~BurnControlWidget() override;
    
    // IDeviceControlWidget interface
    void setDevice(std::shared_ptr<Domain::IDevice> device) override;
    std::shared_ptr<Domain::IDevice> getDevice() const override;
    void updateStatus() override;
    void setControlsEnabled(bool enabled) override;
    QString getDeviceTypeName() const override;

private slots:
    // 主要功能按钮槽函数
    void onSiteScanAndConnectClicked();
    void onLoadProjectClicked();
    void onDoJobClicked();
    void onGetProjectInfoClicked();
    void onDoCustomClicked();
    void onGetProjectInfoExtClicked();
    void onGetSKTInfoClicked();
    void onSendUUIDClicked();
    
    // 设备信号槽
    void onOperationCompleted(const QString &operation, const QJsonObject &result);
    void onOperationFailed(const QString &operation, const QString &error);
    
    // 服务器配置相关
    void onServerConfigChanged();
    
    // 浏览文件
    void onBrowseProjectFileClicked();

    // 新增槽函数
    void onConnectButtonClicked();
    void onDeviceConnected();
    void onDeviceDisconnected();
    void clearLog();
    
    // Aprog路径管理相关槽函数
    void onBrowseAprogPathClicked();
    void onAprogPathChanged();
    void onStartAprogClicked();
    void onStopAprogClicked();
    
    // Aprog进程信号槽
    void onAprogStarted();
    void onAprogFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onAprogError(QProcess::ProcessError error);
    void onAprogOutputReceived(const QString &output);

private:
    void setupUi();
    void createConnectionGroup();
    void createAprogGroup();
    void createBasicOperationsGroup();
    void createAdvancedOperationsGroup();
    void createLoggingGroup();
    void connectSignals();
    void updateConnectionStatus();
    void updateDeviceStatus();
    void updateAprogStatus();
    void appendLog(const QString &message, const QString &level = "INFO");
    void initializeDefaultValues();
    
    // 辅助函数
    QString formatJsonForDisplay(const QJsonValue &value);
    QJsonObject buildDoCustomParams();
    
    // 设备引用
    std::shared_ptr<Domain::BurnDevice> m_burnDevice;
    
    // UI组件 - 连接管理组
    QGroupBox *m_connectionGroup;
    QLineEdit *m_serverHostEdit;
    QSpinBox *m_serverPortSpin;
    QLabel *m_connectionStatusLabel;
    
    // UI组件 - Aprog程序管理组
    QGroupBox *m_aprogGroup;
    QLineEdit *m_aprogPathEdit;
    QPushButton *m_browseAprogBtn;
    QPushButton *m_startAprogBtn;
    QPushButton *m_stopAprogBtn;
    QLabel *m_aprogStatusLabel;
    
    // UI组件 - 基本操作组
    QGroupBox *m_basicOpsGroup;
    QPushButton *m_siteScanConnectBtn;
    QPushButton *m_getProjectInfoBtn;
    QPushButton *m_getSKTInfoBtn;
    QPushButton *m_doJobBtn;
    
    // UI组件 - 项目管理组
    QGroupBox *m_projectGroup;
    QLineEdit *m_projectPathEdit;
    QPushButton *m_browseProjectBtn;
    QPushButton *m_loadProjectBtn;
    QLineEdit *m_projectInfoExtEdit;
    QPushButton *m_getProjectInfoExtBtn;
    
    // UI组件 - 高级操作组
    QGroupBox *m_advancedOpsGroup;
    QLineEdit *m_strIPEdit;
    QSpinBox *m_nHopNumSpin;
    QSpinBox *m_portIDSpin;
    QSpinBox *m_cmdFlagSpin;
    QSpinBox *m_cmdIDSpin;
    QLineEdit *m_sktEnEdit;
    QSpinBox *m_bpuIDSpin;
    QTextEdit *m_docmdSeqJsonEdit;
    QPushButton *m_doCustomBtn;
    
    // UI组件 - UUID操作组
    QGroupBox *m_uuidGroup;
    QLineEdit *m_uuidEdit;
    QLineEdit *m_uuidStrIPEdit;
    QSpinBox *m_uuidNHopNumSpin;
    QLineEdit *m_uuidSktEnableEdit;
    QPushButton *m_sendUUIDBtn;
    
    // UI组件 - 日志组
    QGroupBox *m_loggingGroup;
    QTextEdit *m_logTextEdit;
    QPushButton *m_clearLogBtn;
    
    // 状态跟踪
    bool m_isDeviceConnected;
    QStringList m_logBuffer;
    static const int MAX_LOG_LINES = 1000;
};

} // namespace Presentation

#endif // BURNCONTROLWIDGET_H 
