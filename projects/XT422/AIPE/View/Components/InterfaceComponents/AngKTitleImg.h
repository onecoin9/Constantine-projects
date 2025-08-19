#pragma once

#include <QtWidgets/QWidget>

namespace Ui { class AngKTitleImg; };

class QLabel;
class AngKTitleImg : public QWidget
{
	Q_OBJECT

public:
	AngKTitleImg(QWidget *parent = Q_NULLPTR);
	~AngKTitleImg();

	QLabel* imgLabel();

	QLabel* nameLabel();
private:
	Ui::AngKTitleImg *ui;
};
