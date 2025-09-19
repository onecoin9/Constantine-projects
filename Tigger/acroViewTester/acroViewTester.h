#ifndef ACROVIEWTESTER_H
#define ACROVIEWTESTER_H
#pragma execution_character_set("utf-8")
#include <QtWidgets/QMainWindow>
#include "ui_acroViewTester.h"
#include "QFile.h"
#include "qtextstream.h"
#include "qdebug.h"
#include "directTriggerMenu.h"
#include "setting.h"
#include <QMessageBox>
#include <QSettings>
#include <QMenu>
#include <QAction>
#include "ProductInfoModel.h"
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QTimer>
#include "menubar.h"

QT_BEGIN_NAMESPACE
namespace Ui { class acroViewTester; }
QT_END_NAMESPACE

class acroViewTester : public QMainWindow
{
    Q_OBJECT

public:
    acroViewTester(QWidget *parent = nullptr);
    ~acroViewTester();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void setupMenuBar();
    void createViewMenu();
    void loadViewSettings();
    void saveViewSettings();
    void setupUI();
    void setupConnections();
    void loadSettings();
    void saveSettings();
    void initializeProductInfo();

private slots:
    void initForm();
    void loadStyle(const QString& qssFile);
    void handleMenuTrigger(); 
    void settingTrigger();
    void onViewActionToggled(bool checked);
    void onSettingTriggered();
    void onSettingsChanged();
    void updateViewVisibility(const QStringList& visibleViews);
    void updateProductInfo();
    void onMenuTriggered(QAction* action);
    void onUserManageTriggered();
    void onLanguageTriggered();
    void onAboutTriggered();
    void onExitTriggered();
    void refreshProductList();

private:
    Ui::acroViewTester *ui;
    DirectTriggerMenu* directMenu;
    settingDialog* settingDialog_ui = nullptr;
    QSettings *settings;
    QMenu* viewMenu;
    QMap<QString, QAction*> viewActions;
    QMap<QString, QWidget*> viewWidgets;
    ProductInfoModel *productInfoModel;
};
#endif