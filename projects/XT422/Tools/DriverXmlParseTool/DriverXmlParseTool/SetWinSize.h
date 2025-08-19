#pragma once

#include <QtWidgets/QDialog>
namespace Ui { class SetWinSize; };

class SetWinSize : public QDialog
{
	Q_OBJECT

public:
	SetWinSize(QWidget *parent = Q_NULLPTR);
	~SetWinSize();

	void SetWHSize(int w,int h);

signals:
	void sgnWHSize(int,int);
private:
	Ui::SetWinSize *ui;
};
