// SNCustom_CKD_001.h : SNCustom_CKD_001 DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSNCustom_CKD_001App
// �йش���ʵ�ֵ���Ϣ������� SNCustom_CKD_001.cpp
//

class CSNCustom_CKD_001App : public CWinApp
{
public:
	CSNCustom_CKD_001App();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
