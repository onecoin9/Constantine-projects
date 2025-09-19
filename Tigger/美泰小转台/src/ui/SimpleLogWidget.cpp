#include "ui/SimpleLogWidget.h"
#include <QCheckBox>
#include <QScrollBar>
#include <QTextCursor>
#include <QDebug>

namespace Presentation {

SimpleLogWidget::SimpleLogWidget(QWidget *parent)
    : QWidget(parent)
    , m_minLogLevel(TesterFramework::LogLevel::Debug)
    , m_maxLines(1000)
    , m_autoScroll(true)
{
    setupUI();
    connectSignals();
    
    // 连接到Logger系统
    m_loggerSinkId = TesterFramework::Logger::getInstance().addSink([this](TesterFramework::LogLevel level, const std::string& message) {
        QMutexLocker locker(&m_logMutex);
        m_pendingLogs.enqueue(qMakePair(level, QString::fromStdString(message)));
    });
    
    // 定时器用于批量更新UI（避免UI线程阻塞）
    m_updateTimer = new QTimer(this);

    m_updateTimer->setInterval(200); 
    connect(m_updateTimer, &QTimer::timeout, this, [this]() {
        // 批量获取待处理的日志，最小化锁持有时间
        QList<QPair<TesterFramework::LogLevel, QString>> logsToProcess;

        if (!m_logMutex.tryLock()) {
            return;
        }
        
        if (m_pendingLogs.isEmpty()) {
            m_logMutex.unlock();
            return; 
        }

        // 每次最多处理5条日志
        const int batchSize = 5;
        logsToProcess.reserve(batchSize);
        
        for (int i = 0; i < batchSize && !m_pendingLogs.isEmpty(); ++i) {
            logsToProcess.append(m_pendingLogs.dequeue());
        }
        
        // 如果还有未处理的日志，动态调整定时器间隔
        if (!m_pendingLogs.isEmpty()) {
            m_updateTimer->setInterval(100); // 加快处理频率
        } else {
            m_updateTimer->setInterval(200); // 恢复正常间隔
        }

        m_logMutex.unlock();
        
        // 在锁外批量处理日志，避免UI阻塞
        for (const auto& logItem : logsToProcess) {
            onLogMessage(logItem.first, logItem.second);
        }
    });
    m_updateTimer->start();
}

SimpleLogWidget::~SimpleLogWidget()
{
    if (m_updateTimer) {
        m_updateTimer->stop();
    }
    
    // 从Logger中移除sink，防止访问已销毁的对象
    TesterFramework::Logger::getInstance().removeSink(m_loggerSinkId);
}

void SimpleLogWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(5, 5, 5, 5);
    m_mainLayout->setSpacing(5);
    
    // 创建工具栏
    m_toolbarLayout = new QHBoxLayout();
    
    // 日志级别过滤
    QLabel* levelLabel = new QLabel("日志级别:", this);
    m_logLevelCombo = new QComboBox(this);
    m_logLevelCombo->addItem("全部", static_cast<int>(TesterFramework::LogLevel::Trace));
    m_logLevelCombo->addItem("调试", static_cast<int>(TesterFramework::LogLevel::Debug));
    m_logLevelCombo->addItem("信息", static_cast<int>(TesterFramework::LogLevel::Info));
    m_logLevelCombo->addItem("警告", static_cast<int>(TesterFramework::LogLevel::Warning));
    m_logLevelCombo->addItem("错误", static_cast<int>(TesterFramework::LogLevel::Error));
    m_logLevelCombo->addItem("严重", static_cast<int>(TesterFramework::LogLevel::Critical));
    m_logLevelCombo->setCurrentIndex(1); // 默认显示Debug级别及以上
    
    // 自动滚动
    m_autoScrollCheckBox = new QCheckBox("自动滚动", this);
    m_autoScrollCheckBox->setChecked(m_autoScroll);
    
    m_toolbarLayout->addWidget(levelLabel);
    m_toolbarLayout->addWidget(m_logLevelCombo);
    m_toolbarLayout->addWidget(m_autoScrollCheckBox);
    m_toolbarLayout->addStretch();
    
