#pragma once

#include "AngKDialog.h"
namespace Ui { class AngKSerialNumberDlg; };

class AngKSerialNumberDlg : public AngKDialog
{
	Q_OBJECT

public:
	AngKSerialNumberDlg(QWidget *parent = Q_NULLPTR);
	~AngKSerialNumberDlg();

	void InitText();

	void InitRadioButton();
public slots:
	void onSlotChangePage();
private:
	Ui::AngKSerialNumberDlg *ui;
};
