#include "AngKTabProgram.h"
#include "ui_AngKTabProgram.h"
#include "../View/GlobalInit/StyleInit.h"
#include "AngKGlobalInstance.h"
#include "GlobalDefine.h"
#include <QAction>
#include <QIcon>
#include <QPainter>
#include <QSvgRenderer>
#include <QTreeView>
#include <qheaderview.h>
#include <QStyleFactory>
#include "AngKCheckBoxHeader.h"

AngKTabProgram::AngKTabProgram(QWidget *parent)
	: QWidget(parent)
	, m_pStrManager(nullptr)
	, m_pVarManager(nullptr)
	, m_pVarFactory(nullptr)
	, m_pLineFactory(nullptr)
{
	ui = new Ui::AngKTabProgram();
	ui->setupUi(this);

	InitLabelText();
	InitEditState();
	InitProgPropetry();

	this->setObjectName("AngKTabProgram");
	QT_SET_STYLE_SHEET(objectName());
}

AngKTabProgram::~AngKTabProgram()
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
	delete ui;
}

void AngKTabProgram::InitLabelText()
{
	ui->SplitBarWgt->hide();
	ui->SplitBarWgt2->hide();

	ui->screenLabel->setText(tr("Screen"));
	ui->powerLabel->setText(tr("Power"));
}

void AngKTabProgram::InitEditState()
{
	//ui->snLineEdit->setReadOnly(true);
	//ui->modeLineEdit->setReadOnly(true);
	//ui->VisionLineEdit->setReadOnly(true);
	//ui->ipaddrEdit->setReadOnly(true);
	//ui->ipPortLineEdit->setReadOnly(true);
	//ui->screenProviderLineEdit->setReadOnly(true);
	//ui->screenBatchIDLineEdit->setReadOnly(true);
	//ui->powerProviderLineEdit->setReadOnly(true);
	//ui->powerBatchIDLineEdit->setReadOnly(true);

	//设置Icon
	QSvgRenderer render;
	bool ok = render.load(Utils::AngKPathResolve::localRelativeSkinPath() + "Common/usbImage.svg");
	QSize size = render.defaultSize();
	QPixmap* pix = new QPixmap(25, 25);
	pix->fill(Qt::transparent);
	QPainter painter(pix);
	painter.setRenderHints(QPainter::Antialiasing);
	render.render(&painter);
	QIcon icon(*pix);
	//QAction* pActLeft = new QAction(ui->modeLineEdit);
	//pActLeft->setIcon(icon);
	//ui->modeLineEdit->addAction(pActLeft, QLineEdit::LeadingPosition);

	//ui->modeLineEdit->setText("test USB");
}

void AngKTabProgram::InitProgPropetry()
{

	ui->CommonProperty->setHeaderVisible(true);
	QTreeView* treeView = ui->CommonProperty->findChild<QTreeView*>();
	if (treeView) {
		// 设置自定义工具提示代理  
		treeView->header()->setSectionsClickable(true); // 允许点击列标题调整大小 
		treeView->header()->setSectionResizeMode(QHeaderView::Interactive);
		treeView->header()->resizeSection(0, 150);
		treeView->setStyleSheet(
			"QTreeView::item { border-right: 1px solid #d8d8d8; }"
			"QTreeView::item { color: black; } "
			"QTreeView::item:selected { color: black; background-color: transparent; } ");
		treeView->setItemDelegate(new ToolTipDelegate(treeView));


		QAbstractItemModel* model = treeView->model();
		if (model) {
			model->setHeaderData(0, Qt::Horizontal, tr("Property")); // 设置第一列表头  
			model->setHeaderData(1, Qt::Horizontal, tr("Value")); // 设置第二列表头  
		}

	}


	ui->CommonProperty->clear();
	m_pVarManager = new QtVariantPropertyManager(ui->CommonProperty);
	m_pVarFactory = new QtVariantEditorFactory(ui->CommonProperty);

	m_pStrManager = new QtStringPropertyManager(ui->CommonProperty);
	m_pLineFactory = new QtLineEditFactory(ui->CommonProperty);

	//ProgramInfo AG06级联属性
	QtProperty* progGroup = m_pVarManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("ProgramInfo"));
	//Project 添加子属性
	QtProperty* item = m_pVarManager->addProperty(QVariant::String, tr("Name"));
	progGroup->addSubProperty(item);

	item = m_pStrManager->addProperty(tr("SN"));
	m_pStrManager->setReadOnly(item, true);
	progGroup->addSubProperty(item);

	item = m_pStrManager->addProperty(tr("CommMode"));
	m_pStrManager->setReadOnly(item, true);
	progGroup->addSubProperty(item);

	item = m_pStrManager->addProperty(tr("Vision"));
	m_pStrManager->setReadOnly(item, true);
	progGroup->addSubProperty(item);

	item = m_pStrManager->addProperty(tr("IPAddr"));
	m_pStrManager->setReadOnly(item, true);
	progGroup->addSubProperty(item);

	item = m_pStrManager->addProperty(tr("IPPort"));
	m_pStrManager->setReadOnly(item, true);
	progGroup->addSubProperty(item);

	ui->CommonProperty->addProperty(progGroup);

	//Screen 属性
	QtProperty* screenGroup = m_pVarManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Screen"));
	item = m_pStrManager->addProperty(tr("Provider"));
	m_pStrManager->setReadOnly(item, true);
	screenGroup->addSubProperty(item);

	item = m_pStrManager->addProperty(tr("BatchID"));
	m_pStrManager->setReadOnly(item, true);
	screenGroup->addSubProperty(item);

	ui->CommonProperty->addProperty(screenGroup);

	//Power 属性
	QtProperty* powerGroup = m_pVarManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Power"));
	item = m_pStrManager->addProperty(tr("Provider"));
	m_pStrManager->setReadOnly(item, true);
	powerGroup->addSubProperty(item);

	item = m_pStrManager->addProperty(tr("BatchID"));
	m_pStrManager->setReadOnly(item, true);
	powerGroup->addSubProperty(item);

	ui->CommonProperty->addProperty(powerGroup);

	ui->CommonProperty->setFactoryForManager(m_pVarManager, m_pVarFactory);
	ui->CommonProperty->setFactoryForManager(m_pStrManager, m_pLineFactory);
}
