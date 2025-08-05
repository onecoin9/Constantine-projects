#include "ACProjectProperty.h"
#include "ui_ACProjectProperty.h"
#include "StyleInit.h"
#include "ACChipManager.h"
#include "ACMessageBox.h"
#include "AngKCustomTab.h"
#include "AngKIOHeapManager.h"
#include "AngKPathResolve.h"
#include "AngkLogger.h"
#include "AngKMessageHandler.h"
#include "AngKTransmitSignals.h"
#include "AngKSwapSetting.h"
#include "AngKGlobalInstance.h"
#include "AngKPathResolve.h"
#include <QDesktopWidget>
#include <QFileDialog>
#include <QHostInfo>
#include <QTabBar>
#include <QToolTip>

ACProjectProperty::ACProjectProperty(QWidget* parent, int nIndex, QString projName, AngKDataBuffer* pDataBuf)
	: QWidget(parent)
	, ui(new Ui::ACProjectProperty())
	, m_pProjManager(std::make_unique<ACProjManager>(this, pDataBuf))
	, m_pFileImportDlg(std::make_unique<AngKFilesImport>(this))
	, m_pFileTableModel(std::make_unique<QStandardItemModel>(this))
	, m_pAdvSettingDlg(std::make_unique<AngKAdvanceSetting>(this))
	, m_pCurrentIndex(0)
	, m_nBPUAttribute(0)
	, m_strProjName(projName)
	, m_strRecordProjPath(AngKGlobalInstance::ReadValue("TaskProjectPath", "projPath").toString())
{
	this->setObjectName("ACProjectProperty");
	QT_SET_STYLE_SHEET(objectName());

	ui->setupUi(this);

	InitText();
	InitCommonButton();
	InitPage();


	ui->nameEdit->setText(m_strProjName);
}

ACProjectProperty::~ACProjectProperty()
{
	delete ui;
}

void ACProjectProperty::InitText()
{
	ui->renameBtn->setText(tr("Rename"));
	ui->importButton->setText(tr("Import"));
	ui->saveButton->setText(tr("Save"));
	ui->saveButton->hide();
	ui->preButton->setText(tr("Previous"));
	ui->nextButton->setText(tr("Next"));

	ui->pathLabel->setText(tr("Current Project Path："));
	ui->addressLineEdit->setReadOnly(true);

	QString addressPath = AngKGlobalInstance::ReadValue("TaskProjectPath", "projPath").toString() + "/" + m_strProjName;
	ui->addressLineEdit->setText(addressPath);
}

void ACProjectProperty::InitCommonButton()
{
	ui->preButton->setEnabled(false);

	connect(ui->renameBtn, &QPushButton::clicked, this, &ACProjectProperty::sgnRenameClick);


	connect(ui->importButton, &QPushButton::clicked, this, &ACProjectProperty::onSlotImportProject);
	connect(ui->saveButton, &QPushButton::clicked, this, &ACProjectProperty::onSlotSaveProject);
	connect(ui->preButton, &QPushButton::clicked, this, &ACProjectProperty::onSlotTurnPage);
	connect(ui->nextButton, &QPushButton::clicked, this, &ACProjectProperty::onSlotTurnPage);
	connect(ui->addressBtn, &QPushButton::clicked, this, [=]() {
		m_strRecordProjPath = QFileDialog::getExistingDirectory(this, tr("Select File Directory..."), m_strRecordProjPath, QFileDialog::ShowDirsOnly);
		if (m_strRecordProjPath.isEmpty()) {
			m_strRecordProjPath = Utils::AngKPathResolve::localProjectPath();
			return;
		}
		AngKGlobalInstance::WriteValue("TaskProjectPath", "projPath", m_strRecordProjPath);

		QString addressPath = m_strRecordProjPath + "/" + m_strProjName;
		ui->addressLineEdit->setText(addressPath);
	});

	connect(ui->driverButton, &QPushButton::clicked, this, &ACProjectProperty::onSlotDriverParamConfig);

	connect(&AngKTransmitSignals::GetInstance(), &AngKTransmitSignals::sigOption2PropertyACXMLChipID,
		this, &ACProjectProperty::onSlotUpdateACXMLChipID);

}

void ACProjectProperty::InitPage()
{
	//设置tabBar样式
	ui->projTabWidget->tabBar()->setStyle(new AngKCustomTab(nullptr, 110));

	for (int i = 0; i < ALLNUM; ++i) {//tab页禁止点击
		ui->projTabWidget->tabBar()->setTabEnabled(i, false);
	}

	ui->projTabWidget->setCurrentIndex(0);

	//三个界面先进行隐藏
	ui->projTabWidget->removeTab(ui->projTabWidget->indexOf(ui->SNCPage));
	ui->projTabWidget->removeTab(ui->projTabWidget->indexOf(ui->OtherPage));
	ui->projTabWidget->removeTab(ui->projTabWidget->indexOf(ui->ChipConfigPage));

	//初始化芯片选择界面
	InitChipPage();

	//FilePage
	InitFilePage();

	//ChipConfigPage
	//InitChipConfigPage();

	//DrvParaPage
	InitDrvParaPage();

	//OperationPage
	InitOperationPage(m_pProjManager->GetProjData()->getChipData().getJsonValue<std::string>("chipOperCfgJson"));

	//BufferCheckSum
	InitCheckBufferPage();

	//OtherPage
	//InitOtherPage();
}

bool ACProjectProperty::CheckPageEvent()
{
	if (m_pCurrentIndex == ChipPage) {
		return ChipPageEvent();
	}
	else if (m_pCurrentIndex == FilePage) {
		return FilePageEvent();
	}
	else if (m_pCurrentIndex == ChipConfigPage) {
		return ChipConfigPageEvent();
	}
	else if (m_pCurrentIndex == DrvParaPage) {
		return DrvParaPageEvent();
	}
	else if (m_pCurrentIndex == BufferChecksumPage) {
		return BufferChecksumPageEvent();
	}
	else if (m_pCurrentIndex == OperationPage) {
		return OtherPageEvent();
	}
	else if (m_pCurrentIndex == SNCPage) {
		return SNCPageEvent();
	}
	else if (m_pCurrentIndex == OtherPage) {
		return OtherPageEvent();
	}
	return true;
}

void ACProjectProperty::ChangeTurnPageButton()
{
	if (m_pCurrentIndex == 0) {
		ui->preButton->setEnabled(false);
		ui->nextButton->setEnabled(true);
		ui->saveButton->hide();
	}
	else if (m_pCurrentIndex == ui->projTabWidget->count() - 1) {
		ui->preButton->setEnabled(true);
		ui->nextButton->setEnabled(false);
		ui->saveButton->show();
	}
	else {
		ui->preButton->setEnabled(true);
		ui->nextButton->setEnabled(true);
		ui->saveButton->hide();
	}
}

void ACProjectProperty::InitOperInfoList(OpInfoList& InfoList)
{
	ui->OperationPage->SaveOperationCfg(m_pProjManager->GetProjData()->GetOperInfoList());
}

void ACProjectProperty::SetProjName(QString _projName)
{
	m_strProjName = _projName;
	QString strSuffix = ".eapr";
	if (!m_strProjName.endsWith(strSuffix)) {
		// 如果没有后缀，则添加后缀
		m_strProjName.append(strSuffix);
	}

	QString addressPath = AngKGlobalInstance::ReadValue("TaskProjectPath", "projPath").toString() + "/" + m_strProjName;
	ui->addressLineEdit->setText(addressPath);
	ui->nameEdit->setText(m_strProjName);
}

void ACProjectProperty::InitChipPage()
{
	ACChipManager chipMgr;
	ChipDataJsonSerial chipJson;
	chipJson.serialize(chipMgr.GetChipData());
	m_pProjManager->GetProjData()->setChipData(chipJson);
	m_pProjManager->InitManager();
	ui->ChipPage->insertChipText(chipJson);

	//工程内重新选择芯片
	connect(ui->ChipPage, &AngKProjectChipSelect::sgnChipDataReport, this, [=](ChipDataJsonSerial reportChip) {
		m_pProjManager->GetProjData()->setChipData(reportChip);
		ALOG_INFO("Select chip : %s %s [%s].", "CU", "--", reportChip.getJsonValue<std::string>("manufacture").c_str(),
			reportChip.getJsonValue<std::string>("chipName").c_str(), reportChip.getJsonValue<std::string>("chipAdapter").c_str());
		m_pProjManager->InitManager();
		InitDrvParaPage();
		InitOperationPage(m_pProjManager->GetProjData()->getChipData().getJsonValue<std::string>("chipOperCfgJson"));

		std::string strChipType = reportChip.getJsonValue<std::string>("chipType");
		if (strChipType == "eMMC") {
			ui->pinCheck->setEnabled(false);
		}
		ui->checkID->setChecked(true);
		});
}

