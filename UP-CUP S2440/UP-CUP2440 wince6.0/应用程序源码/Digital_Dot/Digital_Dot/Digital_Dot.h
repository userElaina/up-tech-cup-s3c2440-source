// Digital_Dot.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#ifdef STANDARDSHELL_UI_MODEL
#include "resource.h"
#endif

// CDigital_DotApp:
// �йش����ʵ�֣������ Digital_Dot.cpp
//

class CDigital_DotApp : public CWinApp
{
public:
	CDigital_DotApp();
	
// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CDigital_DotApp theApp;
