#include "ProjectSelectDialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>

ProjectSelectDialog::ProjectSelectDialog(QWidget *parent)
    : QDialog(parent), 
      m_settings("AcroViewTester", "ProjectSelector")
{
    setWindowTitle("选择项目配置");
    setMinimumSize(500, 400);
    
    m_configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/projects";
    QDir dir(m_configDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    setupUI();
    loadProjects();
    applyStyleSheet();
}

ProjectSelectDialog::~ProjectSelectDialog()
{
    saveProjects();
}

void ProjectSelectDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // 标题标签
    QLabel *titleLabel = new QLabel("项目配置选择", this);
    titleLabel->setObjectName("titleLabel");
    mainLayout->addWidget(titleLabel);
    
    // 项目列表区域
    QHBoxLayout *listLayout = new QHBoxLayout();
    
    // 列表部分
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(5);
    
    m_projectList = new QListWidget(this);
    m_projectList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_projectList->setMinimumWidth(200);
    connect(m_projectList, &QListWidget::itemSelectionChanged, this, &ProjectSelectDialog::onProjectSelectionChanged);
    
    leftLayout->addWidget(m_projectList);
    
    // 列表操作按钮
    QHBoxLayout *listButtonLayout = new QHBoxLayout();
    m_addButton = new QPushButton("添加项目", this);
    m_editButton = new QPushButton("编辑项目", this);
    m_removeButton = new QPushButton("移除项目", this);
    
    m_addButton->setObjectName("addButton");
    m_editButton->setObjectName("editButton");
    m_removeButton->setObjectName("removeButton");
    
    m_editButton->setEnabled(false);
    m_removeButton->setEnabled(false);
    
    connect(m_addButton, &QPushButton::clicked, this, &ProjectSelectDialog::onAddProjectClicked);
    connect(m_editButton, &QPushButton::clicked, this, &ProjectSelectDialog::onEditProjectClicked);
    connect(m_removeButton, &QPushButton::clicked, this, &ProjectSelectDialog::onRemoveProjectClicked);
    
    listButtonLayout->addWidget(m_addButton);
    listButtonLayout->addWidget(m_editButton);
    listButtonLayout->addWidget(m_removeButton);
    
    leftLayout->addLayout(listButtonLayout);
    listLayout->addLayout(leftLayout, 1);
    
    // 详情区域
    QVBoxLayout *rightLayout = new QVBoxLayout();
    
    m_detailsGroup = new QGroupBox("项目详情", this);
    QVBoxLayout *detailsLayout = new QVBoxLayout(m_detailsGroup);
    
    m_pathLabel = new QLabel("配置文件路径:", m_detailsGroup);
    m_pathEdit = new QLineEdit(m_detailsGroup);
    m_pathEdit->setReadOnly(true);
    
    m_descLabel = new QLabel("项目描述:", m_detailsGroup);
    m_descEdit = new QTextEdit(m_detailsGroup);
    m_descEdit->setReadOnly(true);
    
    detailsLayout->addWidget(m_pathLabel);
    detailsLayout->addWidget(m_pathEdit);
    detailsLayout->addWidget(m_descLabel);
    detailsLayout->addWidget(m_descEdit);
    
    rightLayout->addWidget(m_detailsGroup);
    rightLayout->addStretch();
    
    listLayout->addLayout(rightLayout, 2);
    mainLayout->addLayout(listLayout);
    
    // 底部描述标签
    QLabel *descriptionLabel = new QLabel("选择一个项目配置以启动应用程序，或者添加新的项目配置。", this);
    descriptionLabel->setObjectName("descriptionLabel");
    descriptionLabel->setWordWrap(true);
    mainLayout->addWidget(descriptionLabel);
    
    // 确定取消按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_okButton = new QPushButton("确定", this);
    m_cancelButton = new QPushButton("取消", this);
    
    m_okButton->setObjectName("okButton");
    m_cancelButton->setObjectName("cancelButton");
    m_okButton->setEnabled(false);
    
    connect(m_okButton, &QPushButton::clicked, this, &ProjectSelectDialog::onOkButtonClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &ProjectSelectDialog::onCancelButtonClicked);
    
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 初始状态下禁用确定按钮和详情显示
    m_detailsGroup->setEnabled(false);
}

