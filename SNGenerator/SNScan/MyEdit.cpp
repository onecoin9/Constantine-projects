// MyEdit.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SNScan.h"
#include "MyEdit.h"


// CMyEdit

IMPLEMENT_DYNAMIC(CMyEdit, CEdit)

CMyEdit::CMyEdit()
: m_bFocus(FALSE)
{

}

CMyEdit::~CMyEdit()
{
}


BEGIN_MESSAGE_MAP(CMyEdit, CEdit)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()



// CMyEdit ��Ϣ�������
void CMyEdit::OnSetFocus(CWnd* pOldWnd)
{
	CEdit::OnSetFocus(pOldWnd);
	m_bFocus=TRUE;
	TRACE(" SetFocus : %s\n",m_strEditName);
	// TODO: �ڴ˴������Ϣ����������
}

void CMyEdit::OnKillFocus(CWnd* pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);
	m_bFocus=FALSE;
	TRACE(" KillFocus : %s\n",m_strEditName);
	// TODO: �ڴ˴������Ϣ����������
}
