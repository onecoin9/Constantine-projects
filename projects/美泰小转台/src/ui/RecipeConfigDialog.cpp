#include "ui/RecipeConfigDialog.h"
#include "core/Logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QGroupBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QComboBox>
#include <QTableWidget>
#include <QTabWidget>
#include <QHeaderView>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QAbstractItemView>
#include <QTimer>
#include <QSettings>
#include <QTextCodec>
#include <QCoreApplication>
#include <QRegExp>
#include <QDir>

namespace Presentation {

RecipeConfigDialog::RecipeConfigDialog(QWidget *parent)
    : QDialog(parent)
    , m_recipeNameEdit(nullptr)
    , m_siteGroupsEdit(nullptr)
    , m_routerFileEdit(nullptr)
    , m_cmd3PathEdit(nullptr)
    , m_taskFileNameEdit(nullptr)
    , m_tabWidget(nullptr)
    , m_chipTypeComboBox(nullptr)
    , m_activationParamsTable(nullptr)
    , m_addParamButton(nullptr)
    , m_removeParamButton(nullptr)
    , m_saveParamButton(nullptr)
    , m_routerBrowseButton(nullptr)
    , m_cmd3BrowseButton(nullptr)
    , m_taskFileBrowseButton(nullptr)
    , m_saveButton(nullptr)
    , m_cancelButton(nullptr)
    , m_loadActestButton(nullptr)
{
    setWindowTitle("编辑配方");
    setWindowIcon(QIcon(":/resources/icons/settings.png"));
    setModal(true);
    resize(1200, 800);
    
    setupUi();
    connectSignals();
}

RecipeConfigDialog::~RecipeConfigDialog()
{
}

void RecipeConfigDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 创建标签页
    m_tabWidget = new QTabWidget(this);
    
    // === 第一个标签页：基本配置 ===
    QWidget* basicConfigTab = new QWidget();
    QFormLayout* basicLayout = new QFormLayout(basicConfigTab);
    
    // 配方名称
    m_recipeNameEdit = new QLineEdit(this);
    m_recipeNameEdit->setPlaceholderText("输入配方名称, 如: A300_Active");
    basicLayout->addRow("配方名称:", m_recipeNameEdit);
    
    // SiteGroups
    m_siteGroupsEdit = new QLineEdit(this);
    m_siteGroupsEdit->setPlaceholderText("站点组, 如: Site01|Site02");
    basicLayout->addRow("站点组:", m_siteGroupsEdit);
    
    // 路由文件信息
    QHBoxLayout* routerLayout = new QHBoxLayout();
    m_routerFileEdit = new QLineEdit(this);
    m_routerFileEdit->setPlaceholderText("选择路由文件, 如: site_workflow_router.json");
    m_routerBrowseButton = new QPushButton("浏览...", this);
    m_routerBrowseButton->setFixedWidth(80);
    routerLayout->addWidget(m_routerFileEdit);
    routerLayout->addWidget(m_routerBrowseButton);
    basicLayout->addRow("路由文件:", routerLayout);
    
    // 自动机配置文件路径
    QHBoxLayout* cmd3Layout = new QHBoxLayout();
    m_cmd3PathEdit = new QLineEdit(this);
    m_cmd3PathEdit->setPlaceholderText("选择自动机任务配置文件(.tsk)");
    m_cmd3BrowseButton = new QPushButton("浏览...", this);
    m_cmd3BrowseButton->setFixedWidth(80);
    cmd3Layout->addWidget(m_cmd3PathEdit);
    cmd3Layout->addWidget(m_cmd3BrowseButton);
    basicLayout->addRow("自动机配置文件:", cmd3Layout);
    
    // 烧录软件任务文件路径
    QHBoxLayout* taskLayout = new QHBoxLayout();
    m_taskFileNameEdit = new QLineEdit(this);
    m_taskFileNameEdit->setPlaceholderText("选择烧录任务配置文件(.actask)");
    m_taskFileBrowseButton = new QPushButton("浏览...", this);
    m_taskFileBrowseButton->setFixedWidth(80);
    taskLayout->addWidget(m_taskFileNameEdit);
    taskLayout->addWidget(m_taskFileBrowseButton);
    basicLayout->addRow("烧录任务文件:", taskLayout);
    
    m_tabWidget->addTab(basicConfigTab, "基本配置");
    
    // === 第二个标签页：激活参数配置 ===
    QWidget* activationTab = new QWidget();
    QVBoxLayout* activationLayout = new QVBoxLayout(activationTab);
    
    // 芯片类型选择
    QHBoxLayout* chipTypeLayout = new QHBoxLayout();
    chipTypeLayout->addWidget(new QLabel("芯片型号:", this));
    m_chipTypeComboBox = new QComboBox(this);
    m_chipTypeComboBox->addItem("A300", "A300");
    m_chipTypeComboBox->addItem("G300", "G300");
    m_chipTypeComboBox->addItem("270", "270");
    chipTypeLayout->addWidget(m_chipTypeComboBox);
    chipTypeLayout->addStretch();
    activationLayout->addLayout(chipTypeLayout);
    
    // 参数表格
    setupActivationParametersTable();
    activationLayout->addWidget(m_activationParamsTable);
    
    // 表格操作按钮
    QHBoxLayout* tableButtonsLayout = new QHBoxLayout();
    m_addParamButton = new QPushButton("添加参数", this);
    m_removeParamButton = new QPushButton("删除参数", this);
    m_saveParamButton = new QPushButton("保存参数", this);
    tableButtonsLayout->addWidget(m_addParamButton);
    tableButtonsLayout->addWidget(m_removeParamButton);
    tableButtonsLayout->addWidget(m_saveParamButton);
    tableButtonsLayout->addStretch();
    activationLayout->addLayout(tableButtonsLayout);
    
