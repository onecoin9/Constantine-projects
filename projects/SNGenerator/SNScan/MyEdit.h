#pragma once

#include <afxwin.h>
// CMyEdit

class CMyEdit : public CEdit
{
	DECLARE_DYNAMIC(CMyEdit)

public:
	CMyEdit();
	virtual ~CMyEdit();
	void SetName(CString strEditName){m_strEditName=strEditName;}
	BOOL IsFocus(){return m_bFocus;}
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
private:
	CString m_strEditName;
	BOOL m_bFocus;
};


