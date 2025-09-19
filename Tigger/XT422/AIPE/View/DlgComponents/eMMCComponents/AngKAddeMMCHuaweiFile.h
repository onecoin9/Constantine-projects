#pragma once

#include "AngKDialog.h"
namespace Ui { class AngKAddeMMCHuaweiFile; };

class AngKAddeMMCHuaweiFile : public AngKDialog
{
	Q_OBJECT

public:
	AngKAddeMMCHuaweiFile(QWidget *parent = Q_NULLPTR);
	~AngKAddeMMCHuaweiFile();

	void InitButton();

signals:
	void sgnAddHuaweiFile(QString, QString);
private:
	Ui::AngKAddeMMCHuaweiFile *ui;
};
