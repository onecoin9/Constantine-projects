#include "configDialog.h"
#include "ui_ConfigDialog.h"
#include <QTableWidgetItem>
#include <QtWidgets>
#include <QtWidgets/qmessagebox.h>
ConfigDialog::ConfigDialog(QWidget* parent) : QWidget(parent),
ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);

    // 连接 btnInport 按钮
    connect(ui->btnInport, &QPushButton::clicked, this, [=]()
        { on_btnInport_clicked("btnInport"); });

    // 连接 btnInportSpec 按钮
    connect(ui->btnInportSpec, &QPushButton::clicked, this, [=]()
        { on_btnInport_clicked("btnInportSpec"); });
    // 设置样式表
    tableStyleSheet = R"(
    QTableWidget {
        color: #000000; /* 单元格文字颜色改为黑色 */
        background: #f8f8f8;
        border: 1px solid #242424;
        alternate-background-color: #525252; /* 交替行颜色 */
        gridline-color: #242424;
    }
    QTableWidget::item {
            text-align: center;
        }
    QTableWidget::item:selected {
        color: #000000; /* 选中项文字颜色改为黑色 */
        background: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #484848, stop:1 #383838);
    }
    QTableWidget::item:hover {
        background: #5B5B5B;
    }
    QHeaderView::section {
        text-align: center;
        background: #c5e3f6; /* 新的表头背景色 */
        padding: 3px;
        margin: 0px;
        color: #000000; /* 表头文字颜色改为黑色 */
        border: 1px solid #242424;
        border-left-width: 0;
    }
    QScrollBar:vertical {
        background: #484848;
        padding: 0px;
        border-radius: 6px;
        max-width: 12px;
    }
    QScrollBar::handle:vertical {
        background: #CCCCCC;
    }
    QScrollBar::handle:hover:vertical, QScrollBar::handle:pressed:vertical {
        background: #A7A7A7;
    }
    QScrollBar::sub-page:vertical {
        background: #444444;
    }
    QScrollBar::add-page:vertical {
        background: #5B5B5B;
    }
    QScrollBar::add-line:vertical {
        background: none;
    }
    QScrollBar::sub-line:vertical {
        background: none;
    }
    QTableWidget::item:selected {
        background: #e2f3f5; /* 选中项背景色 */
    }
    QTableWidget::item:hover {
        background: #88bef5; /* 悬浮项背景色 */
    }
    QTableWidget {
        alternate-background-color: #43dde6; /* 交替行背景色 */
    }
    QTableWidget::item {
        color: #000000; /* 单元格文字颜色改为黑色 */
    }
)";

    initTableFlow();
    initTableSpec();
    initTableBin();
    initTablePin();
    initTableTestPlan(); 
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::initTableFlow()
{
    ui->tableFlow->setRowCount(0);
    ui->tableFlow->setColumnCount(9);

    QStringList headers = { "序号","ID", "Class Name", "Alias", "Loop Count", "Test Mode", "Pass Do", "Fail Do", "Comment" };
    ui->tableFlow->setHorizontalHeaderLabels(headers);

    ui->tableFlow->horizontalHeader()->setStretchLastSection(true);
    ui->tableFlow->verticalHeader()->setVisible(false);
    ui->tableFlow->setColumnWidth(2, 200);

    ui->tableFlow->setStyleSheet(tableStyleSheet);
}

void ConfigDialog::initTableSpec()
{
    ui->tableSpec->setRowCount(0);
    ui->tableSpec->setColumnCount(9);  // 减少列数

    QStringList headers = { 
        "ID", 
        "TestItemName", 
        "Alias Name", 
        "Params",      
        "Value",
        "Lower Limit", // 新增下限列
        "Upper Limit", // 新增上限列
        "Bin Name", 
        "Comment" 
    };
    ui->tableSpec->setHorizontalHeaderLabels(headers);

    ui->tableSpec->horizontalHeader()->setStretchLastSection(true);
    ui->tableSpec->verticalHeader()->setVisible(false);
    ui->tableSpec->setColumnWidth(1, 200);
    ui->tableSpec->setColumnWidth(5, 100); 
    ui->tableSpec->setColumnWidth(6, 100); 
    ui->tableSpec->setStyleSheet(tableStyleSheet);
}
void ConfigDialog::updateTableFlow()
{
    // 暂时断开信号连接
    disconnect(ui->tableFlow, &QTableWidget::cellChanged, this, &ConfigDialog::on_tableFlowCellChanged);

    ui->tableFlow->setRowCount(testItems.size());
    for (int i = 0; i < testItems.size(); ++i)
    {
        QTableWidgetItem* indexItem = new QTableWidgetItem(QString::number(i + 1));
        indexItem->setTextAlignment(Qt::AlignCenter);
        indexItem->setFlags(indexItem->flags() & ~Qt::ItemIsEditable); // 禁止编辑序号列
        ui->tableFlow->setItem(i, 0, indexItem);
        const TestItem& item = testItems[i];
        QTableWidgetItem* idItem = new QTableWidgetItem(item.id);
        QTableWidgetItem* classNameItem = new QTableWidgetItem(item.className);
        QTableWidgetItem* aliasItem = new QTableWidgetItem(item.alias);
        QTableWidgetItem* loopCountItem = new QTableWidgetItem(item.loopCount);
        QTableWidgetItem* passDoItem = new QTableWidgetItem(item.passDo);
        QTableWidgetItem* failDoItem = new QTableWidgetItem(item.failDo);
        QTableWidgetItem* comment = new QTableWidgetItem(item.comment);

        idItem->setTextAlignment(Qt::AlignCenter);
        classNameItem->setTextAlignment(Qt::AlignCenter);
        aliasItem->setTextAlignment(Qt::AlignCenter);
        loopCountItem->setTextAlignment(Qt::AlignCenter);
        passDoItem->setTextAlignment(Qt::AlignCenter);
        failDoItem->setTextAlignment(Qt::AlignCenter);
        comment->setTextAlignment(Qt::AlignCenter);

        ui->tableFlow->setItem(i, 1, idItem);
        ui->tableFlow->setItem(i, 2, classNameItem);
        ui->tableFlow->setItem(i, 3, aliasItem);
        ui->tableFlow->setItem(i, 4, loopCountItem);
        ui->tableFlow->setItem(i, 6, passDoItem);
        ui->tableFlow->setItem(i, 7, failDoItem);
        ui->tableFlow->setItem(i, 8, comment);

        QComboBox* comboBox = new QComboBox(this);
        comboBox->addItems({ "Skip", "Test" });    // Add options for the combo box
        comboBox->setCurrentText(item.testMode); // Set the current text to the item's test mode

        // Set the color based on the current text
        auto updateComboBoxColor = [comboBox](const QString& mode)
        {
            if (mode == "Test")
            {
                comboBox->setStyleSheet("QComboBox { color: green; }");
            }
            else if (mode == "Skip")
            {
                comboBox->setStyleSheet("QComboBox { color: red; }");
            }
        };

        // Apply the initial color
        updateComboBoxColor(item.testMode);

        connect(comboBox, &QComboBox::currentTextChanged, this, [=](const QString& mode)
            {
                updateComboBoxColor(mode); // Update the color when the text changes
                on_tableFlowTestModeChanged(i, mode); });

        ui->tableFlow->setCellWidget(i, 5, comboBox);
    }

    // 恢复信号连接
    connect(ui->tableFlow, &QTableWidget::cellChanged, this, &ConfigDialog::on_tableFlowCellChanged);
}

