#include "setting.h"
#include "ui_setting.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

settingDialog::settingDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::settingDialog)
    , settings("AcroView", "acroViewTester")
{
    ui->setupUi(this);
    initUI();
    setupConnections();
    if (!loadSettings()) {
        QMessageBox::warning(this, "配置文件不存在", "配置文件未找到，请检查路径或创建一个新的配置文件。");
        initDefaultValues();
    }
    

}

settingDialog::~settingDialog()
{
    delete ui;
}

void settingDialog::initUI()
{
    // 设置窗口标题和大小
    setWindowTitle(tr("设置"));
    setFixedSize(800, 600);

    // 初始化场景选择下拉框
    ui->comboBoxOsScene->addItem(tr("老化测试"));
    ui->comboBoxOsScene->addItem(tr("AG06"));
    ui->comboBoxOsScene->addItem(tr("AP8000"));
    ui->comboBoxOsScene->setCurrentIndex(1);
    // 初始化模式选择下拉框
    ui->comboBoxOsMode->addItem(tr("MES模式"));
    ui->comboBoxOsMode->addItem(tr("本地模式"));

    // 初始化自动机类型下拉框
    ui->comboBoxAutoType->addItem(tr("IPS5800S"));
    ui->comboBoxAutoType->addItem(tr("自动机2"));

    // 初始化执行指令下拉框
    ui->comboBoxExecCmd->addItem(tr("指令1"));
    ui->comboBoxExecCmd->addItem(tr("指令2"));

    int rows = settings.value("Cursor/Rows", 2).toInt();
    int cols = settings.value("Cursor/Cols", 1).toInt();

    ui->lineEditSiteRow->setText(QString::number(rows));
    ui->lineEditSiteCol->setText(QString::number(cols));
    connect(ui->lineEditSiteRow, &QLineEdit::editingFinished,
        this, &settingDialog::onGridSizeChanged);
    connect(ui->lineEditSiteCol, &QLineEdit::editingFinished,
        this, &settingDialog::onGridSizeChanged);

    QString currentScene = settings.value("Scene/CurrentScene", "AG06").toString();
    ui->comboBoxOsScene->setCurrentText(currentScene);
    updateUIForScene(currentScene);
}

void settingDialog::accept()
{
    QSettings settings("AcroView", "acroViewTester");

    int rows = ui->lineEditSiteRow->text().toInt();
    int cols = ui->lineEditSiteCol->text().toInt();
    int baseRow = ui->lineEditBaseRow->text().toInt();
    int baseCol = ui->lineEditBaseCol->text().toInt();
    int siteDirection = ui->comboBoxViewDirect->currentIndex();
   
    settings.setValue("Grid/SiteRows", rows);
    settings.setValue("Grid/SiteCols", cols);
    settings.setValue("Grid/BaseRows", baseRow);
    settings.setValue("Grid/BaseCols", baseCol);
    settings.setValue("Grid/SiteDirection", siteDirection);
    saveSettings();
    settings.sync();
    emit sceneChanged(ui->comboBoxOsScene->currentText());
    emit gridSizeChanged(rows, cols, baseRow, baseCol);
    emit settingsChanged();
    QDialog::accept();
}

void settingDialog::saveSettings()
{
    QSettings settings("AcroView", "acroViewTester");
    QStringList keys = settings.allKeys();
    settings.setValue("Scene/CurrentScene", ui->comboBoxOsScene->currentText());
    settings.setValue("Paths/LogPath", ui->lineEditLogPath->text());
    settings.setValue("Paths/LogTransPath", ui->lineEditLogTransPath->text());
    settings.setValue("Paths/ReportPath", ui->lineEditReportPath->text());
    settings.setValue("Paths/AutoPath", ui->lineEditAutoPath->text());
    settings.setValue("Paths/MesPath", ui->lineEditMesPath->text());
    settings.setValue("Paths/RecipePath", ui->lineEditRecipePath->text());
    settings.setValue("Paths/ProjectPath", ui->lineEditProjectPath->text());
    settings.setValue("Paths/Aprog2Path", ui->lineEditAprog2Path->text());
    settings.setValue("Paths/MultiAprogPath", ui->lineEditMultiAprogPath->text());
}

