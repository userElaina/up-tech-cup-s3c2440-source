// GPS_TESTDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "GPS_TEST.h"
#include "GPS_TESTDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//HANDLE hGps=INVALID_HANDLE_VALUE;
//ANDLE hGprs=INVALID_HANDLE_VALUE;

//HANDLE htRcvThread=INVALID_HANDLE_VALUE;
//ANDLE htShowThread=INVALID_HANDLE_VALUE;
char* recivedata;
DWORD dwTStat;
BOOL bStopFlag=FALSE;
BOOL GET_GPS_OK=FALSE;
char GPS_BUF[1024];
GPS_INFO gps_info;
// CGPS_TESTDlg �Ի���

CGPS_TESTDlg::CGPS_TESTDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGPS_TESTDlg::IDD, pParent)
{
		m_year = 0;
	m_month = 0;
	m_day = 0;
	m_hour = 0;
	m_minute = 0;
	m_second = 0;
	m_latitude = 0.0;
	m_longitude = 0.0;
	m_speed = 0.0;
	m_high = 0.0;
	//m_number = _T("");
	m_NS = _T("");
	m_EW = _T("");
	m_status = _T("");
	//m_cMessage = _T("");
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGPS_TESTDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CLOSE_COM, m_ButClose);
	DDX_Control(pDX, IDC_OPEN_COM, m_ButOpen);
	DDX_Control(pDX, IDC_STOP, m_ComboStop);
	DDX_Control(pDX, IDC_PARITY, m_ComboParity);
	DDX_Control(pDX, IDC_PORT, m_ComboPort);
	DDX_Control(pDX, IDC_DATA, m_ComboData);
	DDX_Control(pDX, IDC_BAUD, m_ComboBaud);
	//DDX_Text(pDX, IDC_SEND_EDIT, m_strSendEdit);

	//DDX_Control(pDX, IDC_START, m_BtStart);
	//DDX_Control(pDX, IDC_STOP, m_EditControl);
	//DDX_Control(pDX, IDC_EDIT1, m_EditRcvControl);
	DDX_Text(pDX, IDC_YEAR, m_year);
	DDX_Text(pDX, IDC_MOUN, m_month);
	DDX_Text(pDX, IDC_DAY, m_day);
	DDX_Text(pDX, IDC_HOUR, m_hour);
	DDX_Text(pDX, IDC_MIN, m_minute);
	DDX_Text(pDX, IDC_SEC, m_second);
	DDX_Text(pDX, IDC_LATI, m_latitude);
	DDX_Text(pDX, IDC_LONG, m_longitude);
	DDX_Text(pDX, IDC_SPEED, m_speed);
	DDX_Text(pDX, IDC_HIGH, m_high);
	//DDX_Text(pDX, IDC_EDIT15, m_number);
	DDX_Text(pDX, IDC_NS, m_NS);
	DDX_Text(pDX, IDC_EW, m_EW);
	DDX_Text(pDX, IDC_STATE, m_status);



}

BEGIN_MESSAGE_MAP(CGPS_TESTDlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_OPEN_COM, &CGPS_TESTDlg::OnBnClickedOpenCom)
	ON_BN_CLICKED(IDC_CLOSE_COM, &CGPS_TESTDlg::OnBnClickedCloseCom)
	ON_BN_CLICKED(IDC_CLEAR_REC, &CGPS_TESTDlg::OnBnClickedClearRec)
END_MESSAGE_MAP()


// CGPS_TESTDlg ��Ϣ�������

BOOL CGPS_TESTDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_ComboBaud.SetCurSel(0);			/* Ĭ�ϲ�����Ϊ: 4800 */
	m_ComboData.SetCurSel(1);			/* Ĭ������λΪ: 8λ */
	m_ComboParity.SetCurSel(0);			/* Ĭ��У��Ϊ: �� */
	m_ComboPort.SetCurSel(2);			/* Ĭ�϶˿�Ϊ: COM3 */
	m_ComboStop.SetCurSel(0);			/* Ĭ��ֹͣλΪ: 1λ */

	m_ButClose.EnableWindow(FALSE);		/* "�رն˿�"������Ч */
	m_hComm = INVALID_HANDLE_VALUE;		/* ���ڲ��������Ч */
	m_ExitThreadEvent = NULL;			/* ���ڽ����߳��˳��¼���Ч */
	m_strRecDisp = _T("");				/* ������ʾ�ַ�Ϊ�� */

	
	UpdateData(FALSE);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
