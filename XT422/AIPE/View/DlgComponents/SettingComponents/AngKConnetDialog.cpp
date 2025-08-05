#include "AngKConnetDialog.h"
#include "StyleInit.h"
#include "AngKPathResolve.h"
#include "ACChipManager.h"
#include "AngKDeviceModel.h"
#include "ACDeviceManager.h"
#include "AngkLogger.h"
#include "AngKTransmitSignals.h"
#include "GlobalDefine.h"
#include "MessageType.h"
#include <QMessageBox>
#include <QMouseEvent>
#include <QButtonGroup>
#include <QDesktopWidget>
#include <time.h>
#include "AngKScanManager.h"
#include "../RemoteServer/JsonRpcServer.h"
AngKConnetDialog::AngKConnetDialog(QWidget* parent)
	: QDialog(parent)
	, m_curConnectType(ConnectType::None)
	, m_nOldHeight(0)
	, m_nOldSize(QSize(0, 0))
	, m_desktop(QApplication::desktop())
	, m_nDraggableHeight(50)
	, m_bDragging(false)
	, m_pConTypeGroup(nullptr)
{
	ui = new Ui::AngKConnetDialog();
	ui->setupUi(this);

	this->setWindowFlag(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground, true);

	InitConnectDlg();
	InitButton();

	this->setObjectName("AngKConnetDialog");
	QT_SET_STYLE_SHEET(objectName());
	connect(JsonRpcServer::Instance(),&JsonRpcServer::CloseSiteScanAndConnectUI, this, &AngKConnetDialog::onSlotScanManagerFinished);
}
void AngKConnetDialog::onSlotScanManagerFinished() {
	ALOG_INFO("AngKConnetDialog::onSlotScanManagerFinished:%d", "CU", "--", AngKDeviceModel::instance().GetConnetDevMapSize());
	g_AppMode = ConnectType::Ethernet;
	onSlotConnectFinish();
}
AngKConnetDialog::~AngKConnetDialog()
{
	delete ui;
}

bool AngKConnetDialog::eventFilter(QObject* watched, QEvent* event)
{
	QMouseEvent* _mouse = static_cast<QMouseEvent*>(event);

	return QWidget::eventFilter(watched, event);
}

void AngKConnetDialog::resizeEvent(QResizeEvent* re)
{
	QPoint center = m_desktop->screenGeometry(m_desktop->screenNumber(QCursor::pos())).center();
	QPoint dialogCenter = this->rect().center();

	// 计算对话框的新位置，使其居中
	int newX = center.x() - dialogCenter.x();
	int newY = center.y() - dialogCenter.y();

	// 移动对话框到新位置
	this->move(newX, newY);

	QWidget::resizeEvent(re);
}

void AngKConnetDialog::closeEvent(QCloseEvent* event)
{
	if (AngKDeviceModel::instance().GetConnetDevMapSize() < 1 && g_AppMode != ConnectType::Demo) {
		emit sgnCloseWindow();
	}

	QWidget::closeEvent(event);
}

void AngKConnetDialog::mousePressEvent(QMouseEvent* event)
{
	if (event->y() < m_nDraggableHeight && event->button() == Qt::LeftButton)
	{
		m_clickPos.setX(event->pos().x());
		m_clickPos.setY(event->pos().y());
		m_bDragging = true;
	}
}

void AngKConnetDialog::mouseMoveEvent(QMouseEvent* event)
{
	if (m_bDragging && event->buttons() == Qt::LeftButton)
	{
		this->move(this->pos() + event->pos() - this->m_clickPos);
	}
}

void AngKConnetDialog::mouseReleaseEvent(QMouseEvent* event)
{
	m_bDragging = false; // 重置拖动状态
	QDialog::mouseReleaseEvent(event);
}

void AngKConnetDialog::InitConnectDlg()
{
	ui->conTypeStackedWidget->hide();
	m_nOldHeight = this->height();
	m_nOldSize = QSize(this->width(), this->height());
	QRect desk_rect = m_desktop->screenGeometry(m_desktop->screenNumber(QCursor::pos()));
	move((desk_rect.width() - this->width()) / 2 + desk_rect.left(), (desk_rect.height() - this->height()) / 2 + desk_rect.top());

	ui->conText->setText(tr("Connect"));
	ui->conTypeText->setText(tr("Connect Type"));

	connect(ui->EthernetWidget, &ACEthernetConfigWidget::sgnConnectFinish, this, &AngKConnetDialog::onSlotConnectFinish);
	connect(ui->DemoWidget, &AngKDemoWidget::sgnEnterDemoMode, this, &AngKConnetDialog::onSlotEnterDemoMode);
}

void AngKConnetDialog::InitButton()
{
	ui->usbButton->setText(tr("USB"));
	ui->netButton->setText(tr("Ethernet"));
	ui->demoButton->setText(tr("Demo"));
	ui->usbButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/USBSymbolNormal.svg"));
	ui->netButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/EthernetSymbolNormal.svg"));
	ui->demoButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/DemoSymbolNormal.svg"));

	m_pConTypeGroup = new QButtonGroup(ui->widget_5);

	connect(m_pConTypeGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), [&](int index)
		{
			ui->conTypeStackedWidget->show();
			if ((int)m_curConnectType == index)
				return;

			ui->conTypeStackedWidget->setCurrentIndex(index - 1);

			SwitchConnectTypeWgt(ConnectType(index));
		});

	m_pConTypeGroup->addButton(ui->usbButton, (int)ConnectType::USB);
	m_pConTypeGroup->addButton(ui->netButton, (int)ConnectType::Ethernet);
	m_pConTypeGroup->addButton(ui->demoButton, (int)ConnectType::Demo);

	connect(ui->closeButton, &QPushButton::clicked, this, &AngKConnetDialog::onSlotConnectFinish);
}

void AngKConnetDialog::SwitchConnectTypeWgt(ConnectType nIndexType)
{
	switch (nIndexType)
	{
	case ConnectType::USB:
	{
		this->setFixedHeight(m_nOldHeight + ui->conTypeStackedWidget->currentWidget()->maximumHeight());
		ui->conTypeStackedWidget->hide();
	}
		break;
	case ConnectType::Ethernet:
	{
		ui->conTypeStackedWidget->show();
		this->setFixedHeight(m_nOldHeight + ui->conTypeStackedWidget->currentWidget()->maximumHeight());
	}
		break;
	case ConnectType::Demo:
	{
		ui->conTypeStackedWidget->show();
		this->setFixedHeight(m_nOldHeight);
	}
		break;
	case ConnectType::None:
	default:
		break;
	}

	g_AppMode = nIndexType;
}

void AngKConnetDialog::onSlotEnterDemoMode()
{
	ui->EthernetWidget->CloseWidgetInfo();
	close();
}

void AngKConnetDialog::onSlotConnectFinish()
{
	ui->EthernetWidget->CloseSocket();

	close();
}
