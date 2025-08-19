#include "AngKCommonTools.h"
#include "AngKPathResolve.h"
#include "qzipreader_p.h"
#include "qzipwriter_p.h"
#include "ICD.h"
#include "ACCmdPacket.h"
//#include "AngkLogger.h"
#include "ACReportRecord.h"
#include "ACDeviceManager.h"
#include <sstream>
#include <QBuffer>
#include <QDebug>
#include <QDir>
#include <QFileInfoList>
#include <QRandomGenerator>
#include <QJsonArray>
#include <regex>

namespace Utils
{

	QString GetDrvVersion(const QString& drvPath, const QString& verName) {
		QFile binFile(drvPath);
		if (binFile.open(QIODevice::ReadOnly)) {
			qDebug() << "QFile opened successfully.";
		}
		QZipReader zipReader(&binFile);
		if (zipReader.status() != QZipReader::NoError) {
			qDebug() << "Failed to open ZIP file:" << zipReader.status();
			return "";
		}
		//QVector<QZipReader::FileInfo> file = zipReader.fileInfoList();
		QString ret = "";
		for (const auto& fileInfo : zipReader.fileInfoList())
		{
			QFile file(Utils::AngKPathResolve::localTempFolderPath() + fileInfo.filePath);
			if (fileInfo.filePath.contains("Version.txt") && file.open(QIODevice::WriteOnly))
			{
				QByteArray array = zipReader.fileData(fileInfo.filePath);
				QByteArrayList tmpList = array.split('\n');

				for (auto tmpLine : tmpList) {
					if (tmpLine.indexOf(":") < 0) {
						continue;
					}
					if (tmpLine.split(':')[0].trimmed() == verName) {
						ret = tmpLine.split(':')[1].trimmed();
					}
				}

			}
		}
		zipReader.close();
		return ret;
	}

	void AngKCommonTools::DepressBinFile(std::string& binZip, QStringList& pathList)
	{
		QFile binFile(QString::fromStdString(binZip));
		if (binFile.open(QIODevice::ReadOnly)) {
			qDebug() << "QFile opened successfully.";
		}
		QZipReader zipReader(&binFile);
		if (zipReader.status() != QZipReader::NoError) {
			qDebug() << "Failed to open ZIP file:" << zipReader.status();
			return;
		}
		//QVector<QZipReader::FileInfo> file = zipReader.fileInfoList();
		for (const auto& fileInfo : zipReader.fileInfoList())
		{
			QFile file(Utils::AngKPathResolve::localTempFolderPath() + fileInfo.filePath);
			if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
			{
				QByteArray array = zipReader.fileData(fileInfo.filePath);
				file.write(array);
				file.close();

				pathList.push_back(Utils::AngKPathResolve::localTempFolderPath() + fileInfo.filePath);
			}
		}
		zipReader.close();
	}

	void AngKCommonTools::SwitchFileFormat(std::string origianlFile, std::string newExtension, std::string& newFile)
	{
		// 找到原始文件名中的点的位置
		size_t dotPosition = origianlFile.find_last_of('.');

		// 如果找到点，则将其替换为新的扩展名
		if (dotPosition != std::string::npos) {
			std::string baseName = origianlFile.substr(0, dotPosition);
			newFile = baseName + newExtension;
		}
	}

	int AngKCommonXMLParser::ParserChipConfigXML(QString cfgFile, QString& dataJson)
	{
		pugi::xml_document doc;
		const wchar_t* encodedName = reinterpret_cast<const wchar_t*>(cfgFile.utf16());
		pugi::xml_parse_result result = doc.load_file(encodedName);
		nlohmann::json xmlJson;
		if (!result)
			return XMLMESSAGE_LOAD_FAILED;

		pugi::xml_node root_node = doc.child(XML_NODE_CHIPCONFIG);

		int childNodeCount = std::distance(root_node.children().begin(), root_node.children().end());

		for (pugi::xml_node partNode = root_node; partNode; partNode = partNode.next_sibling())
		{
			if (partNode.name() == XML_NODE_CHIPDATA_DEVICE)
			{
				nlohmann::json deviceJson;


				xmlJson[partNode.name()] = deviceJson;
			}
		}

		return XMLMESSAGE_SUCCESS;
	}

	int AngKCommonXMLParser::ParserChipDataXML(uint16_t nType, unsigned long ulAlgo, QString dataFile, QString& dataJson, bool bAddDesc)
	{
		pugi::xml_document doc;
		const wchar_t* encodedName = reinterpret_cast<const wchar_t*>(dataFile.utf16());
		pugi::xml_parse_result result = doc.load_file(encodedName);
		pugi::xml_node root_node = doc.child(XML_NODE_CHIPDATA);
		nlohmann::json cmdJson;
		if (!result)
			return XMLMESSAGE_LOAD_FAILED;

		switch ((eSubCmdID)nType)
		{
		case eSubCmdID::SubCmd_MU_SetDriverCommon:
		{
			DriverCommonPara_Json(root_node, ulAlgo, cmdJson, bAddDesc);
		}
			break;
		case eSubCmdID::SubCmd_MU_SetDriverSelfPara:
		{
			DriverSelfPara_Json(root_node, ulAlgo, cmdJson, bAddDesc);
		}
			break;
		case eSubCmdID::SubCmd_MU_SetDriverPinMap:
		{
			DriverPinMap_Json(root_node, ulAlgo, cmdJson);
		}
			break;
		case eSubCmdID::SubCmd_MU_SetDriverPartitionTable:
		{
			PartitionBlock_Json(root_node, ulAlgo, cmdJson);
		}
			break;
		case eSubCmdID::SubCmd_MU_DoCmdSequence:
		{

		}
			break;
		case eSubCmdID::SubCmd_MU_SetBufferMapInfo:
		{

		}
			break;
		case eSubCmdID::SubCmd_MU_RebootBPU:
		{

		}
			break;
		default:
			break;
		}

		if (!cmdJson.is_null())
			dataJson = QString::fromStdString(cmdJson.dump());

		return XMLMESSAGE_SUCCESS;
	}

