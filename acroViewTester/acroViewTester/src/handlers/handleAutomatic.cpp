#include "acroViewTester.h"
#include "AppLogger.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>

void acroViewTester::handlerTellDevReady()
{
    
}

QString acroViewTester::buildRdyInfoString()
{
    QString rdyInfo = QString("%1,%2,%3").arg(m_commandId).arg(m_chipCount).arg(m_tskFilePath);

    for (int enabled : m_trayEnabled) {
        rdyInfo += QString(",%1").arg(enabled);
    }

    rdyInfo += ",,,,,,,";

    return rdyInfo;
}

void showTemporaryMessage(QWidget* parent, const QString& message, int duration = 3000) {
    QLabel* floatingMsg = new QLabel(parent);
    floatingMsg->setText(message);
    floatingMsg->setStyleSheet(
        "QLabel { background-color: #2D2D30; color: white; padding: 10px; "
        "border-radius: 4px; font-weight: bold; }"
    );
    floatingMsg->setAlignment(Qt::AlignCenter);
    floatingMsg->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);

    QRect parentRect = parent->geometry();
    QSize msgSize = floatingMsg->sizeHint();
    int x = parentRect.x() + (parentRect.width() - msgSize.width()) / 2;
    int y = parentRect.y() + parentRect.height() - msgSize.height() - 20;

    floatingMsg->move(x, y);
    floatingMsg->show();

    QTimer::singleShot(duration, floatingMsg, &QLabel::deleteLater);
}

void acroViewTester::initAutoMatic()
{
    mAutomatic = ACAutomaticManager::instance()->GetAutomaticPlugin();
    mAutomatic->QuerySiteEn();

    m_commandId = 3;
    m_chipCount = 10;
    m_tskFilePath = "D:/Test/vauto/1.tsk";
    m_trayEnabled = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    ui.lineEditChipCount->setText(QString::number(m_chipCount));
    ui.lineEditTaskFilePath->setText(m_tskFilePath);
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<std::string>("std::string");
    connect(mAutomatic, &IAutomatic::chipIsInPlace, this, &acroViewTester::onChipIsInPlace);
    connect(mAutomatic, &IAutomatic::programResultBack, this, &acroViewTester::onProgramResultBack);

    connect(ui.btnSetTask, &QPushButton::clicked, this, &acroViewTester::onSetTaskClicked);
    connect(ui.btnBrowseTaskFile, &QPushButton::clicked, this, &acroViewTester::onBrowseTaskFileClicked);
    connect(ui.lineEditChipCount, &QLineEdit::textChanged, this, &acroViewTester::onChipCountChanged);
    connect(ui.btnTellDevReady, &QPushButton::clicked, this, &acroViewTester::onTellDevReadyClicked);
    connect(ui.btnSetDoneSite, &QPushButton::clicked, this, &acroViewTester::onSetDoneSiteClicked);
    connect(ui.btnQuerySiteMapping, &QPushButton::clicked, this, &acroViewTester::onbtnQuerySiteMappingClicked);


    initSiteStatusTable();
}

void acroViewTester::initSiteStatusTable()
{
    if (ui.tableWidgetSites) {
        ui.tableWidgetSites->setColumnCount(4);
        ui.tableWidgetSites->setHorizontalHeaderLabels({ "站点索引", "槽位使能", "站点SN", "状态" });
        ui.tableWidgetSites->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui.tableWidgetSites->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }

    ui.tableWidgetSiteMapping->setColumnCount(2);
    ui.tableWidgetSiteMapping->setHorizontalHeaderLabels({"站点索引", "站点别名"});
    ui.tableWidgetSiteMapping->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui.tableWidgetSiteMapping->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.tableWidgetSiteMapping->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.tableWidgetSiteMapping->setAlternatingRowColors(true);
    
    ui.tableWidgetSiteMapping->setStyleSheet(
        "QTableWidget {"
        "   border: 1px solid #d3d3d3;"
        "   gridline-color: #f0f0f0;"
        "   selection-background-color: #0078d7;"
        "}"
        "QTableWidget::item {"
        "   padding: 4px;"
        "   border: none;"
        "}"
        "QHeaderView::section {"
        "   background-color: #f0f0f0;"
        "   padding: 4px;"
        "   border: 1px solid #d3d3d3;"
        "   font-weight: bold;"
        "}"
    );
}

