// DirectTriggerMenu.cpp
#include "directTriggerMenu.h"
#include <QMouseEvent>
DirectTriggerMenu::DirectTriggerMenu(const QString& title, QWidget* parent)
    : QMenu(title, parent)
{
}

void DirectTriggerMenu::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) 
    {
        emit menuTriggered();
        event->accept();
    }
    else {
        QMenu::mousePressEvent(event);
    }
}
bool DirectTriggerMenu::event(QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        if(me->button() == Qt::LeftButton) {
            this->hide();
            emit menuTriggered();
            return true; 
        }
    }
    return QMenu::event(event);
}