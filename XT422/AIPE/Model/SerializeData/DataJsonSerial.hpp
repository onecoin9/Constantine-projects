#pragma once
#include "json.hpp"
#include "ChipModel.h"
#include "GlobalDefine.h"
#include "AngKCommonTools.h"
#include <string>
#include <QMetaType>

//引用第三方库 nlohmann::json，每个需要内部使用的json需要主动进行对象化，简化代码。格式为xxxJsonSerial
//ChipDataJsonSerial 是查询chip之后的芯片数据，进行json序列化，并将内容可以反序列化输出。后续添加对象可按照该类实现
#define DEFAULT_CHIPID "0"

class JsonSerial
{
public:
	JsonSerial() { m_jsonData.clear(); };
	~JsonSerial() { m_jsonData.clear(); };

	void copyJson(nlohmann::json _jsonData)
	{
		m_jsonData = _jsonData;
	}

	template <typename T>
	void setJsonValue(std::string key, T value)
	{
		m_jsonData[key] = value;
	};

	template <typename T>
	T getJsonValue(const std::string& key, const T& default_value = T{}) const
	{
		if (m_jsonData.is_null())
			return default_value;

		return m_jsonData.value(key, default_value);
	}

	virtual std::string json2String()
	{
		return m_jsonData.dump();
	};

	nlohmann::json DataJsonSerial()
	{
		return m_jsonData;
	};

	void ClearJson()
	{
		m_jsonData.clear();
	}

protected:
	nlohmann::json m_jsonData;
};

class ChipDataJsonSerial : public JsonSerial
{

public:
	ChipDataJsonSerial() {};
	~ChipDataJsonSerial() { ClearJson(); };

	std::string DecimalStringToHexString(const std::string& decimalStr) {
		QString qDecimalStr(decimalStr.c_str());

		bool ok;
		qlonglong decimalNumber = qDecimalStr.toLongLong(&ok, 10);

		if (!ok) {
			return "0"; 
		}

		QString qHexStr = QString::number(decimalNumber, 16).toUpper(); 

		return qHexStr.toStdString();
	}

	void serialize(chip info)
	{
		m_jsonData["chipName"]		  = info.strName;
		m_jsonData["manufacture"]	  = info.strManu;
		m_jsonData["bottomBoard"]	  = info.strBottomBoard;
		m_jsonData["chipAdapter"]	  = info.strAdapter;
		m_jsonData["chipAdapter2"]	  = info.strAdapter2;
		m_jsonData["chipAdapter3"]	  = info.strAdapter3;
		m_jsonData["chipPackage"]	  = info.strPack;
		m_jsonData["chipType"]		  = info.strType;
		m_jsonData["chipAlgoFile"]	  = info.strAlgoFile;
		m_jsonData["chipFPGAFile"]	  = info.strFPGAFile;
		m_jsonData["chipFPGAFile2"]	  = info.strFPGAFile2;
		m_jsonData["chipAppFile"]	  = info.strAppFile;
		m_jsonData["chipAdapterID"]	  = DecimalStringToHexString(info.strAdapterID);
		m_jsonData["chipMstkoFile"]	  = info.strMstkoFile;
		m_jsonData["chipBufferSize"]  = info.ulBufferSize;
		m_jsonData["chipChipInfo"]	  = info.strChipInfo;
		m_jsonData["bDebug"]		  = info.bDebug;
		m_jsonData["nVersion"]		  = info.nVersion;
		m_jsonData["chipModifyInfo"]  = info.strModifyInfo;
		m_jsonData["chipId"]		  = info.strChipId;
		m_jsonData["chipIdACXML"]	  = ""; //导入ACXML时ChipID的值
		m_jsonData["chipDrvParam"]	  = info.ulDrvParam;
		m_jsonData["chipSectorSize"]  = info.ulSectorSize;
		m_jsonData["chipHelpFile"]	  = info.strHelpFile;
		m_jsonData["chipProgType"]	  = info.nProgType;
		m_jsonData["chipCurSbk"]	  = info.strCurSbk;
		m_jsonData["chipSbkId"]		  = info.nSbkId;
		m_jsonData["chipStatus"]	  = "";
		m_jsonData["chipBufferSizeHigh"] = info.ulBufferSizeHigh;
		m_jsonData["chipOperateConfigMask"] = info.ulOperateConfigMask;
		m_jsonData["chipOperCfgJson"] = info.strOperCfgJson;
	}

