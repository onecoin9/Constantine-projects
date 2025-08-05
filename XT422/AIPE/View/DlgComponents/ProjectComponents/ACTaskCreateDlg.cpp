#include "ACTaskCreateDlg.h"
#include "ui_ACTaskCreateDlg.h"
#include "StyleInit.h"
#include "ACCmdPacket.h"
#include "AngKGlobalInstance.h"
#include "AngkLogger.h"
#include "ACEventLogger.h"
#include "ACMessageBox.h"
#include <QFileDialog>
#include <QToolTip>
#include <QTableWidget>

ACTaskCreateDlg::ACTaskCreateDlg(QWidget *parent, std::shared_ptr<ACTaskManager> _pTaskMgr)
	: AngKDialog(parent)
	, ui(new Ui::ACTaskCreateDlg())
	, m_pTaskManager(_pTaskMgr)
	, m_pButtonGroup(nullptr)
	, m_curModelIdx(QModelIndex())
	, m_pTaskTableModel(std::make_unique<QStandardItemModel>(this))
	, m_pAddTableButton(std::make_unique<QPushButton>(ui->projectMapping))
{
	this->setObjectName("ACTaskCreateDlg");
	QT_SET_STYLE_SHEET(objectName());

	ui->setupUi(setCentralWidget());

	this->setFixedSize(960, 650);
	this->SetTitle(tr("Task Manage"));

	InitText();
	InitButton();
	InitComboBox();
	InitTaskTable();
	InitBindGroup();
	InitAdapterButton();

	connect(m_pTaskManager.get(), &ACTaskManager::sgnErrorMessage, this, [this](QString title, QString msg) {
		ACMessageBox::showError(this, title, msg);
		return;
	}, Qt::UniqueConnection);

	connect(ui->projectMapping->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this]() {
		auto chooseRows = ui->projectMapping->selectionModel()->selectedRows();
		if (chooseRows.size() <= 0 || (chooseRows.size() == 1 && chooseRows.at(0).row() == m_pTaskTableModel->rowCount() - 1))
			ui->okButton->setText(tr("Add New"));
		else
			ui->okButton->setText(tr("Bind"));

		return;

		}, Qt::UniqueConnection);

	ui->saveButton->setFocus();
}

ACTaskCreateDlg::~ACTaskCreateDlg()
{
	delete ui;
}

void ACTaskCreateDlg::InitText()
{
	ui->tableTitle->setText(tr("Project & Program Mapping"));
	ui->taskPathText->setText(tr("Task Path:"));
	ui->taskNameText->setText(tr("Task Name:"));
	ui->openButton->setText(tr("Open..."));
	ui->saveButton->setText(tr("Save as..."));

	ui->projectGroup->setTitle(tr("Project Bind Setting"));
	ui->bindGroup->setTitle(tr("Bind"));

	ui->projPathText->setText(tr("Project Select:"));
	ui->bindProgAliasBtn->setText(tr("Bind Prog Alias"));
	ui->bindProgSNBtn->setText(tr("Bind Prog SN"));
	ui->bindALLProgBtn->setText(tr("Bind ALL Prog"));

	ui->doubleCheckBox->setText(tr("Double Adapter"));
	ui->okButton->setText(tr("Bind"));

	ui->taskPathEdit->setReadOnly(true);
	ui->projPathEdit->setReadOnly(true);

	ResetTaskPath();
}

void ACTaskCreateDlg::InitButton()
{
	ui->doubleCheckBox->setChecked(true);

	connect(this, &ACTaskCreateDlg::sgnClose, this, &ACTaskCreateDlg::close);
	connect(ui->openButton, &QPushButton::clicked, this, &ACTaskCreateDlg::onSlotOpenTaskPath);
	connect(ui->saveButton, &QPushButton::clicked, this, &ACTaskCreateDlg::onSlotSaveTaskPath);
	connect(ui->projPathButton, &QPushButton::clicked, this, &ACTaskCreateDlg::onSlotBindProject);
	connect(ui->doubleCheckBox, &QCheckBox::clicked, this, &ACTaskCreateDlg::onSlotSwitchAdapterShow);
	connect(ui->okButton, &QPushButton::clicked, this, &ACTaskCreateDlg::onSlotBindTask2Proj);
}

