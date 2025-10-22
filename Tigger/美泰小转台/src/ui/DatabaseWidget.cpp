#include "ui/DatabaseWidget.h"
#include "sql/ChipTestDatabase.h"
#include "core/Logger.h"
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QTextStream>
#include <QHeaderView>
#include <QSplitter>
#include <QDateTime>
#include <QDebug>

namespace Presentation {

DatabaseWidget::DatabaseWidget(QWidget *parent)
    : QWidget(parent)
    , m_database(nullptr)
    , m_mainLayout(nullptr)
    , m_mainSplitter(nullptr)
    , m_queryGroup(nullptr)
    , m_dataTable(nullptr)
    , m_controlGroup(nullptr)
    , m_statusGroup(nullptr)
    , m_detailGroup(nullptr)
    , m_refreshTimer(new QTimer(this))
    , m_isInitialized(false)
    , m_totalRecords(0)
    , m_filteredRecords(0)
{
    setupUi();
    connectSignals();
    
    // 设置定时刷新
    m_refreshTimer->setSingleShot(false);
    m_refreshTimer->setInterval(30000); // 30秒刷新一次
    connect(m_refreshTimer, &QTimer::timeout, this, &DatabaseWidget::updateStatusInfo);
}

DatabaseWidget::~DatabaseWidget()
{
    closeDatabase();
}

bool DatabaseWidget::initializeDatabase(const QString &databasePath)
{
    try {
        // 创建数据库目录
        QFileInfo fileInfo(databasePath);
        QDir dir = fileInfo.absoluteDir();
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        
        m_database = std::make_unique<ChipTestDatabase>(databasePath);
        
        if (!m_database->initializeDatabase()) {
            m_lastError = m_database->getLastError();
            emit errorMessage(QString("数据库初始化失败: %1").arg(m_lastError));
            return false;
        }
        
        m_isInitialized = true;
        
        // 初始化芯片型号下拉框
        QStringList models = m_database->getAllChipModels();
        m_chipModelCombo->clear();
        m_chipModelCombo->addItem("全部型号", "");
        for (const QString &model : models) {
            m_chipModelCombo->addItem(model, model);
        }
        
        // 刷新数据显示
        refreshData();
        
        // 启动定时器
        m_refreshTimer->start();
        
        emit statusMessage("数据库初始化成功");
        LOG_MODULE_INFO("DatabaseWidget", QString("数据库初始化成功: %1").arg(databasePath).toStdString());
        
        return true;
        
    } catch (const std::exception &e) {
        m_lastError = QString("数据库初始化异常: %1").arg(e.what());
        emit errorMessage(m_lastError);
        return false;
    }
}

void DatabaseWidget::closeDatabase()
{
    if (m_refreshTimer->isActive()) {
        m_refreshTimer->stop();
    }
    
    if (m_database) {
        m_database->closeDatabase();
        m_database.reset();
    }
    
    m_isInitialized = false;
}

bool DatabaseWidget::insertChipTestData(const ChipTestData &data)
{
    if (!m_isInitialized || !m_database) {
        emit errorMessage("数据库未初始化");
        return false;
    }
    
    if (!validateChipData(data)) {
        emit errorMessage("数据验证失败");
        return false;
    }
    
    bool success = m_database->insertChipData(data);
    if (success) {
        emit dataChanged();
        emit statusMessage(QString("成功插入芯片数据: %1").arg(data.uid));
        
        // 异步刷新显示（避免频繁刷新）
        QTimer::singleShot(1000, this, &DatabaseWidget::refreshData);
    } else {
        QString error = m_database->getLastError();
        emit errorMessage(QString("插入数据失败: %1").arg(error));
    }
    
    return success;
}

bool DatabaseWidget::insertChipTestDataFromCsv(const QString &csvFilePath)
{
    if (!m_isInitialized || !m_database) {
        emit errorMessage("数据库未初始化");
        return false;
    }
    
    QList<ChipTestData> dataList;
    if (!parseCsvFile(csvFilePath, dataList)) {
        return false;
    }
    
    if (dataList.isEmpty()) {
        emit statusMessage("CSV文件中没有有效数据");
        return true;
    }
    
    // 显示进度
    m_operationProgress->setVisible(true);
    m_operationProgress->setMaximum(dataList.size());
    m_operationProgress->setValue(0);
    
    int successCount = 0;
    int failCount = 0;
    
    for (int i = 0; i < dataList.size(); ++i) {
        const ChipTestData &data = dataList[i];
        
        if (m_database->insertChipData(data)) {
            successCount++;
        } else {
            failCount++;
            LOG_MODULE_WARNING("DatabaseWidget", 
                QString("插入CSV数据失败 [%1]: %2").arg(i+1).arg(m_database->getLastError()).toStdString());
        }
        
        m_operationProgress->setValue(i + 1);
        QApplication::processEvents(); // 保持界面响应
    }
    
    m_operationProgress->setVisible(false);
    
    QString message = QString("CSV导入完成: 成功 %1 条，失败 %2 条").arg(successCount).arg(failCount);
    emit statusMessage(message);
    
    if (successCount > 0) {
        emit dataChanged();
        refreshData();
    }
    
    return failCount == 0;
}

void DatabaseWidget::refreshData()
{
    if (!m_isInitialized || !m_database) {
        return;
    }
    
    onQueryAll();
}

void DatabaseWidget::setupUi()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainSplitter = new QSplitter(Qt::Vertical, this);
    
