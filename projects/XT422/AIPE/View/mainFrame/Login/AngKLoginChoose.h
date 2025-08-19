#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QListView>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
namespace Ui { class AngKLoginChoose; };

class QStandardItemModel;
class AngKLoginChoose : public QWidget
{
	Q_OBJECT

public:
	AngKLoginChoose(QWidget *parent = Q_NULLPTR);
	~AngKLoginChoose();

	void setBackgroudProperty(QString bgName);

	void setIntroProperty(QString introName);

	void setTitle(QString titleName);

	void setWidgetShadow();

	void AppendItem(QString strName);

signals:
	void sgnOpenLoginPage(QObject*);

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
	virtual void paintEvent(QPaintEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
private:
	Ui::AngKLoginChoose *ui;
	QStandardItemModel* m_pComboBoxModel;
	bool ishover;
};
