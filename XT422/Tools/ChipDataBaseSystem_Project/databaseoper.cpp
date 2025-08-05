#include "databaseoper.h"
#include "ui_databaseoper.h"

#include "../Model/AngKDBStandardItemModelEx.h"
#include "../Model/AngKSortFilterProxyModel.h"
#include "../DataBaseOper/DBController.hpp"
#include "../DataBaseOper/SQLBuilder.hpp"
#include "../DataBaseOper/AngKDataBaseOper.h"
#include "GlobalDefine.h"
#include "adddialog.h"
#include "cfgdialog.h"
#include <QStringListModel>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>

Q_DECLARE_METATYPE(chip);
Q_DECLARE_METATYPE(LoaclChipData::chip);

DataBaseOper::DataBaseOper(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataBaseOper),
    m_manufactureListModel(Q_NULLPTR),
    m_manufactureFilterTableModel(Q_NULLPTR),
    m_DBStandardItemModelEx(Q_NULLPTR),
    m_curSelectType(none)
{
    ui->setupUi(this);

    InitText();
    InitChipTable();
    InitListView();
    InitDB();
    InitButton();

    m_dlg = new AddDialog(this);
    connect(m_dlg, &AddDialog::sgnSelectData, this, &DataBaseOper::onSlotSelectData);
}

DataBaseOper::~DataBaseOper()
{
    delete ui;
}

void DataBaseOper::InitText()
{
    ui->openButton->setText(tr("open"));
    ui->addButton->setText(tr("add Data"));
    ui->modifyButton->setText(tr("Modify"));

    ui->chipNameText->setText(tr("chipName:"));
    ui->progText->setText(tr("Prog:"));
    ui->InsModeText->setText(tr("Ins Mode:"));
    ui->indexText->setText(tr("index:"));
    //ui->indexEdit->setReadOnly(true);

    ui->chipManuName->setText(tr("chipManu:"));
    ui->chipPackageName->setText(tr("chipPackage:"));
    ui->adapterName_1->setText(tr("adapterName:"));
    ui->chipTypeName->setText(tr("chipType:"));
    ui->drvFileName->setText(tr("drvFile:"));
    ui->fpgaFileName->setText(tr("fpgaFile:"));
    ui->extFileName->setText(tr("extFile:"));
    ui->masterFileName->setText(tr("masterFile:"));

    ui->fpgaFileName_2->setText(tr("fpgaFile_2:"));
    ui->adapterName_2->setText(tr("adapter_2:"));
    ui->adapterName_3->setText(tr("adapter_3:"));
    ui->chipHelpName->setText(tr("chipHelp:"));
    ui->bufferSizeName->setText(tr("bufferSize:"));
    ui->operAttriName->setText(tr("operAttri:"));
    ui->bufferInfoName->setText(tr("bufferInfo:"));
    ui->sectorSizeName->setText(tr("sectorSize:"));
    ui->chipIDName->setText(tr("chipID:"));
    ui->algoIC->setText(tr("Alg IC:"));
    ui->chipInfoName->setText(tr("chipInfo:"));
    ui->manuGroup->setTitle(tr("ManuList"));
    ui->chipTableGroup->setTitle(tr("chipTable"));

    ui->progComboBox->addItem("Single");
    ui->progComboBox->addItem("S2");
    ui->progComboBox->addItem("S4");
    ui->progComboBox->addItem("S8");
    ui->progComboBox->setCurrentIndex(-1);

    ui->InsModeComboBox->addItem("DMM");
    ui->InsModeComboBox->addItem("DIO");
    ui->InsModeComboBox->addItem("S8");
    ui->InsModeComboBox->addItem("eMMC");
    ui->InsModeComboBox->addItem("SPC1");
    ui->InsModeComboBox->addItem("SPC2");
    ui->InsModeComboBox->addItem("SPC3");
    ui->InsModeComboBox->setCurrentIndex(-1);

}

void DataBaseOper::InitChipTable()
{
    m_DBStandardItemModelEx = new AngKDBStandardItemModelEx();
    m_manufactureFilterTableModel = new AngKSortFilterProxyModel();
    m_manufactureFilterTableModel->setSourceModel(m_DBStandardItemModelEx);

    // 隐藏水平表头
    ui->manufactureTableView->verticalHeader()->setVisible(false);

    QStringList headList;
    m_varDataModelMap["headerlabel"] = headList << tr("index") << tr("Name") << tr("Manufact")  << tr("ChipType") << tr("Package") << tr("Adapter1")
                                                << tr("Adapter2") << tr("Adapter3")<< tr("drvFile")<< tr("fpgaFile1")<< tr("fpgaFile2")<< tr("appFile")
                                                << tr("spcFile") << tr("mstkoFile")<< tr("bufferSize")<< tr("ChipInfo")<< tr("bufferInfo")<< tr("status")
                                                << tr("operAttri") << tr("chipID")<< tr("drvParam")<< tr("SectorSize")<< tr("chipHelp")<< tr("progType")
                                                << tr("Ins Mode") << tr("AllChipInfo");

    m_DBStandardItemModelEx->SetData(m_varDataModelMap);
    //m_DBStandardItemModel->setHorizontalHeaderLabels(headList);
    ui->manufactureTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->manufactureTableView->setModel(m_manufactureFilterTableModel);
    ui->manufactureTableView->setAlternatingRowColors(true);
    ui->manufactureTableView->horizontalHeader()->setHighlightSections(false);
    ui->manufactureTableView->horizontalHeader()->setStretchLastSection(true);
    ui->manufactureTableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

    QHeaderView* manuHead = ui->manufactureTableView->horizontalHeader();

    // 列宽度自适应效果不好，自定宽度并可以拖动
    manuHead->setSectionResizeMode(QHeaderView::Interactive);
    ui->manufactureTableView->setColumnHidden(headList.count() - 1, true); // 隐藏最后一列

    connect(ui->manufactureTableView, &QTableView::doubleClicked, this, &DataBaseOper::onSlotClickTableView);
}

