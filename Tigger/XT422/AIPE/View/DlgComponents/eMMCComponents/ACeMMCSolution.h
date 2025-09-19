#pragma once

#include <QWidget>
#include <QButtonGroup>
#include "eMMCCom.h"
namespace Ui { class ACeMMCSolution; };

class AngKProjDataset;
class ACeMMCSolution : public QWidget
{
	Q_OBJECT

public:
	ACeMMCSolution(QWidget *parent = Q_NULLPTR);
	~ACeMMCSolution();

	void InitText();

	void InitButton();

	void InitComboBox();

	void InitSKTComboBox(bool bDouble);

	int GetFileCount();

	void SetProjectData(AngKProjDataset* _projData);

	std::string GeteMMCOption();

	std::string GeteMMCExtCSDOption();

	bool isExtCSDChecked();

	void SetArchivesFile();

	std::string GetImageFileFormat();

	int StartIntelligentAnalyze(const QString& proj_path, QString& strBytesum);

	void GetEMMCHeaderInfo(const QString& proj_path, eMMCTableHeader& tableHeader, std::string& tableJson, std::string& emmcOptonInfo);
public slots:
	void onSlotImageFileSelect(int);
	void onSlotReadChipExtCSD();
	void onSlotChipIDFetched(std::string resultJson);
	void onSlotExtCSDFetched(std::string resultJson);
signals:
	void sgnFetchedExtCSD(QByteArray resultJson);
private:
	void GetExtCSDInfo(std::string devIP, uint32_t HopNum, uint32_t uExtSize, QString& strDDROffset, uint32_t uCrc16, QString& extLog);
private:;
	Ui::ACeMMCSolution *ui;
	QButtonGroup* m_pButtonGroup;
	AngKProjDataset* m_pProjData;
};