void CGPS_TESTDlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_GPS_TEST_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_GPS_TEST_DIALOG));
	}
}
#endif

BOOL CGPS_TESTDlg::OpenPort(LPCTSTR Port, int BaudRate, int DataBits, int StopBits, int Parity)
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

	
		//bStopFlag=FALSE;                   //��ֹͣ��־λΪFALSE
		this->UpdateData();				
		m_ButOpen.EnableWindow(FALSE);
		//m_EditControl.SetReadOnly(TRUE);
		//m_EditRcvControl.SetReadOnly(TRUE);
		//AfxBeginThread(CommRcv,LPVOID(this));	//����GPS���ݲɼ��߳�
//		AfxBeginThread(ShowGpsInfo,LPVOID(this));
		
	
	return TRUE;		
}


BOOL CGPS_TESTDlg::ClosePort(void)
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
// ���崮�����ò������
const CString PorTbl[6] = {_T("COM1:"),_T("COM2:"),_T("COM3:"),_T("COM4:"),_T("COM5:"), _T("COM6:")};
const DWORD BaudTbl[6] = {4800, 9600, 19200, 38400, 57600,115200};	
const DWORD DataBitTbl[2] = {7, 8};
const BYTE  StopBitTbl[3] = {ONESTOPBIT, ONE5STOPBITS, TWOSTOPBITS};
const BYTE  ParityTbl[4] = {NOPARITY, ODDPARITY, EVENPARITY, MARKPARITY};


void CGPS_TESTDlg::OnBnClickedOpenCom()
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

void CGPS_TESTDlg::OnBnClickedCloseCom()
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

void CGPS_TESTDlg::OnBnClickedClearRec()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_strRecDisp = _T("");					
	SetDlgItemText(IDC_REC_DISP,m_strRecDisp);		/* ������������ַ� */
}
void CGPS_TESTDlg::OnDestroy() 
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

void CALLBACK CGPS_TESTDlg::OnCommRecv(CWnd* pWnd, char *buf, int buflen)
{
	CString tmp;

	CGPS_TESTDlg * pDlg = (CGPS_TESTDlg*)pWnd;
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
	int   nLen   =   pRecvStrEdit->GetWindowTextLength(); 
	pRecvStrEdit->SetSel(nLen,   nLen);
}
BOOL gps_parse(char *line,GPS_INFO *GPS)         //����GPS����
////////////////////////////////////////////////////////////////////////////////
{
	int i,tmp,start,end;
	char c;
	char* buf=line;
	c=buf[5];
	

	if(c=='C' && buf[4]=='M'){//"GPRMC"                  //�ж��Ƿ�Ϊ��Ч����
		GPS->D.hour   =(buf[ 7]-'0')*10+(buf[ 8]-'0');
		GPS->D.minute =(buf[ 9]-'0')*10+(buf[10]-'0');
		GPS->D.second =(buf[11]-'0')*10+(buf[12]-'0');            //��������ʱ��
		tmp = GetComma(9,buf);
		//printf("tmp=%d\n",tmp);
		if(buf[tmp+0]!=',')
		{
		GPS->D.day    =(buf[tmp+0]-'0')*10+(buf[tmp+1]-'0');
		GPS->D.month  =(buf[tmp+2]-'0')*10+(buf[tmp+3]-'0');
		GPS->D.year   =(buf[tmp+4]-'0')*10+(buf[tmp+5]-'0')+2000;
		}
		//------------------------------
		GPS->status	  =buf[GetComma(2,buf)];                      //������λ����
		GPS->latitude =get_double_number(&buf[GetComma(3,buf)]);
		GPS->NS       =buf[GetComma(4,buf)];
		GPS->longitude=get_double_number(&buf[GetComma(5,buf)]);
		GPS->EW       =buf[GetComma(6,buf)];
		//printf("GPRMC\n");
		return TRUE;                                         //������Ч������

	}
	else if(c=='A'){ //"$GPGGA"
		GPS->high     = get_double_number(&buf[GetComma(9,buf)]);
		return TRUE;
		
	}
	else
		return FALSE;                                      //������Ч���ؼ�
     
	
}