    m_tabWidget->addTab(activationTab, "激活参数");
    
    mainLayout->addWidget(m_tabWidget);
    
    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    // 添加加载配置文件按钮
    m_loadActestButton = new QPushButton("加载配置文件", this);
    m_loadActestButton->setToolTip("加载现有的 actest 配置文件");
    buttonLayout->addWidget(m_loadActestButton);
    
    buttonLayout->addStretch();
    
    m_saveButton = new QPushButton("保存配置", this);
    m_cancelButton = new QPushButton("取消", this);
    
    m_saveButton->setDefault(true);
    
    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 加载流程选项、芯片模板和设置样式
    loadConfigTypeOptions();
    loadChipTemplates();
    
    // 设置样式
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #cccccc;
            border-radius: 5px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        QPushButton {
            min-width: 80px;
            min-height: 24px;
            padding: 4px 12px;
        }
        QLineEdit {
            padding: 4px;
            border: 1px solid #cccccc;
            border-radius: 3px;
        }
        QSpinBox {
            padding: 4px;
            border: 1px solid #cccccc;
            border-radius: 3px;
        }
        QTableWidget {
            border: 1px solid #cccccc;
            border-radius: 3px;
            gridline-color: #e0e0e0;
        }
        QHeaderView::section {
            background-color: #f5f5f5;
            padding: 4px;
            border: 1px solid #cccccc;
            font-weight: bold;
        }
    )");
}

void RecipeConfigDialog::connectSignals()
{
    connect(m_routerBrowseButton, &QPushButton::clicked, this, &RecipeConfigDialog::onBrowseRouterFile);
    connect(m_cmd3BrowseButton, &QPushButton::clicked, this, &RecipeConfigDialog::onBrowseCmd3Path);
    connect(m_taskFileBrowseButton, &QPushButton::clicked, this, &RecipeConfigDialog::onBrowseTaskFileName);
    connect(m_saveButton, &QPushButton::clicked, this, &RecipeConfigDialog::onSaveClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &RecipeConfigDialog::onCancelClicked);
    connect(m_loadActestButton, &QPushButton::clicked, this, &RecipeConfigDialog::onLoadActestFile);
    
    // 新增的激活参数相关信号
    connect(m_chipTypeComboBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &RecipeConfigDialog::onChipTypeChanged);
    connect(m_addParamButton, &QPushButton::clicked, this, &RecipeConfigDialog::onAddParameter);
    connect(m_removeParamButton, &QPushButton::clicked, this, &RecipeConfigDialog::onRemoveParameter);
    connect(m_saveParamButton, &QPushButton::clicked, this, &RecipeConfigDialog::onSaveParameters);
}

void RecipeConfigDialog::setupActivationParametersTable()
{
    m_activationParamsTable = new QTableWidget(this);
    m_activationParamsTable->setColumnCount(9);
    
    // 设置表头
    QStringList headers;
    headers << "参数名称" << "测试位" << "拟号值" << "下限" << "上限" << "通过值" << "单位" << "参数类型" << "备注";
    m_activationParamsTable->setHorizontalHeaderLabels(headers);
    
    // 设置列宽
    m_activationParamsTable->setColumnWidth(0, 120);  // 参数名称
    m_activationParamsTable->setColumnWidth(1, 60);   // 测试位
    m_activationParamsTable->setColumnWidth(2, 80);   // 拟号值
    m_activationParamsTable->setColumnWidth(3, 80);   // 下限
    m_activationParamsTable->setColumnWidth(4, 80);   // 上限
    m_activationParamsTable->setColumnWidth(5, 80);   // 通过值
    m_activationParamsTable->setColumnWidth(6, 60);   // 单位
    m_activationParamsTable->setColumnWidth(7, 80);   // 参数类型
    m_activationParamsTable->setColumnWidth(8, 120);  // 备注
    
    // 设置表格属性
    m_activationParamsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_activationParamsTable->setAlternatingRowColors(true);
    m_activationParamsTable->horizontalHeader()->setStretchLastSection(true);
    m_activationParamsTable->setMinimumHeight(300);
}