	void AngKCommonXMLParser::DriverCommonPara_Json(pugi::xml_node& rootNode, unsigned long ulAlgo, nlohmann::json& outJson, bool bAddDesc)
	{
		pugi::xml_node drvCommonSet = rootNode.child(XML_NODE_CHIPDATA_DRVCOMMONPARASET);
		pugi::xml_node drvCommonTable = drvCommonSet.child(XML_NODE_CHIPDATA_DRVPARATABLE);
		int drvCommonCount = std::distance(drvCommonSet.begin(), drvCommonSet.end());
		for (int i = 0; i < drvCommonCount; ++i)
		{
			if (CheckAlgoRange(ulAlgo, drvCommonTable.attribute("Algo").as_string()))
				break;

			drvCommonTable = drvCommonTable.next_sibling();
		}

		if (drvCommonTable == nullptr)
			return;

		pugi::xml_node structNode = drvCommonTable.child(XML_NODE_CHIPDATA_STRUCT);
		int structCount = std::distance(drvCommonTable.begin(), drvCommonTable.end());

		outJson["DrvCommonParaGroupCnt"] = structCount;
		outJson["DrvCommonParas"] = nlohmann::json::array();

		for (int i = 0; i < structCount; ++i) {
			pugi::xml_node propertyNode = structNode.child(XML_NODE_CHIPDATA_PROPERTY);
			int propertyCount = std::distance(structNode.begin(), structNode.end());

			nlohmann::json structJson;
			structJson["Struct"] = structNode.attribute("Name").as_string();
			structJson["MembersGroupCnt"] = propertyCount;

			structJson["Members"] = nlohmann::json::array();
			for (int proIdx = 0; proIdx < propertyCount; ++proIdx) {
				nlohmann::json membersJson;

				membersJson["Name"] = propertyNode.attribute("Name").as_string();
				membersJson["Unit"] = propertyNode.attribute("Unit").as_string();
				membersJson["Value"] = propertyNode.text().as_string();
				if (!propertyNode.attribute("Range").empty()) {
					QString rangeStr = propertyNode.attribute("Range").as_string();
					QStringList rangeList = rangeStr.split("-");
					if (rangeList.count() > 1) {
						membersJson["Min"] = rangeList[0].toStdString();
						membersJson["Max"] = rangeList[1].toStdString();
					}
				}

				if(bAddDesc)
					membersJson["Description"] = propertyNode.attribute("Description").as_string();

				structJson["Members"].push_back(membersJson);
				propertyNode = propertyNode.next_sibling();
			}

			outJson["DrvCommonParas"].push_back(structJson);
			structNode = structNode.next_sibling();
		}

		
	}

	void AngKCommonXMLParser::DriverSelfPara_Json(pugi::xml_node& rootNode, unsigned long ulAlgo, nlohmann::json& outJson, bool bAddDesc)
	{
		pugi::xml_node drvSelfSet = rootNode.child(XML_NODE_CHIPDATA_DRVSELFPARASET);
		pugi::xml_node drvSelfTable = drvSelfSet.child(XML_NODE_CHIPDATA_DRVPARATABLE);
		int drvSelfTableCount = std::distance(drvSelfSet.begin(), drvSelfSet.end());
		for (int i = 0; i < drvSelfTableCount; ++i)
		{
			if (CheckAlgoRange(ulAlgo, drvSelfTable.attribute("Algo").as_string()))
				break;

			drvSelfTable = drvSelfTable.next_sibling();
		}

		if (drvSelfTable == nullptr)
			return;


		pugi::xml_node paramStruct = drvSelfTable.child(XML_NODE_CHIPDATA_STRUCT);
		int structCount = std::distance(drvSelfTable.begin(), drvSelfTable.end());
		outJson["DrvSelfParaGroupCnt"] = structCount;
		outJson["DrvSelfParas"] = nlohmann::json::array();

		for (int i = 0; i < structCount; ++i)
		{
			pugi::xml_node propertyNode = paramStruct.child(XML_NODE_CHIPDATA_PROPERTY);
			int paramStructCount = std::distance(paramStruct.begin(), paramStruct.end());

			nlohmann::json structJson;
			structJson["Struct"] = paramStruct.attribute("Name").as_string();
			structJson["MembersGroupCnt"] = paramStructCount;
			structJson["Members"] = nlohmann::json::array();


			for (int proIdx = 0; proIdx < paramStructCount; ++proIdx)
			{
				nlohmann::json membersJson;

				membersJson["Name"] = propertyNode.attribute("Name").as_string();
				membersJson["Unit"] = propertyNode.attribute("Unit").as_string();
				membersJson["Value"] = propertyNode.text().as_string();
				if (!propertyNode.attribute("Range").empty()) {
					QString rangeStr = propertyNode.attribute("Range").as_string();
					QStringList rangeList = rangeStr.split("-");
					if (rangeList.count() > 1) {
						membersJson["Min"] = rangeList[0].toStdString();
						membersJson["Max"] = rangeList[1].toStdString();
					}
				}

				if (bAddDesc)
					membersJson["Description"] = propertyNode.attribute("Description").as_string();

				structJson["Members"].push_back(membersJson);
				propertyNode = propertyNode.next_sibling();
			}

			outJson["DrvSelfParas"].push_back(structJson);
			paramStruct = paramStruct.next_sibling();
		}
	}

