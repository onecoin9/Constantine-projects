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



// CComboBoxST ��Ϣ�������



void CComboBoxST::OnCbnKillfocus()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CListCtrlST * temp;     
	temp=(CListCtrlST*)GetParent();  
	temp->DisposeComboBox();    //���ø����ڵ�DisposeComboBox()������
}
