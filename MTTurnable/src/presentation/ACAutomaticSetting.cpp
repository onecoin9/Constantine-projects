#include "presentation/ACAutomaticSetting.h"
#include "ui_ACAutomaticSetting.h"
#include <QFileDialog>
#include <QTimer>
#include <QCoreApplication>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QSpacerItem>
#include <QHeaderView>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "core/Logger.h"
#include "core/CoreEngine.h"
#include "services/DeviceManager.h"
#include "domain/HandlerDevice.h"

// 这是一个临时的全局变量，用于获取CoreEngine实例。
// 在实际项目中，应该通过更好的依赖注入方式获取。
extern std::shared_ptr<Core::CoreEngine> g_coreEngine;

namespace Presentation {

ACAutomaticSetting::ACAutomaticSetting(QWidget *parent)
	: QDialog(parent)
{
	ui = new Ui::ACAutomaticSetting();
	ui->setupUi(this);
	this->setWindowTitle(tr("Automatic Setting"));
	InitText();
	initIpsTypeCBox();
	InitButton();
	initCoordWidget();

	adjustSize();
}

ACAutomaticSetting::~ACAutomaticSetting()
{
	delete ui;
}

void ACAutomaticSetting::initCoordWidget()
{
	QVector<QPair<QString, int>> coordInfoList;

	coordInfoList.append(qMakePair(tr("Assigns Supply Tray:"),       (int)TRAY_TYPE_SUPPLY));
	coordInfoList.append(qMakePair(tr("Assigns Production Tray:"),   (int)TRAY_TYPE_PRODUCTION));
	coordInfoList.append(qMakePair(tr("Assigns Reject Tray:"),       (int)TRAY_TYPE_REJECT));

	quint32 row = 0;
	for(const QPair<QString, int> &pair : coordInfoList) {
		QVector<QWidget*> vecWidget;

		auto coordTypelab = new QLabel(pair.first, ui->coordGroup);
		coordTypelab->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

		auto cbox = new QComboBox(ui->coordGroup);
		cbox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

		cbox->addItem(tr("Default"),    TRAY_CFG_DEFAULT);
		cbox->addItem(tr("Manual"),     TRAY_CFG_MANUAL);
		cbox->addItem(tr("Auto"),       TRAY_CFG_AUTO);
		cbox->setCurrentIndex(0);

		auto xCoordLab = new QLabel(QStringLiteral("X:"), ui->coordGroup);
		xCoordLab->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

		auto yCoordLab = new QLabel(QStringLiteral("Y:"), ui->coordGroup);
		yCoordLab->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

		auto xCoordEdit = new QLineEdit(ui->coordGroup);
		auto yCoordEdit = new QLineEdit(ui->coordGroup);

		auto func = [=](int index) {
			qint32 type = cbox->itemData(index).toInt();
			bool state = (type == TRAY_CFG_MANUAL) ? true : false;
			xCoordEdit->setEnabled(state);
			yCoordEdit->setEnabled(state);
		};

		connect(cbox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), func);
		func(TRAY_CFG_DEFAULT);

		int col = 0;
		ui->trayGLayout->addWidget(coordTypelab,    row, col++);
		ui->trayGLayout->addWidget(cbox,            row, col++);
		ui->trayGLayout->addItem(new QSpacerItem(50, 0, QSizePolicy::Expanding, QSizePolicy::Preferred), row, col++);

		ui->trayGLayout->addWidget(xCoordLab,       row, col++);
		ui->trayGLayout->addWidget(xCoordEdit,      row, col++);
		ui->trayGLayout->addWidget(yCoordLab,       row, col++);
		ui->trayGLayout->addWidget(yCoordEdit,      row, col++);

		
		m_trayMap.insert(pair.second, row++);
	}
}

