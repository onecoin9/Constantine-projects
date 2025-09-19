#include "AngKMisc.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace Utils
{
	CString AngKMisc::extractFilePath(const CString& strFileName)
	{
		CString str = strFileName;
		str.replace('\\', '/');
		int index = str.lastIndexOf('/');
		if (-1 == index)
			return strFileName;
		//
		return str.left(index + 1);
	}

	QString AngKMisc::extractFileName(const QString& strFileName)
	{
		QString str = strFileName;
		str.replace('\\', '/');
		int index = str.lastIndexOf('/');
		if (-1 == index)
			return strFileName;

		return strFileName.right(str.length() - index - 1);
	}

	qint64 AngKMisc::getFileSize(const CString& strFileName)
	{
		QFileInfo info(strFileName);
		return info.size();
	}

	void AngKMisc::deleteFile(const CString& strFileName)
	{
		QDir dir(extractFilePath(strFileName));
		dir.remove(extractFileName(strFileName));
	}
}


CString WizFormatString1(const CString& strFormat, int n)
{
	CString str(strFormat);
	str.replace("%1", QString::number(n));
	return str;
}
CString WizFormatString1(const CString& strFormat, const CString& strParam1)
{
	CString str(strFormat);
	str.replace("%1", strParam1);
	return str;
}
CString WizFormatString2(const CString& strFormat, const CString& strParam1, const CString& strParam2)
{
	CString str(strFormat);
	str.replace("%1", strParam1);
	str.replace("%2", strParam2);
	return str;
}
CString WizFormatString3(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3)
{
	CString str(strFormat);
	str.replace("%1", strParam1);
	str.replace("%2", strParam2);
	str.replace("%3", strParam3);
	return str;
}
CString WizFormatString4(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4)
{
	CString str(strFormat);
	str.replace("%1", strParam1);
	str.replace("%2", strParam2);
	str.replace("%3", strParam3);
	str.replace("%4", strParam4);
	return str;
}