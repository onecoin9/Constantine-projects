#pragma once

#include <QObject>
#include "json.hpp"
#define EventBuilder ACEventBuild::eventBuilder()

class ACEventBuild : public QObject
{
	Q_OBJECT

public:
	ACEventBuild(QObject *parent = nullptr);
	~ACEventBuild();

	static ACEventBuild* eventBuilder();
	static std::string GetCurrentDate();

	static std::string GetSoftStatusChange(bool bStart);
	static std::string GetLogIn(std::string strName, std::string strID, std::string strRole);
	static std::string GetProgSettingChange(nlohmann::json jsonData);
	static std::string GetFWUpdate(nlohmann::json jsonData);
	static std::string GetDataTransfer(nlohmann::json jsonData);
	static std::string GetPTTransfer(nlohmann::json jsonData);
	static std::string GetProgScan(nlohmann::json jsonData);
	static std::string GetProgConnect(nlohmann::json jsonData);
	static std::string GetProgStatusChange(nlohmann::json jsonData);
	static std::string GetChipSelect(nlohmann::json jsonData);
	static std::string GetLoadFiles(nlohmann::json jsonData);
	static std::string GetChipConfigChange(nlohmann::json jsonData);
	static std::string GetChipOptionChange(nlohmann::json jsonData);
	static std::string GetDriverParaChange(nlohmann::json jsonData);
	static std::string GetSaveProject(nlohmann::json jsonData);
	static std::string GetLoadProject(nlohmann::json jsonData);
	static std::string GetSaveTask(nlohmann::json jsonData);
	static std::string GetLoadTask(nlohmann::json jsonData);
	static std::string GetSNConfig(nlohmann::json jsonData);
};
