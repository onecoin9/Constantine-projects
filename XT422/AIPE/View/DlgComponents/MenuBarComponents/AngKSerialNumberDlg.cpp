#include "AngKSerialNumberDlg.h"
#include "ui_AngKSerialNumberDlg.h"
#include "../View/GlobalInit/StyleInit.h"
#include "GlobalDefine.h"

AngKSerialNumberDlg::AngKSerialNumberDlg(QWidget *parent)
	: AngKDialog(parent)
{
	this->setObjectName("AngKSerialNumberDlg");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::AngKSerialNumberDlg();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(750, 550);
	this->SetTitle(tr("Serial Number Config"));

	ui->disableButton->setChecked(true);
	ui->stackedWidget->setCurrentIndex(0);

	InitText();
	InitRadioButton();
}

AngKSerialNumberDlg::~AngKSerialNumberDlg()
{
	delete ui;
}

void AngKSerialNumberDlg::InitText()
{
	ui->disableButton->setText(tr("Disable"));
	ui->modeButton->setText(tr("Incremental Mode"));
	ui->fromFileButton->setText(tr("From File"));
	ui->SNCModeButton->setText(tr("SNC Mode"));

	ui->disableTip->setText(tr("No SN configuration!"));
	ui->SNConfigGroup->setTitle(tr("SN Configuration"));

	ui->snSizeText->setText(tr("S/N Size"));
	ui->snSizeTextTip->setText(tr("bytes"));
	ui->startAddrText->setText(tr("Start Address"));
	ui->startAddrTextTip->setText(tr("0h..90600000h"));
	ui->startValueText->setText(tr("Start Value"));
	ui->startValueTextTip->setText(tr("h"));
	ui->stepText->setText(tr("Step"));
	ui->stepTextTip->setText(tr("h"));
	ui->SNmodeGroup->setTitle(tr("S/N mode"));
	ui->asciiButton->setText(tr("ASCII"));
	ui->binButton->setText(tr("BIN"));
	ui->styleGroup->setTitle(tr("Style"));
	ui->decButton->setText(tr("DEC(9->10)"));
	ui->hexButton->setText(tr("HEX(9->A)"));
	ui->bufferGroup->setTitle(tr("Save To Buffer"));
	ui->LSByteFirstButton->setText(tr("LS Byte First"));
	ui->MSByteFirstButton->setText(tr("MS Byte First"));

	ui->fileNameText->setText(tr("File Name"));
	ui->startLabelText->setText(tr("Start Label"));

	ui->modeGroup_1->setTitle(tr("Mode 1"));
	ui->SNCFilePathText->setText(tr("Select SNC File"));
	ui->modeGroup_2->setTitle(tr("Mode 2"));
	ui->generateSNCButton->setText(tr("Generate SNC"));

	ui->okButton->setText(tr("Ok"));
	ui->cancelButton->setText(tr("Cancel"));
}

void AngKSerialNumberDlg::InitRadioButton()
{
	ui->disableButton->setProperty("page", (int)SerialConfigType::Disable);
	ui->modeButton->setProperty("page", (int)SerialConfigType::Incremental_Mode);
	ui->fromFileButton->setProperty("page", (int)SerialConfigType::From_File);
	ui->SNCModeButton->setProperty("page", (int)SerialConfigType::SNC_Mode);

	connect(ui->disableButton, &QRadioButton::clicked, this, &AngKSerialNumberDlg::onSlotChangePage);
	connect(ui->modeButton, &QRadioButton::clicked, this, &AngKSerialNumberDlg::onSlotChangePage);
	connect(ui->fromFileButton, &QRadioButton::clicked, this, &AngKSerialNumberDlg::onSlotChangePage);
	connect(ui->SNCModeButton, &QRadioButton::clicked, this, &AngKSerialNumberDlg::onSlotChangePage);

	connect(this, &AngKSerialNumberDlg::sgnClose, this, &AngKSerialNumberDlg::close);
}

void AngKSerialNumberDlg::onSlotChangePage()
{
	QRadioButton* radio = qobject_cast<QRadioButton*>(sender());

	if(radio != nullptr)
		ui->stackedWidget->setCurrentIndex(radio->property("page").toInt());
}