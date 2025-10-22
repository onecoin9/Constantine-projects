#ifndef DATABASEWIDGET_H
#define DATABASEWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPushButton>
#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QSplitter>
#include <QProgressBar>
#include <QTextEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QTimer>
#include <memory>

// 前向声明
class ChipTestDatabase;
struct ChipTestData;

namespace Presentation {

/**
 * @brief 数据库管理界面组件
 * 提供芯片测试数据的查询、显示、导入导出功能
 */
class DatabaseWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DatabaseWidget(QWidget *parent = nullptr);
    ~DatabaseWidget();

    // 数据库操作接口
    bool initializeDatabase(const QString &databasePath = "chip_test.db");
    void closeDatabase();
    
    // 数据插入接口（供其他模块调用）
    bool insertChipTestData(const ChipTestData &data);
    bool insertChipTestDataFromCsv(const QString &csvFilePath);
    
    // 刷新显示
    void refreshData();

public slots:
    // 查询操作
    void onQueryAll();
    void onQueryByDate();
    void onQueryByBatch();
    void onQueryById();
    void onQueryByMarkingNumber();
    void onClearQuery();
    
    // 数据操作
    void onExportData();
    void onImportCsv();
    void onDeleteSelected();
    void onRefreshData();
    
    // 统计信息更新
    void updateStatistics();

signals:
    // 状态信号
    void statusMessage(const QString &message);
    void errorMessage(const QString &error);
    void dataChanged();

private slots:
    void onTableSelectionChanged();
    void onSearchTextChanged();
    void updateStatusInfo();

private:
    void setupUi();
    void setupQueryPanel();
    void setupDataTable();
    void setupControlPanel();
    void setupStatusPanel();
    void connectSignals();
    
    // 数据操作
    void populateTable(const QList<ChipTestData> &dataList);
    void clearTable();
    QString formatDateTime(const QDateTime &dateTime) const;
    QString formatBoolValue(bool value) const;
    
    // CSV解析
    bool parseCsvFile(const QString &filePath, QList<ChipTestData> &dataList);
    ChipTestData parseCsvLine(const QStringList &fields);
    
    // 验证
    bool validateChipData(const ChipTestData &data) const;

private:
    // 数据库
    std::unique_ptr<ChipTestDatabase> m_database;
    
    // 主布局
    QVBoxLayout *m_mainLayout;
    QSplitter *m_mainSplitter;
    
    // 查询面板
    QGroupBox *m_queryGroup;
    QGridLayout *m_queryLayout;
    QLineEdit *m_searchEdit;
    QDateEdit *m_startDateEdit;
    QDateEdit *m_endDateEdit;
    QLineEdit *m_batchEdit;
    QLineEdit *m_idEdit;
    QLineEdit *m_markingEdit;
    QComboBox *m_chipModelCombo;
    
    // 查询按钮
    QPushButton *m_queryAllBtn;
    QPushButton *m_queryDateBtn;
    QPushButton *m_queryBatchBtn;
    QPushButton *m_queryIdBtn;
    QPushButton *m_queryMarkingBtn;
    QPushButton *m_clearQueryBtn;
    
    // 数据表格
    QTableWidget *m_dataTable;
    
    // 控制面板
    QGroupBox *m_controlGroup;
    QHBoxLayout *m_controlLayout;
    QPushButton *m_exportBtn;
    QPushButton *m_importCsvBtn;
    QPushButton *m_deleteBtn;
    QPushButton *m_refreshBtn;
    
    // 状态面板
    QGroupBox *m_statusGroup;
    QGridLayout *m_statusLayout;
    QLabel *m_totalCountLabel;
    QLabel *m_filteredCountLabel;
    QLabel *m_passRateLabel;
    QLabel *m_lastUpdateLabel;
    QProgressBar *m_operationProgress;
    QLabel *m_statusLabel;
    
    // 详情面板
    QGroupBox *m_detailGroup;
    QTextEdit *m_detailText;
    
    // 定时器
    QTimer *m_refreshTimer;
    
    // 状态
    bool m_isInitialized;
    int m_totalRecords;
    int m_filteredRecords;
    QString m_lastError;
};

} // namespace Presentation

#endif // DATABASEWIDGET_H
