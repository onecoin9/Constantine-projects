#ifndef DUTMONITORWIDGET_H
#define DUTMONITORWIDGET_H

#include <QWidget>
#include <QMap>
#include <memory>
#include "domain/Dut.h"
#include <QTableWidget>

// 前向声明
namespace Ui { class DutMonitorWidget; }
namespace Services { class DutManager; }
class QTableWidget;
class QTableWidgetItem;

namespace Presentation {

class DutMonitorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DutMonitorWidget(QWidget *parent = nullptr);
    ~DutMonitorWidget();

    // 设置DutManager，并开始监听
    void setDutManager(std::shared_ptr<Services::DutManager> dutManager);

private slots:
    // 当DutManager注册新Dut时调用
    void onDutRegistered(const QString& dutId);

    // 当Dut状态改变时调用
    void onDutStateChanged(const QString& dutId, Domain::Dut::State newState);

    // 当Dut工位改变时调用
    void onDutSiteChanged(const QString& dutId, const QString& newSite);

    // 用户双击单元格查看测试结果
    void onCellDoubleClicked(int row, int column);

private:
    void setupUi();
    void updateSiteRow(const QString& siteId);
    int ensureSiteRow(const QString& siteId);
    void placeDutInCell(const QString& siteId, const QString& dutId, std::shared_ptr<Domain::Dut> dut);

    QTableWidget* m_tableWidget;
    std::shared_ptr<Services::DutManager> m_dutManager;
    QMap<QString, int> m_siteRowMap; // siteId -> row
    QMap<QString, int> m_dutIndexMap; // dutId -> index in site (0..n)
    const int m_maxDutPerSite = 8; // 暂定，可扩展 32
};

} // namespace Presentation

#endif // DUTMONITORWIDGET_H 