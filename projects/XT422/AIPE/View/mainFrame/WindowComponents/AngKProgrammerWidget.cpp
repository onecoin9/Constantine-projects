#include "AngKProgrammerWidget.h"
#include "ui_AngKProgrammerWidget.h"
#include "AngKSiteWidget.h"
#include "StyleInit.h"
#include "errorcode.h"
#include "ACDeviceManager.h"
#include <QDebug>

#define MAX_BPUNUM 8
#define MAX_BPUROW 2
#define MAX_BPUCOL 4

AngKProgrammerWidget::AngKProgrammerWidget(int nIndex, QString ipHop, QWidget *parent)
	: QWidget(parent)
	, m_strIPHop(ipHop)
	, m_ConnectState(true)
{
	ui = new Ui::AngKProgrammerWidget();
	ui->setupUi(this);

	InitWidget();
	InitBPUSite();
	InitProgress();
	setProgramerIndex(ipHop);
	//setProgramerIndex(idx);
	//setValue(ui->progressBar->value());
	this->setObjectName("AngKProgrammerWidget");
	QT_SET_STYLE_SHEET(objectName());
	this->setObjectName("AngKProgrammerWidget_" + QString::number(nIndex));
}


AngKProgrammerWidget::~AngKProgrammerWidget()
{
	delete ui;
}

void AngKProgrammerWidget::InitWidget()
{
	ui->programerCheckBox->setText(tr("Programmer#"));
	ui->nameLabel->setText(tr("Name:")); 

	connect(ui->expandCheckBox, &QCheckBox::stateChanged, this, &AngKProgrammerWidget::onSlotExpandWidget);
	connect(ui->programerCheckBox, &QCheckBox::clicked, this, [=](bool bClick) {
		int selectNum = 0;
		for (int i = 0; i < ui->gridLayout->rowCount(); ++i)
		{
			for (int j = 0; j < ui->gridLayout->columnCount(); ++j)
			{
				if (ui->gridLayout->itemAtPosition(i, j) != nullptr)
				{
					AngKSiteWidget* objSite = qobject_cast<AngKSiteWidget*>(ui->gridLayout->itemAtPosition(i, j)->widget());
					objSite->setSiteCheck(bClick);
				}
			}
		}
		// bug:setCheck必为false jwl
		bool setCheck = false;
		if (selectNum == ui->gridLayout->rowCount() * ui->gridLayout->columnCount()) {
			setCheck = true;
		}
		emit sgnCurrentAllSelect(setCheck);
		});
}

void AngKProgrammerWidget::InitBPUSite()
{
	//setProgramerIndex(1);//当前不涉及级联AG06，所以默认为1个，若多个AG06相连，则需要重写这里
	//AddSite(m_BPUSite);
	QStringList ipHopList = m_strIPHop.split(":");
	if (ipHopList.length() != 2)
		return;

	auto devInfo = ACDeviceManager::instance().getDevInfo(ipHopList[0], ipHopList[1].toInt());
	for (int i = 0, siteIdx = 1; i < MAX_BPUNUM; ++i) {
		auto iter = devInfo.bpuInfoArr.cbegin();


		// 找到对应的BPU信息
		for (; iter != devInfo.bpuInfoArr.cend(); iter++) {
			if (iter->idx == i)
				break;
		}

		SingleBPU tmpBpuInfo = {-1, devInfo.nSingleBpuSiteNum, ""};
		if (iter != devInfo.bpuInfoArr.cend())
			tmpBpuInfo = *iter;

		
		int nRow, nCol = 0;
		nRow = MAX_BPUROW - i / MAX_BPUCOL - 1;
		nCol = i % MAX_BPUCOL;

		for (int j = 0; j < tmpBpuInfo.SktCnt; j++, siteIdx++) {
			AngKSiteWidget* siteWgt = new AngKSiteWidget(siteIdx, BPUCalVector[i * 2 + j], tmpBpuInfo.AdapterID);
			ui->gridLayout->addWidget(siteWgt, 2 * nRow + j, nCol, Qt::AlignCenter);
			connect(siteWgt, &AngKSiteWidget::sgnCheckAllSelect, this, &AngKProgrammerWidget::onSlotCheckAllSelect);
			siteWgt->setSiteCheck(true);
			siteWgt->setSiteCheck(false);
			m_mapSiteWgt[siteIdx] = siteWgt;
		}
	}
	ui->gridLayout->setAlignment(Qt::AlignCenter);


	//第一次初始化之后检查一下
	onSlotCheckAllSelect();
}

void AngKProgrammerWidget::setProgramerIndex(QString strProgName)
{
	ui->programerCheckBox->setText(strProgName);
	m_strProgName = strProgName;
}