static double get_double_number(char *s)
{
	char buf[128];
	int i;
	double rev;
	i=GetComma(1,s);
	strncpy(buf,s,i);
	buf[i]=0;
	rev=atof(buf);
//	printf("s=%s ,buf= %s,val= %f\n",s,buf,rev);
	return rev;
	
}
////////////////////////////////////////////////////////////////////////////////
//�õ�ָ����ŵĶ���λ��
static int GetComma(int num,char *str)
{
	int i,j=0;
	int len=strlen(str);
	for(i=0;i<len;i++)
	{
		if(str[i]==',')j++;
		if(j==num)return i+1;	
	}
	return 0;	
}

//////////////////////////////////////////////////////////////

UINT ShowGpsInfo(LPVOID ra)
{
	CGPS_TESTDlg *pGpxDlg=(CGPS_TESTDlg *)ra;
	TCHAR TReadBuffer[1024] = {0};
	while(1)
	{
		if(GET_GPS_OK)
		{
			GET_GPS_OK=FALSE;
			gps_parse(GPS_BUF,&gps_info);
//			show_gps(&gps_info);
			MultiByteToWideChar(CP_ACP,0,(char *)GPS_BUF,sizeof(GPS_BUF),TReadBuffer,sizeof(GPS_BUF));
//			pGpxDlg->m_EditRcvControl.ReplaceSel(TReadBuffer);
			for(int i=0;i < sizeof(GPS_BUF)+2;i++)
			{
				TReadBuffer[i] = 0;
			}

		}
		Sleep(200);
		
		if(bStopFlag)break;
	}
	return 1;
}



DWORD CGPS_TESTDlg::CommRecvTread(LPVOID lparam)
{
	BYTE c;
	int j=0;
	char ReadBuffer[1000]={0};
	char TReadBuffer[1000] = {0};
	//CGpxtextDlg *pGpxDlg=(CGpxtextDlg *)ra;
	DWORD nReadCount;
	BOOL head=FALSE;

	DWORD dwLength;
	char *recvBuf = new char[1024];
	CGPS_TESTDlg *pDlg = (CGPS_TESTDlg*)lparam;

	while(TRUE)
	{																/* �ȴ��߳��˳��¼� */
		if (WaitForSingleObject(pDlg->m_ExitThreadEvent, 0) == WAIT_OBJECT_0)
			break;	

		if (pDlg->m_hComm != INVALID_HANDLE_VALUE)
		{															/* �Ӵ��ڶ�ȡ���� */
			BOOL fReadState = ReadFile(pDlg->m_hComm, recvBuf, 1024, &dwLength, NULL);
			for(int i=0;i<dwLength;i++)
			{
				printf("%c",recvBuf[i]);
				GPS_BUF[i]=recvBuf[i];
			}
			//BOOL fReadState =ReadFile(pDlg->m_hComm,&ReadBuffer,1,&nReadCount,NULL);
			if(!fReadState)
			{
				//MessageBox(_T("�޷��Ӵ��ڶ�ȡ����!"));
			}
			else
			{
				
				if(recvBuf[0]=='$')
				{
					
					//printf("ReadBuffer%c",recvBuf[0]);
				
				
					if(gps_parse(GPS_BUF,&gps_info))                //��������
					{
						pDlg->m_year=gps_info.D.year;
						pDlg->m_month=gps_info.D.month;
						pDlg->m_day=gps_info.D.day;
						pDlg->m_hour=gps_info.D.hour;
						pDlg->m_minute=gps_info.D.minute;
						pDlg->m_second=gps_info.D.second;
						pDlg->m_status=gps_info.status;
						pDlg->m_latitude=gps_info.latitude/100;
						pDlg->m_longitude=gps_info.longitude/100;
						pDlg->m_speed=gps_info.speed;
						pDlg->m_high=gps_info.high;	
						pDlg->m_NS=gps_info.NS;
						pDlg->m_EW=gps_info.EW;
						pDlg->UpdateData(FALSE);                //��ʾ����
					}
					
					OnCommRecv(pDlg, recvBuf, dwLength);			/* ���ճɹ����ûص����� */
						for(int i=0;i<(dwLength+2);i++)
					{
						ReadBuffer[i]=0;
						TReadBuffer[i]=0;

					}
				}
			}
				
			
		}
	}

	delete[] recvBuf;
	return 0;
}