void ACTaskCreateDlg::InitAdapterButton()
{
	m_vecBPUButton.push_back(ui->BPU0_SKT1_Btn);
	m_vecBPUButton.push_back(ui->BPU0_SKT2_Btn);
	m_vecBPUButton.push_back(ui->BPU1_SKT1_Btn);
	m_vecBPUButton.push_back(ui->BPU1_SKT2_Btn);
	m_vecBPUButton.push_back(ui->BPU2_SKT1_Btn);
	m_vecBPUButton.push_back(ui->BPU2_SKT2_Btn);
	m_vecBPUButton.push_back(ui->BPU3_SKT1_Btn);
	m_vecBPUButton.push_back(ui->BPU3_SKT2_Btn);
	m_vecBPUButton.push_back(ui->BPU4_SKT1_Btn);
	m_vecBPUButton.push_back(ui->BPU4_SKT2_Btn);
	m_vecBPUButton.push_back(ui->BPU5_SKT1_Btn);
	m_vecBPUButton.push_back(ui->BPU5_SKT2_Btn);
	m_vecBPUButton.push_back(ui->BPU6_SKT1_Btn);
	m_vecBPUButton.push_back(ui->BPU6_SKT2_Btn);
	m_vecBPUButton.push_back(ui->BPU7_SKT1_Btn);
	m_vecBPUButton.push_back(ui->BPU7_SKT2_Btn);

	for (int i = 0; i < m_vecBPUButton.size(); ++i) {
		m_vecBPUButton[i]->setProperty("BPUValue", BPUCalVector[i]);
		m_vecBPUButton[i]->setChecked(true);
	}
}

void ACTaskCreateDlg::InitComboBox()
{
	ui->allSelectComboBox->addItem(tr("Select ALL"), (int)ChooseType::Select_All);
	ui->allSelectComboBox->addItem(tr("Deselect All"), (int)ChooseType::Select_None);
	ui->allSelectComboBox->addItem(tr("Select Invert"), (int)ChooseType::Select_Invert);

	connect(ui->allSelectComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &ACTaskCreateDlg::onSlotSwitchSelect);
}

void ACTaskCreateDlg::InitBindGroup()
{
	m_pButtonGroup = new QButtonGroup(this);
	connect(m_pButtonGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &ACTaskCreateDlg::onSlotSwitchBindSelect);

	m_pButtonGroup->addButton(ui->bindProgAliasBtn, (int)BindProgType::Bind_Prog_Alias);
	m_pButtonGroup->addButton(ui->bindProgSNBtn, (int)BindProgType::Bind_Prog_SN);
	m_pButtonGroup->addButton(ui->bindALLProgBtn, (int)BindProgType::Bind_ALL_Prog);
	ui->bindALLProgBtn->setChecked(true);
	ui->bindProgAliasEdit->setEnabled(false);
	ui->bindProgSNEdit->setEnabled(false);

	//connect(ui->projectGroup, &QGroupBox::clicked, this, &ACTaskCreateDlg::onSlotEditProj);
}