    setupQueryPanel();
    setupDataTable();
    setupControlPanel();
    setupStatusPanel();
    
    // 设置分割器
    QWidget *topWidget = new QWidget();
    QVBoxLayout *topLayout = new QVBoxLayout(topWidget);
    topLayout->addWidget(m_queryGroup);
    topLayout->addWidget(m_controlGroup);
    topLayout->setContentsMargins(0, 0, 0, 0);
    
    QWidget *middleWidget = new QWidget();
    QVBoxLayout *middleLayout = new QVBoxLayout(middleWidget);
    middleLayout->addWidget(m_dataTable);
    middleLayout->setContentsMargins(0, 0, 0, 0);
    
    m_mainSplitter->addWidget(topWidget);
    m_mainSplitter->addWidget(middleWidget);
    m_mainSplitter->addWidget(m_statusGroup);
    
    // 设置分割器比例
    m_mainSplitter->setStretchFactor(0, 0); // 查询面板固定高度
    m_mainSplitter->setStretchFactor(1, 1); // 数据表格占主要空间
    m_mainSplitter->setStretchFactor(2, 0); // 状态面板固定高度
    
    m_mainLayout->addWidget(m_mainSplitter);
}

void DatabaseWidget::setupQueryPanel()
{
    m_queryGroup = new QGroupBox("查询条件", this);
    m_queryLayout = new QGridLayout(m_queryGroup);
    
    // 搜索框
    m_queryLayout->addWidget(new QLabel("快速搜索:"), 0, 0);
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("输入UID、批次号或打标号进行搜索...");
    m_queryLayout->addWidget(m_searchEdit, 0, 1, 1, 2);
    
    // 日期范围
    m_queryLayout->addWidget(new QLabel("开始日期:"), 1, 0);
    m_startDateEdit = new QDateEdit();
    m_startDateEdit->setDate(QDate::currentDate().addDays(-30));
    m_startDateEdit->setCalendarPopup(true);
    m_queryLayout->addWidget(m_startDateEdit, 1, 1);
    
    m_queryLayout->addWidget(new QLabel("结束日期:"), 1, 2);
    m_endDateEdit = new QDateEdit();
    m_endDateEdit->setDate(QDate::currentDate());
    m_endDateEdit->setCalendarPopup(true);
    m_queryLayout->addWidget(m_endDateEdit, 1, 3);
    
    // 批次号
    m_queryLayout->addWidget(new QLabel("批次号:"), 2, 0);
    m_batchEdit = new QLineEdit();
    m_batchEdit->setPlaceholderText("输入批次号...");
    m_queryLayout->addWidget(m_batchEdit, 2, 1);
    
    // UID
    m_queryLayout->addWidget(new QLabel("芯片UID:"), 2, 2);
    m_idEdit = new QLineEdit();
    m_idEdit->setPlaceholderText("输入芯片UID...");
    m_queryLayout->addWidget(m_idEdit, 2, 3);
    
    // 打标号
    m_queryLayout->addWidget(new QLabel("打标号:"), 3, 0);
    m_markingEdit = new QLineEdit();
    m_markingEdit->setPlaceholderText("输入打标号...");
    m_queryLayout->addWidget(m_markingEdit, 3, 1);
    
    // 芯片型号
    m_queryLayout->addWidget(new QLabel("芯片型号:"), 3, 2);
    m_chipModelCombo = new QComboBox();
    m_queryLayout->addWidget(m_chipModelCombo, 3, 3);
    
    // 查询按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_queryAllBtn = new QPushButton("查询全部");
    m_queryDateBtn = new QPushButton("按日期查询");
    m_queryBatchBtn = new QPushButton("按批次查询");
    m_queryIdBtn = new QPushButton("按UID查询");
    m_queryMarkingBtn = new QPushButton("按打标号查询");
    m_clearQueryBtn = new QPushButton("清空条件");
    
    buttonLayout->addWidget(m_queryAllBtn);
    buttonLayout->addWidget(m_queryDateBtn);
    buttonLayout->addWidget(m_queryBatchBtn);
    buttonLayout->addWidget(m_queryIdBtn);
    buttonLayout->addWidget(m_queryMarkingBtn);
    buttonLayout->addWidget(m_clearQueryBtn);
    buttonLayout->addStretch();
    
    m_queryLayout->addLayout(buttonLayout, 4, 0, 1, 4);
}

