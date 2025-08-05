#pragma once

#include <QWidget>
namespace Ui { class AngKPinMapSetting; };

class AngKPinMapSetting : public QWidget
{
	Q_OBJECT

public:
	AngKPinMapSetting(QWidget *parent = Q_NULLPTR);
	~AngKPinMapSetting();

	void setPinText(QString strPin);
private:
	Ui::AngKPinMapSetting *ui;
};
