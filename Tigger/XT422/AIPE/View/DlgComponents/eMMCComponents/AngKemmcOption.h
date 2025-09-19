#pragma once

#include <QWidget>
#include <QSortFilterProxyModel>
#include "GlobalDefine.h"
#include "eMMCCom.h"
#include "DataJsonSerial.hpp"
#include "ACeMMCAnalyzeManager.h"

enum PartitionType
{
	None = 0,
	USER = 1,
	BOOT1,
	BOOT2,
	RPMB,
	GPP1,
	GPP2,
	GPP3,
	GPP4
};

enum ImageFileType
{
	BinFiles = 0,
	MTK,
	Ambarella,
	Phison,
	CIMG,
	Huawei,
	ACIMG
};

namespace Ui { class AngKemmcOption; };

class QCheckBox;
class QSettings;
class AngKProjDataset;
class QStandardItemModel;
class QSortFilterProxyModel;
class AngKemmcOption : public QWidget
{
	Q_OBJECT

public:
	AngKemmcOption(QWidget *parent = Q_NULLPTR);
	~AngKemmcOption();

	void InitText();

	void InitButton();

	void InitTable();

	void InitSetting();

	void InsertTable(eMMCFileInfo& efInfo);

	void setProjDataset(AngKProjDataset* _projData);

	void ModifyTable(eMMCFileInfo& efInfo);
	
	int ParserACXml(QString acFile, QString binFile);

	int ParserACXml(const QString& proj_path, std::vector<MMAKEFILE>& makeFiles, std::string& strImgType, std::string& IntelligentJson, uint64_t& nBytesum);

	int ParserExtCSD(QString acFile);

	int ParserACXmlChipID(QString acFile);

	void SaveCurProgCheckState(std::map<std::string, uint16_t> progCheck);

	int GetFileCount();

	void SetArchivesFile();

	bool CheckOverlapped(eMMCFileInfo& efInfo, int Idx);

	std::string GetExtendCSDJson();

	std::string GetOptionJson();

	void SetImageFileType(ImageFileType imageType);

	int StartIntelligentAnalyze(const QString& proj_path, ImageFileType _imgType, int _chkType, QString& strBytesum);

	void GetEMMCHeaderInfo(const QString& proj_path, eMMCTableHeader& tableHeader, std::string& tableJson, std::string& emmcOptonInfo);

	bool ParseXML(const QString& xmlPath);

	int CalOtherTypeImageFileChecksum(int _chkType, std::vector<MMAKEFILE>& _mmakeFileVec);
private:
	uint64_t hexStr2Int(QString strHex);

	PartitionType TranslatePartType(QString strName);

	QString TranslatePartIdx(PartitionType partIdx);

	void SwitchRegConfigJson(UI_CFG_EXTCSD cfgEXTCSD, nlohmann::json& regCfgJson);

	void SwitchPartitionSizeJson(PartitionSizeModify cfgEXTCSD, nlohmann::json& partSizeJson);

	bool isAllZeros(const QString& str);

	void GetPartitionSize(QString partName, QString partSize, QString UnitSize, eMMCOPTION_Modify& csdInfo);

	void GetExtCSDReg(QString regAddr, QString regValue, QString regName, eMMCOPTION_Modify& csdInfo);

	int DealhwACXML(std::vector<tHuaweiACFile> hwFileVec, eMMCOPTION_Modify csdInfo, std::map<std::string, std::string> byteMap);

	void GetMMAKEFile(std::vector<MMAKEFILE>& _vecMMAKEFile, ImageFileType _imgType);

signals:
	void sgnDealEXTCSDConfig(eMMCOPTION_Modify);

	void sgnSetCheckExtCSD(bool);

	void sgnParserACXML(QString);

	void sgnRestExtCSDInfo();

	void sgnAcxmlAnaUpdateProgress(int);

	void sgnCLoseProgressDialog();

	void sigACXMLChipIDUpdated(std::string acxmlChipID);
public slots:
	void onSlotAddFile();

	void onSlotModifyFile();

	void onSlotDeleteFile();

	void onSlotBatchAddFile();

	void onSlotExtendedCSDFile();

	void onSlotDealEXTCSDConfig(eMMCOPTION_Modify);
private:
	Ui::AngKemmcOption *ui;
	QStandardItemModel* m_fileTableModel;
	AngKProjDataset* m_projDataset;
	QSortFilterProxyModel* m_sortFilter;
	QVector<eMMCFileRecord> m_vecFileReocrd;
	eMMCOPTION_Modify	m_configExtCSD;
	QSettings* m_driverParaSettings;
	std::map<std::string, uint16_t> m_progCheck;
	ImageFileType	m_nImageFileType;
	std::unique_ptr<ACeMMCAnalyzeManager> m_pAnalyzeMgr;
};
