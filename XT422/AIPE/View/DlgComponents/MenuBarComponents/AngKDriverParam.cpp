#include "AngKDriverParam.h"
#include "ui_AngKDriverParam.h"
#include "StyleInit.h"

AngKDriverParam::AngKDriverParam(QWidget *parent)
	: AngKDialog(parent)
{
	this->setObjectName("AngKDriverParam");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::AngKDriverParam();
	ui->setupUi(setCentralWidget());

	this->SetTitle(tr("Driver Param Configuration"));

	initButton();
}

AngKDriverParam::~AngKDriverParam()
{
	delete ui;
}

void AngKDriverParam::initButton()
{
	ui->okButton->setText(tr("Ok"));
	ui->defaultButton->setText(tr("Default"));
	ui->exitButton->setText(tr("Exit"));

	connect(this, &AngKDriverParam::sgnClose, this, &AngKDriverParam::close);
}
