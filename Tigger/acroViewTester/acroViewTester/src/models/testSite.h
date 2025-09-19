#ifndef TESTSITE_H
#define TESTSITE_H

#include <QWidget>
#include <QPushButton>
#include <QVector>
#include "setting.h"
#include <QJsonObject>
#include <QJsonArray>
QT_BEGIN_NAMESPACE
namespace Ui { class TestSite; }
QT_END_NAMESPACE

class TestSite : public QWidget
{
    Q_OBJECT

public:
    enum Status {
        Idle,
        Enable,
        Testing,
        Completed,
        Failed,
        Disabled
    };

    struct ButtonState {
        bool isClicked;
        QPushButton* button;
        int row;
        int col;
    };


    explicit TestSite(int siteNumber, QWidget* parent = nullptr);
    ~TestSite();

    void setStatus(Status status);
    void updateTime(const QString& time);
    void initForm();
private:
    QString getStatusColor(Status status);
    void createStatusBlocks(int SITE_COLUMN, int SITE_ROW);
    void updateButtonAppearance(const ButtonState& state);
public:
    QJsonObject collectBlocksStatus();
private slots:
    void onPushButtonActionClicked();

signals:
    void startClicked();

private:
    Ui::TestSite* ui;
    int m_siteNumber;
    Status m_status;
    QVector<ButtonState> m_statusBlocks;
    bool isStarted; // 用于跟踪当前状态
    QString baseStyle;
    static const int SITE_COLUMN = 8;  // 状态块列数
    static const int SITE_ROW = 3;     // 状态块行数
};

#endif // TESTSITE_H 
