#include "userManagement.h"
#include "buttonDelegate.h"
#include <QMessageBox>
#include <QSettings>
    
UserManagementDialog::UserManagementDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::UserManagementDialog)
{
    ui->setupUi(this);

    tableViewUsers = ui->tableViewUsers;
    lineEditOldPassword = ui->lineEditOldPassword;
    lineEditNewPassword = ui->lineEditNewPassword;
    lineEditConfirmPassword = ui->lineEditConfirmPassword;
    pushButtonChangePassword = ui->pushButtonChangePassword;
    pushButtonAdd = ui->pushButtonAdd;
    pushButtonEnable = ui->pushButtonEnable;
    pushButtonDisable = ui->pushButtonDisable;
    pushButtonDelete = ui->pushButtonDelete;

    connect(pushButtonChangePassword, &QPushButton::clicked, this, &UserManagementDialog::onChangePasswordClicked);
    connect(pushButtonAdd, &QPushButton::clicked, this, &UserManagementDialog::onAddUserClicked);
    connect(pushButtonEnable, &QPushButton::clicked, this, &UserManagementDialog::onEnableUserClicked);
    connect(pushButtonDisable, &QPushButton::clicked, this, &UserManagementDialog::onDisableUserClicked);
    connect(pushButtonDelete, &QPushButton::clicked, this, &UserManagementDialog::onDeleteUserClicked);
    setupTableView();

}

UserManagementDialog::~UserManagementDialog()
{
   
}
void UserManagementDialog::onChangePasswordClicked()
{
    // 更改密码逻辑
}

void UserManagementDialog::onAddUserClicked()
{
    RoleAddDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // 处理确认后的逻辑
    }
}

void UserManagementDialog::onEnableUserClicked()
{
    // 启用用户逻辑
}

void UserManagementDialog::onDisableUserClicked()
{
    // 禁用用户逻辑
}

void UserManagementDialog::onDeleteUserClicked()
{
    // 删除用户逻辑
}

void UserManagementDialog::setupTableView()
{
    QStandardItemModel* model = new QStandardItemModel(this);
    model->setColumnCount(7);
    model->setHorizontalHeaderLabels({ "", "ID", "用户名", "真实姓名", "角色", "账号状态", "操作" });

    for (int i = 0; i < 10; ++i) {
        QList<QStandardItem*> rowItems;

        QStandardItem* checkItem = new QStandardItem();
        checkItem->setCheckable(true);
        rowItems.append(checkItem);

        rowItems.append(new QStandardItem(QString::number(i + 1)));
        rowItems.append(new QStandardItem(QString("user%1").arg(i + 1)));
        rowItems.append(new QStandardItem(QString("真实姓名%1").arg(i + 1)));
        rowItems.append(new QStandardItem("角色"));
        rowItems.append(new QStandardItem("正常"));

        model->appendRow(rowItems);
    }

    tableViewUsers->setModel(model);
    tableViewUsers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ButtonDelegate* delegate = new ButtonDelegate(this);
    tableViewUsers->setItemDelegateForColumn(6, delegate);
    for (int i = 0; i < 5; ++i) {
        tableViewUsers->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
    tableViewUsers->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(delegate, &ButtonDelegate::resetPasswordClicked, this, &UserManagementDialog::onResetPassword);
    connect(delegate, &ButtonDelegate::deleteUserClicked, this, &UserManagementDialog::onDeleteUser);

}

void UserManagementDialog::onResetPassword()
{

}

void UserManagementDialog::onDeleteUser()
{

}