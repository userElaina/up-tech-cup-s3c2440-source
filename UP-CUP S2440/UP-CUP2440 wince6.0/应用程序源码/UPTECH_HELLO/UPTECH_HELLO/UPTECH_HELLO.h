// UPTECH_HELLO.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#ifdef STANDARDSHELL_UI_MODEL
#include "resource.h"
#endif

// CUPTECH_HELLOApp:
// �йش����ʵ�֣������ UPTECH_HELLO.cpp
//

class CUPTECH_HELLOApp : public CWinApp
{
public:
	CUPTECH_HELLOApp();
	
// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CUPTECH_HELLOApp theApp;
