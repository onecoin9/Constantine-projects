#include "AngKTableView.h"
#include "AngKItemDelegate.h"
#include "../View/GlobalInit/StyleInit.h"
#include <QMouseEvent>
#include <QScrollBar>

AngKTableView::AngKTableView(QWidget *parent)
	: QTableView(parent)
	, m_pModel(nullptr)
{
	setMouseTracking(true);
	setSelectionBehavior(SelectRows);

	AngKItemDelegate* delegate = new AngKItemDelegate(this);
	this->setItemDelegate(delegate);

	//connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(on_verscrollBarChange(int)));

	this->setObjectName("AngKTableView");
	QT_SET_STYLE_SHEET(objectName());
}

AngKTableView::~AngKTableView()
{

}

void AngKTableView::mouseMoveEvent(QMouseEvent* event)
{
	QTableView::mouseMoveEvent(event);

	AngKItemDelegate* delegate = qobject_cast<AngKItemDelegate*>(this->itemDelegate());
	if (delegate)
	{
		QModelIndex index = indexAt(event->pos());
		if (delegate->setHoverRow(index.row()))
		{
			for (int col = 0; col < model()->columnCount(); ++col)
			{
				if (col == index.column())
				{
					continue;
				}
				update(model()->index(index.row(), col));
			}
		}
	}
}

void AngKTableView::leaveEvent(QEvent* event)
{
	QTableView::leaveEvent(event);

	AngKItemDelegate* delegate = qobject_cast<AngKItemDelegate*>(this->itemDelegate());
	if (delegate)
	{
		int last_hover_row = delegate->getHoverRow();
		if (last_hover_row >= 0)
		{
			delegate->setHoverRow(-1);
			for (int col = 0; col < model()->columnCount(); ++col)
			{
				update(model()->index(last_hover_row, col));
			}
		}
	}
}

void AngKTableView::wheelEvent(QWheelEvent* event)
{
	QTableView::wheelEvent(event);

	AngKItemDelegate* delegate = qobject_cast<AngKItemDelegate*>(this->itemDelegate());
	if (delegate)
	{
		QModelIndex index = indexAt(event->pos());
		if (delegate->setHoverRow(index.row()))
		{
			for (int col = 0; col < model()->columnCount(); ++col)
			{
				if (col == index.column())
				{
					continue;
				}
				update(model()->index(index.row(), col));
			}
		}
	}
	//int delta = event->delta();

	//int row = m_pModel->GetCurrentRow();
	//if (delta > 0)
	//{
	//	if (row < 5)
	//	{
	//		return;
	//	}
	//	else
	//	{
	//		row = row - 5;  //每次滚动滚轮，向上加载5个
	//	}
	//}
	//else
	//{
	//	if (m_pModel->GetDataSize() <= 0)
	//		return;

	//	if (row > m_pModel->GetDataSize() - 100)
	//	{
	//		row = m_pModel->GetDataSize() - 100;
	//	}
	//	else
	//	{
	//		row = row + 5;
	//	}

	//}

	//m_pModel->LoadData(row);
}

void AngKTableView::ReceiverData()
{
	ClearAllRow();
	std::vector<chip> dataList = m_pModel->GetFirstData();
	m_pModel->setRowCount(dataList.size());
	for (int i = 0; i < dataList.size(); i++)  //首次加载100个
	{
		chip chipModel = dataList[i];
		m_pModel->setItem(i, 0, new QStandardItem(QString::fromStdString(chipModel.strName)));
		m_pModel->setItem(i, 1, new QStandardItem(QString::fromStdString(chipModel.strManu)));
		m_pModel->setItem(i, 2, new QStandardItem(QString::fromStdString(chipModel.strAdapter)));
		m_pModel->setItem(i, 3, new QStandardItem(QString::fromStdString(chipModel.strPack)));
		m_pModel->setItem(i, 4, new QStandardItem(QString::fromStdString(chipModel.strType)));
		m_pModel->setItem(i, 5, new QStandardItem(""));
	}
}

void AngKTableView::ReceiverChanged()
{
	ClearAllRow();
	m_pModel->setRowCount(100);

	std::vector<chip> dataList = m_pModel->GetSendData();
	m_pModel->setRowCount(100);
	int k = 0;
	for (int i = 0; i < dataList.size(); i++)
	{
		chip chipModel = dataList[i];
		m_pModel->setItem(i, 0, new QStandardItem(QString::fromStdString(chipModel.strName)));
		m_pModel->setItem(i, 1, new QStandardItem(QString::fromStdString(chipModel.strManu)));
		m_pModel->setItem(i, 2, new QStandardItem(QString::fromStdString(chipModel.strAdapter)));
		m_pModel->setItem(i, 3, new QStandardItem(QString::fromStdString(chipModel.strPack)));
		m_pModel->setItem(i, 4, new QStandardItem(QString::fromStdString(chipModel.strType)));
		m_pModel->setItem(i, 5, new QStandardItem(""));
		k++;
	}
}

void AngKTableView::setDBModel(AngKDBStandardItemModel* model)
{
	m_pModel = model;
}

void AngKTableView::ClearAllRow()
{
	if (m_pModel == nullptr)
		return;

	for (int i = m_pModel->rowCount(); i > 1; i--)
	{
		m_pModel->removeRow(i);
	}
}

void AngKTableView::on_verscrollBarChange(int value)
{
	if (m_pModel == nullptr)
		return;

	if (value == this->verticalScrollBar()->maximum())
	{
		int row = m_pModel->GetCurrentRow();

		if (row > m_pModel->GetDataSize() - 200)
		{
			row = m_pModel->GetDataSize() - 100;
		}
		else
		{
			row = row + 100;
			this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum() * 0.9);
		}
		qDebug() << "row::" << row;
		m_pModel->LoadData(row);
	}
	if (value == this->verticalScrollBar()->minimum())
	{
		int row = m_pModel->GetCurrentRow();
		if (row >= 100)
		{
			row = row - 100;
			if (row < 100)
			{
				row = 0;
			}
			this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum() * 0.1);
		}
		qDebug() << "row::" << row;
		m_pModel->LoadData(row);
	}
}