void settingDialog::setupConnections()
{
    // 连接场景选择变化信号
    connect(ui->comboBoxOsScene, &QComboBox::currentTextChanged,
        this, &settingDialog::onSceneChanged);

    // 连接所有浏览按钮的点击信号
    connect(ui->btnLogPath, &QPushButton::clicked, this, &settingDialog::onBrowseButtonClicked);
    connect(ui->btnLogTransPath, &QPushButton::clicked, this, &settingDialog::onBrowseButtonClicked);
    connect(ui->btnReportPath, &QPushButton::clicked, this, &settingDialog::onBrowseButtonClicked);
    connect(ui->btnAutoPath, &QPushButton::clicked, this, &settingDialog::onBrowseButtonClicked);
    connect(ui->btnMesPath, &QPushButton::clicked, this, &settingDialog::onBrowseButtonClicked);
    connect(ui->btnRecipePath, &QPushButton::clicked, this, &settingDialog::onBrowseButtonClicked);
    connect(ui->btnProjectPath, &QPushButton::clicked, this, &settingDialog::onBrowseButtonClicked);
    connect(ui->btnAprog2Path, &QPushButton::clicked, this, &settingDialog::onBrowseButtonClicked);
    connect(ui->btnMultiAprogPath, &QPushButton::clicked, this, &settingDialog::onBrowseButtonClicked);

    // 连接确定按钮的点击信号
    connect(ui->btnOK, &QPushButton::clicked, this, &settingDialog::onSaveSettings);
}

void settingDialog::initDefaultValues()
{
    ui->lineEditLogPath->setText(settings.value("Paths/LogPath", "D:\\Mes\\ProjSave").toString());
    ui->lineEditLogTransPath->setText(settings.value("Paths/LogTransPath", "D:\\Mes\\ProjSave").toString());
    ui->lineEditReportPath->setText(settings.value("Paths/ReportPath", "D:\\Mes\\Report").toString());
    ui->lineEditAutoPath->setText(settings.value("Paths/AutoPath", "c:\\IPS\\TaskData").toString());
    ui->lineEditMesPath->setText(settings.value("Paths/MesPath", "http://127.0.0.1:8088/mes").toString());
    ui->lineEditRecipePath->setText(settings.value("Paths/RecipePath", "D:\\Recipe").toString());
    ui->lineEditProjectPath->setText(settings.value("Paths/ProjectPath", "D:\\Project").toString());
    ui->lineEditAprog2Path->setText(settings.value("Paths/Aprog2Path", "D:\\Aprog2").toString());
    ui->lineEditMultiAprogPath->setText(settings.value("Paths/MultiAprogPath", "D:\\MultiAprog").toString());
    ui->lineEditHandle1->setText(settings.value("Handle1", "192.168.1.100").toString());
    ui->lineEditHandle2->setText(settings.value("Handle2", "64100").toString());
    ui->lineEditAlarmServer1->setText(settings.value("AlarmServer1", "127.0.0.1").toString());
    ui->lineEditAlarmServer2->setText(settings.value("AlarmServer2", "5000").toString());
    ui->lineEditLocalPort->setText(settings.value("LocalPort", "64101").toString());
    ui->lineEditStation->setText(settings.value("Station/CurrentStation", "1").toString());

    // 坐标设置默认值
    ui->lineEditSiteRow->setText(settings.value("Grid/SiteRows", "10").toString());
    ui->lineEditSiteCol->setText(settings.value("Grid/SiteCols", "5").toString());
    ui->lineEditBaseRow->setText(settings.value("Grid/BaseRows", "1").toString());
    ui->lineEditBaseCol->setText(settings.value("Grid/BaseCols", "1").toString());
    ui->comboBoxViewDirect->setCurrentIndex(settings.value("Grid/SiteDirection").toInt());

    // 时间设置默认值
    ui->lineEditCommTimeout->setText(settings.value("CommTimeout", "1").toString());
    ui->lineEditCmdInterval->setText(settings.value("CmdInterval", "5").toString());
    ui->lineEditAuthInterval->setText(settings.value("AuthInterval", "5").toString());
    ui->lineEditAutoLoginInterval->setText(settings.value("AutoLoginInterval", "5").toString());
    ui->lineEditPowerCheckInterval->setText(settings.value("PowerCheckInterval", "5").toString());
    ui->lineEditDataRefreshInterval->setText(settings.value("DataRefreshInterval", "1").toString());
}

