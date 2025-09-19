#include "AngKSiteWidget.h"
#include "ui_AngKSiteWidget.h"
#include "../View/GlobalInit/StyleInit.h"
#include "AngKSiteWidget.h"
#include <QCheckBox>
#include <QMovie>

AngKSiteWidget::AngKSiteWidget(int idx, BPU_SKT_VALUE BPUValue, std::string strAdapterID, QWidget *parent)
	: QWidget(parent)
	, m_curState(SiteStatus::Normal)
	, m_strAdapterID(strAdapterID)
	, m_BPU_Value(BPUValue)
{
	ui = new Ui::AngKSiteWidget();
	ui->setupUi(this);

	setSiteStatus(m_curState);
	this->setObjectName("AngKSiteWidget");
	QT_SET_STYLE_SHEET(objectName());

	setSiteNumLabel(idx);
	connect(ui->siteButton, &QPushButton::clicked, this, &AngKSiteWidget::sgnCheckAllSelect);

	siteSKTInfo.CurCnt = 0;
	siteSKTInfo.FailCnt = 0;
}

AngKSiteWidget::~AngKSiteWidget()
{
	delete ui;
}

void AngKSiteWidget::setSiteStatus(SiteStatus sitState, uint16_t resultCode)
{
	m_curState = sitState;

	bool isRun = true;
	switch (m_curState)
	{
	case SiteStatus::Failed:
		{
			//ui->siteButton->setCheckable(false);
			ui->rightWidget->setProperty("customProperty", "Failed");
			ui->successLabel->hide();
			ui->failedLabel->show();
			ui->busyLabel->hide();
			ui->errMsgLabel->hide();
			siteSKTInfo.FailCnt++;
			siteSKTInfo.CurCnt++;
		}
		break;
	case SiteStatus::Normal:
		{
			//ui->siteButton->setCheckable(true);
			ui->rightWidget->setProperty("customProperty", "Normal");
			ui->successLabel->hide();
			ui->failedLabel->hide();
			ui->busyLabel->hide();
			ui->errMsgLabel->hide();
			isRun = false;
		}
		break;
	case SiteStatus::Busy:
	{
		//ui->siteButton->setCheckable(false);
		ui->rightWidget->setProperty("customProperty", "Busy");
		ui->successLabel->hide();
		ui->failedLabel->hide();
		ui->busyLabel->show();
		ui->errMsgLabel->hide();
	}
	break;
	case SiteStatus::Success:
		{
			//ui->siteButton->setCheckable(false);
			ui->rightWidget->setProperty("customProperty", "Success");
			ui->successLabel->show();
			ui->failedLabel->hide();
			ui->busyLabel->hide();
			ui->errMsgLabel->hide();
			siteSKTInfo.CurCnt++;
		}
		break;
	default:
		break;
	}

	this->setStyle(QApplication::style());

	//ui->rateLabel->setText(QString::number(ui->siteProgressBar->value()) + "%");
}

void AngKSiteWidget::setSiteNumLabel(int index)
{
	QString result = QString::number((index - 1) / 2 + 1) + (index % 2 == 0 ? "B" : "A");
	ui->siteButton->setText(result);
	this->setObjectName("AngKSiteWidget_" + result);

	QMovie* movie = new QMovie(":/Skin/Light/Common/busy.gif"); // 替换为你的GIF文件路径
	ui->busyLabel->setMovie(movie); // 将GIF动画设置给QLabel
	movie->start(); // 启动动画

	ui->busyLabel->setScaledContents(true);

	// 获取QLabel的当前尺寸，并设置GIF动画的缩放尺寸
	QSize size = ui->busyLabel->size(); // 或者你可以设置一个特定的尺寸
	movie->setScaledSize(size);
}

void AngKSiteWidget::setSiteCheck(bool bClick)
{
	ui->siteButton->setChecked(bClick);
}

void AngKSiteWidget::setSiteProgress(int nValue)
{
	ui->siteProgressBar->setValue(nValue);
	//ui->rateLabel->setText(QString::number(ui->siteProgressBar->value()) + "%");
}

int AngKSiteWidget::GetSiteProgress()
{
	return ui->siteProgressBar->value();
}

bool AngKSiteWidget::GetSiteIsCheck()
{
	return ui->siteButton->isChecked();
}

void AngKSiteWidget::SetUseEnable(bool bEnable)
{
	ui->siteButton->setEnabled(bEnable);
}

bool AngKSiteWidget::GetUseEnable()
{
	return ui->siteButton->isEnabled();
}

void AngKSiteWidget::SetSKTInfo(int& nLifeCycleCnt, std::string& strUID)
{
	QString updateUID = QString::fromStdString(strUID);
	if (siteSKTInfo.UID != updateUID) {
		siteSKTInfo.LimitCnt = nLifeCycleCnt;
		siteSKTInfo.UID = updateUID;
		//更换座子次数是否更新？
	}
}

tSktInfo& AngKSiteWidget::GetSKTInfo()
{
	return siteSKTInfo;
}
