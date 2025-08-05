#include "AngKGotoSetting.h"
#include "ui_AngKGotoSetting.h"
#include "StyleInit.h"
#include <QPushButton>
AngKGotoSetting::AngKGotoSetting(QWidget *parent)
	: AngKDialog(parent)
{
	this->setObjectName("AngKGotoSetting");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::AngKGotoSetting();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(375, 150);
	this->SetTitle(tr("Goto Setting"));

	connect(ui->okButton, &QPushButton::clicked, this, [=]() {
		QString text = ui->addressEdit->text();
		emit sgnGoToAddress(text);
		close();
		});

	connect(this, &AngKGotoSetting::sgnClose, this, &AngKGotoSetting::close);
}

AngKGotoSetting::~AngKGotoSetting()
{
	delete ui;
}
