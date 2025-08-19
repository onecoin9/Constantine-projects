#include "ui/DutMonitorWidget.h"
#include "services/DutManager.h"
#include "domain/Dut.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QSettings>
#include <QTimer>
#include <QDateTime>
#include <QMetaEnum>
#include <QJsonObject>
#include <QDebug>
#include <algorithm>
#include <cmath>
#include <QStyle>
#include <QMouseEvent>
#include <QToolTip>
#include <Logger.h>
namespace Presentation {

// SocketCell Implementation
SocketCell::SocketCell(int socketIndex, QWidget *parent)
    : QFrame(parent)
    , m_socketIndex(socketIndex)
    , m_occupied(false)
    , m_enabled(true)
    , m_dutState(Domain::Dut::State::Unknown)
{
    setObjectName("socketCell");
    setFixedSize(100, 40);  // Rectangle: width is 4 times height
    setFrameStyle(QFrame::Box);
    setCursor(Qt::PointingHandCursor);  // Show hand cursor
    setMouseTracking(true);  // Enable mouse tracking

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_statusLabel = new QLabel(QString::number(socketIndex), this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setObjectName("socketLabel");
    layout->addWidget(m_statusLabel);

    setDutState(Domain::Dut::State::Idle);
}

void SocketCell::setDutState(Domain::Dut::State state)
{
    m_dutState = state;

    // Set different properties according to DUT status
    switch (state) {
    case Domain::Dut::State::Unknown:
    case Domain::Dut::State::Idle:
        setProperty("status", "idle");
        break;
    case Domain::Dut::State::AtActivationStation:
    case Domain::Dut::State::AtTestStation:
        setProperty("status", "atstation");
        break;
    case Domain::Dut::State::Activating:
    case Domain::Dut::State::Testing:
    case Domain::Dut::State::Calibrating:
        setProperty("status", "testing");
        break;
    case Domain::Dut::State::ActivationPassed:
    case Domain::Dut::State::TestingPassed:
    case Domain::Dut::State::CalibrationPassed:
        setProperty("status", "passed");
        break;
    case Domain::Dut::State::ActivationFailed:
    case Domain::Dut::State::TestingFailed:
    case Domain::Dut::State::CalibrationFailed:
        setProperty("status", "failed");
        break;
    case Domain::Dut::State::ReadyForCalibration:
    case Domain::Dut::State::ReadyForBinning:
        setProperty("status", "ready");
        break;
    case Domain::Dut::State::Binned:
        setProperty("status", "binned");
        break;
    case Domain::Dut::State::Error:
        setProperty("status", "error");
        break;
    case Domain::Dut::State::SKTEnable:
        setProperty("status", "sktEnable");
        break;
    default:
        setProperty("status", "idle");
        break;
    }

    style()->unpolish(this);
    style()->polish(this);
    update();
}
void SocketCell::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_enabled) {
        emit clicked(m_socketIndex);
    }
    QFrame::mousePressEvent(event);
}

void SocketCell::enterEvent(QEvent *event)
{
    Q_UNUSED(event)

    // Get status string
    QString stateStr = getDutStateString(m_dutState);
    QString tooltip = QString("Socket %1\nStatus: %2").arg(m_socketIndex).arg(stateStr);

    if (m_occupied) {
        tooltip += "\nOccupied";
    } else {
        tooltip += "\nIdle";
    }

    if (!m_enabled) {
        tooltip += "\n(Disabled)";
    }

    QToolTip::showText(QCursor::pos(), tooltip, this);
}

void SocketCell::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    QToolTip::hideText();
}