void ConfigDialog::populateTableSpecRow(int row, const SpecItem& item)
{
    ui->tableSpec->setItem(row, 0, new QTableWidgetItem(item.id));
    ui->tableSpec->setItem(row, 1, new QTableWidgetItem(item.testItemName));
    ui->tableSpec->setItem(row, 2, new QTableWidgetItem(item.aliasName));
    ui->tableSpec->setItem(row, 3, new QTableWidgetItem(item.params));
    ui->tableSpec->setItem(row, 4, new QTableWidgetItem(item.value));
    ui->tableSpec->setItem(row, 5, new QTableWidgetItem(item.lowerLimit));
    ui->tableSpec->setItem(row, 6, new QTableWidgetItem(item.upperLimit));
    ui->tableSpec->setItem(row, 7, new QTableWidgetItem(item.binName));
    ui->tableSpec->setItem(row, 8, new QTableWidgetItem(item.comment));

}

void ConfigDialog::updateTableSpec()
{
    // 暂时断开信号连接
    disconnect(ui->tableSpec, &QTableWidget::cellChanged, this, &ConfigDialog::on_tableSpecCellChanged);

    ui->tableSpec->setRowCount(specItems.size());
    for (int i = 0; i < specItems.size(); ++i)
    {
        populateTableSpecRow(i, specItems[i]);
    }

    // 恢复信号连接
    connect(ui->tableSpec, &QTableWidget::cellChanged, this, &ConfigDialog::on_tableSpecCellChanged);
}

void ConfigDialog::on_tableFlowTestModeChanged(int row, const QString& mode)
{
    QTableWidgetItem* modeItem = ui->tableFlow->item(row, 4);
    if (!modeItem)
    {
        modeItem = new QTableWidgetItem(mode);
        ui->tableFlow->setItem(row, 4, modeItem);
    }
    else
    {
        modeItem->setText(mode);
    }
}

void ConfigDialog::updateSpecFromFlow(const TestItem& flowItem)
{
    // Parse comment string to get parameters
    QStringList params = parseCommentParams(flowItem.comment);
    
    // Generate spec items based on flow item
    generateSpecItems(flowItem, params);
    
    // Update the spec table
    updateTableSpec();
}

QStringList ConfigDialog::parseCommentParams(const QString& comment)
{
    // Split comment by separator (& or space)
    QStringList params = comment.split(QRegExp("[&\\s]+"), Qt::SkipEmptyParts);
    return params;
}

void ConfigDialog::generateSpecItems(const TestItem& flowItem, const QStringList& params)
{
    // Remove existing spec items for this flow item
    auto it = specItems.begin();
    while (it != specItems.end()) {
        if (it->testItemName == flowItem.className && 
            it->aliasName == flowItem.alias) {
            it = specItems.erase(it);
        } else {
            ++it;
        }
    }

    // Create new spec items for each parameter
    for (int i = 0; i < params.size(); ++i) {
        SpecItem specItem;
        specItem.id = generateSpecItemID(flowItem.id, i);
        specItem.testItemName = flowItem.className;
        specItem.aliasName = flowItem.alias;
        specItem.params = params[i];
        specItem.value = "";
        specItem.binName = QString("Bin_%1").arg(flowItem.id);
        specItem.comment = "";

        specItems.append(specItem);
    }
}

QString ConfigDialog::generateSpecItemID(const QString& baseID, int index)
{
    return QString("%1%2").arg(baseID).arg(QChar('a' + index));
}

void ConfigDialog::syncSpecWithFlow()
{
    for (const TestItem& flowItem : testItems) {
        updateSpecFromFlow(flowItem);
    }
}
void ConfigDialog::on_btnInport_clicked(const QString& buttonId)
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择配置文件"), "", tr("JSON 文件 (*.json *.tester_config)"));
    if (filePath.isEmpty())
    {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("错误"), tr("无法打开文件：%1").arg(filePath));
        return;
    }

    QByteArray fileData = file.readAll();
    file.close();

    // 判断按钮 ID，决定更新哪个表格
    if (buttonId == "btnInport")
    {
        // 更新 tableFlow
        QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
        if (jsonDoc.isNull() || !jsonDoc.isObject())
        {
            QMessageBox::warning(this, tr("错误"), tr("文件格式错误或内容无效"));
            return;
        }

        QJsonObject rootObj = jsonDoc.object();
        if (rootObj.contains("FlowConfig") && rootObj["FlowConfig"].isObject())
        {
            QJsonObject flowConfig = rootObj["FlowConfig"].toObject();
            if (flowConfig.contains("TestItems") && flowConfig["TestItems"].isArray())
            {
                QJsonArray jsonTestItems = flowConfig["TestItems"].toArray();
                testItems.clear(); // 清空之前的数据

                for (const QJsonValue& value : jsonTestItems)
                {
                    QJsonObject item = value.toObject();
                    TestItem testItem;
                    testItem.id = item.value("TestItemID").toString();
                    testItem.className = item.value("ClassName").toString();
                    testItem.alias = item.value("Alias").toString();
                    testItem.testMode = item.value("TestMode").toString();
                    testItem.loopCount = QString::number(item.value("LoopCount").toInt());
                    testItem.passDo = item.value("PassDo").toString();
                    testItem.failDo = item.value("FailDo").toString();
                    testItem.comment = item.contains("Comment") ? item.value("Comment").toString() : "";

                    testItems.append(testItem);
                }
                updateTableFlow(); // 更新 tableFlow
            }
            else
            {
                QMessageBox::warning(this, tr("错误"), tr("FlowConfig 中缺少 TestItems 部分"));
            }
        }
        else
        {
            QMessageBox::warning(this, tr("错误"), tr("配置文件中缺少 FlowConfig 部分"));
        }
    }
    else if (buttonId == "btnInportSpec")
    {
        // 更新 tableSpec
        QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
        if (jsonDoc.isNull() || !jsonDoc.isObject())
        {
            QMessageBox::warning(this, tr("错误"), tr("文件格式错误或内容无效"));
            return;
        }

        QJsonObject rootObj = jsonDoc.object();
        if (rootObj.contains("SpecConfig") && rootObj["SpecConfig"].isObject())
        {
            QJsonObject specConfig = rootObj["SpecConfig"].toObject();
            if (specConfig.contains("SpecList") && specConfig["SpecList"].isArray())
            {
                QJsonArray jsonSpecItems = specConfig["SpecList"].toArray();
                specItems.clear(); // 清空之前的数据

                for (const QJsonValue& value : jsonSpecItems)
                {
                    QJsonObject itemObj = value.toObject();
                    SpecItem specItem;
                    specItem.id = itemObj.value("SpecItemID").toString();
                    specItem.testItemName = itemObj.value("TestItemName").toString();
                    specItem.aliasName = itemObj.value("AliasName").toString();
                    specItem.params = itemObj.value("Params").toString();
                    specItem.value = itemObj.value("Value").toString();
                    specItem.lowerLimit = itemObj.value("LowerLimit").toString(); // 添加下限
                    specItem.upperLimit = itemObj.value("UpperLimit").toString(); // 添加上限
                    specItem.binName = itemObj.value("BinName").toString();
                    specItem.comment = itemObj.contains("Comment") ?
                        itemObj.value("Comment").toString() : "";

                    specItems.append(specItem);
                }

                updateTableSpec(); // 更新 tableSpec
            }
            else
            {
                QMessageBox::warning(this, tr("错误"), tr("SpecConfig 中缺少 SpecItems 部分"));
            }
        }
        else
        {
            QMessageBox::warning(this, tr("错误"), tr("配置文件中缺少 SpecConfig 部分"));
        }
    }
    else
    {
        QMessageBox::warning(this, tr("错误"), tr("FlowConfig 中缺少 TestItems 部分"));
    }

}

