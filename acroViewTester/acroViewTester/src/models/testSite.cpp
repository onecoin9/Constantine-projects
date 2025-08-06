#include "testSite.h"
#include "ui_testsite.h"
#include <QPushButton>
#include <QGridLayout>
#include <QGroupBox>
TestSite::TestSite(int siteNumber, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TestSite)
    , m_siteNumber(siteNumber)
    , m_status(Testing)
{
    ui->setupUi(this);
    ui->groupBox->setTitle(QString("Test Site %1").arg(m_siteNumber));

    //ui->labelSiteNum->setText(QString("%1").arg(m_siteNumber));
    initForm();
    
    connect(ui->pushButtonAction, &QPushButton::clicked, this, &TestSite::onPushButtonActionClicked);
}

TestSite::~TestSite()
{
    delete ui;
}

void TestSite::onPushButtonActionClicked()
{
    if (isStarted)
    {
        ui->pushButtonAction->setStyleSheet(baseStyle + "QPushButton { border-image: url(:/acroViewTester/qrc/pics/startG.png); }");
        isStarted = false;
        
    }
    else
    {
        ui->pushButtonAction->setStyleSheet(baseStyle + "QPushButton { border-image: url(:/acroViewTester/qrc/pics/pauseY.png); }");
        isStarted = true;
        emit startClicked();
        // 在这里实现开始的逻辑
    }
}

void TestSite::updateButtonAppearance(const ButtonState& buttonState) 
{
    Status status = Idle;
    QString color; 
    QString statusText;
    switch (status) {
    case Idle: statusText = "Idle"; break;
    case Enable:statusText = "Enable"; break;
    case Testing: statusText = "Testing"; break;
    case Completed: statusText = "Completed"; break;
    case Failed: statusText = "Failed"; break;
    case Disabled: statusText = "Disabled"; break;
    }

    if (buttonState.isClicked) 
    {
        status = Enable;
        color = getStatusColor(status);
        buttonState.button->setStyleSheet(QString("QPushButton { background-color: %1;  border: 1px solid black; }").arg(color));
    }
    else 
    {
        status = Idle;
        color = getStatusColor(status);
        buttonState.button->setStyleSheet(QString("QPushButton { background-color: %1;  border: 1px solid black; }").arg(color));
    }
}


void TestSite::createStatusBlocks(int siteRow, int siteColumn) {
    m_statusBlocks.clear();
    QSettings settings("AcroView", "acroViewTester");
    int SiteRows = settings.value("Grid/SiteRows", SITE_ROW).toInt();
    int SiteCols = settings.value("Grid/SiteCols", SITE_COLUMN).toInt();
    for (int i = 0; i < siteColumn * siteRow; ++i) {
        QPushButton* block = new QPushButton(this);
        if (SiteRows * SiteCols < 16) {
            block->setMinimumSize(30, 30);
        }
        else {
            block->setMinimumSize(15, 15);
        }
        block->setFlat(true);

        int row = i / siteColumn;
        int col = i % siteColumn;

        connect(block, &QPushButton::clicked, this, [this, i]() {
            // 切换按钮状态
            m_statusBlocks[i].isClicked = !m_statusBlocks[i].isClicked;
            // 更新按钮外观
            updateButtonAppearance(m_statusBlocks[i]);
            });

        ButtonState state = { false, block, row, col };
        updateButtonAppearance(state);
        block->setAutoFillBackground(true);

        QString tooltipText = QString("Tester %1\nStatus: %2\nTime: %3")
            .arg(i + 1)
            .arg("空闲")
            .arg("00:00:00");
        block->setToolTip(tooltipText);
        m_statusBlocks.append(state);
        ui->blocksLayout->addWidget(block, row, col);
    }
}

QJsonObject TestSite::collectBlocksStatus() 
{
    QJsonObject siteObject;
    //siteObject["siteNumber"] = m_siteNumber;
    QSettings settings("AcroView", "acroViewTester");
    int siteRows = settings.value("Grid/SiteRows", 2).toInt();
    int siteCols = settings.value("Grid/SiteCols", 2).toInt();
    siteObject["row"] = siteRows; // 设置 row
    siteObject["col"] = siteCols; // 设置 col
    QJsonArray socketsArray;
    for (int i = 0; i < m_statusBlocks.size(); ++i) {
        QJsonObject socketObject;
        int row = i / siteCols + 1; // 设置 row，从 1 开始
        int col = i % siteCols + 1; // 设置 col，从 1 开始
        socketObject["row"] = row;
        socketObject["col"] = col;
        socketObject["enabled"] = m_statusBlocks[i].isClicked;
        socketsArray.append(socketObject);
    }
    siteObject["sockets"] = socketsArray;

    return siteObject;
}

void TestSite::initForm()
{
    setStatus(Idle);
    baseStyle = R"(
        QPushButton {
            background-color: white;
            border: 1px solid #333;
            color: #333;
            border-radius: 4px;
            padding: 8px 16px;
            font-size: 14px;
            font-weight: 500;
            transition: all 0.3s ease;
        }
        QPushButton:hover {
            background-color: #f5f5f5;
            border-color: #666;
        }
        QPushButton:pressed {
            background-color: #e0e0e0;
            border-color: #444;
            transform: translateY(1px);
        }
        QPushButton:disabled {
            background-color: #f0f0f0;
            border-color: #999;
            color: #666;
            cursor: not-allowed;
        }
        QPushButton:focus {
            outline: none;
            box-shadow: 0 0 0 2px rgba(51, 51, 51, 0.3);
        }
    )";

    QSettings settings("AcroView", "acroViewTester");
    int baseRow = settings.value("Grid/BaseRows", SITE_ROW).toInt();
    int baseCol = settings.value("Grid/BaseCols", SITE_COLUMN).toInt();
    createStatusBlocks(baseRow, baseCol);
}


void TestSite::setStatus(Status status)
{
    m_status = status;
    QString color = getStatusColor(status);
    QString statusText;
    switch (status) {
    case Idle: statusText = "Idle"; break;
    case Testing: statusText = "Testing"; break;
    case Completed: statusText = "Completed"; break;
    case Failed: statusText = "Failed"; break;
    case Disabled: statusText = "Disabled"; break;
    }
    ui->statusLabel->setText(statusText);
    ui->statusLabel->setStyleSheet(QString("QLabel { background-color: %1; color: black; padding: 5px; }").arg(color));
    for (int i = 0; i < m_statusBlocks.size(); ++i) {
        QString tooltipText = QString("Tester %1\nStatus: %2\nTime: %3")
            .arg(i + 1)
            .arg(statusText)
            .arg(ui->timeLabel->text());
        m_statusBlocks[i].button->setToolTip(tooltipText);
    }
}

void TestSite::updateTime(const QString& time)
{
    ui->timeLabel->setText(time);
    for (int i = 0; i < m_statusBlocks.size(); ++i) {
        QString tooltipText = QString("Tester %1\nStatus: %2\nTime: %3")
            .arg(i + 1)
            .arg(ui->statusLabel->text())
            .arg(time);
        m_statusBlocks[i].button->setToolTip(tooltipText);
    }
}

QString TestSite::getStatusColor(Status status)
{
    switch (status) {
    case Idle: return "#808080";      // 灰色
    case Enable:return"#0000FF";
    case Testing: return "#FFFA65";   // 橙色
    case Completed: return "#008000"; // 绿色
    case Failed: return "#FF0000";    // 红色
    case Disabled: return "#A9A9A9";  // 深灰色
    default: return "#808080";
    }
}
