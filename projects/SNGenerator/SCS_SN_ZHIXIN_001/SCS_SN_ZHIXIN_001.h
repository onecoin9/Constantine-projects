// SCS_SN_ZHIXIN_001.h : SCS_SN_ZHIXIN_001 DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSCS_SN_ZHIXIN_001App
// �йش���ʵ�ֵ���Ϣ������� SCS_SN_ZHIXIN_001.cpp
//

class CSCS_SN_ZHIXIN_001App : public CWinApp
{
public:
	CSCS_SN_ZHIXIN_001App();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
