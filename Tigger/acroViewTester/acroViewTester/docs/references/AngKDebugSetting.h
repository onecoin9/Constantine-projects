#pragma once

#include "AngKDialog.h"
namespace Ui { class AngKDebugSetting; };

class QComboBox;
class QStandardItemModel;
class AngKDebugSetting : public AngKDialog
{
	Q_OBJECT

public:
	AngKDebugSetting(QMap<QString, int>* uartCbIdxMap, QMap<QString, int>* logCbIdxMap, QWidget *parent = Q_NULLPTR);
	~AngKDebugSetting();

	void InitTable();

	void InitDevice();

	void InitUARTCombobox(QComboBox* combox, const std::string& ip, int hop);

	void InitLogLevelCombobox(QComboBox* combox, const std::string& ip, int hop);
public slots:
	void onTableRebootBtnClicked();

	void onTableSwitchUART(int);

	void onTableSwitchLogLevel(int);
private:
	Ui::AngKDebugSetting *ui;
	QStandardItemModel* m_pDebugProgTableModel;
	QMap<QString, int>* m_pUartComboboxIndexMap;
	QMap<QString, int>* m_pLogLevelComboboxIndexMap;
};
