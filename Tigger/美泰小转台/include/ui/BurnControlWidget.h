#ifndef BURNCONTROLWIDGET_H
#define BURNCONTROLWIDGET_H

#include "ui/IDeviceControlWidget.h"
#include "domain/JsonRpcClient.h"
#include <QJsonObject>
#include <QJsonValue>
#include <QAbstractSocket>
#include <memory>
#include <QProcess>
#include <QTimer>
#include <QBitArray>

// 需要在头文件可见 xt_trim_t 定义
#include "xt_trim_param.h"

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
class QTreeWidget;
class QProgressBar;
class QSplitter;
class QScrollArea;
class QTreeWidgetItem;
QT_END_NAMESPACE

namespace Domain {
    class BurnDevice;
}

class Ag06DoCustomProtocol;

namespace Presentation {

    /**
     * @brief 烧录设备控制界面
     * 集成JsonRpcTestWidget的所有功能，保留Aprog.exe管理功能
     */
    class BurnControlWidget : public IDeviceControlWidget
    {
        Q_OBJECT

    public:
        explicit BurnControlWidget(QWidget* parent = nullptr);
        ~BurnControlWidget() override;

        // IDeviceControlWidget interface
        void setDevice(std::shared_ptr<Domain::IDevice> device) override;
        std::shared_ptr<Domain::IDevice> getDevice() const override;
        void updateStatus() override;
        void setControlsEnabled(bool enabled) override;
        QString getDeviceTypeName() const override;

    private slots:
        // Aprog.exe程序管理相关槽函数
        void onBrowseAprogPathClicked();
        void onAprogPathChanged();
        void onStartAprogClicked();
        void onStopAprogClicked();

        // Aprog进程信号槽
        void onAprogStarted();
        void onAprogFinished(int exitCode, QProcess::ExitStatus exitStatus);
        void onAprogError(QProcess::ProcessError error);
        void onAprogOutputReceived(const QString& output);

        // 定时器槽函数
        void onAprogStatusTimerTimeout();

        // 连接管理
        void onConnectClicked();
        void onDisconnectClicked();
        void onConnectionStateChanged(JsonRpcClient::ConnectionState state);
        void onClientError(const QString& error);

        // RPC方法测试
        void onLoadProjectClicked();
        void onSiteScanAndConnectClicked();
        void onDoJobClicked();
        void onDoCustomClicked();

        // 结果处理
        void onNotificationReceived(const QString& method, const QJsonObject& params);
        void onRpcResponse(const QString& method, bool success, const QJsonObject& result, const QString& error);

        // 界面管理
        void onClearLogsClicked();
        void onAutoScrollToggled(bool enabled);
        void updateConnectionStatus();

        // 站点管理
        void onSiteSelectionChanged();
        void onSocketToggled(int socketIndex, bool enabled);
        void onCmdSequenceSelectionChanged();
        void onExecuteJobClicked();
        void parseSiteNotification(const QString& method, const QJsonObject& params);
        void parseLoadProjectResult(const QJsonObject& params);
        void updateSiteInfoDisplay();
        void updateSocketDisplay();
        void updateCmdSequenceDisplay();
        void updateExecuteButtonState();

        // 自定义JSON编辑
        void onExecuteCustomClicked();
        void onFormatJsonClicked();
        void onClearJsonClicked();
        void onCustomJsonTextChanged();

    private:
        // 先前置声明嵌套结构体，确保可在方法声明中使用
        struct RegBitWidgets;

        void setupUi();
        void setupAprogGroup();
        void setupConnectionGroup();
        void setupMethodsGroup();
        void setupResultsGroup();
        void setupSiteJobGroup();
        // 站点管理改由 DutMonitorWidget 承担，移除本地 Site 管理 UI
        void setupCustomJsonGroup();
        void setupAg06CustomUi();
        void setupAg06TrimEditorUi();
        void openAg06TrimDialog();
        void connectSignals();
        void updateAprogStatus();
        void appendLog(const QString& message, const QString& level = "INFO");
        bool isAprogRunningBySharedMemory() const;

        // 工具方法
        void addLogEntry(const QString& type, const QString& message, const QJsonObject& data = QJsonObject());
        void displayJsonData(const QString& title, const QJsonObject& data);
        void enableMethodButtons(bool enabled);
        QString formatBPUEnBinary(quint32 bpuEn);
        QBitArray convertBPUEnToBitArray(quint32 bpuEn);
        void parseProjectInfo(const QJsonObject& result);
        void parseSKTInfo(const QJsonObject& result);
        void extractCommandSequences(const QJsonObject& jsonObject, const QString& parentKey = "");
        void parseAndUpdateUI(const QString& taskContent);
        void ag06SendUid();
        void ag06SendJsonTrim();
        QJsonObject buildTrimJsonFromUi();
        void setupRegBitRow(QGridLayout* grid, int row, const QString& title, RegBitWidgets& w);

        // 设备引用
        std::shared_ptr<Domain::BurnDevice> m_burnDevice;
        JsonRpcClient* m_jsonRpcClient;