void AngKProgrammerWidget::setProgramerName(QString strText)
{
	ui->nameText->setText(strText);
}

void AngKProgrammerWidget::setBPUProgress(int nBPUID, int nVal)
{
	//根据BPUID来进行设置进度
	int rowCount = ui->gridLayout->rowCount();
	int columnCount = ui->gridLayout->columnCount();

	for (int row = 0; row < rowCount; row++)
	{
		for (int column = 0; column < columnCount; column++)
		{
			// 获取单元格中的控件
			QWidget* widget = ui->gridLayout->itemAtPosition(row, column)->widget();
			AngKSiteWidget* siteWgt = qobject_cast<AngKSiteWidget*>(widget);
			if (siteWgt) {
				//SKTEn >> (nBPUID * 2) & 0x00000003
				if (siteWgt->GetBPU_Value() & (0x3 << (nBPUID * 2)))
				{
					if(siteWgt->GetSiteIsCheck())
						siteWgt->setSiteProgress(nVal);
				}
			}
		}
	}

	CalProgAllSiteProgress();
}

void AngKProgrammerWidget::setProgramerCheck(bool bCheck)
{
	ui->programerCheckBox->setChecked(bCheck);
	emit ui->programerCheckBox->clicked(bCheck);
}

uint32_t AngKProgrammerWidget::GetSitesChecked(int& siteCount)
{
	uint32_t CheckedBPU = 0;

	int rowCount = ui->gridLayout->rowCount();
	int columnCount = ui->gridLayout->columnCount();

	for (int row = 0; row < rowCount; row++)
	{
		for (int column = 0; column < columnCount; column++)
		{
			// 获取单元格中的控件
			QWidget* widget = ui->gridLayout->itemAtPosition(row, column)->widget();
			AngKSiteWidget* pro = qobject_cast<AngKSiteWidget*>(widget);
			if (pro){
				if (pro->GetSiteIsCheck()) {
					CheckedBPU += pro->GetBPU_Value();
					siteCount++;
				}
			}
		}
	}

	return CheckedBPU;
}

void AngKProgrammerWidget::ProjPropSwitchSiteEnable(std::vector<int> vecSite)
{
	ResetSiteCheck(0);

	for (int i = 0; i < vecSite.size(); ++i)
	{
		m_mapSiteWgt[vecSite[i] + 1]->setSiteCheck(true);
	}

}

bool AngKProgrammerWidget::GetProgramerCheck()
{
	bool progCheck = ui->programerCheckBox->isChecked();
	bool siteCheck = false;
	int rowCount = ui->gridLayout->rowCount();
	int columnCount = ui->gridLayout->columnCount();

	for (int row = 0; row < rowCount; row++)
	{
		for (int column = 0; column < columnCount; column++)
		{
			// 获取单元格中的控件
			QWidget* widget = ui->gridLayout->itemAtPosition(row, column)->widget();
			AngKSiteWidget* pro = qobject_cast<AngKSiteWidget*>(widget);
			if (pro && pro->GetSiteIsCheck()) {
				siteCheck = true;
				break;
			}
		}
	}

	return progCheck | siteCheck;
}

QString AngKProgrammerWidget::GetDevProgramIndex()
{
	return ui->programerCheckBox->text();
}

DeviceStu AngKProgrammerWidget::GetDevInfo()
{
	QStringList tmp = m_strIPHop.split(":");

	return tmp.length() == 2 ? ACDeviceManager::instance().getDevInfo(tmp[0], tmp[1].toInt()) : DeviceStu();
}

void AngKProgrammerWidget::SetBPUResultStatus(int nBPUIdx, int nSktIdx, QString RetCode, std::string strExtMsg)
{
	int rowCount = ui->gridLayout->rowCount();
	int columnCount = ui->gridLayout->columnCount();

	for (int row = 0; row < rowCount; row++)
	{
		for (int column = 0; column < columnCount; column++)
		{
			// 获取单元格中的控件
			QWidget* widget = ui->gridLayout->itemAtPosition(row, column)->widget();
			AngKSiteWidget* siteWgt = qobject_cast<AngKSiteWidget*>(widget);
			if (siteWgt) {
				//SKTEn >> (nBPUID * 2) & 0x00000003
				BPU_SKT_VALUE sktValue = BPUCalVector[(nBPUIdx * 2) + nSktIdx];
				if (siteWgt->GetSiteIsCheck() && (siteWgt->GetBPU_Value() == sktValue)){
					SiteStatus siteStu;
					if (RetCode == "4000") {
						siteStu = SiteStatus::Success;
					}
					else if (RetCode == "1001") {//Unused 状态
						siteStu = SiteStatus::Normal;
					}
					else {
						siteStu = SiteStatus::Failed;
					}

					siteWgt->setSiteStatus(siteStu);
					siteWgt->setSiteProgress(100);
				}
			}
		}
	}
}

