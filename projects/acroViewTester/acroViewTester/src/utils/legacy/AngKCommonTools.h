#pragma once

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRect>
#include <QXmlStreamReader>
#include "json.hpp"
#include "pugixml.hpp"
#include "XmlDefine.h"
#include "GlobalDefine.h"
#include "AppModeType.h"
namespace Utils
{
	struct ErrorInfo
	{
		unsigned int id;
		std::string text;
	};

	typedef struct talgoRange {
		uint64_t Min;
		uint64_t Max;

		talgoRange()
		{
			Min = -1;
			Max = -1;
		}
	}aRange;

	//AG06设备的SiteIdx 对应的是 auto
	static std::map<int, int> Soft2AutoMapping = { {1, 9},{2,13},{3,10},{4,14},
												{5, 11},{6,15},{7,12},{8,16},
												{9, 1},{10,5},{11,2},{12,6},
												{13, 3},{14,7},{15,4},{16,8} };

	//auto 对应的是 AG06设备的SiteIdx
	static std::map<int, int> Auto2SoftMapping = {	{1, 9},{2,11},{3,13},{4,15},
												{5, 10},{6,12},{7,14},{8,16},
												{9, 1},{10,3},{11,5},{12,7},
												{13, 2},{14,4},{15,6},{16,8} };

	class AngKCommonTools
	{

	public:
		static QString GetDrvVersion(const QString& drvPath, const QString& verName);

		static void DepressBinFile(std::string& binZip, QStringList& pathList);

		static void SwitchFileFormat(std::string origianlFile, std::string newExtension, std::string& newFile);

		/// <summary>
		/// 获取Byte字节校验和
		/// </summary>
		/// <param name="pData">数据段</param>
		/// <param name="Size">数据长度</param>
		/// <returns></returns>
		static uint64_t GetByteSum(uchar* pData, uint64_t Size);

		static QString QStringToAsciiString(const QString& input);

		static QString GenerateRandomString(int length);

		static uint16_t GetSktEnable(BPUInfo _bpuInfo);

		/// <summary>
		/// 给定一个全路径，返回后缀名
		/// </summary>
		/// <param name="strFilePath">文件路径</param>
		/// <returns></returns>
		static QString GetFileExt(const QString& strFilePath);

		/// <summary>
		/// 给定一个文件名和文件后缀，返回文件名
		/// </summary>
		/// <param name="strFilePath">文件路径</param>
		/// <returns></returns>
		static std::string RemoveExtension(const QString& fileName, const QString& extension);

		static QString TranslateErrorMsg(int nResultcode);

		static bool ContainsChinese(const std::string& str);

		/// <summary>
		/// 获取Device驱动文件路径
		/// </summary>
		/// <param name="devDrvFile">设备驱动文件名</param>
		/// <param name="mstDrvFile">master驱动文件名</param>
		static void GetDeviceDrvPath(std::string& devDrvFile, std::string& devDrvFilePath);

		/// <summary>
		/// 获取Master驱动文件路径
		/// </summary>
		/// <param name="devDrvFile">设备驱动文件名</param>
		/// <param name="mstDrvFile">master驱动文件名</param>
		static void GetMasterDrvPath(std::string& mstDrvFile, std::string& mstDrvFilePath);

		/// <summary>
		/// 根据设备ID值，返回设备名称
		/// </summary>
		/// <param name="devID">BPU0-7，MU8</param>
		/// <returns></returns>
		static std::string GetLogFrom(int devID);

		/// <summary>
		/// 翻译CmdID为命令名称
		/// </summary>
		/// <param name="cmdID">命令ID</param>
		/// <returns></returns>
		static std::string TranslateMessageCmdID(uint16_t cmdID);

		/// <summary>
		/// 计算两数的百分比
		/// </summary>
		/// <param name="part">分子传参</param>
		/// <param name="whole">分母传参</param>
		/// <returns>返回百分比的字符串</returns>
		static std::string calculatePercentage(double part, double whole);

		/// <summary>
		/// 根据SktEn选择情况获取当前使用了哪些BPU
		/// </summary>
		/// <param name="nSktEn">SktEn值，只对应一个设备的</param>
		/// <returns>返回BPU使用了哪些</returns>
		static uint32_t GetBPUEn(uint32_t nSktEn);

		static uint32_t GetBPUCount(uint32_t nBPUEn);

		static uint32_t GetBPUIndex(uint32_t nBPUID);

		static std::string GetModeStr(ConnectType modeTyoe);

		static int SwapSKTIdx_Soft2Auto(int nSoftSktEnable, int nSKTNum);

		static int SwapSKTIdx_Auto2Soft(int nAutoSktEnable, int nSKTNum);

		static QString CreateReportFile(QString& taskFileName);

		static void OutputReportFile(QString& taskFileName, QString& projFileName);

		//获取工程文件相对于TASK文件的相对路径
		static std::string Full2RelativePath(const QString& dst_path, const QString& src_dir);

		//获取工程文件相对于TASK文件的绝对路径
		static std::string Relative2FullPath(const QString& dst_path, const QString& src_dir);

		static QString GetDesFileValue(const QString& fileName, const QString& key);
	};

	class AngKCommonXMLParser
	{