QString SocketCell::getDutStateString(Domain::Dut::State state) const
{
    switch (state) {
    case Domain::Dut::State::Unknown: return "Unknown";
    case Domain::Dut::State::Idle: return "Idle";
    case Domain::Dut::State::AtActivationStation: return "At Activation Station";
    case Domain::Dut::State::Activating: return "Activating";
    case Domain::Dut::State::ActivationPassed: return "Activation Passed";
    case Domain::Dut::State::ActivationFailed: return "Activation Failed";
    case Domain::Dut::State::AtTestStation: return "At Test Station";
    case Domain::Dut::State::Testing: return "Testing";
    case Domain::Dut::State::TestingPassed: return "Testing Passed";
    case Domain::Dut::State::TestingFailed: return "Testing Failed";
    case Domain::Dut::State::ReadyForCalibration: return "Ready For Calibration";
    case Domain::Dut::State::Calibrating: return "Calibrating";
    case Domain::Dut::State::CalibrationPassed: return "Calibration Passed";
    case Domain::Dut::State::CalibrationFailed: return "Calibration Failed";
    case Domain::Dut::State::ReadyForBinning: return "Ready For Binning";
    case Domain::Dut::State::Binned: return "Binned";
    case Domain::Dut::State::Error: return "Error";
    case Domain::Dut::State::SKTEnable: return "Socket Enabled";
    default: return "Unknown Status";
    }
}

// SiteWidget Implementation
SiteWidget::SiteWidget(const QString& siteName, int socketCount, QWidget *parent)
    : QFrame(parent)
    , m_siteName(siteName)
    , m_socketCount(socketCount)
{
    setObjectName("siteWidget");
    setFrameStyle(QFrame::Box);
    setFixedSize(450, 200);  // Increase size
    setMouseTracking(true);  // Enable mouse tracking

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(5);

    // Socket grid
    QWidget* gridWidget = new QWidget(this);
    m_socketGrid = new QGridLayout(gridWidget);
    m_socketGrid->setSpacing(3);  // Increase spacing
    m_socketGrid->setContentsMargins(0, 0, 0, 0);

    // Create socket cells, 4x4 layout
    int cols = 4;

    for (int i = 0; i < socketCount && i < 16; ++i) {
        SocketCell* cell = new SocketCell(i + 1, this);
        m_sockets.append(cell);

        // Connect click signal
        connect(cell, &SocketCell::clicked, this, [this, i](int) {
            emit socketClicked(m_siteName, i + 1);
        });

        int row = i / cols;
        int col = i % cols;
        m_socketGrid->addWidget(cell, row, col);
    }

    mainLayout->addWidget(gridWidget);
}

void SiteWidget::updateSocketDutState(int socketIndex, Domain::Dut::State state)
{
    if (socketIndex > 0 && socketIndex <= m_sockets.size()) {
        m_sockets[socketIndex - 1]->setDutState(state);
    }
}

void SiteWidget::enterEvent(QEvent *event)
{
    Q_UNUSED(event)
    // Show site name on mouse enter
    QString site_ip = m_siteName + "_" + m_ip;
    QToolTip::showText(QCursor::pos(), site_ip,this);
}

void SiteWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    QToolTip::hideText();
}

void SiteWidget::setScanned(bool scanned)
{
    setProperty("scanned", scanned);

    // Force style refresh
    style()->unpolish(this);
    style()->polish(this);
    update();
}

