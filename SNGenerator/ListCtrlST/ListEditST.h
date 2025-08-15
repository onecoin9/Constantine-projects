/**********************************************
Author: sllin
Email:	sllin40@qq.com
**********************************************/
#pragma once
#include "afxwin.h"

#ifndef MAXLEGALCHAR
#define MAXLEGALCHAR 128
#endif
class CListEditST :public CEdit
{
public:
	enum{
		DATATYPE_STR,		///�����ַ���
		DATATYPE_NUM,		///�������0-9��Щ10�����ַ�
		DATATYPE_HEX,		///�������0-9,A-F,a-f��Щ16���Ƶ��ַ�
	};
	CListEditST(void);
	~CListEditST(void);

	void SetTextLimit(UINT nTextLimit);
	void SetDataType(UINT nDataType);

	static UINT64 GetIntVal(CString&strText);
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	void AddLegalChar(char ch);
private:
	UINT m_nTextLimit; 
	char m_LegalChars[MAXLEGALCHAR];
	UINT m_nDataType;
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};