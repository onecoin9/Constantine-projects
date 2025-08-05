#pragma once

#include "AngKDialog.h"
namespace Ui { class AngKDriverParam; };

class AngKDriverParam : public AngKDialog
{
	Q_OBJECT

public:
	AngKDriverParam(QWidget *parent = Q_NULLPTR);
	~AngKDriverParam();

private:
	void initButton();
private:
	Ui::AngKDriverParam *ui;
};