void ACProjectProperty::InitFilePage()
{
	ui->selectButton->setText(tr("Select"));
	ui->loadFileText->setText(tr("File Load"));
	ui->selectAllCheck->setText(tr("Select All"));
	ui->fileDeleteButton->setText(tr("Delete"));
	ui->clearBufferText->setText(tr("Clear Buffer"));
	ui->clBufferCheck->setText(tr("File size is more than buffer allowed will be failed"));

	// 隐藏水平表头
	ui->fileTableView->verticalHeader()->setVisible(false);
	ui->fileTableView->setMouseTracking(true);
	connect(ui->fileTableView, &AngKTableView::entered, this, [=](QModelIndex modelIdx) {
		if (!modelIdx.isValid()) {
			return;

		}
		QToolTip::showText(QCursor::pos(), modelIdx.data().toString());

		});

	//File Tag需要根据芯片的chipData进行区分与查看
	QStringList headList;
	headList << tr("Name") << tr("File Tag") << tr("Size") << tr("File Path") << tr("File Format") << tr("Auto Detecte Format")
		<< tr("Word Address Enable") << tr("Relcation Enable") << tr("Buffer Address") << tr("File Address") << tr("Load Length")
		<< tr("Swap Type") << tr("FileChecksumType") << tr("FileChecksum");

	m_pFileTableModel->setHorizontalHeaderLabels(headList);
	ui->fileTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	m_pProjManager->GetProjDataBuffer()->GetChipFile(chipData);

	ui->fileTableView->setModel(m_pFileTableModel.get());
	ui->fileTableView->setAlternatingRowColors(true);
	ui->fileTableView->horizontalHeader()->setHighlightSections(false);
	ui->fileTableView->horizontalHeader()->setStretchLastSection(true);
	ui->fileTableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	ui->fileTableView->horizontalHeader()->setMinimumSectionSize(100);

	QHeaderView* manuHead = ui->fileTableView->horizontalHeader();

	manuHead->setSectionResizeMode(QHeaderView::ResizeToContents);

	//测试代码
	//FileDataInfo data;
	//data.clear();
	//InsertData(data);
	//m_pTipMdl->setItem(iCurLine, 2, new QStandardItem(QString::fromLocal8Bit(pstTwo)));
	//m_pTipMdl->item(iCurLine, 2)->setTextAlignment(Qt::AlignCenter);

	//Clear Buffer Init
	ui->clBufferCheck->setChecked(true);
	ui->clBufferComboBox->addItem("Disable");
	ui->clBufferComboBox->addItem("0xFF");
	ui->clBufferComboBox->addItem("0x00");
	ui->clBufferComboBox->addItem("Default");

	connect(ui->selectButton, &QPushButton::clicked, this, &ACProjectProperty::onSlotFileSelect);

	connect(m_pFileImportDlg.get(), &AngKFilesImport::sgnSelectFileInfo, this, [=](FileDataInfo fInfo, bool isDoubleClick) {
		if (!isDoubleClick)
			InsertData(fInfo);
		else
			ReInsertData(fInfo);
		m_pFileImportDlg->clearDlg();
		m_pFileImportDlg->close();
		});
	m_pFileImportDlg->hide();

	connect(ui->selectAllCheck, &QCheckBox::stateChanged, this, [=](int state)
		{
			for (int i = 0; i < m_pFileTableModel->rowCount(); ++i)
			{
				m_pFileTableModel->item(i, 0)->setCheckState(Qt::CheckState(state));
			}
		});

	connect(ui->fileDeleteButton, &QPushButton::clicked, this, [=]()
		{
			//利用map存行数，自动排序
			QMap<int, int> rowMap;
			for (int i = 0; i < m_pFileTableModel->rowCount(); ++i)
			{
				if (m_pFileTableModel->item(i, 0) != nullptr) {//说明是非必须FileTag的档案
					if (m_pFileTableModel->item(i, 0)->checkState() == Qt::CheckState::Checked)
					{
						rowMap.insert(i, 0);
					}
				}
			}

			int rowToDel;
			QMapIterator<int, int> rowMapIterator(rowMap);
			rowMapIterator.toBack();
			while (rowMapIterator.hasPrevious())
			{
				rowMapIterator.previous();
				rowToDel = rowMapIterator.key();
				m_pFileTableModel->removeRow(rowToDel);

			}
		});

	connect(ui->fileTableView, &AngKTableView::doubleClicked, this, &ACProjectProperty::onSlotDoubleSetFileLoad);

	ui->emmcFileWidget->SetProjectData(m_pProjManager->GetProjData());
}

void ACProjectProperty::onSlotImportProject()
{
	QString ProjPath = QFileDialog::getOpenFileName(this, tr("Select Project File Load..."), QCoreApplication::applicationDirPath(), tr("eapr Files(*.eapr)"));
	if (ProjPath.isEmpty()) {
		ALOG_ERROR("Select Project File is Empty", "CU", "--");
		return;
	}

	//m_strProjName = QDir::toNativeSeparators(ProjPath);
	m_strProjName = ProjPath;
	//ui->addressLineEdit->setText(m_strProjName);

	int nImporRet = m_pProjManager->ImportProject(m_strProjName);
	if (nImporRet != 0) {
		m_pProjManager->ErrorClearFile();
		ACMessageBox::showError(this, QObject::tr("Message"), QObject::tr("Import project failed!"));
		ALOG_ERROR("Import project failed.", "CU", "--");
		return;
	}
	else {
		//PrintOpenLog();
		ACMessageBox::showInformation(this, QObject::tr("Message"), QObject::tr("Import project successfully!"));
		ALOG_INFO("Import project %s successfully!", "CU", "--", m_strProjName.toStdString().c_str());
		QFileInfo fInfo(m_strProjName);
		SetProjName(fInfo.fileName());
		emit sgnImportProjName(fInfo.fileName());
	}

	//加载工程数据
	ui->ChipPage->insertChipText(m_pProjManager->GetProjData()->getChipData());
	SetArchivesFileInfo();
	SetBufChecksumPage();
	ui->DrvParaPage->InsertCommonDrvPara(m_pProjManager->GetProjData()->GetCommonDrvParaJson());
	ui->DrvParaPage->InsertSelfDrvPara(m_pProjManager->GetProjData()->GetSelfDrvParaJson());
	ui->OperationPage->UI2ChipOper_ProjJson(m_pProjManager->GetProjData()->GetProjProperty().chipOperJson);
	ui->desEdit->setText(m_pProjManager->GetProjDescription());
}

void ACProjectProperty::onSlotSaveProject()
{
	//如果没有芯片信息直接报错，不能保存
	std::string strChipName = m_pProjManager->GetProjData()->getChipData().getJsonValue<std::string>("chipName");
	std::string strChipType = m_pProjManager->GetProjData()->getChipData().getJsonValue<std::string>("chipType");
	if (strChipName.empty())
	{
		ALOG_ERROR("Save project failed, Please select the chip before saving.", "CU", "--");
		ACMessageBox::showError(this, QObject::tr("Message"), QObject::tr("Save project failed, Please select the chip before saving."));
		return;
	}

	//工程保存需要的参数
	QString drvCommonJson, drvSelfJson;
	std::string chipOperateInfo;
	QString strPath = QDir::fromNativeSeparators(ui->addressLineEdit->text());
	QString strProjPwd = m_AdvSetting.sPasswd;
	int	nClearBufferType = ui->clBufferComboBox->currentIndex();
	QString binFileChk;

	//emmc智能分析和获取配置值
	std::string ImageFileFormat;
	std::string emmcOptonInfo;
	std::string emmcExtCSDInfo;
	eMMCTableHeader emmcTableHeaderProj;
	std::string emmcTableHeaderJson;
	if (strChipType == "eMMC") {
		ImageFileFormat = ui->emmcFileWidget->GetImageFileFormat();
		emmcOptonInfo = ui->emmcFileWidget->GeteMMCOption();
		emmcExtCSDInfo = ui->emmcFileWidget->GeteMMCExtCSDOption();
		if (IntelligentAnalyze(strPath, binFileChk) != 0) {
			ALOG_ERROR("Intelligent Analyze failed, Please check the file information.", "CU", "--");
			ACMessageBox::showError(this, QObject::tr("Message"), QObject::tr("Save project failed : Intelligent Analyze failed, Please check the file information."));
			return;
		}

		ui->emmcFileWidget->GetEMMCHeaderInfo(strPath, emmcTableHeaderProj, emmcTableHeaderJson, emmcExtCSDInfo);
	}

	if (!ui->emmcFileWidget->isExtCSDChecked()) {
		emmcExtCSDInfo = "{\"RegConfig\":[],\"PartitionSize\":[]}";
	}

	ui->DrvParaPage->GetDrvParaJson(drvCommonJson, drvSelfJson);
	ui->OperationPage->ChipOper2UI_ProjJson(chipOperateInfo);
	std::string chipBufferChkJson = GetChipBufferChkConfig();

	int nSaveRet = m_pProjManager->SaveProject(strPath, strProjPwd, binFileChk, nClearBufferType, ImageFileFormat
		, emmcOptonInfo, emmcExtCSDInfo, emmcTableHeaderJson, emmcTableHeaderProj
		, drvCommonJson.toStdString(), drvSelfJson.toStdString(), chipOperateInfo, chipBufferChkJson, ui->desEdit->text().toStdString());

	if (nSaveRet != 0) {
		ALOG_ERROR("Save project failed.", "CU", "--");
		ACMessageBox::showError(this, QObject::tr("Message"), QObject::tr("Save project failed."));
		return;
	}
	else {
		PrintSaveLog();
		ALOG_INFO("Save project %s successfully.", "CU", "--", strPath.toStdString().c_str());
		ACMessageBox::showInformation(this, QObject::tr("Message"), QObject::tr("Save project successfully."));
		emit sgnSaveSuccessClose();
	}

}

