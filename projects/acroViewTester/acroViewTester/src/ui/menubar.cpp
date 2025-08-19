#include"acroViewTester.h"
#include "setting.h"
#include "userManagement.h"
#include "login.h"

void acroViewTester::handleMenuTrigger()
{
    if (!settingDialog_ui)
    {
        settingDialog_ui = new settingDialog(this);
    }
    
    settingDialog_ui->show();
}

void acroViewTester::settingTrigger()
{
    if (!settingDialog_ui) {
        settingDialog_ui = new settingDialog(this);
        connect(settingDialog_ui, &settingDialog::settingsChanged,
            this, &acroViewTester::onSettingsChanged);
        settingDialog_ui->setAttribute(Qt::WA_DeleteOnClose);
        connect(settingDialog_ui, &QDialog::finished, [this]() {
            settingDialog_ui = nullptr;
            });
    }
    settingDialog_ui->show();
    settingDialog_ui->raise();
    settingDialog_ui->activateWindow();
}

void acroViewTester::setupMenuBar()
{
    menuBar()->clear();
    if (directMenu) {
        delete directMenu;
        directMenu = nullptr;
    }

    // 设置菜单
    directMenu = new DirectTriggerMenu(tr("设置"), this);
    menuBar()->addMenu(directMenu);
    connect(directMenu, &DirectTriggerMenu::menuTriggered, this, &acroViewTester::handleMenuTrigger, Qt::DirectConnection);

    //viewMenu = menuBar()->addMenu(tr("视图"));
    createViewMenu();
    // 用户管理菜单
    QMenu* userMenu = menuBar()->addMenu(tr("用户管理"));
    QAction* currentUserAction = userMenu->addAction(tr("当前用户"));
    QAction* userManagementAction = userMenu->addAction(tr("用户管理"));

    connect(currentUserAction, &QAction::triggered, this, [this]() {
        Login* loginDialog = new Login(this);
        loginDialog->exec();
        delete loginDialog;
        });
    connect(ui.pushButtonUser, &QPushButton::clicked, this, [this]() {
        Login* loginDialog = new Login(this);
        loginDialog->exec();
        delete loginDialog;
        });

    connect(userManagementAction, &QAction::triggered, this, [this]() {
        UserManagementDialog* dialog = new UserManagementDialog(this);
        dialog->exec();
        delete dialog;
        });

    // 语言菜单
    QMenu* langMenu = menuBar()->addMenu(tr("语言"));
    QActionGroup* langGroup = new QActionGroup(this);
    QAction* zhAction = new QAction(tr("中文"), langGroup);
    QAction* enAction = new QAction(tr("English"), langGroup);
    zhAction->setCheckable(true);
    enAction->setCheckable(true);
    zhAction->setChecked(true);
    langMenu->addAction(zhAction);
    langMenu->addAction(enAction);

    // 关于菜单
    QMenu* aboutMenu = menuBar()->addMenu(tr("关于"));
    aboutMenu->addAction(tr("使用文档"));
    aboutMenu->addAction(tr("关于我们"));

    // 关于菜单的信号槽连接
    connect(aboutMenu, &QMenu::triggered, this, [this](QAction* action) {
        if (action->text() == tr("使用文档")) {
            // TODO: 添加使用文档处理逻辑
        }
        else if (action->text() == tr("关于我们")) {
            QMessageBox::about(this, tr("About"),
                tr("AcroViewTester\n"
                   "Version/版本: 1.0.0\n"
                   "Copyright (C) 2024\n\n"
                   "Shenzhen Aukey Technology Co., Ltd.\n"
                   "深圳市昂科技术有限公司\n\n"
                   "Address/地址: 7F, Hanking Center,\n"
                   "Yuehai Street, Nanshan District,\n"
                   "Shenzhen, 518057, P.R.C\n"
                   "深圳市南山区粤海街道汉京金融中心七楼"));
        }

        });
}