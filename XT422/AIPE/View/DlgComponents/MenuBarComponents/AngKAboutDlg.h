#pragma once

#include "AngKDialog.h"
namespace Ui { class AngKAboutDlg; };

class AngKAboutDlg : public AngKDialog
{
	Q_OBJECT

public:
	AngKAboutDlg(QWidget *parent = Q_NULLPTR);
	~AngKAboutDlg();

	void InitText();
private:
	Ui::AngKAboutDlg *ui;
};
