#ifndef LOGCONFIGDIALOG_H
#define LOGCONFIGDIALOG_H

#include <QDialog>
#include <vector>
#include <string>

class QTableWidget;
class QCheckBox;

namespace Presentation {

class LogConfigDialog : public QDialog {
    Q_OBJECT
public:
    explicit LogConfigDialog(QWidget *parent = nullptr);

signals:
    // 通知 MainWindow 切换日志窗口显示
    void logViewVisibilityChanged(bool visible);

private slots:
    void onShowLogToggled(bool checked);

private:
    void loadData();

private:
    QTableWidget *m_table;
    std::vector<std::string> m_modules;
    QCheckBox *m_showLogCheckBox;
};

} // namespace Presentation

#endif // LOGCONFIGDIALOG_H 