#pragma once
#include <string>
#include <vector>

struct stuProjProperty
{
	std::string name;
	std::string projUUID;
	std::string chipName;
	std::string adapterName;
	std::vector<std::string> binFile;
	std::string typeName;
	std::string manufacturerName;
	std::string packageName;
	std::string checksum;
	std::vector<int> vecUseSite;
	std::string chipOperJson;
	std::string chipBufChkJson;
	std::string projDescription;
	std::string projVersion;
	std::string devDriverVer;
	std::string mstDriverVer;


	void Clear() {
		name = "";
		projUUID = "";
		chipName = "";
		adapterName = "";
		binFile.clear();
		typeName = "";
		manufacturerName = "";
		packageName = "";
		checksum = "";
		vecUseSite.clear();
		chipOperJson = "";
		chipBufChkJson = "";
		projDescription = "";
		devDriverVer = "";
		mstDriverVer = "";
	}
};

struct stuProgProperty
{
	std::string name;
	std::string snName;
	std::string commMode;
	std::string vision;
	std::string ipAddr;
	std::string ipPort;
	std::string screenProvider;
	uint32_t screenBatchID;
	std::string powerProvider;
	std::string powerBatchID;
};