// SNFile.h : SNFile DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSNFileApp
// �йش���ʵ�ֵ���Ϣ������� SNFile.cpp
//

class CSNFileApp : public CWinApp
{
public:
	CSNFileApp();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
