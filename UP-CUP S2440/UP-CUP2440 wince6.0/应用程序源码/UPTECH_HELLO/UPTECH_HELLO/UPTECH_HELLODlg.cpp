// UPTECH_HELLODlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "UPTECH_HELLO.h"
#include "UPTECH_HELLODlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CUPTECH_HELLODlg �Ի���

CUPTECH_HELLODlg::CUPTECH_HELLODlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUPTECH_HELLODlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUPTECH_HELLODlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CUPTECH_HELLODlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	
	ON_BN_CLICKED(IDC_TEST, &CUPTECH_HELLODlg::OnBnClickedTest)
	ON_BN_CLICKED(IDC_CLOSE, &CUPTECH_HELLODlg::OnBnClickedClose)
END_MESSAGE_MAP()


// CUPTECH_HELLODlg ��Ϣ�������

BOOL CUPTECH_HELLODlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
void CUPTECH_HELLODlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_UPTECH_HELLO_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_UPTECH_HELLO_DIALOG));
	}
}
#endif



void CUPTECH_HELLODlg::OnBnClickedTest()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	MessageBox(_T("hello word!"));
}

void CUPTECH_HELLODlg::OnBnClickedClose()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CDialog::OnCancel();
}