	void AngKCommonXMLParser::DriverPinMap_Json(pugi::xml_node& rootNode, unsigned long ulAlgo, nlohmann::json& outJson)
	{
		//pugi::xml_node pinMapSet = rootNode.child(XML_NODE_CHIPDATA_PINMAPSET);
		pugi::xml_node pinMapTable = rootNode.child(XML_NODE_CHIPDATA_PINMAPTABLE);
		int pinMapTableCount = std::distance(pinMapTable.begin(), pinMapTable.end());
		for (int i = 0; i < pinMapTableCount; ++i)
		{
			if (CheckAlgoRange(ulAlgo, pinMapTable.attribute("Algo").as_string()))
				break;

			pinMapTable = pinMapTable.next_sibling();
		}

		if (pinMapTable == nullptr)
			return;

		outJson["PinMapsGroupCnt"] = pinMapTableCount;
		outJson["PinMaps"] = nlohmann::json::array();

		pugi::xml_node pinNode = pinMapTable.child(XML_NODE_CHIPDATA_PIN);
		for (int i = 0; i < pinMapTableCount; ++i)
		{
			int pinNodeCount = std::distance(pinNode.begin(), pinNode.end());

			nlohmann::json pinNodeJson;
			pinNodeJson["PinType"] = pinNode.attribute("Type").as_string();
			pinNodeJson["PinGroupCnt"] = pinNodeCount;
			pinNodeJson["PinGroup"] = nlohmann::json::array();

			pugi::xml_node pinGroup = pinNode.child(XML_NODE_CHIPDATA_GROUP);
			for (int grpIdx = 0; grpIdx < pinNodeCount; ++grpIdx)
			{
				nlohmann::json pinGroupJson;
				pinGroupJson["Name"] = pinGroup.attribute("Name").as_int();
				pinGroupJson["Pin"] = pinGroup.text().as_string();

				pinNodeJson["PinGroup"].push_back(pinGroupJson);
				pinGroup = pinGroup.next_sibling();
			}

			outJson["PinMaps"].push_back(pinNodeJson);
			pinNode = pinNode.next_sibling();
		}
	}

	void AngKCommonXMLParser::PartitionBlock_Json(pugi::xml_node& rootNode, unsigned long ulAlgo, nlohmann::json& outJson)
	{
		//pugi::xml_node blocksSet = rootNode.child(XML_NODE_CHIPDATA_BLOCKSET);
		pugi::xml_node blocksTable = rootNode.child(XML_NODE_CHIPDATA_BLOCKTABLE);
		int blocksTableCount = std::distance(blocksTable.begin(), blocksTable.end());
		for (int i = 0; i < blocksTableCount; ++i)
		{
			if (CheckAlgoRange(ulAlgo, blocksTable.attribute("Algo").as_string()))
				break;

			blocksTable = blocksTable.next_sibling();
		}

		if (blocksTable == nullptr)
			return;


		outJson["PartitionGroupCnt"] = blocksTableCount;
		outJson["Partitions"] = nlohmann::json::array();

		pugi::xml_node blockPartition = blocksTable.child(XML_NODE_CHIPDATA_BLOCKPARTITION);
		for (int i = 0; i < blocksTableCount; ++i)
		{
			int blockPartitionCount = std::distance(blockPartition.begin(), blockPartition.end());

			nlohmann::json partitionJson;
			partitionJson["Name"] = blockPartition.attribute("Name").as_string();
			partitionJson["Index"] = i + 1;
			partitionJson["DevWidth"] = blockPartition.attribute("DevWidth").as_string();
			partitionJson["AddrRange"] = blockPartition.attribute("AddrRange").as_string();
			partitionJson["Default"] = blockPartition.attribute("Default").as_string();

			partitionJson["BlocksGroupCnt"] = blockPartitionCount;
			partitionJson["Blocks"] = nlohmann::json::array();

			pugi::xml_node blockNode = blockPartition.child(XML_NODE_CHIPDATA_BLOCK);
			for (int blockIdx = 0; blockIdx < blockPartitionCount; ++blockIdx)
			{
				nlohmann::json blockJson;
				blockJson["Name"] = blockNode.attribute("Name").as_string();
				blockJson["AddrRange"] = blockNode.attribute("AddrRange").as_string();
				blockJson["OrgValue"] = blockNode.attribute("OrgValue").as_string();
				blockJson["SumMask"] = blockNode.attribute("SumMask").as_string();

				partitionJson["Blocks"].push_back(blockJson);
				blockNode = blockNode.next_sibling();
			}

			outJson["Partitions"].push_back(partitionJson);
			blockPartition = blockPartition.next_sibling();
		}
	}