void acroViewTester::onSetTaskClicked()
{
    if (m_tskFilePath.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择 TSK 文件路径!");
        return;
    }

    QString rdyInfo = buildRdyInfoString();
    qDebug() << "设置任务，RdyInfo:" << rdyInfo;

    int result = mAutomatic->SetTask(rdyInfo.toStdString());

    if (result == 0) {
        QString successMsg = QString("任务设置成功 [%1]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        ui.textBrowser->append(successMsg);
    }
    else {
        QString errorMsg = getSetTaskErrorMessage(result);
        QMessageBox::warning(this, "任务设置失败", errorMsg);

        QString failMsg = QString("任务设置失败: %1 [%2]").arg(errorMsg).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        ui.textBrowser->append(failMsg);
    }

    QString logInfo = QString(
        "发送任务信息:\n"
        "- 命令ID: %1\n"
        "- 芯片数量: %2\n"
        "- TSK文件: %3\n"
        "- 完整RdyInfo: %4\n"
        "- 返回结果: %5\n"
    ).arg(m_commandId)
        .arg(m_chipCount)
        .arg(m_tskFilePath)
        .arg(rdyInfo)
        .arg(result == 0 ? "成功" : QString("错误(%1: %2)").arg(result).arg(getSetTaskErrorMessage(result)));

    ui.textBrowser->append(logInfo);
}

QString acroViewTester::getSetTaskErrorMessage(int errorCode)
{
    switch (errorCode) {
    case 0:
        return "成功";
    case 1:
        return "错误: 烧录机未Ready";
    case 2:
        return "错误: 设备自动运行中";
    case 3:
        return "错误: 前次通信处理进行中";
    case 4:
        return "错误: 当前模式为非一键烧录模式";
    case 5:
        return "错误: 任务数据不正确";
    case 6:
        return "错误: 托盘/卷带指定位置不正确";
    case 7:
        return "错误: 烧录器未初始化成功（CONN）";
    case 8:
        return "错误: Lic验证失败";
    default:
        return QString("未知错误: %1").arg(errorCode);
    }
}

void acroViewTester::onBrowseTaskFileClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("选择 TSK 文件"),
        QDir::homePath(),
        tr("TSK 文件 (*.tsk);;所有文件 (*.*)")
    );

    if (!filePath.isEmpty()) {
        ui.lineEditTaskFilePath->setText(filePath);
        m_tskFilePath = filePath;

        qDebug() << "已选择 TSK 文件路径:" << m_tskFilePath;
    }
}

void acroViewTester::onChipCountChanged(const QString& text)
{
    bool ok;
    int count = text.toInt(&ok);

    if (ok && count > 0) {
        m_chipCount = count;
    }
    else {
        if (!text.isEmpty()) {
            ui.lineEditChipCount->setText(QString::number(m_chipCount));
        }
    }
}

