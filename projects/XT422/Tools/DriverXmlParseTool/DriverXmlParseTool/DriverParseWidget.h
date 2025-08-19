#pragma once

#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QWidget>
#include "CommonControl/ComTool.h"
#include "Depend/tinyxml2/tinyxml2.h"
#include "GlobalDefine.h"
#include "qtpropertymanager.h"
#include "qtvariantproperty.h"
#include "ViewWidget.h"
namespace Ui { class DriverParseWidget; };

using namespace tinyxml2;

class QtProperty;
class QtVariantProperty;
class AGroupBox;
class ALabel;
class AEditText;
class ACheckBox;
class AComboBox;
class ATabWidget;

class DriverParseWidget : public QWidget
{
	Q_OBJECT

public:
	DriverParseWidget(QWidget *parent = Q_NULLPTR);
	~DriverParseWidget();

	void InitEditWgt();

	void InitPropetryWgt();

	void InitChipTreeWgt();

	//创建XML文件
	int CreateXML(QString strPath);

	void CreateChildElement(QObjectList oList, XMLElement* element, XMLDocument* doc);

	void CreateXML2UI(XMLElement* element, QWidget* objParent, ViewWidget& vWgt, bool bView = false);

	//编辑属性修改
	void ModifyLocationPropetry(const QVariant& vari);

	QString CreateNewFile();

	void ClearWgt();

	bool CheckStrHex(QString& strHex);

	void CreateXMLDefaultHead();
signals:
	void sgnWindowTitle(QString);

public slots:
	void OnSlotShowPropetry(QObject*);
	void OnSlotClearPropetry(QObject*);
	void variantPropertyValueChanged(QtProperty*, const QVariant&);

	void OnSlotAddTreeItem();
	void OnSlotDelTreeItem();
	void OnSlotItemClicked(QTreeWidgetItem*, int);
	void OnSlotItemDoubleClicked(QTreeWidgetItem*, int);

	void OnSlotSaveUI2XML();
	void OnSlotOpenViewWgt();

	//菜单栏操作相关
	void OnSlotNewFile();
	void OnSlotLoadFile();
	void OnSlotClearCurrentEditWgt();
private:
	Ui::DriverParseWidget *ui;
	QObject*		m_curShowPropetry;
	int				m_curSelItemCol;
	QString			m_curOperFile;
	tinyxml2::XMLDocument*	m_curXmlDoc;
	std::map<QTreeWidgetItem*, XMLElement*> m_mapAlgoElement;
};