// UPTECH_HELLO.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "UPTECH_HELLO.h"
#include "UPTECH_HELLODlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUPTECH_HELLOApp

BEGIN_MESSAGE_MAP(CUPTECH_HELLOApp, CWinApp)
END_MESSAGE_MAP()


// CUPTECH_HELLOApp ����
CUPTECH_HELLOApp::CUPTECH_HELLOApp()
	: CWinApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CUPTECH_HELLOApp ����
CUPTECH_HELLOApp theApp;

// CUPTECH_HELLOApp ��ʼ��

BOOL CUPTECH_HELLOApp::InitInstance()
{

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	CUPTECH_HELLODlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˴����ô����ʱ�á�ȷ�������ر�
		//  �Ի���Ĵ���
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}
