// SNStep.h : SNStep DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSNStepApp
// �йش���ʵ�ֵ���Ϣ������� SNStep.cpp
//

class CSNStepApp : public CWinApp
{
public:
	CSNStepApp();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