void acroViewTester::onTellDevReadyClicked()
{
    int adapterNum = ui.spinBoxAdapterNum->value();
    
    QString site1SKTRdy = ui.lineEditSite1SKTRdy->text();
    QString site1Alias = ui.lineEditSite1Alias->text();
    int site1EnvRdy = ui.checkBoxSite1EnvRdy->isChecked() ? 1 : 0;
    QString site1SN = ui.lineEditSite1SN->text();
    
    QString site2SKTRdy = ui.lineEditSite2SKTRdy->text();
    QString site2Alias = ui.lineEditSite2Alias->text();
    int site2EnvRdy = ui.checkBoxSite2EnvRdy->isChecked() ? 1 : 0;
    QString site2SN = ui.lineEditSite2SN->text();
    
    QJsonObject projInfoObj;
    projInfoObj["AdapterNum"] = adapterNum;
    
    QJsonObject site1Obj;
    site1Obj["SKTRdy"] = site1SKTRdy;
    site1Obj["SiteAlias"] = site1Alias;
    site1Obj["SiteEnvRdy"] = site1EnvRdy;
    site1Obj["SiteSN"] = site1SN;
    
    QJsonObject site2Obj;
    site2Obj["SKTRdy"] = site2SKTRdy;
    site2Obj["SiteAlias"] = site2Alias;
    site2Obj["SiteEnvRdy"] = site2EnvRdy;
    site2Obj["SiteSN"] = site2SN;
    
    QJsonArray siteReadyArray;
    siteReadyArray.append(site1Obj);
    siteReadyArray.append(site2Obj);
    
    QJsonObject rootObj;
    rootObj["ProjInfo"] = projInfoObj;
    rootObj["SiteReady"] = siteReadyArray;

    QJsonDocument doc(rootObj);
    QString jsonStr = doc.toJson(QJsonDocument::Compact);
    
    mAutomatic->TellDevReady(jsonStr.toStdString());
    
    showTemporaryMessage(this, "已发送设备就绪通知", 3000);
    ui.textBrowser->append("发送设备就绪通知: " + jsonStr + " [" + 
                          QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "]");
}
void acroViewTester::onSetDoneSiteClicked()
{
    int siteIndex = 1;
    uint8_t results[16] = { 1 };
    uint64_t mask = 0xFFFF;

    int ret = mAutomatic->SetDoneSite(siteIndex, results, mask);

    if (ret == 0) {
        showTemporaryMessage(this, "站点完成设置成功", 3000);
        ui.textBrowser->append("站点完成设置成功 [" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "]");
    }
    else {
        showTemporaryMessage(this, "站点完成设置失败", 3000);
        ui.textBrowser->append("站点完成设置失败，错误码: " + QString::number(ret) + " [" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "]");
    }
}

void acroViewTester::onbtnQuerySiteMappingClicked()
{
    std::map<int, std::string> siteMap;
    mAutomatic->GetSiteMap(siteMap);
LOG.info("开始处理数据");
    ui.tableWidgetSiteMapping->clearContents();
    ui.tableWidgetSiteMapping->setRowCount(0);

    ui.textBrowser->append("站点映射信息: [" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "]");

    if (siteMap.empty()) {
        ui.textBrowser->append(" - 没有站点映射信息");
        showTemporaryMessage(this, "没有可用的站点映射信息", 3000);
        
        int row = ui.tableWidgetSiteMapping->rowCount();
        ui.tableWidgetSiteMapping->insertRow(row);
        
        QTableWidgetItem* noDataItem = new QTableWidgetItem("无可用数据");
        noDataItem->setTextAlignment(Qt::AlignCenter);
        ui.tableWidgetSiteMapping->setItem(row, 0, noDataItem);
        ui.tableWidgetSiteMapping->setSpan(row, 0, 1, 2);
        
        return;
    }

    for (const auto& pair : siteMap) {
        int row = ui.tableWidgetSiteMapping->rowCount();
        ui.tableWidgetSiteMapping->insertRow(row);
        
        QTableWidgetItem* indexItem = new QTableWidgetItem(QString::number(pair.first));
        indexItem->setTextAlignment(Qt::AlignCenter);
        ui.tableWidgetSiteMapping->setItem(row, 0, indexItem);
        
        QTableWidgetItem* aliasItem = new QTableWidgetItem(QString::fromStdString(pair.second));
        aliasItem->setTextAlignment(Qt::AlignCenter);
        ui.tableWidgetSiteMapping->setItem(row, 1, aliasItem);
        
        ui.textBrowser->append(QString(" - 站点 %1: %2")
            .arg(pair.first)
            .arg(QString::fromStdString(pair.second)));
    }

    showTemporaryMessage(this, QString("成功获取 %1 个站点映射").arg(siteMap.size()), 3000);
}

