#pragma once

#include "AngKDialog.h"
namespace Ui { class AngKNandImageDlg; };

class AngKNandImageDlg : public AngKDialog
{
	Q_OBJECT

public:
	AngKNandImageDlg(QWidget *parent = Q_NULLPTR);
	~AngKNandImageDlg();

	void InitText();
private:
	Ui::AngKNandImageDlg *ui;
};
