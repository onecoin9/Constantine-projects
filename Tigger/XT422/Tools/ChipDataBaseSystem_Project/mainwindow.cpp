#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "titleBar.h"
#include <QtDebug>
#include <QDesktopWidget>

MainWindow::MainWindow(QWidget *parent)
    : CFramelessWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setMinimumSize(830, 720);
    setResizeableAreaWidth(5);

    ui->titleWidget->setWinWidget(this);
    setTitleBar(ui->titleWidget);

    connect(ui->titleWidget, &TitleBar::sgnClose, this, &MainWindow::close);

    QDesktopWidget* m = QApplication::desktop();
    QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    this->move(desk_rect.width() / 2 - this->width() / 2 + desk_rect.left(), desk_rect.height() / 2 - this->height() / 2 + desk_rect.top());

    connect(ui->controlWidget, &DataBaseOper::sgnSetDBFile, ui->titleWidget, &TitleBar::onSlotGetDBFile);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showEvent(QShowEvent *ev)
{
    qDebug() << "444";

}