void RecipeConfigDialog::loadChipTemplates()
{
    // 创建不同芯片的参数模板
    QJsonObject chipTemplates;
    
    // A300 芯片参数模板
    QJsonArray a300Params;
    QJsonObject param1;
    param1["name"] = "工作电流ICC";
    param1["testBit"] = "";
    param1["refValue"] = "";
    param1["lowerLimit"] = 5.5;
    param1["upperLimit"] = 7.0;
    param1["passValue"] = "";
    param1["unit"] = "mA";
    param1["type"] = "电流";
    param1["note"] = "ICC电流制作参数";
    a300Params.append(param1);
    
    QJsonObject param2;
    param2["name"] = "嵌套环直流电压";
    param2["testBit"] = "38";
    param2["refValue"] = 6;
    param2["lowerLimit"] = 0.9;
    param2["upperLimit"] = 1.7;
    param2["passValue"] = "";
    param2["unit"] = "V";
    param2["type"] = "电压";
    param2["note"] = "嵌套法进成电压范围";
    a300Params.append(param2);
    
    QJsonObject param3;
    param3["name"] = "嵌套环直流电压...";
    param3["testBit"] = "34";
    param3["refValue"] = "";
    param3["lowerLimit"] = 1.3;
    param3["upperLimit"] = 1.4;
    param3["passValue"] = 1.35;
    param3["unit"] = "V";
    param3["type"] = "电压";
    param3["note"] = "调整存在设定，通过目标值";
    a300Params.append(param3);
    
    QJsonObject param4;
    param4["name"] = "驱动直流电压";
    param4["testBit"] = "38";
    param4["refValue"] = 5;
    param4["lowerLimit"] = 1.2;
    param4["upperLimit"] = 1.6;
    param4["passValue"] = "";
    param4["unit"] = "V";
    param4["type"] = "电压";
    param4["note"] = "驱动电压范围";
    a300Params.append(param4);
    
    QJsonObject param5;
    param5["name"] = "驱动直流电压...";
    param5["testBit"] = "";
    param5["refValue"] = "";
    param5["lowerLimit"] = 0;
    param5["upperLimit"] = 300;
    param5["passValue"] = "";
    param5["unit"] = "mV";
    param5["type"] = "电压";
    param5["note"] = "1ms内蜂鸣值范围";
    a300Params.append(param5);
    
    QJsonObject param6;
    param6["name"] = "周期脉冲电压";
    param6["testBit"] = "38";
    param6["refValue"] = 27;
    param6["lowerLimit"] = 150000;
    param6["upperLimit"] = 155000;
    param6["passValue"] = 155000;
    param6["unit"] = "Hz";
    param6["type"] = "频率";
    param6["note"] = "交流电压频率及通过值";
    a300Params.append(param6);
    
    QJsonObject param7;
    param7["name"] = "周期脉冲电压调幅";
    param7["testBit"] = "43";
    param7["refValue"] = "";
    param7["lowerLimit"] = 150000;
    param7["upperLimit"] = 155000;
    param7["passValue"] = 155000;
    param7["unit"] = "Hz";
    param7["type"] = "频率";
    param7["note"] = "调整存在设定，通过目标频率";
    a300Params.append(param7);
    
    QJsonObject param8;
    param8["name"] = "正弦交流电压均值";
    param8["testBit"] = "38";
    param8["refValue"] = 4;
    param8["lowerLimit"] = 0.9;
    param8["upperLimit"] = 1.1;
    param8["passValue"] = "";
    param8["unit"] = "V";
    param8["type"] = "电压";
    param8["note"] = "正弦波电压均值范围";
    a300Params.append(param8);
    
    QJsonObject param9;
    param9["name"] = "正弦交流电压...";
    param9["testBit"] = "";
    param9["refValue"] = "";
    param9["lowerLimit"] = 0.8;
    param9["upperLimit"] = 1.2;
    param9["passValue"] = "";
    param9["unit"] = "V";
    param9["type"] = "电压";
    param9["note"] = "正弦波电压峰值范围";
    a300Params.append(param9);
    
    QJsonObject param10;
    param10["name"] = "正弦交流电压频率";
    param10["testBit"] = "";
    param10["refValue"] = "";
    param10["lowerLimit"] = 19000;
    param10["upperLimit"] = 23000;
    param10["passValue"] = "";
    param10["unit"] = "Hz";
    param10["type"] = "频率";
    param10["note"] = "驱动频率范围";
    a300Params.append(param10);
    
    chipTemplates["A300"] = a300Params;
    
    // G300 芯片参数模板  
    QJsonArray g300Params;
    QJsonObject g300Param1;
    g300Param1["name"] = "工作电流ICC";
    g300Param1["testBit"] = "";
    g300Param1["refValue"] = "";
    g300Param1["lowerLimit"] = 4.5;
    g300Param1["upperLimit"] = 6.5;
    g300Param1["passValue"] = "";
    g300Param1["unit"] = "mA";
    g300Param1["type"] = "电流";
    g300Param1["note"] = "G300芯片工作电流";
    g300Params.append(g300Param1);
    
    QJsonObject g300Param2;
    g300Param2["name"] = "供电电压VDD";
    g300Param2["testBit"] = "35";
    g300Param2["refValue"] = 3.3;
    g300Param2["lowerLimit"] = 3.0;
    g300Param2["upperLimit"] = 3.6;
    g300Param2["passValue"] = "";
    g300Param2["unit"] = "V";
    g300Param2["type"] = "电压";
    g300Param2["note"] = "G300芯片供电电压";
    g300Params.append(g300Param2);
    chipTemplates["G300"] = g300Params;
    
    // 270 芯片参数模板
    QJsonArray chip270Params;
    QJsonObject chip270Param1;
    chip270Param1["name"] = "工作电流ICC";
    chip270Param1["testBit"] = "";
    chip270Param1["refValue"] = "";
    chip270Param1["lowerLimit"] = 3.5;
    chip270Param1["upperLimit"] = 5.0;
    chip270Param1["passValue"] = "";
    chip270Param1["unit"] = "mA";
    chip270Param1["type"] = "电流";
    chip270Param1["note"] = "270芯片工作电流";
    chip270Params.append(chip270Param1);
    
    QJsonObject chip270Param2;
    chip270Param2["name"] = "时钟频率";
    chip270Param2["testBit"] = "27";
    chip270Param2["refValue"] = 150000;
    chip270Param2["lowerLimit"] = 145000;
    chip270Param2["upperLimit"] = 155000;
    chip270Param2["passValue"] = 150000;
    chip270Param2["unit"] = "Hz";
    chip270Param2["type"] = "频率";
    chip270Param2["note"] = "270芯片时钟频率";
    chip270Params.append(chip270Param2);
    chipTemplates["270"] = chip270Params;
    
    m_chipTemplates = chipTemplates;
    
    // 加载默认芯片参数
    if (m_chipTypeComboBox) {
        onChipTypeChanged(m_chipTypeComboBox->currentText());
    }
}

