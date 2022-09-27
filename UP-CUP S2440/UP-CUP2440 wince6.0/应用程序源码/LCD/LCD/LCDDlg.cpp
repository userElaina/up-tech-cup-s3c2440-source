// LCDDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "LCD.h"
#include "LCDDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CLCDDlg �Ի���

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


// CLCDDlg ��Ϣ�������

BOOL CLCDDlg::OnInitDialog()
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	DEVMODE devmode = {0};
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmDisplayOrientation = DMDO_0; //0��ģʽ
	devmode.dmFields = DM_DISPLAYORIENTATION;
	ChangeDisplaySettingsEx(NULL, &devmode, NULL, 0, NULL);
}

void CLCDDlg::OnBnClickedButton2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	DEVMODE devmode = {0};
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmDisplayOrientation = DMDO_270; //270��ģʽ
	devmode.dmFields = DM_DISPLAYORIENTATION;
	ChangeDisplaySettingsEx(NULL, &devmode, NULL, 0, NULL);
}

void CLCDDlg::OnBnClickedButton3()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	DEVMODE devmode = {0};
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmDisplayOrientation = DMDO_90; //90��ģʽ
	devmode.dmFields = DM_DISPLAYORIENTATION;
	ChangeDisplaySettingsEx(NULL, &devmode, NULL, 0, NULL);
}

void CLCDDlg::OnBnClickedButton4()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	DEVMODE devmode = {0};
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmDisplayOrientation = DMDO_180; //180��ģʽ
	devmode.dmFields = DM_DISPLAYORIENTATION;
	ChangeDisplaySettingsEx(NULL, &devmode, NULL, 0, NULL);
}