	void deserialize(chip& info)
	{
		info.strName		= m_jsonData["ChipName"];
		info.strManu		= m_jsonData["Manufacture"];
		info.strBottomBoard = m_jsonData["BottomBoard"];
		info.strAdapter		= m_jsonData["ChipAdapter"];
		info.strAdapter2	= m_jsonData["ChipAdapter2"];
		info.strAdapter3	= m_jsonData["ChipAdapter3"];
		info.strPack		= m_jsonData["ChipPackage"];
		info.strType		= m_jsonData["ChipType"];
		info.strAlgoFile	= m_jsonData["chipAlgoFile"];
		info.strFPGAFile	= m_jsonData["chipFPGAFile"];
		info.strFPGAFile2	= m_jsonData["chipFPGAFile2"];
		info.strAppFile		= m_jsonData["chipAppFile"];
		info.strAdapterID	= m_jsonData["chipAdapterID"];
		info.strMstkoFile	= m_jsonData["chipMstkoFile"];
		info.ulBufferSize	= m_jsonData["chipBufferSize"];
		info.strChipInfo	= m_jsonData["chipChipInfo"];
		info.bDebug			= m_jsonData["bDebug"];
		info.nVersion		= m_jsonData["nVersion"];
		info.strModifyInfo	= m_jsonData["chipModifyInfo"];
		info.strChipId		= m_jsonData["chipId"];
		info.ulDrvParam		= m_jsonData["chipDrvParam"];
		info.ulSectorSize	= m_jsonData["chipSectorSize"];
		info.strHelpFile	= m_jsonData["chipHelpFile"];
		info.nProgType		= m_jsonData["chipProgType"];
		info.strCurSbk		= m_jsonData["chipCurSbk"];
		info.nSbkId			= m_jsonData["chipSbkId"];
		info.ulBufferSizeHigh = m_jsonData["chipBufferSizeHigh"];
		info.ulOperateConfigMask = m_jsonData["chipOperateConfigMask"];
		info.strOperCfgJson = m_jsonData["chipOperCfgJson"];
	}
};

class FileDataJsonSerial : public JsonSerial
{

public:
	FileDataJsonSerial() {};
	~FileDataJsonSerial() { ClearJson(); };

	void serialize(FileDataInfo info) 
	{
		m_jsonData["fileName"] = info.fileNameStr;
		m_jsonData["fileTag"] = info.fileTagStr;
		m_jsonData["fileSize"] = info.fileSizeStr;
		m_jsonData["filePath"] = info.filePathStr;
		m_jsonData["fileFormat"] = info.fileFormatStr;
		m_jsonData["fileCheck"] = info.fileCheckStr;
		m_jsonData["fileAutoDetecteFormat"] = info.fileAutoDetecteFormatStr;
		m_jsonData["fileWordAddressEnable"] = info.fileWordAddressEnable;
		m_jsonData["fileRelcationEnable"] = info.fileRelcationEnable;
		m_jsonData["fileBufferAddress"] = info.fileBufferAddressStr;
		m_jsonData["fileAddress"] = info.fileAddressStr;
		m_jsonData["fileLoadLength"] = info.fileLoadLengthStr;
		m_jsonData["fileSwapType"] = info.fileSwapType;
		m_jsonData["fileSwapTypeStr"] = info.fileSwapTypeStr;
		m_jsonData["fileCheckType"] = info.fileChecksumType;
		m_jsonData["fileCheckValue"] = info.fileChecksumValue;
		m_jsonData["fileBufAddrWriteMin"] = info.fileBufAddrWriteMin;
		m_jsonData["fileBufAddrWriteMax"] = info.fileBufAddrWriteMax;
	};

