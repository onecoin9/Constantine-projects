#include "acroViewTester.h"
#include <QDebug>

acroViewTester::acroViewTester(QWidget *parent)
    : QMainWindow(parent)
    , settings("YourCompany", "acroViewTester")
    , settingDialog_ui(nullptr)
    , directMenu(nullptr)
    , productInfoModel(new ProductInfoModel(this))
{
    ui.setupUi(this);
    initForm();
}

acroViewTester::~acroViewTester()
{
    saveSettings();
    if (settingDialog_ui) {
        delete settingDialog_ui;
        settingDialog_ui = nullptr;
    }
    if (directMenu) {
        delete directMenu;
        directMenu = nullptr;
    }
}

void acroViewTester::initForm()
{
    setupUI();
    setupConnections();
    setupMenuBar();
    setupProductInfo();
    loadSettings();
}

void acroViewTester::setupUI()
{
    // 设置窗口标题和初始大小
    setWindowTitle(tr("AcroView测试工具"));
    resize(1024, 768);

    // 初始化视图组件映射
    viewWidgets["产品信息"] = ui.listViewProdInfo;
    viewWidgets["主视图"] = ui.tabWidgetMainView;
    viewWidgets["副视图"] = ui.tabWidgetViceView;

    // 设置列表视图的样式
    ui.listViewProdInfo->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.listViewProdInfo->setAlternatingRowColors(true);
    ui.listViewProdInfo->setSpacing(2);

    // 设置样式表
    ui.listViewProdInfo->setStyleSheet(
        "QListView {"
        "    background-color: white;"
        "    border: 1px solid #CCCCCC;"
        "}"
        "QListView::item {"
        "    padding: 5px;"
        "}"
        "QListView::item:alternate {"
        "    background-color: #F5F5F5;"
        "}"
        "QListView::item:selected {"
        "    background-color: #E0E0E0;"
        "    color: black;"
        "}"
    );
}

void acroViewTester::setupConnections()
{
    // 连接菜单动作
    connect(ui.actionSetting, &QAction::triggered, this, &acroViewTester::onSettingTriggered);
}

void acroViewTester::setupProductInfo()
{
    // 设置产品信息模型
    ui.listViewProdInfo->setModel(productInfoModel);
    
    // 初始更新产品信息
    updateProductInfo();
}

void acroViewTester::updateProductInfo()
{
    // 更新产品信息数据
    productInfoModel->updateLeakageRate();
    // 可以添加其他更新...
}

void acroViewTester::refreshProductList()
{
    // 刷新列表显示
    if (productInfoModel) {
        productInfoModel->initializeData();
    }
}

void acroViewTester::onSettingTriggered()
{
    if (!settingDialog_ui) {
        settingDialog_ui = new settingDialog(this);
        connect(settingDialog_ui, &settingDialog::settingsChanged,
                this, &acroViewTester::onSettingsChanged);
    }
    settingDialog_ui->show();
}

void acroViewTester::onSettingsChanged()
{
    loadSettings();
    updateProductInfo();
}

// ... 其他必要的函数实现 ...

void acroViewTester::loadSettings()
{
    // 加载设置
    QStringList visibleViews = settings.value("VisibleViews").toStringList();
    if (visibleViews.isEmpty()) {
        visibleViews = viewWidgets.keys();
    }
    updateViewVisibility(visibleViews);

    if (settings.contains("WindowGeometry")) {
        restoreGeometry(settings.value("WindowGeometry").toByteArray());
    }
    if (settings.contains("WindowState")) {
        restoreState(settings.value("WindowState").toByteArray());
    }
}

void acroViewTester::saveSettings()
{
    // 保存设置
    QStringList visibleViews;
    for (auto it = viewWidgets.begin(); it != viewWidgets.end(); ++it) {
        if (it.value()->isVisible()) {
            visibleViews.append(it.key());
        }
    }
    settings.setValue("VisibleViews", visibleViews);
    settings.setValue("WindowGeometry", saveGeometry());
    settings.setValue("WindowState", saveState());
}

void acroViewTester::updateViewVisibility(const QStringList& visibleViews)
{
    for (auto it = viewWidgets.begin(); it != viewWidgets.end(); ++it) {
        bool shouldBeVisible = visibleViews.contains(it.key());
        it.value()->setVisible(shouldBeVisible);
    }
}

void acroViewTester::closeEvent(QCloseEvent *event)
{
    saveSettings();
    event->accept();
}