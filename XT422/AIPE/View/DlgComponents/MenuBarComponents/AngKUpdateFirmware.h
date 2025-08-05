#pragma once

#include "AngKDialog.h"
#include "AngKDeviceModel.h"
#include "MessageType.h"
namespace Ui { class AngKUpdateFirmware; };

class QStandardItemModel;
class AngKUpdateFirmware : public AngKDialog
{
	Q_OBJECT

public:
	

	AngKUpdateFirmware(QWidget *parent = Q_NULLPTR, int nHopNumUse = 0);
	~AngKUpdateFirmware();

	void InitText();

	void InitButton();

	void InitTable(int _UseHop);

	QStringList GetChainIDandHop(QString strChain);


	static int m_UpdateFwNum;

private:
	/// <summary>
	/// 检查更新状态
	/// </summary>
	/// <returns>还有处于更新状态的返回true，否则返回false</returns>
	bool CheckUpdating();

	/// <summary>
	/// 检查固件文件版本是否比当前文件版本小
	/// </summary>
	/// <returns>比当前版本小则返回true，否则返回false</returns>
	bool CheckFirmwareVersion(std::vector<int>& selectDevice, std::string& toVer);
signals:
	void sgnUpdateFirmwareFile(QString, int, QString, QString, QString);
	

public slots:
	void onSlotUpdateFirmware();

	void onSlotUpdateFPGAValue(std::string, int, int);

	void onSlotCheckClose();

	void onSlotUpdateFwStatus(std::string, int);

	void onSlotTimerRebootFinish();
private:
	Ui::AngKUpdateFirmware *ui;
	QStandardItemModel* m_ProgressTableModel;

};
