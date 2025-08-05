#pragma once

#include "AngKDialog.h"
namespace Ui { class AngKCodeGeneratorDlg; };

class AngKCodeGeneratorDlg : public AngKDialog
{
	Q_OBJECT

public:
	AngKCodeGeneratorDlg(QWidget *parent = Q_NULLPTR);
	~AngKCodeGeneratorDlg();

	void InitText();
private:
	Ui::AngKCodeGeneratorDlg *ui;
};
