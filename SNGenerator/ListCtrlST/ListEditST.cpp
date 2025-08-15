/**********************************************
Author: sllin
Email:	sllin40@qq.com
**********************************************/
#include "ListEditST.h"
#include "ListCtrlST.h"
#include <stdio.h>


CListEditST::CListEditST(void)
{
	ZeroMemory(m_LegalChars, MAXLEGALCHAR);
	AddLegalChar(22);
	AddLegalChar(3);
	m_nDataType=DATATYPE_STR;
	m_nTextLimit=(UINT)-1;
}

CListEditST::~CListEditST(void)
{
}

BEGIN_MESSAGE_MAP(CListEditST, CEdit)
	ON_WM_KILLFOCUS()
	ON_WM_CHAR()
END_MESSAGE_MAP()

void CListEditST::OnKillFocus(CWnd* pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);
	CListCtrlST * temp;     
	//�����½���һ��CPage2ListCtrl ���͵�ָ�룬������CListEdit��cpp�ļ�ǰ���������#include "Page2ListCtrl.h"
	temp=(CListCtrlST*)GetParent();  
	temp->DisposeEdit();    //���ø����ڵ�DisposeEdit()������
	// TODO: Add your message handler code here 
	CEdit::OnKillFocus(pNewWnd);
	// TODO: �ڴ˴������Ϣ����������
}

////�������������ٸ��ַ�
void CListEditST::SetTextLimit(UINT nTextLimit)
{
	m_nTextLimit = nTextLimit;
	SetLimitText(m_nTextLimit);
	
}

UINT64 CListEditST::GetIntVal(CString&strText)
{
	UINT64 intTemp = 0;
	if(!strText.IsEmpty())
		sscanf(strText, "%I64X", &intTemp);
	return intTemp;
}

void CListEditST::AddLegalChar(char ch)
{
	int i = 0;
	for(i = 0; i < MAXLEGALCHAR; i++)
	{
		if(m_LegalChars[i] == ch)
			return;

		if(m_LegalChars[i] == '\0')
			break;
	}

	if(i < MAXLEGALCHAR)
		m_LegalChars[i] = ch;
}

///������������
void CListEditST::SetDataType( UINT nDataType )
{
	m_nDataType=nDataType;
}
void CListEditST::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	if(m_nDataType==DATATYPE_NUM||m_nDataType==DATATYPE_HEX){
		int i=0;
		// TODO: Add your message handler code here and/or call default
		if (nChar != 8){
			SetLimitText(m_nTextLimit);
			//����Ƿ��ںϷ��ַ��ڡ�
			for(i = 0; i < MAXLEGALCHAR; i++){
				if(m_LegalChars[i] == (char)nChar)
					break;
			}

			if(i >= MAXLEGALCHAR){
				const char *buf = "0123456789ABCDEFabcdef";
				if (m_nDataType==DATATYPE_NUM) {
					buf = "0123456789";
				}
				const char *p = buf;

				while (*p && (char)nChar != *p) p++;
				if (!*p) {
					return ;
				}

				if (p - buf >=16){//��Сд��a-fת���ɴ�д��A -F
					p -= 6;
				}
				nChar = *p;
			}

			CWnd::DefWindowProc(WM_CHAR, nChar , MAKELPARAM (nRepCnt, nFlags ));

		}else{
			CEdit::OnChar(nChar, nRepCnt, nFlags);
		}
	}
	else{
		CEdit::OnChar(nChar, nRepCnt, nFlags);
	}
}
