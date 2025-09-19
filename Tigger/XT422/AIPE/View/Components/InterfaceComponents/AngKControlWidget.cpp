#include "AngKControlWidget.h"
#include "ui_AngKControlWidget.h"
#include "../View/GlobalInit/StyleInit.h"
#include <QGraphicsDropShadowEffect>

AngKControlWidget::AngKControlWidget(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::AngKControlWidget();
	ui->setupUi(this);

	ui->allCheckBox->setText(tr("Select All"));

	ui->startButton->setText(tr("Start"));
	ui->exitButton->setText(tr("Exit"));

	InitShadow();
	InitButton();

	this->setObjectName("AngKControlWidget");
	QT_SET_STYLE_SHEET(objectName());
}

AngKControlWidget::~AngKControlWidget()
{
	delete ui;
}

void AngKControlWidget::InitShadow()
{
	QGraphicsDropShadowEffect* exitShadow = new QGraphicsDropShadowEffect(this);
	exitShadow->setOffset(0, 0);
	exitShadow->setColor(QColor("#dfdfdf"));
	exitShadow->setBlurRadius(10);

	//给顶层QWidget设置阴影
	ui->exitButton->setGraphicsEffect(exitShadow);
}

void AngKControlWidget::InitButton()
{
	connect(ui->allCheckBox, &QCheckBox::stateChanged, this, &AngKControlWidget::sgnAllCheck);
	connect(ui->startButton, &QPushButton::clicked, this, &AngKControlWidget::sgnStartBurnData);
}

void AngKControlWidget::SetAllSelect(bool bSetCheck)
{
	ui->allCheckBox->blockSignals(true);
	ui->allCheckBox->setChecked(bSetCheck);
	ui->allCheckBox->blockSignals(false);
}