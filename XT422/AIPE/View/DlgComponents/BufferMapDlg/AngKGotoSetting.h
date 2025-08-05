#pragma once

#include "AngKDialog.h"
#include <QWidget>
namespace Ui { class AngKGotoSetting; };

class AngKGotoSetting : public AngKDialog
{
	Q_OBJECT

public:
	AngKGotoSetting(QWidget *parent = Q_NULLPTR);
	~AngKGotoSetting();

signals:
	void sgnGoToAddress(QString);
private:
	Ui::AngKGotoSetting *ui;
};
