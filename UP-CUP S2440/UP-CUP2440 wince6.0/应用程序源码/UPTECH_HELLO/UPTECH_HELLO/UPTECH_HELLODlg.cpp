// UPTECH_HELLODlg.cpp : 实现文件
//

#include "stdafx.h"
#include "UPTECH_HELLO.h"
#include "UPTECH_HELLODlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CUPTECH_HELLODlg 对话框

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


// CUPTECH_HELLODlg 消息处理程序

BOOL CUPTECH_HELLODlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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
	// TODO: 在此添加控件通知处理程序代码
	MessageBox(_T("hello word!"));
}

void CUPTECH_HELLODlg::OnBnClickedClose()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialog::OnCancel();
}