void ACTaskCreateDlg::InitTaskTable()
{
	// 隐藏水平表头
	ui->projectMapping->verticalHeader()->setVisible(false);
	ui->projectMapping->setMouseTracking(true);
	connect(ui->projectMapping, &AngKTableView::entered, this, [=](QModelIndex modelIdx) {
		if (!modelIdx.isValid()) {
			return;

		}
		QToolTip::showText(QCursor::pos(), modelIdx.data().toString());

		});

	QStringList headList;
	headList << tr("Project Path") << tr("Corresponding programmer alias/SN") << tr("Adapter Count") << tr("Adapter Position") << tr("Operate");

	m_pTaskTableModel->setHorizontalHeaderLabels(headList);
	ui->projectMapping->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui->projectMapping->setModel(m_pTaskTableModel.get());
	ui->projectMapping->setAlternatingRowColors(true);
	ui->projectMapping->horizontalHeader()->setHighlightSections(false);
	ui->projectMapping->horizontalHeader()->setStretchLastSection(true);
	ui->projectMapping->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	ui->projectMapping->horizontalHeader()->setMinimumSectionSize(100);

	QHeaderView* manuHead = ui->projectMapping->horizontalHeader();

	manuHead->setSectionResizeMode(QHeaderView::Custom);

	ui->projectMapping->setColumnWidth(0, 290);
	ui->projectMapping->setColumnWidth(1, 250);
	ui->projectMapping->setColumnWidth(2, 125);
	ui->projectMapping->setColumnWidth(3, 125);
	ui->projectMapping->setColumnWidth(4, 110);

	connect(ui->projectMapping, &AngKTableView::clicked, this, &ACTaskCreateDlg::onSlotSetProjectBind);

	//初始化一个默认空行
	QIcon imageIcon(QPixmap(":/Skin/Light/PushButton/defaultAddButton.svg"));
	m_pAddTableButton->setFlat(true);
	m_pAddTableButton->setIcon(imageIcon);
	m_pAddTableButton->setIconSize(QSize(20, 20));
	m_pAddTableButton->setObjectName("defaultAddButton");
	m_pAddTableButton->installEventFilter(this);

	m_pTaskTableModel->insertRow(m_pTaskTableModel->rowCount());
	ui->projectMapping->setIndexWidget(m_pTaskTableModel->index(m_pTaskTableModel->rowCount() - 1, 4), m_pAddTableButton.get());
	connect(m_pAddTableButton.get(), &QPushButton::clicked, this, &ACTaskCreateDlg::onSlotAddTaskTableRow);

	// 默认添加一行
	onSlotAddTaskTableRow();
	ui->projectMapping->selectRow(0);

}

void ACTaskCreateDlg::SetDevBindInfo(QString devBind)
{
	ui->bindProgAliasEdit->clear();
	ui->bindProgSNEdit->clear();
	QString toFind = "-";
	int pos = devBind.indexOf(toFind);

	int nBindProgType = (int)BindProgType::Bind_ALL_Prog;
	if (pos != -1) {
		// 找到了"-"，截取之后的字符串
		QString afterDash = devBind.mid(pos + toFind.length());
		QString beforeDash = devBind.mid(0, pos);
		if (beforeDash == "SN")
		{
			nBindProgType = (int)BindProgType::Bind_Prog_SN;
			ui->bindProgSNEdit->setText(afterDash);
		}
		else if (beforeDash == "Alias")
		{
			nBindProgType = (int)BindProgType::Bind_Prog_Alias;
			ui->bindProgAliasEdit->setText(afterDash);
		}
	}
	else {
		nBindProgType = (int)BindProgType::Bind_ALL_Prog;
	}	
	m_pButtonGroup->button(nBindProgType)->setChecked(true);
}

void ACTaskCreateDlg::SetAdapterIndex(QString adapterIdx)
{
	bool ok;
	int decimalValue = adapterIdx.toInt(&ok, 16); // 指定基数为 16

	for (int i = 0; i < m_vecBPUButton.size(); ++i) {
		bool bCheck = m_vecBPUButton[i]->property("BPUValue").toInt() & decimalValue;
		m_vecBPUButton[i]->setChecked(bCheck);
	}
}

QString ACTaskCreateDlg::GetDevBindInfo()
{
	QString retDevStr;
	switch ((BindProgType)m_pButtonGroup->checkedId())
	{
	case ACTaskCreateDlg::BindProgType::Bind_Prog_Alias:
		retDevStr = "Alias-" + ui->bindProgAliasEdit->text();
		break;
	case ACTaskCreateDlg::BindProgType::Bind_Prog_SN:
		retDevStr = "SN-" + ui->bindProgSNEdit->text();
		break;
	case ACTaskCreateDlg::BindProgType::Bind_ALL_Prog:
		retDevStr = "ALL";
		break;
	default:
		break;
	}
	
	return retDevStr;
}

