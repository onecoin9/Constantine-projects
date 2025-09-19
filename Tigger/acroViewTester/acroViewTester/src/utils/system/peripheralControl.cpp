#include"peripheralControl.h"

PeripheralControl::PeripheralControl(QWidget* parent) : QWidget(parent) {
    // Fan Control
    fanLabel = new QLabel("Fan Control", this);
    for (int i = 0; i < 5; ++i) {
        QPushButton* button = new QPushButton(QString::number(i), this);
        fanButtons.append(button);
        connect(button, &QPushButton::clicked, this, &PeripheralControl::onFanButtonClicked);
    }

    // Power Control
    powerLabel = new QLabel("Power Index", this);
    powerInput = new QLineEdit(this);
    QStringList powerButtonNames = { "Fan On", "Fan Off", "Write Fan", "Power On", "Power Off", "Write Power" };
    for (const QString& name : powerButtonNames) {
        QPushButton* button = new QPushButton(name, this);
        powerButtons.append(button);
        connect(button, &QPushButton::clicked, this, &PeripheralControl::onPowerButtonClicked);
    }

    // Load Instrument Control
    loadInstrumentComboBox = new QComboBox(this);
    loadInstrumentComboBox->addItems({ "Channel 1", "Channel 2", "Channel 3" });
    connect(loadInstrumentComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &PeripheralControl::onLoadInstrumentChannelChanged);
    loadInstrumentLabel = new QLabel("SITE 1", this);
}

void PeripheralControl::onFanButtonClicked() {
    // Handle fan button click event
}

void PeripheralControl::onPowerButtonClicked() {
    // Handle power button click event
}

void PeripheralControl::onLoadInstrumentChannelChanged(int index) {
    // Handle load instrument channel change event
}