void RecipeConfigDialog::loadParametersForChip(const QString& chipType)
{
    if (!m_activationParamsTable || !m_chipTemplates.contains(chipType)) {
        return;
    }
    
    // 清空当前表格
    m_activationParamsTable->setRowCount(0);
    
    // 加载芯片参数
    QJsonArray params = m_chipTemplates[chipType].toArray();
    for (int i = 0; i < params.size(); ++i) {
        QJsonObject param = params[i].toObject();
        addParameterRow(param);
    }
}

void RecipeConfigDialog::addParameterRow(const QJsonObject& param)
{
    if (!m_activationParamsTable) return;
    
    int row = m_activationParamsTable->rowCount();
    m_activationParamsTable->insertRow(row);
    
    // 参数名称
    m_activationParamsTable->setItem(row, 0, new QTableWidgetItem(param["name"].toString()));
    
    // 测试位
    m_activationParamsTable->setItem(row, 1, new QTableWidgetItem(param["testBit"].toString()));
    
    // 拟号值
    m_activationParamsTable->setItem(row, 2, new QTableWidgetItem(param["refValue"].toString()));
    
    // 下限
    m_activationParamsTable->setItem(row, 3, new QTableWidgetItem(QString::number(param["lowerLimit"].toDouble())));
    
    // 上限
    m_activationParamsTable->setItem(row, 4, new QTableWidgetItem(QString::number(param["upperLimit"].toDouble())));
    
    // 通过值
    m_activationParamsTable->setItem(row, 5, new QTableWidgetItem(param["passValue"].toString()));
    
    // 单位
    m_activationParamsTable->setItem(row, 6, new QTableWidgetItem(param["unit"].toString()));
    
    // 参数类型
    m_activationParamsTable->setItem(row, 7, new QTableWidgetItem(param["type"].toString()));
    
    // 备注
    m_activationParamsTable->setItem(row, 8, new QTableWidgetItem(param["note"].toString()));
}

void RecipeConfigDialog::loadConfigTypeOptions()
{

    QSettings settings("config/setting.ini", QSettings::IniFormat);
    
    // 尝试设置编码（兼容Qt5和Qt6）
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    if (codec) {
        settings.setIniCodec(codec);
    }
    
    // 获取所有分组
    QStringList groups = settings.childGroups();

    for (const QString& group : groups) {
        // 跳过System配置组，这是系统内部使用的
        if (group == "System") {
            continue;
        }
        
        settings.beginGroup(group);
        QString description = settings.value("Description", group).toString();
        settings.endGroup();
    }

}

void RecipeConfigDialog::loadConfigByGroup(const QString& selectedGroup)
{
    if (selectedGroup.isEmpty()) return;
    
    QSettings settings("config/setting.ini", QSettings::IniFormat);
    
    // 尝试设置编码（兼容Qt5和Qt6）
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    if (codec) {
        settings.setIniCodec(codec);
    }
    
    settings.beginGroup(selectedGroup);
    
    // 填充字段
    QString recipeName = settings.value("RecipeName", "").toString();
    if (m_recipeNameEdit) {
        m_recipeNameEdit->setText(recipeName);
    }
    
    QString siteGroups = settings.value("SiteGroups", "").toString();
    if (m_siteGroupsEdit) {
        m_siteGroupsEdit->setText(siteGroups);
    }
    
    QString router = settings.value("Router", "").toString();
    if (m_routerFileEdit) {
        m_routerFileEdit->setText(router);
    }
    
    QString tskFile = settings.value("TskFile", "").toString();
    if (m_cmd3PathEdit) {
        m_cmd3PathEdit->setText(tskFile);
    }
    
    QString acTaskFile = settings.value("ACTaskFile", "").toString();
    if (m_taskFileNameEdit) {
        m_taskFileNameEdit->setText(acTaskFile);
    }
    
    settings.endGroup();
}

void RecipeConfigDialog::onBrowseRouterFile()
{
    QTimer::singleShot(0, this, [this]() {
        try {
            QString filePath = QFileDialog::getOpenFileName(this,
                "选择路由文件",
                "config/",
                "JSON Files (*.json);;All Files (*.*)");
            
            if (!filePath.isEmpty()) {
                m_routerFileEdit->setText(filePath);
            }
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "错误", QString("文件选择异常: %1").arg(e.what()));
        } catch (...) {
            QMessageBox::critical(this, "错误", "文件选择发生未知异常");
        }
    });
}

// 槽函数实现
void RecipeConfigDialog::onChipTypeChanged(const QString& chipType)
{
    LOG_MODULE_INFO("RecipeConfigDialog", QString("芯片类型切换到: %1").arg(chipType).toStdString());
    loadParametersForChip(chipType);
}

void RecipeConfigDialog::onAddParameter()
{
    if (!m_activationParamsTable) return;
    
    // 添加空行
    QJsonObject emptyParam;
    emptyParam["name"] = "新参数";
    emptyParam["testBit"] = "";
    emptyParam["refValue"] = "";
    emptyParam["lowerLimit"] = 0.0;
    emptyParam["upperLimit"] = 0.0;
    emptyParam["passValue"] = "";
    emptyParam["unit"] = "";
    emptyParam["type"] = "";
    emptyParam["note"] = "";
    
    addParameterRow(emptyParam);
}

