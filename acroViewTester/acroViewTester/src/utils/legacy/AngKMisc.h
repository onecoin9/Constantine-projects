#pragma once

#include <QtGlobal>
#include "AngKHelper.h"

class QString;

CString WizFormatString1(const CString& strFormat, int n);
CString WizFormatString1(const CString& strFormat, const CString& strParam1);
CString WizFormatString2(const CString& strFormat, const CString& strParam1, const CString& strParam2);
CString WizFormatString3(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3);
CString WizFormatString4(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4);

namespace Utils
{
	class AngKMisc
	{
	public:
		static CString extractFilePath(const CString& strFileName);
		static QString extractFileName(const QString& strFileName);
		static qint64 getFileSize(const CString& strFileName);
		static void deleteFile(const CString& strFileName);
	};

}