	void AngKCommonXMLParser::BufferMapInfo_Json(pugi::xml_node& rootNode, unsigned long ulAlgo, nlohmann::json& outJson)
	{
		pugi::xml_node bufferMapTable = rootNode.child(XML_NODE_CHIPDATA_BUFFERMAPTABLE);
		int bufferMapTableCount = std::distance(bufferMapTable.begin(), bufferMapTable.end());
		for (int i = 0; i < bufferMapTableCount; ++i)
		{
			if (CheckAlgoRange(ulAlgo, bufferMapTable.attribute("Algo").as_string()))
				break;

			bufferMapTable = bufferMapTable.next_sibling();
		}

		if (bufferMapTable == nullptr)
			return;

		outJson["PartitionGroupCnt"] = bufferMapTableCount;
		outJson["Partitions"] = nlohmann::json::array();

		pugi::xml_node bufferPartition = bufferMapTable.child(XML_NODE_CHIPDATA_BUFFERPARTITION);
		for (int i = 0; i < bufferMapTableCount; ++i)
		{
			//if (bufferPartition.attribute("Visible").as_string() == "FALSE") {
			//	bufferPartition = bufferPartition.next_sibling();
			//	continue;
			//}

			nlohmann::json partitionJson;
			partitionJson["Name"] = bufferPartition.attribute("Name").as_string();
			partitionJson["Index"] = bufferPartition.attribute("Index").as_string();
			partitionJson["AddrRange"] = bufferPartition.attribute("AddrRange").as_string();
			partitionJson["Visiable"] = bufferPartition.attribute("Visiable").as_string();

			pugi::xml_node viewNode = bufferPartition.child(XML_NODE_CHIPDATA_VIEW);
			int ViewsCount = std::distance(bufferPartition.begin(), bufferPartition.end());
			partitionJson["ViewsCnt"] = ViewsCount;
			partitionJson["ViewsInfo"] = nlohmann::json::array();
			for (int blockIdx = 0; blockIdx < ViewsCount; ++blockIdx)
			{
				nlohmann::json viewJson;

				viewJson["Name"] = viewNode.attribute("Name").as_string();
				viewJson["Index"] = viewNode.attribute("Index").as_string();
				viewJson["AddrRange"] = viewNode.attribute("AddrRange").as_string();
				viewJson["OrgValue"] = viewNode.attribute("OrgValue").as_string();
				viewJson["Visiable"] = viewNode.attribute("Visiable").as_string();
				viewJson["ShowStyle"] = viewNode.attribute("ShowStyle").as_string();

				partitionJson["ViewsInfo"].push_back(viewJson);
				viewNode = viewNode.next_sibling();
			}


			outJson["Partitions"].push_back(partitionJson);
			bufferPartition = bufferPartition.next_sibling();
		}
	}

	void AngKCommonXMLParser::DataRemapInfo_Json(pugi::xml_node& rootNode, unsigned long ulAlgo, nlohmann::json& outJson)
	{
		pugi::xml_node dataRemapTable = rootNode.child(XML_NODE_CHIPDATA_DATAREMAPTABLE);
		int dataRemapTableCount = std::distance(dataRemapTable.begin(), dataRemapTable.end());
		for (int i = 0; i < dataRemapTableCount; ++i)
		{
			if (CheckAlgoRange(ulAlgo, dataRemapTable.attribute("Algo").as_string()))
				break;

			dataRemapTable = dataRemapTable.next_sibling();
		}

		if (dataRemapTable == nullptr)
			return;

		pugi::xml_node DataMapNode = dataRemapTable.child(XML_NODE_CHIPDATA_DATAMAP);
		outJson["DataMapCnt"] = dataRemapTableCount;
		outJson["DataMaps"] = nlohmann::json::array();
		for (int i = 0; i < dataRemapTableCount; ++i)
		{
			nlohmann::json dataMap;
			dataMap["Name"] = DataMapNode.attribute("Name").as_string();
			dataMap["OrgAddr"] = DataMapNode.attribute("OrgAddr").as_string();
			dataMap["MapAddr"] = DataMapNode.attribute("MapAddr").as_string();
			dataMap["Size"] = DataMapNode.attribute("Size").as_string();

			outJson["DataMaps"].push_back(dataMap);
			DataMapNode = DataMapNode.next_sibling();
		}
	}

	void AngKCommonXMLParser::xmlNodeToJson(const std::string& xmlFile, std::string& jsonStr)
	{
		pugi::xml_document xmlDoc;
		const wchar_t* encodedName = reinterpret_cast<const wchar_t*>(QString::fromStdString(xmlFile).utf16());
		pugi::xml_parse_result result = xmlDoc.load_file(encodedName);
		std::stringstream oss;
		std::string xmlString;
		xmlDoc.save(oss, "", pugi::format_raw);
		xmlString = oss.str();
		//jsonStr = xml2json(xmlString.c_str());
	}

	bool AngKCommonXMLParser::CheckAlgoRange(unsigned long ulAlgo, QString algoRange)
	{
		if (algoRange.isEmpty())
			return false;

		aRange TmpRange;
		std::vector<aRange> rangeVec;
		QStringList strList = algoRange.split(",");
		for (auto strAlgo : strList)
		{
			if (strAlgo.contains("-")) {
				if (!GetInteralVal(strAlgo, TmpRange)) {
					return false;
				}
				rangeVec.push_back(TmpRange);
			}
			else {
				bool bOk;
				uint64_t n2Hex = -1;
				n2Hex = strAlgo.toInt(&bOk, 16);
				if (bOk)
				{
					TmpRange.Min = n2Hex;
					TmpRange.Max = TmpRange.Min;
					rangeVec.push_back(TmpRange);
				}
			}
		}

		for (auto _range : rangeVec)
		{
			if (_range.Min == -1)
				return false;

			if (_range.Max == -1)
				return false;

			if (ulAlgo >= _range.Min && ulAlgo <= _range.Max)
			{
				return true;
			}
		}

		return false;
	}