void ACProjectProperty::InitDrvParaPage()
{
	/*if (m_bCreateMode) */{
		ChipDataJsonSerial chipInfo = m_pProjManager->GetProjData()->getChipData();
		std::string chipName = chipInfo.getJsonValue<std::string>("chipAlgoFile");

		QString drvCommonJson;
		QString drvSelfJson;
		Utils::AngKCommonXMLParser::ParserChipDataXML((uint16_t)eSubCmdID::SubCmd_MU_SetDriverCommon, chipInfo.getJsonValue<ulong>("chipDrvParam"),
			m_pProjManager->GetProjDataBuffer()->GetChipFile(chipData), drvCommonJson, true);
		Utils::AngKCommonXMLParser::ParserChipDataXML((uint16_t)eSubCmdID::SubCmd_MU_SetDriverSelfPara, chipInfo.getJsonValue<ulong>("chipDrvParam"),
			m_pProjManager->GetProjDataBuffer()->GetChipFile(chipData), drvSelfJson, true);

		m_pProjManager->GetProjData()->SetCommonDrvParaJson(drvCommonJson);
		m_pProjManager->GetProjData()->SetSelfDrvParaJson(drvSelfJson);
	}

	ui->DrvParaPage->InsertCommonDrvPara(m_pProjManager->GetProjData()->GetCommonDrvParaJson());
	ui->DrvParaPage->InsertSelfDrvPara(m_pProjManager->GetProjData()->GetSelfDrvParaJson());
}

void ACProjectProperty::InitCheckBufferPage()
{
	ui->checkID->setText(tr("Check ID"));
	ui->pinCheck->setText(tr("Pin Check"));
	ui->chipOverLapCheck->setText(tr("Chip OverLap"));
	ui->masterCheck->setText(tr("Enable Master Copy"));
	ui->masterCheck->hide();
	ui->InsertModeText->setText(tr("Insetion Mode"));
	ui->typeText->setText(tr("Type"));
	ui->userRowGroup->setTitle(tr("User Row"));
	ui->startAddressText->setText(tr("Start Address"));
	ui->endAddressText->setText(tr("End Address"));
	ui->driverButton->setText(tr("Advance Setting"));
	//ui->driverButton->hide();
	ui->checkGroup->setTitle(tr("Check"));
	ui->UIDGroup->setTitle(tr("ReadUID"));
	ui->UIDSwap->setText(tr("UID Swap"));
	ui->UIDSwapByteText->setText(tr("Swap Byte"));
	ui->UIDSwapEdit->setPlaceholderText("0");

	ui->InsertModeComboBox->addItem(tr("Disable"), BufferCheckInsetionMode::Disable);
	ui->InsertModeComboBox->addItem(tr("Insetion Check"), BufferCheckInsetionMode::Insetion_Check);
	ui->InsertModeComboBox->addItem(tr("Auto_Sensing"), BufferCheckInsetionMode::Auto_Sensing);

	ui->typeComboBox->addItem(tr("Byte"), BufferCheckType::Byte);
	ui->typeComboBox->addItem(tr("Word"), BufferCheckType::Word);
	ui->typeComboBox->addItem(tr("CRC16"), BufferCheckType::CRC16);
	ui->typeComboBox->addItem(tr("CRC32"), BufferCheckType::CRC32);

	//ui->InsertModeComboBox->setCurrentIndex(1);
	ui->InsertModeComboBox->setCurrentIndex(0);
	ui->InsertModeComboBox->setEnabled(false);

	QRegExp reg("^[A-F0-9]{1,8}");
	ui->startAddressEdit->setValidator(new QRegExpValidator(reg, this));
	ui->endAddressEdit->setValidator(new QRegExpValidator(reg, this));
	ui->UIDSwapEdit->setValidator(new QRegExpValidator(reg, this));

	ui->startAddressEdit->setText("0");
	ui->endAddressEdit->setText("FF");

	connect(ui->driverButton, &QPushButton::clicked, this, &ACProjectProperty::onSlotDriverParamConfig);

	connect(ui->UIDSwap, &QCheckBox::stateChanged, this, [=](int state) {
		if (state == Qt::CheckState::Checked)
		{
			ui->UIDSwapEdit->setEnabled(true);
		}
		else if (state == Qt::CheckState::Unchecked)
		{
			ui->UIDSwapEdit->setEnabled(false);
		}
		});

	std::string strChipType = m_pProjManager->GetProjData()->getChipData().getJsonValue<std::string>("chipType");
	std::string strChipID = m_pProjManager->GetProjData()->getChipData().getJsonValue<std::string>("chipId");
	if (strChipType == "eMMC") {
		ui->pinCheck->setEnabled(false);
		if (strChipID == DEFAULT_CHIPID){
			ui->checkID->setChecked(false);
		}
	}
}

void ACProjectProperty::InitChipConfigPage()
{
	QString chipConfigXML = m_pProjManager->GetProjDataBuffer()->GetChipFile(chipConfig);

	ParserChipConfigXML(chipConfigXML, 10);

	ui->editEnableCheck->setText(tr("Edit Enable"));
	ui->showPwdCheck->setText(tr("Show Password"));
	ui->loadConfigButton->setText(tr("Load Config"));
	ui->saveConfigButton->setText(tr("Save Config"));

	connect(ui->loadConfigButton, &QPushButton::clicked, ui->chipConfigScrollAreaWidget, &AngKProjectChipConfigWidget::onSlotLoadConfigXML);
	connect(ui->saveConfigButton, &QPushButton::clicked, ui->chipConfigScrollAreaWidget, &AngKProjectChipConfigWidget::onSlotSaveConfigXML);

	ui->confirmButton->setText(tr("Confirm"));
	ui->confirmButton->hide();
}

void ACProjectProperty::InitOperationPage(std::string cfgJson)
{
	ui->OperationPage->SetProjDataSet(m_pProjManager->GetProjData());
	ui->OperationPage->CreateOperToolWgt(cfgJson);
	ShowFileTag();

	ui->OperationPage->SaveOperationCfg(m_pProjManager->GetProjData()->GetOperInfoList());
	//if (!m_bCreateMode)
	//	ui->OperationPage->UI2ChipOper_ProjJson(m_pProjManager->GetProjData()->GetProjProperty().chipOperJson);
}

void ACProjectProperty::InitOtherPage()
{
	ui->volTableGroup->setTitle(tr("VolTableInfo"));
	ui->volNameText->setText(tr("VolName"));
	ui->volCurText->setText(tr("VolCur"));
	ui->volUnitText->setText(tr("VolUnit"));
	ui->volMaxText->setText(tr("VolMax"));
	ui->volMinText->setText(tr("VolMin"));

	ui->drvExtParaGroup->setTitle(tr("DrvExtPara"));
	ui->paraNameText->setText(tr("ParaName"));
	ui->paraValueText->setText(tr("ParaValue"));

	ui->volNameEdit->setReadOnly(true);
	ui->volCurEdit->setReadOnly(true);
	ui->volUnitEdit->setReadOnly(true);
	ui->volMaxEdit->setReadOnly(true);
	ui->volMinEdit->setReadOnly(true);
	ui->paraNameEdit->setReadOnly(true);
	ui->paraValueEdit->setReadOnly(true);
}

