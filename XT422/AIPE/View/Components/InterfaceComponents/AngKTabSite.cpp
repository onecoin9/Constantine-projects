#include "AngKTabSite.h"
#include "ui_AngKTabSite.h"
#include "../View/GlobalInit/StyleInit.h"
#include <QMouseEvent>
#include "AngKCheckBoxHeader.h"

AngKTabSite::AngKTabSite(QWidget *parent)
	: QWidget(parent)
	, m_pStrManager(nullptr)
	, m_pVarManager(nullptr)
	, m_pVarFactory(nullptr)
	, m_pLineFactory(nullptr)
{
	ui = new Ui::AngKTabSite();
	ui->setupUi(this);

	InitLabelText();
	InitEditState();
	InitProgPropetry();

	ui->moreInfoLabel->setAttribute(Qt::WA_Hover, true);//开启悬停事件
	ui->moreInfoLabel->installEventFilter(this);       //安装事件过滤器

	this->setObjectName("AngKTabSite");
	QT_SET_STYLE_SHEET(objectName());
}

AngKTabSite::~AngKTabSite()
{
	delete ui;
}

void AngKTabSite::InitLabelText()
{
	ui->visionLabel->setText(tr("Vision"));
	ui->passCntLabel->setText(tr("Pass Cnt"));
	ui->failCntLabel->setText(tr("Fail Cnt"));
	ui->moreInfoLabel->setText(tr("More Info"));
	ui->socketLabel->setText(tr("Socket"));
	//ui->nameLabel->setText(tr("Name"));
	//ui->snLabel->setText(tr("SN"));
	//ui->dateLabel->setText(tr("Date"));
	//ui->lifeCycleLabel->setText(tr("LifeCycle"));
	//ui->usedCntLabel->setText(tr("Used Cnt"));
	//ui->socketFailCntLabel->setText(tr("Fail Cnt"));
}

void AngKTabSite::InitEditState()
{
	ui->visionLineEdit->setReadOnly(true);
	ui->passCntLineEdit->setReadOnly(true);
	ui->failCntLineEdit->setReadOnly(true);
	//ui->snLineEdit->setReadOnly(true);
	//ui->dateLineEdit->setReadOnly(true);
	//ui->lifeCycleLineEdit->setReadOnly(true);
	//ui->usedCntLineEdit->setReadOnly(true);
	//ui->socketFailCntLineEdit->setReadOnly(true);
}

void AngKTabSite::InitProgPropetry()
{
	ui->socketProperty->setHeaderVisible(true);
	QTreeView* treeView = ui->socketProperty->findChild<QTreeView*>();
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

	ui->socketProperty->clear();
	m_pVarManager = new QtVariantPropertyManager(ui->socketProperty);
	m_pVarFactory = new QtVariantEditorFactory(ui->socketProperty);

	m_pStrManager = new QtStringPropertyManager(ui->socketProperty);
	m_pLineFactory = new QtLineEditFactory(ui->socketProperty);

	//Screen 属性
	QtProperty* socketGroup = m_pVarManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Socket"));
	QtProperty* item = m_pVarManager->addProperty(QVariant::String, tr("Name"));
	socketGroup->addSubProperty(item);

	item = m_pStrManager->addProperty(tr("SN"));
	m_pStrManager->setReadOnly(item, true);
	socketGroup->addSubProperty(item);

	item = m_pStrManager->addProperty(tr("Date"));
	m_pStrManager->setReadOnly(item, true);
	socketGroup->addSubProperty(item);

	item = m_pStrManager->addProperty(tr("LifeCycle"));
	m_pStrManager->setReadOnly(item, true);
	socketGroup->addSubProperty(item);

	item = m_pStrManager->addProperty(tr("Used Count"));
	m_pStrManager->setReadOnly(item, true);
	socketGroup->addSubProperty(item);

	item = m_pStrManager->addProperty(tr("Fail Count"));
	m_pStrManager->setReadOnly(item, true);
	socketGroup->addSubProperty(item);

	ui->socketProperty->addProperty(socketGroup);

	ui->socketProperty->setFactoryForManager(m_pVarManager, m_pVarFactory);
	ui->socketProperty->setFactoryForManager(m_pStrManager, m_pLineFactory);
}

bool AngKTabSite::eventFilter(QObject* watched, QEvent* event)
{
	QMouseEvent* _mouse = static_cast<QMouseEvent*>(event);

	if (event->type() == QEvent::HoverEnter && watched == ui->moreInfoLabel)
	{
		this->setCursor(Qt::PointingHandCursor);
	}
	else if (event->type() == QEvent::HoverLeave && watched == ui->moreInfoLabel)
	{
		this->setCursor(Qt::ArrowCursor);
	}
	else if (_mouse->button() == Qt::LeftButton 
		&& event->type() == QEvent::MouseButtonPress
		&& watched == ui->moreInfoLabel)
	{
		//TODO 发送信号点击弹窗
		qDebug() << "more info click";
	}

	return QWidget::eventFilter(watched, event);
}
