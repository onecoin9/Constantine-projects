#ifndef TESTBOARDCONTROLWIDGET_H
#define TESTBOARDCONTROLWIDGET_H

#include "IDeviceControlWidget.h"
#include "domain/protocols/IXTProtocol.h"
#include <QTimer>
#include <QMutex>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
class QTableWidget;
class QLabel;
class QCheckBox;
class QTextEdit;
class QComboBox;
class QTableWidgetItem;
QT_END_NAMESPACE

namespace Domain {
class TestBoardDevice;
}

namespace Presentation {

/**
 * @brief 测试板设备控制界面
 * 提供数据采集控制、DUT电源管理、数据显示等功能
 */
class TestBoardControlWidget : public IDeviceControlWidget
{
    Q_OBJECT
public:
    explicit TestBoardControlWidget(QWidget *parent = nullptr);
    ~TestBoardControlWidget() override;
    
    // IDeviceControlWidget interface
    void setDevice(std::shared_ptr<Domain::IDevice> device) override;
    std::shared_ptr<Domain::IDevice> getDevice() const override;
    void updateStatus() override;
    QString getDeviceTypeName() const override { return "测试板控制"; }
    void setControlsEnabled(bool enabled) override;

private slots:
    void onStartAcquisition();
    void onStopAcquisition();
    void onDutPowerOn();
    void onDutPowerOff();
    void onQueryVoltage();
    void onQueryFault();
    void onDutSelectionChanged();
    void onUpdateTimer();
    
    // 设备信号处理
    void onTestDataReceived(const Domain::Protocols::TestFeedbackData &data);
    void onTestStateChanged(bool isRunning);
    void onPowerStateChanged(bool powerOn, uint16_t dutPowerState);
    void onVoltageDataReceived(const QJsonObject &voltageData);
    void onFaultStatusReceived(uint32_t faultState);
    void onDataError(const QString &error);

    // 新增：用于处理原始数据帧的槽函数
    void onFrameSent(const QByteArray &frame);
    void onFrameReceived(const QByteArray &frame);

    // 新增：高级指令相关槽函数
    void onSendCalibration();
    void onWriteRegister();
    void onReadRegister();
    void onSetChipType();
    void onCalibrationResult(const QJsonObject &result);
    void onRegisterWriteResult(const QJsonObject &result);
    void onRegisterReadResult(const QJsonObject &result);
    void onChipTypeSetResult(const QJsonObject &result);

    // 性能优化相关槽函数
    void onDataUpdateTimeout();     // 批量更新数据表格
    void onLogUpdateTimeout();      // 批量更新日志

private:
    void setupUi();
    void connectSignals();
    void disconnectSignals();
    uint16_t getSelectedDuts() const;
    void updateDataTable(const Domain::Protocols::TestFeedbackData &data);
    void executeDeviceCommand(const QString& command, const QJsonObject& params);
    
    // 设备
    std::shared_ptr<Domain::TestBoardDevice> m_testBoard;
    
    // UI控件 - 状态显示
    QLineEdit *m_statusEdit;
    QLineEdit *m_serialNumberEdit;
    QLabel *m_errorLabel;
    
    // UI控件 - 控制按钮
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    QPushButton *m_powerOnButton;
    QPushButton *m_powerOffButton;
    QPushButton *m_queryVoltageButton;
    QPushButton *m_queryFaultButton;
    
    // UI控件 - 高级指令
    QComboBox *m_chipTypeComboBox;
    QPushButton *m_setChipTypeButton;
    QLineEdit *m_calibCmdEdit;
    QPushButton *m_calibSendButton;
    QLineEdit *m_regWriteAddrEdit;
    QLineEdit *m_regWriteDataEdit;
    QPushButton *m_regWriteSendButton;
    QLineEdit *m_regReadAddrEdit;
    QLineEdit *m_regReadLengthEdit;
    QPushButton *m_regReadSendButton;

    // UI控件 - DUT选择
    QCheckBox *m_dutCheckBoxes[8];
    QCheckBox *m_selectAllCheckBox;
    
    // UI控件 - 数据显示
    QTableWidget *m_dataTable;
    QTextEdit *m_logTextEdit;
    QCheckBox *m_enableRawFrameCheckBox;  // 控制原始帧日志显示
    
    // 定时器
    QTimer *m_updateTimer;
    QTimer *m_dataUpdateTimer;  // 数据更新定时器
    QTimer *m_logUpdateTimer;   // 日志更新定时器
    
    // 状态
    bool m_isAcquiring;
    uint8_t m_currentChipIndex;
    
    // 性能优化相关
    Domain::Protocols::TestFeedbackData m_pendingData;  // 待更新的数据
    bool m_hasPendingData;                              // 是否有待更新的数据
    QStringList m_logBuffer;                            // 日志缓冲区
    QMutex m_logMutex;                                  // 日志缓冲区互斥锁
    int m_frameCounter;                                 // 帧计数器
    bool m_enableRawFrameLogging;                       // 是否启用原始帧日志
    
    // 表格项缓存（避免频繁创建/销毁）
    QTableWidgetItem* m_tableItems[9][10];              // 9行10列的表格项缓存
};

} // namespace Presentation

#endif // TESTBOARDCONTROLWIDGET_H 