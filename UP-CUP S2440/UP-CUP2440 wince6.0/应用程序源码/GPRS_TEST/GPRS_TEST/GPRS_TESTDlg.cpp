// GPRS_TESTDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "GPRS_TEST.h"
#include "GPRS_TESTDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CGPRS_TESTDlg �Ի���

CGPRS_TESTDlg::CGPRS_TESTDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGPRS_TESTDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_strSendEdit = _T("");

	m_number = _T("");
	//m_cMessage = _T("");
}

void CGPRS_TESTDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
		DDX_Control(pDX, IDC_CLOSE_COM, m_ButClose);
	DDX_Control(pDX, IDC_OPEN_COM, m_ButOpen);
	DDX_Control(pDX, IDC_STOP, m_ComboStop);
	DDX_Control(pDX, IDC_PARITY, m_ComboParity);
	DDX_Control(pDX, IDC_PORT, m_ComboPort);
	DDX_Control(pDX, IDC_DATA, m_ComboData);
	DDX_Control(pDX, IDC_BAUD, m_ComboBaud);
	DDX_Control(pDX, IDC_GPRS_BT, m_ComboBaud_GPRS);
	
	DDX_Text(pDX, IDC_SEND_EDIT, m_strSendEdit);
	DDX_Text(pDX, IDC_PHONE, m_number);
}

BEGIN_MESSAGE_MAP(CGPRS_TESTDlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_OPEN_COM, &CGPRS_TESTDlg::OnBnClickedOpenCom)
	ON_BN_CLICKED(IDC_CLOSE_COM, &CGPRS_TESTDlg::OnBnClickedCloseCom)
	ON_BN_CLICKED(IDC_CLEAR_REC, &CGPRS_TESTDlg::OnBnClickedClearRec)
	ON_BN_CLICKED(IDC_CALL, &CGPRS_TESTDlg::OnBnClickedCall)
	ON_BN_CLICKED(IDC_ANS, &CGPRS_TESTDlg::OnBnClickedAns)
	ON_BN_CLICKED(IDC_HOLD, &CGPRS_TESTDlg::OnBnClickedHold)
	ON_BN_CLICKED(IDC_SEND, &CGPRS_TESTDlg::OnBnClickedSend)
	ON_BN_CLICKED(IDC_CLEAR_SEND, &CGPRS_TESTDlg::OnBnClickedClearSend)
	ON_BN_CLICKED(IDC_BT_OK, &CGPRS_TESTDlg::OnBnClickedBtOk)
END_MESSAGE_MAP()


// CGPRS_TESTDlg ��Ϣ�������

BOOL CGPRS_TESTDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��


	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_ComboBaud.SetCurSel(1);			/* Ĭ�ϲ�����Ϊ: 9600 */
	m_ComboData.SetCurSel(1);			/* Ĭ������λΪ: 8λ */
	m_ComboParity.SetCurSel(0);			/* Ĭ��У��Ϊ: �� */
	m_ComboPort.SetCurSel(2);			/* Ĭ�϶˿�Ϊ: COM3 */
	m_ComboStop.SetCurSel(0);			/* Ĭ��ֹͣλΪ: 1λ */

	m_ButClose.EnableWindow(FALSE);		/* "�رն˿�"������Ч */
	m_hComm = INVALID_HANDLE_VALUE;		/* ���ڲ��������Ч */
	m_ExitThreadEvent = NULL;			/* ���ڽ����߳��˳��¼���Ч */
	m_strRecDisp = _T("");				/* ������ʾ�ַ�Ϊ�� */

	UpdateData(FALSE);
	m_ComboBaud_GPRS.SetCurSel(0);
	
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
void CGPRS_TESTDlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_GPRS_TEST_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_GPRS_TEST_DIALOG));
	}
}
#endif
BOOL CGPRS_TESTDlg::OpenPort(LPCTSTR Port, int BaudRate, int DataBits, int StopBits, int Parity)
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
		DWORD dwactlen;
		WriteFile(m_hComm,"AT\r\n", strlen("AT\r\n"),&dwactlen,NULL); 
		Sleep(10);
		WriteFile(m_hComm,"AT\r\n", strlen("AT\r\n"),&dwactlen,NULL); 
		

	PurgeComm(m_hComm, PURGE_TXCLEAR | PURGE_RXCLEAR);	 /* �����/�������� */		
	return TRUE;		
}


BOOL CGPRS_TESTDlg::ClosePort(void)
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


void CALLBACK CGPRS_TESTDlg::OnCommRecv(CWnd* pWnd, char *buf, int buflen)
{
	CString tmp;

	CGPRS_TESTDlg * pDlg = (CGPRS_TESTDlg*)pWnd;
	CEdit *pRecvStrEdit = (CEdit*)pDlg->GetDlgItem(IDC_REC_DISP);
	
	
														/* ȡ�ÿؼ�ָ�� */
	for (int i = 0; i < buflen; i++, buf++)
	{
		if((*buf)!='\0')
		{
			tmp.Format(_T("%c"), *buf);						/* ���ַ�ת��Ϊ�ַ��� */
			//printf("__%c",*buf);
			pDlg->m_strRecDisp += tmp;
		}
	}

	pRecvStrEdit->SetWindowText(pDlg->m_strRecDisp);	/* ��ʾ�ڴ����� */
	 int   nLen   =   pRecvStrEdit->GetWindowTextLength();
	pRecvStrEdit->SetSel(nLen,   nLen);
}


