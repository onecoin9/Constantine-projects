#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QWidgetAction>
#include <QTimer>
#include "AngKShadowWindow.h"

namespace Ui { class AngKLoginDialog; };

class AngKLoginDialog : public AngKShadowWindow<QDialog>
{	
	Q_OBJECT

public:
	AngKLoginDialog(QWidget *parent = Q_NULLPTR);
	~AngKLoginDialog();

	void InitChooseWgt();
public slots:
	void onSlotSetChooseIntroLabel(QString);
	void onSlotOpenManualLoginPage(QObject*);
	void onSlotRequstLogin(int, QString, QString);
private:
	Ui::AngKLoginDialog *ui;
};
