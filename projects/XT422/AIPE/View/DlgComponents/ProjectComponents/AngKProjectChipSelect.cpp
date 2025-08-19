#include "AngKProjectChipSelect.h"
#include "ui_AngKProjectChipSelect.h"
#include "../View/GlobalInit/StyleInit.h"
#include "AngKChipDialog.h"
#include "AngKTransmitSignals.h"
#include <QDesktopWidget>
AngKProjectChipSelect::AngKProjectChipSelect(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::AngKProjectChipSelect();
	ui->setupUi(this);
	setAttribute(Qt::WA_TranslucentBackground, true);

	InitText();

	ui->horizontalWidget_6->setVisible(false);

	this->setObjectName("AngKProjectChipSelect");
	QT_SET_STYLE_SHEET(objectName());
}

AngKProjectChipSelect::~AngKProjectChipSelect()
{
	m_chipJson.ClearJson();
	delete ui;
}

void AngKProjectChipSelect::InitText()
{
	ui->nameLabel->setText(tr("Name"));
	ui->adapterLabel->setText(tr("Adapter"));
	ui->typeLabel->setText(tr("Type"));
	ui->manuLabel->setText(tr("Manufacurer"));
	ui->packageLabel->setText(tr("Package"));
	ui->bottomLabel->setText(tr("Bottom Board"));
	ui->groupBox->setTitle(tr(" DrvFile "));
	ui->drvNameLabel->setText(tr("DrvName"));
	ui->drvDivisionLabel->setText(tr("DrvVision"));
	ui->drvDateLabel->setText(tr("DrvDate"));
	ui->drvSNLabel->setText(tr("DrvSN"));
	ui->drvAuthorLabel->setText(tr("DrvAuthor"));
	ui->IDLabel->setText(tr("ChipID"));
	ui->SKTIDLabel->setText(tr("SKTID"));
	ui->minByteLabel->setText(tr("ProgMinByte"));
	ui->pinMapLabel->setText(tr("PinMap"));
	ui->helpIdxLabel->setText(tr("HelpIdx"));
	ui->helpNameLabel->setText(tr("HelpName"));
	ui->groupBox_2->setTitle(tr(" Pinmap ")); 
	ui->bufAddrLabel->setText(tr("BufAddr"));
	ui->fileAddrLabel->setText(tr("FileAddr"));
	ui->dataLenLabel->setText(tr("DataLen"));
	ui->groupBox_3->setTitle(tr(" FPGA "));
	ui->FPGANameLabel->setText(tr("Name"));
	ui->FPGAVersionLabel->setText(tr("Version"));
	ui->FPGADateLabel->setText(tr("Data"));
	ui->ExtFileNameLabel->setText(tr("ExtFileName"));
	ui->SelfDefineDescriptionLabel->setText(tr("SelfDefineDescription"));
	//ui->selectButton->hide();
	ui->selectButton->setText(tr("Select Chip"));
	connect(ui->selectButton, &QPushButton::clicked, this, &AngKProjectChipSelect::onSlotSelectChip);
}

void AngKProjectChipSelect::insertChipText(ChipDataJsonSerial serJson)
{
	nlohmann::json jsonInfo = serJson.DataJsonSerial();
	if (!jsonInfo.empty())
	{
		ui->nameEdit->setText(QString::fromStdString(jsonInfo["chipName"]));
		ui->adapterLineEdit->setText(QString::fromStdString(jsonInfo["chipAdapter"]));
		ui->typeLineEdit->setText(QString::fromStdString(jsonInfo["chipType"]));
		ui->manuLineEdit->setText(QString::fromStdString(jsonInfo["manufacture"]));
		ui->packageLineEdit->setText(QString::fromStdString(jsonInfo["chipPackage"]));
		ui->bottomLineEdit->setText(QString::fromStdString(jsonInfo["bottomBoard"]));
		ui->drvNameEdit->setText(QString::fromStdString(jsonInfo["chipAlgoFile"]));//TODO 要获取驱动文件信息
		try {
			ui->IDLineEdit->setText(QString::fromStdString(jsonInfo["chipId"]));
		}
		catch(...){
			unsigned long chipId = jsonInfo["chipId"];
			ui->IDLineEdit->setText(QString::number(chipId));
		};
		ui->FPGANameEdit->setText(QString::fromStdString(jsonInfo["chipFPGAFile"]));
		ui->ExtFileNameEdit->setText(QString::fromStdString(jsonInfo["chipAppFile"]));
	}
}

void AngKProjectChipSelect::onSlotSelectChip()
{
	AngKChipDialog chipDlg(this);
	chipDlg.SetTitle("Select Chip");
	chipDlg.InitChipData();
	connect(&chipDlg, &AngKChipDialog::sgnSelectChipDataJson, this, &AngKProjectChipSelect::onSlotSelectChipDataJson);
	chipDlg.exec();
	disconnect(&chipDlg, &AngKChipDialog::sgnSelectChipDataJson, this, &AngKProjectChipSelect::onSlotSelectChipDataJson);
}

void AngKProjectChipSelect::onSlotSelectChipDataJson(ChipDataJsonSerial jsonChip)
{
	insertChipText(jsonChip);

	m_chipJson = jsonChip;

	emit sgnChipDataReport(m_chipJson);
}
