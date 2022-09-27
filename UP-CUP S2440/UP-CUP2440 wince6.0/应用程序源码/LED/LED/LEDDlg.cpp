// LEDDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "LED.h"
#include "LEDDlg.h"
#define IO_CTL_GPIO_1_ON 0x01
#define IO_CTL_GPIO_2_ON 0x02
#define IO_CTL_GPIO_3_ON 0x03
#define IO_CTL_GPIO_ALL_ON 0x05
#define IO_CTL_GPIO_1_OFF 0x06
#define IO_CTL_GPIO_2_OFF 0x07
#define IO_CTL_GPIO_3_OFF 0x08
#define IO_CTL_GPIO_ALL_OFF 0x0a
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CLEDDlg 对话框

CLEDDlg::CLEDDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLEDDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLEDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLEDDlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_LED1_ON, &CLEDDlg::OnBnClickedLed1On)
	ON_BN_CLICKED(IDC_LED2_ON, &CLEDDlg::OnBnClickedLed2On)
	ON_BN_CLICKED(IDC_LED3_ON, &CLEDDlg::OnBnClickedLed3On)
	ON_BN_CLICKED(IDC_LEDALL_ON, &CLEDDlg::OnBnClickedLedallOn)
	ON_BN_CLICKED(IDC_LED1_OFF, &CLEDDlg::OnBnClickedLed1Off)
	ON_BN_CLICKED(IDC_LED2_OFF, &CLEDDlg::OnBnClickedLed2Off)
	ON_BN_CLICKED(IDC_LED3_OFF, &CLEDDlg::OnBnClickedLed3Off)
	ON_BN_CLICKED(IDC_LEDALL_OFF, &CLEDDlg::OnBnClickedLedallOff)
END_MESSAGE_MAP()


// CLEDDlg 消息处理程序

BOOL CLEDDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	gpiodriver=CreateFile(L"GIO1:",GENERIC_READ | GENERIC_WRITE, 0,NULL,OPEN_EXISTING,0,NULL );
	
	if(!gpiodriver)
		MessageBox(L"打开GPIO设备失败！");
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
void CLEDDlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_LED_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_LED_DIALOG));
	}
}
#endif


void CLEDDlg::OnBnClickedLed1On()
{
	// TODO: 在此添加控件通知处理程序代码
	CDC *pDC = new CClientDC(this);
	CBrush brush;
	DeviceIoControl(gpiodriver,IO_CTL_GPIO_1_ON, NULL,0,NULL,0,NULL,NULL);
	//下面绘制一个圆来表示灯亮和灭。
	brush.CreateSolidBrush(RGB(255,0,0));
	CBrush *pOldBrush = pDC->SelectObject(&brush);
	pDC->Ellipse(190,10,202,22);
	pDC->SelectObject(pOldBrush);
	delete pDC;
}

void CLEDDlg::OnBnClickedLed2On()
{
	// TODO: 在此添加控件通知处理程序代码
	CDC *pDC = new CClientDC(this);
	CBrush brush;
	DeviceIoControl(gpiodriver,IO_CTL_GPIO_2_ON,NULL,0,NULL,0,NULL,NULL);
	//下面绘制一个圆来表示灯亮和灭。
	brush.CreateSolidBrush(RGB(255,0,0));
	CBrush *pOldBrush = pDC->SelectObject(&brush);
	pDC->Ellipse(190,37,202,49);
	pDC->SelectObject(pOldBrush);
	delete pDC;
}

void CLEDDlg::OnBnClickedLed3On()
{
	// TODO: 在此添加控件通知处理程序代码
	CDC *pDC = new CClientDC(this);
	CBrush brush;
	DeviceIoControl(gpiodriver,IO_CTL_GPIO_3_ON,NULL,0,NULL,0,NULL,NULL);
	//下面绘制一个圆来表示灯亮和灭。
	brush.CreateSolidBrush(RGB(255,0,0));
	CBrush *pOldBrush = pDC->SelectObject(&brush);
	pDC->Ellipse(190,64,202,76);
	pDC->SelectObject(pOldBrush);
	delete pDC;
}

void CLEDDlg::OnBnClickedLedallOn()
{
	// TODO: 在此添加控件通知处理程序代码
	CDC *pDC = new CClientDC(this);
	CBrush brush;
	DeviceIoControl(gpiodriver,IO_CTL_GPIO_ALL_ON,NULL,0,NULL,0,NULL,NULL);
	//下面绘制一个圆来表示灯亮和灭。
	brush.CreateSolidBrush(RGB(255,0,0));
	CBrush *pOldBrush = pDC->SelectObject(&brush);
	pDC->Ellipse(190,10,202,22);
	pDC->Ellipse(190,37,202,49);
	pDC->Ellipse(190,64,202,76);
	//pDC->Ellipse(190,91,202,103);

	pDC->SelectObject(pOldBrush);
	delete pDC;
}

void CLEDDlg::OnBnClickedLed1Off()
{
	// TODO: 在此添加控件通知处理程序代码
	CDC *pDC = new CClientDC(this);
	CBrush brush;
	DeviceIoControl(gpiodriver,IO_CTL_GPIO_1_OFF,NULL,0,NULL,0,NULL,NULL);
	//下面绘制一个圆来表示灯亮和灭。
	brush.CreateSolidBrush(RGB(0,0,0));
	CBrush *pOldBrush = pDC->SelectObject(&brush);
	pDC->Ellipse(190,10,202,22);
	pDC->SelectObject(pOldBrush);
	delete pDC;
}

void CLEDDlg::OnBnClickedLed2Off()
{
	// TODO: 在此添加控件通知处理程序代码
	CDC *pDC = new CClientDC(this);
	CBrush brush;
	DeviceIoControl(gpiodriver,IO_CTL_GPIO_2_OFF,NULL,0,NULL,0,NULL,NULL);
	//下面绘制一个圆来表示灯亮和灭。
	brush.CreateSolidBrush(RGB(0,0,0));
	CBrush *pOldBrush = pDC->SelectObject(&brush);
	pDC->Ellipse(190,37,202,49);
	pDC->SelectObject(pOldBrush);
	delete pDC;
}

void CLEDDlg::OnBnClickedLed3Off()
{
	// TODO: 在此添加控件通知处理程序代码
	CDC *pDC = new CClientDC(this);
	CBrush brush;
	DeviceIoControl(gpiodriver,IO_CTL_GPIO_3_OFF,NULL,0,NULL,0,NULL,NULL);
	//下面绘制一个圆来表示灯亮和灭。
	brush.CreateSolidBrush(RGB(0,0,0));
	CBrush *pOldBrush = pDC->SelectObject(&brush);
	pDC->Ellipse(190,64,202,76);
	pDC->SelectObject(pOldBrush);
	delete pDC;
}

void CLEDDlg::OnBnClickedLedallOff()
{
	// TODO: 在此添加控件通知处理程序代码
	CDC *pDC = new CClientDC(this);
	CBrush brush;
	DeviceIoControl(gpiodriver,IO_CTL_GPIO_ALL_OFF,NULL,0,NULL,0,NULL,NULL);
	//下面绘制一个圆来表示灯亮和灭。
	brush.CreateSolidBrush(RGB(0,0,0));
	CBrush *pOldBrush = pDC->SelectObject(&brush);
	pDC->Ellipse(190,10,202,22);
	pDC->Ellipse(190,37,202,49);
	pDC->Ellipse(190,64,202,76);
	//pDC->Ellipse(190,91,202,103);

	pDC->SelectObject(pOldBrush);
	delete pDC;
}
