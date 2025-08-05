#pragma once

#include "ChkSum.h"
#include "eMMCDef.h"
#include <QObject>
#include <iostream>
#include <string>
#include <QSemaphore>
enum ERROR_TYPE {
	E_OK = 0,
	E_NO_ERR,
	E_ALLOC_MEM,
	E_OPEN_FILE,
	E_CANCEL,
	E_FIALED,
};

class ACeMMCAnalyzeManager : public QObject
{
	Q_OBJECT

public:
	ACeMMCAnalyzeManager(QObject *parent);
	~ACeMMCAnalyzeManager();

	int ImgFmtSPBinParserFiles(std::vector<MMAKEFILE>& Files, int ChkSumType, const std::string& strAcMbnFile = "", QWidget* call_wgt = nullptr);

	void SerialvFiletoJson(const QString& proj_path, std::vector<MMAKEFILE>& Files, QString strImgType, std::string& intelligentJson);

	void UnSerialvFiletoJson(const QString& proj_path, std::vector <tSeptBineMMCFileMap>& _binFileInfo, std::string& intelligentJson);
private:
	int calc_crc16sum_checkvirgin(EMMCCRC* pCRC, uint8_t* buf, int size, uint8_t virgin, uint32_t* pIsVirgin);
	bool InitCrcModule(EMMCCRC* pMCrc);
	eMMCVFILE* MallocVFile();
	int CalcVFileFeature(eMMCVFILE* pVFile, std::ifstream& clsFile, uint8_t byVirgin);
	bool PrintFindFile(eMMCVFILE* pVFile);
	int AttachVFile(MMAKEFILE& MFile, eMMCVFILE* pNew);
	int SetProgress(unsigned long long Offset, unsigned long long FileLen);
	std::string GetSumType(int Type);
	bool PrintTotal(MMAKEFILE& MFile, int FileIndex, CHKINFO* pChkInfo, int chkSumType);	
	std::string CCreatFileInfoJson(std::vector<MMAKEFILE>& Files, CHKINFO* pChkInfo);

	QSemaphore* mSemaphore;

signals:
	void updateProgress(int value);

	void sgnCLoseProgressDialog();
};
