// SNStep_Hittech001.h : SNStep_Hittech001 DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSNStep_Hittech001App
// �йش���ʵ�ֵ���Ϣ������� SNStep_Hittech001.cpp
//

class CSNStep_Hittech001App : public CWinApp
{
public:
	CSNStep_Hittech001App();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
