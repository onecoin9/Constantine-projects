#pragma once

#include "AngKDialog.h"
namespace Ui { class AngKemmCDriverPara; };

class QSettings;
class QStandardItemModel;
class AngKemmCDriverPara : public AngKDialog
{
	Q_OBJECT

public:
	AngKemmCDriverPara(QWidget *parent = Q_NULLPTR);
	~AngKemmCDriverPara();

	void InitTable();

	void InsertFixedData();

	void InitButton();

	void SetLocfgIni(QSettings* _setting);

	void ReadLocfgIni();

	QVariant settingReadValue(QString strGroup, QString strValue);

	void settingWriteValue(QString strGroup, QString strName, QVariant strValue);
public slots:

	void onSlotSaveConfigDriverPara();

	void onSlotOK();
private:
	Ui::AngKemmCDriverPara *ui;
	QStandardItemModel* m_DriverParaTableModel;
	QSettings* m_DriverParaSetting;
};
