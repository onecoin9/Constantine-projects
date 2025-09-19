// SNCustom_FC001.h : SNCustom_FC001 DLL 的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号


// CSNCustom_FC001App
// 有关此类实现的信息，请参阅 SNCustom_FC001.cpp
//

class CSNCustom_FC001App : public CWinApp
{
public:
	CSNCustom_FC001App();

// 重写
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
