#ifndef CFGDIALOG_H
#define CFGDIALOG_H

#include <QDialog>

namespace Ui {
class cfgDialog;
}

class cfgDialog : public QDialog
{
    Q_OBJECT

public:
    explicit cfgDialog(QWidget *parent = nullptr);
    ~cfgDialog();

    void InitText();
    void InitButton();
    void setOperAttr(QString attrJson);
signals:
    void sgnSaveOperAttri(QString);
private:
    Ui::cfgDialog *ui;
};

#endif // CFGDIALOG_H
