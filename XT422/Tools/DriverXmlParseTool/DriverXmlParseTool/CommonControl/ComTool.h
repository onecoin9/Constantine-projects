#pragma once

#include <QtCore/QFile>
#include <QtCore/QDir>
#include "GlobalDefine.h"

namespace ComTool
{
	static bool FIleExit(const char* strPath)
	{
		return QFile::exists(strPath);
	}

	static bool CreateFile(const char* strName)
	{
		QFile pfile(strName);

		bool ok = pfile.open(QIODevice::WriteOnly | QIODevice::Text);

		pfile.close();

		return ok;
	}

	static bool CreateFile(QString strName)
	{
		QFile pfile(strName);

		bool ok = pfile.open(QIODevice::WriteOnly | QIODevice::Text);

		pfile.close();

		return ok;
	}

	static bool CreateDir(const char* strPath)
	{
		QDir dir;
		bool ok = false;
		if(!dir.exists(strPath))
			ok = dir.mkdir(strPath);

		return ok;
	}

	static QString Rect2QString(QRect rect)
	{
		return QString::number(rect.x()) + "," + QString::number(rect.y()) + "," 
			+ QString::number(rect.width()) + "," + QString::number(rect.height());
	}

	static std::string Rect2String(QRect rect)
	{
		return QString(QString::number(rect.x()) + "," + QString::number(rect.y()) + "," 
			+ QString::number(rect.width()) + "," + QString::number(rect.height())).toStdString();
	}

	static QRect String2Rect(const char* strRect)
	{
		if (nullptr == strRect)
			return QRect();

		QStringList strList = QString::fromStdString(strRect).split(",");

		if (strList.count() != 4)
			return QRect();


		QRect retRect(strList[0].toInt(), strList[1].toInt(), strList[2].toInt(), strList[3].toInt());

		return retRect;
	}

	static const char* EditVaule_Int2CString(int nType)
	{
		const char* ret = "";
		switch (nType)
		{
		case DEC:
			ret = "DEC";
			break;
		case HEX:
			ret = "HEX";
			break;
		case STR:
			ret = "STR";
			break;
		default:
			break;
		}

		return ret;
	}

	static int EditVaule_CString2Int(const char* strType)
	{
		int ret = -1;

		if (strType == "DEC")
		{
			ret = DEC;
		}
		else if (strType == "HEX")
		{
			ret = HEX;
		}
		else if (strType == "STR")
		{
			ret = STR;
		}
		return ret;
	}

	static const char* EditEndian_Int2CString(int nType)
	{
		const char* ret = "";
		switch (nType)
		{
		case Little:
			ret = "L";
			break;
		case Big:
			ret = "B";
			break;
		}

		return ret;
	}

	static int EditEndian_CString2Int(const char* strType)
	{
		int ret = -1;

		if (strType == "L")
		{
			ret = Little;
		}
		else if (strType == "B")
		{
			ret = Big;
		}
		return ret;
	}

	static int Hex_String2Int(const char* strHex)
	{
		QString qHexString = strHex;

		int nPos = qHexString.lastIndexOf("0x");

		qHexString = qHexString.mid(nPos, qHexString.size() - nPos);

		bool k;
		int nHex = qHexString.toInt(&k, 16);

		return nHex;
	}
}