void DatabaseWidget::setupDataTable()
{
    m_dataTable = new QTableWidget(this);
    
    // 设置列
    QStringList headers = {
        "UID", "芯片型号", "批次ID", "激活时间", "打标号",
        "工作电流(测)", "工作电流(参)", "锁相环电压(测)", "锁相环电压(参)",
        "驱动电压(测)", "驱动电压(参)", "驱动峰值(测)", "驱动峰值(参)",
        "方波频率(测)", "方波频率(参)", "正弦均值(测)", "正弦均值(参)",
        "正弦峰值(测)", "正弦峰值(参)", "正弦频率(测)", "正弦频率(参)",
        "激活结果", "标定结果", "机器号", "工作流文件",
        "自动化文件", "烧录文件", "OS测试", "创建时间", "更新时间"
    };
    
    m_dataTable->setColumnCount(headers.size());
    m_dataTable->setHorizontalHeaderLabels(headers);
    
    // 设置表格属性
    m_dataTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_dataTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_dataTable->setAlternatingRowColors(true);
    m_dataTable->setSortingEnabled(true);
    
    // 设置列宽
    QHeaderView *header = m_dataTable->horizontalHeader();
    header->setStretchLastSection(true);
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents); // UID
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents); // 芯片型号
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents); // 批次ID
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents); // 激活时间
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents); // 打标号
}

void DatabaseWidget::setupControlPanel()
{
    m_controlGroup = new QGroupBox("数据操作", this);
    m_controlLayout = new QHBoxLayout(m_controlGroup);
    
    m_exportBtn = new QPushButton("导出数据");
    m_importCsvBtn = new QPushButton("导入CSV");
    m_deleteBtn = new QPushButton("删除选中");
    m_refreshBtn = new QPushButton("刷新数据");
    
    m_deleteBtn->setEnabled(false); // 初始禁用
    
    m_controlLayout->addWidget(m_exportBtn);
    m_controlLayout->addWidget(m_importCsvBtn);
    m_controlLayout->addWidget(m_deleteBtn);
    m_controlLayout->addWidget(m_refreshBtn);
    m_controlLayout->addStretch();
}

void DatabaseWidget::setupStatusPanel()
{
    m_statusGroup = new QGroupBox("统计信息", this);
    m_statusLayout = new QGridLayout(m_statusGroup);
    
    // 统计标签
    m_statusLayout->addWidget(new QLabel("总记录数:"), 0, 0);
    m_totalCountLabel = new QLabel("0");
    m_statusLayout->addWidget(m_totalCountLabel, 0, 1);
    
    m_statusLayout->addWidget(new QLabel("显示记录:"), 0, 2);
    m_filteredCountLabel = new QLabel("0");
    m_statusLayout->addWidget(m_filteredCountLabel, 0, 3);
    
    m_statusLayout->addWidget(new QLabel("通过率:"), 0, 4);
    m_passRateLabel = new QLabel("0%");
    m_statusLayout->addWidget(m_passRateLabel, 0, 5);
    
    m_statusLayout->addWidget(new QLabel("最后更新:"), 1, 0);
    m_lastUpdateLabel = new QLabel("未更新");
    m_statusLayout->addWidget(m_lastUpdateLabel, 1, 1, 1, 2);
    
    // 进度条
    m_operationProgress = new QProgressBar();
    m_operationProgress->setVisible(false);
    m_statusLayout->addWidget(m_operationProgress, 1, 3, 1, 3);
    
    // 状态标签
    m_statusLabel = new QLabel("就绪");
    m_statusLayout->addWidget(m_statusLabel, 2, 0, 1, 6);
}