	void deserialize(FileDataInfo& info) 
	{
		info.fileNameStr = m_jsonData["fileName"];
		info.fileTagStr = m_jsonData["fileTag"];
		info.fileSizeStr = m_jsonData["fileSize"];
		info.filePathStr = m_jsonData["filePath"];
		info.fileFormatStr = m_jsonData["fileFormat"];
		info.fileCheckStr = m_jsonData["fileCheck"];
		info.fileAutoDetecteFormatStr = m_jsonData["fileAutoDetecteFormat"];
		info.fileWordAddressEnable = m_jsonData["fileWordAddressEnable"];
		info.fileRelcationEnable = m_jsonData["fileRelcationEnable"];
		info.fileBufferAddressStr = m_jsonData["fileBufferAddress"];
		info.fileAddressStr = m_jsonData["fileAddress"];
		info.fileLoadLengthStr = m_jsonData["fileLoadLength"];
		info.fileSwapType = m_jsonData["fileSwapType"].get<int>();
		info.fileSwapTypeStr = m_jsonData["fileSwapTypeStr"];
		info.fileChecksumType = m_jsonData["fileCheckType"];
		info.fileChecksumValue = m_jsonData["fileCheckValue"];
		info.fileBufAddrWriteMin = m_jsonData["fileBufAddrWriteMin"];
		info.fileBufAddrWriteMax = m_jsonData["fileBufAddrWriteMax"];
	}
};

class FileImportDataJsonSerial : public JsonSerial
{
public:
	FileImportDataJsonSerial() {};
	~FileImportDataJsonSerial() { ClearJson(); };

	void serialize(std::vector<FileDataJsonSerial> dataJson, int nVerison, int nClearBufId)
	{
		m_jsonData["version"] = nVerison;
		m_jsonData["clearBufferID"] = nClearBufId;
		m_jsonData["fileNum"] = dataJson.size();
		for (int i = 0; i < dataJson.size(); ++i)
		{
			m_jsonData["fileData"].push_back(dataJson[i].DataJsonSerial());
		}
	}

	void deserialize(std::vector<FileDataJsonSerial>& dataJson, int& nVerison, int& nClearBufId)
	{
		nVerison = m_jsonData["version"];
		nClearBufId = m_jsonData["clearBufferID"];
		for (int j = 0; j < m_jsonData["fileNum"]; ++j)
		{
			FileDataInfo info;
			info.fileNameStr = m_jsonData["fileData"]["fileName"];
			info.fileTagStr = m_jsonData["fileData"]["fileTag"];
			info.fileSizeStr = m_jsonData["fileData"]["fileSize"];
			info.filePathStr = m_jsonData["fileData"]["filePath"];
			info.fileFormatStr = m_jsonData["fileData"]["fileFormat"];
			info.fileCheckStr = m_jsonData["fileData"]["fileCheck"];
			info.fileAutoDetecteFormatStr = m_jsonData["fileData"]["fileAutoDetecteFormat"];
			info.fileWordAddressEnable = m_jsonData["fileData"]["fileWordAddressEnable"];
			info.fileRelcationEnable = m_jsonData["fileData"]["fileRelcationEnable"];
			info.fileBufferAddressStr = m_jsonData["fileData"]["fileBufferAddress"];
			info.fileAddressStr = m_jsonData["fileData"]["fileAddress"];
			info.fileLoadLengthStr = m_jsonData["fileData"]["fileLoadLength"];
			info.fileSwapType = m_jsonData["fileData"]["fileSwapType"];
			info.fileSwapTypeStr = m_jsonData["fileData"]["fileSwapTypeStr"];
			info.fileChecksumType = m_jsonData["fileData"]["fileCheckType"];
			info.fileChecksumValue = m_jsonData["fileData"]["fileCheckValue"];

			FileDataJsonSerial fDataJson;
			fDataJson.serialize(info);

			dataJson.push_back(fDataJson);
		}
	}
};

