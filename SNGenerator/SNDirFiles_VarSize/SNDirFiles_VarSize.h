// SNDirFiles_VarSize.h : SNDirFiles_VarSize DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSNDirFiles_VarSizeApp
// �йش���ʵ�ֵ���Ϣ������� SNDirFiles_VarSize.cpp
//

class CSNDirFiles_VarSizeApp : public CWinApp
{
public:
	CSNDirFiles_VarSizeApp();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