QString ACTaskCreateDlg::GetAdapterIndex()
{
	int nBPU_CalNum = 0;
	for (int i = 0; i < m_vecBPUButton.size(); ++i) {
		if (m_vecBPUButton[i]->isChecked()) {
			nBPU_CalNum += m_vecBPUButton[i]->property("BPUValue").toInt();
		}
	}
	QString swHex = QString("0x%1").arg(nBPU_CalNum, 4, 16, QLatin1Char('0'));
	return swHex;
}

void ACTaskCreateDlg::SetTaskInfo(std::vector<TaskProperty>& _taskVec)
{
	//清空原先其他行
	m_pTaskTableModel->removeRows(0, m_pTaskTableModel->rowCount() - 1);

	for (int i = 0; i < _taskVec.size(); ++i) {

		AddTaskTableRow(QString::fromStdString(_taskVec[i].taskPath), QString::fromStdString(_taskVec[i].devBindInfo)
			, QString::fromStdString(_taskVec[i].doubleCheck), QString::fromStdString(_taskVec[i].adpValue));

	}
}

bool ACTaskCreateDlg::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == QEvent::HoverEnter)
	{
		QPushButton* tableButton = qobject_cast<QPushButton*>(watched);
		if (tableButton)
			tableButton->setCursor(Qt::PointingHandCursor);
	}
	else if (event->type() == QEvent::HoverLeave)
	{
		QPushButton* tableButton = qobject_cast<QPushButton*>(watched);
		if (tableButton)
			tableButton->setCursor(Qt::ArrowCursor);
	}

	return QWidget::eventFilter(watched, event);
}

void ACTaskCreateDlg::AddTaskTableRow(QString _projPath, QString _bindInfo, QString _doubleCheck, QString _adpValue)
{
	int insertRow = m_pTaskTableModel->rowCount() - 1;
	m_pTaskTableModel->insertRow(insertRow);

	m_pTaskTableModel->setData(m_pTaskTableModel->index(insertRow, 0), _projPath);
	m_pTaskTableModel->setData(m_pTaskTableModel->index(insertRow, 1), _bindInfo);
	m_pTaskTableModel->setData(m_pTaskTableModel->index(insertRow, 2), _doubleCheck);
	m_pTaskTableModel->setData(m_pTaskTableModel->index(insertRow, 3), _adpValue);
	m_pTaskTableModel->setData(m_pTaskTableModel->index(insertRow, 4), "");

	QPushButton* DelButton = new QPushButton(ui->projectMapping);
	QIcon imageIcon(QPixmap(":/Skin/Light/PushButton/defaultDelButton.svg"));
	DelButton->setFlat(true);
	DelButton->setIcon(imageIcon);
	DelButton->setIconSize(QSize(20, 20));
	DelButton->setObjectName("defaultAddButton");
	DelButton->installEventFilter(this);
	ui->projectMapping->setIndexWidget(m_pTaskTableModel->index(insertRow, 4), DelButton);
	connect(DelButton, &QPushButton::clicked, this, &ACTaskCreateDlg::onSlotDeleteTableRow);
}

void ACTaskCreateDlg::ResetTaskPath()
{
	m_strRecordTaskPath = AngKGlobalInstance::instance()->ReadValue("TaskProjectPath", "taskPath").toString();
	m_strRecordProjPath = AngKGlobalInstance::instance()->ReadValue("TaskProjectPath", "projPath").toString();
	ui->taskPathEdit->setText(m_strRecordTaskPath);
	ui->taskNameEdit->setText("default.actask");
}

void ACTaskCreateDlg::SetTaskPath()
{
	QFileInfo pFileInfo(m_strRecordTaskPath);
	ui->taskNameEdit->setText(pFileInfo.fileName());
	ui->taskPathEdit->setText(pFileInfo.path());
}

