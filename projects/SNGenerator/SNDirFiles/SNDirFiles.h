// SNDirFiles.h : SNDirFiles DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSNDirFilesApp
// �йش���ʵ�ֵ���Ϣ������� SNDirFiles.cpp
//

class CSNDirFilesApp : public CWinApp
{
public:
	CSNDirFilesApp();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
