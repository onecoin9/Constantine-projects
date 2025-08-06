//// blockdetaildialog.h
//#ifndef BLOCKDETAILDIALOG_H
//#define BLOCKDETAILDIALOG_H
//#pragma execution_character_set("utf-8")
//#include <QDialog>
//#include <QtCharts>
//#include "testsite.h"
//
//namespace Ui {
//    class BlockDetailDialog;
//}
//
//class BlockDetailDialog : public QDialog
//{
//    Q_OBJECT
//
//public:
//    explicit BlockDetailDialog(const TestSite::BlockData& data, QWidget* parent = nullptr);
//    ~BlockDetailDialog();
//
//private:
//    void setupUI();
//    void setupCharts();
//    void updateInfo();
//    void createOSChart();
//    void createLeakageChart();
//
//private:
//    Ui::BlockDetailDialog* ui;
//    TestSite::BlockData m_data;
//    QChartView* m_osChartView;
//    QChartView* m_leakageChartView;
//};
//
//#endif