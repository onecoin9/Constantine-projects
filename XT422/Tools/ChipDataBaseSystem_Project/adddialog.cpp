#include "adddialog.h"
#include "ui_adddialog.h"

AddDialog::AddDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddDialog),
    m_pDataTableModel(Q_NULLPTR),
    m_pAdapterDataTableModel(Q_NULLPTR),
    m_bIsAdapter(false)
{
    ui->setupUi(this);

    InitText();
    InitTable();
    InitButton();
}

AddDialog::~AddDialog()
{
    delete ui;
}

void AddDialog::InitText()
{
    ui->dataName->setText(tr("Name:"));
    ui->adapterID->setText(tr("ID:"));
    ui->addButton->setText(tr("Add"));
    ui->saveButton->setText(tr("Save"));
    ui->deleteButton->setText(tr("Delete"));
    ui->selectButton->setText(tr("Select"));
}

void AddDialog::InitTable()
{
    /*初始化非Adapter表格*/
    m_pDataTableModel = new QStandardItemModel(ui->dataTableView);

    // 隐藏水平表头
    ui->dataTableView->verticalHeader()->setVisible(false);

    QStringList headList;
    headList << tr("Name");

    m_pDataTableModel->setHorizontalHeaderLabels(headList);
    ui->dataTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->dataTableView->setModel(m_pDataTableModel);
    ui->dataTableView->setAlternatingRowColors(true);
    ui->dataTableView->horizontalHeader()->setHighlightSections(false);
    ui->dataTableView->horizontalHeader()->setStretchLastSection(true);
    ui->dataTableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

    QHeaderView* manuHead = ui->dataTableView->horizontalHeader();

    // 列宽度自适应效果不好，自定宽度并可以拖动
    manuHead->setSectionResizeMode(QHeaderView::Interactive);

    connect(ui->dataTableView, &QTableView::clicked, this, &AddDialog::onSlotClickTableView);
    connect(ui->dataTableView, &QTableView::doubleClicked, this, &AddDialog::onSlotClickTableView);

    /*初始化Adapter表格*/
    m_pAdapterDataTableModel = new QStandardItemModel(ui->adapterTableView);
    // 隐藏水平表头
    ui->adapterTableView->verticalHeader()->setVisible(false);

    QStringList adapterHeadList;
    adapterHeadList << tr("Name") << tr("CheckID");

    m_pAdapterDataTableModel->setHorizontalHeaderLabels(adapterHeadList);
    ui->adapterTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->adapterTableView->setModel(m_pAdapterDataTableModel);
    ui->adapterTableView->setAlternatingRowColors(true);
    ui->adapterTableView->horizontalHeader()->setHighlightSections(false);
    ui->adapterTableView->horizontalHeader()->setStretchLastSection(true);
    ui->adapterTableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

    QHeaderView* adapterHead = ui->adapterTableView->horizontalHeader();

    // 列宽度自适应效果不好，自定宽度并可以拖动
    adapterHead->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(ui->adapterTableView, &QTableView::clicked, this, &AddDialog::onSlotClickAdapterTableView);
    connect(ui->adapterTableView, &QTableView::doubleClicked, this, &AddDialog::onSlotClickAdapterTableView);
}

void AddDialog::InitButton()
{
    connect(ui->addButton, &QPushButton::clicked, this, &AddDialog::onSlotClickOperDataBase);
    connect(ui->saveButton, &QPushButton::clicked, this, &AddDialog::onSlotClickOperDataBase);
    connect(ui->deleteButton, &QPushButton::clicked, this, &AddDialog::onSlotClickOperDataBase);
    connect(ui->selectButton, &QPushButton::clicked, this, &AddDialog::onSlotClickOperDataBase);
}

