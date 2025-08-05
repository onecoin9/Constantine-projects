#pragma once

#include "AngKDialog.h"
#include "eMMCCom.h"
#include "DataJsonSerial.hpp"
namespace Ui { class AngKExtendCSD; };

enum MyEnum
{
	WR_REL_SET = 0, 
	FW_CONFIG = 1, 
	BOOT_WP = 2,
	PARTITION_CFG = 3,
	BOOT_CONFIG_PROT = 4,
	BOOT_BUS_CONDITIONS = 5,
	RST_n_FUNCTION = 6, 
	BKOPS_EN = 7,
	PARTITION_SETTING_COMPLETED = 8,
	PARTITIONS_ATTRIBUTE = 9,
	ERASE_GROUP_DEF = 10,
	USER_WP = 11,
	EXT_PARTITIONS_ATTRIBUITE = 12,
	SEC_BAD_BLK_MGMNT = 13, 
	SECURE_REMOVAL_TYPE = 14,
	ENH_START_ADDR = 15,
	ENH_SIZE_MULT = 16
};

class QStandardItemModel;
class AngKExtendCSD : public QWidget
{
	Q_OBJECT

public:
	AngKExtendCSD(QWidget *parent = Q_NULLPTR);
	~AngKExtendCSD();

	void InitText();

	void InitTable();

	void InitButton();

	void InitCombox();

	void GetExtendCSD2Send(eMMCOPTION_Modify& getExtCfg);

	void SetHuaweiExtCSD(eMMCOPTION_Modify hwExtCSD);

	int ParserExtCSDXml(QString sfilePath, bool bACXML = false);

	void GetPartitionSize(QString partName, QString partSize, QString UnitSize, eMMCOPTION_Modify& csdInfo);

	void GetExtCSDReg(QString regAddr, QString regValue, QString regName, eMMCOPTION_Modify& csdInfo);

	void SwitchRegConfigJson(UI_CFG_EXTCSD cfgEXTCSD, nlohmann::json& regCfgJson);

	void SwitchPartitionSizeJson(PartitionSizeModify cfgEXTCSD, nlohmann::json& partSizeJson);

	bool isAllZeros(const QString& str);

	std::string GetExtendCSDJson();

	void SeteMMCExtCSDPara(std::string& extJson);

	std::string GetEXTCSDValue(UI_CFG_EXTCSD& _cfgEXTCSD, int nAddr);

	bool CheckGPPSetting();
signals:
	void sgnEXTCSDConfig(eMMCOPTION_Modify);

private:
	void InsertFixedData();
	void SwitchModifyValue(UI_CFG_EXTCSD& uiExtCSD, MyEnum csdType, QString valueStr);

public slots:
	void onSlotSaveExtendCSDConfig();

	void onSlotDealEXTCSDConfig(eMMCOPTION_Modify);

	void onSlotImportExtCSDConfig();

	void onSlotParserACXML(QString acFilePath);

	void onSlotRestExtCSDInfo();

	void onSlotExtCSDInfoFetched(QByteArray);
private:
	Ui::AngKExtendCSD *ui;
	QStandardItemModel* m_eCSDDataTableModel;
	eMMCOPTION_Modify	m_readExtCSD;
};