	bool AngKCommonXMLParser::GetInteralVal(QString algoRange, aRange& _2Range)
	{
		uint64_t vStart, vEnd;

		QStringList algoList = algoRange.split("-");

		if (algoList.size() < 2)
			return false;

		bool bOk;
		vStart = algoList[0].toInt(&bOk, 16);
		vEnd = algoList[algoList.count() - 1].toInt(&bOk, 16);

		_2Range.Min = vStart;
		_2Range.Max = vEnd;

		return true;
	}

	uint64_t AngKCommonTools::GetByteSum(uchar* pData, uint64_t Size)
	{
		uint64_t ByteSum = 0;
		int64_t i;
		for (i = 0; i < Size; i++) {
			ByteSum += pData[i];
		}
		return ByteSum;
	}

	QString AngKCommonTools::QStringToAsciiString(const QString& input)
	{
		QByteArray asciiData = input.toLatin1();
		QString asciiString = asciiData.toHex();
		return asciiString;
	}

	QString AngKCommonTools::GenerateRandomString(int length)
	{
		QString randomString;
		const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

		for (int i = 0; i < length; ++i) {
			//int index = QRandomGenerator::global()->bounded(possibleCharacters.length());
			//randomString.append(possibleCharacters.at(index));
			char randomChar = QRandomGenerator::global()->bounded(0, 255); // 生成0到255之间的随机ASCII字符
			randomString.append(randomChar);
		}

		return randomString;
	}

	uint16_t AngKCommonTools::GetSktEnable(BPUInfo _bpuInfo)
	{
		uint16_t sktNum = 0;
		for (int i = 0; i < _bpuInfo.vecInfos.size(); ++i) {
			if (_bpuInfo.vecInfos[i].SktCnt == 1)
				sktNum |= BPUCalVector[_bpuInfo.vecInfos[i].idx * 2];
			else if (_bpuInfo.vecInfos[i].SktCnt == 2)
				sktNum |= BPUCalVector[_bpuInfo.vecInfos[i].idx * 2 + 1];
		}
		return sktNum;
	}

	QString AngKCommonTools::GetFileExt(const QString& strFilePath)
	{
		QFileInfo fileInfo(strFilePath);
		return fileInfo.suffix();
	}

	std::string AngKCommonTools::RemoveExtension(const QString& fileName, const QString& extension)
	{
		QString baseName = fileName; // 创建文件名的副本
		if (baseName.endsWith(extension, Qt::CaseInsensitive)) {
			// 如果文件名以指定的扩展名结尾，去除它
			baseName = baseName.left(baseName.length() - extension.length());
		}
		return baseName.toStdString();
	}

	QString AngKCommonTools::TranslateErrorMsg(int nResultcode)
	{
		return QString();
	}

	bool AngKCommonTools::ContainsChinese(const std::string& str)
	{
		std::regex pattern(u8"[\u4e00-\u9fa5]");
		return std::regex_search(str.cbegin(), str.cend(), pattern);
	}

	void AngKCommonTools::GetDeviceDrvPath(std::string& devDrvFile, std::string& devDrvFilePath)
	{
		std::string chipAlgoFileName = devDrvFile;// DeviceDrv驱动文件
		chipAlgoFileName = chipAlgoFileName.substr(0, chipAlgoFileName.find_first_of("."));
		devDrvFilePath = Utils::AngKPathResolve::localDeviceDrvFilePath(QString::fromStdString(chipAlgoFileName)).toStdString();
	}

	void AngKCommonTools::GetMasterDrvPath(std::string& mstDrvFile, std::string& mstDrvFilePath)
	{
		std::string chipMstFileName = mstDrvFile;// MasterDrv驱动文件
		chipMstFileName = chipMstFileName.substr(0, chipMstFileName.find_first_of("."));
		mstDrvFilePath = Utils::AngKPathResolve::localMasterDrvFilePath(QString::fromStdString(chipMstFileName)).toStdString();
	}

	std::string AngKCommonTools::GetLogFrom(int devID)
	{
		QString fromDev;
		if (devID != 8) {
			fromDev = "B" + QString::number(devID);
		}
		else {
			fromDev = "MU";
		}

		return fromDev.toStdString();
	}