void ACTaskCreateDlg::onSlotOpenTaskPath()
{
	QString openTaskFile = QFileDialog::getOpenFileName(this, "Open Task File...", m_strRecordTaskPath, tr("Task File(*.actask)"));
	if (!openTaskFile.isEmpty()) {
		m_strRecordTaskPath = openTaskFile;
		std::vector<TaskProperty> taskPropVec;
		int retLoad = m_pTaskManager->OpenTask(m_strRecordTaskPath, taskPropVec);
		if (retLoad == 0) {
			SetTaskPath();
			SetTaskInfo(taskPropVec);
		}
		else {
			ACMessageBox::showWarning(this, tr("Open Task Warning"), tr("Open task failed!"));
		}
	}
}

void ACTaskCreateDlg::onSlotSaveTaskPath()
{
	m_strRecordTaskPath = QFileDialog::getSaveFileName(this, "Save Task File...", ui->taskPathEdit->text(), tr("Task File(*.actask)"));
	if (m_strRecordTaskPath.isEmpty())
		return;

	//if (m_strRecordTaskPath.indexOf(".actask") == m_strRecordTaskPath.length() - sizeof(".actask")) {
	//	m_strRecordTaskPath = m_strRecordTaskPath.mid(0, m_strRecordTaskPath.length() - sizeof(".actask"));
	//}

	QString tmpStr = m_strRecordTaskPath.mid(m_strRecordTaskPath.lastIndexOf('/') + 1);
	if (tmpStr.length() - sizeof(".actask") + 1 > 156) {
		ACMessageBox::showError(this, tr("Task Error"), tr("Create task failed, the task name cannot exceed 156 bytes."));
		ALOG_INFO("Create task failed, task name cannot exceed 156 bytes.", "CU", "--");
		return;
	}

	int retCreate = m_pTaskManager->CreateTask(m_pTaskTableModel, m_strRecordTaskPath);
	if (retCreate == 0) {
		SetTaskPath();

		nlohmann::json SaveTaskJson;
		SaveTaskJson["FilePath"] = m_strRecordTaskPath.toStdString();
		SaveTaskJson["RetCode"] = retCreate;
		SaveTaskJson["RetInfo"] = retCreate != 0 ? "Save Task Failed" : "Save Task Success";
		EventLogger->SendEvent(EventBuilder->GetSaveTask(SaveTaskJson));

		ALOG_INFO("Create task file : %s successfully.", "CU", "--", m_strRecordTaskPath.toStdString().c_str());
		ACMessageBox::showInformation(this, tr("Task Info"), tr("Create task file successfully."));
	}
	else {
		switch ((XmlMessageType)retCreate)
		{
		case XMLMESSAGE_PROJ_EMPTY_FAILED:
			ACMessageBox::showError(this, tr("Task Error"), tr("Create task file failed. There are projects that have not been bound."));
			break;
		case XMLMESSAGE_CRC_FAILED:
			ACMessageBox::showError(this, tr("Task Error"), tr("Create task file failed. CRC verification error in the project"));
			break;
		default:
			break;
		}
	}
}

void ACTaskCreateDlg::onSlotBindProject()
{
	QString strProjPath = QFileDialog::getOpenFileName(this, "Select Bind Project File...", m_strRecordProjPath, tr("Project File(*.eapr)"));
	if (!strProjPath.isEmpty()) {
		m_strRecordProjPath = strProjPath;
		ui->projPathEdit->setText(m_strRecordProjPath);
	}
}

void ACTaskCreateDlg::onSlotSwitchAdapterShow(bool bCheck)
{
	if (!bCheck)
	{
		for (int i = 1; i < m_vecBPUButton.size(); i += 2) {
			m_vecBPUButton[i]->hide();
		}
	}
	else {
		for (int i = 0; i < m_vecBPUButton.size(); i++) {
			m_vecBPUButton[i]->show();
		}
	}
}

