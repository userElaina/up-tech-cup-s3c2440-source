// GPS_TEST.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#ifdef STANDARDSHELL_UI_MODEL
#include "resource.h"
#endif

// CGPS_TESTApp:
// �йش����ʵ�֣������ GPS_TEST.cpp
//

class CGPS_TESTApp : public CWinApp
{
public:
	CGPS_TESTApp();
	
// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CGPS_TESTApp theApp;
