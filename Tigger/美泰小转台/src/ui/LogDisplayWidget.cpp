#include "ui/LogDisplayWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QScrollBar>
#include <QApplication>
#include <QDateTime>
#include <QTextCursor>
#include <QDebug>
namespace Presentation {

LogDisplayWidget::LogDisplayWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_filterLayout(nullptr)
    , m_logTextEdit(nullptr)
    , m_moduleCombo(nullptr)
    , m_logLevelCombo(nullptr)
    , m_showTimestampCheck(nullptr)
    , m_statusLabel(nullptr)
    , m_minLogLevel(TesterFramework::LogLevel::Info)
    , m_maxLines(1000)
    , m_totalLogCount(0)
{
    setupUI();
    connectSignals();
    
    // 注册spdlog sink，并保存ID用于析构时注销
    m_sinkId = TesterFramework::Logger::getInstance().addSink([this](TesterFramework::LogLevel level, const std::string& message) {
        // 在GUI线程中处理日志
        QMetaObject::invokeMethod(this, [this, level, message]() {
            onLogMessage(level, QString::fromStdString(message));
        }, Qt::QueuedConnection);
    });
}

LogDisplayWidget::~LogDisplayWidget()
{
    // 移除spdlog sink，防止对象销毁后回调继续访问无效this
    if (m_sinkId != 0) {
        TesterFramework::Logger::getInstance().removeSink(m_sinkId);
        m_sinkId = 0;
    }
}

void LogDisplayWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // 过滤器区域
    QGroupBox* filterGroup = new QGroupBox("日志过滤器", this);
    m_filterLayout = new QHBoxLayout(filterGroup);
    
    // 模块过滤
    m_filterLayout->addWidget(new QLabel("模块:"));
    m_moduleCombo = new QComboBox();
    m_moduleCombo->addItem("全部", "");
    m_moduleCombo->setEditable(true);
    m_filterLayout->addWidget(m_moduleCombo);
    
    // 日志级别过滤
    m_filterLayout->addWidget(new QLabel("最低级别:"));
    m_logLevelCombo = new QComboBox();
    m_logLevelCombo->addItem("Trace", static_cast<int>(TesterFramework::LogLevel::Trace));
    m_logLevelCombo->addItem("Debug", static_cast<int>(TesterFramework::LogLevel::Debug));
    m_logLevelCombo->addItem("Info", static_cast<int>(TesterFramework::LogLevel::Info));
    m_logLevelCombo->addItem("Warning", static_cast<int>(TesterFramework::LogLevel::Warning));
    m_logLevelCombo->addItem("Error", static_cast<int>(TesterFramework::LogLevel::Error));
    m_logLevelCombo->setCurrentIndex(2); // 默认Info级别
    m_filterLayout->addWidget(m_logLevelCombo);
    
    
    // 控制按钮
    QPushButton* clearBtn = new QPushButton("清空日志");
    QPushButton* saveBtn = new QPushButton("保存日志");
    m_filterLayout->addWidget(clearBtn);
    m_filterLayout->addWidget(saveBtn);
    
