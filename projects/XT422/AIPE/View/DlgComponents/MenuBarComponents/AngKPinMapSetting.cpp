#include "AngKPinMapSetting.h"
#include "ui_AngKPinMapSetting.h"
#include "../View/../View/GlobalInit/StyleInit.h"

AngKPinMapSetting::AngKPinMapSetting(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::AngKPinMapSetting();
	ui->setupUi(this);

	ui->lineEdit->setText("NC");

	this->setObjectName("AngKPinMapSetting");
	QT_SET_STYLE_SHEET(objectName());
}

AngKPinMapSetting::~AngKPinMapSetting()
{
	delete ui;
}

void AngKPinMapSetting::setPinText(QString strPin)
{
	ui->label->setText(strPin);
}