void ConfigDialog::loadFlowConfig(const QJsonObject& flowConfig)
{
    if (!flowConfig.contains("TestItems") || !flowConfig["TestItems"].isArray())
    {
        QMessageBox::warning(this, tr("错误"), tr("FlowConfig 中缺少 TestItems 部分"));
        return;
    }

    QJsonArray testItems = flowConfig["TestItems"].toArray();

    // 设置 tableFlow 的行数和列数
    ui->tableFlow->setRowCount(testItems.size());
    ui->tableFlow->setColumnCount(8);

    // 遍历 TestItems 数据
    for (int i = 0; i < testItems.size(); ++i)
    {
        QJsonObject item = testItems[i].toObject();

        QString id = QString::number(item.value("TestItemID").toInt());
        QString className = item.value("ClassName").toString();
        QString alias = item.value("Alias").toString();
        QString testMode = item.value("TestMode").toString();
        QString loopCount = QString::number(item.value("LoopCount").toInt());
        QString passDo = item.value("PassDo").toString();
        QString failDo = item.value("FailDo").toString();
        QString comment = item.contains("Comment") ? item.value("Comment").toString() : "";

        // 创建表格项
        QTableWidgetItem* idItem = new QTableWidgetItem(id);
        QTableWidgetItem* classNameItem = new QTableWidgetItem(className);
        QTableWidgetItem* aliasItem = new QTableWidgetItem(alias);
        QTableWidgetItem* loopCountItem = new QTableWidgetItem(loopCount);
        QTableWidgetItem* passDoItem = new QTableWidgetItem(passDo);
        QTableWidgetItem* failDoItem = new QTableWidgetItem(failDo);
        QTableWidgetItem* commentItem = new QTableWidgetItem(comment);

        // 设置表格项到 tableFlow
        ui->tableFlow->setItem(i, 0, idItem);
        ui->tableFlow->setItem(i, 1, classNameItem);
        ui->tableFlow->setItem(i, 2, aliasItem);
        ui->tableFlow->setItem(i, 3, loopCountItem);
        ui->tableFlow->setItem(i, 5, passDoItem);
        ui->tableFlow->setItem(i, 6, failDoItem);
        ui->tableFlow->setItem(i, 7, commentItem);

        // 设置 Test Mode 列为 QComboBox
        QComboBox* comboBox = new QComboBox(this);
        comboBox->addItems({ "Skip", "Test" });
        comboBox->setCurrentText(testMode); // 设置默认值
        connect(comboBox, &QComboBox::currentTextChanged, this, [=](const QString& mode)
            { on_tableFlowTestModeChanged(i, mode); });
        ui->tableFlow->setCellWidget(i, 4, comboBox);
    }
}

void ConfigDialog::initTablePin()
{
    // 初始化 Pin 配置表
    ui->tablePinConfig->setRowCount(0);
    ui->tablePinConfig->setColumnCount(5);

    QStringList pinHeaders = { "Name", "PinType", "Sites", "Disable", "Comment" };
    ui->tablePinConfig->setHorizontalHeaderLabels(pinHeaders);

    ui->tablePinConfig->horizontalHeader()->setStretchLastSection(true);
    ui->tablePinConfig->verticalHeader()->setVisible(false);
    ui->tablePinConfig->setColumnWidth(0, 150); // 设置列宽
    ui->tablePinConfig->setStyleSheet(tableStyleSheet);

    // 初始化 Pin Group 配置表
    ui->tablePinGroupConfig->setRowCount(0);
    ui->tablePinGroupConfig->setColumnCount(3);

    QStringList pinGroupHeaders = { "Name", "Value", "Comment" };
    ui->tablePinGroupConfig->setHorizontalHeaderLabels(pinGroupHeaders);

    ui->tablePinGroupConfig->horizontalHeader()->setStretchLastSection(true);
    ui->tablePinGroupConfig->verticalHeader()->setVisible(false);
    ui->tablePinGroupConfig->setColumnWidth(0, 150); // 设置列宽
    ui->tablePinGroupConfig->setStyleSheet(tableStyleSheet);
}

void ConfigDialog::initTableBin()
{
    ui->tableBinConfig->setRowCount(0);
    ui->tableBinConfig->setColumnCount(6);

    QStringList headers = { "Name", "SW_BIN", "HW_BIN", "IsPassBin", "Disable", "Comment" };
    ui->tableBinConfig->setHorizontalHeaderLabels(headers);

    ui->tableBinConfig->horizontalHeader()->setStretchLastSection(true);
    ui->tableBinConfig->verticalHeader()->setVisible(false);
    ui->tableBinConfig->setColumnWidth(0, 150); // 设置列宽
    ui->tableBinConfig->setStyleSheet(tableStyleSheet);
}

void ConfigDialog::initTableTestPlan()
{
    ui->tableTestPlan->setRowCount(0);
    ui->tableTestPlan->setColumnCount(12);  

    QStringList headers = {
        "测试项", "是否是低温", "电源供电", "电压", "电源稳定时间", "温箱", 
        "温度", "温度稳定时间", "是否进行功能测试", "是否采集", 
        "采集频率(s)", "采集时间"
    };
    ui->tableTestPlan->setHorizontalHeaderLabels(headers);

    ui->tableTestPlan->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->tableTestPlan->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    ui->tableTestPlan->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    
    ui->tableTestPlan->horizontalHeader()->setStretchLastSection(true);
    ui->tableTestPlan->verticalHeader()->setVisible(false);
    
    // 设置各列宽度
    ui->tableTestPlan->setColumnWidth(0, 80);  
    ui->tableTestPlan->setColumnWidth(1, 80);  
    ui->tableTestPlan->setColumnWidth(2, 80);  
    ui->tableTestPlan->setColumnWidth(3, 80);  
    ui->tableTestPlan->setColumnWidth(4, 100); 
    ui->tableTestPlan->setColumnWidth(5, 100); 
    ui->tableTestPlan->setColumnWidth(6, 80);  
    ui->tableTestPlan->setColumnWidth(7, 100); 
    ui->tableTestPlan->setColumnWidth(8, 120); 
    ui->tableTestPlan->setColumnWidth(9, 100); 
    ui->tableTestPlan->setColumnWidth(10, 100);
    ui->tableTestPlan->setColumnWidth(11, 100);

    ui->tableTestPlan->setStyleSheet(tableStyleSheet);
    
    connect(ui->tableTestPlan, &QTableWidget::cellChanged, 
            this, &ConfigDialog::on_tableTestPlanCellChanged);
}

void ConfigDialog::populateTableTestPlanRow(int row, const TestPlanItem& item)
{
  
    QTableWidgetItem* testItem = new QTableWidgetItem(item.testItem);
    QTableWidgetItem* powerSupply = new QTableWidgetItem(item.powerSupply);
    QTableWidgetItem* voltage = new QTableWidgetItem(item.voltage);
    QTableWidgetItem* powerStableTime = new QTableWidgetItem(item.powerStableTime);
    QTableWidgetItem* temperature = new QTableWidgetItem(item.temperature);
    QTableWidgetItem* tempStableTime = new QTableWidgetItem(item.tempStableTime);
    QTableWidgetItem* acqFrequency = new QTableWidgetItem(item.acqFrequency);
    QTableWidgetItem* acqTime = new QTableWidgetItem(item.acqTime);

    testItem->setTextAlignment(Qt::AlignCenter);
    powerSupply->setTextAlignment(Qt::AlignCenter);
    voltage->setTextAlignment(Qt::AlignCenter);
    powerStableTime->setTextAlignment(Qt::AlignCenter);
    temperature->setTextAlignment(Qt::AlignCenter);
    tempStableTime->setTextAlignment(Qt::AlignCenter);
    acqFrequency->setTextAlignment(Qt::AlignCenter);
    acqTime->setTextAlignment(Qt::AlignCenter);

    ui->tableTestPlan->setItem(row, 0, testItem);
    ui->tableTestPlan->setItem(row, 2, powerSupply);
    ui->tableTestPlan->setItem(row, 3, voltage);
    ui->tableTestPlan->setItem(row, 4, powerStableTime);
    ui->tableTestPlan->setItem(row, 6, temperature);
    ui->tableTestPlan->setItem(row, 7, tempStableTime);
    ui->tableTestPlan->setItem(row, 10, acqFrequency);
    ui->tableTestPlan->setItem(row, 11, acqTime);
}