    connect(clearBtn, &QPushButton::clicked, this, &LogDisplayWidget::clearLog);
    connect(saveBtn, &QPushButton::clicked, this, [this]() {
        // 使用异步方式调用文件对话框，避免在主线程中可能触发的死锁问题
        QTimer::singleShot(0, this, [this]() {
            try {
                QString filePath = QFileDialog::getSaveFileName(this, "保存日志", 
                    QString("log_%1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
                    "Text files (*.txt)");
                if (!filePath.isEmpty()) {
                    saveLogToFile(filePath);
                    LOG_INFO(QString("日志保存路径: %1").arg(filePath).toStdString());
                } else {
                    LOG_INFO("用户取消了日志保存");
                }
            } catch (const std::exception& e) {
                LOG_ERROR(QString("保存日志时发生异常: %1").arg(e.what()).toStdString());
                QMessageBox::critical(this, "错误", QString("保存文件时发生错误: %1").arg(e.what()));
            } catch (...) {
                LOG_ERROR("保存日志时发生未知异常");
                QMessageBox::critical(this, "错误", "保存文件时发生未知错误");
            }
        });
    });
    
    m_filterLayout->addStretch();
    
    // 状态标签
    m_statusLabel = new QLabel("日志: 0");
    m_filterLayout->addWidget(m_statusLabel);
    
    m_mainLayout->addWidget(filterGroup);
    
    // 日志显示区域
    m_logTextEdit = new QTextEdit(this);
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setFont(QFont("Consolas", 9));
    m_logTextEdit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_logTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_logTextEdit->document()->setMaximumBlockCount(m_maxLines);
    m_mainLayout->addWidget(m_logTextEdit);
}

void LogDisplayWidget::connectSignals()
{
    connect(m_moduleCombo, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, &LogDisplayWidget::onModuleFilterChanged);
    connect(m_logLevelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LogDisplayWidget::onLogLevelChanged);
    connect(m_showTimestampCheck, &QCheckBox::toggled, this, [this](bool checked) {
        m_showTimestamp = checked;
    });
}

void LogDisplayWidget::setModuleFilter(const QStringList& modules)
{
    m_moduleFilter = modules;
    applyFilter();
}

void LogDisplayWidget::setMinLogLevel(TesterFramework::LogLevel level)
{
    m_minLogLevel = level;
    int index = m_logLevelCombo->findData(static_cast<int>(level));
    if (index >= 0) {
        m_logLevelCombo->setCurrentIndex(index);
    }
    applyFilter();
}

void LogDisplayWidget::clearFilters()
{
    m_moduleCombo->setCurrentIndex(0);
    m_logLevelCombo->setCurrentIndex(2);
    m_moduleFilter.clear();
    applyFilter();
}

void LogDisplayWidget::clearLog()
{
    m_logTextEdit->clear();
    m_totalLogCount = 0;
    m_statusLabel->setText("日志: 0");
}

void LogDisplayWidget::saveLogToFile(const QString& filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << m_logTextEdit->toPlainText();
        QMessageBox::information(this, "保存成功", QString("日志已保存到: %1").arg(filePath));
    } else {
        QMessageBox::warning(this, "保存失败", QString("无法写入文件: %1").arg(filePath));
    }
}


void LogDisplayWidget::setMaxLines(int maxLines)
{
    m_maxLines = maxLines;
    m_logTextEdit->document()->setMaximumBlockCount(maxLines);
}

void LogDisplayWidget::setFilterVisible(bool visible)
{
    // 找到过滤器组
    for (int i = 0; i < m_mainLayout->count(); ++i) {
        QLayoutItem* item = m_mainLayout->itemAt(i);
        if (item && item->widget()) {
            QGroupBox* groupBox = qobject_cast<QGroupBox*>(item->widget());
            if (groupBox && groupBox->title() == "日志过滤器") {
                groupBox->setVisible(visible);
                break;
            }
        }
    }
}

void LogDisplayWidget::applyFilter()
{
    // 过滤器逻辑在接收日志时应用，这里不需要额外处理
}

QString LogDisplayWidget::formatLogMessage(TesterFramework::LogLevel level, const QString& message) const
{
    QString formatted;
    
    // 消息内容（移除spdlog已经添加的级别标识，避免重复）
    QString cleanMessage = message;
    // 移除形如[LEVEL]的标识，因为我们会重新添加格式化的级别（不区分大小写）
    QRegularExpression levelPattern(R"(\[(?:TRACE|DEBUG|INFO|WARN|ERROR|CRIT)\]\s*)", QRegularExpression::CaseInsensitiveOption);
    cleanMessage.remove(levelPattern);
    
    // 添加格式化的日志级别
    QString levelColor = getLogLevelColor(level);
    QString levelText;
    switch (level) {
        case TesterFramework::LogLevel::Trace: levelText = "TRACE"; break;
        case TesterFramework::LogLevel::Debug: levelText = "DEBUG"; break;
        case TesterFramework::LogLevel::Info: levelText = "INFO"; break;
        case TesterFramework::LogLevel::Warning: levelText = "WARN"; break;
        case TesterFramework::LogLevel::Error: levelText = "ERROR"; break;
        case TesterFramework::LogLevel::Critical: levelText = "CRIT"; break;
        default: levelText = "UNKNOWN"; break;
    }
    formatted += QString("<span style='color: %1; font-weight: bold;'>[%2]</span> ")
                .arg(levelColor).arg(levelText);
    
    formatted += cleanMessage;
    
    return formatted;
}

