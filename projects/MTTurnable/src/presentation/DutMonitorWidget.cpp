#include "presentation/DutMonitorWidget.h"
#include "services/DutManager.h"
#include "domain/Dut.h"
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QMetaEnum>
#include <QSet>
#include <QDialog>

namespace Presentation {

DutMonitorWidget::DutMonitorWidget(QWidget *parent) :
    QWidget(parent)
{
    setupUi();
}

DutMonitorWidget::~DutMonitorWidget()
{
}

void DutMonitorWidget::setupUi()
{
    auto layout = new QVBoxLayout(this);
    m_tableWidget = new QTableWidget(this);
    layout->addWidget(m_tableWidget);

    // 列：Site + DUT1..m_maxDutPerSite
    QStringList headers;
    headers << "Site";
    for (int i = 0; i < m_maxDutPerSite; ++i) {
        headers << QString("DUT%1").arg(i+1);
    }

    m_tableWidget->setColumnCount(headers.size());
    m_tableWidget->setHorizontalHeaderLabels(headers);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);

    connect(m_tableWidget, &QTableWidget::cellDoubleClicked,
            this, &DutMonitorWidget::onCellDoubleClicked);
}

void DutMonitorWidget::setDutManager(std::shared_ptr<Services::DutManager> dutManager)
{
    if (m_dutManager) {
        // Disconnect old signals
        disconnect(m_dutManager.get(), &Services::DutManager::dutRegistered, this, &DutMonitorWidget::onDutRegistered);
        disconnect(m_dutManager.get(), &Services::DutManager::dutStateChanged, this, &DutMonitorWidget::onDutStateChanged);
        disconnect(m_dutManager.get(), &Services::DutManager::dutSiteChanged, this, &DutMonitorWidget::onDutSiteChanged);
    }

    m_dutManager = dutManager;

    if (m_dutManager) {
        // Connect new signals
        connect(m_dutManager.get(), &Services::DutManager::dutRegistered, this, &DutMonitorWidget::onDutRegistered);
        connect(m_dutManager.get(), &Services::DutManager::dutStateChanged, this, &DutMonitorWidget::onDutStateChanged);
        connect(m_dutManager.get(), &Services::DutManager::dutSiteChanged, this, &DutMonitorWidget::onDutSiteChanged);

        // Populate with existing DUTs
        m_tableWidget->setRowCount(0);
        m_siteRowMap.clear();
        m_dutIndexMap.clear();
        QStringList dutIds = m_dutManager->getAllDutIds();
        for (const QString& id : dutIds) {
            onDutRegistered(id);
        }
    }
}

void DutMonitorWidget::onDutRegistered(const QString& dutId)
{
    if (!m_dutManager) {
        return;
    }

    auto dut = m_dutManager->getDut(dutId);
    if (!dut) {
        return;
    }

    QString siteId = dut->getCurrentSite();
    if (siteId.isEmpty()) siteId = "Unknown";

    placeDutInCell(siteId, dutId, dut);
}

void DutMonitorWidget::onDutStateChanged(const QString& dutId, Domain::Dut::State newState)
{
    if (!m_dutManager) {
        return;
    }

    auto dut = m_dutManager->getDut(dutId);
    if (!dut) return;

    QString siteId = dut->getCurrentSite();
    if (siteId.isEmpty()) siteId = "Unknown";

    placeDutInCell(siteId, dutId, dut);
}

void DutMonitorWidget::onDutSiteChanged(const QString &dutId, const QString &newSite)
{
    Q_UNUSED(newSite)
    // Simply call state changed handler to reposition
    if (m_dutManager) {
        auto dut = m_dutManager->getDut(dutId);
        if (dut) {
            onDutStateChanged(dutId, dut->getState());
        }
    }
}

int DutMonitorWidget::ensureSiteRow(const QString &siteId)
{
    if (m_siteRowMap.contains(siteId)) {
        return m_siteRowMap.value(siteId);
    }
    int newRow = m_tableWidget->rowCount();
    m_tableWidget->insertRow(newRow);
    m_tableWidget->setItem(newRow,0,new QTableWidgetItem(siteId));
    // init empty cells
    for(int col=1; col<m_tableWidget->columnCount(); ++col){
        m_tableWidget->setItem(newRow,col,new QTableWidgetItem(""));
    }
    m_siteRowMap[siteId] = newRow;
    return newRow;
}

