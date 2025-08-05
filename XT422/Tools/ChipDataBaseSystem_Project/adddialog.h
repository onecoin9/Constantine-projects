#ifndef ADDDIALOG_H
#define ADDDIALOG_H

#include <QDialog>
#include <QStandardItemModel>

enum operDBType
{
    Add = 1,
    Save = 2,
    Delete = 3
};


namespace Ui {
class AddDialog;
}

class AddDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddDialog(QWidget *parent = nullptr);
    ~AddDialog();

    void InitText();

    void InitTable();

    void InitButton();

    void setGroupText(QString infoText, QString tableText);

    void setTitle(QString strTitle, bool bShowTable = false);

    void addData(QString dataValue);
    void addAdapterData(QString dataValue, QString idValue);

    void clearTable();

    void tableReset();
signals:
    void sgnOperDB(int, QString, QString);
    void sgnSelectData(QString);
    void sgnAdapterOperDB(int, QString, QString, QString);
public slots:
    void onSlotClickTableView(const QModelIndex &index);
    void onSlotClickOperDataBase();
    void onSlotClickAdapterTableView(const QModelIndex &index);
private:
    Ui::AddDialog *ui;
    QStandardItemModel*	m_pDataTableModel;
    QStandardItemModel*	m_pAdapterDataTableModel;
    QString m_originalValue;
    bool    m_bIsAdapter;
};

#endif // ADDDIALOG_H
