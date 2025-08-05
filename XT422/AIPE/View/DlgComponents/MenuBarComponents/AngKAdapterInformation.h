#pragma once

#include "AngKDialog.h"
namespace Ui { class AngKAdapterInformation; };

class AngKPinMapSetting;
class AngKAdapterInformation : public AngKDialog
{
	Q_OBJECT

public:
	AngKAdapterInformation(QWidget *parent = Q_NULLPTR);
	~AngKAdapterInformation();

	void InitText();

	void InitPinMap();
private:
	Ui::AngKAdapterInformation *ui;
};