bool settingDialog::loadSettings()
{
    QString configFilePath = QCoreApplication::applicationDirPath() + "/file/settings.ini";
    if (!QFile::exists(configFilePath)) 
    {
        return false;
        initDefaultValues();
    }

    QSettings settings(configFilePath, QSettings::IniFormat);
    QSettings settings1("AcroView", "acroViewTester");
    QStringList keys0 = settings.allKeys();
    QStringList keys1 = settings1.allKeys();
    for (const QString& key : keys1) {
        settings1.remove(key);
    }
    for (const QString& key : keys0) {
        QVariant value = settings.value(key);
        settings1.setValue(key, value);
    }
    settings1.sync();
    keys1 = settings1.allKeys();
    ui->lineEditLogPath->setText(settings.value("Paths/LogPath").toString());
    ui->lineEditLogTransPath->setText(settings.value("Paths/LogTransPath").toString());
    ui->lineEditReportPath->setText(settings.value("Paths/ReportPath").toString());
    ui->lineEditAutoPath->setText(settings.value("Paths/AutoPath").toString());
    ui->lineEditMesPath->setText(settings.value("Paths/MesPath").toString());
    ui->lineEditRecipePath->setText(settings.value("Paths/RecipePath").toString());
    ui->lineEditProjectPath->setText(settings.value("Paths/ProjectPath").toString());
    ui->lineEditAprog2Path->setText(settings.value("Paths/Aprog2Path").toString());
    ui->lineEditMultiAprogPath->setText(settings.value("Paths/MultiAprogPath").toString());
    ui->lineEditHandle1->setText(settings.value("Handle1").toString());
    ui->lineEditHandle2->setText(settings.value("Handle2").toString());
    ui->lineEditAlarmServer1->setText(settings.value("AlarmServer1").toString());
    ui->lineEditAlarmServer2->setText(settings.value("AlarmServer2").toString());
    ui->lineEditLocalPort->setText(settings.value("LocalPort").toString());
    ui->lineEditStation->setText(settings.value("Station/CurrentStation").toString());

    ui->lineEditSiteRow->setText(settings.value("Grid/SiteRows").toString());
    ui->lineEditSiteCol->setText(settings.value("Grid/SiteCols").toString());
    ui->lineEditBaseRow->setText(settings.value("Grid/BaseRows").toString());
    ui->lineEditBaseCol->setText(settings.value("Grid/BaseCols").toString());
    ui->comboBoxViewDirect->setCurrentIndex(settings.value("Grid/SiteDirection").toInt());
    ui->lineEditCommTimeout->setText(settings.value("CommTimeout").toString());
    ui->lineEditCmdInterval->setText(settings.value("CmdInterval").toString());
    ui->lineEditAuthInterval->setText(settings.value("AuthInterval").toString());
    ui->lineEditAutoLoginInterval->setText(settings.value("AutoLoginInterval").toString());
    ui->lineEditPowerCheckInterval->setText(settings.value("PowerCheckInterval").toString());
    ui->lineEditDataRefreshInterval->setText(settings.value("DataRefreshInterval").toString());
    return true;
}

void settingDialog::onSceneChanged(const QString& scene)
{
    currentScene = scene;
    emit sceneChanged(ui->comboBoxOsScene->currentText());
    updateUIForScene(scene);
}

void settingDialog::updateUIForScene(const QString& scene)
{
    hideAllControls();

    if (scene == "老化测试") {
        showAgingTestControls();
    }
    else if (scene == "AG06") {
        showAG06Controls();
    }
    else if (scene == "AP8000") {
        showAP8000Controls();
    }
}

void settingDialog::on_comboBoxScene_currentTextChanged(const QString& text)
{
    emit sceneChanged(text);
}

void settingDialog::hideAllControls()
{
    ui->labelAutoType->hide();
    ui->comboBoxAutoType->hide();
    ui->labelExecCmd->hide();
    ui->comboBoxExecCmd->hide();
    ui->labelStation->hide();
    ui->lineEditStation->hide();
    ui->labelProjectPath->hide();
    ui->lineEditProjectPath->hide();
    ui->btnProjectPath->hide();
    ui->labelAprog2Path->hide();
    ui->lineEditAprog2Path->hide();
    ui->btnAprog2Path->hide();
    ui->labelMultiAprogPath->hide();
    ui->lineEditMultiAprogPath->hide();
    ui->btnMultiAprogPath->hide();

    // 隐藏右侧坐标和时间设置
    bool isAgingTest = (currentScene == "老化测试");
    setCoordTimeControlsVisible(isAgingTest);
}


