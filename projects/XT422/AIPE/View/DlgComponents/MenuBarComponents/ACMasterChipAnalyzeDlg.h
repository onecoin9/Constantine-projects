#pragma once

#include "AngKDialog.h"
#include "ACMasterChipAnalyzeManager.h"
#include "json.hpp"
namespace Ui { class ACMasterChipAnalyzeDlg; };

class QListWidget;
class ACMasterChipAnalyzeDlg : public AngKDialog
{
	Q_OBJECT

public:
	ACMasterChipAnalyzeDlg(QWidget *parent = Q_NULLPTR);
	~ACMasterChipAnalyzeDlg();

	void InitText();

	void InitButton();

	void InitComboBox();

	void SetACXMLPathEnable(bool bEnable);

	void SetAnalyzeLogText(QString text);

	int SaveACIMGFile(uint64_t nTotalBlkNum);

	void RecordPrintLog(QString text, QString fromDev = "--", Utils::AngkLogger::LogLevel logLevel = Utils::AngkLogger::LogLevel::INFO);

	void PrintBasicInfo(std::string& devSN, std::string& devAlias);

	void PrintEntryInfo(nlohmann::json& vectorInfo);

	void PrintPartitionInfo();

	void PrintResultInfo(nlohmann::json& partitionJson, QString& TotalBlkNum, QString& TotalBlkBytesum);

	void SetAnalyzePartitionCount();

	void ResetAnalyzeInfo();

	void PrintAnalyzeTime(QString analyzeLabel);
signals:
	void sgnSetAnalyzeFlag(bool);

	void sgnFinishAnalyze(int);

	void sgnACImageCRCCheckProgress(int, bool);

	void sgnPrintCRCMessage(QString);
public slots:
	void onSlotExecuteChipAnaylze();

	void onSlotStopChipAnalyze();

	void onSlotSelectACxmlPath();

	/// <summary>
	/// 母片分析进度
	/// </summary>
	/// <param name="nProgress">进度值</param>
	void onSlotGetProgress(std::string, int, int);

	/// <summary>
	/// 母片分析进展日志上报
	/// </summary>
	/// <param name="strMsg">上报消息</param>
	void onSlotGetChipAnalyzeInfo(QString);

	void onSlotMasterChipAnalyzeResult(uint32_t, std::string);

	void onSlotHandleEventAnalyzeResult(std::string);

	void onSlotHandleEventExtCSDFetched(std::string);

	void onSlotHandleEventAnalyzeInfo(std::string);

	void onSlotHandleEventAnalyzeStatusChange(std::string);

	void onSlotHandleEventUIDFetched(std::string);

	void onSlotHandleEventChipIDFetched(std::string);

	void onSlotUpdateTime();

	void onSlotFinishAnalyze(int);

	void onSlotACImageCRCCheckProgress(int, bool);

	void onSlotPrintCRCMessage(QString);
private:
	Ui::ACMasterChipAnalyzeDlg* ui;
	std::unique_ptr<QListWidget>	m_pListWidget;
	std::unique_ptr<ACMasterChipAnalyzeManager>	m_pChipAnalyzeManager;
	std::unique_ptr<QTimer> m_pTtimer;
	QString	m_strRecordPath;
	QString m_strACIMGPath;
	QString m_strACXMLPath;
	QString m_strDevIP;
	QString m_curAnalyzePartition;
	int m_nDevHop;
	int elapsedTime; // 记录经过的秒数
	bool bFirstAnalyze;
	int m_RecordPartitionVFileCount;	//每统计一个分区开始计数个数，收到分区结束后打印清空
	int m_RecordPartitionVFileSum;	//每统计一个分区开始计数bytesum，收到分区结束后打印清空
	int m_nCountPartition;
	float m_fCurrentTotalProgress;
};
