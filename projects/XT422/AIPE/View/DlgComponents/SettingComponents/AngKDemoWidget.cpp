#include "AngKDemoWidget.h"
#include "ui_AngKDemoWidget.h"
#include "StyleInit.h"

AngKDemoWidget::AngKDemoWidget(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::AngKDemoWidget();
	ui->setupUi(this);

	ui->okButton->setText(tr("Ok"));
	connect(ui->okButton, &QPushButton::clicked, this, &AngKDemoWidget::sgnEnterDemoMode);

	this->setObjectName("AngKDemoWidget");
	QT_SET_STYLE_SHEET(objectName());
}

AngKDemoWidget::~AngKDemoWidget()
{
	delete ui;
}
