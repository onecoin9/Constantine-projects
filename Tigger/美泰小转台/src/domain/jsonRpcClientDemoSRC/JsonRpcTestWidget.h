#ifndef JSONRPCTESTWIDGET_H
#define JSONRPCTESTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QProgressBar>
#include <QSplitter>
#include <QTabWidget>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>
#include <QScrollArea>
#include <QFormLayout>
#include <QCheckBox>
#include <QTreeWidget>
#include <QHeaderView>
#include <QBitArray>
#include "JsonRpcClient.h"
// 需要在头文件可见 xt_trim_t 定义
#include "xt_trim_param.h"
class Ag06DoCustomProtocol;

/**
 * @brief JsonRpcTestWidget - JSON-RPC客户端GUI测试工具
 * 
 * 这是一个图形化的测试界面，用于测试JsonRpcClient的各种功能：
 * - 连接管理
 * - RPC方法测试
 * - 参数输入
 * - 结果显示
 * - 通知接收
 * - 日志记录
 * - 可扩展的接口支持
 */
class JsonRpcTestWidget : public QWidget
{
    Q_OBJECT

public:
    explicit JsonRpcTestWidget(QWidget *parent = nullptr);
    ~JsonRpcTestWidget();

private slots:
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
    // 界面初始化
    void setupUI();
    void setupConnectionGroup();
    void setupMethodsGroup();
    void setupResultsGroup();
    void setupSiteJobGroup();
    void setupCustomJsonGroup();
    void setupAg06CustomUi();
    void setupAg06TrimEditorUi();
    void openAg06TrimDialog();
    void setupMenuBar();
    
    // 工具方法
    void addLogEntry(const QString& type, const QString& message, const QJsonObject& data = QJsonObject());
    void displayJsonData(const QString& title, const QJsonObject& data);
    QString formatJsonString(const QJsonObject& json);
    void enableMethodButtons(bool enabled);
    void addMethodButton(const QString& text, const QString& tooltip, std::function<void()> callback);
    void addJsonToTree(QTreeWidgetItem* parent, const QJsonObject& obj);
    void addJsonArrayToTree(QTreeWidgetItem* parent, const QJsonArray& array);
    
    // RPC调用辅助方法
    void callRpcMethod(const QString& methodName, const QJsonObject& params, const QString& description);
    QJsonObject getParametersFromForm(const QString& method);
    void updateMethodForm(const QString& method);
    
    // 站点数据处理辅助方法
    QBitArray convertBPUEnToBitArray(quint32 bpuEn);
    quint32 convertBitArrayToBPUEn(const QBitArray& bitArray);
    QString formatBPUEnBinary(quint32 bpuEn);
    void extractIPFromBPUResult(const QString& result, QString& ip, quint32& bpuEn);
    
    // 核心组件
    JsonRpcClient* m_client;
    QTimer* m_statusTimer;
    
    // 连接管理组件
    QGroupBox* m_connectionGroup;
    QLineEdit* m_hostEdit;
    QSpinBox* m_portSpinBox;
    QPushButton* m_connectBtn;
    QPushButton* m_disconnectBtn;
    QLabel* m_statusLabel;
    QProgressBar* m_statusProgress;
    QCheckBox* m_autoReconnectBox;
    QSpinBox* m_reconnectIntervalSpinBox;
    
    // 方法测试组件
    QGroupBox* m_methodsGroup;
    QScrollArea* m_methodsScrollArea;
    QWidget* m_methodsWidget;
    QVBoxLayout* m_methodsLayout;
    QComboBox* m_methodComboBox;
    QWidget* m_parameterFormWidget;
    QFormLayout* m_parameterFormLayout;
    
    // 结果显示组件
    QTabWidget* m_resultsTabWidget;
    QTextEdit* m_responseTextEdit;
    QTextEdit* m_notificationTextEdit;
    QTextEdit* m_logTextEdit;
    QTreeWidget* m_responseTreeWidget;
    
    // 控制组件
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
    // AG06 专用子区块
    QGroupBox* m_ag06Group = nullptr;
    QLineEdit* m_ag06UidEdit = nullptr;
    QPushButton* m_ag06SendUidBtn = nullptr;
    QTextEdit* m_ag06TrimJsonEdit = nullptr;
    QPushButton* m_ag06SendTrimBtn = nullptr;
    QLabel* m_ag06Status = nullptr;
    QPushButton* m_openTrimDialogBtn = nullptr;

    // AG06 Trim 参数编辑子区块
    QGroupBox* m_ag06TrimEditorGroup = nullptr;
    // t1_trim_en
    QCheckBox* m_trimEn_dc6 = nullptr;
    QCheckBox* m_trimEn_dc5 = nullptr;
    QCheckBox* m_trimEn_ac27 = nullptr;
    QCheckBox* m_trimEn_ac4 = nullptr;
    QCheckBox* m_trimEn_eoc = nullptr;

