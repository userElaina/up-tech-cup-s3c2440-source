// Digital_Dot.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#ifdef STANDARDSHELL_UI_MODEL
#include "resource.h"
#endif

// CDigital_DotApp:
// 有关此类的实现，请参阅 Digital_Dot.cpp
//

class CDigital_DotApp : public CWinApp
{
public:
	CDigital_DotApp();
	
// 重写
public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
};

extern CDigital_DotApp theApp;