void DataBaseOper::InitListView()
{
    m_manufactureListModel = new QStringListModel(ui->manufactureListView);
    ui->manufactureListView->setModel(m_manufactureListModel);
    ui->manufactureListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(ui->manufactureListView, &QListView::clicked, this, &DataBaseOper::onSlotClickListView);
}

void DataBaseOper::ClearText()
{
    ui->chipNameEdit->clear();
    ui->progComboBox->clear();
    ui->InsModeComboBox->clear();
    ui->indexEdit->clear();

    ui->chipManuName->clear();
    ui->chipPackageName->clear();
    ui->adapterName_1->clear();
    ui->chipTypeName->clear();
    ui->drvFileName->clear();
    ui->fpgaFileName->clear();
    ui->extFileName->clear();
    ui->masterFileName->clear();

    ui->fpgaFileName_2->clear();
    ui->adapterName_2->clear();
    ui->adapterName_3->clear();
    ui->chipHelpName->clear();
    ui->bufferSizeName->clear();
    ui->bufferInfoName->clear();
    ui->sectorSizeName->clear();
    ui->operAttriName->clear();
    ui->chipIDName->clear();
    ui->algoIC->clear();
    ui->chipInfoName->clear();
}

void DataBaseOper::InitButton()
{
    connect(ui->chipManuButton, &QPushButton::clicked, this, &DataBaseOper::onSlotClickMoreFunc);
    connect(ui->chipPackageButton, &QPushButton::clicked, this, &DataBaseOper::onSlotClickMoreFunc);
    connect(ui->chipTypeButton, &QPushButton::clicked, this, &DataBaseOper::onSlotClickMoreFunc);
    connect(ui->drvFileButton, &QPushButton::clicked, this, &DataBaseOper::onSlotClickMoreFunc);
    connect(ui->fpgaFileButton, &QPushButton::clicked, this, &DataBaseOper::onSlotClickMoreFunc);
    connect(ui->fpgaFileButton_2, &QPushButton::clicked, this, &DataBaseOper::onSlotClickMoreFunc);
    connect(ui->extFileButton, &QPushButton::clicked, this, &DataBaseOper::onSlotClickMoreFunc);
    connect(ui->masterFileButton, &QPushButton::clicked, this, &DataBaseOper::onSlotClickMoreFunc);
    connect(ui->chipHelpButton, &QPushButton::clicked, this, &DataBaseOper::onSlotClickMoreFunc);
    connect(ui->adapterButton_1, &QPushButton::clicked, this, &DataBaseOper::onSlotClickMoreFunc);
    connect(ui->adapterButton_2, &QPushButton::clicked, this, &DataBaseOper::onSlotClickMoreFunc);
    connect(ui->adapterButton_3, &QPushButton::clicked, this, &DataBaseOper::onSlotClickMoreFunc);

    connect(ui->operAttriButton, &QPushButton::clicked, this, &DataBaseOper::onSlotClickCfgDialog);
}

void DataBaseOper::InitDB()
{
    connect(ui->comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DataBaseOper::onSlotSelectModel);
    connect(ui->openButton,&QPushButton::clicked, this, &DataBaseOper::onSlotOpenDBFile);
    connect(ui->addButton, &QPushButton::clicked, this, &DataBaseOper::onSlotAddDataBase);
    connect(ui->modifyButton, &QPushButton::clicked, this, &DataBaseOper::onSlotModifyDataBase);
    //型号从db还是哪里取？xml中取

}

