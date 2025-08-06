#ifndef ACR0VIEWTESTER_H
#define ACR0VIEWTESTER_H

// 1. 系统和Qt基础头文件
#pragma comment(lib, "Qt5Xlsx.lib")
#pragma comment(lib, "Qt5Xlsxd.lib")

#include <QtWidgets/QMainWindow>
#include <QtCore/qprocess.h>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QSettings>
#include <QTimer>
#include <QDateTime>
#include <QAxObject>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QDateTime>
#include <QDir>

// 2. Qt UI相关头文件
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QScrollArea>
#include <QScreen>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <QFileDialog>
#include <QTableWidget>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>

// 3. Qt网络相关头文件
#include <QNetworkInterface>
#include <QAxObject>

// 4. 自定义组件头文件
#include "ui_acroViewTester.h"
#include "testerDefinitions.h"
#include "directTriggerMenu.h"
#include "setting.h"
#include "ProductInfoModel.h"
#include "testsite.h"
#include "testSiteGrid.h"
#include "blockDetailDialog.h"
#include "measureDialog.h"
#include "configDialog.h"
#include "version.h"
#include "inputDialog.h"

// 5. 网络通信相关头文件
#include "tcpIpModule.h"
#include "jsonRpcClient.h"
#include "tcphandler.h"
#include "handlerController.h"
#include "UdpCommunicator.h" 
#include "CommandBuilder.h"

// 6. 自动化管理相关头文件
#include "./AutomaticManager/ACAutomaticManager.h"

// 7. 标准库
#include <map>
#include <QtGui/qstandarditemmodel.h>

// 8.spdlog
#include "logmanager.h"

class acroViewTester : public QMainWindow
{
    Q_OBJECT

public:
    // ========================================
    // 构造和析构
    // ========================================
    acroViewTester(QWidget* parent = nullptr, 
                  const QString &projectConfigPath = QString(), 
                  const QString &projectName = QString());
    ~acroViewTester();

    // ========================================
    // 数据结构定义
    // ========================================
    struct ProductTestInfo {
        QString customerCode;
        QString productModel;
        QString weekly;
        QString customerBatch;
        QString workOrder;
        QString workStep;
        QString process;
        QString deviceNumber;
    };

    struct ItemText {
        QString text;
    };

public:
    // ========================================
    // 1. 初始化系统 - 系统启动和组件初始化
    // ========================================
    void initEssentials();              // 基础组件初始化
    void initNonEssentials();           // 非必需组件初始化
    void initForm();                    // 窗体初始化
    void initProductModel();            // 产品模型初始化
    void initStatusBar();               // 状态栏初始化
    void initAutoMatic();               // 自动化组件初始化
    void initTabContent(int index);     // 标签页内容初始化
    void initBasicTab();                // 基础标签页初始化
    void initDatabaseTab();             // 数据库标签页初始化
    void initAlarmTab();                // 报警标签页初始化
    void initializeUdpInterface();      // UDP接口初始化

    // ========================================
    // 2. UI构建系统 - 界面组件设置和布局
    // ========================================
    void setupTestSites();             // 测试站点设置
    void setupMenuBar();               // 菜单栏设置
    void setupExpandButton();          // 展开按钮设置
    void setupViceView();              // 副视图设置
    void setupMeasurementDialog();     // 测量对话框设置
    void setupTableView();             // 表格视图设置
    void setupDataUI();                // 数据UI设置
    void setupTestButtons();           // 测试按钮设置

    // ========================================
    // 3. UI管理系统 - 界面更新和显示控制
    // ========================================
    void createViewMenu();             // 创建视图菜单
    void updateExpandButton(bool expanded);         // 更新展开按钮状态
    void updateTableViewAlarmData();               // 更新报警数据表格
    void updateTime();                             // 更新时间显示
    void updateProductInfo(const QString& key, const QString& value);  // 更新产品信息
    void updateGlobalCmd(const QString& selectedOption);              // 更新全局命令
    void updateSiteStatus(int siteIndex, uint32_t slotEn, const std::string& strSiteSn);  // 更新站点状态
    void clearViceViewControls();      // 清除副视图控件
    void showWidgetA();                // 显示控件A
    void showAgingTestControls();      // 显示老化测试控件
    void showAG06Controls();           // 显示AG06控件
    void showAP8000Controls();         // 显示AP8000控件

