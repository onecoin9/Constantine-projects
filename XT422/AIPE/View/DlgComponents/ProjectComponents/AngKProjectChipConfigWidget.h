#pragma once

#include <QWidget>
#include<QTableWidget>
#include<QGroupBox>
#include<QLabel>
#include<QTextEdit>
#include<QCheckBox>
#include<QComboBox>
#include "XmlDefine.h"
static QString strGroup = "Group";
static QString strLabel = "Label";
static QString strTextEdit = "TextEdit";
static QString strCheckBox = "CheckBox";
static QString strComboBox = "ComboBox";
static QString strTabWidget = "TabPage";

class QVBoxLayout;
class AngKProjectChipConfigWidget : public QWidget
{
	Q_OBJECT

public:
	AngKProjectChipConfigWidget(QWidget *parent = nullptr);
	~AngKProjectChipConfigWidget();

	int CheckCurWgtIndex(QString strCtrl);

	void ClearAllVec();

	QTabWidget* IsSubTabWgt(QString strName);

	//读取XML建立槽函数链接
	int XmlCreateGroupBox(QGroupBox* agb);
	int XmlCreateLabel(QLabel* aLabel);
	int XmlCreateEditText(QTextEdit* aEditText);
	int XmlCreateCheckBox(QCheckBox* ackBox);
	int XmlCreateComboBox(QComboBox* acbBox);
	int XmlCreateTabWidget(QTabWidget* tabWgt);

	void AddSubWidget(QWidget* _subWgt);

	void SetCurrentDevAlgo(QString strDev, uint32_t uAlgo);

private:
	int SaveNodeValue(QString saveFile);
	void ReadNodeValue(QObjectList varObj, pugi::xml_node saveNode);

	int LoadNodeValue(QString loadFile);
	void GetNodeValue(pugi::xml_node sheetXmlNode);
	void SetNodeValue();

public slots:
	void onSlotLoadConfigXML();

	void onSlotSaveConfigXML();
private:
	std::vector<QGroupBox*>			m_vecGroup;
	std::vector<QLabel*>			m_vecLabel;
	std::vector<QTextEdit*>			m_vecEditText;
	std::vector<QCheckBox*>			m_vecCheckBox;
	std::vector<QComboBox*>			m_vecComboBox;
	std::vector<QTabWidget*>		m_vecTabWidget;
	QVBoxLayout*					m_pLayout;
	QString							m_strDevName;
	uint32_t						m_nAlgo;
	std::map<std::string, std::string> m_mapNodeValue;
};
