// webcam.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "webcam.h"
#include "webcamDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CwebcamApp

BEGIN_MESSAGE_MAP(CwebcamApp, CWinApp)
END_MESSAGE_MAP()


// CwebcamApp ����
CwebcamApp::CwebcamApp()
	: CWinApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CwebcamApp ����
CwebcamApp theApp;

// CwebcamApp ��ʼ��

BOOL CwebcamApp::InitInstance()
{

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	CwebcamDlg dlg;
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
