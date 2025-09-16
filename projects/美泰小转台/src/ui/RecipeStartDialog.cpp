#include "ui/RecipeStartDialog.h"
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
namespace Presentation {

RecipeStartDialog::RecipeStartDialog(QWidget *parent)
    : QDialog(parent)
    , m_recipeNameDisplayEdit(nullptr)
    , m_actestFieNameEdit(nullptr)
    , m_batchNumberEdit(nullptr)
    , m_productionQuantitySpinBox(nullptr)
    , m_param1Edit(nullptr)
    , m_param2Edit(nullptr)
    , m_param3Edit(nullptr)
    , m_tabWidget(nullptr)
    , m_taskFileBrowseButton(nullptr)
    , m_cancelButton(nullptr)
    , m_applyButton(nullptr)
{
    setWindowTitle("开始测试");
    setWindowIcon(QIcon(":/resources/icons/settings.png"));
    setModal(true);
    resize(800, 400);
    
    setupUi();
    connectSignals();
    
    // 加载上次选择的actest文件
    QString lastActestFile = getLastSelectedConfig();
    if (!lastActestFile.isEmpty()) {
        m_actestFieNameEdit->setText(lastActestFile);
        // 从文件中读取配方名称
        loadRecipeNameFromFile(lastActestFile);
    }
}

RecipeStartDialog::~RecipeStartDialog()
{
}

void RecipeStartDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    QWidget* configTypeGroup = new QWidget();
    QFormLayout* configTypeLayout = new QFormLayout(configTypeGroup);
    
    // 配方名称显示框（只读）
    m_recipeNameDisplayEdit = new QLineEdit(this);
    m_recipeNameDisplayEdit->setReadOnly(true);
    m_recipeNameDisplayEdit->setPlaceholderText("从测试文件中读取配方名称");
    m_recipeNameDisplayEdit->setStyleSheet("QLineEdit[readOnly=\"true\"] { background-color: #f0f0f0; color: #666666; }");
    configTypeLayout->addRow("配方名称:", m_recipeNameDisplayEdit);
    
    QHBoxLayout* taskLayout = new QHBoxLayout();
    m_actestFieNameEdit = new QLineEdit(this);
    m_actestFieNameEdit->setPlaceholderText("选择actest测试文件");
    m_taskFileBrowseButton = new QPushButton("浏览...", this);
    m_taskFileBrowseButton->setFixedWidth(80);
    taskLayout->addWidget(m_actestFieNameEdit);
    taskLayout->addWidget(m_taskFileBrowseButton);
    configTypeLayout->addRow("测试文件:", taskLayout);
    
    mainLayout->addWidget(configTypeGroup);
    
    // 批次号
    m_batchNumberEdit = new QLineEdit(this);
    m_batchNumberEdit->setPlaceholderText("输入生产批次号");
    QString defaultBatch = QDateTime::currentDateTime().toString("yyyyMMdd_HHmm");
    m_batchNumberEdit->setText(defaultBatch);
    configTypeLayout->addRow("批次号:", m_batchNumberEdit);
    
    // 生产数量
    m_productionQuantitySpinBox = new QSpinBox(this);
    m_productionQuantitySpinBox->setRange(1, 999999);
    m_productionQuantitySpinBox->setValue(0);
    configTypeLayout->addRow("生产数量:", m_productionQuantitySpinBox);
    
    // 扩展参数输入字段
    m_param1Edit = new QLineEdit(this);
    m_param1Edit->setPlaceholderText("参数1");
    configTypeLayout->addRow("参数1:", m_param1Edit);
    
    m_param2Edit = new QLineEdit(this);
    m_param2Edit->setPlaceholderText("参数2");
    configTypeLayout->addRow("参数2:", m_param2Edit);
    
    m_param3Edit = new QLineEdit(this);
    m_param3Edit->setPlaceholderText("参数3");
    configTypeLayout->addRow("参数3:", m_param3Edit);
 
    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_applyButton = new QPushButton("开始", this);
    m_cancelButton = new QPushButton("取消", this);
    