void RecipeConfigDialog::onRemoveParameter()
{
    if (!m_activationParamsTable) return;
    
    int currentRow = m_activationParamsTable->currentRow();
    if (currentRow >= 0) {
        m_activationParamsTable->removeRow(currentRow);
    } else {
        QMessageBox::information(this, "提示", "请先选择要删除的参数行");
    }
}

void RecipeConfigDialog::onSaveParameters()
{
    if (!m_activationParamsTable || !m_chipTypeComboBox) {
        QMessageBox::warning(this, "保存失败", "参数表格未初始化");
        return;
    }
    
    // 获取当前芯片类型
    QString chipType = m_chipTypeComboBox->currentText();
    if (chipType.isEmpty()) {
        QMessageBox::warning(this, "保存失败", "请先选择芯片类型");
        return;
    }
    
    // 构建参数数据
    QJsonArray paramsArray;
    for (int row = 0; row < m_activationParamsTable->rowCount(); ++row) {
        QJsonObject param;
        param["name"] = m_activationParamsTable->item(row, 0) ? m_activationParamsTable->item(row, 0)->text() : "";
        param["testBit"] = m_activationParamsTable->item(row, 1) ? m_activationParamsTable->item(row, 1)->text() : "";
        param["refValue"] = m_activationParamsTable->item(row, 2) ? m_activationParamsTable->item(row, 2)->text() : "";
        param["lowerLimit"] = m_activationParamsTable->item(row, 3) ? m_activationParamsTable->item(row, 3)->text().toDouble() : 0.0;
        param["upperLimit"] = m_activationParamsTable->item(row, 4) ? m_activationParamsTable->item(row, 4)->text().toDouble() : 0.0;
        param["passValue"] = m_activationParamsTable->item(row, 5) ? m_activationParamsTable->item(row, 5)->text() : "";
        param["unit"] = m_activationParamsTable->item(row, 6) ? m_activationParamsTable->item(row, 6)->text() : "";
        param["type"] = m_activationParamsTable->item(row, 7) ? m_activationParamsTable->item(row, 7)->text() : "";
        param["note"] = m_activationParamsTable->item(row, 8) ? m_activationParamsTable->item(row, 8)->text() : "";
        paramsArray.append(param);
    }
    
    // 更新内存中的模板数据
    m_chipTemplates[chipType] = paramsArray;
    
    // 可以选择保存到文件（这里暂时只保存到内存中的模板）
    QMessageBox::information(this, "保存成功", 
        QString("已保存 %1 个参数到 %2 芯片模板")
        .arg(paramsArray.size())
        .arg(chipType));
    
    LOG_MODULE_INFO("RecipeConfigDialog", 
        QString("已保存激活参数到芯片模板: %1, 参数数量: %2")
        .arg(chipType).arg(paramsArray.size()).toStdString());
}

void RecipeConfigDialog::loadCurrentConfig()
{
    // 查找是否已存在全局配置步骤
    if (m_recipeData.contains("steps") && m_recipeData["steps"].isArray()) {
        QJsonArray steps = m_recipeData["steps"].toArray();
        
        // 检查第一个步骤是否为全局配置
        if (!steps.isEmpty()) {
            QJsonObject firstStep = steps[0].toObject();
            if (firstStep.value("type").toString() == "GlobalConfig") {
                QJsonObject config = firstStep.value("config").toObject();
                

                
                // 加载激活参数配置
                if (config.contains("activationParams") && config["activationParams"].isObject()) {
                    QJsonObject activationParams = config["activationParams"].toObject();
                    
                    // 设置芯片类型
                    if (activationParams.contains("chipType") && m_chipTypeComboBox) {
                        QString chipType = activationParams["chipType"].toString();
                        int index = m_chipTypeComboBox->findText(chipType);
                        if (index >= 0) {
                            m_chipTypeComboBox->setCurrentIndex(index);
                        }
                    }
                    
                    // 加载参数表格数据
                    if (activationParams.contains("parameters") && activationParams["parameters"].isArray() && m_activationParamsTable) {
                        m_activationParamsTable->setRowCount(0);
                        QJsonArray paramsArray = activationParams["parameters"].toArray();
                        for (const auto& value : paramsArray) {
                            QJsonObject param = value.toObject();
                            addParameterRow(param);
                        }
                    }
                }
                
                LOG_MODULE_INFO("RecipeConfigDialog", "已加载现有的全局配置参数");
                return;
            }
        }
    }
    
    LOG_MODULE_INFO("RecipeConfigDialog", "使用默认配置参数");
}

bool RecipeConfigDialog::saveRecipeConfig()
{
    if (!validateInputs()) {
        return false;
    }
    
    return updateRecipeFile();
}

