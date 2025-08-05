#pragma once
#ifndef DATABASEOPER_H
#define DATABASEOPER_H

#include <QWidget>
#include <QMap>
//#include "GlobalDefine.h"
#include "../DataBaseOper/ChipModel.h"
#include "../DataBaseOper/ChipDBModel.h"

enum selectBtnType
{
    none = 0,
    manuButton,
    packageButton,
    typeButton,
    drvFileButton,
    fpgaFileButton,
    fpgeFileButton2,
    extFileButton,
    masterFileButton,
    helpFileButton,
    adapterButton1,
    adapterButton2,
    adapterButton3
};

namespace Ui {
class DataBaseOper;
}

class AngKSortFilterProxyModel;
class QStringListModel;
class AngKDBStandardItemModelEx;
class AddDialog;
class DataBaseOper : public QWidget
{
    Q_OBJECT

public:
    explicit DataBaseOper(QWidget *parent = nullptr);
    ~DataBaseOper();

    void InitText();

    void InitChipTable();

    void InitListView();

    void ClearText();

    void InitButton();
private:
    void InitDB();
    void LoadChipData(std::vector<LoaclChipData::chip> vec);
    void LoadManufactureData(std::vector<manufacture> vec);
    void LoadPackageData(std::vector<package> vec);
    void LoadAdapterData(std::vector<adapter> vec);
    void LoadChipTypeData(std::vector<chiptype> vec);
    void LoadDrvFileData(std::vector<algofile> vec);
    void LoadFpgaFileData(std::vector<fpgafile> vec);
    void LoadExtFileData(std::vector<appfile> vec);
    void LoadMasterFileData(std::vector<mstkofile> vec);
    void LoadChipHelpFileData(std::vector<helpfile> vec);
    QString getProgType(int nProg);
    QString getInsModeType(int nProg);
    void setDefaultCombobox();
    chip getCurrentChipData();
    void addDialogOperDB(int operType, int tableType, QString dValue);

    template<typename T>
    void switchOperDB(int operType, QString dValue, QString origValue);
    void switchOperDB2Adapter(int operType, QString dValue, QString IDValue, QString origValue);

    QString QStrToBase64(QString str);
    QString Base64ToQStr(QString base64Str);

    void RefreshTable();
signals:
    void sgnSetDBFile(QString);

public slots:
    void onSlotSelectModel(int);
    void onSlotOpenDBFile();
    void onSlotClickListView(const QModelIndex&);
    void onSlotClickTableView(const QModelIndex &index);
    void onSlotAddDataBase();
    void onSlotModifyDataBase();
    void onSlotClickMoreFunc();
    void onSlotSelectData(QString);
    void onSlotClickCfgDialog();
private:
    Ui::DataBaseOper *ui;
    QStringListModel*			m_manufactureListModel;
    AngKSortFilterProxyModel*	m_manufactureFilterTableModel;
    AngKDBStandardItemModelEx*	m_DBStandardItemModelEx;
    std::vector<LoaclChipData::chip>			m_chipVec;
    QVariantMap					m_varDataModelMap;
    QString                     m_openFile;
    AddDialog*                  m_dlg;
    selectBtnType               m_curSelectType;
};

#endif // DATABASEOPER_H
