#pragma once

#include <QtWidgets/QDialog>
#include <QtWidgets/QWidget>
#include "DataJsonSerial.hpp"
namespace Ui { class AngKChipDialog; };

class AngKChipDialog : public QDialog
{
	Q_OBJECT

public:
	AngKChipDialog(QWidget *parent = Q_NULLPTR);
	~AngKChipDialog();

	void SetTitle(QString strTitle);

	void InitChipData();

signals:

	void sgnSelectChipDataJson(ChipDataJsonSerial);

protected:
	virtual void mousePressEvent(QMouseEvent* event);

	virtual void mouseMoveEvent(QMouseEvent* event);
private:
	Ui::AngKChipDialog*ui;
	QPoint clickPos;
};
