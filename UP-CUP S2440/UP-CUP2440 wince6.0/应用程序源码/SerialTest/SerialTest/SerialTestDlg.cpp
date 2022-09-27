// SerialTestDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SerialTest.h"
#include "SerialTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSerialTestDlg �Ի���

CSerialTestDlg::CSerialTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSerialTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_strSendEdit = _T("");
	//groupbox1.BackColor = Color.Transparent��

}

void CSerialTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CLOSE_COM, m_ButClose);
	DDX_Control(pDX, IDC_OPEN_COM, m_ButOpen);
	DDX_Control(pDX, IDC_STOP, m_ComboStop);
	DDX_Control(pDX, IDC_PARITY, m_ComboParity);
	DDX_Control(pDX, IDC_PORT, m_ComboPort);
	DDX_Control(pDX, IDC_DATA, m_ComboData);
	DDX_Control(pDX, IDC_BAUD, m_ComboBaud);
	DDX_Text(pDX, IDC_SEND_EDIT, m_strSendEdit);
}

BEGIN_MESSAGE_MAP(CSerialTestDlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CLEAR_REC, &CSerialTestDlg::OnBnClickedClearRec)
	ON_BN_CLICKED(IDC_SEND, &CSerialTestDlg::OnBnClickedSend)
	ON_BN_CLICKED(IDC_CLEAR_SEND, &CSerialTestDlg::OnBnClickedClearSend)
	ON_BN_CLICKED(IDC_OPEN_COM, &CSerialTestDlg::OnBnClickedOpenCom)
	ON_BN_CLICKED(IDC_CLOSE_COM, &CSerialTestDlg::OnBnClickedCloseCom)
	
END_MESSAGE_MAP()


// CSerialTestDlg ��Ϣ�������

BOOL CSerialTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_ComboBaud.SetCurSel(5);			/* Ĭ�ϲ�����Ϊ: 115200 */
	m_ComboData.SetCurSel(1);			/* Ĭ������λΪ: 8λ */
	m_ComboParity.SetCurSel(0);			/* Ĭ��У��Ϊ: �� */
	m_ComboPort.SetCurSel(0);			/* Ĭ�϶˿�Ϊ: COM1 */
	m_ComboStop.SetCurSel(0);			/* Ĭ��ֹͣλΪ: 1λ */

	m_ButClose.EnableWindow(FALSE);		/* "�رն˿�"������Ч */
	m_hComm = INVALID_HANDLE_VALUE;		/* ���ڲ��������Ч */
	m_ExitThreadEvent = NULL;			/* ���ڽ����߳��˳��¼���Ч */
	m_strRecDisp = _T("");				/* ������ʾ�ַ�Ϊ�� */

	UpdateData(FALSE);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
void CSerialTestDlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_SERIALTEST_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_SERIALTEST_DIALOG));
	}
}
#endif
BOOL CSerialTestDlg::OpenPort(LPCTSTR Port, int BaudRate, int DataBits, int StopBits, int Parity)
{
	COMMTIMEOUTS CommTimeOuts;

	// �򿪴���
	m_hComm = CreateFile(Port, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	if(m_hComm == INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("�޷��򿪶˿ڻ�˿��Ѵ�!�����Ƿ��ѱ�ռ��."));
		return FALSE;
	}

	GetCommState(m_hComm, &dcb);						/* ��ȡ���ڵ�DCB */
	dcb.BaudRate = BaudRate;			
	dcb.ByteSize = DataBits;
	dcb.Parity = Parity;
	dcb.StopBits = StopBits;	
	dcb.fParity = FALSE;								/* ��ֹ��żУ�� */
	dcb.fBinary = TRUE;
	dcb.fDtrControl = 0;								/* ��ֹ�������� */
	dcb.fRtsControl = 0;
	dcb.fOutX = 0;
	dcb.fInX = 0;
	dcb.fTXContinueOnXoff = 0;
	
	//����״̬����
	SetCommMask(m_hComm, EV_RXCHAR);					/* �����¼�:���յ�һ���ַ� */	
	SetupComm(m_hComm, 16384, 16384);					/* ���ý����뷢�͵Ļ�������С */
	if(!SetCommState(m_hComm, &dcb))					/* ���ô��ڵ�DCB */
	{
		MessageBox(_T("�޷�����ǰ�������ö˿ڣ��������!"));
		ClosePort();
		return FALSE;
	}
		
	//���ó�ʱ����
	GetCommTimeouts(m_hComm, &CommTimeOuts);		
	CommTimeOuts.ReadIntervalTimeout = 100;				/* �����ַ������ʱ���� */
	CommTimeOuts.ReadTotalTimeoutMultiplier = 1;		
	CommTimeOuts.ReadTotalTimeoutConstant = 100;		/* �������ܳ�ʱ���� */
	CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
	CommTimeOuts.WriteTotalTimeoutConstant = 0;		
	if(!SetCommTimeouts(m_hComm, &CommTimeOuts))
	{
		MessageBox(_T("�޷����ó�ʱ����!"));
		ClosePort();
		return FALSE;
	}
		
	PurgeComm(m_hComm, PURGE_TXCLEAR | PURGE_RXCLEAR);	 /* �����/�������� */		
	return TRUE;		
}