    // ========================================
    // 4. 配置管理系统 - 设置和配置文件处理
    // ========================================
    void loadViewSettings();           // 加载视图设置
    void saveViewSettings();           // 保存视图设置
    void loadSettings();               // 加载系统设置
    void loadStyle(const QString& qssFile);        // 加载样式文件
    void loadComboBoxItems();          // 加载下拉框项目
    void loadDoJobComboBoxItems();     // 加载作业下拉框项目
    bool loadProjectConfig();          // 加载项目配置
    void applyProjectConfig();         // 应用项目配置

    // ========================================
    // 5. 数据处理系统 - 数据操作和模型管理
    // ========================================
    void addSampleData();              // 添加示例数据
    void addLegendToGroupBox();        // 添加图例到组框
    void addItemsToHandlers(const QList<QString>& items);             // 添加项目到处理器
    void mergeModels(const QList<QStandardItemModel*>& models, QStandardItemModel* targetModel);  // 合并数据模型
    QJsonObject collectAllBlocksStatus();          // 收集所有块状态
    void exportToExcel(const QString& fileName);   // 导出到Excel
    void updatePageInfo(bool forceRefresh = false); // 更新分页信息
    void resetSearch();                             // 重置搜索

    // ========================================
    // 6. UI创建工具 - 动态UI组件创建
    // ========================================
    QWidget* createContentWidget(int index);       // 创建内容控件
    QWidget* createStatusLegend();                  // 创建状态图例
    void createPathLabel(const QString& label, const QString& value, int row);  // 创建路径标签

    // ========================================
    // 7. 网络通信系统 - 网络连接和数据传输
    // ========================================
    void connectToJsonRpcServer();     // 连接到JSON-RPC服务器
    void handleJsonRequest(const QString& methodName, const QJsonObject& params);  // 处理JSON请求
    QString getLocalIPAddress();       // 获取本地IP地址

    // ========================================
    // 8. 命令发送系统 - 各种命令的发送
    // ========================================
    void sendLoadProject();            // 发送加载项目命令
    void sendStartService();           // 发送启动服务命令
    void sendSiteScanAndConnect();     // 发送站点扫描连接命令
    void sendDoJob();                  // 发送执行作业命令
    void sendGetAdapterEn();           // 发送获取适配器使能命令
    void sendDoCustom();               // 发送自定义命令
    void sendLogInterface();           // 发送日志接口命令
    void sendEventInterface();         // 发送事件接口命令
    void sendGetJobResult();           // 发送获取作业结果命令
    void sendAdapterEn();              // 发送适配器使能命令
    void sendGetProgrammerInfo();      // 发送获取编程器信息命令
    void sendGetProjectInfo();         // 发送获取项目信息命令
    void sendGetSiteStatus();          // 发送获取站点状态命令
    void sendGetSktInfo();             // 发送获取套接字信息命令
    void sendJsonRpcData();            // 发送JSON-RPC数据