void ConfigDialog::setupComboBoxForTestPlanTable(int row)
{
    QComboBox* isLowTempComboBox = new QComboBox();
    isLowTempComboBox->addItems({"是", "否"});
    if (row < testPlanItems.size()) {
        isLowTempComboBox->setCurrentText(testPlanItems[row].isLowTemp);
    } else {
        isLowTempComboBox->setCurrentText("否"); 
    }
    connect(isLowTempComboBox, &QComboBox::currentTextChanged, this, [=](const QString& text) {
        on_isLowTempChanged(row, text);
    });
    ui->tableTestPlan->setCellWidget(row, 1, isLowTempComboBox);

    QComboBox* chamberComboBox = new QComboBox();
    chamberComboBox->addItems({"定值", "斜坡", "不控制"});
    if (row < testPlanItems.size()) {
        chamberComboBox->setCurrentText(testPlanItems[row].chamberMode);
    } else {
        chamberComboBox->setCurrentText("定值"); 
    }
    connect(chamberComboBox, &QComboBox::currentTextChanged, this, [=](const QString& text) {
        on_chamberModeChanged(row, text);
    });
    ui->tableTestPlan->setCellWidget(row, 5, chamberComboBox);

    QComboBox* funcTestComboBox = new QComboBox();
    funcTestComboBox->addItems({"是", "否"});
    if (row < testPlanItems.size()) {
        funcTestComboBox->setCurrentText(testPlanItems[row].funcTest);
    } else {
        funcTestComboBox->setCurrentText("是"); 
    }
    connect(funcTestComboBox, &QComboBox::currentTextChanged, this, [=](const QString& text) {
        on_funcTestChanged(row, text);
    });
    ui->tableTestPlan->setCellWidget(row, 8, funcTestComboBox);

    QComboBox* dataAcqComboBox = new QComboBox();
    dataAcqComboBox->addItems({"是", "否"});
    if (row < testPlanItems.size()) {
        dataAcqComboBox->setCurrentText(testPlanItems[row].dataAcq);
    } else {
        dataAcqComboBox->setCurrentText("否"); 
    }
    connect(dataAcqComboBox, &QComboBox::currentTextChanged, this, [=](const QString& text) {
        on_dataAcqChanged(row, text);
    });
    ui->tableTestPlan->setCellWidget(row, 9, dataAcqComboBox);
}

void ConfigDialog::updateTableTestPlan()
{
    disconnect(ui->tableTestPlan, &QTableWidget::cellChanged, 
               this, &ConfigDialog::on_tableTestPlanCellChanged);

    ui->tableTestPlan->setRowCount(testPlanItems.size());
    for (int i = 0; i < testPlanItems.size(); ++i) {
        populateTableTestPlanRow(i, testPlanItems[i]);
        setupComboBoxForTestPlanTable(i);
    }

    connect(ui->tableTestPlan, &QTableWidget::cellChanged, 
            this, &ConfigDialog::on_tableTestPlanCellChanged);
}

void ConfigDialog::on_chamberModeChanged(int row, const QString& mode)
{
    if (row >= 0 && row < testPlanItems.size()) {
        testPlanItems[row].chamberMode = mode;
    }
}

void ConfigDialog::on_funcTestChanged(int row, const QString& value)
{
    if (row >= 0 && row < testPlanItems.size()) {
        testPlanItems[row].funcTest = value;
    }
}

void ConfigDialog::on_dataAcqChanged(int row, const QString& value)
{
    if (row >= 0 && row < testPlanItems.size()) {
        testPlanItems[row].dataAcq = value;
    }
}

void ConfigDialog::on_tableTestPlanCellChanged(int row, int column)
{
    if (row < 0 || row >= testPlanItems.size()) {
        return;
    }

    TestPlanItem& item = testPlanItems[row];
    switch (column) {
    case 0: 
        item.testItem = ui->tableTestPlan->item(row, column)->text();
        break;
    case 2: 
        item.powerSupply = ui->tableTestPlan->item(row, column)->text();
        break;
    case 3: 
        item.voltage = ui->tableTestPlan->item(row, column)->text();
        break;
    case 4: 
        item.powerStableTime = ui->tableTestPlan->item(row, column)->text();
        break;
    case 6: 
        item.temperature = ui->tableTestPlan->item(row, column)->text();
        break;
    case 7: 
        item.tempStableTime = ui->tableTestPlan->item(row, column)->text();
        break;
    case 10: 
        item.acqFrequency = ui->tableTestPlan->item(row, column)->text();
        break;
    case 11: 
        item.acqTime = ui->tableTestPlan->item(row, column)->text();
        break;
    }

    QTableWidgetItem* item1 = ui->tableTestPlan->item(row, column);
    if (item1) {
        item1->setTextAlignment(Qt::AlignCenter);
    }
}

void ConfigDialog::on_isLowTempChanged(int row, const QString& value)
{
    if (row >= 0 && row < testPlanItems.size()) {
        testPlanItems[row].isLowTemp = value;
    }
}

void ConfigDialog::on_btnAddTestPlan_clicked()
{
    TestPlanItem newItem;
    newItem.testItem = "新测试项";
    newItem.powerSupply = "";
    newItem.voltage = "";
    newItem.powerStableTime = "5";  
    newItem.chamberMode = "定值";
    newItem.temperature = "25";     
    newItem.tempStableTime = "10"; 
    newItem.funcTest = "是";
    newItem.dataAcq = "否";
    newItem.acqFrequency = "1";    
    newItem.acqTime = "60";        
    newItem.isLowTemp = "否";

    testPlanItems.append(newItem);

    updateTableTestPlan();

    ui->tableTestPlan->scrollToBottom();
}

void ConfigDialog::on_btnInsertTestPlan_clicked()
{
    int currentRow = ui->tableTestPlan->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择要插入的位置"));
        return;
    }

    TestPlanItem newItem;
    newItem.testItem = "新测试项";
    newItem.powerSupply = "";
    newItem.voltage = "";
    newItem.powerStableTime = "5"; 
    newItem.chamberMode = "定值";
    newItem.temperature = "25";    
    newItem.tempStableTime = "10"; 
    newItem.funcTest = "是";
    newItem.dataAcq = "否";
    newItem.acqFrequency = "1";    
    newItem.acqTime = "60";        
    newItem.isLowTemp = "否";

    testPlanItems.insert(currentRow, newItem);

    updateTableTestPlan();

    ui->tableTestPlan->selectRow(currentRow);
}

void ConfigDialog::on_btnDeleteTestPlan_clicked()
{

    int currentRow = ui->tableTestPlan->currentRow();
    if (currentRow < 0 || currentRow >= testPlanItems.size()) {
        QMessageBox::warning(this, tr("警告"), tr("请先选择要删除的行"));
        return;
    }

    testPlanItems.removeAt(currentRow);

    updateTableTestPlan();
}

