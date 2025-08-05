#ifndef TESTBOARDCONTROLWIDGET_H
#define TESTBOARDCONTROLWIDGET_H

#include "domain/TestBoardDevice.h"
#include "ui/IDeviceControlWidget.h"
#include "domain/protocols/IXTProtocol.h"
#include <QTimer>
#include <QCheckBox>
#include <QLabel>
#include <QTableWidgetItem>
#include <QMutex>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class TestBoardControlWidget; }
QT_END_NAMESPACE

namespace Domain {
    class TestBoardDevice;
    class IDevice;
}

namespace Presentation {
    class LogDisplayWidget;
}

namespace Presentation {

class TestBoardControlWidget : public IDeviceControlWidget
{
    Q_OBJECT

public:
    TestBoardControlWidget(QWidget *parent = nullptr);
    ~TestBoardControlWidget() override;

    // IDeviceControlWidget 接口
    QString getDeviceTypeName() const override;
    void setDevice(std::shared_ptr<Domain::IDevice> device) override;
    std::shared_ptr<Domain::IDevice> getDevice() const override;

signals:
    void parseProgressUpdated(int progress);

private slots:
    void onStartAcquisition();
    void onStopAcquisition();
    void onDutPowerOn();
    void onDutPowerOff();
    void onQueryVoltage();
    void onQueryFault();
    void onDutSelectionChanged();
    void onUpdateTimer();
    void onDataUpdateTimeout();
    
    void onTestStateChanged(bool isRunning);
    void onPowerStateChanged(const Domain::Protocols::PowerFeedbackData &data);
    void onVoltageDataReceived(const QJsonObject &voltageData);
    void onFaultStatusReceived(uint32_t faultState);
    void onDataError(const QString &error);
    
    void onFrameSent(const QByteArray &frame);
    void onFrameReceived(const QByteArray &frame);
    
    void onSendCalibration();
    void onWriteRegister();
    void onReadRegister();
    void onCalibrationResult(const QJsonObject &result);
    void onRegisterWriteResult(const QJsonObject &result);
    void onRegisterReadResult(const QJsonObject &result);
    void onSetChipType();
    void onChipTypeSetResult(const QJsonObject &result);
    void onParseBinFile();

private:
    void setupUiLogic();
    void connectSignals();
    void disconnectSignals();
    void updateStatus();
    void setControlsEnabled(bool enabled);
    uint16_t getSelectedDuts() const;
    void executeDeviceCommand(const QString& command, const QJsonObject& params);
    void updateDataTable(const Domain::Protocols::TestFeedbackData &data);
    
    Ui::TestBoardControlWidget *ui;
    std::shared_ptr<Domain::TestBoardDevice> m_testBoard;
    
    // 定时器
    QTimer* m_updateTimer;
    QTimer* m_dataUpdateTimer;
    
    // UI状态
    bool m_isAcquiring;
    uint8_t m_currentChipIndex;
    bool m_hasPendingData;
    int m_frameCounter;
    
    // UI组件数组
    QCheckBox* m_dutCheckBoxes[8];
    QLabel* m_powerStatusLabels[9]; // 8个DUT + 1个主电源
    QTableWidgetItem* m_tableItems[9][10]; // 9行(8个DUT + 1个外部) x 10列
    
    // 设备名称用于日志标识
    QString m_deviceName;
};

} // namespace Presentation

#endif // TESTBOARDCONTROLWIDGET_H 