void DatabaseWidget::connectSignals()
{
    // 查询按钮
    connect(m_queryAllBtn, &QPushButton::clicked, this, &DatabaseWidget::onQueryAll);
    connect(m_queryDateBtn, &QPushButton::clicked, this, &DatabaseWidget::onQueryByDate);
    connect(m_queryBatchBtn, &QPushButton::clicked, this, &DatabaseWidget::onQueryByBatch);
    connect(m_queryIdBtn, &QPushButton::clicked, this, &DatabaseWidget::onQueryById);
    connect(m_queryMarkingBtn, &QPushButton::clicked, this, &DatabaseWidget::onQueryByMarkingNumber);
    connect(m_clearQueryBtn, &QPushButton::clicked, this, &DatabaseWidget::onClearQuery);
    
    // 控制按钮
    connect(m_exportBtn, &QPushButton::clicked, this, &DatabaseWidget::onExportData);
    connect(m_importCsvBtn, &QPushButton::clicked, this, &DatabaseWidget::onImportCsv);
    connect(m_deleteBtn, &QPushButton::clicked, this, &DatabaseWidget::onDeleteSelected);
    connect(m_refreshBtn, &QPushButton::clicked, this, &DatabaseWidget::onRefreshData);
    
    // 表格选择
    connect(m_dataTable, &QTableWidget::itemSelectionChanged, 
            this, &DatabaseWidget::onTableSelectionChanged);
    
    // 搜索框
    connect(m_searchEdit, &QLineEdit::textChanged, 
            this, &DatabaseWidget::onSearchTextChanged);
    
    // 状态信号
    connect(this, &DatabaseWidget::statusMessage, 
            [this](const QString &msg) { m_statusLabel->setText(msg); });
    connect(this, &DatabaseWidget::errorMessage, 
            [this](const QString &error) { 
                m_statusLabel->setText(QString("错误: %1").arg(error));
                QMessageBox::warning(this, "数据库错误", error);
            });
}

void DatabaseWidget::onQueryAll()
{
    if (!m_isInitialized || !m_database) {
        return;
    }
    
    emit statusMessage("正在查询所有数据...");
    
    QList<ChipTestData> dataList = m_database->getAllChipData();
    populateTable(dataList);
    
    emit statusMessage(QString("查询完成，共 %1 条记录").arg(dataList.size()));
    updateStatistics();
}

void DatabaseWidget::onQueryByDate()
{
    if (!m_isInitialized || !m_database) {
        return;
    }
    
    QDateTime startTime = QDateTime(m_startDateEdit->date());
    QDateTime endTime = QDateTime(m_endDateEdit->date().addDays(1)); // 包含结束日期整天
    
    emit statusMessage("正在按日期查询...");
    
    QList<ChipTestData> dataList = m_database->getChipDataByDateRange(startTime, endTime);
    populateTable(dataList);
    
    emit statusMessage(QString("日期查询完成，共 %1 条记录").arg(dataList.size()));
    updateStatistics();
}

void DatabaseWidget::onQueryByBatch()
{
    if (!m_isInitialized || !m_database) {
        return;
    }
    
    QString batchId = m_batchEdit->text().trimmed();
    if (batchId.isEmpty()) {
        QMessageBox::information(this, "提示", "请输入批次号");
        return;
    }
    
    emit statusMessage("正在按批次查询...");
    
    QList<ChipTestData> dataList = m_database->getChipDataByLot(batchId);
    populateTable(dataList);
    
    emit statusMessage(QString("批次查询完成，共 %1 条记录").arg(dataList.size()));
    updateStatistics();
}

