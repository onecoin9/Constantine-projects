// SNCustom_MIDEA.h : CSNCustomEspApp DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSNCustomEspApp
// �йش���ʵ�ֵ���Ϣ������� CSNCustomEspApp.cpp
//

class CSNCustomEspApp : public CWinApp
{
public:
	CSNCustomEspApp();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
