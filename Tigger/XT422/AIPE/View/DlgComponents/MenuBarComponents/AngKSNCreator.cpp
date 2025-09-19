#include "AngKSNCreator.h"
#include "ui_AngKSNCreator.h"
#include "../View/GlobalInit/StyleInit.h"

AngKSNCreator::AngKSNCreator(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::AngKSNCreator();
	ui->setupUi(this);

	InitText();

	this->setObjectName("AngKSNCreatorDlg");
	QT_SET_STYLE_SHEET(objectName());
}

AngKSNCreator::~AngKSNCreator()
{
	delete ui;
}

void AngKSNCreator::InitText()
{
	ui->SNSettingGroup->setTitle(tr("SN Setting"));

	ui->startIdxText->setText(tr("Start Index"));
	ui->chipNameText->setText(tr("Chip Name"));
	ui->totalNumberText->setText(tr("Total Number"));
	ui->ProgIdxText->setText(tr("Index to Program"));
	ui->SNGeneratorText->setText(tr("SN Generator"));


	ui->SERButton->setText(tr("Get SER"));
	ui->queryButton->setText(tr("Query"));
	ui->saveButton->setText(tr("Save"));
	ui->loadButton->setText(tr("Load"));

	ui->SNFileGroup->setTitle(tr("SN File"));
	ui->SNCFilePathText->setText(tr("Select SNC File"));
}

void AngKSNCreator::SetHideSNCFile(bool isHide)
{
	if (isHide)
	{
		ui->SNFileGroup->show();
	}
	else
	{
		ui->SNFileGroup->hide();
	}
}
