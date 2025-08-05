#pragma once

#include <QtWidgets/QWidget>
namespace Ui { class AngKControlWidget; };

class AngKControlWidget : public QWidget
{
	Q_OBJECT

public:
	AngKControlWidget(QWidget *parent = Q_NULLPTR);
	~AngKControlWidget();

	void SetAllSelect(bool bCheck);
private:
	void InitShadow();
	void InitButton();
signals:
	void sgnAllCheck(int);
	void sgnStartBurnData();

private:
	Ui::AngKControlWidget *ui;
};
