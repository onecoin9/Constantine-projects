#include"acroViewTester.h"
#include "logManager.h"
#include <QDateTime>
#include <QInputDialog>
void acroViewTester::setupDataUI()
{
    ui.tableMain->setAlternatingRowColors(true);
    ui.tableMain->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.tableMain->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.tableMain->horizontalHeader()->setStretchLastSection(true);
    ui.tableMain->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    m_tableModel = new QStandardItemModel(this);
    ui.tableMain->setModel(m_tableModel);

    QStringList headers = { "日志序号", "日志名称", "日志发生时间", "日志类型", "日志内容", "告警详情", "告警数据" };
    m_tableModel->setHorizontalHeaderLabels(headers);


    updatePageInfo();
}

void acroViewTester::addSampleData()
{
    struct SampleLogEntry {
        QString eventName;
        LogManager::LogLevel level;
        QString additionalDetail;
        QString jsonData;
    };

    QVector<SampleLogEntry> sampleEntries = {
        {
            QStringLiteral("系统启动"),
            LogManager::Info,
            QStringLiteral("系统正常启动，所有模块已加载"),
            QStringLiteral("{\"module_count\": 5}")
        },
        {
            QStringLiteral("温度告警"),
            LogManager::Warn,
            QStringLiteral("CPU核心温度达到阈值"),
            QStringLiteral("{\"sensor\":\"CPU01\", \"temp\":85, \"threshold\":80}")
        },
        {
            QStringLiteral("用户登录尝试"),
            LogManager::Debug,
            QStringLiteral("用户 'testuser' 尝试登录"),
            QStringLiteral("{\"ip\":\"192.168.1.100\"}")
        },
        {
            QStringLiteral("数据库连接失败"),
            LogManager::Error,
            QStringLiteral("无法连接到主数据库服务器"),
            QStringLiteral("{\"server\":\"main_db_server\", \"error_code\":1045}")
        }
    };

    for (const auto& entry : sampleEntries) {

        LogManager::instance().addLogToDB(entry.level, entry.eventName, entry.additionalDetail, entry.jsonData);
    }
}

void acroViewTester::on_exportButton_clicked() 
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("导出Excel文件"),
        "", tr("Excel Files (*.xlsx *.xls)"));
    if (fileName.isEmpty()) return;
    exportToExcel(fileName);
}

void acroViewTester::exportToExcel(const QString& fileName)
{
    QStandardItemModel* modelToExport = LogManager::instance().getLogModel();
    if (!modelToExport) {
        QMessageBox::warning(this, tr("错误"), tr("日志模型不可用"));
        return;
    }

    // 创建 Excel 应用程序实例
    QAxObject* excel = new QAxObject("Excel.Application", this);
    if (excel->isNull()) {
        QMessageBox::critical(this,
            QStringLiteral("错误"),
            QStringLiteral("无法创建Excel应用程序实例"));
        delete excel;
        return;
    }
    excel->setProperty("Visible", false); // 后台运行

    QAxObject* workbooks = excel->querySubObject("Workbooks");
    QAxObject* workbook = workbooks->querySubObject("Add");
    QAxObject* worksheets = workbook->querySubObject("Worksheets");

    if (!worksheets) {
        QMessageBox::critical(this,
            QString::fromUtf8("错误"),
            QString::fromUtf8("无法获取工作表集合"));
        excel->dynamicCall("Quit()");
        delete excel;
        return;
    }

    QAxObject* worksheet = worksheets->querySubObject("Item(int)", 1);
    worksheet->setProperty("Name", QString::fromUtf8("日志导出"));

    // 写入表头
    for (int col = 0; col < modelToExport->columnCount(); ++col) {
        QAxObject* cell = worksheet->querySubObject("Cells(int,int)", 1, col + 1);
        QString headerText = modelToExport->headerData(col, Qt::Horizontal).toString();
        cell->setProperty("Value", headerText);
        // cell->setProperty("HorizontalAlignment", -4108); // xlCenter
        delete cell;
    }

    // 写入数据
    for (int row = 0; row < modelToExport->rowCount(); ++row) {
        for (int col = 0; col < modelToExport->columnCount(); ++col) {
            QAxObject* cell = worksheet->querySubObject("Cells(int,int)", row + 2, col + 1);
            QStandardItem* item = modelToExport->item(row, col);
            if (item) {
                cell->setProperty("Value", item->text());
            }
            // cell->setProperty("HorizontalAlignment", -4108);
            delete cell;
        }
    }

    QAxObject* usedRange = worksheet->querySubObject("UsedRange");
    if (usedRange) {
        usedRange->querySubObject("Columns")->dynamicCall("AutoFit");
        delete usedRange;
    }

    workbook->dynamicCall("SaveAs(const QString&)", QDir::toNativeSeparators(fileName));
    workbook->dynamicCall("Close(false)"); // Close without saving changes again
    excel->dynamicCall("Quit()");

    delete worksheet;
    delete worksheets;
    delete workbook;
    delete workbooks;
    delete excel;

    QMessageBox::information(this,
        QString::fromUtf8("导出完成"),
        QString::fromUtf8("成功导出到：%1").arg(fileName));
}

