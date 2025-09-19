#pragma once

#include <QtWidgets/QWidget>
namespace Ui { class AngKSNCreator; };

class AngKSNCreator : public QWidget
{
	Q_OBJECT

public:
	AngKSNCreator(QWidget *parent = Q_NULLPTR);
	~AngKSNCreator();

	void InitText();

	void SetHideSNCFile(bool isHide);
private:
	Ui::AngKSNCreator *ui;
};
