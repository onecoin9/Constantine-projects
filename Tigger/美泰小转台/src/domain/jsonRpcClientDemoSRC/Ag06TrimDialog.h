 #ifndef AG06_TRIM_DIALOG_H
 #define AG06_TRIM_DIALOG_H

 #include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
 #include <QLabel>
 #include <QSpinBox>
 #include <QCheckBox>
 #include <QPushButton>
 #include <QGridLayout>
 #include <QHBoxLayout>
 #include <QVBoxLayout>
 #include <QFormLayout>
 #include <QFileDialog>
 #include <QJsonObject>
 #include <QJsonDocument>
#include <QTableWidget>
#include <QVector>
#include <QHash>
#include <QTabWidget>
 #include "xt_trim_param.h"

 class Ag06TrimDialog : public QDialog
 {
     Q_OBJECT
 public:
     explicit Ag06TrimDialog(QWidget* parent = nullptr);

signals:
    // JSON 文本下发（供 JsonRpcTestWidget.cpp 连接使用）
    void sendTrimJsonRequested(const QString& json);

     void sendUidRequested(const QString& uid);
     void sendTrimStructRequested(const xt_trim_t& trim);

 private:
    // UI 构建与各页签
    void buildUi();
    void setupBinaryTab();
    void setupTableTab();
    void buildTable();

    // RegBit 行编辑辅助
    struct RegBitWidgets { QSpinBox* addr=nullptr; QSpinBox* start_bit=nullptr; QSpinBox* width_bit=nullptr; QSpinBox* write_back=nullptr; };
    void setupRegBitRow(QGridLayout* grid, int row, const QString& title, RegBitWidgets& w);

    // JSON/Struct 序列化
    QJsonObject buildTrimJson() const;
    void loadTrimJson(const QJsonObject& obj);
    xt_trim_t collectTrim() const;

    // 表格数据访问
    struct RowDef { QString zh; QString path; QString type; QString unit; quint64 minV; quint64 maxV; QString example; };
    int findRow(const QString& path) const; // 未找到返回 -1
    QString getCellText(const QString& path) const;
    quint64 getCellUInt(const QString& path) const; // 根据 type 自动解析
    void setCellText(const QString& path, const QString& text);
    void setCellUInt(const QString& path, quint64 v);
    QString rangeHint(const RowDef& r) const;
    void validateRow(int row);
    void validateAll();

    // 页签
    QTabWidget* m_tabs = nullptr;
    QWidget* m_binaryTab = nullptr;
    QWidget* m_trimTab = nullptr;

    // 表格页
    QTableWidget* m_table = nullptr; // 表格
    QVector<RowDef> m_rowDefs;
    QHash<QString,int> m_rowIndex; // path -> row
    QPushButton* m_tableImportJsonBtn = nullptr;
    QPushButton* m_tableExportJsonBtn = nullptr;
    QPushButton* m_tableSendTrimBtn = nullptr;
    QLabel* m_tableStatus = nullptr;
    bool m_isUpdating = false;

     QLineEdit* m_uidEdit = nullptr;
     QPushButton* m_sendUidBtn = nullptr;
     QLabel* m_status = nullptr;

    // 二进制页
    QTextEdit* m_trimJsonEdit = nullptr;
    QPushButton* m_sendTrimJsonBtn = nullptr;

    // Trim 编辑器页（已用表格替代，不再在 UI 中使用，但保留成员以兼容旧代码）
    // 使能
    QCheckBox* m_trimEn_dc6 = nullptr;
    QCheckBox* m_trimEn_dc5 = nullptr;
    QCheckBox* m_trimEn_ac27 = nullptr;
    QCheckBox* m_trimEn_ac4 = nullptr;
    QCheckBox* m_trimEn_eoc = nullptr;

    // 寄存器位域
    RegBitWidgets m_reg_output_ctrl;
    RegBitWidgets m_reg_dc_trim;
    RegBitWidgets m_reg_ac_en;
    RegBitWidgets m_reg_ac_trim;
    RegBitWidgets m_reg_eoc;

    // 输出控制
    QSpinBox* m_out_dc6 = nullptr;
    QSpinBox* m_out_dc5 = nullptr;
    QSpinBox* m_out_ac27 = nullptr;
    QSpinBox* m_out_ac4 = nullptr;

    // 参数区
    QSpinBox* m_icc_min = nullptr; QSpinBox* m_icc_max = nullptr;
    QSpinBox* m_dc_basic_min = nullptr; QSpinBox* m_dc_basic_max = nullptr; QSpinBox* m_dc_p2p_max = nullptr;
    QSpinBox* m_dc_trim_min = nullptr; QSpinBox* m_dc_trim_max = nullptr; QSpinBox* m_dc_trim_best = nullptr;
    QSpinBox* m_ac_trim_min = nullptr; QSpinBox* m_ac_trim_max = nullptr; QSpinBox* m_ac_trim_best = nullptr;
    QSpinBox* m_ac_avg_min = nullptr; QSpinBox* m_ac_avg_max = nullptr; QSpinBox* m_ac_p2p_min = nullptr; QSpinBox* m_ac_p2p_max = nullptr;
    QSpinBox* m_ac_freq_min = nullptr; QSpinBox* m_ac_freq_max = nullptr;

    // 延时参数
    QSpinBox* m_power_on_delay_ms = nullptr; QSpinBox* m_t1_dc_stable_ms = nullptr; QSpinBox* m_t1_ac_stable_ms = nullptr;
    QSpinBox* m_delay_after_program_ms = nullptr; QSpinBox* m_reg_operation_delay_us = nullptr;

    // 按钮
    QPushButton* m_saveJsonBtn = nullptr;
    QPushButton* m_loadJsonBtn = nullptr;
    QPushButton* m_sendTrimBtn = nullptr;
 };

 #endif // AG06_TRIM_DIALOG_H