void AddDialog::setGroupText(QString infoText, QString tableText)
{
    ui->InfoWgt->setTitle(infoText);
    ui->InfoTableWgt->setTitle(tableText);
    ui->dataNameEdit->setText(m_pDataTableModel->data(m_pDataTableModel->index(0, 0)).toString());
    m_originalValue = m_pDataTableModel->data(m_pDataTableModel->index(0, 0)).toString();
}

void AddDialog::setTitle(QString strTitle, bool bShowTable)
{
    this->setWindowTitle(strTitle);
    m_bIsAdapter = bShowTable;
    if(bShowTable)
    {
        ui->adapterIDWgt->show();
        ui->adapterTableView->show();
        ui->dataTableView->hide();
    }
    else
    {
        ui->adapterIDWgt->hide();
        ui->adapterTableView->hide();
        ui->dataTableView->show();
    }
}

void AddDialog::addData(QString dataValue)
{
    int row = m_pDataTableModel->rowCount();
    m_pDataTableModel->insertRow(m_pDataTableModel->rowCount());
    m_pDataTableModel->setData(m_pDataTableModel->index(row, 0), dataValue);
    tableReset();
}

void AddDialog::addAdapterData(QString dataValue, QString idValue)
{
    int row = m_pAdapterDataTableModel->rowCount();
    m_pAdapterDataTableModel->insertRow(m_pAdapterDataTableModel->rowCount());
    m_pAdapterDataTableModel->setData(m_pAdapterDataTableModel->index(row, 0), dataValue);
    m_pAdapterDataTableModel->setData(m_pAdapterDataTableModel->index(row, 1), idValue);
    tableReset();
}

void AddDialog::clearTable()
{
    m_pDataTableModel->removeRows(0, m_pDataTableModel->rowCount());
    m_pAdapterDataTableModel->removeRows(0, m_pAdapterDataTableModel->rowCount());
    tableReset();
}

void AddDialog::tableReset()
{
    ui->dataTableView->reset();
    ui->adapterTableView->reset();
}

void AddDialog::onSlotClickTableView(const QModelIndex &index)
{
    QString strName = m_pDataTableModel->data(m_pDataTableModel->index(index.row(), 0)).toString();
    ui->dataNameEdit->setText(strName);
    m_originalValue = strName;
}

void AddDialog::onSlotClickOperDataBase()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());

    if(!m_bIsAdapter)
    {
        if(ui->addButton == btn)
            emit sgnOperDB(operDBType::Add, ui->dataNameEdit->text(), m_originalValue);
        else if(ui->saveButton == btn)
            emit sgnOperDB(operDBType::Save, ui->dataNameEdit->text(), m_originalValue);
        else if(ui->deleteButton == btn)
            emit sgnOperDB(operDBType::Delete, ui->dataNameEdit->text(), m_originalValue);
    }
    else
    {
        if(ui->addButton == btn)
        {
            emit sgnAdapterOperDB(operDBType::Add, ui->dataNameEdit->text(), ui->adapterIDEdit->text(), m_originalValue);
        }
        else if(ui->saveButton == btn)
        {
            emit sgnAdapterOperDB(operDBType::Save, ui->dataNameEdit->text(), ui->adapterIDEdit->text(), m_originalValue);
        }
        else if(ui->deleteButton == btn)
        {
            emit sgnAdapterOperDB(operDBType::Delete, ui->dataNameEdit->text(), ui->adapterIDEdit->text(), m_originalValue);
        }
    }

    if(ui->selectButton == btn)
    {
        emit sgnSelectData(ui->dataNameEdit->text());
        close();
    }
    //clearTable();
}

void AddDialog::onSlotClickAdapterTableView(const QModelIndex &index)
{
    QString strName = m_pAdapterDataTableModel->data(m_pAdapterDataTableModel->index(index.row(), 0)).toString();
    ui->dataNameEdit->setText(strName);
    QString strID = m_pAdapterDataTableModel->data(m_pAdapterDataTableModel->index(index.row(), 1)).toString();
    ui->adapterIDEdit->setText(strID);
    m_originalValue = strName;
}