    // t1_trim_regs 各寄存器字段输入控件
    struct RegBitWidgets {
        QSpinBox* addr = nullptr;
        QSpinBox* start_bit = nullptr;
        QSpinBox* width_bit = nullptr;
        QSpinBox* write_back = nullptr;
    };
    RegBitWidgets m_reg_output_ctrl;
    RegBitWidgets m_reg_dc_trim;
    RegBitWidgets m_reg_ac_en;
    RegBitWidgets m_reg_ac_trim;
    RegBitWidgets m_reg_eoc;

    // t1_output_ctrl_value
    QSpinBox* m_out_dc6 = nullptr;
    QSpinBox* m_out_dc5 = nullptr;
    QSpinBox* m_out_ac27 = nullptr;
    QSpinBox* m_out_ac4 = nullptr;

    // t1_trim_params（uint16/uint32）
    QSpinBox* m_icc_min = nullptr;   // 使用大范围 QSpinBox（有符号），实际范围在 UI 限制
    QSpinBox* m_icc_max = nullptr;
    QSpinBox* m_dc_basic_min = nullptr;
    QSpinBox* m_dc_basic_max = nullptr;
    QSpinBox* m_dc_p2p_max = nullptr;
    QSpinBox* m_dc_trim_min = nullptr;
    QSpinBox* m_dc_trim_max = nullptr;
    QSpinBox* m_dc_trim_best = nullptr;
    QSpinBox* m_ac_trim_min = nullptr;
    QSpinBox* m_ac_trim_max = nullptr;
    QSpinBox* m_ac_trim_best = nullptr;
    QSpinBox* m_ac_avg_min = nullptr;
    QSpinBox* m_ac_avg_max = nullptr;
    QSpinBox* m_ac_p2p_min = nullptr;
    QSpinBox* m_ac_p2p_max = nullptr;
    QSpinBox* m_ac_freq_min = nullptr;
    QSpinBox* m_ac_freq_max = nullptr;

    // delay_set
    QSpinBox* m_power_on_delay_ms = nullptr;
    QSpinBox* m_t1_dc_stable_ms = nullptr;
    QSpinBox* m_t1_ac_stable_ms = nullptr;
    QSpinBox* m_delay_after_program_ms = nullptr;
    QSpinBox* m_reg_operation_delay_us = nullptr;

    QPushButton* m_trimSaveJsonBtn = nullptr;
    QPushButton* m_trimLoadJsonBtn = nullptr;
    QPushButton* m_trimSendBtn = nullptr;
    QTextEdit* m_customJsonEdit;
    QPushButton* m_executeCustomBtn;
    QPushButton* m_formatJsonBtn;
    QPushButton* m_clearJsonBtn;
    QLabel* m_customStatusLabel;
    
    // 方法按钮映射
    QMap<QString, QPushButton*> m_methodButtons;
    QMap<QString, QWidget*> m_parameterForms;
    
    // 状态管理
    bool m_isConnected;
    int m_requestCounter;

    // 协议适配器
    Ag06DoCustomProtocol* m_ag06Proto = nullptr;

    // AG06 Trim 构建与解析
    QJsonObject buildTrimJsonFromEditor() const;
    void loadTrimJsonToEditor(const QJsonObject& obj);
    xt_trim_t collectTrimFromEditor() const;
    void setupRegBitRow(QGridLayout* grid, int row, const QString& title, RegBitWidgets& w);
    
    // 站点数据结构
    struct SiteInfo {
        QString alias;              // Site02
        QString ip;                 // 192.168.31.30
        QString macAddr;            // MAC地址
        QString firmwareVersion;    // 固件版本
        QJsonObject siteDetails;    // 完整站点信息
    };
    
    struct ProjectInfo {
        QString projectUrl;         // 项目路径
        QJsonArray cmdSequenceArray; // 命令序列数组
        QJsonObject chipData;       // 芯片数据
    };
    
    struct BPUInfo {
        QString ip;                 // 对应IP
        quint32 bpuEn;             // BPU使能值（如3855）
        QBitArray socketStates;     // 16个座子的使能状态
    };
    
    struct SiteData {
        SiteInfo siteInfo;
        ProjectInfo projectInfo;
        BPUInfo bpuInfo;
        bool hasProject = false;    // 是否已加载项目
        bool hasBPU = false;        // 是否有BPU信息
    };
    
    // 站点数据存储
    QMap<QString, SiteData> m_siteDataMap; // 以IP为key
    
    // 预定义的方法配置
    struct MethodConfig {
        QString name;
        QString description;
        QStringList parameters;
        QMap<QString, QString> parameterTypes;
        QMap<QString, QVariant> defaultValues;
    };
    
    QList<MethodConfig> m_methodConfigs;
    void initializeMethodConfigs();
    QWidget* createParameterForm(const MethodConfig& config);
};

#endif // JSONRPCTESTWIDGET_H 