void DatabaseWidget::onQueryById()
{
    if (!m_isInitialized || !m_database) {
        return;
    }
    
    QString uid = m_idEdit->text().trimmed();
    if (uid.isEmpty()) {
        QMessageBox::information(this, "提示", "请输入芯片UID");
        return;
    }
    
    emit statusMessage("正在按UID查询...");
    
    // 由于UID需要配合批次号作为复合主键，这里进行模糊查询
    QList<ChipTestData> allData = m_database->getAllChipData();
    QList<ChipTestData> filteredData;
    
    for (const ChipTestData &data : allData) {
        if (data.uid.contains(uid, Qt::CaseInsensitive)) {
            filteredData.append(data);
        }
    }
    
    populateTable(filteredData);
    
    emit statusMessage(QString("UID查询完成，共 %1 条记录").arg(filteredData.size()));
    updateStatistics();
}

void DatabaseWidget::onQueryByMarkingNumber()
{
    if (!m_isInitialized || !m_database) {
        return;
    }
    
    QString markingNumber = m_markingEdit->text().trimmed();
    if (markingNumber.isEmpty()) {
        QMessageBox::information(this, "提示", "请输入打标号");
        return;
    }
    
    emit statusMessage("正在按打标号查询...");
    
    // 进行模糊查询
    QList<ChipTestData> allData = m_database->getAllChipData();
    QList<ChipTestData> filteredData;
    
    for (const ChipTestData &data : allData) {
        if (data.markingNumber.contains(markingNumber, Qt::CaseInsensitive)) {
            filteredData.append(data);
        }
    }
    
    populateTable(filteredData);
    
    emit statusMessage(QString("打标号查询完成，共 %1 条记录").arg(filteredData.size()));
    updateStatistics();
}

void DatabaseWidget::onClearQuery()
{
    m_searchEdit->clear();
    m_startDateEdit->setDate(QDate::currentDate().addDays(-30));
    m_endDateEdit->setDate(QDate::currentDate());
    m_batchEdit->clear();
    m_idEdit->clear();
    m_markingEdit->clear();
    m_chipModelCombo->setCurrentIndex(0);
    
    clearTable();
    emit statusMessage("查询条件已清空");
}

void DatabaseWidget::onExportData()
{
    if (m_dataTable->rowCount() == 0) {
        QMessageBox::information(this, "提示", "没有数据可以导出");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this, 
        "导出数据", 
        QString("chip_test_data_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss")),
        "CSV文件 (*.csv)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法创建导出文件");
        return;
    }
    
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    
    // 写入表头
    QStringList headers;
    for (int col = 0; col < m_dataTable->columnCount(); ++col) {
        headers << m_dataTable->horizontalHeaderItem(col)->text();
    }
    stream << headers.join(",") << "\n";
    
    // 写入数据
    for (int row = 0; row < m_dataTable->rowCount(); ++row) {
        QStringList rowData;
        for (int col = 0; col < m_dataTable->columnCount(); ++col) {
            QTableWidgetItem *item = m_dataTable->item(row, col);
            rowData << (item ? item->text() : "");
        }
        stream << rowData.join(",") << "\n";
    }
    
    file.close();
    
    emit statusMessage(QString("数据已导出到: %1").arg(fileName));
    QMessageBox::information(this, "导出完成", QString("成功导出 %1 条记录到文件:\n%2")
                            .arg(m_dataTable->rowCount()).arg(fileName));
}

void DatabaseWidget::onImportCsv()
{
    QString fileName = QFileDialog::getOpenFileName(this, 
        "导入CSV文件", 
        "",
        "CSV文件 (*.csv);;所有文件 (*.*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    if (insertChipTestDataFromCsv(fileName)) {
        QMessageBox::information(this, "导入完成", "CSV文件导入成功");
    } else {
        QMessageBox::critical(this, "导入失败", QString("CSV文件导入失败: %1").arg(m_lastError));
    }
}

