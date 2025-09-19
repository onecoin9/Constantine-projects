#pragma once

#include <QtWidgets/QWidget>

namespace Ui { class WarningWgt; };

class WarningWgt : public QWidget
{
	Q_OBJECT

public:
	WarningWgt(QWidget *parent = Q_NULLPTR);
	~WarningWgt();

	void setOutputDevNums(int nums);

	void setAllDevNums(int nums);

	int GetOutputDevNums();

	int GetAllDevNums();

	void setWarnWgtNums(int nums);

	void setWarnHidden(bool bWarn);

	void AddOutputNum();

	float GetCurrentYield();
private:
	Ui::WarningWgt *ui;
};