class eMMCFileDataJsonSerial : public JsonSerial
{

public:
	eMMCFileDataJsonSerial() {};
	~eMMCFileDataJsonSerial() { ClearJson(); };

	void serialize(eMMCFileInfo info)
	{
		m_jsonData["fileFullPath"] = info.sFilePath;
		m_jsonData["partitionName"] = info.sPartitionName;
		m_jsonData["startAddress"] = info.nStartAddr;
		m_jsonData["fileSize"] = info.nFileSize;
		m_jsonData["sectorAlign"] = info.bSectorAlign;
		m_jsonData["checkSum"] = info.nCheckSum;
	};

	void deserialize(eMMCFileInfo& info)
	{
		info.sFilePath = m_jsonData["fileFullPath"];
		info.sPartitionName = m_jsonData["partitionName"];
		info.nStartAddr = m_jsonData["startAddress"];
		info.nFileSize = m_jsonData["fileSize"];
		info.bSectorAlign = m_jsonData["sectorAlign"];
		info.nCheckSum = m_jsonData["checkSum"];
	}
};

class eMMCFileImportDataJsonSerial : public JsonSerial
{
public:
	eMMCFileImportDataJsonSerial() {};
	~eMMCFileImportDataJsonSerial() { ClearJson(); };

	void serialize(std::vector<eMMCFileDataJsonSerial> dataJson, std::string imageFileFormat, const QString& proj_path)
	{
		m_jsonData["imageFileFormat"] = imageFileFormat;
		m_jsonData["fileNum"] = dataJson.size();
		for (int i = 0; i < dataJson.size(); ++i)
		{
			auto&& file_path = dataJson[i].getJsonValue<std::string>("fileFullPath");
			dataJson[i].setJsonValue("fileFullPath", Utils::AngKCommonTools::Full2RelativePath(QString::fromStdString(file_path), proj_path));
			m_jsonData["eMMCFileData"].push_back(dataJson[i].DataJsonSerial());
		}
	}

	void deserialize(std::vector<eMMCFileDataJsonSerial>& dataJson, std::string& imageFileFormat, const QString& proj_path)
	{
		imageFileFormat = m_jsonData["imageFileFormat"];
		for (int j = 0; j < m_jsonData["fileNum"]; ++j)
		{
			nlohmann::json emmcFile = m_jsonData["eMMCFileData"][j];
			auto&& file_path = emmcFile["fileFullPath"].get<std::string>();
			emmcFile["fileFullPath"] = Utils::AngKCommonTools::Relative2FullPath(QString::fromStdString(file_path), proj_path);

			/*eMMCFileInfo info;
			info.sFilePath = emmcFile[j]["fileFullPath"];
			info.sPartitionName = emmcFile[j]["partitionName"];
			info.nStartAddr = emmcFile[j]["startAddress"];
			info.nFileSize = emmcFile[j]["fileSize"];
			info.bSectorAlign = emmcFile[j]["sectorAlign"];
			info.nCheckSum = emmcFile[j]["checkSum"];*/

			eMMCFileDataJsonSerial fDataJson;
			fDataJson.copyJson(emmcFile);
			//fDataJson.serialize(info);

			dataJson.push_back(fDataJson);
		}
	}
};

Q_DECLARE_METATYPE(ChipDataJsonSerial)
Q_DECLARE_METATYPE(FileDataJsonSerial)
Q_DECLARE_METATYPE(FileImportDataJsonSerial)
Q_DECLARE_METATYPE(eMMCFileDataJsonSerial)
Q_DECLARE_METATYPE(eMMCFileImportDataJsonSerial)