void ACTaskCreateDlg::onSlotBindTask2Proj()
{
	int lastRow = m_pTaskTableModel->rowCount() - 1;
	QModelIndex currentModel = ui->projectMapping->currentIndex();
	QString projPath = QDir::fromNativeSeparators(ui->projPathEdit->text());
	QString devBindInfo = GetDevBindInfo();
	QString AdpValue = GetAdapterIndex();

	QString doubleAdp;
	ui->doubleCheckBox->isChecked() ? doubleAdp = "D" : doubleAdp = "S";
	auto chooseRows = ui->projectMapping->selectionModel()->selectedRows();
	if (chooseRows.size() <= 0 || (chooseRows.size() == 1 && chooseRows.at(0).row() == m_pTaskTableModel->rowCount() - 1)) {
		//AddTaskTableRow("", devBindInfo, doubleAdp, "0xffff");

		//校验是否有绑定重复的
		bool bOk = false;
		int bindAdpValue = AdpValue.toInt(&bOk, 16);
		for (int i = 0; i < lastRow; ++i) {

			QString checkDoubleAdp = m_pTaskTableModel->data(m_pTaskTableModel->index(i, 2)).toString();
			if (checkDoubleAdp != doubleAdp) {
				ACMessageBox::showError(this, tr("Task Error"), tr("Binding failed, single seat and double seat heads cannot be bound together."));
				return;
			}

			int nAdpValue = m_pTaskTableModel->data(m_pTaskTableModel->index(i, 3)).toString().toInt(&bOk, 16);
			if (nAdpValue & bindAdpValue) {
				ACMessageBox::showError(this, tr("Task Error"), tr("Binding failed, there is a conflict in the BPU bound to the project."));
				return;
			}
		}
		AddTaskTableRow(projPath, devBindInfo, doubleAdp, AdpValue);
	}
	else {
		if (projPath.isEmpty()) {
			return;
		}

		//校验是否有绑定重复的
		bool bOk = false;
		int bindAdpValue = AdpValue.toInt(&bOk, 16);
		for (int i = 0; i < lastRow; ++i) {
			m_curModelIdx = ui->projectMapping->currentIndex();
			if(i == m_curModelIdx.row())
				continue;

			QString checkDoubleAdp = m_pTaskTableModel->data(m_pTaskTableModel->index(i, 2)).toString();
			if (checkDoubleAdp != doubleAdp) {
				ACMessageBox::showError(this, tr("Task Error"), tr("Binding failed, single seat and double seat heads cannot be bound together."));
				return;
			}

			int nAdpValue = m_pTaskTableModel->data(m_pTaskTableModel->index(i, 3)).toString().toInt(&bOk, 16);
			if (nAdpValue & bindAdpValue) {
				ACMessageBox::showError(this, tr("Task Error"), tr("Binding failed, there is a conflict in the BPU bound to the project."));
				return;
			}
		}

		m_curModelIdx = chooseRows.at(0);
		m_pTaskTableModel->setData(m_pTaskTableModel->index(m_curModelIdx.row(), 0), projPath);
		m_pTaskTableModel->setData(m_pTaskTableModel->index(m_curModelIdx.row(), 1), devBindInfo);
		m_pTaskTableModel->setData(m_pTaskTableModel->index(m_curModelIdx.row(), 2), doubleAdp);
		m_pTaskTableModel->setData(m_pTaskTableModel->index(m_curModelIdx.row(), 3), AdpValue);
	}
}

void ACTaskCreateDlg::onSlotSwitchSelect(int nChooseType)
{
	switch ((ChooseType)nChooseType)
	{
	case ACTaskCreateDlg::ChooseType::Select_All:
	{
		for (int i = 0; i < m_vecBPUButton.size(); i++) {
			m_vecBPUButton[i]->setChecked(true);
		}
	}
		break;
	case ACTaskCreateDlg::ChooseType::Select_None:
	{
		for (int i = 0; i < m_vecBPUButton.size(); i++) {
			m_vecBPUButton[i]->setChecked(false);
		}
	}
		break;
	case ACTaskCreateDlg::ChooseType::Select_Invert:
	{
		for (int i = 0; i < m_vecBPUButton.size(); i++) {
			m_vecBPUButton[i]->setChecked(!m_vecBPUButton[i]->isChecked());
		}
	}
		break;
	default:
		break;
	}
}

