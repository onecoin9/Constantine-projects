#include "AngKProgressDelegate.h"
#include <QApplication>
#include <QProgressBar>
#include <QPainter>

AngKProgressDelegate::AngKProgressDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
	, nProgressIdx(0)
{
}

AngKProgressDelegate::~AngKProgressDelegate()
{
}

void AngKProgressDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (index.column() == nProgressIdx) // 假设进度条要插入的列为第二列
	{
		int processed = index.data(Qt::UserRole + eIV_Value).toInt();//设置当前进度
		QString processType = index.data(Qt::UserRole + eIV_ProgressType).toString();//当前进度状态

		QStyleOptionProgressBar processBar;
		processBar.rect = QRect(option.rect.left(), option.rect.top() + (option.rect.height() / 3), option.rect.width(), option.rect.height() / 2);
		processBar.minimum = 0;
		processBar.maximum = 100;
		processBar.progress = processed;
		processBar.text = QString::number(processed) + "%";
		processBar.textAlignment = Qt::AlignBottom;
		processBar.textVisible = true;

		//QApplication::style()->drawControl(QStyle::CE_ProgressBar, &processBar, painter);

		// 自定义绘制进度条
		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, true);

		QColor bgColor;
		if (index.row() % 2 == 0) {
			bgColor = QColor("#ffffff");
		}
		else {
			bgColor = QColor("#f5f5f5");
		}

		// 绘制背景
		painter->setBrush(bgColor); // 背景色
		painter->fillRect(option.rect, QBrush(bgColor)); // 使用 fillRect 而不是 drawRect

		// 绘制进度条
		QRect progressRect(option.rect.left(), option.rect.top() + (option.rect.height() / 3), option.rect.width() * processed / 100, option.rect.height() / 2);
		//painter->setBrush(QColor("#b3d9ff")); // 进度条的前景色（填充颜色）
		//painter->fillRect(progressRect, QBrush(QColor("#b3d9ff")));
		if (processType == tr("Success")) {
			painter->setBrush(QColor("#20dfb2"));
			painter->fillRect(progressRect, QBrush(QColor("#20dfb2")));
		}
		else if (processType == tr("Failed")) {
			painter->setBrush(QColor("#ff6d8a"));
			painter->fillRect(progressRect, QBrush(QColor("#ff6d8a")));
		}
		else {
			painter->setBrush(m_color);
			painter->fillRect(progressRect, QBrush(m_color));
		}


		// 绘制文本
		painter->setPen(QColor(Qt::black)); // 文本颜色
		painter->drawText(option.rect, Qt::AlignCenter, processBar.text);

		painter->restore();
	}
	else
	{
		QStyledItemDelegate::paint(painter, option, index);
	}
}

void AngKProgressDelegate::setProgressIndex(int nProgress)
{
	nProgressIdx = nProgress;
}

void AngKProgressDelegate::UpdateProgressColor(QColor progColor)
{
	m_color = progColor;
}
