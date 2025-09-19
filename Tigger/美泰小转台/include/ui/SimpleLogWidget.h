#ifndef SIMPLELOGWIDGET_H
#define SIMPLELOGWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMutex>
#include <QQueue>
#include <QTimer>
#include <QCheckBox>
#include "core/Logger.h"

namespace Presentation {

class SimpleLogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SimpleLogWidget(QWidget *parent = nullptr);
    ~SimpleLogWidget();
    
    void setMaxLines(int maxLines);

private slots:
    void onLogMessage(TesterFramework::LogLevel level, const QString& message);
    void onLogLevelChanged();
    void onScrollToBottomChanged();

private:
    void setupUI();
    void connectSignals();
    QString formatLogMessage(TesterFramework::LogLevel level, const QString& message) const;
    QString getLogLevelColor(TesterFramework::LogLevel level) const;
    void trimLogIfNeeded();

private:
    // UI组件
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_toolbarLayout;
    QTextEdit* m_logTextEdit;
    QComboBox* m_logLevelCombo;
    QCheckBox* m_autoScrollCheckBox;
    
    // 配置
    TesterFramework::LogLevel m_minLogLevel;
    int m_maxLines;
    bool m_autoScroll;
    
    // 用于线程安全的日志队列
    QMutex m_logMutex;
    QQueue<QPair<TesterFramework::LogLevel, QString>> m_pendingLogs;
    QTimer* m_updateTimer;
    size_t m_loggerSinkId;  // Logger sink的唯一标识符
};

} // namespace Presentation

#endif // SIMPLELOGWIDGET_H