void settingDialog::setCoordControlsVisible(bool visible) {
    ui->labelCursor->setVisible(visible);
    ui->lineEditSiteRow->setVisible(visible);
    ui->labelCursorRow->setVisible(visible);
    ui->lineEditSiteCol->setVisible(visible);
    ui->labelCursorCol->setVisible(visible);

    ui->labelViewDirect->setVisible(visible);
    ui->comboBoxViewDirect->setVisible(visible);

    ui->labelMouse->setVisible(visible);
    ui->lineEditBaseRow->setVisible(visible);
    ui->labelMouseRow->setVisible(visible);
    ui->lineEditBaseCol->setVisible(visible);
    ui->labelMouseCol->setVisible(visible);
    ui->groupBoxAgingTest->setVisible(visible);
}

void settingDialog::setTimeControlsVisible(bool visible) {
    ui->labelCommTimeout->setVisible(visible);
    ui->lineEditCommTimeout->setVisible(visible);
    ui->labelCommTimeoutUnit->setVisible(visible);

    ui->labelCmdInterval->setVisible(visible);
    ui->lineEditCmdInterval->setVisible(visible);
    ui->labelCmdIntervalUnit->setVisible(visible);

    ui->labelAuthInterval->setVisible(visible);
    ui->lineEditAuthInterval->setVisible(visible);
    ui->labelAuthIntervalUnit->setVisible(visible);

    ui->labelAutoLoginInterval->setVisible(visible);
    ui->lineEditAutoLoginInterval->setVisible(visible);
    ui->labelAutoLoginIntervalUnit->setVisible(visible);

    ui->labelPowerCheckInterval->setVisible(visible);
    ui->lineEditPowerCheckInterval->setVisible(visible);
    ui->labelPowerCheckIntervalUnit->setVisible(visible);

    ui->labelDataRefreshInterval->setVisible(visible);
    ui->lineEditDataRefreshInterval->setVisible(visible);
    ui->labelDataRefreshIntervalUnit->setVisible(visible);

    
    ui->groupBoxComm->setVisible(visible);
}

void settingDialog::setCoordTimeControlsVisible(bool visible) 
{
    setCoordControlsVisible(visible);
    setTimeControlsVisible(visible);
    adjustSize();
}


void settingDialog::showAgingTestControls()
{
    ui->labelLogPath->show();
    ui->lineEditLogPath->show();
    ui->btnLogPath->show();
    ui->labelLogTransPath->show();
    ui->lineEditLogTransPath->show();
    ui->btnLogTransPath->show();
    ui->labelReportPath->show();
    ui->lineEditReportPath->show();
    ui->btnReportPath->show();
    ui->labelAutoPath->show();
    ui->lineEditAutoPath->show();
    ui->btnAutoPath->show();
    ui->labelMesPath->show();
    ui->lineEditMesPath->show();
    ui->btnMesPath->show();
    ui->labelRecipePath->show();
    ui->lineEditRecipePath->show();
    ui->btnRecipePath->show();
    ui->labelHandle->show();
    ui->lineEditHandle1->show();
    ui->lineEditHandle2->show();
    ui->labelAlarmServer->show();
    ui->lineEditAlarmServer1->show();
    ui->lineEditAlarmServer2->show();
    ui->labelLocalPort->show();
    ui->lineEditLocalPort->show();
    setCoordTimeControlsVisible(true);
}

void settingDialog::showAG06Controls()
{
    showAgingTestControls();

    // 显示AG06特有控件
    ui->labelAutoType->show();
    ui->comboBoxAutoType->show();
    ui->labelExecCmd->show();
    ui->comboBoxExecCmd->show();
    ui->labelStation->show();
    ui->lineEditStation->show();
    ui->labelProjectPath->show();
    ui->lineEditProjectPath->show();
    ui->btnProjectPath->show();
    ui->labelAprog2Path->show();
    ui->lineEditAprog2Path->show();
    ui->btnAprog2Path->show();

    // 隐藏右侧坐标和时间设置
    setCoordControlsVisible(true);
    setTimeControlsVisible(false);
    adjustSize();

}

void settingDialog::showAP8000Controls()
{
    showAgingTestControls();

    // 显示AP8000特有控件
    ui->labelAutoType->show();
    ui->comboBoxAutoType->show();
    ui->labelExecCmd->show();
    ui->comboBoxExecCmd->show();
    ui->labelStation->show();
    ui->lineEditStation->show();
    ui->labelProjectPath->show();
    ui->lineEditProjectPath->show();
    ui->btnProjectPath->show();
    ui->labelMultiAprogPath->show();
    ui->lineEditMultiAprogPath->show();
    ui->btnMultiAprogPath->show();

    // 隐藏右侧坐标和时间设置
    setCoordTimeControlsVisible(false);
}