void ConfigDialog::on_btnImportTestPlan_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择测试计划文件"), "", tr("JSON 文件 (*.json)"));
    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"), tr("无法打开文件：%1").arg(filePath));
        return;
    }

    QByteArray fileData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        QMessageBox::warning(this, tr("错误"), tr("文件格式错误或内容无效"));
        return;
    }

    QJsonObject rootObj = jsonDoc.object();
    if (rootObj.contains("TestPlan") && rootObj["TestPlan"].isArray()) {
        QJsonArray jsonTestPlanItems = rootObj["TestPlan"].toArray();
        testPlanItems.clear(); 

        for (const QJsonValue& value : jsonTestPlanItems) {
            QJsonObject item = value.toObject();
            TestPlanItem planItem;
            planItem.testItem = item.value("TestItem").toString();
            planItem.powerSupply = item.value("PowerSupply").toString();
            planItem.voltage = item.value("Voltage").toString();
            planItem.powerStableTime = item.value("PowerStableTime").toString();
            planItem.chamberMode = item.value("ChamberMode").toString();
            planItem.temperature = item.value("Temperature").toString();
            planItem.tempStableTime = item.value("TempStableTime").toString();
            planItem.funcTest = item.value("FuncTest").toString();
            planItem.dataAcq = item.value("DataAcq").toString();
            planItem.acqFrequency = item.value("AcqFrequency").toString();
            planItem.acqTime = item.value("AcqTime").toString();
            planItem.isLowTemp = item.value("IsLowTemp").toString("否"); 
            testPlanItems.append(planItem);
        }

        updateTableTestPlan();
    } else {
        QMessageBox::warning(this, tr("错误"), tr("文件中没有找到TestPlan数据"));
    }
}

void ConfigDialog::on_btnExportTestPlan_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("保存测试计划文件"), "", tr("JSON 文件 (*.json)"));
    if (filePath.isEmpty()) {
        return;
    }

    QJsonArray testPlanArray;
    for (const TestPlanItem& item : testPlanItems) {
        QJsonObject itemObj;
        itemObj["TestItem"] = item.testItem;
        itemObj["PowerSupply"] = item.powerSupply;
        itemObj["Voltage"] = item.voltage;
        itemObj["PowerStableTime"] = item.powerStableTime;
        itemObj["ChamberMode"] = item.chamberMode;
        itemObj["Temperature"] = item.temperature;
        itemObj["TempStableTime"] = item.tempStableTime;
        itemObj["FuncTest"] = item.funcTest;
        itemObj["DataAcq"] = item.dataAcq;
        itemObj["AcqFrequency"] = item.acqFrequency;
        itemObj["AcqTime"] = item.acqTime;
        itemObj["IsLowTemp"] = item.isLowTemp; 
        testPlanArray.append(itemObj);
    }

    QJsonObject rootObj;
    rootObj["TestPlan"] = testPlanArray;
    
    QJsonDocument jsonDoc(rootObj);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"), tr("无法保存文件：%1").arg(filePath));
        return;
    }

    file.write(jsonDoc.toJson(QJsonDocument::Indented));
    file.close();

    QMessageBox::information(this, tr("成功"), tr("测试计划已成功保存到：%1").arg(filePath));
}

void ConfigDialog::on_tableSpecCellChanged(int row, int column)
{
    if (row < 0 || row >= specItems.size()) {
        return;
    }

    SpecItem& item = specItems[row];

    switch (column) {
    case 0: // ID
        item.id = ui->tableSpec->item(row, column)->text();
        break;
    case 1: // TestItemName
        item.testItemName = ui->tableSpec->item(row, column)->text();
        break;
    case 2: // AliasName
        item.aliasName = ui->tableSpec->item(row, column)->text();
        break;
    case 3: // Params
        item.params = ui->tableSpec->item(row, column)->text();
        break;
    case 4: // Value
        item.value = ui->tableSpec->item(row, column)->text();
        break;
    case 5: // Lower Limit
        item.lowerLimit = ui->tableSpec->item(row, column)->text();
        break;
    case 6: // Upper Limit
        item.upperLimit = ui->tableSpec->item(row, column)->text();
        break;
    case 7: // BinName
        item.binName = ui->tableSpec->item(row, column)->text();
        break;
    case 8: // Comment
        item.comment = ui->tableSpec->item(row, column)->text();
        break;
    }
}

void ConfigDialog::on_tableFlowCellChanged(int row, int column)
{
    if (row < 0 || row >= testItems.size())
    {
        return; // 如果行号无效，直接返回
    }

    TestItem& item = testItems[row]; // 获取对应的 TestItem

    // 根据列号更新 TestItem 的对应字段
    switch (column)
    {
    case 0: // ID
        item.id = ui->tableFlow->item(row, column)->text();
        break;
    case 1: // ClassName
        item.className = ui->tableFlow->item(row, column)->text();
        break;
    case 2: // Alias
        item.alias = ui->tableFlow->item(row, column)->text();
        break;
    case 3: // LoopCount
        item.loopCount = ui->tableFlow->item(row, column)->text();
        break;
    case 4: // TestMode (handled by ComboBox)
        if (QComboBox* comboBox = qobject_cast<QComboBox*>(ui->tableFlow->cellWidget(row, column)))
        {
            item.testMode = comboBox->currentText();
        }
        break;
    case 5: // PassDo
        item.passDo = ui->tableFlow->item(row, column)->text();
        break;
    case 6: // FailDo
        item.failDo = ui->tableFlow->item(row, column)->text();
        break;
    case 7: // Comment
        item.comment = ui->tableFlow->item(row, column)->text();
        break;
    default:
        break;
    }
}

void ConfigDialog::on_btnExportFlow_clicked()
{
    // 打开文件保存对话框
    QString filePath = QFileDialog::getSaveFileName(this, tr("保存配置文件"), "", tr("JSON 文件 (*.json *.tester_config)"));
    if (filePath.isEmpty())
    {
        return;
    }

    // 读取现有的 JSON 文件
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("错误"), tr("无法打开文件：%1").arg(filePath));
        return;
    }

    QByteArray fileData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
    if (jsonDoc.isNull() || !jsonDoc.isObject())
    {
        QMessageBox::warning(this, tr("错误"), tr("文件格式错误或内容无效"));
        return;
    }

    QJsonObject rootObject = jsonDoc.object();

    // 构造新的 TestItems 数据
    QJsonArray testItemsArray;
    for (const TestItem& item : testItems)
    {
        QJsonObject testObject;
        testObject["TestItemID"] = item.id;
        testObject["ClassName"] = item.className;
        testObject["Alias"] = item.alias;
        testObject["TestMode"] = item.testMode;
        testObject["LoopCount"] = item.loopCount.toInt();
        testObject["PassDo"] = item.passDo;
        testObject["FailDo"] = item.failDo;
        testObject["Comment"] = item.comment;

        testItemsArray.append(testObject);
    }

    // 更新 FlowConfig 部分
    if (!rootObject.contains("FlowConfig") || !rootObject["FlowConfig"].isObject())
    {
        rootObject["FlowConfig"] = QJsonObject(); // 如果 FlowConfig 不存在，则创建
    }

    QJsonObject flowConfig = rootObject["FlowConfig"].toObject();
    flowConfig["TestItems"] = testItemsArray; // 更新 TestItems
    rootObject["FlowConfig"] = flowConfig;

    // 将更新后的 JSON 写回文件
    jsonDoc.setObject(rootObject);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("错误"), tr("无法保存文件：%1").arg(filePath));
        return;
    }

    file.write(jsonDoc.toJson(QJsonDocument::Indented));
    file.close();

    QMessageBox::information(this, tr("成功"), tr("FlowConfig 已成功更新到文件：%1").arg(filePath));
}

void ConfigDialog::on_btnAddFlow_clicked()
{
    // 创建一个默认的 TestItem
    TestItem newItem;
    newItem.id = ""; // 暂时不设置 ID
    newItem.className = "NewClass";
    newItem.alias = "NewAlias";
    newItem.testMode = "Test";
    newItem.loopCount = "1";
    newItem.passDo = "";
    newItem.failDo = "";
    newItem.comment = "";

    // 添加到 testItems 列表
    testItems.append(newItem);

    // 重新分配所有 TestItem 的 ID
    for (int i = 0; i < testItems.size(); ++i)
    {
        testItems[i].id = QString::number(i + 1); // ID 从 1 开始
    }

    // 更新表格
    updateTableFlow();

    // 滚动到最后一行
    ui->tableFlow->scrollToBottom();
}