    // ========================================
    // 9. 工具函数系统 - 辅助功能和工具方法
    // ========================================
    QString getCurrentVersion();       // 获取当前版本
    void openFileApp();                // 打开文件应用
    void openConfigDialog();           // 打开配置对话框
    int findSiteRow(int siteIndex);    // 查找站点行
    void initSiteStatusTable();        // 初始化站点状态表
    void startProgramming(int siteIndex, uint32_t slotEn, const std::string& strSiteSn);  // 开始编程
    QString getSetTaskErrorMessage(int errorCode);  // 获取设置任务错误消息
    void printMessage(QString errStr);              // 打印消息
    void spdlogTest();                              // spdlog测试
    QString buildRdyInfoString();                   // 构建就绪信息字符串
    void logToUdpInterface(const QString& message); // 记录到UDP接口

public slots:
    // ========================================
    // UI事件处理槽函数
    // ========================================
    void on_pushButtonExpand_clicked();             // 展开按钮点击
    void on_pushButtonSendUID_clicked();            // 发送UID按钮点击
    void on_exportButton_clicked();                 // 导出按钮点击
    void onPushButtonSiteClicked(int index);        // 站点按钮点击
    void onpushButtonJsonRpcResultClicked();        // JSON-RPC结果按钮点击
    void onPushButtonStartClicked();                // 开始按钮点击
    void onSettingsChanged();                       // 设置改变
    void onGridSizeChanged(int rows, int cols, int baseRows, int baseCols);  // 网格大小改变
    void onViewActionToggled(bool checked);         // 视图动作切换
    void onProductItemSelected(const QModelIndex& current, const QModelIndex& previous);  // 产品项目选择
    void onSceneChanged(const QString& scene);      // 场景改变

    // ========================================
    // JSON-RPC事件处理槽函数
    // ========================================
    void onJsonRpcConnected();                      // JSON-RPC连接成功
    void onJsonRpcDisconnected();                   // JSON-RPC断开连接
    void onJsonRpcSocketError(QAbstractSocket::SocketError error);  // JSON-RPC套接字错误
    void onJsonRpcProtocolError(const QString& errorString);        // JSON-RPC协议错误
    void onJsonRpcServerCommandReceived(const QString& cmd, const QJsonObject& data);  // 接收服务器命令
    void onJsonRpcResponseReceived(qint64 id, const QJsonValue& result);  // 接收响应

    // ========================================
    // 其他事件处理槽函数
    // ========================================
    void settingTrigger();                          // 设置触发
    void handleMenuTrigger();                       // 菜单触发处理
    void handlerTellDevReady();                     // 处理设备就绪通知
    void onComboBoxDoJobJsonChanged(const QString& selectedOption);  // 作业JSON下拉框改变
    void onBrowseTaskFileClicked();                 // 浏览任务文件点击
    void onChipCountChanged(const QString& text);   // 芯片数量改变
    void onChipIsInPlace(int siteIndex, uint32_t slotEn, std::string strSiteSn);  // 芯片就位
    void onProgramResultBack(int result, int errCode, const char* errStr);  // 编程结果返回

    // ========================================
    // 自动化管理事件槽函数
    // ========================================
    void onbtnQuerySiteMappingClicked();            // 查询站点映射按钮点击
    void onSetTaskClicked();                        // 设置任务点击
    void onTellDevReadyClicked();                   // 通知设备就绪点击
    void onSetDoneSiteClicked();                    // 设置完成站点点击

    // ========================================
    // 数据管理事件槽函数
    // ========================================
    void on_btnFirst_clicked();                     // 首页按钮点击
    void on_btnPreVious_clicked();                  // 上一页按钮点击
    void on_btnNext_clicked();                      // 下一页按钮点击
    void on_btnLast_clicked();                      // 末页按钮点击
    void on_btnInsert_clicked();                    // 插入按钮点击
    void on_btnDelete_clicked();                    // 删除按钮点击
    void on_btnSelect_clicked();                    // 选择按钮点击
    void onLogDataChanged();                        // 日志数据改变
    void on_importDataButton_clicked();             // 导入数据按钮点击
    void importFromDB();                            // 从数据库导入
    void importFromJson();                          // 从JSON导入
    void importFromExcel();                         // 从Excel导入