    // 创建日志显示区域
    m_logTextEdit = new QTextEdit(this);
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setFont(QFont("Consolas", 9));
    m_logTextEdit->setStyleSheet(
        "QTextEdit {"
        "    background-color: #2b2b2b;"
        "    color: #ffffff;"
        "    border: 1px solid #555555;"
        "}"
    );
    
    m_mainLayout->addLayout(m_toolbarLayout);
    m_mainLayout->addWidget(m_logTextEdit);
}

void SimpleLogWidget::connectSignals()
{
    connect(m_logLevelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &SimpleLogWidget::onLogLevelChanged);
    connect(m_autoScrollCheckBox, &QCheckBox::toggled, this, &SimpleLogWidget::onScrollToBottomChanged);
}

void SimpleLogWidget::onLogMessage(TesterFramework::LogLevel level, const QString& message)
{
    // 过滤日志级别
    if (level < m_minLogLevel) {
        return;
    }
    
    // 格式化消息
    QString formattedMessage = formatLogMessage(level, message);
    
    // 添加到文本框
    QTextCursor cursor = m_logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertHtml(formattedMessage + "<br>");
    
    // 日志计数已移除
    
    // 限制行数
    trimLogIfNeeded();
    
    // 自动滚动到底部
    if (m_autoScroll) {
        QScrollBar* scrollBar = m_logTextEdit->verticalScrollBar();
        scrollBar->setValue(scrollBar->maximum());
    }
}


void SimpleLogWidget::onLogLevelChanged()
{
    int levelValue = m_logLevelCombo->currentData().toInt();
    m_minLogLevel = static_cast<TesterFramework::LogLevel>(levelValue);
}

void SimpleLogWidget::onScrollToBottomChanged()
{
    m_autoScroll = m_autoScrollCheckBox->isChecked();
}


void SimpleLogWidget::setMaxLines(int maxLines)
{
    m_maxLines = maxLines;
}

QString SimpleLogWidget::formatLogMessage(TesterFramework::LogLevel level, const QString& message) const
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString levelText;
    QString color = getLogLevelColor(level);
    
    switch (level) {
        case TesterFramework::LogLevel::Trace:
            levelText = "TRACE";
            break;
        case TesterFramework::LogLevel::Debug:
            levelText = "DEBUG";
            break;
        case TesterFramework::LogLevel::Info:
            levelText = "INFO";
            break;
        case TesterFramework::LogLevel::Warning:
            levelText = "WARN";
            break;
        case TesterFramework::LogLevel::Error:
            levelText = "ERROR";
            break;
        case TesterFramework::LogLevel::Critical:
            levelText = "CRIT";
            break;
    }
    
    return QString("<span style='color: #888888;'>[%1]</span> "
                   "<span style='color: %2; font-weight: bold;'>[%3]</span> "
                   "<span style='color: #ffffff;'>%4</span>")
           .arg(timestamp)
           .arg(color)
           .arg(levelText)
           .arg(message.toHtmlEscaped());
}

QString SimpleLogWidget::getLogLevelColor(TesterFramework::LogLevel level) const
{
    switch (level) {
        case TesterFramework::LogLevel::Trace:
            return "#888888";
        case TesterFramework::LogLevel::Debug:
            return "#00aaff";
        case TesterFramework::LogLevel::Info:
            return "#00ff00";
        case TesterFramework::LogLevel::Warning:
            return "#ffaa00";
        case TesterFramework::LogLevel::Error:
            return "#ff4444";
        case TesterFramework::LogLevel::Critical:
            return "#ff0000";
        default:
            return "#ffffff";
    }
}

void SimpleLogWidget::trimLogIfNeeded()
{
    QTextDocument* doc = m_logTextEdit->document();
    if (doc->blockCount() > m_maxLines) {
        QTextCursor cursor(doc);
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, doc->blockCount() - m_maxLines);
        cursor.removeSelectedText();
    }
}




} // namespace Presentation