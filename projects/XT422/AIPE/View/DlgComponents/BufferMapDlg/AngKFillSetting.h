#pragma once

#include "AngKDialog.h"
#include <QWidget>
namespace Ui { class AngKFillSetting; };

enum SettingType{
	fillType,
	findType
};

class AngKFillSetting : public AngKDialog
{
	Q_OBJECT

public:
	AngKFillSetting(QString title, SettingType ntype, QWidget *parent = Q_NULLPTR);
	~AngKFillSetting();

	void switchSettingDlg(SettingType nType);

	void InitButton();

signals:
	void sgnFillBuffer(QString, QString, QString, bool, bool);
	void sgnFindBuffer(QString, QString, QString, bool);
	void sgnFindNextBuffer(QString, QString, QString, bool);
	
private:
	Ui::AngKFillSetting *ui;
};
