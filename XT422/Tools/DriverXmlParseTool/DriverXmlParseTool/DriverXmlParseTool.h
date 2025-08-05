#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QFrame>
#include "ui_DriverXmlParseTool.h"
#include "DriverParseWidget.h"

class DriverXmlParseTool : public QMainWindow
{
	Q_OBJECT

public:
	DriverXmlParseTool(QWidget *parent = Q_NULLPTR);

	void SetupWidgets();

	void InitMenuBar();
private:
	Ui::DriverXmlParseToolClass ui;

	DriverParseWidget* m_pDriverParseWgt;

	QMenuBar* m_menuBar;
};