// DutMonitorWidget Implementation
DutMonitorWidget::DutMonitorWidget(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_scrollWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_columnsPerRow(4)  // 4 columns per row by default
{
    setupUi();
}

DutMonitorWidget::~DutMonitorWidget()
{
}

void DutMonitorWidget::setDutManager(std::shared_ptr<Services::DutManager> dutManager)
{
    if (m_dutManager) {
        disconnect(m_dutManager.get(), nullptr, this, nullptr);
    }

    m_dutManager = dutManager;

    if (m_dutManager) {
        connect(m_dutManager.get(), &Services::DutManager::siteConfigurationLoaded,
                this, &DutMonitorWidget::onSiteConfigurationLoaded);
        connect(m_dutManager.get(), &Services::DutManager::siteChipPlacementChanged,
                this, &DutMonitorWidget::onSiteChipPlacementChanged);
        connect(m_dutManager.get(), &Services::DutManager::siteUpdated,
                this, &DutMonitorWidget::onSiteUpdated);
        connect(m_dutManager.get(), &Services::DutManager::dutStateChanged,
                this, &DutMonitorWidget::onDutStateChanged);

        // If site configuration already exists, create UI immediately
        if (!m_dutManager->getAllSiteInfo().isEmpty()) {
            createSiteWidgets();
        }
    }
}

void DutMonitorWidget::setColumnsPerRow(int columns)
{
    if (columns > 0 && columns <= 6 && columns != m_columnsPerRow) {
        m_columnsPerRow = columns;
        if (m_dutManager && !m_dutManager->getAllSiteInfo().isEmpty()) {
            createSiteWidgets();
        }
    }
}

void DutMonitorWidget::setupUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Create top toolbar
    QWidget* toolBar = new QWidget(this);
    toolBar->setObjectName("dutMonitorToolBar");
    toolBar->setFixedHeight(40);
    QHBoxLayout* toolBarLayout = new QHBoxLayout(toolBar);
    toolBarLayout->setContentsMargins(10, 5, 10, 5);

    // Add column count setting
    QLabel* colLabel = new QLabel("Sites per row:", toolBar);
    QSpinBox* colSpinBox = new QSpinBox(toolBar);
    colSpinBox->setMinimum(1);
    colSpinBox->setMaximum(6);
    colSpinBox->setValue(m_columnsPerRow);
    connect(colSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        m_columnsPerRow = value;
        createSiteWidgets();  // Recreate the layout
    });

    toolBarLayout->addWidget(colLabel);
    toolBarLayout->addWidget(colSpinBox);
    toolBarLayout->addStretch();

    // Add status color legend
    toolBarLayout->addWidget(createLegendWidget());

    layout->addWidget(toolBar);

    // Create scroll area
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameStyle(QFrame::NoFrame);
    m_scrollArea->setObjectName("dutMonitorScrollArea");

    m_scrollWidget = new QWidget();
    m_scrollArea->setWidget(m_scrollWidget);

    m_mainLayout = new QGridLayout(m_scrollWidget);
    m_mainLayout->setSpacing(20);  // Increase spacing
    m_mainLayout->setContentsMargins(20, 20, 20, 20);

    layout->addWidget(m_scrollArea);
}

QWidget* DutMonitorWidget::createLegendWidget()
{
    QWidget* legend = new QWidget();
    QHBoxLayout* legendLayout = new QHBoxLayout(legend);
    legendLayout->setContentsMargins(0, 0, 0, 0);
    legendLayout->setSpacing(10);

    // Status description
    QLabel* legendLabel = new QLabel("Status Legend:", legend);
    legendLabel->setStyleSheet("font-weight: bold;");
    legendLayout->addWidget(legendLabel);

    // Add color samples for various statuses
    struct LegendItem {
        QString label;
        QString color;
        QString property;
    };

    QList<LegendItem> items = {
        {"Unknown/Idle", "#e8e8e8", "idle"},
        {"At Station", "#B3E5FC", "atstation"},
        {"In Progress", "#FFB74D", "testing"},
        {"Passed", "#81C784", "passed"},
        {"Failed", "#E57373", "failed"},
        {"Ready", "#A5D6A7", "ready"},
        {"Binned", "#90A4AE", "binned"},
        {"Error", "rgb(196, 6, 6)", "error"},
        {"Socket Enabled", "rgb(32, 35, 194)", "sktEnable"},
        {"Disabled", "#d0d0d0", "disabled"}
    };

    for (const auto& item : items) {
        QFrame* colorBox = new QFrame(legend);
        colorBox->setFixedSize(16, 16);
        colorBox->setStyleSheet(QString("background-color: %1; border: 1px solid #999;").arg(item.color));

        QLabel* label = new QLabel(item.label, legend);
        label->setStyleSheet("font-size: 12px;");

        legendLayout->addWidget(colorBox);
        legendLayout->addWidget(label);
    }

    return legend;
}

