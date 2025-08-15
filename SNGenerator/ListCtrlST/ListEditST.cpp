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
	//这里新建了一个CPage2ListCtrl 类型的指针，所以在CListEdit的cpp文件前必须先添加#include "Page2ListCtrl.h"
	temp=(CListCtrlST*)GetParent();  
	temp->DisposeEdit();    //调用父窗口的DisposeEdit()函数。
	// TODO: Add your message handler code here 
	CEdit::OnKillFocus(pNewWnd);
	// TODO: 在此处添加消息处理程序代码
}

////设置最大输入多少个字符
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

///设置数据类型
void CListEditST::SetDataType( UINT nDataType )
{
	m_nDataType=nDataType;
}
void CListEditST::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if(m_nDataType==DATATYPE_NUM||m_nDataType==DATATYPE_HEX){
		int i=0;
		// TODO: Add your message handler code here and/or call default
		if (nChar != 8){
			SetLimitText(m_nTextLimit);
			//检查是否在合法字符内。
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

				if (p - buf >=16){//把小写的a-f转化成大写的A -F
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