// 修改函数名
void acroViewTester::on_btnFirst_clicked()
{
    m_currentPage = 1;
    updatePageInfo();
}

void acroViewTester::on_btnPreVious_clicked()
{
    if (m_currentPage > 1) {
        m_currentPage--;
        updatePageInfo();
    }
}

void acroViewTester::on_btnNext_clicked()
{
    if (m_currentPage < m_totalPages) {
        m_currentPage++;
        updatePageInfo();
    }
}

void acroViewTester::on_btnLast_clicked()
{
    m_currentPage = m_totalPages;
    updatePageInfo();
}

void acroViewTester::on_btnSelect_clicked()
{
    // 获取查询条件
    QString searchText = QInputDialog::getText(this, tr("查询"), tr("请输入查询关键词:"));
    if (searchText.isEmpty()) return;

    // 重置所有行的显示状态（先取消分页隐藏）
    for (int row = 0; row < m_tableModel->rowCount(); ++row) {
        ui.tableMain->setRowHidden(row, false);
    }

    // 根据查询条件过滤表格行
    bool foundAny = false;
    for (int row = 0; row < m_tableModel->rowCount(); ++row) {
        bool matchFound = false;

        // 在所有列中查找匹配的文本
        for (int col = 0; col < m_tableModel->columnCount(); ++col) {
            QStandardItem* item = m_tableModel->item(row, col);
            if (item && item->text().contains(searchText, Qt::CaseInsensitive)) {
                matchFound = true;
                foundAny = true;
                break;
            }
        }

        // 隐藏不匹配的行
        ui.tableMain->setRowHidden(row, !matchFound);
    }

    // 显示查询结果
    if (foundAny) {
        ui.labPageTotal->setText(tr("查询结果: 找到匹配项"));
        
        // 禁用分页按钮
        ui.btnFirst->setEnabled(false);
        ui.btnPreVious->setEnabled(false);
        ui.btnNext->setEnabled(false);
        ui.btnLast->setEnabled(false);
    } else {
        QMessageBox::information(this, tr("查询结果"), tr("未找到匹配的记录"));
        
        // 恢复分页显示
        updatePageInfo(true);
    }
}

void acroViewTester::onLogDataChanged()
{
    // 从 LogManager 获取最新日志数据
    QStandardItemModel* logModel = LogManager::instance().getLogModel();
    
    if (logModel) {
        // 保存当前页
        int savedCurrentPage = m_currentPage;
        
        // 清空当前模型
        m_tableModel->removeRows(0, m_tableModel->rowCount());
        
        // 复制数据到当前模型
        for (int row = 0; row < logModel->rowCount(); ++row) {
            QList<QStandardItem*> rowItems;
            for (int col = 0; col < logModel->columnCount(); ++col) {
                QStandardItem* item = logModel->item(row, col);
                if (item) {
                    rowItems.append(new QStandardItem(item->text()));
                } else {
                    rowItems.append(new QStandardItem(""));
                }
            }
            m_tableModel->appendRow(rowItems);
        }
        
        // 尝试恢复到原来的页，或者重置到第一页
        m_currentPage = (savedCurrentPage <= m_totalPages) ? savedCurrentPage : 1;
        
        // 强制刷新分页视图
        updatePageInfo(true);
    }
}

void acroViewTester::resetSearch()
{
    // 重置搜索状态，恢复分页显示
    m_currentPage = 1;
    updatePageInfo(true);
}