void ACProjectProperty::InsertData(FileDataInfo& fileInfo)
{
	int row = m_pFileTableModel->rowCount();
	m_pFileTableModel->insertRow(m_pFileTableModel->rowCount());
	m_pFileTableModel->setData(m_pFileTableModel->index(row, FileName), QString::fromStdString(fileInfo.fileNameStr));
	m_pFileTableModel->item(row, FileName)->setCheckable(true);

	//m_fileTableModel->setData(m_fileTableModel->index(row, FileTag), QString::fromStdString(fileInfo.fileTagStr));
	m_pFileTableModel->setData(m_pFileTableModel->index(row, FileSize), QString::fromStdString(fileInfo.fileSizeStr));
	m_pFileTableModel->setData(m_pFileTableModel->index(row, FilePath), QString::fromStdString(fileInfo.filePathStr));
	m_pFileTableModel->setData(m_pFileTableModel->index(row, FileFormat), QString::fromStdString(fileInfo.fileFormatStr));
	m_pFileTableModel->setData(m_pFileTableModel->index(row, FileAutoDetecteFormat), QString::fromStdString(fileInfo.fileAutoDetecteFormatStr));
	m_pFileTableModel->setData(m_pFileTableModel->index(row, FileWordAddressEnable), QString::fromStdString(fileInfo.fileWordAddressEnable));
	m_pFileTableModel->setData(m_pFileTableModel->index(row, FileRelcationEnable), QString::fromStdString(fileInfo.fileRelcationEnable));
	m_pFileTableModel->setData(m_pFileTableModel->index(row, FileBufferAddress), QString::fromStdString(fileInfo.fileBufferAddressStr));
	m_pFileTableModel->setData(m_pFileTableModel->index(row, FileAddress), QString::fromStdString(fileInfo.fileAddressStr));
	m_pFileTableModel->setData(m_pFileTableModel->index(row, FileLoadLength), QString::fromStdString(fileInfo.fileLoadLengthStr));
	m_pFileTableModel->setData(m_pFileTableModel->index(row, FileSwapType), QString::fromStdString(fileInfo.fileSwapTypeStr));
	m_pFileTableModel->setData(m_pFileTableModel->index(row, FileChecksumType), QString::fromStdString(fileInfo.fileChecksumType));
	m_pFileTableModel->setData(m_pFileTableModel->index(row, FileChecksum), QString::fromStdString(fileInfo.fileChecksumValue));

	FileDataJsonSerial fileJson;
	fileJson.serialize(fileInfo);
	m_pProjManager->GetProjData()->getFileData().push_back(fileJson);
}

void ACProjectProperty::onSlotTurnPage()
{
	QPushButton* pageButton = qobject_cast<QPushButton*>(sender());
	if (pageButton == ui->preButton)
	{
		if (m_pCurrentIndex - 1 >= 0) {
			m_pCurrentIndex -= 1;
			ui->projTabWidget->setCurrentIndex(m_pCurrentIndex);
		}
	}
	else if (pageButton == ui->nextButton && CheckPageEvent())
	{
		if (m_pCurrentIndex + 1 <= ui->projTabWidget->count()) {
			m_pCurrentIndex += 1;
			ui->projTabWidget->setCurrentIndex(m_pCurrentIndex);
		}
	}

	ChangeTurnPageButton();
}

void ACProjectProperty::ReInsertData(FileDataInfo& fileInfo)
{
	QModelIndex currentSelectModel = ui->fileTableView->currentIndex();
	m_pFileTableModel->setData(m_pFileTableModel->index(currentSelectModel.row(), FileName), QString::fromStdString(fileInfo.fileNameStr));
	m_pFileTableModel->setData(m_pFileTableModel->index(currentSelectModel.row(), FileSize), QString::fromStdString(fileInfo.fileSizeStr));
	m_pFileTableModel->setData(m_pFileTableModel->index(currentSelectModel.row(), FilePath), QString::fromStdString(fileInfo.filePathStr));
	m_pFileTableModel->setData(m_pFileTableModel->index(currentSelectModel.row(), FileAutoDetecteFormat), QString::fromStdString(fileInfo.fileAutoDetecteFormatStr));
	m_pFileTableModel->setData(m_pFileTableModel->index(currentSelectModel.row(), FileWordAddressEnable), QString::fromStdString(fileInfo.fileWordAddressEnable));
	m_pFileTableModel->setData(m_pFileTableModel->index(currentSelectModel.row(), FileRelcationEnable), QString::fromStdString(fileInfo.fileRelcationEnable));
	m_pFileTableModel->setData(m_pFileTableModel->index(currentSelectModel.row(), FileBufferAddress), QString::fromStdString(fileInfo.fileBufferAddressStr));
	m_pFileTableModel->setData(m_pFileTableModel->index(currentSelectModel.row(), FileAddress), QString::fromStdString(fileInfo.fileAddressStr));
	m_pFileTableModel->setData(m_pFileTableModel->index(currentSelectModel.row(), FileLoadLength), QString::fromStdString(fileInfo.fileLoadLengthStr));
	m_pFileTableModel->setData(m_pFileTableModel->index(currentSelectModel.row(), FileSwapType), QString::fromStdString(fileInfo.fileSwapTypeStr));

	FileDataJsonSerial refileJson;
	refileJson.serialize(fileInfo);

	int testnum = m_pProjManager->GetProjData()->getFileData().size();
	if (currentSelectModel.row() <= m_pProjManager->GetProjData()->getFileData().size()) {
		m_pProjManager->GetProjData()->getFileData().erase(m_pProjManager->GetProjData()->getFileData().begin() + currentSelectModel.row());

		m_pProjManager->GetProjData()->getFileData().insert(m_pProjManager->GetProjData()->getFileData().begin() + currentSelectModel.row(), refileJson);
	}
}

void ACProjectProperty::FileWriteBuffer()
{
	ADR	  adrBuffOff;
	uchar byFillData = 0x00;
	uchar* byBuff = new uchar[BUFFER_RW_SIZE];
	bool useDefault = false;
	bool reInit = true;

	if (ui->clBufferComboBox->currentIndex() != 0) {
		if (ui->clBufferComboBox->currentIndex() == 1) {
			byFillData = 0xFF;
		}
		else if (ui->clBufferComboBox->currentIndex() == 2) {
			byFillData = 0x00;
		}
		else {
			useDefault = true;
		}
	}
	else {
		reInit = false;
	}

	if (reInit) {
		memset(byBuff, byFillData, BUFFER_RW_SIZE);
		//ALOG_INFO("<================ Initialize buffer ================>", "CU");
		std::vector<tPartitionInfo*> vPartitionBufInfo;
		m_pProjManager->GetProjDataBuffer()->GetPartitionInfo(vPartitionBufInfo);

		int partCnt = m_pProjManager->GetProjDataBuffer()->GetPartitionCount();
		for (int partIdx = 0; partIdx < partCnt; ++partIdx)
		{
			const tPartitionInfo* partInfo;
			partInfo = m_pProjManager->GetProjDataBuffer()->GetPartitionInfo(partIdx);

			if (!partInfo->PartitionShow)
				continue;

			int bufCnt = m_pProjManager->GetProjDataBuffer()->GetBufferCount(partIdx);
			for (int bufIdx = 0; bufIdx < bufCnt; ++bufIdx) {
				const tBufInfo* BufInfo;
				BufInfo = m_pProjManager->GetProjDataBuffer()->GetBufferInfo(bufIdx, partIdx);

				if (!BufInfo->m_bBufferShow)
					continue;

				if (useDefault) {
					memset(byBuff, BufInfo[bufIdx].uBufOrgValue, BUFFER_RW_SIZE);
				}
				else {
					memset(byBuff, byFillData, BUFFER_RW_SIZE);
				}

				for (adrBuffOff = BufInfo[bufIdx].llBufStart; adrBuffOff <= BufInfo[bufIdx].llBufEnd; adrBuffOff += BUFFER_RW_SIZE) {
					m_pProjManager->GetProjDataBuffer()->BufferWrite(adrBuffOff, byBuff, BUFFER_RW_SIZE);
				}
			}
		}
		//ALOG_INFO("<================ Initialize buffer complete. ================>", "CU");

		delete[] byBuff;
	}

	LoadFileBuffer();
}

