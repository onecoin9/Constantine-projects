// WT2105.h : WT2105 DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CWT2105App
// �йش���ʵ�ֵ���Ϣ������� WT2105.cpp
//

class CWT2105App : public CWinApp
{
public:
	CWT2105App();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
