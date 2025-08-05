#pragma once

#include <QtWidgets/QWidget>
namespace Ui { class AngKLoginInput; };

class AngKLoginInput : public QWidget
{
	Q_OBJECT

public:
	AngKLoginInput(QWidget *parent = Q_NULLPTR);
	~AngKLoginInput();

	void InitText();

	void setLoginPath(QString strPath);
protected:
	bool eventFilter(QObject* obj, QEvent* event) override;
signals:
	void sgnRequstLogin(int, QString, QString);

public slots:
	void onSlotClickLogin();
private:
	Ui::AngKLoginInput *ui;
};
