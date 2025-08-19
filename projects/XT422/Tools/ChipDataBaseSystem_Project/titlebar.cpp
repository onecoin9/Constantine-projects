#include "titlebar.h"
#include "ui_titlebar.h"

#include <QMouseEvent>
#include <QDebug>
TitleBar::TitleBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TitleBar),
    m_window(Q_NULLPTR)
{
    ui->setupUi(this);

    ui->titleText->setText(tr("ChipDB-Sys"));
    ui->minButton->setText(tr("min"));
    ui->maxButton->setText(tr("max"));
    ui->closeButton->setText(tr("close"));

    connect(ui->closeButton, &QPushButton::clicked, this, &TitleBar::sgnClose);
    connect(ui->minButton, &QPushButton::clicked, this, &TitleBar::showSmall);
    connect(ui->maxButton, &QPushButton::clicked, this, &TitleBar::showMaxRestore);
}

TitleBar::~TitleBar()
{
    delete ui;
}

void TitleBar::setWinWidget(QWidget *wgt)
{
    m_window = wgt;
}

void TitleBar::mousePressEvent(QMouseEvent *event)
{
    m_startPos = event->globalPos();
    m_clickPos = mapTo(this, event->pos());

    QWidget::mousePressEvent(event);
}

void TitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (Qt::WindowMaximized == m_window->windowState())
        return;

    m_window->move(event->globalPos() - m_clickPos);

    QWidget::mouseMoveEvent(event);
}

void TitleBar::showSmall()
{
    m_window->showMinimized();
}

void TitleBar::showMaxRestore()
{
    if (Qt::WindowMaximized == m_window->windowState())
    {
        //m_maximize->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/maxButton.svg"));
        m_window->showNormal();
    }
    else
    {
        //m_maximize->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/normalButton.svg"));
        m_window->showMaximized();
    }
}

void TitleBar::onSlotGetDBFile(QString strFile)
{
    if(strFile.isEmpty())
        return;

    ui->dbFileText->setText(strFile);
}