    m_applyButton->setDefault(false);
    
    buttonLayout->addWidget(m_applyButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
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

void RecipeStartDialog::connectSignals()
{
    connect(m_taskFileBrowseButton, &QPushButton::clicked, this, &RecipeStartDialog::onBrowseTaskFileName);
    connect(m_cancelButton, &QPushButton::clicked, this, &RecipeStartDialog::onCancelClicked);
    connect(m_applyButton, &QPushButton::clicked, this, &RecipeStartDialog::onApplyClicked);
}

bool RecipeStartDialog::validateInputs()
{
    
    if (m_batchNumberEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "验证失败", "请输入批次号");
        m_batchNumberEdit->setFocus();
        return false;
    }
    
    QString taskFileName = m_actestFieNameEdit->text().trimmed();
    if (!taskFileName.isEmpty()) {
        QFileInfo taskFileInfo(taskFileName);
        if (!taskFileInfo.exists()) {
            QMessageBox::warning(this, "验证失败", 
                QString("测试文件不存在:\n%1").arg(taskFileName));
            return false;
        }
    }
    
    return true;
}

QJsonObject RecipeStartDialog::getConfigParameters()
{    
    QSettings settings(m_actestFieNameEdit->text().trimmed(), QSettings::IniFormat);
    
    // 尝试设置编码（兼容Qt5和Qt6）
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    if (codec) {
        settings.setIniCodec(codec);
    }
    
    settings.beginGroup("system");
    QJsonObject params;
    params["siteGroups"] = settings.value("SiteGroups", "").toString();
    params["recipes"] = settings.value("Recipes", "").toString();
    params["routerFile"] = settings.value("Router", "").toString();
    params["cmd3TaskPath"] = settings.value("TskFile", "").toString();
    params["taskFileName"] = settings.value("ACTaskFile", "").toString();
    params["productionQuantity"] = m_productionQuantitySpinBox->value();
    params["batchNumber"] = m_batchNumberEdit->text().trimmed();
    
    // 添加扩展参数
    params["param1"] = m_param1Edit->text().trimmed();
    params["param2"] = m_param2Edit->text().trimmed();
    params["param3"] = m_param3Edit->text().trimmed();
    
    settings.endGroup();
    
    return params;
}

/**
 * @brief 浏览烧录任务文件路径
 * 使用异步方式执行文件对话框操作，避免触发KVirtualFolder::Initialize导致死锁
 */
void RecipeStartDialog::onBrowseTaskFileName()
{
    // 使用QTimer::singleShot将文件对话框操作延迟到下一个事件循环
    QTimer::singleShot(0, this, [this]() {
        try {
            QString filePath = QFileDialog::getOpenFileName(this,
                "选择测试文件",
                "",
                "Actest Files (*.actest);;All Files (*.*)");
            
            if (!filePath.isEmpty()) {
                m_actestFieNameEdit->setText(filePath);
                // 保存选择的actest文件路径
                saveLastSelectedConfig(filePath);
                // 从文件中读取配方名称
                loadRecipeNameFromFile(filePath);
            }
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "错误", QString("文件选择异常: %1").arg(e.what()));
        } catch (...) {
            QMessageBox::critical(this, "错误", "文件选择发生未知异常");
        }
    });
}

void RecipeStartDialog::onApplyClicked()
{
    LOG_MODULE_INFO("RecipeStartDialog", "配置应用到配方文件成功");
    accept();
}

void RecipeStartDialog::onCancelClicked()
{
    reject(); // 关闭对话框并返回QDialog::Rejected
}