void DutMonitorWidget::placeDutInCell(const QString &siteId, const QString &dutId, std::shared_ptr<Domain::Dut> dut)
{
    int row = ensureSiteRow(siteId);
    // Determine index of dut within site
    int index;
    if (m_dutIndexMap.contains(dutId)) {
        index = m_dutIndexMap.value(dutId);
    } else {
        // assign next free index
        QSet<int> used;
        for (auto it = m_dutIndexMap.begin(); it != m_dutIndexMap.end(); ++it) {
            if (it.value()>=0 && it.key()!=dutId && m_dutManager->getDut(it.key())->getCurrentSite()==siteId) {
                used.insert(it.value());
            }
        }
        index = 0;
        while(used.contains(index) && index < m_maxDutPerSite) ++index;
        if (index >= m_maxDutPerSite) return; // exceed
        m_dutIndexMap[dutId]=index;
    }

    int col = index + 1; // column after Site
    QTableWidgetItem* item = m_tableWidget->item(row,col);
    if (!item){
        item = new QTableWidgetItem();
        m_tableWidget->setItem(row,col,item);
    }

    // text state
    const QMetaEnum metaEnum = QMetaEnum::fromType<Domain::Dut::State>();
    const char* stateName = metaEnum.valueToKey(static_cast<int>(dut->getState()));
    item->setText(stateName? stateName: "Unknown");
    // color
    QColor color = Qt::white;
    switch(dut->getState()){
        case Domain::Dut::State::Activating:
        case Domain::Dut::State::Testing:
        case Domain::Dut::State::Calibrating:
            color=QColor(255, 243, 205); // Light Yellow
            break;
        case Domain::Dut::State::ActivationPassed:
        case Domain::Dut::State::TestingPassed:
        case Domain::Dut::State::CalibrationPassed:
            color=QColor(212, 237, 218); // Light Green
            break;
        case Domain::Dut::State::ActivationFailed:
        case Domain::Dut::State::TestingFailed:
        case Domain::Dut::State::CalibrationFailed:
        case Domain::Dut::State::Error:
            color=QColor(248, 215, 218); // Light Red
            break;
        case Domain::Dut::State::AtActivationStation:
        case Domain::Dut::State::AtTestStation:
            color=QColor(209, 236, 241); // Light Blue
            break;
        default: 
            break;
    }
    item->setBackground(color);
    item->setData(Qt::UserRole, dutId);
}

void DutMonitorWidget::onCellDoubleClicked(int row, int column)
{
    if (column==0) return; // site column
    QTableWidgetItem* item = m_tableWidget->item(row,column);
    if(!item) return;
    QString dutId = item->data(Qt::UserRole).toString();
    if(dutId.isEmpty()) return;

    auto dut = m_dutManager? m_dutManager->getDut(dutId): nullptr;
    if(!dut) return;

    // Show dialog
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle(QString("DUT %1 测试结果").arg(dutId));
    QVBoxLayout *lay = new QVBoxLayout(dlg);
    QTableWidget *tbl = new QTableWidget(dlg);
    lay->addWidget(tbl);
    tbl->setColumnCount(2);
    tbl->setHorizontalHeaderLabels({"Key","Value"});
    auto updateFunc=[tbl,dut](){
        QVariantMap data = dut->getTestData();
        tbl->setRowCount(data.size());
        int r=0;
        for(auto it=data.begin(); it!=data.end(); ++it){
            tbl->setItem(r,0,new QTableWidgetItem(it.key()));
            tbl->setItem(r,1,new QTableWidgetItem(it.value().toString()));
            ++r;
        }
    };
    updateFunc();
    connect(dut.get(), &Domain::Dut::testDataChanged, dlg, [updateFunc](const QString&, const QVariantMap&){ updateFunc(); });

    dlg->resize(400,300);
    dlg->exec();
}

} // namespace Presentation 