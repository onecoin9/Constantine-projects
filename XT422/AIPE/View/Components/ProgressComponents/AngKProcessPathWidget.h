#pragma once

#include <QtWidgets/QWidget>
namespace Ui { class AngKProcessPathWidget; };

class AngKProcessPathWidget : public QWidget
{
	Q_OBJECT

public:
	AngKProcessPathWidget(QWidget *parent = Q_NULLPTR);
	~AngKProcessPathWidget();

	void setPath(QString strPath);

private:
	void InitShadow();
private:
	Ui::AngKProcessPathWidget *ui;
};
