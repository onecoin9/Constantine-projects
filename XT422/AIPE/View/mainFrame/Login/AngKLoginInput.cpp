#include "AngKLoginInput.h"
#include "ui_AngKLoginInput.h"
#include "../View/GlobalInit/StyleInit.h"
#include <QPainter>
#include <QPainterPath>
#include <QKeyEvent>
AngKLoginInput::AngKLoginInput(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::AngKLoginInput();
	ui->setupUi(this);

	InitText();
	setLoginPath("D:/ProgramSpace/AG06Client/AG06Client/Skin/Light/Background/mario.jpg");

	connect(ui->loginInputButton, &QPushButton::clicked, this, &AngKLoginInput::onSlotClickLogin);

	installEventFilter(this);

	this->setObjectName("AngKLoginInput");
	QT_SET_STYLE_SHEET(objectName());
}

AngKLoginInput::~AngKLoginInput()
{
	delete ui;
}

void AngKLoginInput::InitText()
{
	ui->signupLabel->setOpenExternalLinks(true);//设置为true才能打开网页
	ui->signupLabel->setText(QStringLiteral("<a style='color: #4896f0; text-decoration: none; font-size:20px' href = https://www.acroview.com><u>Sign up</u>"));
	ui->signupLabel->setAlignment(Qt::AlignCenter);

	ui->loginInputButton->setText(tr("Log in"));

	ui->comboBox->addItem("User", 1);
	ui->comboBox->addItem("Developer", 2);
	ui->comboBox->setCurrentIndex(1);

	//ui->IDEdit->setFocus();
	ui->IDEdit->setText("Developer");
	ui->pwdEdit->setText("123456");

	connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int nCurSelect) {
		if (nCurSelect == 0){
			ui->IDEdit->setText("User");
		}
		else {
			ui->IDEdit->setText("Developer");
		}
		});
}

void AngKLoginInput::setLoginPath(QString strPath)
{
	QPixmap pixmap(strPath);
	pixmap = pixmap.scaled(ui->personLabel->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	int width = ui->personLabel->size().width();
	int height = ui->personLabel->size().height();
	QPixmap image(width, height);
	image.fill(Qt::transparent);
	QPainterPath painterPath;
	painterPath.addEllipse(0, 0, width, height);
	QPainter painter(&image);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
	painter.setClipPath(painterPath);
	painter.drawPixmap(0, 0, width, height, pixmap);
	//绘制到label
	ui->personLabel->setPixmap(image);

	//另外一种方法，也可绘制圆头像，保留学习
#if 0
	QPixmap pixmap(strPath);
	pixmap = pixmap.scaled(ui->personLabel->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
	// 创建圆形mask，使用QPainter绘制
	QPixmap roundPixmap(ui->personLabel->size());
	roundPixmap.fill(Qt::transparent);
	QPainter painter(&roundPixmap);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setPen(Qt::NoPen);
	painter.setBrush(Qt::white);
	painter.drawEllipse(roundPixmap.rect());
	// 将原始图片按照mask绘制到圆形pixmap上
	painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
	painter.drawPixmap(0, 0, pixmap);
	// 将圆形pixmap设置为label的背景图像
	ui->personLabel->setPixmap(roundPixmap);
#endif

}

bool AngKLoginInput::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
			// 处理 Enter 键事件
			emit sgnRequstLogin(ui->comboBox->currentData().value<int>(), ui->IDEdit->text(), ui->pwdEdit->text());
			return true; // 事件已处理，不再传递
		}
	}
	// 其他事件传递给父类处理
	return QWidget::eventFilter(obj, event);
}

void AngKLoginInput::onSlotClickLogin()
{
	//TODO
	//登录请求判断，起码得有http。先空着登录成功之后
	emit sgnRequstLogin(ui->comboBox->currentData().value<int>(), ui->IDEdit->text(), ui->pwdEdit->text());
}