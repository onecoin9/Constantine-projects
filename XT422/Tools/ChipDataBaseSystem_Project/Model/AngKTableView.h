#pragma once

#include <QTableView>
#include "../DataBaseOper/AngKDBStandardItemModel.h"

class AngKTableView : public QTableView
{
	Q_OBJECT

public:
	explicit AngKTableView(QWidget *parent);
	~AngKTableView();

	void setDBModel(AngKDBStandardItemModel* model);
	 void ClearAllRow();
protected:
	void mouseMoveEvent(QMouseEvent* event);
	void leaveEvent(QEvent* event);
	void wheelEvent(QWheelEvent* event);

public slots:
    void on_verscrollBarChange(int value);
private:
	AngKDBStandardItemModel*	m_pModel;
};
