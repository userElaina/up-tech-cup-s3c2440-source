// GPRS_TEST.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#ifdef STANDARDSHELL_UI_MODEL
#include "resource.h"
#endif

// CGPRS_TESTApp:
// �йش����ʵ�֣������ GPRS_TEST.cpp
//

class CGPRS_TESTApp : public CWinApp
{
public:
	CGPRS_TESTApp();
	
// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CGPRS_TESTApp theApp;
