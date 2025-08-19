// MeasurementDialog.h

#ifndef MEASUREMENTDIALOG_H
#define MEASUREMENTDIALOG_H

#include <QDialog>

struct CheckBoxStatus {
    bool pinContinuityTest;
    bool powerOnCurrentTest;
    bool frequencyScreening;
    bool scaleCalibration;
    bool functionalSafetyCalibration;
    bool writeOTP;
    bool readPhaseInfo;
    bool readZeroNoise;
    bool readManchesterCodeCurrent;
    bool measureBitTime;
    bool measureTDelay;
    bool syncPulseVDD;
    bool asynchronousMode;
    bool dualTimeSlotMode;
};

namespace Ui {
    class MeasurementDialog;
}

class MeasurementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MeasurementDialog(QWidget* parent = nullptr);
    ~MeasurementDialog();
    void initCheckbox();
    // 处理函数接口
    void handlePinContinuityTest();
    void handlePowerOnCurrentTest();
    void handleFrequencyScreening();
    void handleScaleCalibration();
    void handleFunctionalSafetyCalibration();
    void handleWriteOTP();
    void handlePSI5Test();
    CheckBoxStatus collectCheckBoxStatus();

private slots:
    void updateCheckBoxStatus();
private:
    Ui::MeasurementDialog* ui;
    CheckBoxStatus status;
};

#endif // MEASUREMENTDIALOG_H