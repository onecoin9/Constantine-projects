#include "AngKDBStandardItemModel.h"
#include <QMetaType>

Q_DECLARE_METATYPE(std::vector<chip>);

AngKDBStandardItemModel::AngKDBStandardItemModel(QObject *parent)
	: QStandardItemModel(parent)
	, m_nCurrentRow(0)
{
	qRegisterMetaType<std::vector<chip>>("std::vector<chip>&");
	m_nTotalRows = 50;
}

AngKDBStandardItemModel::~AngKDBStandardItemModel()
{
}

void AngKDBStandardItemModel::updateViewData()
{
	this->removeRows(0, this->rowCount());
	this->setRowCount(m_nTotalRows);
	int k = 0;
	for (int i = m_nCurrentRow; i < m_nCurrentRow + m_nTotalRows; i++)
	{
		if (i == m_DataList.size()) break;
		chip chipModel = m_DataList[i];
		this->setItem(k, 0, new QStandardItem(QString::fromStdString(chipModel.strName)));
		this->setItem(k, 1, new QStandardItem(QString::fromStdString(chipModel.strName)));
		this->setItem(k, 2, new QStandardItem(QString::fromStdString(chipModel.strName)));
		this->setItem(k, 3, new QStandardItem(QString::fromStdString(chipModel.strName)));
		this->setItem(k, 4, new QStandardItem(QString::fromStdString(chipModel.strName)));
		this->setItem(k, 5, new QStandardItem(QString::fromStdString(chipModel.strName)));
		k++;
	}
}

int AngKDBStandardItemModel::GetCurrentRow()
{
	return m_nCurrentRow;
}

std::vector<chip> AngKDBStandardItemModel::GetDataList()
{
	return m_DataList;
}

void AngKDBStandardItemModel::LoadData(int row)
{
	m_SendData.clear();
	for (int i = row; i < row + 100; i++)
	{
		chip data = m_DataList[i];
		m_SendData.push_back(data);
	}
	m_nCurrentRow = row;
	emit sSendValueChanaged();

}

std::vector<chip> AngKDBStandardItemModel::GetSendData()
{
	return m_SendData;
}

std::vector<chip> AngKDBStandardItemModel::GetFirstData()
{
	return m_FirstData;
}

int AngKDBStandardItemModel::GetDataSize()
{
	return m_DataSize;
}

void AngKDBStandardItemModel::ReceiverChanged(int row)
{
	m_nCurrentRow += row;
	if (m_nCurrentRow<0 || m_nCurrentRow>m_DataList.size() - 50)
	{
		m_nCurrentRow -= row;
		return;
	}
	updateViewData();
}

void AngKDBStandardItemModel::CreateData(std::vector<chip> chipVec)
{
	m_DataList = chipVec;
	for (int i = 0; i < m_nTotalRows; i++)
	{
		chip chipModel = m_DataList[i];
		this->setItem(i, 0, new QStandardItem(QString::fromStdString(chipModel.strName)));
		this->setItem(i, 1, new QStandardItem(QString::fromStdString(chipModel.strName)));
		this->setItem(i, 2, new QStandardItem(QString::fromStdString(chipModel.strName)));
		this->setItem(i, 3, new QStandardItem(QString::fromStdString(chipModel.strName)));
		this->setItem(i, 4, new QStandardItem(QString::fromStdString(chipModel.strName)));
		this->setItem(i, 5, new QStandardItem(QString::fromStdString(chipModel.strName)));

		if (i <= 100)
		{
			m_FirstData.push_back(chipModel);
		}

		if (i == 100)
		{
			emit sSendFinished();
		}
	}
	m_DataSize = m_DataList.size();
}

void AngKDBStandardItemModel::on_verscrollBar(double value)
{
	m_nCurrentRow = value * m_DataList.size();
	if (m_nCurrentRow > m_DataList.size() - 40)
		m_nCurrentRow = m_DataList.size() - 40;
	updateViewData();
}
