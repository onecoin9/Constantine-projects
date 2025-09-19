#include "AngKTabProject.h"
#include "ui_AngKTabProject.h"
#include "../View/GlobalInit/StyleInit.h"
#include "AngKChipDialog.h"
#include "AngKFilesImport.h"
#include "AngKTransmitSignals.h"
#include <QGraphicsDropShadowEffect>
#include <QFileDialog>
#include "AngKProjFile.h"
#include "AngKCheckBoxHeader.h"
AngKTabProject::AngKTabProject(QWidget *parent)
	: QWidget(parent)
	, m_projDataset(nullptr)
	, m_pStrManager(nullptr)
	, m_pVarManager(nullptr)
	, m_pVarFactory(nullptr)
	, m_pLineFactory(nullptr)
	, m_pTaskPropertyManager(nullptr)
	, m_pCcomboBoxFactory(nullptr)
	, m_pStrProjFileManager(nullptr)
{
	ui = new Ui::AngKTabProject();
	ui->setupUi(this);

	InitProjPropetry();

	this->setObjectName("AngKTabProject"); 
	QT_SET_STYLE_SHEET(objectName());
}

AngKTabProject::~AngKTabProject()
{
	if (m_pLineFactory)
	{
		m_pLineFactory = nullptr;
		delete m_pLineFactory;
	}
	if (m_pVarFactory)
	{
		m_pVarFactory = nullptr;
		delete m_pVarFactory;
	}
	if (m_pVarManager)
	{
		m_pVarManager = nullptr;
		delete m_pVarManager;
	}
	if (m_pStrManager)
	{
		m_pStrManager = nullptr;
		delete m_pStrManager;
	}
	if (m_pStrProjFileManager)
	{
		m_pStrProjFileManager = nullptr;
		delete m_pStrProjFileManager;
	}
	delete ui;
}

void AngKTabProject::InitProjPropetry()
{
	ui->ProjectProperty->setHeaderVisible(true);
	QTreeView* treeView = ui->ProjectProperty->findChild<QTreeView*>();
	//if (treeView) {
	//	// 设置自定义工具提示代理  
	//	treeView->header()->setSectionsClickable(true); // 允许点击列标题调整大小 
	//	treeView->header()->setSectionResizeMode(QHeaderView::Interactive);
	//	treeView->header()->resizeSection(0, 150);
	//	treeView->setStyleSheet(
	//		"QTreeView::item { border-right: 1px solid #d8d8d8; }"
	//		"QTreeView::item { color: black; } "
	//		"QTreeView::item:selected { color: black; background-color: transparent; } ");
	//	treeView->setItemDelegate(new ToolTipDelegate(treeView));
	//}  

	QAbstractItemModel* model = treeView->model();
	if (model) {
		model->setHeaderData(0, Qt::Horizontal, tr("Property")); // 设置第一列表头  
		model->setHeaderData(1, Qt::Horizontal, tr("Value")); // 设置第二列表头  
	}

	ui->ProjectProperty->clear();
	m_pVarManager = new QtVariantPropertyManager(ui->ProjectProperty);
	m_pVarFactory = new QtVariantEditorFactory(ui->ProjectProperty);

	m_pStrManager = new QtStringPropertyManager(ui->ProjectProperty);
	m_pLineFactory = new QtLineEditFactory(ui->ProjectProperty);

	m_pTaskPropertyManager = new QtEnumPropertyManager(ui->ProjectProperty);
	m_pCcomboBoxFactory = new QtEnumEditorFactory(ui->ProjectProperty);

	m_pStrProjFileManager = new QtStringPropertyManager(ui->ProjectProperty);
	//工程属性
	QtProperty* projGroup = m_pVarManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("TaskInfo"));
	//Project 添加子属性
	QtProperty* item = m_pStrManager->addProperty(tr("Name"));
	m_pStrManager->setReadOnly(item, true);
	projGroup->addSubProperty(item);

	item = m_pTaskPropertyManager->addProperty(tr("ProgjectList"));
	//QStringList enumNames;
	//enumNames << "Proj1" << "Proj2" << "Proj3" << "Proj4";
	//m_pTaskManager->setEnumNames(item, enumNames);
	projGroup->addSubProperty(item);

	ui->ProjectProperty->addProperty(projGroup);

	QtProperty* projInfoGroup = m_pVarManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("ProjectInfo"));
	item = m_pStrManager->addProperty(tr("Chip"));
	m_pStrManager->setReadOnly(item, true);
	projInfoGroup->addSubProperty(item);

	item = m_pStrManager->addProperty(tr("Adapter"));
	m_pStrManager->setReadOnly(item, true);
	projInfoGroup->addSubProperty(item);



	item = m_pStrManager->addProperty(tr("Type"));
	m_pStrManager->setReadOnly(item, true);
	projInfoGroup->addSubProperty(item);

	item = m_pStrManager->addProperty(tr("Manufacturer"));
	m_pStrManager->setReadOnly(item, true);
	projInfoGroup->addSubProperty(item);

	item = m_pStrManager->addProperty(tr("Package"));
	m_pStrManager->setReadOnly(item, true);
	projInfoGroup->addSubProperty(item);


	item = m_pStrManager->addProperty(tr("DevDriverVersion"));
	m_pStrManager->setReadOnly(item, true);
	projInfoGroup->addSubProperty(item);


	item = m_pStrManager->addProperty(tr("MstDriverVersion"));
	m_pStrManager->setReadOnly(item, true);
	projInfoGroup->addSubProperty(item);


	item = m_pStrManager->addProperty(tr("StdChecksum"));
	m_pStrManager->setReadOnly(item, true);
	projInfoGroup->addSubProperty(item);

	item = m_pStrManager->addProperty(tr("ExtChecksum"));
	m_pStrManager->setReadOnly(item, true);
	projInfoGroup->addSubProperty(item);

	ui->ProjectProperty->addProperty(projInfoGroup);

	QtProperty* subProjFileGroup = m_pVarManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("ProjectFile"));

	ui->ProjectProperty->addProperty(subProjFileGroup);


	connect(m_pVarManager, SIGNAL(valueChanged(QtProperty*, const QVariant&)), this, SLOT(onSlotVariantPropertyValueChanged(QtProperty*, const QVariant&)));
	connect(m_pTaskPropertyManager, SIGNAL(valueChanged(QtProperty*, int)), this, SLOT(onSlotProjIndexValueChanged(QtProperty*, int)));
	connect(ui->ProjectProperty, SIGNAL(currentItemChanged(QtBrowserItem*)), this, SLOT(onSlotCurrentItemChanged(QtBrowserItem*)));
	ui->ProjectProperty->setFactoryForManager(m_pVarManager, m_pVarFactory);
	ui->ProjectProperty->setFactoryForManager(m_pStrManager, m_pLineFactory);
	ui->ProjectProperty->setFactoryForManager(m_pTaskPropertyManager, m_pCcomboBoxFactory);

	connect(this, &AngKTabProject::sgnProperty2Mainframe, &AngKTransmitSignals::GetInstance(), &AngKTransmitSignals::sgnProperty2Mainframe);
}

