// SNCustom_MIDEA.h : SNCustom_MIDEA DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSNCustom_MIDEAApp
// �йش���ʵ�ֵ���Ϣ������� SNCustom_MIDEA.cpp
//

class CSNCustom_MIDEAApp : public CWinApp
{
public:
	CSNCustom_MIDEAApp();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