void ProjectSelectDialog::applyStyleSheet()
{
    // 主对话框样式
    QString dialogStyle = R"(
        QDialog {
            background-color: #F5F5F5;
            border: 1px solid #D0D0D0;
        }
        QLabel {
            color: #333333;
            font-size: 12px;
            font-weight: bold;
        }
        QLabel#titleLabel {
            font-size: 16px;
            font-weight: bold;
            color: #2C3E50;
            margin-bottom: 10px;
            border-bottom: 1px solid #BDC3C7;
            padding-bottom: 5px;
        }
        QLabel#descriptionLabel {
            color: #7F8C8D;
            font-size: 11px;
            font-weight: normal;
            font-style: italic;
            padding-top: 5px;
        }
    )";
    
    // 列表样式
    QString listStyle = R"(
        QListWidget {
            background-color: white;
            border: 1px solid #D0D0D0;
            border-radius: 4px;
            padding: 5px;
            selection-background-color: #3498DB;
            selection-color: white;
        }
        QListWidget::item {
            border-bottom: 1px solid #EEE;
            padding: 6px;
            border-radius: 2px;
        }
        QListWidget::item:hover {
            background-color: #E3F2FD;
        }
        QListWidget::item:selected {
            background-color: #2980B9;
            color: white;
        }
    )";
    
    // 按钮样式
    QString buttonStyle = R"(
        QPushButton {
            background-color: #3498DB;
            color: white;
            border: none;
            border-radius: 3px;
            padding: 8px 15px;
            min-width: 80px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #2980B9;
        }
        QPushButton:pressed {
            background-color: #1A5276;
        }
        QPushButton:disabled {
            background-color: #BDC3C7;
            color: #7F8C8D;
        }
        QPushButton#addButton, QPushButton#editButton, QPushButton#removeButton {
            font-size: 11px;
            padding: 5px 10px;
            min-width: 60px;
        }
        QPushButton#addButton {
            background-color: #2ECC71;
        }
        QPushButton#addButton:hover {
            background-color: #27AE60;
        }
        QPushButton#editButton {
            background-color: #F39C12;
        }
        QPushButton#editButton:hover {
            background-color: #D35400;
        }
        QPushButton#removeButton {
            background-color: #E74C3C;
        }
        QPushButton#removeButton:hover {
            background-color: #C0392B;
        }
    )";
    
    // 详情区域样式
    QString detailsStyle = R"(
        QGroupBox {
            border: 1px solid #D0D0D0;
            border-radius: 4px;
            margin-top: 20px;
            padding-top: 10px;
            background-color: white;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 10px;
            padding: 0 5px;
            color: #2C3E50;
            font-weight: bold;
        }
        QTextEdit {
            border: 1px solid #D0D0D0;
            border-radius: 2px;
            background-color: #F9F9F9;
            color: #444;
            font-family: "Consolas", monospace;
        }
        QLineEdit {
            border: 1px solid #D0D0D0;
            border-radius: 2px;
            padding: 5px;
            background-color: #F9F9F9;
            selection-background-color: #3498DB;
        }
    )";

    // 滚动条样式
    QString scrollBarStyle = R"(
        QScrollBar:vertical {
            border: none;
            background: #F0F0F0;
            width: 8px;
            margin: 0px 0px 0px 0px;
        }
        QScrollBar::handle:vertical {
            background: #BCBCBC;
            min-height: 20px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical:hover {
            background: #3498DB;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: none;
        }
    )";

    // 应用所有样式
    setStyleSheet(dialogStyle + listStyle + buttonStyle + detailsStyle + scrollBarStyle);
    
    // 为特定控件设置对象名称以应用特定样式
    if (m_projectList) {
        m_projectList->setAlternatingRowColors(true); // 启用交替行颜色
    }
    
    // 查找并设置标题标签的对象名称
    QList<QLabel*> labels = findChildren<QLabel*>();
    for (QLabel* label : labels) {
        if (label->text().contains("项目列表", Qt::CaseInsensitive)) {
            label->setObjectName("titleLabel");
        } else if (label->text().contains("描述", Qt::CaseInsensitive)) {
            label->setObjectName("descriptionLabel");
        }
    }
    
    // 设置按钮的对象名称
    QList<QPushButton*> buttons = findChildren<QPushButton*>();
    for (QPushButton* button : buttons) {
        if (button->text().contains("添加", Qt::CaseInsensitive)) {
            button->setObjectName("addButton");
        } else if (button->text().contains("编辑", Qt::CaseInsensitive)) {
            button->setObjectName("editButton");
        } else if (button->text().contains("移除", Qt::CaseInsensitive)) {
            button->setObjectName("removeButton");
        } else if (button->text().contains("确定", Qt::CaseInsensitive)) {
            button->setObjectName("okButton");
        } else if (button->text().contains("取消", Qt::CaseInsensitive)) {
            button->setObjectName("cancelButton");
        }
    }
}

