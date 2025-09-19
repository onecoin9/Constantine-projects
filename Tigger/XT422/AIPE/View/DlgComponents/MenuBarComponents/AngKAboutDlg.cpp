#include "AngKAboutDlg.h"
#include "ui_AngKAboutDlg.h"
#include "StyleInit.h"
#include "AngKGlobalInstance.h"

AngKAboutDlg::AngKAboutDlg(QWidget *parent)
	: AngKDialog(parent)
{
	this->setObjectName("AngKAboutDlg");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::AngKAboutDlg();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(500, 400);
	this->SetTitle(tr("About"));

	InitText();

	connect(ui->okButton, &QPushButton::clicked, this, &AngKAboutDlg::close);
	connect(this, &AngKAboutDlg::sgnClose, this, &AngKAboutDlg::close);
}

AngKAboutDlg::~AngKAboutDlg()
{
	delete ui;
}

void AngKAboutDlg::InitText()
{
	ui->progNameLabel->setText(tr("Acroview Programmer"));
	ui->versionLabel->setText("V" + AngKGlobalInstance::ReadValue("Version", "BuildVer").toString());
	ui->copyRightLabel->setText(QString("Copyright %1 2013-2025 Acroview. All Rights Reserved.昂科技术 版权所有").arg(QChar(0xA9)));
	ui->customGroup->setTitle(tr("Custom Support"));
}
