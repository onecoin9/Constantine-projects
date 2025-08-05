#include "WarningNum.h"
#include "ui_WarningNum.h"
#include "../View/../View/GlobalInit/StyleInit.h"
#include "../Controller/Utils/AngKPathResolve.h"

WarningNum::WarningNum(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::WarningNum();
	ui->setupUi(this);

	QPixmap pix(Utils::AngKPathResolve::localRelativeSkinPath() + "/Common/Warning.png");

	ui->warnLabel->setPixmap(pix);
	ui->warnLabel->setScaledContents(true);

	this->setObjectName("WarningNum");
	QT_SET_STYLE_SHEET(objectName());
}

WarningNum::~WarningNum()
{
	delete ui;
}

void WarningNum::setWarnNums(int nums)
{
	QString strNums = "";
	if (nums > 999)
		strNums = "999+";
	else if(nums == 0)
		strNums = "";
	else
		strNums = QString::number(nums);

	ui->numLabel->setText(strNums);
}

void WarningNum::setIsWarn(bool bWarn)
{
	if (bWarn) {
		ui->warnLabel->show();
	}
	else {
		ui->warnLabel->hide();
	}
}