void DutMonitorWidget::createSiteWidgets()
{
    // Clear existing widgets
    qDeleteAll(m_siteWidgets);
    m_siteWidgets.clear();

    // Clear all items from the layout
    QLayoutItem* item;
    while ((item = m_mainLayout->takeAt(0)) != nullptr) {
        delete item;
    }

    // Get all site info from DutManager
    QMap<QString, Services::DutManager::SiteInfo> siteInfoMap = m_dutManager->getAllSiteInfo();

    // Sort by siteIndex
    QList<Services::DutManager::SiteInfo> sortedSites = siteInfoMap.values();
    std::sort(sortedSites.begin(), sortedSites.end(),
              [](const Services::DutManager::SiteInfo& a, const Services::DutManager::SiteInfo& b) {
                  return a.siteIndex < b.siteIndex;
              });

    // Create site widgets
    int col = 0;
    int row = 0;

    for (const auto& siteInfo : sortedSites) {
        SiteWidget* siteWidget = new SiteWidget(siteInfo.siteAlias, siteInfo.socketCount, this);
        m_siteWidgets[siteInfo.siteAlias] = siteWidget;

        // Connect socket click signal
        connect(siteWidget, &SiteWidget::socketClicked, this, &DutMonitorWidget::onSocketClicked);

        // Set initial scan status
        siteWidget->setScanned(siteInfo.isScanned);

        // Add to grid layout
        m_mainLayout->addWidget(siteWidget, row, col);

        col++;
        if (col >= m_columnsPerRow) {
            col = 0;
            row++;
        }
    }

    // Add stretchable space
    m_mainLayout->setRowStretch(row + 1, 1);
    m_mainLayout->setColumnStretch(m_columnsPerRow, 1);
}

void DutMonitorWidget::onSiteConfigurationLoaded(int siteCount, int enabledSiteCount)
{
    Q_UNUSED(siteCount)
    Q_UNUSED(enabledSiteCount)

    qDebug() << "DutMonitorWidget: Site configuration loaded, creating widgets...";
    createSiteWidgets();
}

void DutMonitorWidget::onSiteChipPlacementChanged(int siteIndex, quint64 chipMask)
{
    Q_UNUSED(chipMask);
    // Find the corresponding site by siteIndex
    Services::DutManager::SiteInfo siteInfo = m_dutManager->getSiteInfoByIndex(siteIndex);
    if (!siteInfo.siteAlias.isEmpty()) {
        updateSiteDisplay(siteInfo.siteAlias,siteInfo.ip);
    }
}

void DutMonitorWidget::onSiteUpdated(const QString& siteAlias,const QString& ip)
{
    LOG_MODULE_INFO("DutMonitorWidget", QString("DutMonitorWidget::onSiteUpdated Updating site '%1' from scan data...").arg(siteAlias).toStdString());
    updateSiteDisplay(siteAlias,ip);
}

void DutMonitorWidget::updateSiteDisplay(const QString& siteAlias,const QString& ip)
{
    if (!m_siteWidgets.contains(siteAlias)) {
        return;
    }
    LOG_MODULE_INFO("DutMonitorWidget", QString("DutMonitorWidget::updateSiteDisplay Updating site '%1' from scan data...").arg(siteAlias).toStdString());
    Services::DutManager::SiteInfo siteInfo = m_dutManager->getSiteInfo(siteAlias);
    SiteWidget* siteWidget = m_siteWidgets[siteAlias];

    // Update socket status
    siteWidget->m_ip = ip;
    // Update scan status - set red border if the site has been scanned
    siteWidget->setScanned(siteInfo.isScanned);
}

void DutMonitorWidget::onDutStateChanged(const QString& dutId, Domain::Dut::State newState)
{
    // Get the site where the DUT is located
    auto dut = m_dutManager->getDut(dutId);
    if (dut) {
        QString site = dut->getCurrentSite();
        if (!site.isEmpty() && m_siteWidgets.contains(site)) {
            // Assuming dutId format is "site_socket", extract socket index
            QStringList parts = dutId.split('_');
            if (parts.size() >= 2) {
                bool ok;
                int socketIndex = parts.last().toInt(&ok);
                if (ok) {
                    m_siteWidgets[site]->updateSocketDutState(socketIndex, newState);
                }
            }
        }
    }
}

void DutMonitorWidget::onSocketClicked(const QString& siteAlias, int socketIndex)
{
    qDebug() << "Socket clicked:" << siteAlias << "Socket" << socketIndex;
    // Logic for handling socket clicks can be added here
    emit socketClicked(siteAlias, socketIndex);
}

} // namespace Presentation
