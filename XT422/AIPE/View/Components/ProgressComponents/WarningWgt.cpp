#include "WarningWgt.h"
#include "ui_WarningWgt.h"
#include "../View/GlobalInit/StyleInit.h"

WarningWgt::WarningWgt(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::WarningWgt();
	ui->setupUi(this);

	ui->bgWidget->setProperty("customProperty", "SUCCESS");
	ui->warnNumWgt->setIsWarn(false);
	ui->warnNumWgt->hide();
	ui->bigTitle->setText(tr("YIELD QUANTITY"));

	//ui->littleTitle->setText(tr("MACHINE COUNT"));
	ui->outputLabel->setText(tr("Output"));
	ui->allLabel->setText(tr("Expect"));

	setOutputDevNums(0);
	setAllDevNums(0);
	setWarnWgtNums(0);

	this->setObjectName("WarningWgt");
	QT_SET_STYLE_SHEET(objectName());
}

WarningWgt::~WarningWgt()
{
	delete ui;
}

void WarningWgt::setOutputDevNums(int nums)
{
	ui->outputNum->setText(QString::number(nums));
}

void WarningWgt::setAllDevNums(int nums)
{
	ui->allNum->setText(QString::number(nums));
}

int WarningWgt::GetOutputDevNums()
{
	return ui->outputNum->text().toInt();
}

int WarningWgt::GetAllDevNums()
{
	return ui->allNum->text().toInt();
}

void WarningWgt::setWarnWgtNums(int nums)
{
	ui->warnNumWgt->setWarnNums(nums);
}

void WarningWgt::setWarnHidden(bool bWarn)
{
	QString strProp;
	if (bWarn) {
		strProp = tr("WARNING");
	}
	else
	{
		strProp = tr("SUCCESS");
	}
	ui->bgWidget->setProperty("customProperty", strProp);
	ui->warnNumWgt->setIsWarn(bWarn);
}

void WarningWgt::AddOutputNum()
{
	int addNum = ui->outputNum->text().toInt() + 1;
	ui->outputNum->setText(QString::number(addNum));
}

float WarningWgt::GetCurrentYield()
{
	int outputNum = ui->outputNum->text().toInt();
	int allNum = ui->allNum->text().toInt();


	return (outputNum * 1.0) / allNum * 100;
}
