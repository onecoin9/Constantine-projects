// SCS_SN_YCHUANG.h : SCS_SN_YCHUANG DLL 主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含"stdafx.h"以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号


// CSCS_SN_YCHUANGApp
// 有关此类实现的信息，请参阅 SCS_SN_YCHUANG.cpp
//

class CSCS_SN_YCHUANGApp : public CWinApp
{
public:
	CSCS_SN_YCHUANGApp();

// 重写
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