void ProjectSelectDialog::loadProjects()
{
    m_projects.clear();
    m_projectList->clear();
    
    int size = m_settings.beginReadArray("projects");
    for (int i = 0; i < size; ++i) {
        m_settings.setArrayIndex(i);
        ProjectInfo project;
        project.name = m_settings.value("name").toString();
        project.path = m_settings.value("path").toString();
        project.description = m_settings.value("description").toString();
        
        // 检查文件是否存在
        if (QFile::exists(project.path)) {
            m_projects.append(project);
            m_projectList->addItem(project.name);
        }
    }
    m_settings.endArray();
}

void ProjectSelectDialog::saveProjects()
{
    m_settings.beginWriteArray("projects");
    for (int i = 0; i < m_projects.size(); ++i) {
        m_settings.setArrayIndex(i);
        m_settings.setValue("name", m_projects[i].name);
        m_settings.setValue("path", m_projects[i].path);
        m_settings.setValue("description", m_projects[i].description);
    }
    m_settings.endArray();
    m_settings.sync();
}

void ProjectSelectDialog::onProjectSelectionChanged()
{
    bool hasSelection = m_projectList->currentItem() != nullptr;
    m_editButton->setEnabled(hasSelection);
    m_removeButton->setEnabled(hasSelection);
    m_okButton->setEnabled(hasSelection);
    m_detailsGroup->setEnabled(hasSelection);
    
    if (hasSelection) {
        updateProjectDetails();
    } else {
        m_pathEdit->clear();
        m_descEdit->clear();
    }
}

void ProjectSelectDialog::updateProjectDetails()
{
    QListWidgetItem* currentItem = m_projectList->currentItem();
    if (!currentItem) return;
    
    int index = m_projectList->row(currentItem);
    if (index < 0 || index >= m_projects.size()) return;
    
    const ProjectInfo& project = m_projects[index];
    m_pathEdit->setText(project.path);
    
    // 加载项目描述（从JSON配置中获取）
    QString description;
    QFile configFile(project.path);
    if (configFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(configFile.readAll());
        configFile.close();
        
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("description")) {
                description = obj["description"].toString();
            }
        }
    }
    
    if (description.isEmpty()) {
        description = "（无项目描述）";
    }
    
    m_descEdit->setText(description);
}

