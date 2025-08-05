#pragma once
#include <QtGlobal>
#include <QString>
#include <set>


class CString : public QString
{
public:
	inline CString() : QString() {}
	CString(const QChar* unicode, int size) : QString(unicode, size) {}
	explicit CString(const QChar* unicode) : QString(unicode) {}
	CString(QChar c) :QString(c) {}
	CString(int size, QChar c) :QString(size, c) {}
	inline CString(const QLatin1String& latin1) : QString(latin1) {}
	inline CString(const QString& src) : QString(src) {}
	CString(const char* ch) : QString(ch) {}
	CString(const unsigned short* pUtf16) { *this = fromUtf16(pUtf16); }
	CString(const unsigned short* pUtf16, int size) { *this = fromUtf16(pUtf16, size); }
	CString(const wchar_t* pUtf16) { *this = fromWCharArray(pUtf16); }
	operator const unsigned short* () const { return utf16(); }
	void trim() { *this = trimmed(); }
	void trim(char ch);
	void trimLeft();
	void trimRight();
	CString makeLower() { *this = toLower(); return *this; }
	CString makeUpper() { *this = toUpper(); return *this; }
	int getLength() const { return length(); }
	//
	using QString::compare;
	int compareNoCase(const CString& strOther) const { return compare(strOther, Qt::CaseInsensitive); }
	int compare(const CString& strOther) const { return compare(strOther, Qt::CaseSensitive); }

	void format(QString strFormat, ...);
	void empty() { clear(); }
	int find(char ch) const { return indexOf(ch); }
	int find(char ch, int start) const { return indexOf(ch, start); }
	int find(const CString& str) const { return indexOf(str); }
	void appendChar(char ch) { append(ch); }
	void setAt(int index, QChar ch);
	int findOneOf(const CString& strFind) const;
};

int AngKSmartScaleUI(int spec);