	std::string AngKCommonTools::TranslateMessageCmdID(uint16_t cmdID)
	{
		std::string PTCmdName = "";
		switch ((eSubCmdID)cmdID)
		{
		case eSubCmdID::SubCmd_DataTrans_FIBER2SSD:
			PTCmdName = "FIBER2SSD";
			break;
		case eSubCmdID::SubCmd_DataTrans_FIBER2SKT:
			PTCmdName = "FIBER2SKT";
			break;
		case eSubCmdID::SubCmd_DataTrans_FIBER2DDR:
			PTCmdName = "FIBER2DDR";
			break;
		case eSubCmdID::SubCmd_DataTrans_SSD2FIBER:
			PTCmdName = "SSD2FIBER";
			break;
		case eSubCmdID::SubCmd_DataTrans_SSD2SKT:
			PTCmdName = "SSD2SKT";
			break;
		case eSubCmdID::SubCmd_DataTrans_SKT2FIBER:
			PTCmdName = "SKT2FIBER";
			break;
		case eSubCmdID::SubCmd_DataTrans_DDR2FIBER:
			PTCmdName = "DDR2FIBER";
			break;
		case eSubCmdID::SubCmd_DataTrans_SSD2DDR:
			PTCmdName = "SSD2DDR";
			break;
		case eSubCmdID::SubCmd_DataTrans_DDR2SSD:
			PTCmdName = "DDR2SSD";
			break;
		case eSubCmdID::SubCmd_ReadCRC32:
			PTCmdName = "ReadCRC32";
			break;
		case eSubCmdID::SubCmd_MU_InstallFPGA:
			PTCmdName = "InstallFPGA";
			break;
		case eSubCmdID::SubCmd_MU_InstallDriver:
			PTCmdName = "InstallDriver";
			break;
		case eSubCmdID::SubCmd_MU_SetChipInfo:
			PTCmdName = "SetChipInfo";
			break;
		case eSubCmdID::SubCmd_MU_SetDriverSelfPara:
			PTCmdName = "SetDriverSelfPara";
			break;
		case eSubCmdID::SubCmd_MU_SetDriverCommon:
			PTCmdName = "SetDriverCommon";
			break;
		case eSubCmdID::SubCmd_MU_SetDriverPinMap:
			PTCmdName = "SetDriverPinMap";
			break;
		case eSubCmdID::SubCmd_MU_SetDriverPartitionTable:
			PTCmdName = "SetDriverPartitionTable";
			break;
		case eSubCmdID::SubCmd_MU_DoCmdSequence:
			PTCmdName = "DoCmdSequence";
			break;
		case eSubCmdID::SubCmd_MU_SetDataBufferInfo:
			PTCmdName = "SetDataBufferInfo";
			break;
		case eSubCmdID::SubCmd_MU_DownloadSSDComplete:
			PTCmdName = "DownloadSSDComplete";
			break;
		case eSubCmdID::SubCmd_MU_SetBPUAttribute:
			PTCmdName = "SetBPUAttribute";
			break;
		case eSubCmdID::SubCmd_MU_SetSNWithJson:
			PTCmdName = "SetSNWithJson";
			break;
		case eSubCmdID::SubCmd_MU_AdapterRead:
			PTCmdName = "AdapterRead";
			break;
		case eSubCmdID::SubCmd_MU_AdapterWrite:
			PTCmdName = "AdapterWrite";
			break;
		case eSubCmdID::SubCmd_MU_AdapterIncreaseCount:
			PTCmdName = "AdapterIncreaseCount";
			break;
		case eSubCmdID::SubCmd_MU_GetBPUInfo:
			PTCmdName = "GetBPUInfo";
			break;
		case eSubCmdID::SubCmd_MU_GetDeviceInfo:
			PTCmdName = "GetDeviceInfo";
			break;
		case eSubCmdID::SubCmd_MU_SetDeviceAlias:
			PTCmdName = "SetDeviceAlias";
			break;
		case eSubCmdID::SubCmd_MU_GetSktInfo:
			PTCmdName = "GetSktInfo";
			break;
		case eSubCmdID::SubCmd_MU_GetSktInfoSimple:
			PTCmdName = "GetSktInfoSimple";
			break;
		case eSubCmdID::SubCmd_MU_GetMainBoardInfo:
			PTCmdName = "GetMainBoardInfo";
			break;
		case eSubCmdID::SubCmd_MU_RebootBPU:
			PTCmdName = "RebootBPU";
			break;
		case eSubCmdID::SubCmd_MU_SetBufferMapInfo:
			PTCmdName = "SetBufferMapInfo";
			break;
		case eSubCmdID::SubCmd_MU_UpdateFw:
			PTCmdName = "UpdateFw";
			break;
		case eSubCmdID::SubCmd_MU_RebootMU:
			PTCmdName = "RebootMU";
			break;
		case eSubCmdID::SubCmd_MU_UpdateDeviceTime:
			PTCmdName = "UpdateDeviceTime";
			break;
		case eSubCmdID::SubCmd_MU_DebugSetting:
			PTCmdName = "DebugSetting";
			break;
		case eSubCmdID::SubCmd_MU_GetRebootCause:
			PTCmdName = "GetRebootCause";
			break;
		case eSubCmdID::SubCmd_MU_ProgramSetting:
			PTCmdName = "ProgramSetting";
			break;
		case eSubCmdID::SubCmd_MU_GetProgramSetting:
			PTCmdName = "GetProgramSetting";
			break;
		case eSubCmdID::SubCmd_MU_ProgrammerSelfTest:
			PTCmdName = "ProgrammerSelfTest";
			break;
		case eSubCmdID::SubCmd_MU_MasterChipAnalyze:
			PTCmdName = "MasterChipAnalyze";
			break;
		case eSubCmdID::SubCmd_MU_ReadChipExtcsd:
			PTCmdName = "ReadChipExtcsd";
			break;
		case eSubCmdID::SubCmd_MU_GetSKTEnable:
			PTCmdName = "GetSKTEnable";
			break;
		case eSubCmdID::SubCmd_MU_SetProgress:
			PTCmdName = "SetProgress";
			break;
		case eSubCmdID::SubCmd_MU_SetLog:
			PTCmdName = "SetLog";
			break;
		case eSubCmdID::SubCmd_MU_SetEvent:
			PTCmdName = "SetEvent";
			break;
		case eSubCmdID::SubCmd_ReadBuffData:
			PTCmdName = "ReadBuffData";
			break;
		case eSubCmdID::SubCmd_MU_SetPartitionTableHeadAddr:
			PTCmdName = "SetPartitionTableHeadAddr";
			break;
		default:
			break;
		}
		return PTCmdName;
	}

