#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>

namespace Ui {
class TitleBar;
}

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBar(QWidget *parent = nullptr);
    ~TitleBar();

    void setWinWidget(QWidget* wgt);
protected:
    virtual void mousePressEvent(QMouseEvent* event);

    virtual void mouseMoveEvent(QMouseEvent* event);

signals:
    void sgnClose();

public slots:
    void showSmall();
    void showMaxRestore();
    void onSlotGetDBFile(QString);
private:
    Ui::TitleBar *ui;
    QPoint m_startPos;
    QPoint m_clickPos;
    QWidget* m_window;
};

#endif // TITLEBAR_H