void ACProjectProperty::LoadFileBuffer()
{
	AngKIOHeapManager IOMgr;
	IOMgr.setDataBuffer(m_pProjManager->GetProjDataBuffer());
	ADR WriteAdrMin, WriteAdrMax;
	for (int i = 0; i < m_pFileTableModel->rowCount(); ++i)
	{
		if (m_pFileTableModel->data(m_pFileTableModel->index(i, FileName)).toString().isEmpty())
			continue;

		m_pProjManager->GetProjDataBuffer()->ResetWriteRange();
		//QFile dataFile(Utils::AngKPathResolve::localTempFolderPath() + "IOHeap.bin");
		//FIXME just for XT Debug.
		//QString filePath = m_pFileTableModel->data(m_pFileTableModel->index(i, FilePath)).toString();
		//QFile dataFile(filePath);

		QFile dataFile(Utils::AngKPathResolve::localTempFolderPath() + "IOHeap.bin");
		m_pFileImportDlg->TransferFileFormat(m_pFileTableModel->data(m_pFileTableModel->index(i, FilePath)).toString(), dataFile);
		

		dataFile.seek(0);
		QByteArray pData = dataFile.readAll();
		int32_t dataLength = pData.size();
		dataFile.close();

		// 检查读取的数据是否为空
		if (dataLength == 0) {
			ALOG_ERROR("Read empty data from file: %s", "CU", "--", m_pFileTableModel->data(m_pFileTableModel->index(i, FilePath)).toString());
			return;
		}

		uint64_t offset = m_pFileTableModel->data(m_pFileTableModel->index(i, FileAddress)).toString().toULongLong(nullptr, 16);
		uint64_t bufferAddr = m_pFileTableModel->data(m_pFileTableModel->index(i, FileBufferAddress)).toString().toULongLong(nullptr, 16);

		int32_t retWrite = IOMgr.HeapWrite(bufferAddr, m_pProjManager->GetProjData()->getChipData().getJsonValue<ulong>("chipDrvParam"), IOChannel::Ethernet, DataArea::SSD, pData);
		ALOG_INFO("LoadFileBuffer %s dataLength 0x%X offset 0x%X bufferAddr 0x%X", "CU", "--", 
			m_pFileTableModel->data(m_pFileTableModel->index(i, FilePath)).toString(), dataLength, offset, bufferAddr);
		FileDataJsonSerial& fDataSerial = m_pProjManager->GetProjData()->getFileData()[i];
		FileDataInfo fileInfo;
		fDataSerial.deserialize(fileInfo);
		m_pProjManager->GetProjDataBuffer()->GetWriteRange(WriteAdrMin, WriteAdrMax);
		fileInfo.fileBufAddrWriteMin = WriteAdrMin;
		fileInfo.fileBufAddrWriteMax = WriteAdrMax;
		fDataSerial.serialize(fileInfo);

		if (fileInfo.fileSwapType != (int)AngKSwapSetting::SwapMode::NoMode) {
			if (SwapBuffer(fileInfo.fileSwapType, WriteAdrMin, WriteAdrMax) != 0) {
				ALOG_WARN("Swap data failed!", "CU", "--");
				return;
			}
		}
	}

	//检查是否存在地址重复覆盖的情况
	if (CheckFilesLoadAddressOverlap() == false) {
		//存在覆盖直接返回
		return;
	}
}

int ACProjectProperty::SwapBuffer(int swapType, ADR _writeAdrMin, ADR _writeAdrMax)
{
	uchar Virgin = m_pProjManager->GetProjDataBuffer()->GetVirgin();
	uint64_t adrOff = 9, BytesRead, uiReadLen;
	uchar* byBuff = new uchar[BUFFER_RW_SIZE];

	int partCnt = m_pProjManager->GetProjDataBuffer()->GetPartitionCount();
	for (int partIdx = 0; partIdx < partCnt; ++partIdx)
	{
		const tPartitionInfo* partInfo;
		partInfo = m_pProjManager->GetProjDataBuffer()->GetPartitionInfo(partIdx);

		if (!partInfo->PartitionShow)
			continue;

		int bufCnt = m_pProjManager->GetProjDataBuffer()->GetBufferCount(partIdx);
		for (int bufIdx = 0; bufIdx < bufCnt; ++bufIdx) {
			const tBufInfo* BufInfo;
			BufInfo = m_pProjManager->GetProjDataBuffer()->GetBufferInfo(bufIdx, partIdx);

			if (!BufInfo->m_bBufferShow)
				continue;

			adrOff = BufInfo->llBufStart;
			while (adrOff <= BufInfo->llBufEnd) {///压缩存储
				BytesRead = (BufInfo->llBufEnd + 1 - adrOff) > BUFFER_RW_SIZE ? BUFFER_RW_SIZE : (BufInfo->llBufEnd + 1 - adrOff);
				memset(byBuff, Virgin, BUFFER_RW_SIZE);
				uiReadLen = m_pProjManager->GetProjDataBuffer()->BufferRead(adrOff, byBuff, BytesRead);
				if (uiReadLen <= 0) {
					break;
				}

				if (swapType == AngKSwapSetting::SwapMode::ByteMode || swapType == AngKSwapSetting::SwapMode::WordMode) {
					for (int i = 0; i < uiReadLen - 1; i += 2) {
						uchar byTemp = byBuff[i];
						byBuff[i] = byBuff[i + 1];
						byBuff[i + 1] = byTemp;
					}
				}

				if (swapType == AngKSwapSetting::SwapMode::BWMode || swapType == AngKSwapSetting::SwapMode::WordMode) {
					for (int i = 0; i < uiReadLen - 3; i += 4) {
						uchar byTemp = byBuff[i];
						byBuff[i] = byBuff[i + 3];
						byBuff[i + 3] = byTemp;
						byTemp = byBuff[i + 1];
						byBuff[i + 1] = byBuff[i + 2];
						byBuff[i + 2] = byTemp;
					}
				}

				m_pProjManager->GetProjDataBuffer()->BufferWrite(adrOff, byBuff, uiReadLen);
				adrOff += uiReadLen;
			}
		}
	}

	delete[] byBuff;

	return 0;
}

bool ACProjectProperty::CheckFilesLoadAddressOverlap()
{
	bool IsOverLap = false;

	std::vector<FileDataJsonSerial> tempFileData = m_pProjManager->GetProjData()->getFileData();
	int vecSize = tempFileData.size();

	for (int i = 0; i < vecSize; ++i)
	{
		for (int j = i + 1; j < vecSize; ++j)
		{
			uint64_t _IdxBufAddrWriteMin = tempFileData[i].getJsonValue<uint64_t>("fileBufAddrWriteMin");
			uint64_t _IdxBufAddrWriteMax = tempFileData[i].getJsonValue<uint64_t>("fileBufAddrWriteMax");

			uint64_t _JdxBufAddrWriteMin = tempFileData[j].getJsonValue<uint64_t>("fileBufAddrWriteMin");
			uint64_t _JdxBufAddrWriteMax = tempFileData[j].getJsonValue<uint64_t>("fileBufAddrWriteMax");
			if (_JdxBufAddrWriteMin >= _JdxBufAddrWriteMax ||
				_JdxBufAddrWriteMax <= _IdxBufAddrWriteMin) {
				continue;
			}
			else {
				std::string iFileName = tempFileData[i].getJsonValue<std::string>("fileName");
				std::string jFileName = tempFileData[j].getJsonValue<std::string>("fileName");
				ALOG_FATAL("The range of File[%s] is overlapped with File[%s].", "CU", "--", iFileName.c_str(), jFileName.c_str());
				ALOG_FATAL("File[%s] Range=[0x%llu,0x%llu), File[%s] Range=[0x%llu,0x%llu).", "CU", "--",
					iFileName.c_str(), _IdxBufAddrWriteMin, _IdxBufAddrWriteMax,
					jFileName.c_str(), _JdxBufAddrWriteMin, _JdxBufAddrWriteMax);
				IsOverLap = true;
				goto __end;
			}
		}
	}

__end:
	return IsOverLap;
}

void ACProjectProperty::onSlotUpdateACXMLChipID(std::string acxmlChipID) {
	//更新checkID的状态
	m_pProjManager->GetProjData()->getChipData().setJsonValue<std::string>("chipIdACXML", acxmlChipID);
	std::string chipIDDB = m_pProjManager->GetProjData()->getChipData().getJsonValue<std::string>("chipId");
	if (acxmlChipID == "" && chipIDDB == DEFAULT_CHIPID) {
		ui->checkID->setEnabled(false);
		ui->checkID->setChecked(false);
	}
	else {
		ui->checkID->setEnabled(true);
		ui->checkID->setChecked(true);
	}
}

void ACProjectProperty::onSlotDoubleSetFileLoad(const QModelIndex& clickModel)
{
	bool bHaveTag = m_pFileImportDlg->GetFileTagHide();
	FileDataInfo selectFileInfo;
	if (bHaveTag)
		selectFileInfo.fileTagStr = m_pFileTableModel->data(m_pFileTableModel->index(clickModel.row(), FileTag)).toString().toStdString();

	selectFileInfo.fileFormatStr = m_pFileTableModel->data(m_pFileTableModel->index(clickModel.row(), FileFormat)).toString().toStdString();
	selectFileInfo.fileRelcationEnable = m_pFileTableModel->data(m_pFileTableModel->index(clickModel.row(), FileRelcationEnable)).toString().toStdString();
	selectFileInfo.fileBufferAddressStr = m_pFileTableModel->data(m_pFileTableModel->index(clickModel.row(), FileBufferAddress)).toString().toStdString();

	m_pFileImportDlg->SetFileDataInfo(selectFileInfo);
	m_pFileImportDlg->show();
	m_pFileImportDlg->raise();
}