void AngKTabProject::SetProjPariInfo(QMap<QString, QPair<QString, ACProjManager*>> _projPairInfo)
{
	m_mapProjPair = _projPairInfo;
}

void AngKTabProject::UpdateProjUI()
{
	QStringList projList;
	for (auto& projInfo : m_mapProjPair.toStdMap()) {
		stuProjProperty& sProp = (projInfo.second).second->GetProjData()->GetProjProperty();
		projList << QString::fromStdString(sProp.name);
	}

	//先更新工程列表
	QSet<QtProperty*> taskPpropertyList = m_pTaskPropertyManager->properties();

	for (auto vat : taskPpropertyList){
		if (vat->propertyName() == tr("ProgjectList")) {
			m_pTaskPropertyManager->setEnumNames(vat, projList);
		}
	}

	if (projList.empty()) {
		return;
	}

	//更新其他工程属性值
	//ChangeProjInfo(0);
}

void AngKTabProject::ChangeProjInfo(QString propText, int nIndex)
{
	m_pStrProjFileManager->clear();
	for (auto& projInfo : m_mapProjPair.toStdMap()) {
		stuProjProperty& sProp = (projInfo.second).second->GetProjData()->GetProjProperty();

		if(propText.toStdString() != sProp.name)
			continue;

		QSet<QtProperty*> varPropertyList = m_pVarManager->properties();
		for (auto vat : varPropertyList)
		{
			if (vat->propertyName() == tr("ProjectFile")) {
				for (int i = 0; i < sProp.binFile.size(); ++i) {
					QtProperty* item = m_pStrProjFileManager->addProperty(tr("User File[%1]").arg(i));
					m_pStrProjFileManager->setReadOnly(item, true);
					vat->addSubProperty(item);
					m_pStrProjFileManager->setValue(item, sProp.binFile[i].c_str());
					item->setToolTip(sProp.binFile[i].c_str());
				}
			}
		}

		QSet<QtProperty*> strPropertyList = m_pStrManager->properties();
		for (auto vat : strPropertyList)
		{
			if (vat->propertyName() == tr("Type")) {
				m_pStrManager->setValue(vat, QString::fromStdString(sProp.typeName));
				vat->setToolTip(QString::fromStdString(sProp.typeName));
			}
			else if (vat->propertyName() == tr("Manufacturer")) {
				m_pStrManager->setValue(vat, QString::fromStdString(sProp.manufacturerName));
				vat->setToolTip(QString::fromStdString(sProp.manufacturerName));
			}
			else if (vat->propertyName() == tr("Package")) {
				m_pStrManager->setValue(vat, QString::fromStdString(sProp.packageName));
				vat->setToolTip(QString::fromStdString(sProp.packageName));
			}
			else if (vat->propertyName() == tr("Name")) {
				QString projPath = QString::fromStdString(sProp.name);
				QFileInfo projFile(projPath);
				std::string projName = Utils::AngKCommonTools::RemoveExtension(projFile.fileName(), ".eapr");
				m_pStrManager->setValue(vat, QString::fromStdString(projName));
				vat->setToolTip(QString::fromStdString(projName));
			}
			else if (vat->propertyName() == tr("Chip")) {
				m_pStrManager->setValue(vat, QString::fromStdString(sProp.chipName));
				vat->setToolTip(QString::fromStdString(sProp.chipName));
			}
			else if (vat->propertyName() == tr("Adapter")) {
				m_pStrManager->setValue(vat, QString::fromStdString(sProp.adapterName));
				vat->setToolTip(QString::fromStdString(sProp.adapterName));
			}
			else if (vat->propertyName() == tr("StdChecksum")) {
				m_pStrManager->setValue(vat, QString::fromStdString(sProp.checksum));
				vat->setToolTip(QString::fromStdString(sProp.checksum));
			}
			else if (vat->propertyName() == tr("ExtChecksum")) {
				m_pStrManager->setValue(vat, "0x00000000");
				vat->setToolTip(QString::fromStdString(sProp.checksum));
			}
			else if (vat->propertyName() == tr("DevDriverVersion")) {
				m_pStrManager->setValue(vat, QString::fromStdString(sProp.devDriverVer));
				vat->setToolTip(QString::fromStdString(sProp.devDriverVer));
			}
			else if (vat->propertyName() == tr("MstDriverVersion")) {
				m_pStrManager->setValue(vat, QString::fromStdString(sProp.mstDriverVer));
				vat->setToolTip(QString::fromStdString(sProp.mstDriverVer));
			}
		}
	}
	emit sgnProperty2Mainframe(propText, nIndex);
}

