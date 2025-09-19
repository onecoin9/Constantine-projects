#include "ACProjectCreateDlg.h"
#include "ui_ACProjectCreateDlg.h"
#include "StyleInit.h"
#include "ACProjectProperty.h"
#include "ACMessageBox.h"
#include "AngKCustomTab.h"
#include <QLabel>
#include <QPushButton>
#include <QTabBar>
#include <QToolButton>
#include <QInputDialog>

ACProjectCreateDlg::ACProjectCreateDlg(QWidget* parent, std::shared_ptr<AngKDataBuffer> pDataBuffer)
	: AngKDialog(parent)
	, ui(new Ui::ACProjectCreateDlg())
	/*, m_pAddButton(std::make_unique<QToolButton>(this))*/
{
	this->setObjectName("ACProjectCreateDlg");
	QT_SET_STYLE_SHEET(objectName());

	ui->setupUi(setCentralWidget());

	InitText();
	InitButton();

	//默认初始化一个工程属性界面
	AddProjTab(pDataBuffer.get());

	this->setFixedSize(960, 850);
}

ACProjectCreateDlg::~ACProjectCreateDlg()
{
	delete ui;
}

void ACProjectCreateDlg::InitText()
{
	this->SetTitle(tr("Project Create Wizard"));
}

void ACProjectCreateDlg::InitButton()
{
	connect(this, &ACProjectCreateDlg::sgnClose, this, &ACProjectCreateDlg::close);
	//设置tabBar样式
	ui->projTabWidget->tabBar()->setStyle(new AngKCustomTab(nullptr, 0, 0));

	//connect(ui->projTabWidget->tabBar(), &QTabBar::tabBarDoubleClicked, this, &ACProjectCreateDlg::onSlotTabBarDoubleClicked);


	//多工程操作，暂不放开，可以使用
	//m_pAddButton->setText("+");
	//m_pAddButton->setAutoRaise(true);
	//connect(m_pAddButton.get(), &QPushButton::clicked, this, &ACProjectCreateDlg::onSlotAddProjTab);
	//connect(ui->projTabWidget, &QTabWidget::tabCloseRequested, this, &ACProjectCreateDlg::onSlotRemoveProjTab);

	//ui->projTabWidget->addTab(new QLabel(), "");
	//ui->projTabWidget->setTabEnabled(0, false);
	//ui->projTabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, m_pAddButton.get());
}

void ACProjectCreateDlg::AddProjTab(AngKDataBuffer* pDataBuf)
{
	ui->projTabWidget->setCurrentIndex(0);
	QString tabName = "default.eapr";
	ACProjectProperty* projProp = new ACProjectProperty(ui->projTabWidget, ui->projTabWidget->count() - 1, tabName, pDataBuf);
	ui->projTabWidget->insertTab(ui->projTabWidget->count() - 1, projProp, tabName);
	connect(projProp, &ACProjectProperty::sgnSaveSuccessClose, this, &ACProjectCreateDlg::close);
	connect(projProp, &ACProjectProperty::sgnImportProjName, this, &ACProjectCreateDlg::onSlotImportProjName);
	connect(projProp, &ACProjectProperty::sgnRenameClick, this, &ACProjectCreateDlg::onSlotTabBarDoubleClicked);
}

void ACProjectCreateDlg::onSlotRemoveProjTab(int nIndex)
{
	ui->projTabWidget->setCurrentIndex(nIndex);

	auto ret = ACMessageBox::showWarning(this, tr("Remove Warn"), tr("Do you want to delete the project configuration page?"), ACMessageBox::ACMsgButton::MSG_OK_CANCEL_BTN);

	if (ret == ACMessageBox::ACMsgType::MSG_OK) {
		ui->projTabWidget->removeTab(nIndex);
		//删除之后显示删除tab的前一个
		ui->projTabWidget->setCurrentIndex(nIndex - 1);
	}
}

void ACProjectCreateDlg::onSlotTabBarDoubleClicked()
{
	QInputDialog dialog(this);
	dialog.setWindowTitle(tr("Project Rename"));
	dialog.setLabelText(tr("Project Name:"));
	dialog.setTextValue(ui->projTabWidget->tabText(0).mid(0, ui->projTabWidget->tabText(0).indexOf(".eapr")));
	dialog.setInputMode(QInputDialog::TextInput);
	dialog.setFixedSize(250, 200); 
	dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);

	

	if (dialog.exec() == QDialog::Accepted) {
		QString newName = dialog.textValue();
		if (newName.indexOf(".eapr") == newName.length() - sizeof(".eapr")) {
			newName = newName.mid(0, newName.length() - sizeof(".eapr"));
		}

		if (newName.toUtf8().length() > 156)
		{
			ACMessageBox::showError(this, tr("Rename Error"), tr("Project rename failed, project name cannot exceed 156 bytes."));
			ALOG_INFO("Project rename failed, project name cannot exceed 156 bytes.", "CU", "--");
			return;
		}

		ui->projTabWidget->setTabText(0, newName);
		ACProjectProperty* projProWgt = qobject_cast<ACProjectProperty*>(ui->projTabWidget->widget(0));
		projProWgt->SetProjName(newName);
	}
}

void ACProjectCreateDlg::onSlotImportProjName(QString _projName)
{
	ui->projTabWidget->setTabText(0, _projName);
}

void ACProjectCreateDlg::onSlotAddProjTab()
{
	//AddProjTab();
}


void ACProjectCreateDlg::FTabCreateEmptyEapr() {
	ACProjectProperty* projProWgt = qobject_cast<ACProjectProperty*>(ui->projTabWidget->widget(0));
	projProWgt->MasterChipAnalyzeCreateEapr();
	projProWgt->onSlotSaveProject();
}