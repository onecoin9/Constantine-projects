#pragma once

#include "AngKDialog.h"
#include "DeviceModel.h"
namespace Ui { class AngKChainSelectDlg; };

class QStandardItemModel;
class AngKChainSelectDlg : public AngKDialog
{
	Q_OBJECT

public:
	AngKChainSelectDlg(QWidget *parent = Q_NULLPTR);
	~AngKChainSelectDlg();

	void InitTable();

	void InitButton();

	void SetChainInfo(DeviceStu& devInfo);

signals:
	void sgnSelectChain(std::string, std::vector<int>);

public slots:
	void onSlotSelectChain();
private:
	Ui::AngKChainSelectDlg *ui;
	QStandardItemModel* m_pChainModel;
	std::string	m_strIP;
};