void ACProjectProperty::onSlotFileSelect()
{
	if (!m_pFileImportDlg->isHidden())
		return;

	QDesktopWidget* m = QApplication::desktop();
	QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
	m_pFileImportDlg->move(desk_rect.width() / 2 - this->width() / 2 + desk_rect.left(), desk_rect.height() / 2 - this->height() / 2 + desk_rect.top());
	//m_fileImportDlg 要增加一个获取<FileRelocationSet> 字段中，通过解析chipdata.xml获取，并设置到界面中
	m_pFileImportDlg->show();
	m_pFileImportDlg->raise();
}

bool ACProjectProperty::ChipPageEvent()
{
	//FilePage需要根据eMMC界面变化
	ChipDataJsonSerial chipInfo = m_pProjManager->GetProjData()->getChipData();
	std::string chipName = chipInfo.DataJsonSerial()["chipName"];
	if (chipName.empty()) {
		ACMessageBox::showWarning(this, tr("Warning"), tr("Please choose a chip first!"), ACMessageBox::ACMsgButton::MSG_OK_BTN);
		return false;
	}
	if (!chipInfo.DataJsonSerial().empty() && chipInfo.DataJsonSerial()["chipType"] == "eMMC")
	{
		ui->emmcFileWidget->show();
		ui->normalFileWidget->hide();
		//ui->emmcFileWidget->SaveCurProgCheckState(m_mapProgCheck);
	}
	else
	{
		ui->emmcFileWidget->hide();
		ui->normalFileWidget->show();
	}

	return true;
}

bool ACProjectProperty::FilePageEvent()
{
	if (!ui->emmcFileWidget->isHidden()) {//EMMC档案检查
		if (ui->emmcFileWidget->GetFileCount() < 1) {
			auto ret = ACMessageBox::showWarning(this, tr("Warning"), tr("No files have been added. Do you want to continue ?"),
				ACMessageBox::ACMsgButton::MSG_OK_CANCEL_BTN);
			if (ret == ACMessageBox::ACMsgType::MSG_OK) {
				return true;
			}
			else {
				return false;
			}
		}
		else {
		}

	}
	else if (!ui->normalFileWidget->isHidden()) {//非EMMC档案检查
		if (m_pFileTableModel->rowCount() < 1) {
			auto ret = ACMessageBox::showWarning(this, tr("Warning"), tr("No files have been added. Do you want to continue ?"),
				ACMessageBox::ACMsgButton::MSG_OK_CANCEL_BTN);

			if (ret == ACMessageBox::ACMsgType::MSG_OK) {
				return true;
			}
			else {
				return false;
			}
		}

		//非EMMC选择档案之后进行写入buffer
		//ALOG_INFO("<================ File Import Begin ================>", "CU");
		FileWriteBuffer();
		//ALOG_INFO("<================ File Import End   ================>", "CU");
	}

	return true;
}

bool ACProjectProperty::ChipConfigPageEvent()
{
	return true;
}

bool ACProjectProperty::DrvParaPageEvent()
{
	return true;
}

bool ACProjectProperty::BufferChecksumPageEvent()
{
	//先添加小命令
	OpInfoList& addBufCheck = m_pProjManager->GetProjData()->GetOperInfoList();

	for (auto& opInfo : addBufCheck)
	{
		if (ui->checkID->isChecked()) {
			opInfo.vecOpList.push_back((int)ChipOperCfgSubCmdID::CheckID);
		}

		if (ui->pinCheck->isChecked()) {
			opInfo.vecOpList.push_back((int)ChipOperCfgSubCmdID::PinCheck);
		}

		//insertionCRC校验 TODO
		int insertMode = ui->InsertModeComboBox->currentData().toInt();
		if (insertMode == BufferCheckInsetionMode::Insetion_Check) {
			opInfo.vecOpList.push_back((int)ChipOperCfgSubCmdID::InsertionCheck);
		}
	}

	//校驗值類型這個(可透過 SetBPUAttribute來做)
	if (ui->InsertModeComboBox->currentData().toInt() == BufferCheckInsetionMode::Auto_Sensing) {
		m_nBPUAttribute |= 1 << 3;
	}
	if (ui->UIDGroup->isChecked()) {
		m_nBPUAttribute |= 1 << 2;
	}

	return true;
}

void ACProjectProperty::ShowFileTag()
{
	int rowCount = m_pFileTableModel->rowCount();
	m_pFileTableModel->removeRows(0, rowCount);

	std::vector<FileTagInfo> outTagVec;
	bool bHave = CheckFileTag(outTagVec);
	m_pFileImportDlg->SetFileTagHide(bHave);
	ui->fileTableView->hideColumn(1);
	if (bHave) {
		ui->fileTableView->showColumn(1);
		for (int i = 0; i < outTagVec.size(); ++i) {
			int row = m_pFileTableModel->rowCount();
			m_pFileTableModel->insertRow(m_pFileTableModel->rowCount());
			m_pFileTableModel->setData(m_pFileTableModel->index(row, FileTag), QString::fromStdString(outTagVec[i].sTagName));
			m_pFileTableModel->setData(m_pFileTableModel->index(row, FileFormat), QString::fromStdString(outTagVec[i].sFileType));
			m_pFileTableModel->setData(m_pFileTableModel->index(row, FileRelcationEnable), QString::fromStdString(outTagVec[i].sReloadEn));
			m_pFileTableModel->setData(m_pFileTableModel->index(row, FileBufferAddress), QString::fromStdString(outTagVec[i].sReloadAddr));

			FileDataJsonSerial fileJson;
			FileDataInfo dataInfo;
			dataInfo.fileTagStr = outTagVec[i].sTagName;
			dataInfo.fileFormatStr = outTagVec[i].sFileType;
			dataInfo.fileRelcationEnable = outTagVec[i].sReloadEn;
			dataInfo.fileBufferAddressStr = outTagVec[i].sReloadAddr;

			fileJson.serialize(dataInfo);
			m_pProjManager->GetProjData()->getFileData().push_back(fileJson);
		}
	}
}

bool ACProjectProperty::CheckFileTag(std::vector<FileTagInfo>& fileTagVec)
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(m_pProjManager->GetProjDataBuffer()->GetChipFile(chipData).toStdString().c_str());
	pugi::xml_node root_node = doc.child(XML_NODE_CHIPDATA);
	if (!result)
		return false;

	ulong uAlgo = m_pProjManager->GetProjData()->getChipData().getJsonValue<ulong>("chipDrvParam");

	pugi::xml_node fileInfoSet = root_node.child(XML_NODE_CHIPDATA_FILEINFOSET);
	pugi::xml_node fileInfoNode = fileInfoSet.child(XML_NODE_CHIPDATA_FILEINFO);
	int fileInfoCount = std::distance(fileInfoSet.begin(), fileInfoSet.end());
	for (int i = 0; i < fileInfoCount; ++i)
	{
		if (Utils::AngKCommonXMLParser::CheckAlgoRange(uAlgo, fileInfoNode.attribute("Algo").as_string()))
			break;

		fileInfoNode = fileInfoNode.next_sibling();
	}

	if (fileInfoNode == nullptr)
		return false;

	int fileTagCount = std::distance(fileInfoNode.begin(), fileInfoNode.end());
	pugi::xml_node fileTagNode = fileInfoNode.child(XML_NODE_CHIPDATA_FILETAG);

	for (int tagIdx = 0; tagIdx < fileTagCount; ++tagIdx) {

		nlohmann::json tagJson;
		FileTagInfo tagInfo;
		tagInfo.sTagName = fileTagNode.attribute("Name").as_string();
		tagInfo.sFileType = fileTagNode.attribute("FileType").as_string();
		tagInfo.sReloadEn = fileTagNode.attribute("ReloadEn").as_string();
		tagInfo.sReloadAddr = fileTagNode.attribute("ReloadAddr").as_string();

		fileTagVec.push_back(tagInfo);
		fileTagNode = fileTagNode.next_sibling();
	}

	if (!fileTagVec.empty())
		return true;

	return false;
}

bool ACProjectProperty::OperationPageEvent()
{
	ui->OperationPage->SaveOperationCfg(m_pProjManager->GetProjData()->GetOperInfoList());
	return true;
}

bool ACProjectProperty::SNCPageEvent()
{
	return true;
}

bool ACProjectProperty::OtherPageEvent()
{
	return true;
}

