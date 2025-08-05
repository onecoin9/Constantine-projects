#include "DriverXmlParseTool.h"

DriverXmlParseTool::DriverXmlParseTool(QWidget *parent)
	: QMainWindow(parent)
{
	//ui.setupUi(this);

	this->setWindowTitle("	-DriverParseXMLTool");

	SetupWidgets();
	InitMenuBar();
}

void DriverXmlParseTool::SetupWidgets()
{
	QFrame* frame = new QFrame;
	QHBoxLayout* wgtLayout = new QHBoxLayout(frame);

	m_pDriverParseWgt = new DriverParseWidget(this);
	connect(m_pDriverParseWgt, &DriverParseWidget::sgnWindowTitle, [=](QString strTitle)
		{
			this->setWindowTitle(strTitle + "	-DriverParseXMLTool");
		});

	wgtLayout->addWidget(m_pDriverParseWgt);
	setCentralWidget(frame);
}

void DriverXmlParseTool::InitMenuBar()
{
	QMenu* openMenu = new QMenu("File", this);
	QMenu* editMenu = new QMenu("Edit", this);

	QAction* newFile = new QAction("New File...");
	QAction* openFile = new QAction("Open File...");
	QAction* clearWnd = new QAction("Clear Current Window");

	openMenu->addAction(newFile);
	openMenu->addAction(openFile);
	editMenu->addAction(clearWnd);

	connect(newFile, &QAction::triggered, m_pDriverParseWgt, &DriverParseWidget::OnSlotNewFile);
	connect(openFile, &QAction::triggered, m_pDriverParseWgt, &DriverParseWidget::OnSlotLoadFile);
	connect(clearWnd, &QAction::triggered, m_pDriverParseWgt, &DriverParseWidget::OnSlotClearCurrentEditWgt);

	menuBar()->addMenu(openMenu);
	menuBar()->addMenu(editMenu);
}