BOOL CSerialTestDlg::ClosePort(void)
{
	if(m_hComm != INVALID_HANDLE_VALUE)
	{
		SetCommMask(m_hComm, 0);		
		PurgeComm(m_hComm, PURGE_TXCLEAR | PURGE_RXCLEAR);	/* �����/������ */
		CloseHandle(m_hComm);								/* �رմ��ڲ������ */
		m_hComm = INVALID_HANDLE_VALUE;
		return TRUE;
	}

	return FALSE;
}


void CALLBACK CSerialTestDlg::OnCommRecv(CWnd* pWnd, char *buf, int buflen)
{
	CString tmp;

	CSerialTestDlg * pDlg = (CSerialTestDlg*)pWnd;
	CEdit *pRecvStrEdit = (CEdit*)pDlg->GetDlgItem(IDC_REC_DISP);
														/* ȡ�ÿؼ�ָ�� */
	for (int i = 0; i < buflen; i++, buf++)
	{
		if((*buf)!='\0')
		{
			tmp.Format(_T("%c"), *buf);						/* ���ַ�ת��Ϊ�ַ��� */
			pDlg->m_strRecDisp += tmp;
		}
	}


	pRecvStrEdit->SetWindowText(pDlg->m_strRecDisp);	/* ��ʾ�ڴ����� */
	pRecvStrEdit->SetWindowText(pDlg->m_strRecDisp);	/* ��ʾ�ڴ����� */
	int   nLen   =   pRecvStrEdit->GetWindowTextLength(); 
	pRecvStrEdit->SetSel(nLen,   nLen);
}


DWORD CSerialTestDlg::CommRecvTread(LPVOID lparam)
{
	DWORD dwLength;
	char *recvBuf = new char[1024];
	CSerialTestDlg *pDlg = (CSerialTestDlg*)lparam;

	while(TRUE)
	{																/* �ȴ��߳��˳��¼� */
		if (WaitForSingleObject(pDlg->m_ExitThreadEvent, 0) == WAIT_OBJECT_0)
			break;	

		if (pDlg->m_hComm != INVALID_HANDLE_VALUE)
		{															/* �Ӵ��ڶ�ȡ���� */
			BOOL fReadState = ReadFile(pDlg->m_hComm, recvBuf, 1024, &dwLength, NULL);
			if(!fReadState)
			{
				//MessageBox(_T("�޷��Ӵ��ڶ�ȡ����!"));
			}
			else
			{
					for(int i=0;i<dwLength;i++)
				{
					printf("%c",recvBuf[i]);
				}
					
				if(dwLength != 0)
					OnCommRecv(pDlg, recvBuf, dwLength);			/* ���ճɹ����ûص����� */
			}
		}
	}		

	delete[] recvBuf;
	return 0;
}


