#include "AngKHelper.h"
#include <QtCore>
#include <QtGui>
#include <QApplication>

void CString::trim(char ch)
{
	while (startsWith(ch))
	{
		remove(0, 1);
	}
	//
	while (endsWith(ch))
	{
		remove(length() - 1, 1);
	}
}

void CString::trimLeft()
{
	while (!isEmpty())
	{
		if (begin()->isSpace())
		{
			remove(0, 1);
		}
		else
		{
			break;
		}
	}
}

void CString::trimRight()
{
	while (!isEmpty())
	{
		if (at(length() - 1).isSpace())
		{
			remove(length() - 1, 1);
		}
		else
		{
			break;
		}
	}
}

void CString::format(QString strFormat, ...)
{
	CString strFormat2 = strFormat;
	strFormat2.replace("%s", "%ls");
	//
	va_list argList;
	va_start(argList, strFormat);
	vsprintf(strFormat2.toUtf8(), argList);
	va_end(argList);
}

void CString::setAt(int index, QChar ch)
{
	replace(index, 1, ch);
}

int CString::findOneOf(const CString& strFind) const
{
	std::set<QChar> chars;
	for (int i = 0; i < strFind.length(); i++)
	{
		chars.insert(strFind[i]);
	}
	//
	for (int i = 0; i < length(); i++)
	{
		if (chars.find(at(i)) != chars.end())
			return i;
	}
	//
	return -1;
}

int AngKSmartScaleUI(int spec)
{
	static double rate = 0;
	if (0 == (int)rate)
	{
		//
		QList<QScreen*> screens = QApplication::screens();
		if (screens.size() > 0)
		{
			QScreen* screen = screens[0];
			double dpi = screen->logicalDotsPerInch();
			//
			rate = dpi / 96.0;
			//

			if (rate < 1.1)
			{
				rate = 1.0;
			}
			else if (rate < 1.4)
			{
				rate = 1.25;
			}
			else if (rate < 1.6)
			{
				rate = 1.5;
			}
			else if (rate < 1.8)
			{
				rate = 1.75;
			}
			else
			{
				rate = 2.0;
			}
		}
		else
		{
			// 当程序窗口还没有初始化时，将得不到程序关联屏幕的相关数据，应该在窗口初始化完成后调用本函数。
			//Q_ASSERT(false);
		}
	}
	//
	return int(spec * rate);
}