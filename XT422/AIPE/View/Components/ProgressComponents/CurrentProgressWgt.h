#pragma once

#include <QtWidgets/QWidget>
namespace Ui { class CurrentProgressWgt; };

class CurrentProgressWgt : public QWidget
{
	Q_OBJECT

public:
	CurrentProgressWgt(QWidget *parent = Q_NULLPTR);
	~CurrentProgressWgt();

	void setCurrentProgress(int value);
private:
	Ui::CurrentProgressWgt *ui;
};