void acroViewTester::onChipIsInPlace(int siteIndex, uint32_t slotEn, std::string strSiteSn)
{
    qDebug() << "收到芯片放置信号 - 站点:" << siteIndex
        << "槽位使能:" << QString("0x%1").arg(slotEn, 8, 16, QChar('0'))
        << "站点SN:" << QString::fromStdString(strSiteSn);

    QString message = QString("芯片已放置 - 站点: %1, 槽位: 0x%2, SN: %3")
        .arg(siteIndex)
        .arg(slotEn, 8, 16, QChar('0'))
        .arg(QString::fromStdString(strSiteSn));

    ui.textBrowser->append(message);
    showTemporaryMessage(this, "芯片已放置", 3000);

    updateSiteStatus(siteIndex, slotEn, strSiteSn);
}


void acroViewTester::printMessage(QString errStr)
{
    
    QString formattedMessage = QString("错误详情: %1 [%2]")
        .arg(errStr)
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    
    ui.textBrowser->append(formattedMessage);

    if (errStr.length() > 100) {
      
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("编程过程中发生错误");
        msgBox.setDetailedText(errStr);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }
}


void acroViewTester::onProgramResultBack(int result, int errCode, const char* errStr)
{
    QString status = result == 0 ? "成功" : "失败";
    QString errString = errStr ? QString::fromUtf8(errStr) : "未知错误";

    QString message = QString("编程结果: %1 (错误码: %2) [%3]")
        .arg(status)
        .arg(errCode)
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    qDebug() << message;
    ui.textBrowser->append(message);

    if (result == 0) {
        showTemporaryMessage(this, "编程成功", 3000);
    }
    else {
    
        showTemporaryMessage(this, "编程失败: " + QString::number(errCode), 3000);

        if (errStr && strlen(errStr) > 0) {
            printMessage(errString);
        }
        else {
  
            printMessage("错误码: " + QString::number(errCode));
        }

    }
}

void acroViewTester::updateSiteStatus(int siteIndex, uint32_t slotEn, const std::string& strSiteSn)
{
    if (ui.tableWidgetSites) {
        int row = findSiteRow(siteIndex);
        if (row >= 0) {
            ui.tableWidgetSites->item(row, 0)->setText(QString::number(siteIndex));
            ui.tableWidgetSites->item(row, 1)->setText(QString("0x%1").arg(slotEn, 8, 16, QChar('0')));
            ui.tableWidgetSites->item(row, 2)->setText(QString::fromStdString(strSiteSn));
            ui.tableWidgetSites->item(row, 3)->setText("就绪");
        }
    }
}

int acroViewTester::findSiteRow(int siteIndex)
{
    if (!ui.tableWidgetSites) {
        return -1;
    }

    for (int i = 0; i < ui.tableWidgetSites->rowCount(); i++) {
        QTableWidgetItem* item = ui.tableWidgetSites->item(i, 0);
        if (item && item->text().toInt() == siteIndex) {
            return i;
        }
    }

    int row = ui.tableWidgetSites->rowCount();
    ui.tableWidgetSites->insertRow(row);

    ui.tableWidgetSites->setItem(row, 0, new QTableWidgetItem(QString::number(siteIndex)));
    ui.tableWidgetSites->setItem(row, 1, new QTableWidgetItem(""));
    ui.tableWidgetSites->setItem(row, 2, new QTableWidgetItem(""));
    ui.tableWidgetSites->setItem(row, 3, new QTableWidgetItem(""));

    return row;
}

void acroViewTester::startProgramming(int siteIndex, uint32_t slotEn, const std::string& strSiteSn)
{
    qDebug() << "自动开始对站点" << siteIndex << "进行编程";

    QString message = QString("正在对站点 %1 (SN: %2) 进行编程... [%3]")
        .arg(siteIndex)
        .arg(QString::fromStdString(strSiteSn))
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    ui.textBrowser->append(message);

    if (ui.tableWidgetSites) {
        int row = findSiteRow(siteIndex);
        if (row >= 0) {
            ui.tableWidgetSites->item(row, 3)->setText("编程中");
        }
    }
}