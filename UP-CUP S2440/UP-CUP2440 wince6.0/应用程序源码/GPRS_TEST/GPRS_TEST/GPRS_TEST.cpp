// GPRS_TEST.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "GPRS_TEST.h"
#include "GPRS_TESTDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGPRS_TESTApp

BEGIN_MESSAGE_MAP(CGPRS_TESTApp, CWinApp)
END_MESSAGE_MAP()


// CGPRS_TESTApp ����
CGPRS_TESTApp::CGPRS_TESTApp()
	: CWinApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CGPRS_TESTApp ����
CGPRS_TESTApp theApp;

// CGPRS_TESTApp ��ʼ��

BOOL CGPRS_TESTApp::InitInstance()
{

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	CGPRS_TESTDlg dlg;
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