void ConfigDialog::on_btnInsertFlow_clicked()
{
    // 获取当前选中的行
    int currentRow = ui->tableFlow->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::warning(this, tr("警告"), tr("请先选择要插入的位置"));
        return;
    }

    // 创建一个默认的 TestItem
    TestItem newItem;
    newItem.id = ""; // 暂时不设置 ID
    newItem.className = "NewClass";
    newItem.alias = "NewAlias";
    newItem.testMode = "Test";
    newItem.loopCount = "1";
    newItem.passDo = "";
    newItem.failDo = "";
    newItem.comment = "";

    // 插入到 testItems 列表
    testItems.insert(currentRow, newItem);

    // 重新分配所有 TestItem 的 ID
    for (int i = 0; i < testItems.size(); ++i)
    {
        testItems[i].id = QString::number(i + 1); // ID 从 1 开始
    }

    // 更新表格
    updateTableFlow();

    // 选中插入的行
    ui->tableFlow->selectRow(currentRow);
}

void ConfigDialog::on_btnDeleteFlow_clicked()
{
    // 获取当前选中的行
    int currentRow = ui->tableFlow->currentRow();
    if (currentRow < 0 || currentRow >= testItems.size())
    {
        QMessageBox::warning(this, tr("警告"), tr("请先选择要删除的行"));
        return;
    }

    // 从 testItems 列表中删除对应的项
    testItems.removeAt(currentRow);

    // 更新表格
    updateTableFlow();
}

void ConfigDialog::on_btnMoveUpFlow_clicked()
{
    int currentRow = ui->tableFlow->currentRow();
    if (currentRow <= 0) // 如果当前行是第一行或未选中，无法上移
    {
        QMessageBox::warning(this, tr("警告"), tr("无法上移：已是第一行或未选中"));
        return;
    }

    // 交换 `specItems` 中的当前行和上一行
    std::swap(testItems[currentRow], testItems[currentRow - 1]);

    // 更新表格
    updateTableFlow();

    // 选中上移后的行
    ui->tableFlow->selectRow(currentRow - 1);
}

void ConfigDialog::on_btnMoveDownFlow_clicked()
{
    int currentRow = ui->tableFlow->currentRow();
    if (currentRow < 0 || currentRow >= testItems.size() - 1) // 如果当前行是最后一行或未选中，无法下移
    {
        QMessageBox::warning(this, tr("警告"), tr("无法下移：已是最后一行或未选中"));
        return;
    }

    // 交换 `specItems` 中的当前行和下一行
    std::swap(testItems[currentRow], testItems[currentRow + 1]);

    // 更新表格
    updateTableFlow();

    // 选中下移后的行
    ui->tableFlow->selectRow(currentRow + 1);
}

void ConfigDialog::processTestFlowItem(int testIndex)
{
    if (testIndex < 0 || testIndex >= testPlanItems.size()) {
        qDebug() << "无效的测试项索引:" << testIndex;
        return;
    }

    TestPlanItem& item = testPlanItems[testIndex];
    
    qDebug() << "开始处理测试项:" << item.testItem << "(索引:" << testIndex << ")";
    
    qDebug() << "处理低温模块...";
    if (item.isLowTemp == "是") {
        qDebug() << "该测试项为低温测试，开启冷水阀";
        openColdWaterValve();
    } else {
        qDebug() << "该测试项非低温测试，跳过冷水阀控制";
    }
    
    qDebug() << "处理电源模块...";

    if (!item.powerSupply.isEmpty()) {

        setupPowerSupply(item.powerSupply);
        
        if (!item.voltage.isEmpty()) {
            bool ok;
            double voltage = item.voltage.toDouble(&ok);
            if (ok) {
                setVoltage(voltage);
            } else {
                qDebug() << "电压值无效:" << item.voltage;
            }
        }
        
        if (!item.powerStableTime.isEmpty()) {
            bool ok;
            int stableTime = item.powerStableTime.toInt(&ok);
            if (ok && stableTime > 0) {
                qDebug() << "等待电源稳定，时间:" << stableTime << "秒";
                waitForPowerStable(stableTime);
            }
        }
    } else {
        qDebug() << "未指定电源供电方式，跳过电源模块";
    }
    
    qDebug() << "处理温度模块...";
    if (item.chamberMode == "定值") {
        if (!item.temperature.isEmpty()) {
            bool ok;
            double temp = item.temperature.toDouble(&ok);
            if (ok) {
                setFixedTemperature(temp);
                
                if (!item.tempStableTime.isEmpty()) {
                    bool ok;
                    int stableTime = item.tempStableTime.toInt(&ok);
                    if (ok && stableTime > 0) {
                        qDebug() << "等待温度稳定，时间:" << stableTime << "秒";
                        waitForTemperatureStable(stableTime);
                    }
                }
            } else {
                qDebug() << "温度值无效:" << item.temperature;
            }
        }
    } else if (item.chamberMode == "斜坡") {

        if (!item.temperature.isEmpty()) {
            bool ok;
            double temp = item.temperature.toDouble(&ok);
            if (ok) {
                setRampTemperature(temp);
                
                if (!item.tempStableTime.isEmpty()) {
                    bool ok;
                    int stableTime = item.tempStableTime.toInt(&ok);
                    if (ok && stableTime > 0) {
                        qDebug() << "等待温度到达目标，时间:" << stableTime << "秒";
                        waitForTemperatureReach(stableTime);
                    }
                }
            } else {
                qDebug() << "温度值无效:" << item.temperature;
            }
        }
    } else if (item.chamberMode == "不控制") {
        qDebug() << "温箱模式为不控制，跳过温度设置";
    } else {
        qDebug() << "未知的温箱模式:" << item.chamberMode;
    }
    
    qDebug() << "处理ID读取模块...";
    QString deviceID = readDeviceID();
    if (!deviceID.isEmpty()) {
        qDebug() << "成功读取设备ID:" << deviceID;
    } else {
        qDebug() << "读取设备ID失败!";

    }
    
    qDebug() << "处理功能测试模块...";
    if (item.funcTest == "是") {
        qDebug() << "执行功能测试...";
        bool funcTestResult = executeFunctionTest(testIndex, item.testItem);
        if (funcTestResult) {
            qDebug() << "功能测试通过!";
        } else {
            qDebug() << "功能测试失败!";
        }
    } else {
        qDebug() << "跳过功能测试模块";
    }
    
    qDebug() << "处理数据采集模块...";
    if (item.dataAcq == "是") {
        bool ok1, ok2;
        int frequency = item.acqFrequency.toInt(&ok1);
        int duration = item.acqTime.toInt(&ok2);
        
        if (ok1 && ok2 && frequency > 0 && duration > 0) {
            qDebug() << "开始采集数据，频率:" << frequency << "秒，时长:" << duration << "秒";
            startDataAcquisition(frequency, duration);
        } else {
            qDebug() << "采集参数无效，频率:" << item.acqFrequency 
                     << "，时长:" << item.acqTime;
        }
    } else {
        qDebug() << "跳过数据采集模块";
    }
    
    qDebug() << "测试项" << item.testItem << "处理完成！";
}

void ConfigDialog::openColdWaterValve()
{
    
    qDebug() << "开启冷水阀";
    // 示例：通过某种硬件接口控制冷水阀
    // coldWaterValveController.open();
    
    QThread::msleep(500);
}

void ConfigDialog::setupPowerSupply(const QString& supplyType)
{

    qDebug() << "设置电源供电方式:" << supplyType;

    if (supplyType == "内部电源") {
        // powerController.useInternalPower();
    } else if (supplyType == "外部电源") {
        // powerController.useExternalPower();
    }
    
    QThread::msleep(300);
}

void ConfigDialog::setVoltage(double voltage)
{
    // TODO: 设置电源电压
    qDebug() << "设置电压:" << voltage << "V";
    // 示例：设置电源电压
    // powerController.setVoltage(voltage);

    QThread::msleep(200);
}

