/**********************************************
Author: sllin
Email:	sllin40@qq.com
**********************************************/
#pragma once

#include <afxcmn.h>
// CComboBoxST

class CComboBoxST : public CComboBox
{
	DECLARE_DYNAMIC(CComboBoxST)

public:
	CComboBoxST();
	virtual ~CComboBoxST();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnKillfocus();
};


