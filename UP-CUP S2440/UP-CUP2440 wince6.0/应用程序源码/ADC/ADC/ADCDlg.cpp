// ADCDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ADC.h"
#include "ADCDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
HANDLE hAdc=INVALID_HANDLE_VALUE;
BOOL StopFlag=FALSE;
// CADCDlg �Ի���

CADCDlg::CADCDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CADCDlg::IDD, pParent)
{
	m_fCH1 = 0.0f;
	m_fCH2 = 0.0f;
	m_fCH3 = 0.0f;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CADCDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Text(pDX, IDC_EDIT1, m_fCH1);
	DDX_Text(pDX, IDC_EDIT2, m_fCH2);
	DDX_Text(pDX, IDC_EDIT3, m_fCH3);
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CADCDlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_START, OnButtonStart)
	ON_BN_CLICKED(IDC_STOP, OnButtonStop)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CADCDlg ��Ϣ�������

BOOL CADCDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��
CenterWindow(GetDesktopWindow());	// center to the hpc screen

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
void CADCDlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_ADC_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_ADC_DIALOG));
	}
}
#endif






UINT Adc(LPVOID   pParam)
{
		DWORD dwactlen;
	 int data=int(0xff);
	 int buffer;
    
	 float temp[3];
	 CADCDlg *dlg=(CADCDlg *)pParam;
	 for(;;)
	 {
		 if(StopFlag)break;
		 for(int i=0;i<=2;i++)
		 {
			//data=((i)<<16) | (data);
			 if(i==0)
				 data=0xff;
			 else if(i==1)
				 data=0x100ff;
			 else
				 data=0x200ff;
			WriteFile(hAdc,&data,1,&dwactlen,NULL);
			ReadFile(hAdc,&buffer,1,&dwactlen,NULL);
			temp[i] = buffer*3.3/1024.0;
		 
		 }
		 dlg->m_fCH1=temp[0];
		 dlg->m_fCH2=temp[1];
		 dlg->m_fCH3=temp[2];
		 dlg->UpdateData (FALSE);
		 Sleep(30);
			
	 }
	 
	 return 0;

}



void CADCDlg::OnButtonStart() 
{
	// TODO: Add your control notification handler code here
	if(hAdc==INVALID_HANDLE_VALUE)
		hAdc=CreateFile(TEXT("ADC1:"),GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,0);
	StopFlag=FALSE;
	AfxBeginThread(Adc,LPVOID(this));
	




	
}

void CADCDlg::OnButtonStop() 
{
	// TODO: Add your control notification handler code here
	StopFlag=TRUE;
	CloseHandle(hAdc);
	hAdc=INVALID_HANDLE_VALUE;
	
}

void CADCDlg::OnDestroy() 
{
	StopFlag=TRUE;
	Sleep(10);
	CloseHandle(hAdc);
	hAdc=INVALID_HANDLE_VALUE;
	
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here

	
}


