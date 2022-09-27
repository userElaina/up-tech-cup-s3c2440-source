// Digital_DotDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Digital_Dot.h"
#include "Digital_DotDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define COMMAND_IOCTRL_ARRAYLED		2
#define COMMAND_IOCTRL_LED			3
#define COMMAND_IOCTRL_LEDOFF		4
#define COMMAND_IOCTRL_ARRAYOFF		5
HANDLE hLed;


// CDigital_DotDlg 对话框

CDigital_DotDlg::CDigital_DotDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDigital_DotDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_show = 0;
	m_arrayshow = 0;
}

void CDigital_DotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_show);
	DDV_MinMaxInt(pDX, m_show, 0, 99);
	DDX_Text(pDX, IDC_EDIT2, m_arrayshow);
	DDV_MinMaxUInt(pDX, m_arrayshow, 0, 99);
}

BEGIN_MESSAGE_MAP(CDigital_DotDlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_TUBR_START, &CDigital_DotDlg::OnBnClickedTubrStart)
	ON_BN_CLICKED(IDC_DOT_START, &CDigital_DotDlg::OnBnClickedDotStart)
	ON_BN_CLICKED(IDC_TUBR_STOP, &CDigital_DotDlg::OnBnClickedTubrStop)
	ON_BN_CLICKED(IDC_DOT_STOP, &CDigital_DotDlg::OnBnClickedDotStop)
END_MESSAGE_MAP()


// CDigital_DotDlg 消息处理程序

BOOL CDigital_DotDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//打开设备
	hLed=CreateFile(L"DDT1:",GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	if(!hLed)
		printf("打开DOT设备失败！");
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
	
}

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
void CDigital_DotDlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_DIGITAL_DOT_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_DIGITAL_DOT_DIALOG));
	}
}
#endif


void CDigital_DotDlg::OnBnClickedTubrStart()
{

	this->UpdateData();
	

	PBYTE *temp=(PBYTE*)(&m_show);
	DeviceIoControl(hLed,COMMAND_IOCTRL_LED,temp,sizeof(m_show),NULL,NULL,NULL,NULL); //IoControl控制数码管显示，传递现实参数
}

void CDigital_DotDlg::OnBnClickedDotStart()
{


	UpdateData();

	PBYTE *temp=(PBYTE*)(&m_arrayshow);
	DeviceIoControl(hLed,COMMAND_IOCTRL_ARRAYLED,temp,sizeof(m_arrayshow),NULL,NULL,NULL,NULL); //设备iocontrol控制设备显示

	
}

void CDigital_DotDlg::OnBnClickedTubrStop()
{
	
	DeviceIoControl(hLed,COMMAND_IOCTRL_LEDOFF,NULL,NULL,NULL,NULL,NULL,NULL);
}

void CDigital_DotDlg::OnBnClickedDotStop()
{
	DeviceIoControl(hLed,COMMAND_IOCTRL_ARRAYOFF,NULL,NULL,NULL,NULL,NULL,NULL);
}
