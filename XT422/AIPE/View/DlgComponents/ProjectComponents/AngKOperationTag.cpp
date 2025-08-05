#include "AngKOperationTag.h"
#include "ui_AngKOperationTag.h"
#include "../View/GlobalInit/StyleInit.h"
#include "AngKGlobalInstance.h"
#include <QtGui/QDrag>
#include <QtCore/QMimeData>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

AngKOperationTag::AngKOperationTag(QWidget *parent)
	: QWidget(parent)
	, m_bDrag(false)
	, m_bRightClick(false)
	, m_operType(OperationTagType::None)
{
	ui = new Ui::AngKOperationTag();
	ui->setupUi(this);

	ui->bgWidget->setProperty("customProperty", "fix");
	ui->symbolText->setProperty("customProperty", "fix");
	InitSymbol();
	setAcceptDrops(true);

	//TODO comboBox内容后续保存在map中

	this->setObjectName("AngKOperationTag");
	QT_SET_STYLE_SHEET(objectName());
}

AngKOperationTag::~AngKOperationTag()
{
	delete ui;
}

void AngKOperationTag::setLabelText(QString text)
{
	ui->symbolText->setText(text);
	m_operFlowText = text;
}

void AngKOperationTag::setSymbolLabel(QString symbol)
{
	QPixmap* pix = new QPixmap(symbol);
	ui->symbolLabel->setPixmap(pix->scaled(ui->symbolLabel->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void AngKOperationTag::ClearText()
{
	ui->symbolText->clear();
	ui->symbolLabel->clear();
	m_operType = OperationTagType::None;
}

void AngKOperationTag::setBgWidgetProperty(QString value, bool bgWidget)
{
	if (!bgWidget)
	{
		ui->bgWidget->setProperty("customProperty", value);
	}
	ui->symbolText->setProperty("customProperty", value);

	this->setStyle(QApplication::style());
}

void AngKOperationTag::setDrag(bool bDrag)
{
	m_bDrag = bDrag;
}

void AngKOperationTag::setRightClick(bool bRight)
{
	m_bRightClick = bRight;
}

void AngKOperationTag::InitWidget(QString text, QString symbol, OperationTagType operType)
{
	setLabelText(text);
	setSymbolLabel(symbol);
	setCurOperateType(operType);
}

void AngKOperationTag::setCurOperateType(OperationTagType operType)
{
	m_operType = operType;
}

QString AngKOperationTag::getLabelText()
{
	return ui->symbolText->text();
}

QString AngKOperationTag::getFlowLabelText()
{
	return m_operFlowText;
}

void AngKOperationTag::setFlowLabelText(QString flowText)
{
	m_operFlowText = flowText;
}

OperationTagType AngKOperationTag::GetOperationTagType()
{
	return m_operType;
}

void AngKOperationTag::PaintSymbolLabel(QString operStr)
{
	setLabelText(operStr);
	setSymbolLabel(m_mapOperSymbolPath[operStr]);
}

void AngKOperationTag::mousePressEvent(QMouseEvent* event)
{
	if (m_bRightClick && event->buttons() == Qt::RightButton )
	{
		ClearText();
		emit sgnDropSymbolType(this, (int)OperationTagType::None);
		return;
	}

	m_startPos = event->pos();
}

void AngKOperationTag::mouseMoveEvent(QMouseEvent* event)
{
	// 只允许左键拖动
	if (!(event->buttons() == Qt::LeftButton))
	{
		return;
	}

	if (!m_bDrag)
		return;

	// 移动一定距离后才算是开始拖动
	if ((event->pos() - m_startPos).manhattanLength() < QApplication::startDragDistance())
	{
		return;
	}

	QByteArray byteData;
	// 创建数据
	QDrag* drag = new QDrag(this);
	QMimeData* mimeData = new QMimeData();
	mimeData->setData(ui->symbolText->text(), byteData);
	mimeData->setText(ui->symbolText->text());
	drag->setMimeData(mimeData);
	// 设置拖动时的图像显示
	QPixmap drag_img = this->grab();
	QPainter painter(&drag_img);
	painter.setOpacity(0.5); // 设置透明度为 50%
	//painter.drawText(QRectF(20, 0, 120, 18), ui->symbolText->text(), QTextOption(Qt::AlignVCenter));
	drag->setPixmap(drag_img);
	// 启动拖放 start a drag
	Qt::DropAction result = drag->exec(Qt::CopyAction | Qt::MoveAction);
	// 检查操作有没有成功，有没有被取消
	if (Qt::IgnoreAction != result)
	{
		qDebug() << QString::fromLocal8Bit("成功完成拖拽");
		drag->setPixmap(QPixmap());
	}
}

void AngKOperationTag::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat(OPERAT_ERASE)
		|| event->mimeData()->hasFormat(OPERAT_BLANK)
		|| event->mimeData()->hasFormat(OPERAT_PROGRAM)
		|| event->mimeData()->hasFormat(OPERAT_VERIFY)
		|| event->mimeData()->hasFormat(OPERAT_SECURE)
		|| event->mimeData()->hasFormat(OPERAT_READ))
	{
		qDebug() << QString::fromLocal8Bit("成功完成拖拽到显示区");
		if(!m_bDrag && m_bRightClick)
			event->acceptProposedAction();
	}

	QWidget::dragEnterEvent(event);
}

void AngKOperationTag::dropEvent(QDropEvent* event)
{
	qDebug() << QString::fromLocal8Bit("接收到了拖拽事件 %1...").arg(event->mimeData()->text());

	QString dropText = event->mimeData()->text();

	if (OPERAT_ERASE == dropText)
	{
		m_operType = OperationTagType::Erase;
	}
	else if (OPERAT_BLANK == dropText)
	{
		m_operType = OperationTagType::Blank;
	}
	else if (OPERAT_PROGRAM == dropText)
	{
		m_operType = OperationTagType::Program;
	}
	else if (OPERAT_VERIFY == dropText)
	{
		m_operType = OperationTagType::Verify;
	}
	else if (OPERAT_SECURE == dropText)
	{
		m_operType = OperationTagType::Secure;
	}
	else if (OPERAT_READ == dropText)
	{
		m_operType = OperationTagType::Read;
	}

	ui->symbolText->setText(dropText);
	setSymbolLabel(m_mapOperSymbolPath[dropText]);

	emit sgnDropSymbolType(this, (int)m_operType);

	QWidget::dropEvent(event);
}

void AngKOperationTag::InitSymbol()
{
	bool bDark = AngKGlobalInstance::ReadValue("Skin", "mode").toInt() == (int)ViewMode::Dark;
	QString relativeSkinPath = Utils::AngKPathResolve::localRelativeSkinPath() + "/PushButton/";
	if (!bDark)
	{
		m_mapOperSymbolPath[OPERAT_ERASE] = relativeSkinPath + "eraseButtonblack.svg";
		m_mapOperSymbolPath[OPERAT_BLANK] = relativeSkinPath + "blankButtonblack.svg";
		m_mapOperSymbolPath[OPERAT_PROGRAM] = relativeSkinPath + "programButtonblack.svg";
		m_mapOperSymbolPath[OPERAT_VERIFY] = relativeSkinPath + "verifyButtonblack.svg";
		m_mapOperSymbolPath[OPERAT_SECURE] = relativeSkinPath + "secureButtonblack.svg";
		m_mapOperSymbolPath[OPERAT_READ] = relativeSkinPath + "readButtonblack.svg";
	}
	else
	{
		m_mapOperSymbolPath[OPERAT_ERASE] = relativeSkinPath + "eraseButtonHover.svg";
		m_mapOperSymbolPath[OPERAT_BLANK] = relativeSkinPath + "blankButtonHover.svg";
		m_mapOperSymbolPath[OPERAT_PROGRAM] = relativeSkinPath + "programButtonHover.svg";
		m_mapOperSymbolPath[OPERAT_VERIFY] = relativeSkinPath + "verifyButtonHover.svg";
		m_mapOperSymbolPath[OPERAT_SECURE] = relativeSkinPath + "secureButtonHover.svg";
		m_mapOperSymbolPath[OPERAT_READ] = relativeSkinPath + "readButtonHover.svg";
	}
}