	public:
		/// <summary>
		/// 解析chipConfigXML,界面展示UI自动生成并设置spcBit
		/// </summary>
		/// <param name="cfgFile">文件路径</param>
		/// <param name="dataJson">转换后的json</param>
		/// <returns>xml解析是否成功</returns>
		static int ParserChipConfigXML(QString cfgFile, QString& dataJson);

		/// <summary>
		/// 解析chipDataXML
		/// </summary>
		/// <param name="nType">下发的指令ID，解析的xml节点不同</param>
		/// <param name="ulAlgo">选择芯片获取的algo</param>
		/// <param name="dataFile">xml文件路径</param>
		/// <param name="dataJson">输出json</param>
		/// <returns>xml解析是否成功</returns>
		static int ParserChipDataXML(uint16_t nType, unsigned long ulAlgo, QString dataFile, QString& dataJson, bool bAddDesc = false);

		/// <summary>
		/// DriverVoltage命令输出JSON
		/// </summary>
		/// <param name="rootNode">文件根节点</param>
		/// <param name="ulAlgo">选择芯片获取的algo</param>
		/// <param name="outJson">输出json</param>
		static void DriverCommonPara_Json(pugi::xml_node& rootNode, unsigned long ulAlgo, nlohmann::json& outJson, bool bAddDesc = false);

		/// <summary>
		/// DriverPara命令输出JSON
		/// </summary>
		/// <param name="rootNode">文件根节点</param>
		/// <param name="ulAlgo">选择芯片获取的algo</param>
		/// <param name="outJson">输出json</param>
		static void DriverSelfPara_Json(pugi::xml_node& rootNode, unsigned long ulAlgo, nlohmann::json& outJson, bool bAddDesc = false);

		/// <summary>
		/// DriverPinMap命令输出JSON
		/// </summary>
		/// <param name="rootNode">文件根节点</param>
		/// <param name="ulAlgo">选择芯片获取的algo</param>
		/// <param name="outJson">输出json</param>
		static void DriverPinMap_Json(pugi::xml_node& rootNode, unsigned long ulAlgo, nlohmann::json& outJson);

		/// <summary>
		/// Block命令输出JSON
		/// </summary>
		/// <param name="rootNode">文件根节点</param>
		/// <param name="ulAlgo">选择芯片获取的algo</param>
		/// <param name="outJson">输出json</param>
		static void PartitionBlock_Json(pugi::xml_node& rootNode, unsigned long ulAlgo, nlohmann::json& outJson);

		/// <summary>
		/// BufferMapInfo命令输出JSON
		/// </summary>
		/// <param name="rootNode">文件根节点</param>
		/// <param name="ulAlgo">选择芯片获取的algo</param>
		/// <param name="outJson">输出json</param>
		static void BufferMapInfo_Json(pugi::xml_node& rootNode, unsigned long ulAlgo, nlohmann::json& outJson);

		/// <summary>
		/// DataRemapInfo命令输出JSON
		/// </summary>
		/// <param name="rootNode">文件根节点</param>
		/// <param name="ulAlgo">选择芯片获取的algo</param>
		/// <param name="outJson">输出json</param>
		static void DataRemapInfo_Json(pugi::xml_node& rootNode, unsigned long ulAlgo, nlohmann::json& outJson);

		/// <summary>
		/// xml节点转Json
		/// </summary>
		/// <param name="xmlFile">读取之后的xml文件</param>
		/// <param name="jsonStr">输出json</param>
		static void xmlNodeToJson(const std::string& xmlFile, std::string& jsonStr);

		/// <summary>
		/// 检查Algo是否在范围内
		/// </summary>
		/// <param name="ulAlgo">选择芯片获取的algo</param>
		/// <param name="algoRange">xml中对应的algo范围</param>
		/// <returns></returns>
		static bool CheckAlgoRange(unsigned long ulAlgo, QString algoRange);

		/// <summary>
		/// algo范围最大最小值
		/// </summary>
		/// <param name="algoRange">algo范围</param>
		/// <param name="_2Range">输出两个边界值</param>
		/// <returns></returns>
		static bool GetInteralVal(QString algoRange, aRange& _2Range);

		/// <summary>
		/// 检查ChipData_xxxx.xml这个文件内是否存在目标节点
		/// </summary>
		/// <param name="doc">xml_document引用传入，否则函数返回节点会导致垂悬指针</param>
		/// <param name="targetNodeName">目标节点名称</param>
		/// <returns></returns>
		static pugi::xml_node* findNode(pugi::xml_document& doc, const char* targetNodeName);

		/// <summary>
		/// findNode内部进行递归调用，判断节点是否存在并返回
		/// </summary>
		/// <param name="node">递归节点</param>
		/// <param name="targetNodeName">目标名称</param>
		/// <returns></returns>
		static pugi::xml_node* findNodeRecursive(pugi::xml_node node, const char* targetNodeName);
	};

	class AngKChipConfigTools 
	{
	public:
		static QRect String2Rect(const char* strRect);

		static const char* EditVaule_Int2CString(int nType);

		static int EditVaule_CString2Int(const char* strType);

		static const char* EditEndian_Int2CString(int nType);

		static int EditEndian_CString2Int(const char* strType);

		static int Hex_String2Int(const char* strHex);
	};
}