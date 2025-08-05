#pragma once

#include <QStandardItemModel>
#include "ChipDBModel.h"
class AngKDBStandardItemModel : public QStandardItemModel
{
	Q_OBJECT

public:
	AngKDBStandardItemModel(QObject *parent = nullptr);
	~AngKDBStandardItemModel();

public:
	void updateViewData();
	int GetCurrentRow();
	std::vector<chip> GetDataList();
	void LoadData(int row);
	std::vector<chip> GetSendData();
	std::vector<chip> GetFirstData();
	int GetDataSize();

signals:
	void sSendFinished();
	void sSendValueChanaged();

public slots:
	void on_verscrollBar(double value);
	void ReceiverChanged(int row);
	void CreateData(std::vector<chip> chipVec);

private:
	int m_nCurrentRow;
	int m_nTotalRows;
	int m_DataSize = 0;
	std::vector<chip>	m_DataList;
	std::vector<chip>	m_SendData;
	std::vector<chip>	m_FirstData;
};

//使用方法：

//信号槽注册新类型
//qRegisterMetaType<std::vector<chip>>("std::vector<chip>&");
////使用自定义数据库model
//m_DBStandardItemModel = new AngKDBStandardItemModel();
////开启数据加载显示在单独线程
//m_DBStandardItemModel->moveToThread(&m_thread);
//connect(this, &AngKChipSelectWidget::startRunning, m_DBStandardItemModel, &AngKDBStandardItemModel::CreateData);
//connect(m_DBStandardItemModel, &AngKDBStandardItemModel::sSendFinished, ui->manufactureTableView, &AngKTableView::ReceiverData);
//connect(m_DBStandardItemModel, &AngKDBStandardItemModel::sSendValueChanaged, ui->manufactureTableView, &AngKTableView::ReceiverChanged);
//connect(this, &AngKChipSelectWidget::sgnClose, &m_thread, &QThread::quit);
//m_thread.start();