bool RecipeConfigDialog::validateInputs()
{    
    if (m_recipeNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "验证失败", "请填写配方名称");
        m_recipeNameEdit->setFocus();
        return false;
    }
    
    if (m_siteGroupsEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "验证失败", "请填写站点组");
        m_siteGroupsEdit->setFocus();
        return false;
    }
    
    if (m_routerFileEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "验证失败", "请选择路由文件");
        m_routerFileEdit->setFocus();
        return false;
    }
    
    if (m_cmd3PathEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "验证失败", "请选择自动机配置文件路径");
        m_cmd3PathEdit->setFocus();
        return false;
    }
    
    // 验证文件路径
    QString routerFile = m_routerFileEdit->text().trimmed();
    QFileInfo routerFileInfo(routerFile);
    if (!routerFileInfo.exists()) {
        QMessageBox::warning(this, "验证失败", 
            QString("路由文件不存在:\n%1").arg(routerFile));
        return false;
    }
    
    QString cmd3Path = m_cmd3PathEdit->text().trimmed();
    QFileInfo cmd3FileInfo(cmd3Path);
    if (!cmd3FileInfo.exists()) {
        QMessageBox::warning(this, "验证失败", 
            QString("自动机配置文件不存在:\n%1").arg(cmd3Path));
        return false;
    }
    
    QString taskFileName = m_taskFileNameEdit->text().trimmed();
    if (!taskFileName.isEmpty()) {
        QFileInfo taskFileInfo(taskFileName);
        if (!taskFileInfo.exists()) {
            QMessageBox::warning(this, "验证失败", 
                QString("烧录任务文件不存在:\n%1").arg(taskFileName));
            return false;
        }
    }
    
    return true;
}

bool RecipeConfigDialog::updateRecipeFile()
{
    // 创建全局配置步骤
    QJsonObject globalConfigStep;
    globalConfigStep["id"] = "global_config";
    globalConfigStep["name"] = "全局配置参数";
    globalConfigStep["type"] = "GlobalConfig";
    
    QJsonObject config;
    config["cmd3TaskPath"] = m_cmd3PathEdit->text().trimmed();
    config["taskFileName"] = m_taskFileNameEdit->text().trimmed();
    
    // 保存激活参数配置
    QJsonObject activationParams;
    if (m_chipTypeComboBox) {
        activationParams["chipType"] = m_chipTypeComboBox->currentText();
    }
    
    // 保存参数表格数据
    if (m_activationParamsTable) {
        QJsonArray paramsArray;
        for (int row = 0; row < m_activationParamsTable->rowCount(); ++row) {
            QJsonObject param;
            param["name"] = m_activationParamsTable->item(row, 0) ? m_activationParamsTable->item(row, 0)->text() : "";
            param["testBit"] = m_activationParamsTable->item(row, 1) ? m_activationParamsTable->item(row, 1)->text() : "";
            param["refValue"] = m_activationParamsTable->item(row, 2) ? m_activationParamsTable->item(row, 2)->text() : "";
            param["lowerLimit"] = m_activationParamsTable->item(row, 3) ? m_activationParamsTable->item(row, 3)->text().toDouble() : 0.0;
            param["upperLimit"] = m_activationParamsTable->item(row, 4) ? m_activationParamsTable->item(row, 4)->text().toDouble() : 0.0;
            param["passValue"] = m_activationParamsTable->item(row, 5) ? m_activationParamsTable->item(row, 5)->text() : "";
            param["unit"] = m_activationParamsTable->item(row, 6) ? m_activationParamsTable->item(row, 6)->text() : "";
            param["type"] = m_activationParamsTable->item(row, 7) ? m_activationParamsTable->item(row, 7)->text() : "";
            param["note"] = m_activationParamsTable->item(row, 8) ? m_activationParamsTable->item(row, 8)->text() : "";
            paramsArray.append(param);
        }
        activationParams["parameters"] = paramsArray;
    }
    
    config["activationParams"] = activationParams;
    
    globalConfigStep["config"] = config;
    
    // 更新步骤数组
    QJsonArray steps;
    if (m_recipeData.contains("steps") && m_recipeData["steps"].isArray()) {
        steps = m_recipeData["steps"].toArray();
    }
    
    // 检查第一个步骤是否已是全局配置
    bool hasGlobalConfig = false;
    if (!steps.isEmpty()) {
        QJsonObject firstStep = steps[0].toObject();
        if (firstStep.value("type").toString() == "GlobalConfig") {
            // 更新现有的全局配置
            steps[0] = globalConfigStep;
            hasGlobalConfig = true;
        }
    }
    
    // 如果没有全局配置，插入到开头
    if (!hasGlobalConfig) {
        steps.prepend(globalConfigStep);
    }
    
    m_recipeData["steps"] = steps;
    
    // 生成基于配方名称的文件路径
    QString recipeName = m_recipeNameEdit ? m_recipeNameEdit->text().trimmed() : "unnamed_recipe";
    if (recipeName.isEmpty()) {
        recipeName = "unnamed_recipe";
    }
    
    // 确保文件名安全（移除不安全字符）
    recipeName.replace(QRegExp("[\\/:*?\"<>|]"), "_");
    
    QString configDir = QCoreApplication::applicationDirPath() + "/config";
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QString recipesFilePath = configDir + "/" + recipeName + "_recipe.json";
    
    // 保存文件
    QJsonDocument doc(m_recipeData);
    QFile file(recipesFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "保存失败", 
            QString("无法打开配方文件进行写入:\n%1").arg(recipesFilePath));
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    LOG_MODULE_INFO("RecipeConfigDialog", QString("配方配置已保存到: %1").arg(recipesFilePath).toStdString());
    return true;
}

