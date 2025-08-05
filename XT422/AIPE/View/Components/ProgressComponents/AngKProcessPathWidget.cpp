#include "AngKProcessPathWidget.h"
#include "ui_AngKProcessPathWidget.h"
#include "../View/GlobalInit/StyleInit.h"
#include <QGraphicsDropShadowEffect>

AngKProcessPathWidget::AngKProcessPathWidget(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::AngKProcessPathWidget();
	ui->setupUi(this);

	InitShadow();

	this->setObjectName("AngKProcessPathWidget");
	QT_SET_STYLE_SHEET(objectName());
}

AngKProcessPathWidget::~AngKProcessPathWidget()
{
	delete ui;
}

void AngKProcessPathWidget::setPath(QString strPath)
{
	//TODO 显示操作顺序逻辑
	ui->pathLabel->setText(strPath);
}

void AngKProcessPathWidget::InitShadow()
{
	QGraphicsDropShadowEffect* wgtShadow = new QGraphicsDropShadowEffect(this);
	wgtShadow->setOffset(0, 0);
	wgtShadow->setColor(QColor("#dfdfdf"));
	wgtShadow->setBlurRadius(10);

	//给顶层QWidget设置阴影
	this->setGraphicsEffect(wgtShadow);
}
