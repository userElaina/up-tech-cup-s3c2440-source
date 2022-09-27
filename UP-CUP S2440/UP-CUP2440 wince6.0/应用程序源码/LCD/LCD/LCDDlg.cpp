// LCDDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "LCD.h"
#include "LCDDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CLCDDlg 对话框

CLCDDlg::CLCDDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLCDDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLCDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLCDDlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CLCDDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CLCDDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CLCDDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CLCDDlg::OnBnClickedButton4)
END_MESSAGE_MAP()


// CLCDDlg 消息处理程序

BOOL CLCDDlg::OnInitDialog()
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
void CLCDDlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_LCD_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_LCD_DIALOG));
	}
}
#endif


void CLCDDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	DEVMODE devmode = {0};
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmDisplayOrientation = DMDO_0; //0°模式
	devmode.dmFields = DM_DISPLAYORIENTATION;
	ChangeDisplaySettingsEx(NULL, &devmode, NULL, 0, NULL);
}

void CLCDDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	DEVMODE devmode = {0};
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmDisplayOrientation = DMDO_270; //270°模式
	devmode.dmFields = DM_DISPLAYORIENTATION;
	ChangeDisplaySettingsEx(NULL, &devmode, NULL, 0, NULL);
}

void CLCDDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	DEVMODE devmode = {0};
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmDisplayOrientation = DMDO_90; //90°模式
	devmode.dmFields = DM_DISPLAYORIENTATION;
	ChangeDisplaySettingsEx(NULL, &devmode, NULL, 0, NULL);
}

void CLCDDlg::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码
	DEVMODE devmode = {0};
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmDisplayOrientation = DMDO_180; //180°模式
	devmode.dmFields = DM_DISPLAYORIENTATION;
	ChangeDisplaySettingsEx(NULL, &devmode, NULL, 0, NULL);
}
