#pragma once

#include <QtWidgets/QWidget>
#include <QStandardItemModel>
namespace Ui { class ACEthernetConfigWidget; };

struct EthernetInfo
{
	QString strMac;
	QString strMacName;
	QString strIPv4;
	QString strNetMask;
	QString strNetName;
};

class AngKScanManager;
class AngKDeviceModel;
class ACEthernetConfigWidget : public QWidget
{
	Q_OBJECT

	enum LinkMode
	{
		SINGLE = 1,
		SERIES = 2
	};

public:
	ACEthernetConfigWidget(QWidget *parent = Q_NULLPTR);
	~ACEthernetConfigWidget();

	void InitText();

	void InitTable();

	void InsertData(QStandardItemModel* model, QString sn, QString alias, QString strIP, QString strStatus, QString strHop = "", QString strLinkNum = "", bool testCheck = false);

	void InitButton();

	void InitScan();

	void CloseSocket();

	void CloseWidgetInfo();
private:
	bool CheckInsertORUpdate(QString strIP, int& row);

	/// <summary>
	/// 计算子网掩码长度
	/// </summary>
	/// <param name="strNetmask">子网掩码</param>
	/// <returns>子网掩码长度</returns>
	int CalNetMaskLength(QString strNetmask);

	void FormatConnectEvent();
signals:
	void sgnConnectFinish();

public slots:
	void onSlotScanLoaclNetWork();
	void onSlotConnectNet();
	void onSlotDisConnectNet();
	void onSlotNetWorkSelectAllCheck(int);
	void onSlotConnectSelectAllCheck(int);
	void onSlotUpdateDevIPScan(QString);
	void onSlotSelectChainDev(const QModelIndex&);
	void onSlotChangedIPScan(int);
	void onSlotSendProgScanEvent();
private:
	Ui::ACEthernetConfigWidget *ui;
	QStandardItemModel* m_netModel;
	QStandardItemModel* m_connectedModel;
	//AngKScanManager*	m_pScanManager;
	int					m_nDefaultNetLength;
};
