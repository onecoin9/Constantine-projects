#pragma once

#include <QtWidgets/QWidget>
namespace Ui { class YieldRateWgt; };

class YieldRateWgt : public QWidget
{
	Q_OBJECT

public:
	YieldRateWgt(QWidget *parent = Q_NULLPTR);
	~YieldRateWgt();

	void setPreLabel(float strPre, bool bAdvance);

	void setRoundProgressValue(int value);
private:
	Ui::YieldRateWgt *ui;
};