void ConfigDialog::waitForPowerStable(int seconds)
{
    qDebug() << "等待电源稳定，倒计时:" << seconds << "秒";
    
    // 实现一个倒计时，可以更新UI显示剩余时间
    for (int i = seconds; i > 0; i--) {
        qDebug() << "电源稳定倒计时:" << i << "秒";
        QThread::sleep(1); // 等待1秒
        
        // emit powerStabilizingProgress(i, seconds);
    }
    
    qDebug() << "电源稳定完成";
}

void ConfigDialog::setFixedTemperature(double temperature)
{

    qDebug() << "设置定值温度:" << temperature << "°C";
    // temperatureController.setMode(FIXED_MODE);
    // temperatureController.setTargetTemperature(temperature);
    
    // 模拟操作延时
    QThread::msleep(300);
}

void ConfigDialog::setRampTemperature(double temperature)
{
    qDebug() << "设置斜坡目标温度:" << temperature << "°C";
    // temperatureController.setMode(RAMP_MODE);
    // temperatureController.setTargetTemperature(temperature);
    
    // 模拟操作延时
    QThread::msleep(300);
}

void ConfigDialog::waitForTemperatureStable(int seconds)
{

    qDebug() << "等待温度稳定，倒计时:" << seconds << "秒";
    
    for (int i = seconds; i > 0; i--) {
        qDebug() << "温度稳定倒计时:" << i << "秒";
        QThread::sleep(1); // 等待1秒
        
        // emit temperatureStabilizingProgress(i, seconds);
    }
    
    qDebug() << "温度稳定完成";
}

void ConfigDialog::waitForTemperatureReach(int seconds)
{
    qDebug() << "等待温度达到目标，倒计时:" << seconds << "秒";
    
    for (int i = seconds; i > 0; i--) {
        qDebug() << "温度到达倒计时:" << i << "秒";
        QThread::sleep(1); // 等待1秒
        
        // emit temperatureReachingProgress(i, seconds);
    }
    
    qDebug() << "温度已达到目标";
}

QString ConfigDialog::readDeviceID()
{
    qDebug() << "读取设备ID";
    
    // 这里应该实现与硬件通信读取设备ID的代码
    // 示例：模拟读取一个设备ID
    QString deviceID = "DEV" + QString::number(QDateTime::currentDateTime().toSecsSinceEpoch() % 10000);
    
    // 模拟操作延时
    QThread::msleep(500);
    
    return deviceID;
}

bool ConfigDialog::executeFunctionTest(int testIndex, const QString& testName)
{

    qDebug() << "执行功能测试 - 索引:" << testIndex << "，名称:" << testName;
    
    bool result = (QRandomGenerator::global()->bounded(100) < 90); // 90%的通过率
    
    QThread::sleep(2);
    
    return result;
}

void ConfigDialog::startDataAcquisition(int frequency, int duration)
{

    qDebug() << "开始数据采集，频率:" << frequency << "秒，时长:" << duration << "秒";
    
    int totalSamples = duration / frequency;
    
    QList<QPair<QDateTime, QVariantMap>> acquisitionData;
    
    for (int i = 0; i < totalSamples; i++) {
        QDateTime timestamp = QDateTime::currentDateTime();
        
        // 模拟读取多种数据
        QVariantMap sampleData;
        sampleData["temperature"] = 20.0 + QRandomGenerator::global()->bounded(10) + QRandomGenerator::global()->generateDouble();
        sampleData["voltage"] = 3.3 + (QRandomGenerator::global()->bounded(100) - 50) / 100.0;
        sampleData["current"] = 0.5 + (QRandomGenerator::global()->bounded(100) - 50) / 200.0;
        
        acquisitionData.append(qMakePair(timestamp, sampleData));
        
        qDebug() << "采样 #" << (i+1) << "/" << totalSamples 
                 << "，时间:" << timestamp.toString("hh:mm:ss.zzz")
                 << "，温度:" << sampleData["temperature"].toDouble()
                 << "，电压:" << sampleData["voltage"].toDouble()
                 << "，电流:" << sampleData["current"].toDouble();
        
        QThread::sleep(frequency);
    }
    
    saveAcquisitionData(acquisitionData);
    
    qDebug() << "数据采集完成，共" << acquisitionData.size() << "个样本";
}

void ConfigDialog::saveAcquisitionData(const QList<QPair<QDateTime, QVariantMap>>& data)
{
    qDebug() << "保存采集数据，样本数:" << data.size();
    
    QString filename = "acquisition_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".csv";
    
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        
        out << "Timestamp,Temperature,Voltage,Current\n";
        
        for (const auto& sample : data) {
            out << sample.first.toString("yyyy-MM-dd hh:mm:ss.zzz") << ","
                << sample.second["temperature"].toString() << ","
                << sample.second["voltage"].toString() << ","
                << sample.second["current"].toString() << "\n";
        }
        
        file.close();
        qDebug() << "数据已保存到文件:" << filename;
    } else {
        qDebug() << "无法打开文件进行数据保存:" << filename;
    }
}

void ConfigDialog::executeTestPlan()
{
    // 首先检查是否有测试计划项
    if (testPlanItems.isEmpty()) {
        qDebug() << "测试计划为空，无法执行";
        QMessageBox::warning(this, tr("警告"), tr("测试计划为空，无法执行"));
        return;
    }
    
    qDebug() << "开始执行测试计划，共" << testPlanItems.size() << "个测试项";
    
    // 对于每一个测试计划项
    for (int i = 0; i < testPlanItems.size(); i++) {
        TestPlanItem& item = testPlanItems[i];
        
        qDebug() << "\n======== 执行测试项 " << (i+1) << "/" << testPlanItems.size() 
                 << ": " << item.testItem << " ========";
        
        // 处理环境设置（低温、电源、温度等）
        setupTestEnvironment(item);
        
        // 如果该测试项需要进行功能测试
        if (item.funcTest == "是") {
            qDebug() << "执行功能测试...";
            
            // 执行ChipTest Flow (将Flow和Spec关联起来)
            executeChipTestFlow();
        }
        
        // 处理数据采集
        if (item.dataAcq == "是") {
            handleDataAcquisition(item);
        }
        
        qDebug() << "测试项" << item.testItem << "处理完成！";
    }
    
    qDebug() << "\n====== 测试计划执行完成 ======";
    QMessageBox::information(this, tr("完成"), tr("测试计划执行完成"));
}

void ConfigDialog::setupTestEnvironment(const TestPlanItem& item)
{
    // 处理低温模块
    if (item.isLowTemp == "是") {
        qDebug() << "该测试项为低温测试，开启冷水阀";
        openColdWaterValve();
    } else {
        qDebug() << "该测试项非低温测试，跳过冷水阀控制";
    }
    
    // 处理电源模块
    if (!item.powerSupply.isEmpty()) {
        setupPowerSupply(item.powerSupply);
        
        if (!item.voltage.isEmpty()) {
            bool ok;
            double voltage = item.voltage.toDouble(&ok);
            if (ok) {
                setVoltage(voltage);
            } else {
                qDebug() << "电压值无效:" << item.voltage;
            }
        }
        
        if (!item.powerStableTime.isEmpty()) {
            bool ok;
            int stableTime = item.powerStableTime.toInt(&ok);
            if (ok && stableTime > 0) {
                qDebug() << "等待电源稳定，时间:" << stableTime << "秒";
                waitForPowerStable(stableTime);
            }
        }
    }
    
    // 处理温度模块
    handleTemperatureSettings(item);
}