void acroViewTester::updatePageInfo(bool forceRefresh)
{
    // 如果是强制刷新，先重置所有行的可见性
    if (forceRefresh) {
        for (int row = 0; row < m_tableModel->rowCount(); ++row) {
            ui.tableMain->setRowHidden(row, false);
        }
    }

    // 计算总页数
    int totalRows = m_tableModel->rowCount();
    m_totalPages = (totalRows + m_pageSize - 1) / m_pageSize; // 向上取整

    if (m_totalPages == 0) m_totalPages = 1; // 至少有一页

    // 确保当前页有效
    if (m_currentPage > m_totalPages) m_currentPage = m_totalPages;
    if (m_currentPage < 1) m_currentPage = 1;

    // 更新页码标签
    ui.labPageTotal->setText(QString("第 %1/%2 页").arg(m_currentPage).arg(m_totalPages));

    // 计算当前页应显示的行范围
    int startRow = (m_currentPage - 1) * m_pageSize;
    int endRow = qMin(startRow + m_pageSize, totalRows);

    // 显示当前页数据
    for (int row = 0; row < totalRows; ++row) {
        if (row >= startRow && row < endRow) {
            ui.tableMain->setRowHidden(row, false);
        }
        else {
            ui.tableMain->setRowHidden(row, true);
        }
    }

    // 更新导航按钮状态
    ui.btnFirst->setEnabled(m_currentPage > 1);
    ui.btnPreVious->setEnabled(m_currentPage > 1);
    ui.btnNext->setEnabled(m_currentPage < m_totalPages);
    ui.btnLast->setEnabled(m_currentPage < m_totalPages);
}

void acroViewTester::on_btnInsert_clicked()
{
    int currentRow = ui.tableMain->currentIndex().row();
    if (currentRow < 0) {
        // 如果没有选中行，则在末尾添加
        m_tableModel->insertRow(m_tableModel->rowCount());
    }
    else {
        // 获取真实行索引（考虑分页）
        int realRow = (m_currentPage - 1) * m_pageSize;
        for (int i = 0; i < m_tableModel->rowCount(); i++) {
            if (!ui.tableMain->isRowHidden(i)) {
                if (realRow == currentRow) {
                    m_tableModel->insertRow(i);
                    break;
                }
                realRow++;
            }
        }
    }

    // 更新页面信息
    updatePageInfo();
}

void acroViewTester::on_btnDelete_clicked()
{
    int currentRow = ui.tableMain->currentIndex().row();
    if (currentRow >= 0) {
        // 获取真实行索引（考虑分页）
        int realRow = -1;
        int visibleRowCount = 0;
        for (int i = 0; i < m_tableModel->rowCount(); i++) {
            if (!ui.tableMain->isRowHidden(i)) {
                if (visibleRowCount == currentRow) {
                    realRow = i;
                    break;
                }
                visibleRowCount++;
            }
        }

        if (realRow >= 0) {
            m_tableModel->removeRow(realRow);
        }
    }

    // 更新页面信息
    updatePageInfo();
}

void acroViewTester::on_importDataButton_clicked()
{
    QMenu importMenu;
    importMenu.addAction("从数据库导入", this, &acroViewTester::importFromDB);
    importMenu.addAction("从JSON文件导入", this, &acroViewTester::importFromJson);
    importMenu.addAction("从Excel导入", this, &acroViewTester::importFromExcel);

    // 显示菜单在导入按钮位置（使用新的按钮名称）
    importMenu.exec(ui.importDataButton->mapToGlobal(QPoint(0, ui.importDataButton->height())));
}

void acroViewTester::importFromDB()
{
    // 清空现有数据
    m_tableModel->removeRows(0, m_tableModel->rowCount());

    // 从日志管理器获取数据
    QStandardItemModel* dbModel = LogManager::instance().getLogModel();
    if (!dbModel) {
        QMessageBox::warning(this, tr("错误"), tr("数据库数据不可用"));
        return;
    }

    // 复制数据到当前模型
    for (int row = 0; row < dbModel->rowCount(); ++row) {
        QList<QStandardItem*> rowItems;
        for (int col = 0; col < dbModel->columnCount(); ++col) {
            QStandardItem* item = dbModel->item(row, col);
            if (item) {
                rowItems.append(new QStandardItem(item->text()));
            }
            else {
                rowItems.append(new QStandardItem(""));
            }
        }
        m_tableModel->appendRow(rowItems);
    }

    // 更新界面
    updatePageInfo();
    QMessageBox::information(this, tr("导入成功"), tr("成功从数据库导入数据"));
}

