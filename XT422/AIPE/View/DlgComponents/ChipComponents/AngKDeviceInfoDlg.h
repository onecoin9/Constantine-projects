#pragma once

#include "AngKDialog.h"
#include "AngKProjDataset.h"
namespace Ui { class AngKDeviceInfoDlg; };

class AngKDeviceInfoDlg : public AngKDialog
{
	Q_OBJECT

public:
	AngKDeviceInfoDlg(QWidget *parent = Q_NULLPTR);
	~AngKDeviceInfoDlg();

	void SetChipDeviceInfo(ChipDataJsonSerial chipInfo);
private:
	Ui::AngKDeviceInfoDlg *ui;
};