void AngKProgrammerWidget::RestSiteStatus()
{
	InitProgress();
}

int AngKProgrammerWidget::GetProgramProgress()
{
	return ui->progressBar->value();
}

int AngKProgrammerWidget::IndexSwitchSiteIdx(int _idx)
{
	int nSiteIdx = 0;
	if (_idx >= 0 && _idx <= 3) {//第一行
		nSiteIdx = _idx * 2 + 9;
	}
	else if (_idx >= 4 && _idx <= 7) {//第二行
		nSiteIdx = _idx * 2 + 2;
	}
	else if (_idx >= 8 && _idx <= 11) {//第三行
		nSiteIdx = _idx * 2 - 15;
	}
	else if (_idx >= 12 && _idx <= 15) {//第四行
		nSiteIdx = _idx * 2 - 22;
	}

	return nSiteIdx;
}

std::string AngKProgrammerWidget::GetAdapterID(int nSiteIdx, bool& bFind)
{

	std::string adapterID = "";


	QStringList ipHopList = m_strIPHop.split(":");
	if (ipHopList.length() != 2)
		return "";

	auto m_BPUSite = ACDeviceManager::instance().getDevInfo(ipHopList[0], ipHopList[1].toInt()).bpuInfoArr;
	for (int i = 0; i < m_BPUSite.size(); ++i)
	{

		if (nSiteIdx >= m_BPUSite[i].idx * 2 + 1 && nSiteIdx <= m_BPUSite[i].idx * 2 + m_BPUSite[i].SktCnt) {
			adapterID = m_BPUSite[i].AdapterID;
			bFind = true;
			break;
		}
	}
	return adapterID;
}

void AngKProgrammerWidget::ResetSiteCheck(int nHopNum)
{
	for (auto siteVar : m_mapSiteWgt)
	{
		if(siteVar.second != nullptr)
			siteVar.second->setSiteCheck(false);
	}
}

void AngKProgrammerWidget::SetSiteStatus(int nSktEnable, SiteStatus typeStatu)
{
	for (auto siteVar : m_mapSiteWgt)
	{
		if (siteVar.second != nullptr) {

			if ((siteVar.second->GetBPU_Value() & nSktEnable) && siteVar.second->GetSiteIsCheck()) {
				siteVar.second->setSiteStatus(typeStatu);
			}
		}
	}
}

void AngKProgrammerWidget::SetBPUCntState(uint32_t& nTrueSktEn, int& nLifeCycleCnt, std::string& strUID)
{
	for (auto siteVar : m_mapSiteWgt)
	{
		if (siteVar.second != nullptr) {
			if (siteVar.second->GetBPU_Value() & nTrueSktEn)
			{
				siteVar.second->SetSKTInfo(nLifeCycleCnt, strUID);
			}
		}
	}
}

void AngKProgrammerWidget::GetBPUExeCnt(uint32_t& nTotalNum, uint32_t& nFailNum)
{
	for (auto siteVar : m_mapSiteWgt)
	{
		if (siteVar.second != nullptr) {
			nTotalNum += siteVar.second->GetSKTInfo().CurCnt;
			nFailNum += siteVar.second->GetSKTInfo().FailCnt;
		}
	}
}

void AngKProgrammerWidget::InitProgress()
{
	ui->progressBar->setValue(0);
	ui->barValue->setText(QString::number(ui->progressBar->value()) + "%");

	for (int i = 0; i < ui->gridLayout->rowCount(); ++i)
	{
		for (int j = 0; j < ui->gridLayout->columnCount(); ++j)
		{
			if (ui->gridLayout->itemAtPosition(i, j) != nullptr)
			{
				AngKSiteWidget* objSite = qobject_cast<AngKSiteWidget*>(ui->gridLayout->itemAtPosition(i, j)->widget());
				objSite->setSiteProgress(0);
				objSite->setSiteStatus(SiteStatus::Normal);
			}
		}
	}
}

void AngKProgrammerWidget::CalProgAllSiteProgress()
{
	int AllValue = 0;

	int rowCount = ui->gridLayout->rowCount();
	int columnCount = ui->gridLayout->columnCount();

	int getCheckNum = 0;
	for (auto site : m_mapSiteWgt)
	{
		if (site.second) {
			if (site.second->GetSiteIsCheck()) {
				getCheckNum++;
			}
		}
	}


	if (getCheckNum > 0) {
		AllValue = getCheckNum * 100;
		int siteProgress = 0;
		for (auto site : m_mapSiteWgt)
		{
			if (site.second) {
				if (site.second->GetSiteIsCheck()) {
					siteProgress += site.second->GetSiteProgress();
				}
			}
		}
		ui->progressBar->setValue((static_cast<float>(siteProgress) / AllValue) * 100);
		ui->barValue->setText(QString::number(ui->progressBar->value()) + "%");
	}
}