void ConfigDialog::handleTemperatureSettings(const TestPlanItem& item)
{
    if (item.chamberMode == "定值") {
        if (!item.temperature.isEmpty()) {
            bool ok;
            double temp = item.temperature.toDouble(&ok);
            if (ok) {
                setFixedTemperature(temp);
                
                if (!item.tempStableTime.isEmpty()) {
                    bool ok;
                    int stableTime = item.tempStableTime.toInt(&ok);
                    if (ok && stableTime > 0) {
                        waitForTemperatureStable(stableTime);
                    }
                }
            } else {
                qDebug() << "温度值无效:" << item.temperature;
            }
        }
    } else if (item.chamberMode == "斜坡") {
        // 斜坡温度处理...
        // 与现有代码类似
    } else if (item.chamberMode == "不控制") {
        qDebug() << "温箱模式为不控制，跳过温度设置";
    }
}

void ConfigDialog::handleDataAcquisition(const TestPlanItem& item)
{
    bool ok1, ok2;
    int frequency = item.acqFrequency.toInt(&ok1);
    int duration = item.acqTime.toInt(&ok2);
    
    if (ok1 && ok2 && frequency > 0 && duration > 0) {
        qDebug() << "开始采集数据，频率:" << frequency << "秒，时长:" << duration << "秒";
        startDataAcquisition(frequency, duration);
    } else {
        qDebug() << "采集参数无效，频率:" << item.acqFrequency 
                 << "，时长:" << item.acqTime;
    }
}
void ConfigDialog::executeChipTestFlow()
{
    // 生成集成测试流程
    ChipTestFlow flow = generateChipTestFlow();
    
    qDebug() << "开始执行ChipTest Flow: " << flow.flowName;
    qDebug() << flow.description;
    qDebug() << "共有测试项: " << flow.testItems.size() << "个";
    
    for (int i = 0; i < flow.testItems.size(); ++i) {
        const IntegratedTestItem& item = flow.testItems[i];
        
        // 如果测试模式是Skip，则跳过
        if (item.testMode.toLower() == "skip") {
            qDebug() << "跳过测试项 #" << (i+1) << ": " << item.className 
                     << " (" << item.alias << ")";
            continue;
        }
        
        qDebug() << "\n===== 执行测试项 #" << (i+1) << ": " << item.className 
                 << " (" << item.alias << ") =====";
        
        // 执行测试前的准备工作
        prepareForTest(item);
        
        // 执行测试
        bool testResult = executeTest(item);
        
        // 根据测试结果确定下一步
        if (testResult) {
            qDebug() << "测试通过! 执行PassDo: " << item.passDo;
            executePassDo(item);
        } else {
            qDebug() << "测试失败! 执行FailDo: " << item.failDo;
            executeFailDo(item);
        }
        
        // 如果有循环次数设置，执行多次
        int loopCount = item.loopCount.toInt();
        for (int loop = 1; loop < loopCount; ++loop) {
            qDebug() << "执行循环 " << (loop+1) << "/" << loopCount;
            
            testResult = executeTest(item);
            
            if (testResult) {
                qDebug() << "循环测试通过!";
            } else {
                qDebug() << "循环测试失败!";
                break;  // 可以根据需要决定是否在失败时中断循环
            }
        }
    }
    
    qDebug() << "\n===== ChipTest Flow 执行完成 =====";
}

// 测试前准备
void ConfigDialog::prepareForTest(const IntegratedTestItem& item)
{
    qDebug() << "准备执行测试: " << item.className << " (" << item.alias << ")";
    
    // 可以根据测试项的特性进行特定设置
    // 例如设置电源、温度等
    
    // 读取相关的Spec项，做必要的配置
    if (!item.specItems.isEmpty()) {
        qDebug() << "测试规格项数量: " << item.specItems.size();
        
        for (const SpecItem& spec : item.specItems) {
            qDebug() << "  - 参数: " << spec.params 
                     << ", 下限: " << spec.lowerLimit 
                     << ", 上限: " << spec.upperLimit;
            
            // 可以根据参数类型进行相应设置
            setupTestParam(spec);
        }
    }
}

// 设置测试参数
void ConfigDialog::setupTestParam(const SpecItem& spec)
{
    // 根据参数类型设置测试仪器
    // 例如，如果参数涉及电压
    if (spec.params.contains("voltage", Qt::CaseInsensitive)) {
        double value = spec.value.toDouble();
        double lower = spec.lowerLimit.toDouble();
        double upper = spec.upperLimit.toDouble();
        
        qDebug() << "设置电压测试参数: " << value 
                 << " V (范围: " << lower << " - " << upper << " V)";
    }
    // 其他参数类型的处理...
}

// 执行测试
bool ConfigDialog::executeTest(const IntegratedTestItem& item)
{
    qDebug() << "执行测试: " << item.className;
    
    // 在真实环境中，这里应该调用相应测试类的方法
    // 例如通过反射或工厂方法创建对应的测试对象
    
    // 模拟测试执行
    QThread::sleep(1);  // 模拟测试时间
    
    // 检查测试结果是否在规格范围内
    bool allPassed = true;
    for (const SpecItem& spec : item.specItems) {
        // 模拟获取测量值
        double measuredValue = simulateMeasurement(spec);
        
        // 检查是否在范围内
        double lower = spec.lowerLimit.toDouble();
        double upper = spec.upperLimit.toDouble();
        
        bool passed = (measuredValue >= lower && measuredValue <= upper);
        qDebug() << "  参数 " << spec.params 
                 << " 测量值: " << measuredValue 
                 << " 结果: " << (passed ? "通过" : "失败");
        
        if (!passed) {
            allPassed = false;
        }
    }
    
    return allPassed;
}

// 模拟测量
double ConfigDialog::simulateMeasurement(const SpecItem& spec)
{
    // 在实际应用中，这里应该是真实的测量
    // 这里只是模拟一个测量值
    
    double targetValue = spec.value.toDouble();
    double variation = (QRandomGenerator::global()->generateDouble() - 0.5) * 0.1 * targetValue;
    return targetValue + variation;
}

// 执行通过动作
void ConfigDialog::executePassDo(const IntegratedTestItem& item)
{
    if (item.passDo.isEmpty()) {
        qDebug() << "无通过后动作";
        return;
    }
    
    // 解析并执行PassDo指令
    QStringList actions = item.passDo.split(";");
    for (const QString& action : actions) {
        qDebug() << "执行PassDo动作: " << action;
        
        // 这里可以解析不同类型的动作并执行
        // 例如跳转到其他测试项、设置标志等
    }
}

// 执行失败动作
void ConfigDialog::executeFailDo(const IntegratedTestItem& item)
{
    if (item.failDo.isEmpty()) {
        qDebug() << "无失败后动作";
        return;
    }
    
    // 解析并执行FailDo指令
    QStringList actions = item.failDo.split(";");
    for (const QString& action : actions) {
        qDebug() << "执行FailDo动作: " << action;
        
        // 这里可以解析不同类型的动作并执行
    }
}

ChipTestFlow ConfigDialog::generateChipTestFlow()
{
    ChipTestFlow flow;
    flow.flowName = "ChipTest Flow";  // 可以从UI中获取
    flow.description = "Generated from ConfigDialog";

    // 遍历tableFlow的所有行，构建集成测试项
    for (int i = 0; i < testItems.size(); ++i) {
        const TestItem& item = testItems[i];

        IntegratedTestItem integratedItem;
        // 复制Flow部分数据
        integratedItem.id = item.id;
        integratedItem.className = item.className;
        integratedItem.alias = item.alias;
        integratedItem.testMode = item.testMode;
        integratedItem.loopCount = item.loopCount;
        integratedItem.passDo = item.passDo;
        integratedItem.failDo = item.failDo;
        integratedItem.comment = item.comment;

        // 查找与当前Flow项匹配的所有Spec项
        for (const SpecItem& specItem : specItems) {
            if (specItem.testItemName == item.className &&
                specItem.aliasName == item.alias) {
                integratedItem.specItems.append(specItem);
            }
        }

        flow.testItems.append(integratedItem);
    }

    return flow;
}