void ACProjectProperty::PrintSaveLog()
{
	ALOG_INFO("<================Option Configuration Begin ==============>", "CU", "--");
	std::string checkIDOpt = ui->checkID->isChecked() ? "Enabled" : "Disabled";
	std::string pinCheckOpt = ui->pinCheck->isChecked() ? "Enabled" : "Disabled";
	std::string chipOverLapCheckOpt = ui->chipOverLapCheck->isChecked() ? "Enabled" : "Disabled";
	//std::string insertionModeOpt = ui->InsertModeComboBox->currentText().toStdString();

	std::string insertionModeOpt;
	switch (ui->InsertModeComboBox->currentIndex())
	{
	case 0: insertionModeOpt = "Disable"; break;
	case 1: insertionModeOpt = "Insetion Check"; break;
	case 2: insertionModeOpt = "Auto_Sensing"; break;

	default:
		break;
	}


	std::string readUIDOpt = ui->UIDGroup->isChecked() ? "Enabled" : "Disabled";
	ALOG_INFO("Option : Check ID %s", "CU", "--", checkIDOpt.c_str());
	ALOG_INFO("Option : Pin Check %s", "CU", "--", pinCheckOpt.c_str());
	ALOG_INFO("Option : Chip OverLap %s", "CU", "--", chipOverLapCheckOpt.c_str());
	ALOG_INFO("Option : Insertion Mode %s", "CU", "--", insertionModeOpt.c_str());
	ALOG_INFO("Option : Read UID %s", "CU", "--", readUIDOpt.c_str());
	ALOG_INFO("<================Option Configuration End ==============>", "CU", "--");
}

std::string ACProjectProperty::GetChipBufferChkConfig()
{
	nlohmann::json bufferChkJson;
	bool bOk;
	bufferChkJson["CheckID"] = ui->checkID->isChecked();
	bufferChkJson["PinCheck"] = ui->pinCheck->isChecked();
	bufferChkJson["ChipOverLap"] = ui->chipOverLapCheck->isChecked();
	bufferChkJson["InsertionMode"] = ui->InsertModeComboBox->currentIndex();
	bufferChkJson["InsertionModeType"] = ui->typeComboBox->currentIndex();
	bufferChkJson["UserRowStartAddr"] = ui->startAddressEdit->text().toStdString();
	bufferChkJson["UserRowEndAddr"] = ui->endAddressEdit->text().toStdString();
	bufferChkJson["ReadUID"] = ui->UIDGroup->isChecked();
	if (ui->UIDGroup->isChecked()) {
		bufferChkJson["UIDSwap"] = ui->UIDSwap->isChecked();
		bufferChkJson["UIDSwapEdit"] = QString("%1").arg(ui->UIDSwapEdit->text().toLongLong(&bOk, 16)).toStdString();
	}

	return bufferChkJson.dump();
}

void ACProjectProperty::PrintOpenLog()
{
	QString strVersion = AngKGlobalInstance::ReadValue("Version", "BuildVer").toString();
	ALOG_INFO("Project Build version : %s", "CU", "--", strVersion.toStdString().c_str());
	QFileInfo fInfo(m_strProjName);
	QString timeFormat = fInfo.birthTime().toString("yyyy/MM/dd hh:mm:ss");
	ALOG_INFO("Project Build time : %s", "CU", "--", timeFormat.toStdString().c_str());
	QString hostName = QHostInfo::localHostName();
	ALOG_INFO("Project Author : %s", "CU", "--", hostName.toStdString().c_str());
	ALOG_INFO("Project Build on : AP9900", "CU", "--");
}

void ACProjectProperty::SetArchivesFileInfo()
{
	if (!ui->emmcFileWidget->isHidden()) {
		ui->emmcFileWidget->SetProjectData(m_pProjManager->GetProjData());
		ui->emmcFileWidget->SetArchivesFile();
	}
	else {

	}
}

void ACProjectProperty::SetBufChecksumPage()
{
	std::string strChipBufChkJson = m_pProjManager->GetProjData()->GetProjProperty().chipBufChkJson;
	try {
		auto BufChkJson = nlohmann::json::parse(strChipBufChkJson);

		ui->checkID->setChecked(BufChkJson["CheckID"]);
		ui->pinCheck->setChecked(BufChkJson["PinCheck"]);
		ui->chipOverLapCheck->setChecked(BufChkJson["ChipOverLap"]);
		ui->InsertModeComboBox->setCurrentIndex(BufChkJson["InsertionMode"]);
		ui->typeComboBox->setCurrentIndex(BufChkJson["InsertionModeType"]);
		ui->startAddressEdit->setText(QString::fromStdString(BufChkJson["UserRowStartAddr"]));
		ui->endAddressEdit->setText(QString::fromStdString(BufChkJson["UserRowEndAddr"]));
		ui->UIDGroup->setChecked(BufChkJson["ReadUID"]);
		if (ui->UIDGroup->isChecked()) {
			ui->UIDSwap->setChecked(BufChkJson["UIDSwap"]);
			ui->UIDSwapEdit->setText(QString::fromStdString(BufChkJson["UIDSwapEdit"]));
		}
	}
	catch (nlohmann::json::exception& e) {
		ALOG_FATAL("Parse ChipBufChk Json error: ", "CU", "--", e.what());
	}
}

int ACProjectProperty::ParserChipConfigXML(QString chipXml, uint32_t nAlgo)
{
	pugi::xml_document doc;
	const wchar_t* encodedName = reinterpret_cast<const wchar_t*>(chipXml.utf16());
	pugi::xml_parse_result result = doc.load_file(encodedName);
	pugi::xml_node root_node = doc.child(XML_NODE_CHIPCONFIG);
	nlohmann::json cmdJson;
	if (!result)
		return XMLMESSAGE_LOAD_FAILED;

	QString strDevName;

	//判断algo在哪个xml内
	pugi::xml_node deviceNode = root_node.child(XML_NODE_CHIPCONFIG_DEVICE);
	strDevName = deviceNode.attribute("DrvName").as_string();
	ui->chipConfigScrollAreaWidget->SetCurrentDevAlgo(strDevName, nAlgo);

	pugi::xml_node chipListNode = deviceNode.child(XML_NODE_CHIPCONFIG_CHIPLIST);
	pugi::xml_node chipNode = chipListNode.child(XML_NODE_CHIPCONFIG_CHIP);
	int chipListCount = std::distance(chipListNode.begin(), chipListNode.end());
	for (int i = 0; i < chipListCount; ++i) {
		if (Utils::AngKCommonXMLParser::CheckAlgoRange(nAlgo, chipNode.attribute("Algo").as_string())) {
			break;
		}

		chipNode = chipNode.next_sibling();
	}

	if (chipNode == nullptr)
		return XMLMESSAGE_LOAD_FAILED;

	//找到对应的Algo值XML回读构建UI
	pugi::xml_node propertysheetNode = chipNode.child(XML_NODE_CHIPCONFIG_PROPERTYSHEET);
	pugi::xml_node propertyNode = propertysheetNode.first_child();

	if (propertyNode == nullptr)
		return XMLMESSAGE_LOAD_FAILED;


	CreateXML2UI(propertyNode, ui->chipConfigScrollAreaWidget);

	return XMLMESSAGE_SUCCESS;
}

void ACProjectProperty::onSlotDriverParamConfig()
{
	m_pAdvSettingDlg->InitSetting(&m_AdvSetting);
	m_pAdvSettingDlg->show();
}

int ACProjectProperty::IntelligentAnalyze(const QString& proj_path, QString& strBytesum)
{
	return ui->emmcFileWidget->StartIntelligentAnalyze(proj_path, strBytesum);
}

