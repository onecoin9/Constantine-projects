#include "YieldRateWgt.h"
#include "ui_YieldRateWgt.h"
#include "../View/GlobalInit/StyleInit.h"
#include "../Controller/Utils/AngKPathResolve.h"
#include <QPixmap>

YieldRateWgt::YieldRateWgt(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::YieldRateWgt();
	ui->setupUi(this);

	ui->titleLabel->setText(tr("YIELD  RATE"));
	//ui->rateLabel->setText(tr("Target Yield Rate"));

	setRoundProgressValue(0);

	setPreLabel(0, true);
	//ui->symbolWgt->hide();
	this->setObjectName("YieldRateWgt");
	QT_SET_STYLE_SHEET(objectName());
}

YieldRateWgt::~YieldRateWgt()
{
	delete ui;
}

void YieldRateWgt::setPreLabel(float strPre, bool bAdvance)
{
	//if (bAdvance)
	//	ui->symbolPerLabel->setPixmap(QPixmap(Utils::AngKPathResolve::localRelativeSkinPath() + "/Common/topArrow.svg"));
	//else
	//	ui->symbolPerLabel->setPixmap(QPixmap(Utils::AngKPathResolve::localRelativeSkinPath() + "/Common/downArrow.svg"));

	//ui->perLabel->setText(QString::number(strPre) + "%");
}

void YieldRateWgt::setRoundProgressValue(int value)
{
	ui->roundProgress->setValue(value);
}