        // UI组件 - Aprog程序管理
        QGroupBox* m_aprogGroup;
        QLineEdit* m_aprogPathEdit;
        QPushButton* m_browseAprogBtn;
        QPushButton* m_startAprogBtn;
        QPushButton* m_stopAprogBtn;
        QLabel* m_aprogStatusLabel;

        // UI组件 - 连接管理
        QGroupBox* m_connectionGroup;
        QLineEdit* m_hostEdit;
        QSpinBox* m_portSpin;
        QPushButton* m_connectBtn;
        QPushButton* m_disconnectBtn;
        QLabel* m_statusLabel;
        QProgressBar* m_progressBar;
        QSpinBox* m_reconnectIntervalSpin;

        // UI组件 - 方法测试
        QGroupBox* m_methodsGroup;
        QLineEdit* m_projectPathEdit;
        QLineEdit* m_taskFileEdit;
        QPushButton* m_browseProjectBtn;
        QPushButton* m_browseTaskBtn;
        QPushButton* m_loadProjectBtn;
        QPushButton* m_siteScanBtn;
        QPushButton* m_doJobBtn;
        QPushButton* m_doCustomBtn;

        // UI组件 - 结果显示
        QTabWidget* m_resultsTabWidget;
        QTextEdit* m_logTextEdit;
        QTreeWidget* m_responseTreeWidget;
        QTreeWidget* m_notificationTreeWidget;
        QPushButton* m_clearLogsBtn;
        QCheckBox* m_autoScrollBox;
        QCheckBox* m_prettyJsonBox;

        // 站点管理和作业执行组件
        QGroupBox* m_siteJobGroup;
        QComboBox* m_siteComboBox;
        QLabel* m_siteInfoLabel;
        QWidget* m_socketWidget;
        QGridLayout* m_socketLayout;
        QCheckBox* m_socketCheckBoxes[16];  // 16个座子的复选框
        QComboBox* m_cmdSequenceComboBox;
        QPushButton* m_executeJobBtn;
        QLabel* m_selectedSocketsLabel;

        // 自定义JSON编辑组件
        QGroupBox* m_customJsonGroup;
        QTextEdit* m_customJsonEdit;
        QPushButton* m_formatJsonBtn;
        QPushButton* m_clearJsonBtn;
        QPushButton* m_executeCustomBtn;
        QLabel* m_jsonStatusLabel;

        // AG06 专用子区块
        QGroupBox* m_ag06Group;
        QLineEdit* m_ag06UidEdit;
        QPushButton* m_ag06SendUidBtn;
        QTextEdit* m_ag06TrimJsonEdit;
        QPushButton* m_ag06SendTrimBtn;
        QLabel* m_ag06Status;
        QPushButton* m_openTrimDialogBtn;

        // AG06 Trim 参数编辑子区块
        QGroupBox* m_ag06TrimEditorGroup;
        QCheckBox* m_trimEn_dc6;
        QCheckBox* m_trimEn_dc5;
        QCheckBox* m_trimEn_ac27;
        QCheckBox* m_trimEn_ac4;
        QCheckBox* m_trimEn_eoc;

        // t1_trim_regs 各寄存器字段输入控件
        struct RegBitWidgets {
            QSpinBox* addr;
            QSpinBox* start_bit;
            QSpinBox* width_bit;
            QSpinBox* write_back;
        };
        RegBitWidgets m_reg_output_ctrl;
        RegBitWidgets m_reg_dc_trim;
        RegBitWidgets m_reg_ac_en;
        RegBitWidgets m_reg_ac_trim;
        RegBitWidgets m_reg_eoc;

        // t1_output_ctrl_value
        QSpinBox* m_out_dc6;
        QSpinBox* m_out_dc5;
        QSpinBox* m_out_ac27;
        QSpinBox* m_out_ac4;

        // t1_trim_params（uint16/uint32）
        QSpinBox* m_icc_min;
        QSpinBox* m_icc_max;
        QSpinBox* m_dc_basic_min;
        QSpinBox* m_dc_basic_max;
        QSpinBox* m_dc_p2p_max;
        QSpinBox* m_ac_vol_min;
        QSpinBox* m_ac_vol_max;
        QSpinBox* m_ac_p2p_max;
        QSpinBox* m_dc_trim_code;
        QSpinBox* m_ac_trim_code;
        QSpinBox* m_dc_trim_offset;
        QSpinBox* m_ac_trim_offset;
        QSpinBox* m_dc_trim_step;
        QSpinBox* m_ac_trim_step;
        QSpinBox* m_icc_offset;
        QSpinBox* m_dc_offset;
        QSpinBox* m_ac_offset;

        QPushButton* m_applyTrimBtn;

        // 数据存储
        QJsonObject m_currentSiteInfo;
        QJsonObject m_currentProjectInfo;
        QJsonObject m_currentSKTInfo;
        QMap<QString, QJsonObject> m_cmdSequences;  // key为命令序列名，value为命令序列JSON
        QList<QJsonObject> m_siteList;  // 站点列表

        // 状态
        bool m_isConnected;
        int m_requestCounter;
        QTimer* m_statusTimer;
        QTimer* m_aprogStatusTimer;
    };

} // namespace Presentation

#endif // BURNCONTROLWIDGET_H