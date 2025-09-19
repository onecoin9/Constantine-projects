#include "AngKSNCreatorDlg.h"
#include "ui_AngKSNCreatorDlg.h"
#include "../View/GlobalInit/StyleInit.h"

AngKSNCreatorDlg::AngKSNCreatorDlg(QWidget *parent)
	: AngKDialog(parent)
{
	this->setObjectName("AngKSNCreatorDlg");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::AngKSNCreatorDlg();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(750, 550);
	this->SetTitle(tr("SNC Creator"));

	ui->SNCWidget->SetHideSNCFile(false);

	connect(this, &AngKSNCreatorDlg::sgnClose, this, &AngKSNCreatorDlg::close);
}

AngKSNCreatorDlg::~AngKSNCreatorDlg()
{
	delete ui;
}