void AngKTabProject::onSlotSelectChipDataJson(ChipDataJsonSerial chipJson)
{
	emit sgnAcceptChipDlg();
}

void AngKTabProject::onSlotSelectFile()
{
	AngKFilesImport fileDlg(this);
	connect(&fileDlg, &AngKFilesImport::sgnSelectFileInfo, this, [=](FileDataInfo fInfo, bool isDoubleClicked) {
		//ui->fileLineEdit->setText(QString::fromStdString(fInfo.filePathStr));
		//ui->checkLineEdit->setText(QString::fromStdString(fInfo.fileCheckStr));
	});

	fileDlg.exec();
}

void AngKTabProject::onSlotSaveProj()
{
	QString filePath = QFileDialog::getSaveFileName(this, "Save As...", QCoreApplication::applicationDirPath(), tr("All Files(*.*)"));

	AngKProjFile pFile(this, 0);

	pFile.CreateProjFile(filePath, 30, 0, 0, "test", "1000", "asdf12");
	
	//pFile.LoadFile(filePath);
	pFile.CloseFile();
}

void AngKTabProject::onSlotVariantPropertyValueChanged(QtProperty* changeProperty, const QVariant& changeVar)
{
	
}

void AngKTabProject::onSlotCurrentItemChanged(QtBrowserItem* pClickItem)
{
	//几个特殊属性点击之后要进行弹框选择
}

void AngKTabProject::onSlotProjIndexValueChanged(QtProperty* changeProperty, int nIndex)
{
	//只有一个属性用了TaskManager，不做判断区分, 更新其他工程属性值
	if (changeProperty->propertyName() == tr("ProgjectList")) {
		ChangeProjInfo(changeProperty->valueText(), nIndex);
		changeProperty->setToolTip(changeProperty->valueText());
	}
	
	if (changeProperty->propertyName() == tr("File")) {
		changeProperty->setToolTip(changeProperty->valueText());
	}
}

void AngKTabProject::onSlotSelectProjFile()
{
	QString filePath = QFileDialog::getOpenFileName(this, "Save As...", QCoreApplication::applicationDirPath(), tr("bin Files(*.aprs);; All Files(*.*)"));
	//ui->addressLineEdit->setText(filePath);

	//ui->nameLineEdit->setText(filePath.mid(filePath.lastIndexOf("/") + 1));
}

void AngKTabProject::onSlotSelectChip()
{
	AngKChipDialog chipDlg(this);
	chipDlg.SetTitle("Select Chip");
	chipDlg.InitChipData();
	connect(&AngKTransmitSignals::GetInstance(), &AngKTransmitSignals::sgnSelectChipDataJson, this, &AngKTabProject::onSlotSelectChipDataJson);
	connect(this, &AngKTabProject::sgnAcceptChipDlg, &chipDlg, &AngKChipDialog::accept);
	chipDlg.exec();
	disconnect(&AngKTransmitSignals::GetInstance(), &AngKTransmitSignals::sgnSelectChipDataJson, this, &AngKTabProject::onSlotSelectChipDataJson);
	disconnect(this, &AngKTabProject::sgnAcceptChipDlg, &chipDlg, &AngKChipDialog::accept);

}