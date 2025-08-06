// DirectTriggerMenu.h
#ifndef DIRECTTRIGGERMENU_H
#define DIRECTTRIGGERMENU_H
#include <QMenu>

class DirectTriggerMenu : public QMenu
{
    Q_OBJECT
public:
    explicit DirectTriggerMenu(const QString& title, QWidget* parent = nullptr);

protected:
    bool event(QEvent* event) override; 

protected:
    void mousePressEvent(QMouseEvent* event) override;

signals:
    void menuTriggered(); // 自定义触发信号
};
#endif