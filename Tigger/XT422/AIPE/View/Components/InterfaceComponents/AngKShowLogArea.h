#pragma once

#include <QtWidgets/QWidget>
namespace Ui { class AngKShowLogArea; };

class AngKShowLogArea : public QWidget
{
	Q_OBJECT

public:
	AngKShowLogArea(QWidget *parent = Q_NULLPTR);
	~AngKShowLogArea();

	void setLogText(QString text);

	int GetUIMinHeight();

	void SetExpandCheck(bool bCheck);

	bool GetExpandCheck();
signals:
	void sgnLogAreaExpand(int);

private:
	Ui::AngKShowLogArea *ui;
	int	m_nMinHeight;
};
