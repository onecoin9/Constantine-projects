#include "AngKWindowTitleBar.h"
#include <QPixmap>
#include <QStyle>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QApplication>
#include <QComboBox>
#include <QLineEdit>
#include <QStandardItemModel>
#include <QAbstractItemView>
#include <QPainterPath>
#include <QPainter>
#include "../View/GlobalInit/StyleInit.h"
#include "AngKTitleImg.h"
#include "AngKGlobalInstance.h"
#include "GlobalDefine.h"

AngKWindowTitleBar::AngKWindowTitleBar(QWidget* parent, QWidget* window, QWidget* shadowContainerWidget, bool canResize)
	: QWidget(parent)
	, m_window(window)
	, m_shadowContainerWidget(shadowContainerWidget)
	, m_oldContentsMargin(10, 10, 10, 10)
	, m_canResize(canResize)
	, m_titleTextLabel(nullptr)
	, m_versionTextLabel(nullptr)
{
	// 不继承父组件的背景色
	setAutoFillBackground(true);

	InitButton();
	InitComboBox();

	m_titleLabel = new QLabel(this);
	m_titleLabel->setText("");
	m_window->setWindowTitle("AProg.exe");
	m_titleLabel->setPixmap(QApplication::windowIcon().pixmap(QApplication::windowIcon().actualSize(QSize(25, 21))));
	
	m_titleTextLabel = new QLabel(this);
	m_titleTextLabel->setText(tr("Log in"));
	m_titleTextLabel->setObjectName("titleTextLabel");

	m_versionTextLabel = new QLabel(this);
	m_versionTextLabel->setText("AP9900-Acroview Build Version " + AngKGlobalInstance::ReadValue("Version", "BuildVer").toString());
	m_versionTextLabel->setObjectName("versionTextLabel");

	//m_personImg = new AngKTitleImg(this);
	//PersonImgChange("");
	//PersonNameChange("John");
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setFocusPolicy(Qt::FocusPolicy::NoFocus);
	
	m_maximize->setEnabled(m_canResize);
	m_minimize->setEnabled(m_canResize);

	m_maximize->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	m_minimize->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	m_close->setFocusPolicy(Qt::FocusPolicy::NoFocus);

	this->setObjectName("AngKWindowTitleBar");
	QT_SET_STYLE_SHEET(objectName());
}

AngKWindowTitleBar::~AngKWindowTitleBar()
{
}

void AngKWindowTitleBar::showSmall()
{
	m_window->showMinimized();
}

void AngKWindowTitleBar::showMaxRestore()
{
	if (!m_canResize)
		return;

	if (Qt::WindowMaximized == m_window->windowState()) 
	{
		emit sgnIntroPropetry("minWgtFont");
		m_maximize->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/maxButton.svg"));
		m_shadowContainerWidget->setContentsMargins(m_oldContentsMargin);
		m_window->showNormal();
	}
	else 
	{
		emit sgnIntroPropetry("maxWgtFont");
		m_maximize->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/normalButton.svg"));
		m_oldContentsMargin = m_shadowContainerWidget->contentsMargins();
		m_shadowContainerWidget->setContentsMargins(0, 0, 0, 0);
		m_window->showMaximized();
	}
}

void AngKWindowTitleBar::InitButton()
{
	m_minimize = new QPushButton(this);
	m_maximize = new QPushButton(this);
	m_close = new QPushButton(this);

	m_minimize->setObjectName("m_minimize");
	m_maximize->setObjectName("m_maximize");
	m_close->setObjectName("m_close");

	m_close->setFixedSize(35, 35);
	m_minimize->setFixedSize(35, 35);
	m_maximize->setFixedSize(35, 35);
	m_minimize->setFlat(true);
	m_maximize->setFlat(true);
	m_close->setFlat(true);
	m_minimize->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/minButton.svg"));
	m_maximize->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/maxButton.svg"));
	m_close->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/closeButton.svg"));

	connect(m_close, SIGNAL(clicked()), m_window, SLOT(close()));
	connect(m_minimize, SIGNAL(clicked()), this, SLOT(showSmall()));
	connect(m_maximize, SIGNAL(clicked()), this, SLOT(showMaxRestore()));
}

