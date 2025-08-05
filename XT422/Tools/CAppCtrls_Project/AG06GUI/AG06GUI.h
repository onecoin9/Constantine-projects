#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_AG06GUI.h"

class AG06GUI : public QMainWindow
{
	Q_OBJECT

public:
	AG06GUI(QWidget *parent = Q_NULLPTR);

private:
	Ui::AG06GUIClass ui;
};
