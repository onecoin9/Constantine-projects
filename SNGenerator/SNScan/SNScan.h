// SNScan.h : SNScan DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSNScanApp
// �йش���ʵ�ֵ���Ϣ������� SNScan.cpp
//

class CSNScanApp : public CWinApp
{
public:
	CSNScanApp();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
