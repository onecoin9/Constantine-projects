#ifndef DUTMONITORWIDGET_H
#define DUTMONITORWIDGET_H

#include <QWidget>
#include <QMap>
#include <QList>
#include <memory>
#include "domain/Dut.h"
#include <QFrame>

// Forward declarations
namespace Ui { class DutMonitorWidget; }
namespace Services { class DutManager; }
class QTableWidget;
class QTableWidgetItem;
class QScrollArea;
class QSpinBox;
class QLabel;
class QGridLayout;
class QFrame;
class QEvent;
class QMouseEvent;

namespace Presentation {

// Visual component representing a single Socket
class SocketCell : public QFrame
{
    Q_OBJECT
public:
    explicit SocketCell(int socketIndex, QWidget *parent = nullptr);
    void setDutState(Domain::Dut::State state);

signals:
    void clicked(int socketIndex);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QString getDutStateString(Domain::Dut::State state) const;

    int m_socketIndex;
    QLabel* m_statusLabel;
    bool m_occupied;
    bool m_enabled;
    Domain::Dut::State m_dutState;
};

// Visual component representing a single Site
class SiteWidget : public QFrame
{
    Q_OBJECT
public:
    explicit SiteWidget(const QString& siteName, int socketCount, QWidget *parent = nullptr);
    void updateSocketDutState(int socketIndex, Domain::Dut::State state);
    void setScanned(bool scanned);  // New: set scan status

signals:
    void socketClicked(const QString& siteName, int socketIndex);

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
public:
    QString m_ip;
    QString m_siteName;
private:
    int m_socketCount;
    QList<SocketCell*> m_sockets;
    QGridLayout* m_socketGrid;
};

class DutMonitorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DutMonitorWidget(QWidget *parent = nullptr);
    ~DutMonitorWidget();

    // Set DutManager and start listening
    void setDutManager(Services::DutManager* dutManager);

    // Set the number of sites displayed per row
    void setColumnsPerRow(int columns);

signals:
    void socketClicked(const QString& siteAlias, int socketIndex);

private slots:
    void onSiteConfigurationLoaded(int siteCount, int enabledSiteCount);
    void onSiteChipPlacementChanged(int siteIndex, quint64 chipMask);
    void onSiteUpdated(const QString& siteAlias,const QString& ip);
    void onDutStateChanged(const QString& dutId, Domain::Dut::State newState);
    void onSocketClicked(const QString& siteAlias, int socketIndex);

private:
    void setupUi();
    void createSiteWidgets();
    void updateSiteDisplay(const QString& siteAlias,const QString& ip);
    QWidget* createLegendWidget();

    Services::DutManager* m_dutManager;
    QMap<QString, SiteWidget*> m_siteWidgets;  // siteAlias -> SiteWidget
    QGridLayout* m_mainLayout;
    QScrollArea* m_scrollArea;
    QWidget* m_scrollWidget;
    int m_columnsPerRow;
};

} // namespace Presentation

#endif // DUTMONITORWIDGET_H
