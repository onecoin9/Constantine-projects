#ifndef USERMANAGEMENT_H
#define USERMANAGEMENT_H
#include "ui_userManagement.h"
#include "roleAddDialog.h"
#include <QDialog>
#include <QTableView>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardItem>

namespace Ui {
    class UserManagementDialog;
}

class UserManagementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserManagementDialog(QWidget* parent = nullptr);
    ~UserManagementDialog();

private:
    Ui::UserManagementDialog* ui;

    QTableView* tableViewUsers;
    QLineEdit* lineEditOldPassword;
    QLineEdit* lineEditNewPassword;
    QLineEdit* lineEditConfirmPassword;
    QPushButton* pushButtonChangePassword;
    QPushButton* pushButtonAdd;
    QPushButton* pushButtonEnable;
    QPushButton* pushButtonDisable;
    QPushButton* pushButtonDelete;

private slots:
    void onChangePasswordClicked();
    void onAddUserClicked();
    void onEnableUserClicked();
    void onDisableUserClicked();
    void onDeleteUserClicked();
private:
    void setupTableView();
    void onResetPassword();
    void onDeleteUser();
};

#endif // USERMANAGEMENT_H