void ACProjectProperty::CreateXML2UI(pugi::xml_node& element, QWidget* objParent)
{
	if (element == nullptr)
		return;

	if (element != nullptr && element.name() == strTabWidget)
	{
		QTabWidget* tabWgt;
		tabWgt = ui->chipConfigScrollAreaWidget->IsSubTabWgt(element.attribute("ObjectName").as_string());

		if (nullptr == tabWgt)
		{
			tabWgt = new QTabWidget(objParent);
			//ui->chipConfigScrollAreaWidget->AddSubWidget(tabWgt);
			ui->chipConfigScrollAreaWidget->XmlCreateTabWidget(tabWgt);
		}

		tabWgt->setProperty("type", strTabWidget);
		tabWgt->setProperty("name", element.attribute("ObjectName").as_string());
		//添加子窗口
		QWidget* tab = new QWidget();
		//QVBoxLayout* vlayout = new QVBoxLayout(tab);
		//tab->setLayout(vlayout);
		tab->setObjectName(QString::fromUtf8("tab") + QString::fromUtf8(element.attribute("Caption").as_string()));
		tabWgt->addTab(tab, QString());
		tabWgt->setTabText(tabWgt->count() - 1, element.attribute("Caption").as_string());
		tab->setProperty("caption", element.attribute("Caption").as_string());
		tab->setProperty("type", strTabWidget);
		tabWgt->setProperty("wrect", Utils::AngKChipConfigTools::String2Rect(element.attribute("wrect").as_string()));
		QRect showPos = Utils::AngKChipConfigTools::String2Rect(element.attribute("wrect").as_string());
		tabWgt->setGeometry(showPos);
		tabWgt->show();

		pugi::xml_node childElement = element.first_child();
		CreateXML2UI(childElement, tab);
	}
	else if (element != nullptr && element.name() == strTextEdit)
	{
		QTextEdit* aeditText = new QTextEdit(objParent);
		//QLayout* editLayout = objParent->layout();
		ui->chipConfigScrollAreaWidget->XmlCreateEditText(aeditText);

		aeditText->setProperty("type", strTextEdit);
		aeditText->setProperty("name", element.attribute("ObjectName").as_string());
		aeditText->setProperty("addr", Utils::AngKChipConfigTools::Hex_String2Int(element.attribute("addr").as_string()));
		aeditText->setProperty("len", Utils::AngKChipConfigTools::Hex_String2Int(element.attribute("len").as_string()));
		aeditText->setProperty("default", Utils::AngKChipConfigTools::Hex_String2Int(element.attribute("default").as_string()));
		aeditText->setProperty("ubuff", element.attribute("ubuff").as_bool());
		aeditText->setProperty("wrect", Utils::AngKChipConfigTools::String2Rect(element.attribute("wrect").as_string()));
		aeditText->setProperty("passwd", element.attribute("passwd").as_bool());
		aeditText->setProperty("valuetype", Utils::AngKChipConfigTools::EditVaule_CString2Int(element.attribute("valuetype").as_string()));
		aeditText->setProperty("shift", element.attribute("shift").as_string());
		aeditText->setProperty("showzerohead", element.attribute("showzerohead").as_bool());
		aeditText->setProperty("edian", Utils::AngKChipConfigTools::EditEndian_CString2Int(element.attribute("edian").as_string()));
		aeditText->setProperty("edited", element.attribute("edited").as_bool());
		QRect showPos = Utils::AngKChipConfigTools::String2Rect(element.attribute("wrect").as_string());
		aeditText->setText(element.text().as_string());
		aeditText->setGeometry(showPos);
		//editLayout->addWidget(aeditText);
		aeditText->show();
	}
	else if (element != nullptr && element.name() == strGroup)
	{
		QGroupBox* agb = new QGroupBox(objParent);
		//QLayout* agbLayout = objParent->layout();
		//QVBoxLayout* vlayout = new QVBoxLayout(agb);
		//agb->setLayout(vlayout);
		ui->chipConfigScrollAreaWidget->XmlCreateGroupBox(agb);

		agb->setProperty("type", strGroup);
		agb->setProperty("name", element.attribute("ObjectName").as_string());
		agb->setProperty("addr", Utils::AngKChipConfigTools::Hex_String2Int(element.attribute("addr").as_string()));
		agb->setProperty("len", Utils::AngKChipConfigTools::Hex_String2Int(element.attribute("len").as_string()));
		agb->setProperty("default", Utils::AngKChipConfigTools::Hex_String2Int(element.attribute("default").as_string()));
		agb->setProperty("ubuff", element.attribute("ubuff").as_bool());
		agb->setProperty("wrect", Utils::AngKChipConfigTools::String2Rect(element.attribute("wrect").as_string()));
		agb->setProperty("edited", element.attribute("edited").as_bool());
		QRect showPos = Utils::AngKChipConfigTools::String2Rect(element.attribute("wrect").as_string());
		agb->setTitle(element.attribute("ObjectName").as_string());
		agb->setObjectName(element.attribute("ObjectName").as_string());
		agb->setGeometry(showPos);
		//agbLayout->addWidget(agb);
		agb->show();

		pugi::xml_node childElement = element.first_child();
		CreateXML2UI(childElement, agb);
	}
	else if (element != nullptr && element.name() == strCheckBox)
	{
		QCheckBox* ackBox = new QCheckBox(objParent);
		//QLayout* chBoxlayout = objParent->layout();
		ui->chipConfigScrollAreaWidget->XmlCreateCheckBox(ackBox);

		ackBox->setProperty("type", strCheckBox);
		ackBox->setProperty("name", element.attribute("ObjectName").as_string());
		ackBox->setProperty("addr", Utils::AngKChipConfigTools::Hex_String2Int(element.attribute("addr").as_string()));
		ackBox->setProperty("len", Utils::AngKChipConfigTools::Hex_String2Int(element.attribute("len").as_string()));
		ackBox->setProperty("default", Utils::AngKChipConfigTools::Hex_String2Int(element.attribute("default").as_string()));
		ackBox->setProperty("ubuff", element.attribute("ubuff").as_bool());
		ackBox->setProperty("wrect", Utils::AngKChipConfigTools::String2Rect(element.attribute("wrect").as_string()));
		ackBox->setProperty("value", Utils::AngKChipConfigTools::Hex_String2Int(element.attribute("value").as_string()));
		ackBox->setProperty("mask", element.attribute("mask").as_int());
		ackBox->setProperty("edited", element.attribute("edited").as_bool());
		QRect showPos = Utils::AngKChipConfigTools::String2Rect(element.attribute("wrect").as_string());
		ackBox->setText(element.attribute("Name").as_string());
		ackBox->setGeometry(showPos);
		//chBoxlayout->addWidget(ackBox);
		ackBox->show();
	}
	else if (element != nullptr && element.name() == strComboBox)
	{
		QComboBox* acbBox = new QComboBox(objParent);
		//QLayout* cbBoxlayout = objParent->layout();
		ui->chipConfigScrollAreaWidget->XmlCreateComboBox(acbBox);

		acbBox->setProperty("type", strComboBox);
		acbBox->setProperty("name", element.attribute("ObjectName").as_string());
		acbBox->setProperty("addr", Utils::AngKChipConfigTools::Hex_String2Int(element.attribute("addr").as_string()));
		acbBox->setProperty("len", Utils::AngKChipConfigTools::Hex_String2Int(element.attribute("len").as_string()));
		acbBox->setProperty("default", Utils::AngKChipConfigTools::Hex_String2Int(element.attribute("default").as_string()));
		acbBox->setProperty("ubuff", element.attribute("ubuff").as_bool());
		acbBox->setProperty("wrect", Utils::AngKChipConfigTools::String2Rect(element.attribute("wrect").as_string()));
		acbBox->setProperty("mask", element.attribute("mask").as_int());
		acbBox->setProperty("edited", element.attribute("edited").as_bool());
		QRect showPos = Utils::AngKChipConfigTools::String2Rect(element.attribute("wrect").as_string());
		acbBox->setGeometry(showPos);
		//cbBoxlayout->addWidget(acbBox);
		acbBox->show();

		//comboBox比较特殊有子元素
		pugi::xml_node childElement = element.next_sibling();
		CreateXML2UI(childElement, acbBox);
		acbBox->setCurrentIndex(element.text().as_int());
	}
	else if (element != nullptr && element.name() == strLabel)
	{
		QLabel* alabel = new QLabel(objParent);
		//QLayout* labellayout = objParent->layout();
		ui->chipConfigScrollAreaWidget->XmlCreateLabel(alabel);

		alabel->setProperty("type", strLabel);
		alabel->setProperty("name", element.attribute("ObjectName").as_string());
		alabel->setProperty("wrect", Utils::AngKChipConfigTools::String2Rect(element.attribute("wrect").as_string()));
		QRect showPos = Utils::AngKChipConfigTools::String2Rect(element.attribute("wrect").as_string());
		alabel->setText(QString(element.text().as_string()));
		alabel->setGeometry(showPos);
		//labellayout->addWidget(alabel);
		alabel->show();
	}
	else if (element != nullptr && element.name() == "ComboItem")
	{
		QComboBox* acbBox = qobject_cast<QComboBox*>(objParent);
		QString name = element.attribute("name").as_string();
		int nname = Utils::AngKChipConfigTools::Hex_String2Int(element.attribute("value").as_string());
		acbBox->addItem(element.attribute("name").as_string());
		acbBox->setProperty(element.attribute("name").as_string(), Utils::AngKChipConfigTools::Hex_String2Int(element.attribute("value").as_string()));
	}

	element = element.next_sibling();
	CreateXML2UI(element, objParent);
}

void ACProjectProperty::MasterChipAnalyzeCreateEapr() {
	ui->ChipPage->onSlotSelectChip();
}