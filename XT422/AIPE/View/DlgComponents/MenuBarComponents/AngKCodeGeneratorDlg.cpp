#include "AngKCodeGeneratorDlg.h"
#include "ui_AngKCodeGeneratorDlg.h"
#include "../View/GlobalInit/StyleInit.h"

AngKCodeGeneratorDlg::AngKCodeGeneratorDlg(QWidget *parent)
	: AngKDialog(parent)
{
	this->setObjectName("AngKCodeGeneratorDlg");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::AngKCodeGeneratorDlg();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(492, 230);
	this->SetTitle(tr("Code Generator"));

	InitText();

	connect(this, &AngKCodeGeneratorDlg::sgnClose, this, &AngKCodeGeneratorDlg::close);
}

AngKCodeGeneratorDlg::~AngKCodeGeneratorDlg()
{
	delete ui;
}

void AngKCodeGeneratorDlg::InitText()
{
	ui->randomCodeText->setText(tr("Random Code:"));
	ui->secureKeyText->setText(tr("Secure Key:"));
	ui->checkCodeText->setText(tr("Check Code:"));
	ui->checkCodeEdit->setReadOnly(true);
	ui->generateButton->setText(tr("Generate"));
	ui->exitButton->setText(tr("Exit"));
}
