// SNCustom_CVTE_001.h : SNCustom_CVTE_001 DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSNCustom_CVTE_001App
// �йش���ʵ�ֵ���Ϣ������� SNCustom_CVTE_001.cpp
//

class CSNCustom_CVTE_001App : public CWinApp
{
public:
	CSNCustom_CVTE_001App();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