void ACAutomaticSetting::getCoordinfo(int type, QStringList& infoList)
{
	infoList.clear();

	// 获取托盘类型所在行
	int row = m_trayMap.value((int)type);

	// 托盘配置类型
	auto cbox = dynamic_cast<QComboBox *>(ui->trayGLayout->itemAtPosition(row, 1)->widget());
	int cfg = (int)cbox->itemData(cbox->currentIndex()).toInt();
	infoList.append(QString::number(cfg));

	// 托盘X和Y坐标
	auto xEdit = dynamic_cast<QLineEdit*>(ui->trayGLayout->itemAtPosition(row, 4)->widget());
	auto yEdit = dynamic_cast<QLineEdit*>(ui->trayGLayout->itemAtPosition(row, 6)->widget());
	int xPos = 0, yPos = 0;

	if (type == TRAY_CFG_MANUAL) {
	   xPos = xEdit->text().toInt();
	   yPos = yEdit->text().toInt();
	}

	infoList.append(QString::number(xPos));
	infoList.append(QString::number(yPos));

}

void ACAutomaticSetting::getIpsTypeDesc(quint32 index, QString &desc)
{
	int type = ui->ipsTypeCBox->itemData(index).toInt();

	if (type == IPSTYPE_5000_5800) {
		desc = QString("IPS5000_IPS5800");
	}
	else if (type == IPSTYPE_3000_5200) {
		desc = QString("IPS3000_IPS5200");
	}
	else if (type == IPSTYPE_7000_PHA2000) {
		desc = QString("IPS7000_PHA2000");
	}
	else {
		desc = QString("Other");
	}
}

void ACAutomaticSetting::initIpsTypeCBox()
{
	ui->ipsTypeCBox->addItem(QString("IPS5000/IPS5800"),    IPSTYPE_5000_5800);
	ui->ipsTypeCBox->addItem(QString("IPS3000/IPS5200"),    IPSTYPE_3000_5200);
	ui->ipsTypeCBox->addItem(QString("IPS7000/PHA2000"),    IPSTYPE_7000_PHA2000);
	ui->ipsTypeCBox->addItem(tr("Other"),                   IPSTYPE_OTHER);

	//qint32 type = AngKGlobalInstance::ReadValue("IPSCFG", "IPSType", IPSTYPE_OTHER).toInt();
	qint32 type = IPSTYPE_5000_5800;
	//ui->ipsTypeCBox->setCurrentIndex(type);

	auto slotCBoxfunc = [this](int index) {

		QString section;
		getIpsTypeDesc(index, section);

		//QString path = AngKGlobalInstance::ReadValue("IPSCFG", section, m_strRecordProjPath).toString();
		QString path = "";
		QFile file(path);
		if (file.exists()) {
			m_strRecordProjPath = path;
		}
		else {
			m_strRecordProjPath = QCoreApplication::applicationDirPath();
		}

		//AngKGlobalInstance::WriteValue("IPSCFG", "IPSType", index);
	};

	connect(ui->ipsTypeCBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), slotCBoxfunc);

	slotCBoxfunc(type);
}


void ACAutomaticSetting::InitText()
{
	ui->passlotText->setText(tr("Pass Lot:"));
	ui->tskPathText->setText(tr("Tsk Path:"));
	ui->coordGroup->setTitle(tr("Tray Start Position"));
	ui->realStartPosText->setText(tr("Reel Start Position:"));
	ui->ipsTypeLab->setText(tr("IPS Type:"));
	ui->loadButton->setText(tr("Load"));
	ui->hideBtn->setText(tr("Hide"));
	ui->cmd3GroupSetting->setTitle(tr("Command3 Setting"));

	ui->defaultTrayLab->setText(tr("Default: Pick and place from the default starting coordinate"));
	ui->manualTrayLab->setText(tr("Manual: Set the X and Y coordinate manually"));
	ui->autoTrayLab->setText(tr("Auto: Set the starting coordinate of the tray style automatically"));
}

