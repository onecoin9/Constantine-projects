#pragma once

#include <QWidget>
#include <QMap>
#include "GlobalDefine.h"
#include "ChipOperatorCfgDefine.h"
namespace Ui { class AngKProjectOperation; };

/// <summary>
/// 该类用于操作选择UI设置，可对照AP8000的OPTION界面。
/// 命令分为大命令和小命令。大命令分为基本操作命令Program、Erase、Prog、Read、Secure、Verify、Custom
/// 小命令获取方式是通过ChipDataBaseSystem工具添加到json之后保存到本地StuOperatorJson
/// 比如大命令Erase，默认有两个小命令Erase+BlankCheck，执行流为Erase->BlankCheck。
/// 若某IC不支持BlankCheck，所以执行流仅为Erase
/// </summary>

class QComboBox;
class QButtonGroup;
class AngKOperationTag;
class AngKProjDataset;
class AngKProjectOperation : public QWidget
{
	Q_OBJECT

public:
	AngKProjectOperation(QWidget *parent = Q_NULLPTR);
	~AngKProjectOperation();

	//Init
	void InitOperatePath();
	void InitErasePage();
	void InitBlankPage();
	void InitProgramPage();
	void InitVerifyPage();
	void InitSecurePage();
	void InitReadPage();
	void InitSelfPage();
	void InitWidgetInfo();
	void InitButtonGroup();

	//get page flow
	void GetPageFlow(int nPage);
	void GetErasePageFlow();
	void GetBlankPageFlow();
	void GetProgramPageFlow();
	void GetVerifyPageFlow();
	void GetSecurePageFlow();
	void GetReadPageFlow();
	void GetSelfPageFlow();

	void SaveOperationCfg(OpInfoList& operList);

	void CreateOperToolWgt(std::string cfgJson);

	void SetProjDataSet(AngKProjDataset* _progD) { m_pProjDataset = _progD; }

	/// <summary>
	/// 将芯片操作界面的UI转换成Json保存到工程文件中
	/// </summary>
	/// <param name="chipOperJson">引用传参返回Json</param>
	void ChipOper2UI_ProjJson(std::string& chipOperJson);

	/// <summary>
	/// 将工程文件中的Json转换成操作界面的UI
	/// </summary>
	/// <param name="chipOperJson">芯片数据Json</param>
	void UI2ChipOper_ProjJson(std::string chipOperJson);

	static std::string TranslateProgOperType(OperationTagType operTagType);
private:

	void DynamicChangeOperPage();

	AngKOperationTag* GetOperationTagByType(OperationTagType operTagType);
public slots:
	void onSlotChangeWidget(int);
	void onSlotDropSymbolType(QObject*, int);
private:
	Ui::AngKProjectOperation *ui;
	QButtonGroup*						m_pageGroup;
	std::map<OperationTagType, QString>	m_mapOperatePath;
	QMap<QComboBox*, AngKOperationTag*>	m_box2Widget;
	QMap<AngKOperationTag*, QComboBox*>	m_widget2Box;
	StuOperatorJson	m_stuOperJson;
	AngKProjDataset* m_pProjDataset;
};