void DataBaseOper::LoadChipData(std::vector<LoaclChipData::chip> vec)
{
    m_chipVec = vec;

    for (size_t i = 0; i < vec.size(); ++i)
    {
        QVariantMap map_1;
        map_1[tr("index")] = m_chipVec[i].id;
        map_1[tr("Name")] = QString::fromStdString(m_chipVec[i].strName);
        map_1[tr("Manufact")] = QString::fromStdString(m_chipVec[i].strManu);
        map_1[tr("ChipType")] = QString::fromStdString(m_chipVec[i].strType);
        map_1[tr("Package")] = QString::fromStdString(m_chipVec[i].strPack);
        map_1[tr("Adapter1")] = QString::fromStdString(m_chipVec[i].strAdapter);
        map_1[tr("Adapter2")] = QString::fromStdString(m_chipVec[i].strAdapter2);
        map_1[tr("Adapter3")] = QString::fromStdString(m_chipVec[i].strAdapter3);
        map_1[tr("drvFile")] = QString::fromStdString(m_chipVec[i].strAlgoFile);
        map_1[tr("fpgaFile1")] = QString::fromStdString(m_chipVec[i].strFPGAFile);
        map_1[tr("fpgaFile2")] = QString::fromStdString(m_chipVec[i].strFPGAFile2);
        map_1[tr("appFile")] = QString::fromStdString(m_chipVec[i].strAppFile);
        map_1[tr("spcFile")] = QString::fromStdString(m_chipVec[i].strSpcFile);
        map_1[tr("mstkoFile")] = QString::fromStdString(m_chipVec[i].strMstkoFile);
        map_1[tr("bufferSize")] = QString("%1").arg(m_chipVec[i].ulBufferSize, 8, 16, QLatin1Char('0'));//QString::number(m_chipVec[i].ulBufferSize, 16);
        map_1[tr("ChipInfo")] = QString::fromStdString(m_chipVec[i].strChipInfo);
        map_1[tr("bufferInfo")] = QString::fromStdString(m_chipVec[i].strBuffInfo);
        map_1[tr("status")] = "";
        map_1[tr("operAttri")] = QString::fromStdString(m_chipVec[i].strBuffInfo);//这个操作属性后续先按照json去存
        map_1[tr("chipID")] = QString("%1").arg(m_chipVec[i].ulChipId, 8, 16, QLatin1Char('0'));//QString::number(m_chipVec[i].ulChipId, 16);
        map_1[tr("drvParam")] = QString("%1").arg(m_chipVec[i].ulDrvParam, 8, 16, QLatin1Char('0'));//QString::number(m_chipVec[i].ulDrvParam, 16);
        map_1[tr("SectorSize")] = QString("%1").arg(m_chipVec[i].ulSectorSize, 8, 16, QLatin1Char('0'));//QString::number(m_chipVec[i].ulSectorSize, 16);
        map_1[tr("chipHelp")] = QString::fromStdString(m_chipVec[i].strHelpFile);
        map_1[tr("progType")] = getProgType(m_chipVec[i].nProgType);
        map_1[tr("Ins Mode")] = getInsModeType(m_chipVec[i].nProgType);
        map_1[tr("AllChipInfo")] = QVariant::fromValue(m_chipVec[i]);
        m_varDataModelMap[QString("%1").arg(i, 8, 10, QLatin1Char('0'))] = map_1;
    }

    m_DBStandardItemModelEx->SetData(m_varDataModelMap);
}

void DataBaseOper::LoadManufactureData(std::vector<manufacture> vec)
{
    ui->chipManuComboBox->clear();
    for (int i = 0; i < vec.size(); ++i)
    {
        m_manufactureListModel->insertRow(m_manufactureListModel->rowCount());
        m_manufactureListModel->setData(m_manufactureListModel->index(m_manufactureListModel->rowCount() - 1, 0), QString::fromStdString(vec[i].name));
        ui->chipManuComboBox->addItem(QString::fromStdString(vec[i].name));
    }
}

void DataBaseOper::LoadPackageData(std::vector<package> vec)
{
    ui->chipPackageComboBox->clear();
    for(int i = 0;i < vec.size(); ++i)
    {
        ui->chipPackageComboBox->addItem(QString::fromStdString(vec[i].name));
    }
}

void DataBaseOper::LoadAdapterData(std::vector<adapter> vec)
{
    ui->adapterComboBox_1->clear();
    ui->adapterComboBox_2->clear();
    ui->adapterComboBox_3->clear();
    for(int i = 0;i < vec.size(); ++i)
    {
        ui->adapterComboBox_1->addItem(QString::fromStdString(vec[i].name));
        ui->adapterComboBox_2->addItem(QString::fromStdString(vec[i].name));
        ui->adapterComboBox_3->addItem(QString::fromStdString(vec[i].name));
    }
}

void DataBaseOper::LoadChipTypeData(std::vector<chiptype> vec)
{
    ui->chipTypeComboBox->clear();
    for(int i = 0;i < vec.size(); ++i)
    {
        ui->chipTypeComboBox->addItem(QString::fromStdString(vec[i].name));
    }
}

void DataBaseOper::LoadDrvFileData(std::vector<algofile> vec)
{
    ui->drvFileComboBox->clear();
    for(int i = 0;i < vec.size(); ++i)
    {
        ui->drvFileComboBox->addItem(QString::fromStdString(vec[i].name));
    }
}

void DataBaseOper::LoadFpgaFileData(std::vector<fpgafile> vec)
{
    ui->fpgaFileComboBox->clear();
    ui->fpgaFileComboBox_2->clear();
    for(int i = 0;i < vec.size(); ++i)
    {
        ui->fpgaFileComboBox->addItem(QString::fromStdString(vec[i].name));
        ui->fpgaFileComboBox_2->addItem(QString::fromStdString(vec[i].name));
    }
}

void DataBaseOper::LoadExtFileData(std::vector<appfile> vec)
{
    ui->extFileComboBox->clear();
    for(int i = 0;i < vec.size(); ++i)
    {
        ui->extFileComboBox->addItem(QString::fromStdString(vec[i].name));
    }
}

void DataBaseOper::LoadMasterFileData(std::vector<mstkofile> vec)
{
    ui->masterFileComboBox->clear();
    for(int i = 0;i < vec.size(); ++i)
    {
        ui->masterFileComboBox->addItem(QString::fromStdString(vec[i].name));
    }
}

