#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QFormLayout>
#include <QPushButton>
#include <QSettings>

class InputDialog : public QDialog {
    Q_OBJECT

public:
    explicit InputDialog(QWidget* parent = nullptr);

private slots:
    void onSave();

private:
    QLineEdit* customerCodeEdit;
    QLineEdit* productModelEdit;
    QLineEdit* weeklyEdit;
    QLineEdit* customerBatchEdit;
    QLineEdit* workOrderEdit;
    QLineEdit* workStepEdit;
    QLineEdit* processEdit;
    QLineEdit* deviceNumberEdit;
};

#endif // INPUTDIALOG_H