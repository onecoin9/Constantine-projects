#include "AngKNandImageDlg.h"
#include "ui_AngKNandImageDlg.h"
#include "../View/GlobalInit/StyleInit.h"

AngKNandImageDlg::AngKNandImageDlg(QWidget *parent)
	: AngKDialog(parent)
{
	this->setObjectName("AngKNandImageDlg");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::AngKNandImageDlg();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(636, 150);
	this->SetTitle(tr("NandImage"));

	InitText();

	connect(this, &AngKNandImageDlg::sgnClose, this, &AngKNandImageDlg::close);
}

AngKNandImageDlg::~AngKNandImageDlg()
{
	delete ui;
}

void AngKNandImageDlg::InitText()
{
	ui->formatText->setText(tr("Format Select: "));
	ui->okButton->setText(tr("Ok"));
	ui->cancelButton->setText(tr("Cancel"));
}
