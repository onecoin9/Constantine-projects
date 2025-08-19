#ifndef LOGDISPLAYWIDGET_H
#define LOGDISPLAYWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QMutex>
#include <QQueue>
#include "core/Logger.h"

namespace Presentation {

class LogDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LogDisplayWidget(QWidget *parent = nullptr);
    ~LogDisplayWidget();
    
    // 设置过滤器
    void setModuleFilter(const QStringList& modules);
    void setMinLogLevel(TesterFramework::LogLevel level);
    void clearFilters();
    
    // 控制方法
    void clearLog();
    void saveLogToFile(const QString& filePath);
    
    // 设置显示格式
    void setShowTimestamp(bool show);
    void setMaxLines(int maxLines);
    void setFilterVisible(bool visible);

protected:
    void setupUI();
    void connectSignals();
    void applyFilter();
    QString formatLogMessage(TesterFramework::LogLevel level, const QString& message) const;
    QString getLogLevelColor(TesterFramework::LogLevel level) const;
    QString extractModuleName(const QString& message) const;

private slots:
    void onLogMessage(TesterFramework::LogLevel level, const QString& message);
    void onModuleFilterChanged();
    void onLogLevelChanged();

private:
    // UI组件
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_filterLayout;
    QTextEdit* m_logTextEdit;
    QComboBox* m_moduleCombo;
    QComboBox* m_logLevelCombo;
    QCheckBox* m_showTimestampCheck;
    QLabel* m_statusLabel;
    
    // 过滤器配置
    QStringList m_moduleFilter;
    TesterFramework::LogLevel m_minLogLevel;
    
    // 配置
    bool m_showTimestamp;
    int m_maxLines;
    
    // 统计
    int m_totalLogCount;
    
    // 回调ID，用于注册spdlog sink
    QString m_sinkCallbackId;
};

} // namespace Presentation

#endif // LOGDISPLAYWIDGET_H 