void settingDialog::onBrowseButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    QString lineEditName = button->objectName().replace("btn", "lineEdit");
    QLineEdit* lineEdit = findChild<QLineEdit*>(lineEditName);
    if (!lineEdit) return;

    QString path;
    if (button->objectName() == "btnAutoPath") {
        path = QFileDialog::getOpenFileName(
            this,
            tr("选择文件"),
            lineEdit->text(),
            tr("ACSTask Files (*.actask);;BIN Files (*.bin)")
        );
    }
    else {
        path = QFileDialog::getExistingDirectory(
            this,
            tr("选择路径"),
            lineEdit->text(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );
    }
    if (!path.isEmpty()) {
        lineEdit->setText(path);
    }
}

void settingDialog::onSaveSettings()
{
    QString configPath = QCoreApplication::applicationDirPath() + "/file/settings.ini";
    QSettings settings(configPath, QSettings::IniFormat);

    settings.setValue("Scene/CurrentScene", ui->comboBoxOsScene->currentText());
    settings.setValue("Mode/CurrentMode", ui->comboBoxOsMode->currentText());
    settings.setValue("AutoType/CurrentAutoType", ui->comboBoxAutoType->currentText());
    settings.setValue("ExecCmd/CurrentExecCmd", ui->comboBoxExecCmd->currentText());
    settings.setValue("Station/CurrentStation", ui->lineEditStation->text());

    // 保存路径设置
    settings.setValue("Paths/LogPath", ui->lineEditLogPath->text());
    settings.setValue("Paths/LogTransPath", ui->lineEditLogTransPath->text());
    settings.setValue("Paths/ReportPath", ui->lineEditReportPath->text());
    settings.setValue("Paths/AutoPath", ui->lineEditAutoPath->text());
    settings.setValue("Paths/MesPath", ui->lineEditMesPath->text());
    settings.setValue("Paths/RecipePath", ui->lineEditRecipePath->text());
    settings.setValue("Paths/ProjectPath", ui->lineEditProjectPath->text());
    settings.setValue("Paths/Aprog2Path", ui->lineEditAprog2Path->text());
    settings.setValue("Paths/MultiAprogPath", ui->lineEditMultiAprogPath->text());

    // 保存网络设置
    settings.setValue("Handle1", ui->lineEditHandle1->text());
    settings.setValue("Handle2", ui->lineEditHandle2->text());
    settings.setValue("AlarmServer1", ui->lineEditAlarmServer1->text());
    settings.setValue("AlarmServer2", ui->lineEditAlarmServer2->text());
    settings.setValue("LocalPort", ui->lineEditLocalPort->text());

    // 保存坐标设置
    settings.setValue("Grid/SiteDirection", ui->comboBoxViewDirect->currentIndex());
    settings.setValue("Grid/SiteRows", ui->lineEditSiteRow->text());
    settings.setValue("Grid/SiteCols", ui->lineEditSiteCol->text());
    settings.setValue("Grid/BaseRows", ui->lineEditBaseRow->text());
    settings.setValue("Grid/BaseCols", ui->lineEditBaseCol->text());

    // 保存时间间隔设置
    settings.setValue("CommTimeout", ui->lineEditCommTimeout->text());
    settings.setValue("CmdInterval", ui->lineEditCmdInterval->text());
    settings.setValue("AuthInterval", ui->lineEditAuthInterval->text());
    settings.setValue("AutoLoginInterval", ui->lineEditAutoLoginInterval->text());
    settings.setValue("PowerCheckInterval", ui->lineEditPowerCheckInterval->text());
    settings.setValue("DataRefreshInterval", ui->lineEditDataRefreshInterval->text());

    // 显示保存成功提示
    QMessageBox::information(this, tr("提示"), tr("设置已保存"));

    // 发送设置改变信号
    emit settingsChanged();

    // 关闭对话框
    accept();
}

void settingDialog::onGridSizeChanged()
{
    bool ok1, ok2,ok3,ok4;
    int rows = ui->lineEditSiteRow->text().toInt(&ok1);
    int columns = ui->lineEditSiteCol->text().toInt(&ok2);
    int baseRows = ui->lineEditBaseRow->text().toInt(&ok3);
    int baseColumns = ui->lineEditBaseCol->text().toInt(&ok4);
    if (ok1 && ok2 && rows > 0 && columns > 0) {
        emit gridSizeChanged(rows, columns,baseRows,baseColumns);
    }
}