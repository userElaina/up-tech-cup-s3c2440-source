// Digital_DotDlg.cpp : ʵ���ļ�
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


// CDigital_DotDlg �Ի���

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


// CDigital_DotDlg ��Ϣ�������

BOOL CDigital_DotDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	//���豸
	hLed=CreateFile(L"DDT1:",GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	if(!hLed)
		printf("��DOT�豸ʧ�ܣ�");
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
	
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
	DeviceIoControl(hLed,COMMAND_IOCTRL_LED,temp,sizeof(m_show),NULL,NULL,NULL,NULL); //IoControl�����������ʾ��������ʵ����
}

void CDigital_DotDlg::OnBnClickedDotStart()
{


	UpdateData();

	PBYTE *temp=(PBYTE*)(&m_arrayshow);
	DeviceIoControl(hLed,COMMAND_IOCTRL_ARRAYLED,temp,sizeof(m_arrayshow),NULL,NULL,NULL,NULL); //�豸iocontrol�����豸��ʾ

	
}

void CDigital_DotDlg::OnBnClickedTubrStop()
{
	
	DeviceIoControl(hLed,COMMAND_IOCTRL_LEDOFF,NULL,NULL,NULL,NULL,NULL,NULL);
}

void CDigital_DotDlg::OnBnClickedDotStop()
{
	DeviceIoControl(hLed,COMMAND_IOCTRL_ARRAYOFF,NULL,NULL,NULL,NULL,NULL,NULL);
}