	std::string AngKCommonTools::calculatePercentage(double part, double whole)
	{
		if (whole == 0) {
			// 防止除以零的情况
			return "Invalid operation";
		}

		double percentage = (part / whole) * 100.0;
		percentage = std::round(percentage * 100.0) / 100.0;
		std::string perStr = QString("%1%").arg(percentage, 0, 'f', 2).toStdString();
		return perStr; // 返回格式化后的字符串
	}

	uint32_t AngKCommonTools::GetBPUEn(uint32_t nSktEn)
	{
		uint32_t BPUEn = 0;
		for (int32_t i = 0; i < 8; i++) {
			if ((nSktEn & 0x03) != 0) {
				BPUEn |= (1 << i);
			}
			nSktEn >>= 2;
		}
		return BPUEn;
	}

	uint32_t AngKCommonTools::GetBPUCount(uint32_t nSktEn)
	{
		uint32_t BPUEn = GetBPUEn(nSktEn);
		int count = 0;
		for (int i = 0; i < 8; ++i) {
			if (BPUEn & (1 << i)) {
				++count;
			}
		}
		return count;
	}

	uint32_t AngKCommonTools::GetBPUIndex(uint32_t nSktEn)
	{
		uint32_t BPUEn = GetBPUEn(nSktEn);
		int nIndex = 0;
		for (int i = 0; i < 8; ++i) {
			if (BPUEn & (1 << i)) {
				nIndex = i;
				break;
			}
		}
		return nIndex;
	}

	std::string AngKCommonTools::GetModeStr(ConnectType modeTyoe)
	{
		std::string modeStr;

		switch (modeTyoe)
		{
		case ConnectType::USB:
			modeStr = "USB";
			break;
		case ConnectType::Ethernet:
			modeStr = "Ethernet";
			break;
		case ConnectType::Demo:
			modeStr = "Demo";
			break;
		case ConnectType::None:
		default:
			break;
		}

		return modeStr;
	}

	int AngKCommonTools::SwapSKTIdx_Soft2Auto(int nSoftSktEnable, int nSKTNum)
	{
		int nAutoSktEnable = 0;

		for (int i = 0; i < nSKTNum; ++i) {
			if (nSoftSktEnable & 0x1) {
				int nTemp = 1 << (Soft2AutoMapping[i + 1] - 1);
				nAutoSktEnable |= nTemp;
			}
			nSoftSktEnable = nSoftSktEnable >> 1;
		}

		return nAutoSktEnable;
	}

	int AngKCommonTools::SwapSKTIdx_Auto2Soft(int nAutoSktEnable, int nSKTNum)
	{
		int nSoftSktEnable = 0;
		for (int i = 0; i < nSKTNum; ++i) {
			if (nAutoSktEnable & 0x1) {
				int nTemp = 1 << (Auto2SoftMapping[i + 1] - 1);
				nSoftSktEnable |= nTemp;
			}
			nAutoSktEnable = nAutoSktEnable >> 1;
		}

		return nSoftSktEnable;
	}

	QString AngKCommonTools::CreateReportFile(QString& taskFileName)
	{
		QString reportFile = QString("%1/%2_Report[%3].html").arg(Utils::AngKPathResolve::logPath(), taskFileName, QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
		return reportFile;
	}

	void AngKCommonTools::OutputReportFile(QString& taskFileName, QString& projFileName)
	{
		ACReportRecord& GReporter = GetReporter();

		if (taskFileName.isEmpty())
			return;

		if (!projFileName.isEmpty()) {

			QVector<DeviceStu> devVec = ACDeviceManager::instance().getAllDevInfo();
			for (auto& devInfo : devVec) {
				GReporter.AddSite(projFileName, &devInfo, 8);
			}

			ACHtmlLogWriter ReportWriter;
			//切换Task时，需要输出一次报告
			QFileInfo fInfo(taskFileName);
			QString strReportPath = Utils::AngKCommonTools::CreateReportFile(fInfo.baseName());
			if (ReportWriter.CreateLog(strReportPath, ACHtmlLogWriter::LOGTYPE_REPORT)) {

				if (GReporter.WriteReportToAloneFile(&ReportWriter) == ACERR_OK) {
					//ALOG_INFO("Report Task File : %s successfully", "CU", "--", strReportPath.toStdString().c_str());
				}
				else {
					//ALOG_ERROR("Report Task File : %s failed", "CU", "--", strReportPath.toStdString().c_str());
				}
			}
		}
	}

	std::string AngKCommonTools::Full2RelativePath(const QString& dst_path, const QString& src_path) {
		QString src_dir = src_path;
		src_dir.replace('\\', '/');
		src_dir = src_dir.mid(0, src_dir.lastIndexOf('/') + 1);
		if (dst_path.mid(0, 1) != src_dir.mid(0, 1))
			return dst_path.toStdString();

		int slow = 0, fast = src_dir.indexOf('/');
		while (fast != -1) {
			if (dst_path.mid(0, fast) != src_dir.mid(0, fast))
				break;
			slow = fast;
			fast = src_dir.indexOf('/', slow + 1);
		}

		QString rel_path;
		if (fast != -1) {
			int cnt = src_dir.mid(fast + 1).split('/').size();
			for (int i = 0; i < cnt; ++i) {
				rel_path += "../";
			}
			rel_path += dst_path.mid(slow + 1);
		}
		else rel_path = QString(".%1").arg(dst_path.mid(slow));
		return rel_path.toStdString();
	}

	std::string AngKCommonTools::Relative2FullPath(const QString& dst_path, const QString& src_path) {
		QString src_dir = src_path;
		src_dir.replace('\\', '/');
		src_dir = src_dir.mid(0, src_dir.lastIndexOf('/') + 1);
		if (dst_path[0] != '.')
			return dst_path.toStdString();

		if (dst_path[0] == '.' && dst_path[1] == '/') {
			src_dir += dst_path.mid(2);
			return src_dir.toStdString();
		}

		int flag = 0;
		while (dst_path[flag] == '.' && dst_path[flag + 1] == '.' && dst_path[flag + 2] == '/') {
			flag += 3;
		}

		flag /= 3;
		for (int i = 0; i <= flag; ++i) {
			src_dir = src_dir.mid(0, src_dir.lastIndexOf('/'));
		}

		src_dir += dst_path.mid(flag * 3 - 1);

		return src_dir.toStdString();
	}

	QString AngKCommonTools::GetDesFileValue(const QString& fileName, const QString& key) {
		QFile file(fileName);
		if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
			//ALOG_ERROR("Open File : %s failed", "CU", "--", fileName.toStdString().c_str());
			return "";
		}

		QString tmpLine;
		while (!file.atEnd()) {
			tmpLine = file.readLine();
			if (tmpLine.indexOf(":") < 0) {
				continue;
			}
			if (tmpLine.split(":")[0].trimmed() == key) {
				return tmpLine.split(":")[1].trimmed();
			}
		}
		return "";
	}