void DataBaseOper::LoadChipHelpFileData(std::vector<helpfile> vec)
{
    ui->chipHelpComboBox->clear();
    for(int i = 0;i < vec.size(); ++i)
    {
        ui->chipHelpComboBox->addItem(QString::fromStdString(vec[i].name));
    }
}

QString DataBaseOper::getProgType(int nProg)
{
    QString strItemValue;
    if ((nProg & 0x0F) == 0x01) {
        strItemValue = "S2";
    }
    else if ((nProg & 0x0F) == 0x02) {
        strItemValue = "S4";
    }
    else if ((nProg & 0x0F) == 0x03) {
        strItemValue = "S8";
    }
    else {
        strItemValue = "Single";
    }
    return strItemValue;
}

QString DataBaseOper::getInsModeType(int nProg)
{
    QString strItemValue;
    if (((nProg >> 24) & 0xFF) == 0x01) {
        strItemValue = "DIO";
    }
    else if (((nProg >> 24) & 0xFF) == 0x02) {
        strItemValue = "S8";
    }
    else if (((nProg >> 24) & 0xFF) == 0x03) {
        strItemValue = "eMMC";
    }
    else {
        strItemValue = "DMM";
    }
    return strItemValue;
}

void DataBaseOper::setDefaultCombobox()
{
    ui->chipManuComboBox->setCurrentIndex(-1);
    ui->chipPackageComboBox->setCurrentIndex(-1);
    ui->adapterComboBox_1->setCurrentIndex(-1);
    ui->adapterComboBox_2->setCurrentIndex(-1);
    ui->adapterComboBox_3->setCurrentIndex(-1);
    ui->chipTypeComboBox->setCurrentIndex(-1);
    ui->drvFileComboBox->setCurrentIndex(-1);
    ui->fpgaFileComboBox->setCurrentIndex(-1);
    ui->fpgaFileComboBox_2->setCurrentIndex(-1);
    ui->extFileComboBox->setCurrentIndex(-1);
    ui->masterFileComboBox->setCurrentIndex(-1);
    ui->chipHelpComboBox->setCurrentIndex(-1);
}

chip DataBaseOper::getCurrentChipData()
{
    chip chipData;
    bool bChange = false;
    chipData.id          = ui->indexEdit->text().toInt();
    chipData.name        = ui->chipNameEdit->text().toStdString();
    chipData.manuid      = Utils::AngKDataBaseOper::OperSelectOtherDataClassType<manufacture>(m_openFile, ui->chipManuComboBox->currentText(), "name =").id;
    chipData.adapterid   = Utils::AngKDataBaseOper::OperSelectOtherDataClassType<adapter>(m_openFile, ui->adapterComboBox_1->currentText(), "name =").id;
    chipData.adapter2id  = Utils::AngKDataBaseOper::OperSelectOtherDataClassType<adapter>(m_openFile, ui->adapterComboBox_2->currentText(), "name =").id;
    chipData.adapter3id  = Utils::AngKDataBaseOper::OperSelectOtherDataClassType<adapter>(m_openFile, ui->adapterComboBox_3->currentText(), "name =").id;
    chipData.adapter4id  = 0;
    chipData.packid      = Utils::AngKDataBaseOper::OperSelectOtherDataClassType<package>(m_openFile, ui->chipPackageComboBox->currentText(), "name =").id;
    chipData.typeId      = Utils::AngKDataBaseOper::OperSelectOtherDataClassType<chiptype>(m_openFile, ui->chipTypeComboBox->currentText(), "name =").id;
    chipData.algofileid  = Utils::AngKDataBaseOper::OperSelectOtherDataClassType<algofile>(m_openFile, ui->drvFileComboBox->currentText(), "name =").id;
    chipData.fpgafileid  = Utils::AngKDataBaseOper::OperSelectOtherDataClassType<fpgafile>(m_openFile, ui->fpgaFileComboBox->currentText(), "name =").id;
    chipData.fpgafile2id = Utils::AngKDataBaseOper::OperSelectOtherDataClassType<fpgafile>(m_openFile, ui->fpgaFileComboBox_2->currentText(), "name =").id;
    chipData.appfileid   = Utils::AngKDataBaseOper::OperSelectOtherDataClassType<appfile>(m_openFile, ui->extFileComboBox->currentText(), "name =").id;
    chipData.mstkfileid  = Utils::AngKDataBaseOper::OperSelectOtherDataClassType<mstkofile>(m_openFile, ui->masterFileComboBox->currentText(), "name =").id;
    chipData.helpfileid  = Utils::AngKDataBaseOper::OperSelectOtherDataClassType<helpfile>(m_openFile, ui->chipHelpComboBox->currentText(), "name =").id;
    chipData.buffsize    = ui->bufferSizeEdit->text().toULong(&bChange, 16);
    chipData.chipinfo    = ui->chipInfoEdit->text().toStdString();
    chipData.buffinfo    = ui->bufferInfoEdit->text().toStdString();
    chipData.debug       = 0;
    chipData.opcfgmask   = 0;
    chipData.version     = 0;
    chipData.modifyinfo  = "";
    chipData.devid       = ui->chipIDEdit->text().toULong(&bChange, 16);
    chipData.drvparam    = ui->algoICEdit->text().toULong(&bChange, 16);
    chipData.spcfileid   = NULL;
    chipData.designerid  = NULL;
    chipData.history     = "";
    chipData.sectorsize  = ui->sectorSizeEdit->text().toULong(&bChange, 16);
    chipData.progtype    = 0;
    chipData.opcfgattr   = ui->operAttriEdit->text().toStdString();

    return chipData;
}