void DatabaseWidget::onDeleteSelected()
{
    QList<QTableWidgetItem*> selectedItems = m_dataTable->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }
    
    // 获取选中的行
    QSet<int> selectedRows;
    for (QTableWidgetItem *item : selectedItems) {
        selectedRows.insert(item->row());
    }
    
    int ret = QMessageBox::question(this, "确认删除", 
        QString("确定要删除选中的 %1 条记录吗？\n此操作不可撤销！").arg(selectedRows.size()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (ret != QMessageBox::Yes) {
        return;
    }
    
    if (!m_isInitialized || !m_database) {
        return;
    }
    
    int successCount = 0;
    int failCount = 0;
    
    for (int row : selectedRows) {
        QTableWidgetItem *uidItem = m_dataTable->item(row, 0);
        QTableWidgetItem *lotItem = m_dataTable->item(row, 2);
        
        if (uidItem && lotItem) {
            QString uid = uidItem->text();
            QString lotid = lotItem->text();
            
            if (m_database->deleteChipData(uid, lotid)) {
                successCount++;
            } else {
                failCount++;
            }
        }
    }
    
    QString message = QString("删除完成: 成功 %1 条，失败 %2 条").arg(successCount).arg(failCount);
    emit statusMessage(message);
    
    if (successCount > 0) {
        emit dataChanged();
        refreshData();
    }
}

void DatabaseWidget::onRefreshData()
{
    refreshData();
    emit statusMessage("数据已刷新");
}

void DatabaseWidget::updateStatistics()
{
    if (!m_isInitialized || !m_database) {
        return;
    }
    
    m_totalRecords = m_database->getChipCount();
    m_filteredRecords = m_dataTable->rowCount();
    
    m_totalCountLabel->setText(QString::number(m_totalRecords));
    m_filteredCountLabel->setText(QString::number(m_filteredRecords));
    
    // 计算通过率
    int passCount = 0;
    for (int row = 0; row < m_dataTable->rowCount(); ++row) {
        QTableWidgetItem *activationItem = m_dataTable->item(row, 21); // 激活结果列
        QTableWidgetItem *calibrationItem = m_dataTable->item(row, 22); // 标定结果列
        
        if (activationItem && calibrationItem) {
            bool activationPass = (activationItem->text() == "通过");
            bool calibrationPass = (calibrationItem->text() == "通过");
            
            if (activationPass && calibrationPass) {
                passCount++;
            }
        }
    }
    
    double passRate = (m_filteredRecords > 0) ? (double(passCount) / m_filteredRecords * 100.0) : 0.0;
    m_passRateLabel->setText(QString("%1%").arg(passRate, 0, 'f', 1));
    
    m_lastUpdateLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
}

void DatabaseWidget::onTableSelectionChanged()
{
    bool hasSelection = !m_dataTable->selectedItems().isEmpty();
    m_deleteBtn->setEnabled(hasSelection);
}

void DatabaseWidget::onSearchTextChanged()
{
    QString searchText = m_searchEdit->text().trimmed();
    if (searchText.isEmpty()) {
        return;
    }
    
    // 实时搜索功能
    if (!m_isInitialized || !m_database) {
        return;
    }
    
    QList<ChipTestData> allData = m_database->getAllChipData();
    QList<ChipTestData> filteredData;
    
    for (const ChipTestData &data : allData) {
        if (data.uid.contains(searchText, Qt::CaseInsensitive) ||
            data.lotid.contains(searchText, Qt::CaseInsensitive) ||
            data.markingNumber.contains(searchText, Qt::CaseInsensitive) ||
            data.chipModel.contains(searchText, Qt::CaseInsensitive)) {
            filteredData.append(data);
        }
    }
    
    populateTable(filteredData);
    updateStatistics();
}

void DatabaseWidget::updateStatusInfo()
{
    if (m_isInitialized && m_database) {
        updateStatistics();
    }
}

void DatabaseWidget::populateTable(const QList<ChipTestData> &dataList)
{
    clearTable();
    
    if (dataList.isEmpty()) {
        return;
    }
    
    m_dataTable->setRowCount(dataList.size());
    
    for (int row = 0; row < dataList.size(); ++row) {
        const ChipTestData &data = dataList[row];
        
        int col = 0;
        m_dataTable->setItem(row, col++, new QTableWidgetItem(data.uid));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(data.chipModel));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(data.lotid));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(data.activationTime));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(data.markingNumber));
        
        // 测量数据
        m_dataTable->setItem(row, col++, new QTableWidgetItem(QString::number(data.workingCurrentMeasured, 'f', 3)));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(QString::number(data.workingCurrentReference, 'f', 3)));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(QString::number(data.pllDcVoltageMeasured, 'f', 3)));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(QString::number(data.pllDcVoltageReference, 'f', 3)));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(QString::number(data.driveDcVoltageMeasured, 'f', 3)));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(QString::number(data.driveDcVoltageReference, 'f', 3)));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(QString::number(data.driveDcPeakVoltageMeasured, 'f', 3)));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(QString::number(data.driveDcPeakVoltageReference, 'f', 3)));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(QString::number(data.squareWaveFreqMeasured, 'f', 2)));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(QString::number(data.squareWaveFreqReference, 'f', 2)));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(QString::number(data.sineWaveVoltageAvgMeasured, 'f', 3)));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(QString::number(data.sineWaveVoltageAvgReference, 'f', 3)));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(QString::number(data.sineWavePeakVoltageMeasured, 'f', 3)));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(QString::number(data.sineWavePeakVoltageReference, 'f', 3)));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(QString::number(data.sineWaveFreqMeasured, 'f', 2)));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(QString::number(data.sineWaveFreqReference, 'f', 2)));
        
        // 测试结果
        m_dataTable->setItem(row, col++, new QTableWidgetItem(formatBoolValue(data.activationOkNg)));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(formatBoolValue(data.turntableCalibrationOkNg)));
        
        // 文件信息
        m_dataTable->setItem(row, col++, new QTableWidgetItem(data.machineNumber));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(data.workflowFile));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(data.automationTaskFile));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(data.burnTaskFile));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(formatBoolValue(data.osTestResult)));
        
        // 时间戳
        m_dataTable->setItem(row, col++, new QTableWidgetItem(formatDateTime(data.createdAt)));
        m_dataTable->setItem(row, col++, new QTableWidgetItem(formatDateTime(data.updatedAt)));
    }
    
    m_filteredRecords = dataList.size();
}