// ���崮�����ò������
const CString PorTbl[6] = {_T("COM1:"),_T("COM2:"),_T("COM3:"),_T("COM4:"),_T("COM5:"), _T("COM6:")};
const DWORD BaudTbl[6] = {4800, 9600, 19200, 38400, 57600,115200};	
const DWORD DataBitTbl[2] = {7, 8};
const BYTE  StopBitTbl[3] = {ONESTOPBIT, ONE5STOPBITS, TWOSTOPBITS};
const BYTE  ParityTbl[4] = {NOPARITY, ODDPARITY, EVENPARITY, MARKPARITY};


void CSerialTestDlg::OnBnClickedClearRec()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_strRecDisp = _T("");					
	SetDlgItemText(IDC_REC_DISP,m_strRecDisp);		/* ������������ַ� */
}

void CSerialTestDlg::OnBnClickedSend()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	DWORD dwactlen;

	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("����δ��!"));
		return;
	}

	UpdateData(TRUE);
	int len = m_strSendEdit.GetLength();				 /* ȡ�������ַ������� */
	len++;
	char *psendbuf = new char[len];

	for(int i = 0; i < len;i++)
		psendbuf[i] = (char)m_strSendEdit.GetAt(i);		 /* ת��Ϊ���ֽ��ַ� */

	WriteFile(m_hComm, psendbuf, len, &dwactlen, NULL);	 /* �Ӵ��ڷ������� */
	
	delete[] psendbuf;
}

void CSerialTestDlg::OnBnClickedClearSend()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_strSendEdit = _T("");							/* ������������ַ� */
	UpdateData(FALSE);
}

void CSerialTestDlg::OnBnClickedOpenCom()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	DWORD IDThread;
	HANDLE hRecvThread;												/* �����߳̾�� */
	UpdateData(TRUE);

	CString strPort = PorTbl[m_ComboPort.GetCurSel()];				/* ����ȡ����ֵ */
	DWORD baud		= BaudTbl[m_ComboBaud.GetCurSel()];
	DWORD databit   = DataBitTbl[m_ComboData.GetCurSel()];
	BYTE stopbit    = StopBitTbl[m_ComboStop.GetCurSel()];
	BYTE parity		= ParityTbl[m_ComboParity.GetCurSel()];

	BOOL ret = OpenPort(strPort, baud, databit, stopbit, parity);	/* �򿪴��� */
	if (ret == FALSE)
		return;

	m_ExitThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);		/* �������ڽ����߳��˳��¼�*/

	// �������ڽ����߳�
	hRecvThread = CreateThread(0, 0, CommRecvTread, this, 0, &IDThread);
	if (hRecvThread == NULL) 
	{
		MessageBox(_T("���������߳�ʧ��!"));
		return;
	}	
	CloseHandle(hRecvThread);

	m_ButOpen.EnableWindow(FALSE);									/* �򿪶˿ڰ�����ֹ */				
	m_ButClose.EnableWindow(TRUE);									/* �رն˿ڰ���ʹ�� */	
	MessageBox(_T("��") + strPort + _T("�ɹ�!"));
}

void CSerialTestDlg::OnBnClickedCloseCom()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_ExitThreadEvent != NULL)
	{
		SetEvent(m_ExitThreadEvent);					/* ֪ͨ�߳��˳� */
		Sleep(1000);
		CloseHandle(m_ExitThreadEvent);
		m_ExitThreadEvent = NULL;
	}

	m_ButOpen.EnableWindow(TRUE);						/* �򿪶˿ڰ�����ֹ */				
	m_ButClose.EnableWindow(FALSE);						/* �رն˿ڰ���ʹ�� */	
	ClosePort();
}


void CSerialTestDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	if (m_ExitThreadEvent != NULL)
	{
		SetEvent(m_ExitThreadEvent);				/* ֪ͨ���ڽ����߳��˳� */
		Sleep(1000);
		CloseHandle(m_ExitThreadEvent);				/* �رս����߳��˳��¼���� */
		m_ExitThreadEvent = NULL;
	}

	ClosePort();									/* �رմ��� */
}