void AngKWindowTitleBar::InitComboBox()
{
	m_comboBox = new QComboBox(this);
	m_comboBox->setFixedSize(108, 30);
	m_comboBox->addItem("AP9900");

	QLineEdit* lineEdit = m_comboBox->lineEdit();
	lineEdit = new QLineEdit(m_comboBox);
	lineEdit->setObjectName("comboEdit");
	QString stylyStr = "border-top-left-radius: 8px;border-bottom-left-radius: 8px; background: transparent;";
	lineEdit->setStyleSheet(stylyStr);
	lineEdit->setReadOnly(true);
	lineEdit->setAlignment(Qt::AlignCenter);
	lineEdit->setFocusPolicy(Qt::NoFocus);
	QLineEdit::connect(lineEdit, &QLineEdit::selectionChanged, [=] {
		lineEdit->deselect();
		m_comboBox->showPopup();
		});

	m_comboBox->setLineEdit(lineEdit);
	QStandardItemModel* model = static_cast<QStandardItemModel*>(m_comboBox->view()->model());
	if (model)
	{
		for (int i = 0; i < model->rowCount(); i++)
		{
			if (model->item(i))
				model->item(i)->setTextAlignment(Qt::AlignCenter);
		}
	}
}

void AngKWindowTitleBar::PersonImgChange(QString sPath)
{
	QString path = "D:/ProgramSpace/AG06Client/AG06Client/Skin/Dark/Background/testPerson.jpg";
	QPixmap pixmap(path);
	pixmap = pixmap.scaled(m_personImg->imgLabel()->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation); 
	int width = m_personImg->imgLabel()->size().width();
	int height = m_personImg->imgLabel()->size().height();
	QPixmap image(width, height);
	image.fill(Qt::transparent); 
	QPainterPath painterPath;
	painterPath.addEllipse(0, 0, width, height);
	QPainter painter(&image);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
	painter.setClipPath(painterPath);
	painter.drawPixmap(0, 0, width, height, pixmap);
	//绘制到label
	m_personImg->imgLabel()->setPixmap(image);
}

void AngKWindowTitleBar::PersonNameChange(QString sName)
{
	m_personImg->nameLabel()->setText(sName);
}

void AngKWindowTitleBar::setContentsMargins(QMargins margins)
{
	m_oldContentsMargin = margins;
	QWidget::setContentsMargins(margins);
	layout()->setContentsMargins(margins);
}

void AngKWindowTitleBar::setText(QString title)
{
	m_titleLabel->setText(title);
}

QString AngKWindowTitleBar::text() const
{
	return m_titleLabel->text();
}

void AngKWindowTitleBar::setShowTitleComponent(bool bShow)
{
	if (bShow)
	{
		//m_personImg->show();
		m_comboBox->show();
		m_titleTextLabel->hide();
	}
	else
	{
		//m_personImg->hide();
		m_comboBox->hide();
	}
}

void AngKWindowTitleBar::mousePressEvent(QMouseEvent* me)
{
	m_startPos = me->globalPos();
	m_clickPos = mapTo(m_window, me->pos());

	QWidget::mousePressEvent(me);
}

void AngKWindowTitleBar::mouseMoveEvent(QMouseEvent* me)
{
	if (Qt::WindowMaximized == m_window->windowState())
		return;

	m_window->move(me->globalPos() - m_clickPos);

	QWidget::mouseMoveEvent(me);
}

void AngKWindowTitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		showMaxRestore();
	}
	QWidget::mouseDoubleClickEvent(event);
}

void AngKWindowTitleBar::layoutTitleBar()
{
	QHBoxLayout* bgLayout = new QHBoxLayout(this);
	QWidget* bgWidget = new QWidget();
	bgWidget->setFixedHeight(50);
	bgWidget->setObjectName("bgWidget");
	QHBoxLayout* hbox = new QHBoxLayout(this);

	hbox->addWidget(m_titleLabel);
	hbox->addWidget(m_versionTextLabel);
	hbox->addWidget(m_titleTextLabel);
	hbox->addWidget(m_comboBox);
	//hbox->addWidget(m_personImg);

	//QSpacerItem* space = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
	//m_personImg->setFixedWidth(90);
	//hbox->addItem(space);
	hbox->addWidget(m_minimize);
	hbox->addWidget(m_maximize);
	hbox->addWidget(m_close);

	hbox->insertStretch(3, 500);
	hbox->setSpacing(0);
	hbox->insertSpacing(1, 20);
	bgWidget->setLayout(hbox);
	bgWidget->setContentsMargins(0, 0, 0, 0);
	bgLayout->addWidget(bgWidget);
	this->setLayout(bgLayout);
}

void AngKWindowTitleBar::windowStateChanged()
{
	if (Qt::WindowMaximized == m_window->windowState())
	{
		m_shadowContainerWidget->setContentsMargins(0, 0, 0, 0);
		m_maximize->setStyleSheet(m_restoreStyleSheet);
	}
	else
	{
		m_shadowContainerWidget->setContentsMargins(m_oldContentsMargin);
		m_maximize->setStyleSheet(m_maxSheet);
	}
}
