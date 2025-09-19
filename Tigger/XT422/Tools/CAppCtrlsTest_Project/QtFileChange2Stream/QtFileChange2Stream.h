#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtFileChange2Stream.h"


class QSettings;
class QtFileChange2Stream : public QMainWindow
{
	Q_OBJECT

public:
	QtFileChange2Stream(QWidget *parent = Q_NULLPTR);

	void InitSetting();

	QString localGlobalSettingFile(bool& isCreate);
private:
	QVariant settingReadValue(QString strGroup, QString strValue);

	void settingWriteValue(QString strGroup, QString strName, QVariant strValue);
public slots:
	void onSlotFileChange();

	void onSlotDrvFileFusion();
private:
	Ui::QtFileChange2StreamClass ui;
	QSettings* m_globalSettings;
};
