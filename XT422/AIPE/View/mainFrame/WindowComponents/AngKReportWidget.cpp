#include "AngKReportWidget.h"
#include "ui_AngKReportWidget.h"
#include "StyleInit.h"

extern bool bAutoConnect;

AngKReportWidget::AngKReportWidget(QWidget *parent)
	: QWidget(parent)
	, m_nFailedNum(0)
{
	ui = new Ui::AngKReportWidget();
	ui->setupUi(this);

	ui->label->setText(tr("Report"));

	this->setObjectName("AngKReportWidget");
	QT_SET_STYLE_SHEET(objectName());
}

AngKReportWidget::~AngKReportWidget()
{
	delete ui;
}

void AngKReportWidget::SetCurrentProgress(int nProgress)
{
	ui->currentProgress->setCurrentProgress(nProgress);
}

void AngKReportWidget::UpdateCurrentProgress()
{
	//FIXME, use cmd3
	bool currrentServerMode = bAutoConnect;;
	if (currrentServerMode) {
		int nExpectNumber = ui->warningWgt->GetOutputDevNums();
		float fRate = (nExpectNumber * 1.0) / (ui->warningWgt->GetAllDevNums()) * 100;
		SetCurrentProgress(fRate);
	}
	else {
		int nSuccessAndFail = ui->warningWgt->GetOutputDevNums() + m_nFailedNum;
		float fRate = (nSuccessAndFail * 1.0) / (ui->warningWgt->GetAllDevNums()) * 100;
		SetCurrentProgress(fRate);
	}
}

void AngKReportWidget::SetoutputSlot(int nOutput)
{
	ui->warningWgt->setOutputDevNums(nOutput);
}

void AngKReportWidget::SetExpectSlot(int nExpect)
{
	ui->warningWgt->setAllDevNums(nExpect);
}

void AngKReportWidget::ResetExpectAndOutput()
{
	SetoutputSlot(0);
	SetExpectSlot(0);
	m_nFailedNum = 0;
	ui->yieldRate->setRoundProgressValue(0);
}

void AngKReportWidget::AddOutputNum()
{
	ui->warningWgt->AddOutputNum();

	//每次添加后都要计算一下当前良率
	CalCurrentYield();
}

void AngKReportWidget::AddFailedNum()
{
	m_nFailedNum++;
	CalCurrentYield();
}

void AngKReportWidget::CalCurrentYield()
{
	float passNum = ui->warningWgt->GetOutputDevNums();
	float fRate = (passNum * 1.0) / (passNum + static_cast<float>(m_nFailedNum)) * 100;
	ui->yieldRate->setRoundProgressValue(fRate);
	
}
