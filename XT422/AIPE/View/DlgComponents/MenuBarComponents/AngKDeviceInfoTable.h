#pragma once

#include "AngKDialog.h"
namespace Ui { class AngKDeviceInfoTable; };

enum DeviceTableType
{
	IPHop,
	DevSiteSN,
	DevSiteAlias,
	FirmwareVersion,
	HardwareVersion,
	MUAPPVersion,
	FPGAVersion,
	FPGALocation
};

class QStandardItemModel;
class AngKDeviceInfoTable : public AngKDialog
{
	Q_OBJECT

public:
	AngKDeviceInfoTable(QWidget *parent = Q_NULLPTR);
	~AngKDeviceInfoTable();

	void InitDevTable();

	void InitData();

private:
	Ui::AngKDeviceInfoTable* ui;
	QStandardItemModel* m_DeviceTableModel;
};