void DataBaseOper::addDialogOperDB(int operType, int tableType, QString dValue)
{

}

void DataBaseOper::onSlotSelectModel(int nIndex)
{

}

void DataBaseOper::onSlotOpenDBFile()
{
    m_openFile = QFileDialog::getOpenFileName(this, "Select File...", QCoreApplication::applicationDirPath(), tr("db Files(*.db);; dbs Files(*.dbs)"));
    emit sgnSetDBFile(m_openFile);

    //加载芯片数据
    LoadChipData(Utils::AngKDataBaseOper::OperSelectChipData(m_openFile));
    //加载厂商列表
    LoadManufactureData(Utils::AngKDataBaseOper::OperSelectChipDataClassType<manufacture>(m_openFile));
    //加载封装列表
    LoadPackageData(Utils::AngKDataBaseOper::OperSelectChipDataClassType<package>(m_openFile));
    //加载封装列表
    LoadAdapterData(Utils::AngKDataBaseOper::OperSelectChipDataClassType<adapter>(m_openFile));
    //加载芯片类型列表
    LoadChipTypeData(Utils::AngKDataBaseOper::OperSelectChipDataClassType<chiptype>(m_openFile));
    //加载drvFile列表
    LoadDrvFileData(Utils::AngKDataBaseOper::OperSelectChipDataClassType<algofile>(m_openFile));
    //加载FpgaFile列表
    LoadFpgaFileData(Utils::AngKDataBaseOper::OperSelectChipDataClassType<fpgafile>(m_openFile));
    //加载EXT列表
    LoadExtFileData(Utils::AngKDataBaseOper::OperSelectChipDataClassType<appfile>(m_openFile));
    //加载masterKoFile列表
    LoadMasterFileData(Utils::AngKDataBaseOper::OperSelectChipDataClassType<mstkofile>(m_openFile));
    //加载芯片帮助列表
    LoadChipHelpFileData(Utils::AngKDataBaseOper::OperSelectChipDataClassType<helpfile>(m_openFile));

    //加载完成之后都不需要默认显示
    setDefaultCombobox();

}

void DataBaseOper::onSlotClickListView(const QModelIndex & modelIdx)
{
    m_manufactureFilterTableModel->setSearchStr(m_manufactureListModel->data(modelIdx).toString());
    m_manufactureFilterTableModel->invalidate();
    m_manufactureFilterTableModel->setFilterKeyColumn(ChipManufact);

    ui->manufactureTableView->reset();
}

void DataBaseOper::onSlotClickTableView(const QModelIndex &index)
{
    LoaclChipData::chip chipValue = m_manufactureFilterTableModel->data(m_manufactureFilterTableModel->index(index.row(), m_manufactureFilterTableModel->columnCount() - 1)).value<LoaclChipData::chip>();

    ui->chipNameEdit->setText(QString::fromStdString(chipValue.strName));
    ui->indexEdit->setText(QString::number(chipValue.id));
    ui->progComboBox->setCurrentText(getProgType(chipValue.nProgType));
    ui->InsModeComboBox->setCurrentText(getInsModeType(chipValue.nProgType));

    ui->bufferSizeEdit->setText(QString("%1").arg(chipValue.ulBufferSize, 8, 16, QLatin1Char('0')));
    ui->bufferInfoEdit->setText(QString::fromStdString(chipValue.strBuffInfo));
    ui->sectorSizeEdit->setText(QString("%1").arg(chipValue.ulSectorSize, 8, 16, QLatin1Char('0')));
    ui->chipManuComboBox->setCurrentText(QString::fromStdString(chipValue.strManu));
    ui->chipPackageComboBox->setCurrentText(QString::fromStdString(chipValue.strPack));
    ui->adapterComboBox_1->setCurrentText(QString::fromStdString(chipValue.strAdapter));
    ui->adapterComboBox_2->setCurrentText(QString::fromStdString(chipValue.strAdapter2));
    ui->adapterComboBox_3->setCurrentText(QString::fromStdString(chipValue.strAdapter3));
    ui->chipTypeComboBox->setCurrentText(QString::fromStdString(chipValue.strType));
    ui->drvFileComboBox->setCurrentText(QString::fromStdString(chipValue.strAlgoFile));
    ui->fpgaFileComboBox->setCurrentText(QString::fromStdString(chipValue.strFPGAFile));
    ui->fpgaFileComboBox_2->setCurrentText(QString::fromStdString(chipValue.strFPGAFile2));
    ui->extFileComboBox->setCurrentText(QString::fromStdString(chipValue.strAppFile));
    ui->masterFileComboBox->setCurrentText(QString::fromStdString(chipValue.strMstkoFile));
    ui->chipHelpComboBox->setCurrentText(QString::fromStdString(chipValue.strHelpFile));
    ui->chipIDEdit->setText(QString("%1").arg(chipValue.ulChipId, 8, 16, QLatin1Char('0')));
    ui->algoICEdit->setText(QString("%1").arg(chipValue.ulDrvParam, 8, 16, QLatin1Char('0')));
    ui->chipInfoEdit->setText(QString::fromStdString(chipValue.strChipInfo));

    //qDebug() << Base64ToQStr(chipValue.strOperCfgJson);
    ui->operAttriEdit->setText(chipValue.strOperCfgJson);
}