void acroViewTester::importFromJson()
{
    // 选择JSON文件
    QString fileName = QFileDialog::getOpenFileName(this, tr("导入JSON文件"),
        "", tr("JSON Files (*.json)"));
    if (fileName.isEmpty()) return;

    // 读取JSON文件
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("错误"), tr("无法打开文件"));
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    // 解析JSON
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &error);
    if (error.error != QJsonParseError::NoError) {
        QMessageBox::warning(this, tr("错误"), tr("JSON解析错误: %1").arg(error.errorString()));
        return;
    }

    if (!jsonDoc.isArray()) {
        QMessageBox::warning(this, tr("错误"), tr("JSON格式不正确，应为数组"));
        return;
    }

    // 清空现有数据
    m_tableModel->removeRows(0, m_tableModel->rowCount());

    // 导入JSON数据
    QJsonArray logArray = jsonDoc.array();
    for (int i = 0; i < logArray.size(); ++i) {
        QJsonObject logObj = logArray[i].toObject();

        QList<QStandardItem*> rowItems;
        
        // 尝试读取不同可能的字段名，确保兼容性
        QString id = logObj["日志序号"].toString();
        if (id.isEmpty()) id = logObj["id"].toString();
        
        QString name = logObj["日志名称"].toString();
        if (name.isEmpty()) name = logObj["事件"].toString();
        if (name.isEmpty()) name = logObj["name"].toString();
        
        QString time = logObj["日志发生时间"].toString();
        if (time.isEmpty()) time = logObj["时间"].toString();
        if (time.isEmpty()) time = logObj["time"].toString();
        
        QString level = logObj["日志类型"].toString();
        if (level.isEmpty()) level = logObj["级别"].toString();
        if (level.isEmpty()) level = logObj["level"].toString();
        
        QString content = logObj["日志内容"].toString();
        if (content.isEmpty()) content = logObj["内容"].toString();
        if (content.isEmpty()) content = logObj["content"].toString();
        
        QString detail = logObj["告警详情"].toString();
        if (detail.isEmpty()) detail = logObj["详细信息"].toString();
        if (detail.isEmpty()) detail = logObj["detail"].toString();
        
        QString data = logObj["告警数据"].toString();
        if (data.isEmpty()) data = logObj["JSON数据"].toString();
        if (data.isEmpty()) data = logObj["data"].toString();
        
        // 添加所有字段
        rowItems.append(new QStandardItem(id));
        rowItems.append(new QStandardItem(name));
        rowItems.append(new QStandardItem(time));
        rowItems.append(new QStandardItem(level));
        rowItems.append(new QStandardItem(content));
        rowItems.append(new QStandardItem(detail));
        rowItems.append(new QStandardItem(data));

        m_tableModel->appendRow(rowItems);
    }

    // 更新界面
    updatePageInfo(true);
    QMessageBox::information(this, tr("导入成功"), tr("成功从JSON导入数据"));
}

void acroViewTester::importFromExcel()
{
    // 选择Excel文件
    QString fileName = QFileDialog::getOpenFileName(this, tr("导入Excel文件"),
        "", tr("Excel Files (*.xlsx *.xls)"));
    if (fileName.isEmpty()) return;

    // 创建Excel应用实例
    QAxObject excel("Excel.Application");
    if (excel.isNull()) {
        QMessageBox::critical(this, tr("错误"), tr("无法创建Excel应用程序实例"));
        return;
    }

    excel.setProperty("Visible", false);
    QAxObject* workbooks = excel.querySubObject("Workbooks");
    QAxObject* workbook = workbooks->querySubObject("Open(const QString&)", fileName);
    QAxObject* worksheet = workbook->querySubObject("Worksheets(int)", 1);

    // 获取已使用范围
    QAxObject* usedRange = worksheet->querySubObject("UsedRange");
    QAxObject* rows = usedRange->querySubObject("Rows");
    QAxObject* columns = usedRange->querySubObject("Columns");

    int rowCount = rows->property("Count").toInt();
    int columnCount = columns->property("Count").toInt();

    // 清空现有数据
    m_tableModel->removeRows(0, m_tableModel->rowCount());

    // 设置表头（如果Excel第一行是表头）
    QStringList headers;
    for (int col = 1; col <= columnCount; col++) {
        QAxObject* cell = usedRange->querySubObject("Cells(int,int)", 1, col);
        headers << cell->property("Value").toString();
        delete cell;
    }
    m_tableModel->setHorizontalHeaderLabels(headers);

    // 导入数据（从第二行开始，假设第一行是表头）
    for (int row = 2; row <= rowCount; row++) {
        QList<QStandardItem*> rowItems;
        for (int col = 1; col <= columnCount; col++) {
            QAxObject* cell = usedRange->querySubObject("Cells(int,int)", row, col);
            QVariant value = cell->property("Value");
            rowItems.append(new QStandardItem(value.toString()));
            delete cell;
        }
        m_tableModel->appendRow(rowItems);
    }

    // 关闭Excel
    workbook->dynamicCall("Close(Boolean)", false);
    excel.dynamicCall("Quit()");

    delete usedRange;
    delete rows;
    delete columns;
    delete worksheet;
    delete workbook;
    delete workbooks;

    // 更新界面
    updatePageInfo();
    QMessageBox::information(this, tr("导入成功"), tr("成功从Excel导入数据"));
}