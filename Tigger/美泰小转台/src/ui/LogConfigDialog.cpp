#include "ui/LogConfigDialog.h"
#include <QTableWidget>
#include <QVBoxLayout>
#include <QComboBox>
#include <QHeaderView>
#include <QCheckBox>
#include "core/Logger.h"

using namespace TesterFramework;

namespace Presentation {

LogConfigDialog::LogConfigDialog(QWidget *parent)
    : QDialog(parent)
    , m_table(new QTableWidget(this))
{
    setWindowTitle("模块日志级别配置");
    resize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(this);
    m_showLogCheckBox = new QCheckBox("显示日志窗口", this);
    layout->addWidget(m_showLogCheckBox);
    layout->addWidget(m_table);

    // 初始状态根据父窗口的可见性
    if (parentWidget()) {
        m_showLogCheckBox->setChecked(true);
    }

    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels(QStringList() << "模块" << "级别");
    m_table->horizontalHeader()->setStretchLastSection(true);

    loadData();

    connect(m_showLogCheckBox, &QCheckBox::toggled, this, &LogConfigDialog::onShowLogToggled);
}

void LogConfigDialog::loadData()
{
    m_modules = Logger::getInstance().getRegisteredLoggerNames();
    m_table->setRowCount(static_cast<int>(m_modules.size()));

    int row = 0;
    for (const auto &name : m_modules) {
        QTableWidgetItem *nameItem = new QTableWidgetItem(QString::fromStdString(name));
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(row, 0, nameItem);

        QComboBox *combo = new QComboBox();
        combo->addItems({"Trace", "Debug", "Info", "Warning", "Error", "Critical"});
        LogLevel lvl = Logger::getInstance().getModuleLevel(name);
        combo->setCurrentIndex(static_cast<int>(lvl));
        m_table->setCellWidget(row, 1, combo);

        // 连接级别变更
        connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [name](int idx){
            Logger::getInstance().setModuleLevel(name, static_cast<LogLevel>(idx));
        });
        ++row;
    }
}

void LogConfigDialog::onShowLogToggled(bool checked)
{
    emit logViewVisibilityChanged(checked);
}

} // namespace Presentation 