    // ========================================
    // UDP通信事件槽函数
    // ========================================
    void on_udpApplyConfigButton_clicked();         // UDP应用配置按钮点击
    void on_udpSendDeviceControlButton_clicked();   // UDP发送设备控制按钮点击
    void on_udpSendDataAcqConfigButton_clicked();   // UDP发送数据采集配置按钮点击
    void handleUdpDeviceControlResponse(int code, int timestamp, const QHostAddress& senderAddress, quint16 senderPort);  // 处理UDP设备控制响应
    void handleUdpFunctionTestResponse(int code, int timestamp, const QHostAddress& senderAddress, quint16 senderPort);   // 处理UDP功能测试响应
    void handleUdpSensorDataReceived(const CommandBuilder::SensorData& sensorData, const QHostAddress& senderAddress, quint16 senderPort);  // 处理UDP传感器数据接收
    void handleUdpUnknownPacketReceived(quint16 cmdID, const QJsonObject& jsonData, const QHostAddress& senderAddress, quint16 senderPort);  // 处理UDP未知数据包接收
    void handleUdpBindError(const QString& errorString);   // 处理UDP绑定错误
    void handleUdpSendError(const QString& errorString);   // 处理UDP发送错误

    // ========================================
    // 调试和测试槽函数
    // ========================================
    void onTestNullPointer();                       // 测试空指针
    void onTestArrayOverflow();                     // 测试数组溢出
    void onTestDivideByZero();                      // 测试除零
    void onTestStackOverflow();                     // 测试栈溢出

public:
    // ========================================
    // 核心组件成员变量
    // ========================================
    Ui::acroViewTesterClass ui;
    DirectTriggerMenu* directMenu;
    settingDialog* settingDialog_ui = nullptr;
    QSettings settings;
    ConfigDialog* m_configDialog = nullptr;
    IAutomatic* mAutomatic;
    UdpCommunicator *m_udpCommunicator;

    // ========================================
    // UI组件成员变量
    // ========================================
    QMenu* viewMenu;
    QStatusBar* statusBar;
    QLabel* ipLabel;
    QLabel* versionLabel;
    QLabel* timeLabel;
    QLabel* userLabel;
    QLabel* m_statusDot;
    QLabel* m_arrowIcon;
    QLabel* jsonRpcResultLabel;
    QTimer* timer;
    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
    QWidget* m_widgetA;
    QWidget* m_scrollContent;
    QTableView* tableViewAlarmData;
    QTableWidget* tableWidgetSiteMapping;
    QLineEdit* lineEditTaskInfo;
    QTextEdit* textEditDevReadyInfo;
    QLineEdit* lineEditSiteIndex;
    QLineEdit* lineEditBurnResult;

    // ========================================
    // 数据模型成员变量
    // ========================================
    ProductInfoModel* productModel;
    QStandardItemModel* dataModel;
    QStandardItemModel* jsonRpcResultModel;
    QStandardItemModel* jsonRpcResultModel1;
    QStandardItemModel* m_tableModel;
    QList<QStandardItemModel*> jsonModels;
    QList<QStandardItemModel*> jsonModels1;

    // ========================================
    // 集合类成员变量
    // ========================================
    QMap<QString, QAction*> viewActions;
    QMap<QString, QWidget*> viewWidgets;
    QVector<TestSite*> m_testSites;
    QVector<bool> m_tabsInitialized;
    QVector<int> m_trayEnabled;
    QVector<QString> m_extraParams;
    std::map<QString, std::function<void()>> handlers;

    // ========================================
    // 网络通信成员变量
    // ========================================
    TcpIpModule tcpModule;
    JsonRpcClient jsonRpcClient;
    TcpHandler tcphanlder;
    HandlerController handlercontroller;

    // ========================================
    // 业务数据成员变量
    // ========================================
    ProductTestInfo testInfo;
    DeviceInfo deviceInfo;
    JobResult jobResult;
    ProjectInfo projectInfo;
    QString m_projectConfigPath;
    QString m_projectName;
    QJsonObject m_projectConfig;

    // ========================================
    // 状态和索引成员变量
    // ========================================
    int currentModelIndex;
    int currentModelIndex1;
    bool m_isExpanded;
    int m_commandId;
    int m_chipCount;
    QString m_tskFilePath;
    int m_currentPage = 1;
    int m_totalPages = 1;
    int m_pageSize = 12;

    // ========================================
    // UI控制结构
    // ========================================
    struct ViceViewControls {
        QVector<QPair<QLabel*, QLabel*>> pathLabels;
    };
    ViceViewControls m_viceViewControls;
};

#endif // ACR0VIEWTESTER_H