void ACAutomaticSetting::InitButton()
{
	// connect(this, &QDialog::, [this]() {
	// 	if (GetLoadStatus()) {
	// 		//ACMessageBox::showWarning(this, tr("Tips"), tr("The Automatic programming has not finished yet."));
	// 		return;
	// 	}
	// 	this->close();
	// 	});

	connect(ui->tskPathButton, &QPushButton::clicked, this, &ACAutomaticSetting::onSlotTskPathSetting);
	connect(ui->loadButton, &QPushButton::clicked, this, &ACAutomaticSetting::onSlotLoadAutoSetting);

	connect(ui->hideBtn, &QPushButton::clicked, this, [=]{
		this->hide();
	});
}
void ACAutomaticSetting::packCmdMsg(QString &cmdList)
{
	cmdList.clear();

	// 将参数存储到 QStringList 中
	QStringList params;
	params << QString::number(ui->passlotEdit->text().toInt()) << ui->tskPathEdit->text();

	// 托盘坐标信息
	QStringList trayInfo;
	getCoordinfo(TRAY_TYPE_SUPPLY, trayInfo);
	params.append(trayInfo);

	getCoordinfo(TRAY_TYPE_PRODUCTION, trayInfo);
	params.append(trayInfo);

	getCoordinfo(TRAY_TYPE_REJECT, trayInfo);
	params.append(trayInfo);

	// 卷带起始位置
	params.append(QString::number(ui->realStartPosEdit->text().toInt()));

	// 拼接字符串，"3"代表命令码
	cmdList = "3," + params.join(",") + ",,,,,,,";
}

/// <summary>
/// 加载tsk文件和托盘信息
/// </summary>
void ACAutomaticSetting::onSlotLoadAutoSetting()
{
	if (!g_coreEngine) {
		LOG_MODULE_ERROR("ACAutoSetting", "CoreEngine instance is not available.");
		QMessageBox::critical(this, tr("错误"), tr("核心引擎未初始化，无法开始流程。"));
		return;
	}

	// The old startAutoProcess is deprecated.
	// The new architecture uses a dedicated workflow for system initialization.
	if (!g_coreEngine->startSystemInitialization()) {
		QMessageBox::critical(this, tr("错误"), tr("启动系统初始化流程失败，请检查日志。"));
	} else {
		this->hide();
	}
}

void ACAutomaticSetting::onSlotTskPathSetting()
{
	QString strProjPath = QFileDialog::getOpenFileName(this, tr("Select Tsk File..."), m_strRecordProjPath, tr("auto task File(*.tsk)"));
	if (!strProjPath.isEmpty()) {
		m_strRecordProjPath = strProjPath;
		ui->tskPathEdit->setText(QDir::fromNativeSeparators(m_strRecordProjPath));

		// 更新IPS默认路径
		QString section;
		getIpsTypeDesc(ui->ipsTypeCBox->currentIndex(), section);

		//AngKGlobalInstance::WriteValue("IPSCFG", section, QFileInfo(m_strRecordProjPath).absolutePath());
	}
}


void ACAutomaticSetting::onSlotAutomicOver() {
	m_timeOutTimer->stop();
	setLoadStatus(false);
}

void ACAutomaticSetting::setLoadStatus(bool status) {

	//auto appStatus = status ? ApplicationStatus::AutomaticBurn : ApplicationStatus::Normal;
	//AngKGlobalInstance::instance()->setAppRunStatus(appStatus);
	status = false;
	m_bLoading = status;
	ui->loadButton->setEnabled(status ? false : true);
	if (status)
		m_textChangeTimer->start(500);
	else {
		m_textChangeTimer->stop();
	}
	ui->loadButton->setText(status ? tr("Loading") : tr("Load"));
	emit sgnAutoStartBurn(status);
}

bool ACAutomaticSetting::GetLoadStatus() {
	return m_bLoading;
}
} // namespace Presentation