void AngKProgrammerWidget::onSlotCheckAllSelect()
{
	int selectNum = 0;
	for (int i = 0; i < ui->gridLayout->rowCount(); ++i)
	{
		for (int j = 0; j < ui->gridLayout->columnCount(); ++j)
		{
			if (ui->gridLayout->itemAtPosition(i, j) != nullptr)
			{
				AngKSiteWidget* objSite = qobject_cast<AngKSiteWidget*>(ui->gridLayout->itemAtPosition(i, j)->widget());
				if (objSite->GetSiteIsCheck())
					selectNum++;
			}
		}
	}

	ui->programerCheckBox->blockSignals(true);
	bool setCheck = false;
	if (selectNum == ui->gridLayout->rowCount() * ui->gridLayout->columnCount()) {
		setCheck = true;
	}

	ui->programerCheckBox->setChecked(setCheck);
	ui->programerCheckBox->blockSignals(false);
	emit sgnCurrentAllSelect(setCheck);
}

void AngKProgrammerWidget::onSlotExpandWidget(int state)
{
	switch (state)
	{
	case Qt::CheckState::Unchecked:
	{
		ui->bottomWidget->show();
	}
	break;
	case Qt::CheckState::Checked:
	{
		ui->bottomWidget->hide();
	}
	break;
	default:
		break;
	}
}

void AngKProgrammerWidget::SetDevConnectState(bool state) {
	m_ConnectState = state;
	ui->programerCheckBox->setEnabled(state);
	for (int i = 0; i < ui->gridLayout->rowCount(); ++i)
	{
		for (int j = 0; j < ui->gridLayout->columnCount(); ++j)
		{
			if (ui->gridLayout->itemAtPosition(i, j) != nullptr)
			{
				AngKSiteWidget* objSite = qobject_cast<AngKSiteWidget*>(ui->gridLayout->itemAtPosition(i, j)->widget());
				objSite->setEnabled(state);
			}
		}
	}
	if (!state) {
		ui->programerCheckBox->setChecked(false);
		ui->programerCheckBox->clicked(false);
		ui->programerCheckBox->setText(m_strProgName + "(Offline)");
	}
	else {
		ui->programerCheckBox->setText(m_strProgName);
	}
}

void AngKProgrammerWidget::SetProjBindBPUState(int nSktEn, std::string strAdapterID)
{
	QStringList ipHopList = m_strIPHop.split(":");
	if (ipHopList.length() != 2)
		return;

	QString hexAdapterID;
	if (!QString::fromStdString(strAdapterID).contains("0x")) {
		hexAdapterID = "0x" + QString::fromStdString(strAdapterID);
	}
	else {
		hexAdapterID = QString::fromStdString(strAdapterID);
	}
	auto devInfo = ACDeviceManager::instance().getDevInfo(ipHopList[0], ipHopList[1].toInt());
	for (auto site : m_mapSiteWgt)
	{
		if (site.second) {
			if (site.second->GetBPU_Value() & nSktEn) {
				int nBPUIndex = Utils::AngKCommonTools::GetBPUIndex(site.second->GetBPU_Value());

				for (int i = 0; i < devInfo.bpuInfoArr.size(); ++i) {
					if (devInfo.bpuInfoArr[i].idx == nBPUIndex && hexAdapterID.toStdString() == devInfo.bpuInfoArr[i].AdapterID) {
						if (!site.second->GetSiteIsCheck()) {
							site.second->setSiteCheck(true);
						}
					}
				}
			}
		}
	}
}

void AngKProgrammerWidget::SetProjBindBPUState(int nSktEn)
{
	QStringList ipHopList = m_strIPHop.split(":");
	if (ipHopList.length() != 2)
		return;

	auto devInfo = ACDeviceManager::instance().getDevInfo(ipHopList[0], ipHopList[1].toInt());
	for (auto site : m_mapSiteWgt)
	{
		if (site.second) {
			if (site.second->GetBPU_Value() & nSktEn) {
				if (!site.second->GetSiteIsCheck()) {
					site.second->setSiteCheck(true);
				}
			}
		}
	}
}


void AngKProgrammerWidget::SetDevBurnStatus(bool burnStatus) {
	if (burnStatus) {
		ui->bottomWidget->setEnabled(false);
		ui->programerCheckBox->setEnabled(false);
	}
	else {
		ui->bottomWidget->setEnabled(true);
		ui->programerCheckBox->setEnabled(true);
	}
}