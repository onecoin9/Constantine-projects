// SNCustom_FC001.h : SNCustom_FC001 DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSNCustom_FC001App
// �йش���ʵ�ֵ���Ϣ������� SNCustom_FC001.cpp
//

class CSNCustom_FC001App : public CWinApp
{
public:
	CSNCustom_FC001App();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