void ProjectSelectDialog::onAddProjectClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, 
        "选择项目配置文件", 
        QDir::homePath(), 
        "JSON配置文件 (*.json)");
    
    if (filePath.isEmpty()) return;
    
    QFileInfo fileInfo(filePath);
    QString projectName = QInputDialog::getText(this, 
        "项目名称", 
        "请输入项目名称:", 
        QLineEdit::Normal, 
        fileInfo.baseName());
    
    if (projectName.isEmpty()) return;
    
    // 检查名称是否已存在
    for (const ProjectInfo& project : m_projects) {
        if (project.name == projectName) {
            QMessageBox::warning(this, "名称重复", 
                "项目名称已存在，请选择不同的名称。");
            return;
        }
    }
    
    // 添加新项目
    ProjectInfo newProject;
    newProject.name = projectName;
    newProject.path = filePath;
    
    // 尝试读取描述
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("description")) {
                newProject.description = obj["description"].toString();
            }
        }
    }
    
    m_projects.append(newProject);
    QListWidgetItem* item = new QListWidgetItem(projectName);
    m_projectList->addItem(item);
    m_projectList->setCurrentItem(item);
    
    saveProjects();
}

void ProjectSelectDialog::onEditProjectClicked()
{
    QListWidgetItem* currentItem = m_projectList->currentItem();
    if (!currentItem) return;
    
    int index = m_projectList->row(currentItem);
    if (index < 0 || index >= m_projects.size()) return;
    
    ProjectInfo& project = m_projects[index];
    
    QString newName = QInputDialog::getText(this, 
        "编辑项目", 
        "请输入新的项目名称:", 
        QLineEdit::Normal, 
        project.name);
    
    if (newName.isEmpty() || newName == project.name) return;
    
    // 检查名称是否已存在
    for (int i = 0; i < m_projects.size(); ++i) {
        if (i != index && m_projects[i].name == newName) {
            QMessageBox::warning(this, "名称重复", 
                "项目名称已存在，请选择不同的名称。");
            return;
        }
    }
    
    // 更新项目名称
    project.name = newName;
    currentItem->setText(newName);
    
    saveProjects();
}

void ProjectSelectDialog::onRemoveProjectClicked()
{
    QListWidgetItem* currentItem = m_projectList->currentItem();
    if (!currentItem) return;
    
    int index = m_projectList->row(currentItem);
    if (index < 0 || index >= m_projects.size()) return;
    
    QString projectName = m_projects[index].name;
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, 
        "确认删除", 
        QString("确定要删除项目 \"%1\" 吗？").arg(projectName),
        QMessageBox::Yes | QMessageBox::No);
        
    if (reply != QMessageBox::Yes) return;
    
    m_projects.removeAt(index);
    delete m_projectList->takeItem(index);
    
    saveProjects();
}

void ProjectSelectDialog::onOkButtonClicked()
{
    if (m_projectList->selectedItems().isEmpty()) {
        QMessageBox::warning(this, "未选择项目", "请选择一个项目配置。");
        return;
    }
    
    // 获取选择的项目路径
    m_selectedProjectPath = getSelectedProjectPath();
    
    // 检查文件是否存在
    QFile file(m_selectedProjectPath);
    if (!file.exists()) {
        QMessageBox::warning(this, "文件不存在", "选择的配置文件不存在。");
        return;
    }
    
    accept();
}

void ProjectSelectDialog::onCancelButtonClicked()
{
    reject();
}

QString ProjectSelectDialog::getSelectedProjectPath() const
{
    QListWidgetItem* currentItem = m_projectList->currentItem();
    if (!currentItem) return QString();
    
    int index = m_projectList->row(currentItem);
    if (index < 0 || index >= m_projects.size()) return QString();

    
    return m_projects[index].path;
}

QString ProjectSelectDialog::getSelectedProjectName() const
{
    QListWidgetItem* currentItem = m_projectList->currentItem();
    if (!currentItem) return QString();
    
    int index = m_projectList->row(currentItem);
    if (index < 0 || index >= m_projects.size()) return QString();
    
    return m_projects[index].name;
}