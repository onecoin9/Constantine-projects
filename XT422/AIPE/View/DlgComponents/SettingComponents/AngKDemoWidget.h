#pragma once

#include <QWidget>
namespace Ui { class AngKDemoWidget; };

class AngKDemoWidget : public QWidget
{
	Q_OBJECT

public:
	AngKDemoWidget(QWidget *parent = Q_NULLPTR);
	~AngKDemoWidget();


signals:
	void sgnEnterDemoMode();
private:
	Ui::AngKDemoWidget *ui;
};
