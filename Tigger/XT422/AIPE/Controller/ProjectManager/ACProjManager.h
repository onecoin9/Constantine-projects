#pragma once

#include <QObject>
#include <memory>
#include "AngKProjDataset.h"
#include "AngKDataBuffer.h"
#include "AngKProjFile.h"
#include "ACeMMCAnalyzeManager.h"
#include "DeviceModel.h"
#include "eMMCCom.h"

enum eMMCImageFileType
{
	emmc_BinFiles = 0,
	emmc_MTK,
	emmc_Ambarella,
	emmc_Phison,
	emmc_CIMG,
	emmc_Huawei,
	emmc_ACIMG
};

class ACProjManager : public QObject
{
	Q_OBJECT

public:
	ACProjManager(QObject *parent, AngKDataBuffer* _pDataBuf = nullptr);
	~ACProjManager();

	void InitManager();

	int SaveProject(QString& _projPath, QString& _projPwd, QString& _chkFile, int _nClearBufferType, std::string& _ImageFileFormat
		, std::string& _eMMCOption, std::string& _eMMCExtCSD, std::string& _eMMCSSDHeaderData, eMMCTableHeader& _eMMCSSDTableHeader
		, std::string& _drvCommon, std::string& _drvSelf, std::string& _chipOperate, std::string& _chipBufferChk, std::string& _description);

	int ImportProject(QString _projPath);

	void ErrorClearFile();

	AngKProjDataset* GetProjData();

	AngKDataBuffer* GetProjDataBuffer();

	AngKProjFile* GetProjFilePtr();

	QString GetProjDescription();

	void SetProjName(QString _sProjName);

	QString& GetProjName();

	void SwitchOperInfoSave();

	int LoadeMMCFile(QString strIP, int nHop);

	bool CheckEmmcFile(QString strIP, int nHop);

	int LoadSeparatedBinFiles(QString strIP, int nHop, std::vector<eMMCFileDataJsonSerial>& _efileDataJson);

	QByteArray GetBufferCheckBPUAttribute();

	bool CheckSSDTaskDownload(DeviceStu& devInfo, std::vector<tSeptBineMMCFileMap>& vecBinFile);

	void ClearEmmcHeaderData(QString strIP, int nHop, std::string& devSN);

	int CheckAllFileChkSum();

	QString GetEMMCExtCSDParaJson() { return m_pProjDataset->GeteMMCExtCSDParaJson(); };
	QString GetProjTableHeaderJson() { return m_sTableHeaderJsonStr; };
	uint64_t CalcBinFilesSize();
	std::vector<tSeptBineMMCFileMap> GetBinFileMapInfoVec();

signals:
	void sgnUpdateChkProgress(double);

	void sgnUpdateFileDownloadProgress(int, QString, int);
private:
	void CalBinFilesByteSum(std::vector<tSeptBineMMCFileMap>& vecBinFile, nlohmann::json& _bytesumJson);

	//打印工程函数
	void PrintOpenProjLog(QString _fPath);
	void PrintExtCSDLog(std::string& extcsdJson);


	void GetPartitionSize(QString partName, QString partSize, QString UnitSize, eMMCOPTION_Modify& csdInfo);
	void GetExtCSDReg(QString regAddr, QString regValue, QString regName, eMMCOPTION_Modify& csdInfo);
private:
	std::unique_ptr<AngKProjDataset>	m_pProjDataset;
	std::unique_ptr<AngKProjFile>		m_pProjFile;
	std::unique_ptr<AngKDataBuffer>		m_pDataBuffer;
	std::unique_ptr<ACeMMCAnalyzeManager>	m_pAnalyzeMgr;
	QString	m_sProjUUID;
	QString m_sProjName;

	QString m_sTableHeaderJsonStr;

	//key: chkType value:待校验的文件和比对校验值
	QMap<int, QMap<QString, uint64_t>>	m_mapReadyCheckFile;
};