	pugi::xml_node* AngKCommonXMLParser::findNode(pugi::xml_document& doc, const char* targetNodeName)
	{
		QString chipDataXML;
		QDir tempDir = Utils::AngKPathResolve::localTempFolderPath();

		QFileInfoList filelist = tempDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
		for (const auto& xmlFileInfo : tempDir.entryInfoList()) {
			if (xmlFileInfo.fileName().contains("chipData", Qt::CaseInsensitive)) {
				chipDataXML = xmlFileInfo.absoluteFilePath();
			}
		}

		const wchar_t* encodedName = reinterpret_cast<const wchar_t*>(chipDataXML.utf16());
		if (doc.load_file(encodedName)) {
			pugi::xml_node targetNode = doc.child(targetNodeName);
			if (targetNode) {
				return new pugi::xml_node(targetNode);
			}
			else {
				return findNodeRecursive(doc.first_child(), targetNodeName);
			}
		}
		return nullptr; // 返回一个空节点表示未找到
	}

	pugi::xml_node* AngKCommonXMLParser::findNodeRecursive(pugi::xml_node node, const char* targetNodeName)
	{
		if (node.empty()) {
			return nullptr; // 返回空指针表示未找到
		}
		if (std::string(node.name()) == targetNodeName) {
			return new pugi::xml_node(node);
		}
		for (pugi::xml_node child : node.children()) {
			pugi::xml_node* foundNode = findNodeRecursive(child, targetNodeName);
			if (foundNode) {
				return foundNode;
			}
		}
		return nullptr; // 返回空指针表示未找到
	}

	QRect AngKChipConfigTools::String2Rect(const char* strRect)
	{
		if (nullptr == strRect)
			return QRect();

		QStringList strList = QString::fromStdString(strRect).split(",");

		if (strList.count() != 4)
			return QRect();


		QRect retRect(strList[0].toInt(), strList[1].toInt(), strList[2].toInt(), strList[3].toInt());

		return retRect;
	}

	const char* AngKChipConfigTools::EditVaule_Int2CString(int nType)
	{
		const char* ret = "";
		switch (nType)
		{
		case DEC:
			ret = "DEC";
			break;
		case HEX:
			ret = "HEX";
			break;
		case STR:
			ret = "STR";
			break;
		default:
			break;
		}

		return ret;
	}

	int AngKChipConfigTools::EditVaule_CString2Int(const char* strType)
	{
		int ret = -1;

		if (strType == "DEC")
		{
			ret = DEC;
		}
		else if (strType == "HEX")
		{
			ret = HEX;
		}
		else if (strType == "STR")
		{
			ret = STR;
		}
		return ret;
	}

	const char* AngKChipConfigTools::EditEndian_Int2CString(int nType)
	{
		const char* ret = "";
		switch (nType)
		{
		case Little:
			ret = "L";
			break;
		case Big:
			ret = "B";
			break;
		}

		return ret;
	}

	int AngKChipConfigTools::EditEndian_CString2Int(const char* strType)
	{
		int ret = -1;

		if (strType == "L")
		{
			ret = Little;
		}
		else if (strType == "B")
		{
			ret = Big;
		}
		return ret;
	}

	int AngKChipConfigTools::Hex_String2Int(const char* strHex)
	{
		QString qHexString = strHex;

		int nPos = qHexString.lastIndexOf("0x");

		qHexString = qHexString.mid(nPos, qHexString.size() - nPos);

		bool k;
		int nHex = qHexString.toInt(&k, 16);

		return nHex;
	}
}