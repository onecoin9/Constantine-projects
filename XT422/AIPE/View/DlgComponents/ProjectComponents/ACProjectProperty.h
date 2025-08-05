#pragma once

#include <QWidget>
#include "ACProjManager.h"
#include "AngKFilesImport.h"
#include "AngKAdvanceSetting.h"
namespace Ui { class ACProjectProperty; };

class QStandardItemModel;
class ACProjectProperty : public QWidget
{
	Q_OBJECT

	enum propertyPage {
		ChipPage = 0,
		FilePage,
		ChipConfigPage,
		DrvParaPage,
		BufferChecksumPage,
		OperationPage,
		SNCPage,
		OtherPage,
		ALLNUM
	};
public:
	ACProjectProperty(QWidget *parent, int nIndex, QString projName, AngKDataBuffer* pDataBuf);
	~ACProjectProperty();

	/// <summary>
	/// 初始化文本
	/// </summary>
	void InitText();

	/// <summary>
	/// 初始化基本按钮
	/// </summary>
	void InitCommonButton();

	/// <summary>
	/// 初始化向导配置页
	/// </summary>
	void InitPage();

	/// <summary>
	/// 检查页面当前页面是否填写正确
	/// </summary>
	bool CheckPageEvent();

	/// <summary>
	/// 切换向导页
	/// </summary>
	void ChangeTurnPageButton();

	void InitOperInfoList(OpInfoList& InfoList);

	void SetProjName(QString _projName);

	void MasterChipAnalyzeCreateEapr();
private:
	void InitChipPage();

	void InitFilePage();

	void InitDrvParaPage();

	void InitCheckBufferPage();

	void InitChipConfigPage();

	void InitOperationPage(std::string cfgJson);

	void InitOtherPage();

	void InsertData(FileDataInfo& fileInfo);

	void ReInsertData(FileDataInfo& fileInfo);

	void FileWriteBuffer();

	void LoadFileBuffer();

	int SwapBuffer(int swapType, ADR _writeAdrMin, ADR _writeAdrMax);

	bool CheckFilesLoadAddressOverlap();

	bool CheckFileTag(std::vector<FileTagInfo>& fileTagVec);

	void ShowFileTag();

	void PrintSaveLog();

	void PrintOpenLog();

	/// <summary>
	/// 判断芯片类型，加载当前工程文件中的档案数据
	/// </summary>
	void SetArchivesFileInfo();

	/// <summary>
	/// 根据打开的工程文件设置BufferCheck页面
	/// </summary>
	void SetBufChecksumPage();

	int ParserChipConfigXML(QString chipXml, uint32_t nAlgo);

	std::string GetChipBufferChkConfig();

	void CreateXML2UI(pugi::xml_node& element, QWidget* objParent);

	int IntelligentAnalyze(const QString& proj_path, QString& strBytesum);

	/// <summary>
	/// 工程各页面中触发事件
	/// </summary>
	bool ChipPageEvent();
	bool FilePageEvent();
	bool ChipConfigPageEvent();
	bool DrvParaPageEvent();
	bool BufferChecksumPageEvent();
	bool OperationPageEvent();
	bool SNCPageEvent();
	bool OtherPageEvent();

signals:
	void sgnRenameClick();

	void sgnSaveSuccessClose();

	void sgnImportProjName(QString);
public slots:
	void onSlotImportProject();

	void onSlotSaveProject();

	void onSlotTurnPage();

	void onSlotUpdateACXMLChipID(std::string acxmlChipID);

	void onSlotDoubleSetFileLoad(const QModelIndex&);

	void onSlotFileSelect();

	void onSlotDriverParamConfig();
private:
	Ui::ACProjectProperty* ui;
	ADVSETTING	m_AdvSetting;
	std::unique_ptr<ACProjManager> m_pProjManager;
	std::unique_ptr<AngKFilesImport> m_pFileImportDlg;
	std::unique_ptr<QStandardItemModel> m_pFileTableModel;
	std::unique_ptr <AngKAdvanceSetting> m_pAdvSettingDlg;
	int	m_pCurrentIndex;
	int	m_nBPUAttribute;
	QString m_strProjName;
	QString m_strRecordProjPath;
};