QJsonObject RecipeConfigDialog::getConfigParameters() const
{
    QJsonObject params;
    params["recipeName"] = m_recipeNameEdit ? m_recipeNameEdit->text().trimmed() : "";
    params["siteGroups"] = m_siteGroupsEdit ? m_siteGroupsEdit->text().trimmed() : "";
    params["routerFile"] = m_routerFileEdit ? m_routerFileEdit->text().trimmed() : "";
    params["cmd3TaskPath"] = m_cmd3PathEdit ? m_cmd3PathEdit->text().trimmed() : "";
    params["taskFileName"] = m_taskFileNameEdit ? m_taskFileNameEdit->text().trimmed() : "";
    
    // 生成激活参数摘要信息
    QString activationSummary;
    if (m_chipTypeComboBox && m_activationParamsTable) {
        activationSummary = QString("芯片型号:%1, 参数数量:%2")
            .arg(m_chipTypeComboBox->currentText())
            .arg(m_activationParamsTable->rowCount());
    }
    params["activationParams"] = activationSummary;
    
    return params;
}

/**
 * @brief 浏览自动机配置文件路径
 * 使用异步方式执行文件对话框操作，避免触发KVirtualFolder::Initialize导致死锁
 */
void RecipeConfigDialog::onBrowseCmd3Path()
{
    // 使用QTimer::singleShot将文件对话框操作延迟到下一个事件循环
    QTimer::singleShot(0, this, [this]() {
        try {
            QString filePath = QFileDialog::getOpenFileName(this,
                "选择自动机配置文件",
                "C:/TestTask",
                "Task Files (*.tsk);;All Files (*.*)");
            
            if (!filePath.isEmpty()) {
                m_cmd3PathEdit->setText(filePath);
            }
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "错误", QString("文件选择异常: %1").arg(e.what()));
        } catch (...) {
            QMessageBox::critical(this, "错误", "文件选择发生未知异常");
        }
    });
}

/**
 * @brief 浏览烧录任务文件路径
 * 使用异步方式执行文件对话框操作，避免触发KVirtualFolder::Initialize导致死锁
 */
void RecipeConfigDialog::onBrowseTaskFileName()
{
    // 使用QTimer::singleShot将文件对话框操作延迟到下一个事件循环
    QTimer::singleShot(0, this, [this]() {
        try {
            QString filePath = QFileDialog::getOpenFileName(this,
                "选择烧录任务文件",
                "",
                "Actask Files (*.actask);;All Files (*.*)");
            
            if (!filePath.isEmpty()) {
                m_taskFileNameEdit->setText(filePath);
            }
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "错误", QString("文件选择异常: %1").arg(e.what()));
        } catch (...) {
            QMessageBox::critical(this, "错误", "文件选择发生未知异常");
        }
    });
}

void RecipeConfigDialog::onSaveClicked()
{
    if (!validateInputs()) {
        return;
    }

// 步骤1：确定config目录路径（基于程序可执行文件目录）
        QString appDir = QCoreApplication::applicationDirPath(); // 程序所在目录
        QDir configDir(appDir + "/config");                       // config目录路径

        // 步骤2：检查并创建config目录（若不存在）
        if (!configDir.exists()) {
            if (!configDir.mkpath(".")) { // mkpath会递归创建所有缺失的父目录
                QMessageBox::critical(this, "错误", "无法创建config目录！");
                return;
            }
        }

        // 步骤3：弹出文件保存对话框（初始目录为config，后缀过滤为.actest）
        QString filePath = QFileDialog::getSaveFileName(
            this,
            "保存Actest文件",                  // 对话框标题
            configDir.absolutePath(),          // 初始目录
            "Actest Files (*.actest)"          // 文件过滤器（仅显示.actest文件）
        );

        // 用户取消对话框时，filePath为空，直接返回
        if (filePath.isEmpty()) return;

        // 确保文件后缀为.actest（对话框已过滤，但保险起见再次检查）
        if (!filePath.endsWith(".actest", Qt::CaseInsensitive)) {
            filePath += ".actest";
        }

        // 步骤4：保存配置到actest文件
        if (saveCurrentConfigToSettings(filePath, "system")) {
            QMessageBox::information(this, "成功", "配置文件保存成功！\n路径：" + filePath);
            LOG_MODULE_INFO("RecipeConfigDialog", QString("配置已保存到: %1").arg(filePath).toStdString());
        } else {
            QMessageBox::critical(this, "错误", "配置文件保存失败：" + filePath);
        }

}

void RecipeConfigDialog::onCancelClicked()
{
    reject(); // 关闭对话框并返回QDialog::Rejected
}

void RecipeConfigDialog::saveLastSelectedConfig(const QString& configGroup)
{
    QSettings settings("config/setting.ini", QSettings::IniFormat);
    
    // 尝试设置编码（兼容Qt5和Qt6）
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    if (codec) {
        settings.setIniCodec(codec);
    }
    
    settings.beginGroup("System");
    settings.setValue("LastSelect", configGroup);
    settings.endGroup();
    
    LOG_MODULE_DEBUG("RecipeConfigDialog", QString("保存上次选择的配置: %1").arg(configGroup).toStdString());
}

QString RecipeConfigDialog::getLastSelectedConfig()
{
    QSettings settings("config/setting.ini", QSettings::IniFormat);
    
    // 尝试设置编码（兼容Qt5和Qt6）
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    if (codec) {
        settings.setIniCodec(codec);
    }
    
    settings.beginGroup("System");
    QString lastSelect = settings.value("LastSelect", "A300_Active").toString();
    settings.endGroup();
    
    LOG_MODULE_DEBUG("RecipeConfigDialog", QString("读取上次选择的配置: %1").arg(lastSelect).toStdString());
    
    return lastSelect;
}