QString LogDisplayWidget::getLogLevelColor(TesterFramework::LogLevel level) const
{
    switch (level) {
        case TesterFramework::LogLevel::Trace: return "#808080";
        case TesterFramework::LogLevel::Debug: return "#0080FF";
        case TesterFramework::LogLevel::Info: return "#008000";
        case TesterFramework::LogLevel::Warning: return "#FF8000";
        case TesterFramework::LogLevel::Error: return "#FF0000";
        case TesterFramework::LogLevel::Critical: return "#800080";
        default: return "#000000";
    }
}

void LogDisplayWidget::onLogMessage(TesterFramework::LogLevel level, const QString& message)
{
    // 应用级别过滤
    if (level < m_minLogLevel) {
        return;
    }
    
    // 提取模块名
    QString moduleName = extractModuleName(message);
    
    // 调试信息：打印到控制台以便调试
    qDebug() << "LogDisplayWidget收到日志:" << "Level:" << static_cast<int>(level) << "Module:" << moduleName << "Message:" << message;
    
    // 应用模块过滤
    if (!m_moduleFilter.isEmpty()) {
        bool moduleMatches = false;
        for (const QString& module : m_moduleFilter) {
            if (module.isEmpty() || module == "全部") {
                moduleMatches = true;
                break;
            }
            // 改进的模块匹配逻辑
            if (moduleName.contains(module, Qt::CaseInsensitive) || 
                message.contains(module, Qt::CaseInsensitive)) {
                moduleMatches = true;
                break;
            }
        }
        if (!moduleMatches) {
            qDebug() << "模块过滤器过滤掉了日志:" << moduleName;
            return;
        }
    }
    
    // 动态更新模块下拉框
    if (!moduleName.isEmpty()) {
        int existingIndex = m_moduleCombo->findData(moduleName);
        if (existingIndex < 0) {
            m_moduleCombo->addItem(moduleName, moduleName);
        }
    }
    
    // 格式化并显示日志
    QString formattedMessage = formatLogMessage(level, message);
    m_logTextEdit->moveCursor(QTextCursor::End);
    m_logTextEdit->insertHtml(formattedMessage + "<br>");
    
    m_totalLogCount++;
    m_statusLabel->setText(QString("日志: %1").arg(m_totalLogCount));
    
    // 自动滚动到底部
    QScrollBar* scrollBar = m_logTextEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

QString LogDisplayWidget::extractModuleName(const QString& message) const
{
    // 尝试多种模块名提取方式
    
    // 1. 匹配新的spdlog格式: [LEVEL] [ModuleName] message
    QRegularExpression moduleRegex1(R"(\[.*?\]\s*\[([^\]]+)\])");
    QRegularExpressionMatch match1 = moduleRegex1.match(message);
    if (match1.hasMatch()) {
        QString moduleName = match1.captured(1);
        // 过滤掉不是真正模块名的内容
        if (moduleName != "TRACE" && moduleName != "DEBUG" && moduleName != "INFO" && 
            moduleName != "WARN" && moduleName != "ERROR" && moduleName != "CRIT") {
            return moduleName;
        }
    }
    
    // 2. 匹配形如 [level] moduleName: message 的格式
    QRegularExpression moduleRegex2(R"(\[.*?\]\s*([A-Za-z0-9_]+):\s)");
    QRegularExpressionMatch match2 = moduleRegex2.match(message);
    if (match2.hasMatch()) {
        return match2.captured(1);
    }
    
    // 3. 如果消息包含特定的模块标识符，直接返回
    if (message.contains("BurnDevice")) return "BurnDevice";
    if (message.contains("TcpChannel")) return "TcpChannel";
    if (message.contains("SUCCESS")) return "SUCCESS";
    if (message.contains("ERROR")) return "ERROR";
    if (message.contains("CONFIG")) return "CONFIG";
    if (message.contains("DeviceManager")) return "DeviceManager";
    if (message.contains("WorkflowManager")) return "WorkflowManager";
    
    return QString();
}

void LogDisplayWidget::onModuleFilterChanged()
{
    QString currentModule = m_moduleCombo->currentText().trimmed();
    if (currentModule.isEmpty() || currentModule == "全部") {
        m_moduleFilter.clear();
    } else {
        m_moduleFilter = QStringList() << currentModule;
    }
    applyFilter();
}

void LogDisplayWidget::onLogLevelChanged()
{
    int levelValue = m_logLevelCombo->currentData().toInt();
    m_minLogLevel = static_cast<TesterFramework::LogLevel>(levelValue);
    applyFilter();
}

} // namespace Presentation
