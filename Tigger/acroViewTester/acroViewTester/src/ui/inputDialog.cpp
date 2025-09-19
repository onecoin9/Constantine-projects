#include "inputDialog.h"

InputDialog::InputDialog(QWidget* parent) : QDialog(parent) {
    setStyleSheet(
        "QDialog {"
        "    background-color: #f0f0f0;"
        "    border-radius: 10px;"
        "}"
        "QLineEdit {"
        "    border: 1px solid #ccc;"
        "    border-radius: 5px;"
        "    padding: 5px;"
        "    background-color: #fff;"
        "}"
        "QPushButton {"
        "    background-color: #4CAF50;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    padding: 10px 20px;"
        "    text-align: center;"
        "    text-decoration: none;"
        "    display: inline-block;"
        "    font-size: 16px;"
        "    margin: 4px 2px;"
        "    transition-duration: 0.4s;"
        "    cursor: pointer;"
        "}"
        "QPushButton:hover {"
        "    background-color: #45a049;"
        "}"
        "QFormLayout {"
        "    margin: 20px;"
        "}"
    );
    QFormLayout* layout = new QFormLayout(this);
    customerCodeEdit = new QLineEdit(this);
    productModelEdit = new QLineEdit(this);
    weeklyEdit = new QLineEdit(this);
    customerBatchEdit = new QLineEdit(this);
    workOrderEdit = new QLineEdit(this);
    workStepEdit = new QLineEdit(this);
    processEdit = new QLineEdit(this);
    deviceNumberEdit = new QLineEdit(this);

    layout->addRow("客户代码:", customerCodeEdit);
    layout->addRow("产品型号:", productModelEdit);
    layout->addRow("周记:", weeklyEdit);
    layout->addRow("客户批次:", customerBatchEdit);
    layout->addRow("工单号:", workOrderEdit);
    layout->addRow("工步:", workStepEdit);
    layout->addRow("工序:", processEdit);
    layout->addRow("设备编号:", deviceNumberEdit);

    QPushButton* saveButton = new QPushButton("保存", this);
    connect(saveButton, &QPushButton::clicked, this, &InputDialog::onSave);
    layout->addWidget(saveButton);
}

void InputDialog::onSave() {
    QSettings settings("AcroView", "AcroViewTester");

    settings.setValue("customerCode", customerCodeEdit->text());
    settings.setValue("productModel", productModelEdit->text());
    settings.setValue("weekly", weeklyEdit->text());
    settings.setValue("customerBatch", customerBatchEdit->text());
    settings.setValue("workOrder", workOrderEdit->text());
    settings.setValue("workStep", workStepEdit->text());
    settings.setValue("process", processEdit->text());
    settings.setValue("deviceNumber", deviceNumberEdit->text());

    accept(); // 关闭对话框
}