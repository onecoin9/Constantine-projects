#include "ACEventBuild.h"
#include "AngKGlobalInstance.h"

ACEventBuild::ACEventBuild(QObject *parent)
	: QObject(parent)
{
}

ACEventBuild::~ACEventBuild()
{
}

ACEventBuild* ACEventBuild::eventBuilder()
{
	static ACEventBuild eventBuilder;
	return &eventBuilder;
}

std::string ACEventBuild::GetCurrentDate()
{
	QString strTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
	return strTime.toStdString();
}

std::string ACEventBuild::GetSoftStatusChange(bool bStart)
{
	nlohmann::json SoftStatusChangeEvent;

	SoftStatusChangeEvent["ESender"] = "AprogPlus";
	SoftStatusChangeEvent["ETime"] = GetCurrentDate();
	SoftStatusChangeEvent["EName"] = "SoftStatusChange";

	nlohmann::json DataEvent;
	DataEvent["Status"] = bStart ? "Start" : "Stop";

	if (bStart) {
		nlohmann::json infoData;
		infoData["BuildVersion"] = AngKGlobalInstance::ReadValue("Version", "BuildVer").toString().toStdString();
		infoData["RetInfo"] = "SortStart Sucess";

		DataEvent["Info"] = infoData;
	}
	SoftStatusChangeEvent["EData"] = DataEvent;

	return SoftStatusChangeEvent.dump();
}

std::string ACEventBuild::GetLogIn(std::string strName, std::string strID, std::string strRole)
{
	nlohmann::json LogInEvent;

	LogInEvent["ESender"] = "AprogPlus";
	LogInEvent["ETime"] = GetCurrentDate();
	LogInEvent["EName"] = "LogIn";

	nlohmann::json DataEvent;

	DataEvent["Name"] = strName;
	DataEvent["ID"] = strID;
	DataEvent["Role"] = strRole;
	LogInEvent["EData"] = DataEvent;

	return LogInEvent.dump();
}

std::string ACEventBuild::GetProgSettingChange(nlohmann::json jsonData)
{
	return std::string();
}

std::string ACEventBuild::GetFWUpdate(nlohmann::json jsonData)
{
	nlohmann::json FWUpdateEvent;

	FWUpdateEvent["ESender"] = "AprogPlus";
	FWUpdateEvent["ETime"] = GetCurrentDate();
	FWUpdateEvent["EName"] = "FWUpdate";
	FWUpdateEvent["EData"] = jsonData;

	return FWUpdateEvent.dump();
}

std::string ACEventBuild::GetDataTransfer(nlohmann::json jsonData)
{
	nlohmann::json DataTransferEvent;

	DataTransferEvent["ESender"] = "AprogPlus";
	DataTransferEvent["ETime"] = GetCurrentDate();
	DataTransferEvent["EName"] = "DataTransfer";
	DataTransferEvent["EData"] = jsonData;

	return DataTransferEvent.dump();
}

std::string ACEventBuild::GetPTTransfer(nlohmann::json jsonData)
{
	nlohmann::json PTTransferEvent;

	PTTransferEvent["ESender"] = "AprogPlus";
	PTTransferEvent["ETime"] = GetCurrentDate();
	PTTransferEvent["EName"] = "PTTransfer";
	PTTransferEvent["EData"] = jsonData;

	return PTTransferEvent.dump();
}

std::string ACEventBuild::GetProgScan(nlohmann::json jsonData)
{
	nlohmann::json ProgScanEvent;

	ProgScanEvent["ESender"] = "AprogPlus";
	ProgScanEvent["ETime"] = GetCurrentDate();
	ProgScanEvent["EName"] = "ProgScan";
	ProgScanEvent["EData"] = jsonData;

	return ProgScanEvent.dump();
}

std::string ACEventBuild::GetProgConnect(nlohmann::json jsonData)
{
	nlohmann::json ProgConnectEvent;

	ProgConnectEvent["ESender"] = "AprogPlus";
	ProgConnectEvent["ETime"] = GetCurrentDate();
	ProgConnectEvent["EName"] = "ProgConnect";
	ProgConnectEvent["EData"] = jsonData;

	return ProgConnectEvent.dump();
}

std::string ACEventBuild::GetProgStatusChange(nlohmann::json jsonData)
{
	nlohmann::json ProgStatusChangeEvent;

	ProgStatusChangeEvent["ESender"] = "AprogPlus";
	ProgStatusChangeEvent["ETime"] = GetCurrentDate();
	ProgStatusChangeEvent["EName"] = "ProgStatusChange";
	ProgStatusChangeEvent["EData"] = jsonData;

	return ProgStatusChangeEvent.dump();
}

