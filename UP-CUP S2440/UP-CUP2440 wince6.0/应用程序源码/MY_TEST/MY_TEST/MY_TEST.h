// MY_TEST.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#ifdef STANDARDSHELL_UI_MODEL
#include "resource.h"
#endif

// CMY_TESTApp:
// �йش����ʵ�֣������ MY_TEST.cpp
//

class CMY_TESTApp : public CWinApp
{
public:
	CMY_TESTApp();
	
// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CMY_TESTApp theApp;
