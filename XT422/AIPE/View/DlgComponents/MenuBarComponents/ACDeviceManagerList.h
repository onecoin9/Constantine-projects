#pragma once

#include "AngKDialog.h"
#include "qtpropertymanager.h"
#include "qtvariantproperty.h"
#include "qteditorfactory.h"
#include "DeviceModel.h"
namespace Ui { class ACDeviceManagerList; };

class QTreeWidgetItem;
class ACDeviceManagerList : public AngKDialog
{
	Q_OBJECT

public:
	ACDeviceManagerList(QWidget *parent = Q_NULLPTR);
	~ACDeviceManagerList();

	void InitDeviceTree();
	void InitDeviceProperty();

private:
	void UpdateDeviceProperty(DeviceStu devInfo);

public slots:
	void onSlotPropertyEdit(bool);

	void onSlotDeviceModify();

	void onSlotShowDevProperty(QTreeWidgetItem*, int);
private:
	Ui::ACDeviceManagerList *ui;
	std::unique_ptr<QtVariantPropertyManager> m_pVarManager;
	std::unique_ptr<QtVariantEditorFactory> m_pVarFactory;
	std::unique_ptr<QtStringPropertyManager> m_pStrManager;
	std::unique_ptr<QtLineEditFactory> m_pLineFactory;
	std::unique_ptr<QTreeWidgetItem> m_pRootItem;
	std::string	m_strBeforeIP;
	std::string	m_strBeforeMac;
	std::string	m_strBeforeAlias;
};
