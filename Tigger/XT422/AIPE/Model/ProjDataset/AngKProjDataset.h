#pragma once

#include <QObject>
#include "DataJsonSerial.hpp"
#include "ChipOperatorCfgDefine.h"
#include "PropertyDefine.h"
#include "eMMCCom.h"
/*
该类用于保存创建工程时需要外部获取的所有数据集合
	- 芯片数据
	- 档案加载数据
	- eMMC烧录存档路径
	- 操作命令存储
	- BPU烧录座启用值
	- 通用和自定义驱动设置
*/
class AngKDataBuffer;
class AngKProjDataset : public QObject
{
	Q_OBJECT

public:
	AngKProjDataset(QObject *parent = nullptr);
	~AngKProjDataset();

	ChipDataJsonSerial& getChipData() { return m_chipDataJson; };

	void setChipData(ChipDataJsonSerial chipData) { m_chipDataJson = chipData; };

	std::vector<FileDataJsonSerial>& getFileData() { return m_fileDataJsonVec; };

	void setFileData(std::vector<FileDataJsonSerial> fileData) { m_fileDataJsonVec = fileData; };

	std::vector<eMMCFileDataJsonSerial>& geteMMCFileData() { return m_eMMCFileDataJsonVec; };

	void seteMMCFileData(std::vector<eMMCFileDataJsonSerial> efileData) { m_eMMCFileDataJsonVec = efileData; };

	FileImportDataJsonSerial& getFileImportData() { return m_fileImportDataJsonSerial; };

	void setFileImportData(FileImportDataJsonSerial efileData) { m_fileImportDataJsonSerial = efileData; };

	eMMCFileImportDataJsonSerial& geteMMCFileImportData() { return m_eMMCFileImportDataJsonSerial; };

	void seteMMCFileImportData(eMMCFileImportDataJsonSerial efileData) { m_eMMCFileImportDataJsonSerial = efileData; };

	OpInfoList& GetOperInfoList() { return m_operList; }

	void SetOperInfoList(OpInfoList _opInfo) { m_operList = _opInfo; }

	void SetBPUInfo(std::string recvIP, BPUInfo infos) { m_BPUInfos[recvIP] = infos; }

	std::map<std::string, BPUInfo>& GetBPUInfo() { return m_BPUInfos; }

	void SetCommonDrvParaJson(QString comJson) { m_CommonDrvParaJson = comJson; }

	QString& GetCommonDrvParaJson() { return m_CommonDrvParaJson; }

	void SetSelfDrvParaJson(QString comJson) { m_SelfDrvParaJson = comJson; }

	QString& GetSelfDrvParaJson() { return m_SelfDrvParaJson; }

	void SeteMMCExtCSDParaJson(QString comJson) { m_eMMCExtCSDJson = comJson; }

	QString& GeteMMCExtCSDParaJson() { return m_eMMCExtCSDJson; }

	void SeteMMCOptionJson(QString comJson) { m_eMMCOptionJson = comJson; }

	QString& GeteMMCOptionJson() { return m_eMMCOptionJson; }

	void SeteMMCIntelligentJson(QString comJson) { m_eMMCIntelligentJson = comJson; }

	QString& GeteMMCIntelligentJson() { return m_eMMCIntelligentJson; }

	void SeteMMCTableHeaderJson(QString comJson) { m_stuTableHeaderJson = comJson; }

	QString& GeteMMCTableHeaderJson() { return m_stuTableHeaderJson; }

	void SeteMMCeMMCTableHeader(eMMCTableHeader comJson) { m_stuTableHeader = comJson; }

	eMMCTableHeader& GeteMMCTableHeader() { return m_stuTableHeader; }

	stuProjProperty& GetProjProperty() { return m_ProjProperty; }

	void SetProjProperty(stuProjProperty strProjProp) { m_ProjProperty = strProjProp; }

	AngKDataBuffer* GetBuffer() { return m_pDataBuffer; }

	void ClearData();
private:
	ChipDataJsonSerial					m_chipDataJson;	//芯片Json
	std::vector<FileDataJsonSerial>		m_fileDataJsonVec;//文件数据
	FileImportDataJsonSerial			m_fileImportDataJsonSerial;
	eMMCFileImportDataJsonSerial		m_eMMCFileImportDataJsonSerial;
	std::vector<eMMCFileDataJsonSerial>	m_eMMCFileDataJsonVec;//eMMC档案数据
	std::map<std::string, BPUInfo>		m_BPUInfos;// BPU选择信息
	stuProjProperty						m_ProjProperty;// 工程基本属性，是否启用暂定

	OpInfoList	m_operList;				//工程设置烧录命令操作
	QString		m_CommonDrvParaJson;	//通用驱动参数
	QString		m_SelfDrvParaJson;		//自定义驱动参数
	QString		m_eMMCOptionJson;		//emmc配置选项
	QString		m_eMMCExtCSDJson;		//emmc寄存器参数
	QString		m_eMMCIntelligentJson;	//emmc智能分析参数
	eMMCTableHeader	m_stuTableHeader;	//emmc写入SSD的Header数据
	QString		m_stuTableHeaderJson;	//emmc写入SSD的HeaderJson数据

	AngKDataBuffer* m_pDataBuffer;		//工程档案写入Buffer
};
