// dcmotorDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "dcmotor.h"
#include "dcmotorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CdcmotorDlg �Ի���
HANDLE hDcm=INVALID_HANDLE_VALUE;
int setpwm=100;
#define DCMOTOR_IOCTRL_RUN	1
#define DCMOTOR_IOCTRL_ANTIRUN	2
#define DCMOTOR_IOCTRL_STOP	3
CdcmotorDlg::CdcmotorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CdcmotorDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CdcmotorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CdcmotorDlg, CDialog)
	ON_BN_CLICKED(IDC_CLOCKWISE, OnClockwise)
	ON_BN_CLICKED(IDC_Eastern, OnEastern)
	ON_BN_CLICKED(IDC_STOP, OnStop)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	
END_MESSAGE_MAP()


// CdcmotorDlg ��Ϣ�������

BOOL CdcmotorDlg::OnInitDialog()
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
void CdcmotorDlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_DCMOTOR_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_DCMOTOR_DIALOG));
	}
}
#endif



void CdcmotorDlg::OnClockwise() 
{
	// TODO: Add your control notification handler code here
	if(hDcm==INVALID_HANDLE_VALUE)
		hDcm=CreateFile(TEXT("DCM1:"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);
	
	setpwm=100;
	PBYTE temp=(PBYTE)(&setpwm);
	
	DeviceIoControl(hDcm,DCMOTOR_IOCTRL_RUN,temp,sizeof(temp),NULL,NULL,NULL,NULL);


	
}

void CdcmotorDlg::OnEastern() 
{
	// TODO: Add your control notification handler code here
	if(hDcm==INVALID_HANDLE_VALUE)
		hDcm=CreateFile(TEXT("DCM1:"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);
	
	setpwm=-100;
	PBYTE temp=(PBYTE)(&setpwm);
	
	DeviceIoControl(hDcm,DCMOTOR_IOCTRL_ANTIRUN,temp,sizeof(temp),NULL,NULL,NULL,NULL);
	
}

void CdcmotorDlg::OnStop() 
{
	// TODO: Add your control notification handler code here
	if(hDcm==INVALID_HANDLE_VALUE)
		hDcm=CreateFile(TEXT("DCM1:"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);
	
	setpwm=0;
	PBYTE temp=(PBYTE)(&setpwm);
	DeviceIoControl(hDcm,DCMOTOR_IOCTRL_STOP,temp,sizeof(temp),NULL,NULL,NULL,NULL);

	CloseHandle(hDcm);
	hDcm=INVALID_HANDLE_VALUE;


	
}