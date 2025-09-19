#pragma once
#include <QtWidgets>
class PeripheralControl : public QWidget {
    Q_OBJECT

public:
    explicit PeripheralControl(QWidget* parent = nullptr);

private slots:
    void onFanButtonClicked();
    void onPowerButtonClicked();
    void onLoadInstrumentChannelChanged(int index);

private:
    // 风扇控制
    QList<QPushButton*> fanButtons;
    QLabel* fanLabel;

    // 电源控制
    QLabel* powerLabel;
    QLineEdit* powerInput;
    QList<QPushButton*> powerButtons;

    // 负载仪控制
    QComboBox* loadInstrumentComboBox;
    QLabel* loadInstrumentLabel;
};