bool RecipeConfigDialog::saveCurrentConfigToSettings(const QString& filePath, const QString& configGroup)
{
    try {
        QSettings settings(filePath, QSettings::IniFormat);
        
        // 尝试设置编码（兼容Qt5和Qt6）
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        if (codec) {
            settings.setIniCodec(codec);
        }
        
        settings.beginGroup(configGroup);
        
        // 保存各个字段
        settings.setValue("RecipeName", m_recipeNameEdit ? m_recipeNameEdit->text().trimmed() : "");
        settings.setValue("SiteGroups", m_siteGroupsEdit ? m_siteGroupsEdit->text().trimmed() : "");
        settings.setValue("Router", m_routerFileEdit ? m_routerFileEdit->text().trimmed() : "");
        settings.setValue("TskFile", m_cmd3PathEdit ? m_cmd3PathEdit->text().trimmed() : "");
        settings.setValue("ACTaskFile", m_taskFileNameEdit ? m_taskFileNameEdit->text().trimmed() : "");
        
        settings.endGroup();
        
        // 强制同步到文件
        settings.sync();
        
        LOG_MODULE_INFO("RecipeConfigDialog", QString("已保存配置: %1").arg(configGroup).toStdString());
        return true;
        
    } catch (const std::exception& e) {
        LOG_MODULE_ERROR("RecipeConfigDialog", QString("保存配置失败: %1").arg(e.what()).toStdString());
        QMessageBox::critical(this, "保存失败", QString("保存配置时发生错误: %1").arg(e.what()));
        return false;
    } catch (...) {
        LOG_MODULE_ERROR("RecipeConfigDialog", "保存配置时发生未知错误");
        QMessageBox::critical(this, "保存失败", "保存配置时发生未知错误");
        return false;
    }
}

void RecipeConfigDialog::onLoadActestFile()
{
    QTimer::singleShot(0, this, [this]() {
        try {
            QString filePath = QFileDialog::getOpenFileName(this,
                "选择 Actest 配置文件",
                QCoreApplication::applicationDirPath() + "/config",
                "Actest Files (*.actest);;All Files (*.*)");
            
            if (filePath.isEmpty()) {
                return;
            }
            
            // 验证文件是否存在
            QFileInfo fileInfo(filePath);
            if (!fileInfo.exists()) {
                QMessageBox::warning(this, "文件不存在", 
                    QString("选择的文件不存在:\n%1").arg(filePath));
                return;
            }
            
            // 加载配置文件
            if (loadActestConfigFile(filePath)) {
                QMessageBox::information(this, "加载成功", 
                    QString("已成功加载配置文件:\n%1").arg(fileInfo.fileName()));
                LOG_MODULE_INFO("RecipeConfigDialog", 
                    QString("成功加载Actest配置文件: %1").arg(filePath).toStdString());
            }
            
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "错误", QString("加载文件异常: %1").arg(e.what()));
        } catch (...) {
            QMessageBox::critical(this, "错误", "加载文件发生未知异常");
        }
    });
}

bool RecipeConfigDialog::loadActestConfigFile(const QString& filePath)
{
    try {
        QSettings settings(filePath, QSettings::IniFormat);
        
        // 尝试设置编码（兼容Qt5和Qt6）
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        if (codec) {
            settings.setIniCodec(codec);
        }
        
        // 检查文件状态
        if (settings.status() != QSettings::NoError) {
            QMessageBox::critical(this, "加载失败", 
                QString("无法读取配置文件:\n%1\n\n可能的原因:\n- 文件格式不正确\n- 文件被占用\n- 编码问题").arg(filePath));
            return false;
        }
        
        settings.beginGroup("system");
        
        // 检查system组是否存在
        QStringList keys = settings.childKeys();
        if (keys.isEmpty()) {
            QMessageBox::warning(this, "加载警告", 
                "配置文件中未找到 [system] 节或该节为空。\n将使用空白配置。");
        }
        
        // 加载各个配置字段
        QString recipeName = settings.value("RecipeName", "").toString();
        QString siteGroups = settings.value("SiteGroups", "").toString();
        QString router = settings.value("Router", "").toString();
        QString tskFile = settings.value("TskFile", "").toString();
        QString acTaskFile = settings.value("ACTaskFile", "").toString();
        
        settings.endGroup();
        
        // 填充UI控件
        if (m_recipeNameEdit) {
            m_recipeNameEdit->setText(recipeName);
        }
        
        if (m_siteGroupsEdit) {
            m_siteGroupsEdit->setText(siteGroups);
        }
        
        if (m_routerFileEdit) {
            m_routerFileEdit->setText(router);
        }
        
        if (m_cmd3PathEdit) {
            m_cmd3PathEdit->setText(tskFile);
        }
        
        if (m_taskFileNameEdit) {
            m_taskFileNameEdit->setText(acTaskFile);
        }
        
        // 记录加载的配置信息
        LOG_MODULE_DEBUG("RecipeConfigDialog", 
            QString("加载配置 - RecipeName: %1, SiteGroups: %2, Router: %3, TskFile: %4, ACTaskFile: %5")
            .arg(recipeName).arg(siteGroups).arg(router).arg(tskFile).arg(acTaskFile).toStdString());
        
        return true;
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "加载失败", 
            QString("加载配置文件时发生错误:\n%1").arg(e.what()));
        LOG_MODULE_ERROR("RecipeConfigDialog", 
            QString("加载Actest配置文件失败: %1").arg(e.what()).toStdString());
        return false;
    } catch (...) {
        QMessageBox::critical(this, "加载失败", "加载配置文件时发生未知错误");
        LOG_MODULE_ERROR("RecipeConfigDialog", "加载Actest配置文件时发生未知错误");
        return false;
    }
}

} // namespace Presentation