void RecipeStartDialog::saveLastSelectedConfig(const QString& actestFilePath)
{
    QSettings settings("config/setting.ini", QSettings::IniFormat);
    
    // 尝试设置编码（兼容Qt5和Qt6）
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    if (codec) {
        settings.setIniCodec(codec);
    }
    
    settings.beginGroup("System");
    settings.setValue("LastActestFile", actestFilePath);
    settings.endGroup();
    
    LOG_MODULE_DEBUG("RecipeStartDialog", QString("保存上次选择的actest文件路径: %1").arg(actestFilePath).toStdString());
}

QString RecipeStartDialog::getLastSelectedConfig()
{
    QSettings settings("config/setting.ini", QSettings::IniFormat);
    
    // 尝试设置编码（兼容Qt5和Qt6）
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    if (codec) {
        settings.setIniCodec(codec);
    }
    
    settings.beginGroup("System");
    QString lastActestFile = settings.value("LastActestFile", "").toString();
    settings.endGroup();
    
    LOG_MODULE_DEBUG("RecipeStartDialog", QString("读取上次选择的actest文件路径: %1").arg(lastActestFile).toStdString());
    
    return lastActestFile;
}

void RecipeStartDialog::loadRecipeNameFromFile(const QString& filePath)
{
    if (filePath.isEmpty() || !m_recipeNameDisplayEdit) {
        return;
    }
    
    try {
        QSettings settings(filePath, QSettings::IniFormat);
        
        // 尝试设置编码（兼容Qt5和Qt6）
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        if (codec) {
            settings.setIniCodec(codec);
        }
        
        // 检查文件是否存在和可读
        QFileInfo fileInfo(filePath);
        if (!fileInfo.exists() || !fileInfo.isReadable()) {
            m_recipeNameDisplayEdit->setText("文件不存在或无法读取");
            m_recipeNameDisplayEdit->setStyleSheet("QLineEdit[readOnly=\"true\"] { background-color: #ffe6e6; color: #cc0000; }");
            return;
        }
        
        settings.beginGroup("system");
        
        // 读取配方名称
        QString recipeName = settings.value("RecipeName", "").toString();
        
        // 尝试读取扩展参数
        QString param1 = settings.value("param1", "").toString();
        QString param2 = settings.value("param2", "").toString();
        QString param3 = settings.value("param3", "").toString();
        
        settings.endGroup();
        
        if (!recipeName.isEmpty()) {
            m_recipeNameDisplayEdit->setText(recipeName);
            m_recipeNameDisplayEdit->setStyleSheet("QLineEdit[readOnly=\"true\"] { background-color: #e6ffe6; color: #006600; }");
            LOG_MODULE_DEBUG("RecipeStartDialog", QString("成功读取配方名称: %1").arg(recipeName).toStdString());
        } else {
            m_recipeNameDisplayEdit->setText("未找到配方名称");
            m_recipeNameDisplayEdit->setStyleSheet("QLineEdit[readOnly=\"true\"] { background-color: #fff3cd; color: #856404; }");
            LOG_MODULE_DEBUG("RecipeStartDialog", "配置文件中未找到配方名称");
        }
        
        // 设置扩展参数值（如果存在）
        if (!param1.isEmpty()) m_param1Edit->setText(param1);
        if (!param2.isEmpty()) m_param2Edit->setText(param2);
        if (!param3.isEmpty()) m_param3Edit->setText(param3);
        
    } catch (const std::exception& e) {
        m_recipeNameDisplayEdit->setText("读取配方名称失败");
        m_recipeNameDisplayEdit->setStyleSheet("QLineEdit[readOnly=\"true\"] { background-color: #ffe6e6; color: #cc0000; }");
        LOG_MODULE_ERROR("RecipeStartDialog", QString("读取配方名称失败: %1").arg(e.what()).toStdString());
    } catch (...) {
        m_recipeNameDisplayEdit->setText("读取配方名称异常");
        m_recipeNameDisplayEdit->setStyleSheet("QLineEdit[readOnly=\"true\"] { background-color: #ffe6e6; color: #cc0000; }");
        LOG_MODULE_ERROR("RecipeStartDialog", "读取配方名称时发生未知异常");
    }
}


} // namespace Presentation