std::string ACEventBuild::GetChipSelect(nlohmann::json jsonData)
{
	nlohmann::json ChipSelectEvent;

	ChipSelectEvent["ESender"] = "AprogPlus";
	ChipSelectEvent["ETime"] = GetCurrentDate();
	ChipSelectEvent["EName"] = "ChipSelect";
	ChipSelectEvent["EData"] = jsonData;

	return ChipSelectEvent.dump();
}

std::string ACEventBuild::GetLoadFiles(nlohmann::json jsonData)
{
	nlohmann::json LoadFilesEvent;

	LoadFilesEvent["ESender"] = "AprogPlus";
	LoadFilesEvent["ETime"] = GetCurrentDate();
	LoadFilesEvent["EName"] = "LoadFiles";
	LoadFilesEvent["EData"] = jsonData;

	return LoadFilesEvent.dump();
}

std::string ACEventBuild::GetChipConfigChange(nlohmann::json jsonData)
{
	nlohmann::json ChipConfigChangeEvent;

	ChipConfigChangeEvent["ESender"] = "AprogPlus";
	ChipConfigChangeEvent["ETime"] = GetCurrentDate();
	ChipConfigChangeEvent["EName"] = "ChipConfigChange";
	ChipConfigChangeEvent["EData"] = jsonData;

	return ChipConfigChangeEvent.dump();
}

std::string ACEventBuild::GetChipOptionChange(nlohmann::json jsonData)
{
	nlohmann::json ChipOptionChangeEvent;

	ChipOptionChangeEvent["ESender"] = "AprogPlus";
	ChipOptionChangeEvent["ETime"] = GetCurrentDate();
	ChipOptionChangeEvent["EName"] = "ChipOptionChange";
	ChipOptionChangeEvent["EData"] = jsonData;

	return ChipOptionChangeEvent.dump();
}

std::string ACEventBuild::GetDriverParaChange(nlohmann::json jsonData)
{
	nlohmann::json DriverParaChangeEvent;

	DriverParaChangeEvent["ESender"] = "AprogPlus";
	DriverParaChangeEvent["ETime"] = GetCurrentDate();
	DriverParaChangeEvent["EName"] = "DriverParaChange";
	DriverParaChangeEvent["EData"] = jsonData;

	return DriverParaChangeEvent.dump();
}

std::string ACEventBuild::GetSaveProject(nlohmann::json jsonData)
{
	nlohmann::json SaveProjectEvent;

	SaveProjectEvent["ESender"] = "AprogPlus";
	SaveProjectEvent["ETime"] = GetCurrentDate();
	SaveProjectEvent["EName"] = "SaveProject";
	SaveProjectEvent["EData"] = jsonData;

	return SaveProjectEvent.dump();
}

std::string ACEventBuild::GetLoadProject(nlohmann::json jsonData)
{
	nlohmann::json LoadProjectEvent;

	LoadProjectEvent["ESender"] = "AprogPlus";
	LoadProjectEvent["ETime"] = GetCurrentDate();
	LoadProjectEvent["EName"] = "LoadProject";
	LoadProjectEvent["EData"] = jsonData;

	return LoadProjectEvent.dump();
}

std::string ACEventBuild::GetSaveTask(nlohmann::json jsonData)
{
	nlohmann::json SaveTaskEvent;

	SaveTaskEvent["ESender"] = "AprogPlus";
	SaveTaskEvent["ETime"] = GetCurrentDate();
	SaveTaskEvent["EName"] = "SaveTask";
	SaveTaskEvent["EData"] = jsonData;

	return SaveTaskEvent.dump();
}

std::string ACEventBuild::GetLoadTask(nlohmann::json jsonData)
{
	nlohmann::json LoadTaskEvent;

	LoadTaskEvent["ESender"] = "AprogPlus";
	LoadTaskEvent["ETime"] = GetCurrentDate();
	LoadTaskEvent["EName"] = "LoadTask";
	LoadTaskEvent["EData"] = jsonData;

	return LoadTaskEvent.dump();
}

std::string ACEventBuild::GetSNConfig(nlohmann::json jsonData)
{
	nlohmann::json SNConfigEvent;

	SNConfigEvent["ESender"] = "AprogPlus";
	SNConfigEvent["ETime"] = GetCurrentDate();
	SNConfigEvent["EName"] = "SNConfig";
	SNConfigEvent["EData"] = jsonData;

	return SNConfigEvent.dump();
}
