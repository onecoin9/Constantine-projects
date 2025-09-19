#pragma once

#include "AngKDialog.h"
namespace Ui { class AngKProgrammerInfoDlg; };

class AngKProgrammerInfoDlg : public AngKDialog
{
	Q_OBJECT

public:
	AngKProgrammerInfoDlg(QWidget *parent = Q_NULLPTR);
	~AngKProgrammerInfoDlg();

	void InitText();
private:
	Ui::AngKProgrammerInfoDlg *ui;
};
