#pragma once

#include <QWidget>
namespace Ui { class WarningNum; };

class WarningNum : public QWidget
{
	Q_OBJECT

public:
	WarningNum(QWidget *parent = Q_NULLPTR);
	~WarningNum();

	void setWarnNums(int nums);

	void setIsWarn(bool bWarn);
private:
	Ui::WarningNum *ui;
};