void ACTaskCreateDlg::onSlotSwitchBindSelect(int nBind)
{
	switch ((BindProgType)nBind)
	{
	case ACTaskCreateDlg::BindProgType::Bind_Prog_Alias:
	{
		ui->bindProgAliasEdit->setEnabled(true);
		ui->bindProgSNEdit->setEnabled(false);
	}
		break;
	case ACTaskCreateDlg::BindProgType::Bind_Prog_SN:
	{
		ui->bindProgAliasEdit->setEnabled(false);
		ui->bindProgSNEdit->setEnabled(true);
	}
		break;
	case ACTaskCreateDlg::BindProgType::Bind_ALL_Prog:
	{
		ui->bindProgAliasEdit->setEnabled(false);
		ui->bindProgSNEdit->setEnabled(false);
	}
		break;
	default:
		break;
	}
}

void ACTaskCreateDlg::onSlotAddTaskTableRow()
{
	AddTaskTableRow("", "ALL", "D", "0xffff");
}

void ACTaskCreateDlg::onSlotDeleteTableRow()
{
	QModelIndex currentModel = ui->projectMapping->currentIndex();

	m_pTaskTableModel->removeRow(currentModel.row());
}

void ACTaskCreateDlg::onSlotSetProjectBind(const QModelIndex& nIndex)
{
	int lastRow = m_pTaskTableModel->rowCount() - 1;
	if (nIndex.row() == lastRow) {
		return;
	}

	//ui->projectGroup->setChecked(false);
	m_curModelIdx = nIndex;

	QString projPath = m_pTaskTableModel->data(m_pTaskTableModel->index(nIndex.row(), 0)).toString();
	if (!projPath.isEmpty()) {
		m_strRecordProjPath = projPath;
	}
	ui->projPathEdit->setText(projPath);

	QString devBindInfo = m_pTaskTableModel->data(m_pTaskTableModel->index(nIndex.row(), 1)).toString();
	SetDevBindInfo(devBindInfo);

	QString doubleAdp = m_pTaskTableModel->data(m_pTaskTableModel->index(nIndex.row(), 2)).toString();
	bool bDouble = false;
	doubleAdp == "D" ? bDouble = true : bDouble = false;
	onSlotSwitchAdapterShow(bDouble);

	QString AdpValue = m_pTaskTableModel->data(m_pTaskTableModel->index(nIndex.row(), 3)).toString();
	SetAdapterIndex(AdpValue);
}

void ACTaskCreateDlg::onSlotEditProj(bool bCheck)
{
	if (bCheck) {
		int lastRow = m_pTaskTableModel->rowCount() - 1;
		QModelIndex currentModel = ui->projectMapping->currentIndex();
		if (currentModel.row() < 0 || currentModel.row() == lastRow) {

			//ui->projectGroup->setChecked(!bCheck);
			ACMessageBox::showError(this, tr("Edit Error"), tr("Please first set the project to be edited"));
			return;
		}
	}

}


void ACTaskCreateDlg::CreateMasterAnalyzeTask() {
	m_pTaskTableModel->setData(m_pTaskTableModel->index(0, 0), "C:/JWL/AG06/code/AIPE/Build/project/default.eapr");

	QString tmpTskFilePath = "C:/JWL/AG06/code/AIPE/Build/project/1.actask";
	int retCreate = m_pTaskManager->CreateTask(m_pTaskTableModel, tmpTskFilePath);
	if (retCreate == 0) {
		SetTaskPath();

		nlohmann::json SaveTaskJson;
		SaveTaskJson["FilePath"] = tmpTskFilePath.toStdString();
		SaveTaskJson["RetCode"] = retCreate;
		SaveTaskJson["RetInfo"] = retCreate != 0 ? "Save Task Failed" : "Save Task Success";
		EventLogger->SendEvent(EventBuilder->GetSaveTask(SaveTaskJson));

		ALOG_INFO("Create task file : %s successfully.", "CU", "--", tmpTskFilePath.toStdString().c_str());
		ACMessageBox::showInformation(this, tr("Task Info"), tr("Create task file successfully."));
	}
}