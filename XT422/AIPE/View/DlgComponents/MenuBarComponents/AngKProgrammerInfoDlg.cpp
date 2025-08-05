#include "AngKProgrammerInfoDlg.h"
#include "ui_AngKProgrammerInfoDlg.h"
#include "StyleInit.h"
#include <QDateTime>
AngKProgrammerInfoDlg::AngKProgrammerInfoDlg(QWidget *parent)
	: AngKDialog(parent)
{
	this->setObjectName("AngKProgrammerInfoDlg");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::AngKProgrammerInfoDlg();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(636, 200);
	this->SetTitle(tr("Programmer Info"));

	InitText();

	connect(this, &AngKProgrammerInfoDlg::sgnClose, this, &AngKProgrammerInfoDlg::close);
}

AngKProgrammerInfoDlg::~AngKProgrammerInfoDlg()
{
	delete ui;
}

void AngKProgrammerInfoDlg::InitText()
{
	ui->clientNameText->setText(tr("Client Name:"));
	ui->maintainText->setText(tr("The last maintain:"));
	ui->uploadButton->setText(tr("Upload"));
	ui->downloadButton->setText(tr("Download"));

	ui->maintainDateEdit->setDisplayFormat("yyyy/MM/dd");
	ui->maintainDateEdit->setDateTime(QDateTime::currentDateTime());
}
