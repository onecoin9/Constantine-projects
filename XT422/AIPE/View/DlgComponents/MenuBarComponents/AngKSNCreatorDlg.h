#pragma once

#include "AngKDialog.h"
namespace Ui { class AngKSNCreatorDlg; };

class AngKSNCreatorDlg : public AngKDialog
{
	Q_OBJECT

public:
	AngKSNCreatorDlg(QWidget *parent = Q_NULLPTR);
	~AngKSNCreatorDlg();

private:
	Ui::AngKSNCreatorDlg *ui;
};
