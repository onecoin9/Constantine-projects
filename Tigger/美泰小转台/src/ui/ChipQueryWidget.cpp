#include "ChipQueryWidget.h"
#include "ui_ChipQueryWidget.h"
#include <QDate>
#include <QDebug>

ChipQueryWidget::ChipQueryWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ChipQueryWidget),
    model(new QStandardItemModel(this))
{
    ui->setupUi(this);

    // 设置默认日期范围为最近30天
    QDate endDate = QDate::currentDate();
    QDate startDate = endDate.addDays(-30);
    ui->startDateEdit->setDate(startDate);
    ui->endDateEdit->setDate(endDate);

    // 初始化模型和表格
    setupModel();
    populateModelWithTestData();

    // 连接信号槽
    connect(ui->pageSizeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &ChipQueryWidget::on_pageSizeComboBox_currentIndexChanged);
}

ChipQueryWidget::~ChipQueryWidget()
{
    delete ui;
    delete model;
}

void ChipQueryWidget::setupModel()
{
    // 设置表头
    model->setHorizontalHeaderLabels({
        tr("芯片UID"), tr("芯片型号"), tr("批次ID"), tr("激活时间"),
        tr("打标号"), tr("激活结果"), tr("转台标定"), tr("OS测试"), tr("机器号"), tr("创建时间")
        });

    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
}

void ChipQueryWidget::populateModelWithTestData()
{
    // 清空现有数据
    model->removeRows(0, model->rowCount());

    // 添加测试数据
    QList<QList<QVariant>> testData = {
        {"CHIP202310001", "AX1001", "LOT202310A", "2023-10-15 09:30:25",
         "MK202310001", "OK", "OK", "OK", "MACH001", "2023-10-15 10:15:30"},
        {"CHIP202310002", "AX1002", "LOT202310B", "2023-10-16 14:20:10",
         "MK202310002", "NG", "OK", "NG", "MACH002", "2023-10-16 15:05:45"},
        {"CHIP202310003", "AX1001", "LOT202310A", "2023-10-17 11:45:30",
         "MK202310003", "OK", "NG", "OK", "MACH001", "2023-10-17 12:30:15"}
    };

    for (const auto& rowData : testData) {
        QList<QStandardItem*> items;
        for (const auto& data : rowData) {
            QStandardItem* item = new QStandardItem(data.toString());
            // 根据测试结果设置颜色
            if (data == "OK") {
                item->setForeground(Qt::darkGreen);
            }
            else if (data == "NG") {
                item->setForeground(Qt::red);
            }
            items.append(item);
        }
        model->appendRow(items);
    }

    ui->resultCountLabel->setText(QString("查询结果: %1 条记录").arg(model->rowCount()));
}

void ChipQueryWidget::on_searchButton_clicked()
{
    // 获取查询条件
    QDate startDate = ui->startDateEdit->date();
    QDate endDate = ui->endDateEdit->date();
    QString lotId = ui->lotIdLineEdit->text();
    QString uid = ui->uidLineEdit->text();
    QString markingNo = ui->markingNoLineEdit->text();

    // 在实际应用中，这里应该执行数据库查询
    // 这里我们只是重新加载测试数据来模拟查询
    populateModelWithTestData();

    qDebug() << "执行查询:"
        << "开始日期:" << startDate.toString("yyyy-MM-dd")
        << "结束日期:" << endDate.toString("yyyy-MM-dd")
        << "批次ID:" << lotId
        << "芯片UID:" << uid
        << "打标号:" << markingNo;
}

void ChipQueryWidget::on_resetButton_clicked()
{
    // 重置日期范围为最近30天
    QDate endDate = QDate::currentDate();
    QDate startDate = endDate.addDays(-30);
    ui->startDateEdit->setDate(startDate);
    ui->endDateEdit->setDate(endDate);

    // 清空其他输入框
    ui->lotIdLineEdit->clear();
    ui->uidLineEdit->clear();
    ui->markingNoLineEdit->clear();

    // 重置表格显示所有数据
    populateModelWithTestData();
}

void ChipQueryWidget::on_pageSizeComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index)
        int pageSize = ui->pageSizeComboBox->currentText().toInt();
    qDebug() << "每页显示数量改为:" << pageSize;
    // 在实际应用中，这里应该重新查询并分页显示数据
}
