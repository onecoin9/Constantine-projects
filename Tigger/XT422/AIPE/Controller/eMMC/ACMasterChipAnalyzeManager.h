#pragma once

#include <QObject>
#include "pugixml.hpp"
#include "json.hpp"
#include "AngKLogManager.h"
#include "eMMCCom.h"
class ACMasterChipAnalyzeManager : public QObject
{
	Q_OBJECT

public:
	ACMasterChipAnalyzeManager(QObject *parent);
	~ACMasterChipAnalyzeManager();

	int ExecuteChipAnaylze(std::string devIP, uint32_t HopNum, uint32_t SKTEnable, uint32_t nPartition, uint32_t nDefaultValue, uint32_t nAnalyzeSize, uint32_t nAnalyzeGrain, uint64_t nSSDOffset);

	int ExecuteReadDataAndSaveToFile(std::string devIP, QString strFilePath, QString Type, uint32_t HopNum, uint64_t SrcAddr, uint64_t Length);

	void AppendNode(std::string nodeName, nlohmann::json& nodeInfoJson);

	void AppendNode(std::string nodeName, uchar* extCSDInfo);

	void SaveACXMLFile(QString& strFilePath);

	void GetExtCSDInfo(std::string devIP, uint32_t HopNum, uint32_t uExtSize, QString& strDDROffset, uint32_t uCrc16, QString& extLog);

	void RecordExtCSDInfo(QString& extcsdInfo);

	void ResetDocument();

	static uint32_t __GetEnhancedUserAreaSize(UI_CFG_EXTCSD* pRawExtCSD);

	static uint32_t __GetUserAreaSize(UI_CFG_EXTCSD* pRawExtCSD);

	static uint32_t __GetGPPAreaSize(UI_CFG_EXTCSD* pRawExtCSD, int idx);

	static uint32_t __GetBootAreaSize(UI_CFG_EXTCSD* pRawExtCSD);

	static uint32_t __GetRPMBAreaSize(UI_CFG_EXTCSD* pRawExtCSD);

	uint32_t GetUserAreaSize() { return __GetUserAreaSize(&m_readExtCSD.modify_extcsd); }//单位MB

	uint32_t GetBootAreaSize() { return __GetBootAreaSize(&m_readExtCSD.modify_extcsd); }

	uint32_t GetGPPAreaSize(int idx) { return __GetGPPAreaSize(&m_readExtCSD.modify_extcsd, idx); }

	uint32_t GetRPMBAreaSize() { return __GetRPMBAreaSize(&m_readExtCSD.modify_extcsd); }

signals:
	void sgnSendWriteProgressValue(std::string, int, int);

private:
	pugi::xml_document m_pDocument;
	eMMCOPTION_Modify	m_readExtCSD;
};