DWORD CGPRS_TESTDlg::CommRecvTread(LPVOID lparam)
{
	DWORD dwLength;
	char *recvBuf = new char[1024];
	CGPRS_TESTDlg *pDlg = (CGPRS_TESTDlg*)lparam;

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



void CGPRS_TESTDlg::OnBnClickedOpenCom()
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

void CGPRS_TESTDlg::OnBnClickedCloseCom()
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

void CGPRS_TESTDlg::OnBnClickedClearRec()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_strRecDisp = _T("");					
	SetDlgItemText(IDC_REC_DISP,m_strRecDisp);		/* ������������ַ� */
}

void CGPRS_TESTDlg::OnBnClickedCall()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(m_hComm == INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("����δ��."));
		return ;
	}

	UpdateData(TRUE);

	DWORD dwactlen;
	if(m_number.GetLength()!=11)
	{
		MessageBox(_T("�绰���볤�ȴ���."));
		return ;
	}
	char *psendbuf = new char[11];

	for(int i = 0; i < 11;i++)
	{
		psendbuf[i] = (char)m_number.GetAt(i);		 /* ת��Ϊ���ֽ��ַ� */
		//printf("%c",psendbuf[i]);
	}

    WriteFile(m_hComm,"ATD", strlen("ATD"),&dwactlen,NULL); 
	WriteFile(m_hComm,psendbuf, 11, &dwactlen,NULL);
	WriteFile(m_hComm,";\r\n",strlen(";\r\n"),&dwactlen,NULL);

	Sleep(10);
	
	delete[] psendbuf;
}

void CGPRS_TESTDlg::OnBnClickedAns()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(m_hComm == INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("����δ��."));
		return ;
	}
		UpdateData(TRUE);

	DWORD dwactlen;

    WriteFile(m_hComm,"ATA\r\n", strlen("ATA\r\n"),&dwactlen,NULL); 

	Sleep(10);
	
	
}

void CGPRS_TESTDlg::OnBnClickedHold()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(m_hComm == INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("����δ��."));
		return ;
	}
		UpdateData(TRUE);

	DWORD dwactlen;

    WriteFile(m_hComm,"ATH\r\n", strlen("ATH\r\n"),&dwactlen,NULL); 

	Sleep(10);


}

void CGPRS_TESTDlg::OnBnClickedSend()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(m_hComm == INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("����δ��."));
		return ;
	}
	UpdateData(TRUE);

	DWORD dwactlen;
	if(m_number.GetLength()!=11)
	{
		MessageBox(_T("�绰���볤�ȴ���."));
		return ;
	}
	char ctl[]={26,0};
	char *pnumbuf = new char[11];
	int len = m_strSendEdit.GetLength();				 /* ȡ�������ַ������� */
	len++;
	char *psendbuf = new char[len];
	

	for(int i = 0; i < 11;i++)
		pnumbuf[i] = (char)m_number.GetAt(i);		 /* ת��Ϊ���ֽ��ַ� */
	for(int i = 0; i < len;i++)
		psendbuf[i] = (char)m_strSendEdit.GetAt(i);		 /* ת��Ϊ���ֽ��ַ� */

	WriteFile(m_hComm,"AT+CMGF=1\r\n", strlen("AT+CMGF=1\r\n"),&dwactlen,NULL); //ATָ��Ͷ���Ϣ
	//printf("AT+CMGF__");
	Sleep(500);
	WriteFile(m_hComm,"AT+CMGS=", strlen("AT+CMGS="),&dwactlen,NULL);
	WriteFile(m_hComm,"\"+86", strlen("\"+86"),&dwactlen,NULL);
	WriteFile(m_hComm,pnumbuf, 11, &dwactlen,NULL);
	WriteFile(m_hComm,"\"", strlen("\""),&dwactlen,NULL);
	WriteFile(m_hComm,";\r\n",strlen(";\r\n"),&dwactlen,NULL);
	Sleep(500);
	WriteFile(m_hComm,psendbuf, len, &dwactlen,NULL);
	WriteFile(m_hComm,ctl,1,&dwactlen,NULL);
	UpdateData(TRUE);

	Sleep(10);
	
	delete[] pnumbuf;
	delete[] psendbuf;

}

void CGPRS_TESTDlg::OnBnClickedClearSend()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
		m_strSendEdit = _T("");							/* ������������ַ� */
	UpdateData(FALSE);
}
void CGPRS_TESTDlg::OnDestroy() 
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
void CGPRS_TESTDlg::OnBnClickedBtOk()//const DWORD Baud_GPRSl[7] = {0,4800, 9600, 19200, 38400, 57600,115200};	
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(m_hComm == INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("����δ��."));
		return ;
	}
		UpdateData(TRUE);

	DWORD dwactlen;
	
    WriteFile(m_hComm,"AT+IPR=", strlen("AT+IPR="),&dwactlen,NULL); 
	int baud2		= m_ComboBaud_GPRS.GetCurSel();
	switch(baud2)
	{
	case 0:
			WriteFile(m_hComm,"0", strlen("0"), &dwactlen,NULL);
			break;
	case 1:
			WriteFile(m_hComm,"4800", strlen("4800"), &dwactlen,NULL);
			break;
	case 2:
			WriteFile(m_hComm,"9600", strlen("9600"), &dwactlen,NULL);
			break;
	case 3:
			WriteFile(m_hComm,"19200", strlen("19200"), &dwactlen,NULL);
			break;
	case 4:
			WriteFile(m_hComm,"38400", strlen("38400"), &dwactlen,NULL);
			break;
	case 5:
			WriteFile(m_hComm,"57600", strlen("57600"), &dwactlen,NULL);
			break;
	case 6:
			WriteFile(m_hComm,"115200", strlen("115200"), &dwactlen,NULL);
			break;
	default:
		break;


	}
	WriteFile(m_hComm,"\r\n", strlen("\r\n"), &dwactlen,NULL);
	

}
