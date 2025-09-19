#include "CurrentProgressWgt.h"
#include "ui_CurrentProgressWgt.h"
#include "../View/../View/GlobalInit/StyleInit.h"

CurrentProgressWgt::CurrentProgressWgt(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::CurrentProgressWgt();
	ui->setupUi(this);

	ui->titleLabel->setText(tr("CURRENT  PROGRESS"));

	ui->progressBar->setRange(0, 100);
	
	setCurrentProgress(0);

	this->setObjectName("CurrentProgressWgt");
	QT_SET_STYLE_SHEET(objectName());
}

CurrentProgressWgt::~CurrentProgressWgt()
{
	delete ui;
}

void CurrentProgressWgt::setCurrentProgress(int value)
{
	ui->progressBar->setValue(value);
	ui->proLabel->setText(QString::number(value));
}
