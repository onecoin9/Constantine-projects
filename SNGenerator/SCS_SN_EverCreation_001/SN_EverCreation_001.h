// SN_EverCreation_001.h : SN_EverCreation_001 DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSN_EverCreation_001App
// �йش���ʵ�ֵ���Ϣ������� SN_EverCreation_001.cpp
//

class CSN_EverCreation_001App : public CWinApp
{
public:
	CSN_EverCreation_001App();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
