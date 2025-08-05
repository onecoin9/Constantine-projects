#include "AngKFillSetting.h"
#include "ui_AngKFillSetting.h"
#include "StyleInit.h"
#include <QCheckBox>
#include <QPushButton>
#include <QRegExpValidator>

AngKFillSetting::AngKFillSetting(QString title, SettingType ntype, QWidget *parent)
	: AngKDialog(parent)
{
	this->setObjectName("AngKGotoSetting");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::AngKFillSetting();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(400, 220);
	this->SetTitle(title);

	switchSettingDlg(ntype);

	InitButton();
}

AngKFillSetting::~AngKFillSetting()
{
	delete ui;
}

void AngKFillSetting::switchSettingDlg(SettingType nType)
{
	switch (nType)
	{
	case fillType:
		{
			ui->useRandomCheck->show();
			ui->okButton->show();
			ui->findButton->hide();
			ui->nextButton->hide();
		}
		break;
	case findType:
		{
			ui->useRandomCheck->hide();
			ui->okButton->hide();
			ui->findButton->show();
			ui->nextButton->show();
		}
		break;
	default:
		break;
	}
}

void AngKFillSetting::InitButton()
{
	QRegExp hexRegexADDR("[0-9A-Fa-f]{1,8}");
	ui->StartEdit->setValidator(new QRegExpValidator(hexRegexADDR, ui->StartEdit));
	ui->StartEdit->setMaxLength(8);

	ui->endEdit->setValidator(new QRegExpValidator(hexRegexADDR, ui->endEdit));
	ui->endEdit->setMaxLength(8);

	connect(ui->okButton, &QPushButton::clicked, this, [=]() {
		bool randomCheck = ui->useRandomCheck->isChecked();
		bool asciiType = ui->asciiTypeCheck->isChecked();
		emit sgnFillBuffer(ui->StartEdit->text(), ui->endEdit->text(), ui->dataEdit->text(), asciiType, randomCheck);
	});

	connect(ui->findButton, &QPushButton::clicked, this, [=]() {
		bool asciiType = ui->asciiTypeCheck->isChecked();
		emit sgnFindBuffer(ui->StartEdit->text(), ui->endEdit->text(), ui->dataEdit->text(), asciiType);
	});

	connect(ui->nextButton, &QPushButton::clicked, this, [=]() {
		bool asciiType = ui->asciiTypeCheck->isChecked();
		emit sgnFindNextBuffer(ui->StartEdit->text(), ui->endEdit->text(), ui->dataEdit->text(), asciiType);
	});

	connect(ui->useRandomCheck, &QCheckBox::clicked, this, [=](bool isCheck) {
		if (isCheck){
			ui->asciiTypeCheck->setEnabled(true);
			ui->dataEdit->setEnabled(true);
		}
		else {
			ui->asciiTypeCheck->setEnabled(false);
			ui->dataEdit->setEnabled(false);
		}
	});

	connect(this, &AngKFillSetting::sgnClose, this, &AngKFillSetting::close);
}
