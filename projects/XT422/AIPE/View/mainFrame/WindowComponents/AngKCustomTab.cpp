#include "AngKCustomTab.h"
#include "AngKGlobalInstance.h"
#include <QStyleOptionTab>
#include <QPainter>
#include <QDebug>
#include <QFontDatabase>
AngKCustomTab::AngKCustomTab(QStyle* parent, int width, int height)
	: QProxyStyle(parent)
	, m_nWidth(width)
	, m_nHeight(height)
{

}

AngKCustomTab::~AngKCustomTab()
{
}

void AngKCustomTab::drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
	// 步骤一：调用父类的绘制控件函数
	QProxyStyle::drawControl(element, option, painter, widget);

	// 步骤二：重新绘制tab标签页文本
	if (element == CE_TabBarTabLabel) {
		if (const QStyleOptionTab* tab = qstyleoption_cast<const QStyleOptionTab*>(option)) {
			QRect controlRect = tab->rect;
			//if (tab->state & QStyle::State_Selected) {
			//	//save用以保护坐标，restore用来退出状态
			//	painter->save();
			//	painter->setBrush(QBrush(0x464646));
			//	//矩形
			//	//painter->drawRect(allRect.adjusted(0, 0, 0, 0));
			//	//带有弧线矩形

			//	painter->drawRect(tab->rect.adjusted(-1, -1, 1, 1));
			//	painter->restore();
			//}
			////hover状态
			//else if (tab->state & QStyle::State_MouseOver) {
			//	painter->save();
			//	painter->setBrush(QBrush(0x323232));
			//	painter->drawRect(controlRect.adjusted(-1, -1, 1, 1));
			//	painter->restore();
			//}
			//else {
			//	painter->save();
			//	painter->setBrush(QBrush(0x282828));
			//	painter->drawRect(controlRect.adjusted(-1, -1, 1, 1));
			//	painter->restore();
			//}

			painter->setRenderHints(QPainter::Antialiasing, false);   //关闭反走样
			QString tabText;
			if (m_orientation == Qt::Vertical)
			{
				// 将文本字符串换行处理
				for (int i = 0; i < tab->text.length(); i++)
				{
					tabText.append(tab->text.at(i));
					tabText.append('\n');
				}
				if (tabText.length() > 1)
					tabText = tabText.mid(0, tabText.length() - 1);
			}
			else
				tabText = tab->text;

			// 文本居中对齐
			QTextOption option;
			option.setAlignment(Qt::AlignCenter);
			QPen pen = painter->pen();
			int skinColor = 0x2f3133;
			if ((ViewMode)AngKGlobalInstance::GetSkinMode() == ViewMode::Dark) {
				skinColor = 0xffffff;
			}
			pen.setColor(QColor(skinColor));	// 文本颜色
			painter->setPen(pen);
			painter->drawText(controlRect, tabText, option);
		}
	}
}

void AngKCustomTab::drawItemText(QPainter* painter, const QRect& rect, int flags, const QPalette& pal, bool enabled, const QString& text, QPalette::ColorRole textRole) const
{
}

QSize AngKCustomTab::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& contentsSize, const QWidget* widget) const
{
	
	QSize s = QProxyStyle::sizeFromContents(type, option, contentsSize, widget);
	if (type == QStyle::CT_TabBarTab) {
		s.transpose();
		s.rwidth() = m_nWidth; // 设置每个tabBar中item的大小
		s.rheight() = m_nHeight;
	}
	return s;
}