void DatabaseWidget::clearTable()
{
    m_dataTable->setRowCount(0);
    m_filteredRecords = 0;
}

QString DatabaseWidget::formatDateTime(const QDateTime &dateTime) const
{
    if (dateTime.isValid()) {
        return dateTime.toString("yyyy-MM-dd hh:mm:ss");
    }
    return "";
}

QString DatabaseWidget::formatBoolValue(bool value) const
{
    return value ? "通过" : "失败";
}

bool DatabaseWidget::parseCsvFile(const QString &filePath, QList<ChipTestData> &dataList)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("无法打开CSV文件: %1").arg(filePath);
        return false;
    }
    
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    
    bool isFirstLine = true;
    int lineNumber = 0;
    
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        lineNumber++;
        
        if (line.isEmpty()) {
            continue;
        }
        
        // 跳过表头
        if (isFirstLine) {
            isFirstLine = false;
            continue;
        }
        
        QStringList fields = line.split(',');
        
        try {
            ChipTestData data = parseCsvLine(fields);
            if (validateChipData(data)) {
                dataList.append(data);
            } else {
                LOG_MODULE_WARNING("DatabaseWidget", 
                    QString("CSV第%1行数据验证失败").arg(lineNumber).toStdString());
            }
        } catch (const std::exception &e) {
            LOG_MODULE_WARNING("DatabaseWidget", 
                QString("CSV第%1行解析失败: %2").arg(lineNumber).arg(e.what()).toStdString());
        }
    }
    
    file.close();
    return true;
}

ChipTestData DatabaseWidget::parseCsvLine(const QStringList &fields)
{
    ChipTestData data;
    
    // 这里需要根据实际的CSV格式进行解析
    // 以下是示例解析逻辑，需要根据MT软件的CSV格式调整
    
    if (fields.size() >= 5) {
        data.uid = fields[0].trimmed();
        data.chipModel = fields[1].trimmed();
        data.lotid = fields[2].trimmed();
        data.activationTime = fields[3].trimmed();
        data.markingNumber = fields[4].trimmed();
    }
    
    // 解析测量数据（需要根据实际CSV格式调整索引）
    if (fields.size() >= 25) {
        bool ok;
        data.workingCurrentMeasured = fields[5].toDouble(&ok);
        if (ok) data.workingCurrentReference = fields[6].toDouble();
        if (ok) data.pllDcVoltageMeasured = fields[7].toDouble();
        if (ok) data.pllDcVoltageReference = fields[8].toDouble();
        // ... 继续解析其他字段
    }
    
    // 设置默认值
    data.activationOkNg = true;
    data.turntableCalibrationOkNg = true;
    data.osTestResult = true;
    
    return data;
}

bool DatabaseWidget::validateChipData(const ChipTestData &data) const
{
    // 基本验证
    if (data.uid.isEmpty() || data.lotid.isEmpty()) {
        return false;
    }
    
    // 可以添加更多验证逻辑
    return true;
}

} // namespace Presentation