void DataBaseOper::onSlotAddDataBase()
{
    if(QMessageBox::warning(this, tr("Warning"), tr("Are you sure you want to add data ?"), QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok)
    {
        chip chipValue = getCurrentChipData();
        int ret = Utils::AngKDataBaseOper::OperInsertChipData(m_openFile, chipValue);
        qDebug() << ret;
    }

    RefreshTable();
}

void DataBaseOper::onSlotModifyDataBase()
{
    if(QMessageBox::warning(this, tr("Warning"), tr("Are you sure you want to modify data ?"), QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok)
    {
        chip chipValue = getCurrentChipData();
        int ret = Utils::AngKDataBaseOper::OperUpdateTableData(m_openFile, chipValue);
        qDebug() << ret;
    }

    RefreshTable();
}

void DataBaseOper::onSlotClickMoreFunc()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if(ui->chipManuButton == btn)
    {
        m_curSelectType = manuButton;
        for(int i = 0;i < ui->chipManuComboBox->count(); ++i)
        {
            m_dlg->addData(ui->chipManuComboBox->itemText(i));
        }

        m_dlg->setTitle(tr("ChipManuTitle"), false);
        m_dlg->setGroupText(tr("ManuInfo"), tr("ManuTable"));
        connect(m_dlg, &AddDialog::sgnOperDB, this, [=](int operType, QString dValue, QString origValue){

            switchOperDB<manufacture>(operType, dValue, origValue);
            LoadManufactureData(Utils::AngKDataBaseOper::OperSelectChipDataClassType<manufacture>(m_openFile));
            m_dlg->clearTable();
            for(int i = 0;i < ui->chipManuComboBox->count(); ++i)
            {
                m_dlg->addData(ui->chipManuComboBox->itemText(i));
            }
        });


    }
    else if(ui->chipPackageButton == btn)
    {
        m_curSelectType = packageButton;
        for(int i = 0;i < ui->chipPackageComboBox->count(); ++i)
        {
            m_dlg->addData(ui->chipPackageComboBox->itemText(i));
        }

        m_dlg->setTitle(tr("ChipPackageTitle"), false);
        m_dlg->setGroupText(tr("PackageInfo"), tr("PackageTable"));
        connect(m_dlg, &AddDialog::sgnOperDB, this, [=](int operType, QString dValue, QString origValue){

            switchOperDB<package>(operType, dValue, origValue);
            LoadPackageData(Utils::AngKDataBaseOper::OperSelectChipDataClassType<package>(m_openFile));
            m_dlg->clearTable();
            for(int i = 0;i < ui->chipPackageComboBox->count(); ++i)
            {
                m_dlg->addData(ui->chipPackageComboBox->itemText(i));
            }
        });
    }
    else if(ui->chipTypeButton == btn)
    {
        m_curSelectType = typeButton;
        for(int i = 0;i < ui->chipTypeComboBox->count(); ++i)
        {
            m_dlg->addData(ui->chipTypeComboBox->itemText(i));
        }

        m_dlg->setTitle(tr("ChipTypeTitle"), false);
        m_dlg->setGroupText(tr("TypeInfo"), tr("TypeTable"));
        connect(m_dlg, &AddDialog::sgnOperDB, this, [=](int operType, QString dValue, QString origValue){

            switchOperDB<chiptype>(operType, dValue, origValue);
            LoadChipTypeData(Utils::AngKDataBaseOper::OperSelectChipDataClassType<chiptype>(m_openFile));
            m_dlg->clearTable();
            for(int i = 0;i < ui->chipTypeComboBox->count(); ++i)
            {
                m_dlg->addData(ui->chipTypeComboBox->itemText(i));
            }
        });
    }
    else if(ui->drvFileButton == btn)
    {
        m_curSelectType = drvFileButton;
        for(int i = 0;i < ui->drvFileComboBox->count(); ++i)
        {
            m_dlg->addData(ui->drvFileComboBox->itemText(i));
        }

        m_dlg->setTitle(tr("drvFileTitle"), false);
        m_dlg->setGroupText(tr("drvFileInfo"), tr("drvFileTable"));
        connect(m_dlg, &AddDialog::sgnOperDB, this, [=](int operType, QString dValue, QString origValue){

            switchOperDB<algofile>(operType, dValue, origValue);
            LoadDrvFileData(Utils::AngKDataBaseOper::OperSelectChipDataClassType<algofile>(m_openFile));
            m_dlg->clearTable();
            for(int i = 0;i < ui->drvFileComboBox->count(); ++i)
            {
                m_dlg->addData(ui->drvFileComboBox->itemText(i));
            }
        });
    }
    else if(ui->fpgaFileButton == btn || ui->fpgaFileButton_2 == btn)
    {
        if(ui->fpgaFileButton == btn)
            m_curSelectType = fpgaFileButton;
        else
            m_curSelectType = fpgeFileButton2;

        for(int i = 0;i < ui->fpgaFileComboBox->count(); ++i)
        {
            m_dlg->addData(ui->fpgaFileComboBox->itemText(i));
        }

        m_dlg->setTitle(tr("fpgaFileTitle"), false);
        m_dlg->setGroupText(tr("fpgaFileInfo"), tr("fpgaFileTable"));
        connect(m_dlg, &AddDialog::sgnOperDB, this, [=](int operType, QString dValue, QString origValue){

            switchOperDB<fpgafile>(operType, dValue, origValue);
            LoadFpgaFileData(Utils::AngKDataBaseOper::OperSelectChipDataClassType<fpgafile>(m_openFile));
            m_dlg->clearTable();
            for(int i = 0;i < ui->fpgaFileComboBox->count(); ++i)
            {
                m_dlg->addData(ui->fpgaFileComboBox->itemText(i));
            }
        });
    }
    else if(ui->extFileButton == btn)
    {
        m_curSelectType = extFileButton;
        for(int i = 0;i < ui->extFileComboBox->count(); ++i)
        {
            m_dlg->addData(ui->extFileComboBox->itemText(i));
        }

        m_dlg->setTitle(tr("extFileTitle"), false);
        m_dlg->setGroupText(tr("extFileInfo"), tr("extFileTable"));
        connect(m_dlg, &AddDialog::sgnOperDB, this, [=](int operType, QString dValue, QString origValue){

            switchOperDB<appfile>(operType, dValue, origValue);
            LoadExtFileData(Utils::AngKDataBaseOper::OperSelectChipDataClassType<appfile>(m_openFile));
            m_dlg->clearTable();
            for(int i = 0;i < ui->extFileComboBox->count(); ++i)
            {
                m_dlg->addData(ui->extFileComboBox->itemText(i));
            }
        });
    }
    else if(ui->masterFileButton == btn)
    {
        m_curSelectType = masterFileButton;
        for(int i = 0;i < ui->masterFileComboBox->count(); ++i)
        {
            m_dlg->addData(ui->masterFileComboBox->itemText(i));
        }

        m_dlg->setTitle(tr("masterFileTitle"), false);
        m_dlg->setGroupText(tr("masterFileInfo"), tr("masterFileTable"));
        connect(m_dlg, &AddDialog::sgnOperDB, this, [=](int operType, QString dValue, QString origValue){

            switchOperDB<mstkofile>(operType, dValue, origValue);
            LoadMasterFileData(Utils::AngKDataBaseOper::OperSelectChipDataClassType<mstkofile>(m_openFile));
            m_dlg->clearTable();
            for(int i = 0;i < ui->masterFileComboBox->count(); ++i)
            {
                m_dlg->addData(ui->masterFileComboBox->itemText(i));
            }
        });
    }
    else if(ui->chipHelpButton == btn)
    {
        m_curSelectType = helpFileButton;
        for(int i = 0;i < ui->chipHelpComboBox->count(); ++i)
        {
            m_dlg->addData(ui->chipHelpComboBox->itemText(i));
        }

        m_dlg->setTitle(tr("chipHelpTitle"), false);
        m_dlg->setGroupText(tr("chipHelpInfo"), tr("chipHelpTable"));
        connect(m_dlg, &AddDialog::sgnOperDB, this, [=](int operType, QString dValue, QString origValue){

            switchOperDB<helpfile>(operType, dValue, origValue);
            LoadChipHelpFileData(Utils::AngKDataBaseOper::OperSelectChipDataClassType<helpfile>(m_openFile));
            m_dlg->clearTable();
            for(int i = 0;i < ui->chipHelpComboBox->count(); ++i)
            {
                m_dlg->addData(ui->chipHelpComboBox->itemText(i));
            }
        });
    }
    else if(ui->adapterButton_1 == btn || ui->adapterButton_2 == btn || ui->adapterButton_3 == btn)
    {
        if(ui->adapterButton_1 == btn)
            m_curSelectType = adapterButton1;
        else if(ui->adapterButton_2 == btn)
            m_curSelectType = adapterButton2;
        else if(ui->adapterButton_3 == btn)
            m_curSelectType = adapterButton3;

        std::vector<adapter> adapterVec = Utils::AngKDataBaseOper::OperSelectChipDataClassType<adapter>(m_openFile);
        for(int i = 0;i < adapterVec.size(); ++i)
        {
            m_dlg->addAdapterData(QString::fromStdString(adapterVec[i].name), QString("%1").arg(adapterVec[i].chipid, 4, 16, QLatin1Char('0')) );
        }

        m_dlg->setTitle(tr("adapterManagerTitle"), true);
        m_dlg->setGroupText(tr("adapterInfo"), tr("adapterTable"));
        connect(m_dlg, &AddDialog::sgnAdapterOperDB, this, [=](int operType, QString dValue, QString IDValue, QString origValue){

            switchOperDB2Adapter(operType, dValue, IDValue, origValue);

            LoadAdapterData(Utils::AngKDataBaseOper::OperSelectChipDataClassType<adapter>(m_openFile));
            m_dlg->clearTable();

            std::vector<adapter> adapterVec = Utils::AngKDataBaseOper::OperSelectChipDataClassType<adapter>(m_openFile);
            for(int i = 0;i < adapterVec.size(); ++i)
            {
                m_dlg->addAdapterData(QString::fromStdString(adapterVec[i].name), QString("%1").arg(adapterVec[i].chipid, 4, 16, QLatin1Char('0')) );
            }
        });
    }

    m_dlg->exec();
    m_dlg->clearTable();
    //setDefaultCombobox();
}

void DataBaseOper::onSlotSelectData(QString strData)
{
    switch (m_curSelectType)
    {
    case manuButton:
        ui->chipManuComboBox->setCurrentText(strData);
        break;
    case packageButton:
        ui->chipPackageComboBox->setCurrentText(strData);
        break;
    case typeButton:
        ui->chipTypeComboBox->setCurrentText(strData);
        break;
    case drvFileButton:
        ui->drvFileComboBox->setCurrentText(strData);
        break;
    case fpgaFileButton:
        ui->fpgaFileComboBox->setCurrentText(strData);
        break;
    case fpgeFileButton2:
        ui->fpgaFileComboBox_2->setCurrentText(strData);
        break;
    case extFileButton:
        ui->extFileComboBox->setCurrentText(strData);
        break;
    case masterFileButton:
        ui->masterFileComboBox->setCurrentText(strData);
        break;
    case helpFileButton:
        ui->chipHelpComboBox->setCurrentText(strData);
        break;
    case adapterButton1:
        ui->adapterComboBox_1->setCurrentText(strData);
        break;
    case adapterButton2:
        ui->adapterComboBox_2->setCurrentText(strData);
        break;
    case adapterButton3:
        ui->adapterComboBox_3->setCurrentText(strData);
        break;
    default:
        break;
    }

    m_curSelectType = none;
}

void DataBaseOper::onSlotClickCfgDialog()
{
    cfgDialog cfgDlg(this);
    //cfgDlg.setOperAttr(Base64ToQStr(ui->operAttriEdit->text()));
    cfgDlg.setOperAttr(ui->operAttriEdit->text());
    connect(&cfgDlg, &cfgDialog::sgnSaveOperAttri, this, [=](QString strJson){
        qDebug() << strJson;
        //ui->operAttriEdit->setText(QStrToBase64(strJson));
        ui->operAttriEdit->setText(strJson);
    });
    cfgDlg.exec();
}

template<typename T>
void DataBaseOper::switchOperDB(int operType, QString dValue, QString origValue)
{
    T value;
    value.name = dValue.toStdString();
    switch(operType)
    {
    case operDBType::Add://insert
    {
        int ret = Utils::AngKDataBaseOper::OperInsertTableData(m_openFile, value);
        qDebug() << ret;
    }
    break;
    case operDBType::Save://update
    {
        value.id = Utils::AngKDataBaseOper::OperSelectOtherDataClassType<T>(m_openFile, origValue, "name =").id;
        int ret = Utils::AngKDataBaseOper::OperUpdateTableData(m_openFile, value);
        qDebug() << ret;
    }
    break;
    case operDBType::Delete://delete
    {
        value.id = Utils::AngKDataBaseOper::OperSelectOtherDataClassType<T>(m_openFile, origValue, "name =").id;
        int ret = Utils::AngKDataBaseOper::OperDeleteTableData<T>(m_openFile, value.id);
        qDebug() << ret;
    }
    break;
    }
}


void DataBaseOper::switchOperDB2Adapter(int operType, QString dValue, QString IDValue, QString origValue)
{
    adapter value;
    bool bOk;
    value.name = dValue.toStdString();
    value.chipid = IDValue.toULong(&bOk, 16);
    switch(operType)
    {
    case operDBType::Add://insert
    {
        int ret = Utils::AngKDataBaseOper::OperInsertTableData(m_openFile, value);
        qDebug() << ret;
    }
    break;
    case operDBType::Save://update
    {
        value.id = Utils::AngKDataBaseOper::OperSelectOtherDataClassType<adapter>(m_openFile, origValue, "name =").id;
        int ret = Utils::AngKDataBaseOper::OperUpdateTableData(m_openFile, value);
        qDebug() << ret;
    }
    break;
    case operDBType::Delete://delete
    {
        value.id = Utils::AngKDataBaseOper::OperSelectOtherDataClassType<adapter>(m_openFile, origValue, "name =").id;
        int ret = Utils::AngKDataBaseOper::OperDeleteTableData<adapter>(m_openFile, value.id);
        qDebug() << ret;
    }
    break;
    }
}

QString DataBaseOper::QStrToBase64(QString str)
{
    QByteArray byteA;
    byteA=str.toUtf8();
    byteA=byteA.toBase64();
    char * cbyteA=byteA.data();

    return QString(cbyteA);
}

QString DataBaseOper::Base64ToQStr(QString base64Str)
{
    QByteArray byteA;
    std::string stdStr = base64Str.toStdString();
    byteA=QByteArray(stdStr.c_str() );
    byteA=byteA.fromBase64(byteA);

    return  QString::fromUtf8(byteA);
}

void DataBaseOper::RefreshTable()
{
    m_DBStandardItemModelEx->removeRows(0, m_DBStandardItemModelEx->rowCount(m_DBStandardItemModelEx->index(0,0)));
    ui->manufactureTableView->reset();
    //加载芯片数据
    LoadChipData(Utils::AngKDataBaseOper::OperSelectChipData(m_openFile));
}
