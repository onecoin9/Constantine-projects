/**********************************************
Author: sllin
Email:	sllin40@qq.com
**********************************************/
#include "ComboBoxST.h"

// CComboBoxST
#include "ListCtrlST.h"

IMPLEMENT_DYNAMIC(CComboBoxST, CComboBox)

CComboBoxST::CComboBoxST()
{

}

CComboBoxST::~CComboBoxST()
{
}


BEGIN_MESSAGE_MAP(CComboBoxST, CComboBox)
	ON_CONTROL_REFLECT(CBN_KILLFOCUS, &CComboBoxST::OnCbnKillfocus)
END_MESSAGE_MAP()



// CComboBoxST 消息处理程序



void CComboBoxST::OnCbnKillfocus()
{
	// TODO: 在此添加控件通知处理程序代码
	CListCtrlST * temp;     
	temp=(CListCtrlST*)GetParent();  
	temp->DisposeComboBox();    //调用父窗口的DisposeComboBox()函数。
}
