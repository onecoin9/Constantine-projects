// ID206.h : ID206 DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CID206App
// �йش���ʵ�ֵ���Ϣ������� ID206.cpp
//

class CID206App : public CWinApp
{
public:
	CID206App();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
