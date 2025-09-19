#include "AngKAdapterInformation.h"
#include "ui_AngKAdapterInformation.h"
#include "StyleInit.h"
#include "AngKPinMapSetting.h"

AngKAdapterInformation::AngKAdapterInformation(QWidget *parent)
	: AngKDialog(parent)
{
	this->setObjectName("AngKAdapterInformation");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::AngKAdapterInformation();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(900, 495);
	this->SetTitle(tr("Adapter Information"));

	InitText();
	InitPinMap();

	connect(this, &AngKAdapterInformation::sgnClose, this, &AngKAdapterInformation::close);
	connect(ui->exitButton, &QPushButton::clicked, this, &AngKAdapterInformation::close);
}

AngKAdapterInformation::~AngKAdapterInformation()
{
	delete ui;
}

void AngKAdapterInformation::InitText()
{
	ui->adapterGroup->setTitle(tr("Adapter"));
	ui->adapterIDText->setText(tr("Adapter ID:"));
	ui->serialStringText->setText(tr("Serial String:"));
	ui->numSiteText->setText(tr("Number of Site:"));
	ui->adapterVersionText->setText(tr("Adapter Version:"));
	ui->dateText->setText(tr("Date:"));
	ui->limitCountText->setText(tr("Limit Count:"));
	ui->insertCountText->setText(tr("Insertion Count:"));

	ui->otherInfoGroup->setTitle(tr("Other Information"));
	ui->licenseFlagText->setText(tr("License Flag:"));
	ui->rfailedTotalText->setText(tr("Failed Total:"));

	ui->pinMapGroup->setTitle(tr("Pin Map Setting"));

	ui->bottomBoardCheck->setText(tr("Bottom Board"));
	ui->uploadButton->setText(tr("Upload"));
	ui->saveFileButton->setText(tr("Save To File"));
	ui->loadFileButton->setText(tr("Load From File"));
	ui->exitButton->setText(tr("Ok"));
}

void AngKAdapterInformation::InitPinMap()
{
	int nCount = 0;
	for (int i = 0; i < 13; ++i)
	{
		for (int j = 0; j < 5; ++j)
		{
			QString name;
			if (nCount < 10)
			{
				name = "PD0" + QString::number(nCount) + ":";
			}
			else
			{
				name = "PD" + QString::number(nCount) + ":";
			}

			nCount++;
			if(nCount == 65)
				break;


			AngKPinMapSetting* m_pinMap = new AngKPinMapSetting();
			m_pinMap->setPinText(name);
			ui->gridLayout->addWidget(m_pinMap,i,j);
		}
	}
}
