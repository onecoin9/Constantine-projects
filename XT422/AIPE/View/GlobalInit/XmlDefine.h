#pragma once
#include "../pugixml/pugixml.hpp"

/*AP8000xml原保存方式*/
#define XML_NODE_BUFFERMAP				"BufMap"
#define XML_NODE_BUFFERMAP_CHILD_BUFFER	"Buffer"


/// <summary>
/// ChipConfig.XML 定义节点名称
/// </summary>
#define XML_NODE_CHIPCONFIG					"ChipConfig"
#define XML_NODE_CHIPCONFIG_DEVICE			"Device"
#define XML_NODE_CHIPCONFIG_CHIPLIST		"ChipList"
#define XML_NODE_CHIPCONFIG_CHIP			"Chip"
#define XML_NODE_CHIPCONFIG_PROPERTYSHEET	"PropertySheet"
#define XML_NODE_CHIPCONFIG_TABPAGE			"TabPage"
#define XML_NODE_CHIPCONFIG_GROUP			"Group"
#define XML_NODE_CHIPCONFIG_LABEL			"Label"
#define XML_NODE_CHIPCONFIG_TEXTEDIT		"TextEdit"
#define XML_NODE_CHIPCONFIG_COMBOBOX		"ComboBox"
#define XML_NODE_CHIPCONFIG_COMBOITEM		"ComboItem"
#define XML_NODE_CHIPCONFIG_CHECKBOX		"CheckBox"


/// <summary>
/// ChipData.XML 定义节点名称
/// </summary>
#define XML_NODE_CHIPDATA					"ChipData"
///VlotageSet: 通用驱动参数集合
#define XML_NODE_CHIPDATA_DRVCOMMONPARASET	"DrvCommonParaSet"
#define XML_NODE_CHIPDATA_DRVPARATABLE		"DrvParaTable"
#define XML_NODE_CHIPDATA_STRUCT			"Struct"
#define XML_NODE_CHIPDATA_PROPERTY			"Property"

///DrvParaSet: 自定义驱动参数集集合
#define XML_NODE_CHIPDATA_DRVSELFPARASET	"DrvSelfParaSet"

///PinMapSet: 引脚映射集集合
#define XML_NODE_CHIPDATA_PINMAPSET			"PinMapSet"
#define XML_NODE_CHIPDATA_PINMAPTABLE		"PinMapTable"
#define XML_NODE_CHIPDATA_PIN				"Pin"
#define XML_NODE_CHIPDATA_GROUP				"Group"

///BlocksSet: 块集合
#define XML_NODE_CHIPDATA_BLOCKSET			"BlocksSet"
#define XML_NODE_CHIPDATA_BLOCKTABLE		"BlocksTable"
#define XML_NODE_CHIPDATA_BLOCKPARTITION	"Partition"
#define XML_NODE_CHIPDATA_BLOCK				"Block"

///BufferMapSet: 缓冲区映射表集合
#define XML_NODE_CHIPDATA_BUFFERMAPSET		"BufferMapSet"
#define XML_NODE_CHIPDATA_BUFFERMAPTABLE	"BufferMapTable"
#define XML_NODE_CHIPDATA_BUFFERPARTITION	"Partition"
#define XML_NODE_CHIPDATA_VIEW				"View"

///DataRemapSet: 数据写入时的映射集合
#define XML_NODE_CHIPDATA_DATAREMAPSET		"DataRemapSet"
#define XML_NODE_CHIPDATA_DATAREMAPTABLE	"DataRemapTable"
#define XML_NODE_CHIPDATA_DATAMAP			"DataMap"

///FileInfoSet: 数据写入时可以指定放入的档案类型
#define XML_NODE_CHIPDATA_FILEINFOSET		"FileInfoSet"
#define XML_NODE_CHIPDATA_FILEINFO			"FileInfo"
#define XML_NODE_CHIPDATA_FILETAG			"FileTag"

///FileRelocationSet: 档案被加载的时候自动指定的Relocation信息
#define XML_NODE_CHIPDATA_FILERELOCATIONSET		"FileRelocationSet"
#define XML_NODE_CHIPDATA_FILERELOCATIONTABLE	"FileRelocationTable"
#define XML_NODE_CHIPDATA_RELOCATION			"Relocation"

///FileParserSet: 文件解析器集
#define XML_NODE_CHIPDATA_FILEPARSERSET			"FileParserSet"
#define XML_NODE_CHIPDATA_FILEPARSERTABLE		"FileParserTable"
#define XML_NODE_CHIPDATA_PARSER				"Parser"

///DriverFeatureSet: 驱动特性集合
#define XML_NODE_CHIPDATA_DRIVERFEATURESET		"DriverFeatureSet"
#define XML_NODE_CHIPDATA_DRIVERFEATURETABLE	"DriverFeatureTable"
#define XML_NODE_CHIPDATA_UIDSUPPORT			"UIDSupport"
#define XML_NODE_CHIPDATA_SECURITYSOLUTION		"SecuritySolution"
#define XML_NODE_CHIPDATA_TESTSOLUTION			"TestSolution"

///Device: 设备操作执行函数
#define XML_NODE_CHIPDATA_DEVICE			"Device"
#define XML_NODE_CHIPDATA_TIMESPAN			"TimeSpan"
#define XML_NODE_CHIPDATA_FUNCTION			"Function"

/// <summary>
/// xxxx.acxml 定义节点名称
/// </summary>
#define XML_ROOTNODE_EMMC			"eMMC"	//根节点

#define XML_NODE_EMMC_CHIP			"ChipID"

#define XML_NODE_EMMC_VERSION		"Version"
#define XML_EMMC_VERSION			"1"

//Checksum 节点
#define XML_NODE_EMMC_CHECKSUM		"Checksum"
#define XML_NODE_EMMC_BYTESUM		"ByteSum"
#define XML_NODE_EMMC_PARTITION		"Partition"

//Partitions 节点
#define XML_NODE_EMMC_PARTITIONS	"Partitions"
#define XML_NODE_EMMC_ENTRY			"Entry"

//ExtCSD 节点
#define XML_NODE_EMMC_EXTCSD		"ExtCSD"
#define XML_NODE_EMMC_PARTITIONSIZE	"PartitionSize"
#define XML_NODE_EMMC_Reg			"Reg"

/// <summary>
/// xxxx.actask 定义节点名称
/// </summary>

#define XML_NODE_TASK			"Task"
#define XML_NODE_TASK_PROJECT	"Project"
#define XML_NODE_CRC16			"TaskFileCRC16"

enum XmlMessageType
{
	XMLMESSAGE_NODE_COMPARE_FAILED = -4,
	XMLMESSAGE_PROJ_EMPTY_FAILED = -3,
	XMLMESSAGE_CRC_FAILED = -2,
	XMLMESSAGE_LOAD_FAILED = -1,
	XMLMESSAGE_SUCCESS = 0,
};