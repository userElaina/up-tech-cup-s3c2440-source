// GPRS_TESTDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "GPRS_TEST.h"
#include "GPRS_TESTDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CGPRS_TESTDlg 对话框

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


// CGPRS_TESTDlg 消息处理程序

BOOL CGPRS_TESTDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标


	// TODO: 在此添加额外的初始化代码
	m_ComboBaud.SetCurSel(1);			/* 默认波特率为: 9600 */
	m_ComboData.SetCurSel(1);			/* 默认数据位为: 8位 */
	m_ComboParity.SetCurSel(0);			/* 默认校验为: 无 */
	m_ComboPort.SetCurSel(2);			/* 默认端口为: COM3 */
	m_ComboStop.SetCurSel(0);			/* 默认停止位为: 1位 */

	m_ButClose.EnableWindow(FALSE);		/* "关闭端口"按键无效 */
	m_hComm = INVALID_HANDLE_VALUE;		/* 串口操作句柄无效 */
	m_ExitThreadEvent = NULL;			/* 串口接收线程退出事件无效 */
	m_strRecDisp = _T("");				/* 接收显示字符为空 */

	UpdateData(FALSE);
	m_ComboBaud_GPRS.SetCurSel(0);
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

	// 打开串口
	m_hComm = CreateFile(Port, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	if(m_hComm == INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("无法打开端口或端口已打开!请检查是否已被占用."));
		return FALSE;
	}

	GetCommState(m_hComm, &dcb);						/* 读取串口的DCB */
	dcb.BaudRate = BaudRate;			
	dcb.ByteSize = DataBits;
	dcb.Parity = Parity;
	dcb.StopBits = StopBits;	
	dcb.fParity = FALSE;								/* 禁止奇偶校验 */
	dcb.fBinary = TRUE;
	dcb.fDtrControl = 0;								/* 禁止流量控制 */
	dcb.fRtsControl = 0;
	dcb.fOutX = 0;
	dcb.fInX = 0;
	dcb.fTXContinueOnXoff = 0;
	
	//设置状态参数
	SetCommMask(m_hComm, EV_RXCHAR);					/* 串口事件:接收到一个字符 */	
	SetupComm(m_hComm, 16384, 16384);					/* 设置接收与发送的缓冲区大小 */
	if(!SetCommState(m_hComm, &dcb))					/* 设置串口的DCB */
	{
		MessageBox(_T("无法按当前参数配置端口，请检查参数!"));
		ClosePort();
		return FALSE;
	}
		
	//设置超时参数
	GetCommTimeouts(m_hComm, &CommTimeOuts);		
	CommTimeOuts.ReadIntervalTimeout = 100;				/* 接收字符间最大时间间隔 */
	CommTimeOuts.ReadTotalTimeoutMultiplier = 1;		
	CommTimeOuts.ReadTotalTimeoutConstant = 100;		/* 读数据总超时常量 */
	CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
	CommTimeOuts.WriteTotalTimeoutConstant = 0;		
	if(!SetCommTimeouts(m_hComm, &CommTimeOuts))
	{
		MessageBox(_T("无法设置超时参数!"));
		ClosePort();
		return FALSE;
	}
		DWORD dwactlen;
		WriteFile(m_hComm,"AT\r\n", strlen("AT\r\n"),&dwactlen,NULL); 
		Sleep(10);
		WriteFile(m_hComm,"AT\r\n", strlen("AT\r\n"),&dwactlen,NULL); 
		

	PurgeComm(m_hComm, PURGE_TXCLEAR | PURGE_RXCLEAR);	 /* 清除收/发缓冲区 */		
	return TRUE;		
}


BOOL CGPRS_TESTDlg::ClosePort(void)
{
	if(m_hComm != INVALID_HANDLE_VALUE)
	{
		SetCommMask(m_hComm, 0);		
		PurgeComm(m_hComm, PURGE_TXCLEAR | PURGE_RXCLEAR);	/* 清除收/发缓冲 */
		CloseHandle(m_hComm);								/* 关闭串口操作句柄 */
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
	
	
														/* 取得控件指针 */
	for (int i = 0; i < buflen; i++, buf++)
	{
		if((*buf)!='\0')
		{
			tmp.Format(_T("%c"), *buf);						/* 将字符转换为字符串 */
			//printf("__%c",*buf);
			pDlg->m_strRecDisp += tmp;
		}
	}

	pRecvStrEdit->SetWindowText(pDlg->m_strRecDisp);	/* 显示在窗口上 */
	 int   nLen   =   pRecvStrEdit->GetWindowTextLength();
	pRecvStrEdit->SetSel(nLen,   nLen);
}


DWORD CGPRS_TESTDlg::CommRecvTread(LPVOID lparam)
{
	DWORD dwLength;
	char *recvBuf = new char[1024];
	CGPRS_TESTDlg *pDlg = (CGPRS_TESTDlg*)lparam;

	while(TRUE)
	{																/* 等待线程退出事件 */
		if (WaitForSingleObject(pDlg->m_ExitThreadEvent, 0) == WAIT_OBJECT_0)
			break;	

		if (pDlg->m_hComm != INVALID_HANDLE_VALUE)
		{															/* 从串口读取数据 */
			BOOL fReadState = ReadFile(pDlg->m_hComm, recvBuf, 1024, &dwLength, NULL);
			if(!fReadState)
			{
				//MessageBox(_T("无法从串口读取数据!"));
			}
			else
			{
					for(int i=0;i<dwLength;i++)
				{
					printf("%c",recvBuf[i]);
				}
					
				if(dwLength != 0)
					OnCommRecv(pDlg, recvBuf, dwLength);			/* 接收成功调用回调函数 */
			}
		}
	}		

	delete[] recvBuf;
	return 0;
}


// 定义串口设置参数表格
const CString PorTbl[6] = {_T("COM1:"),_T("COM2:"),_T("COM3:"),_T("COM4:"),_T("COM5:"), _T("COM6:")};
const DWORD BaudTbl[6] = {4800, 9600, 19200, 38400, 57600,115200};	
const DWORD DataBitTbl[2] = {7, 8};
const BYTE  StopBitTbl[3] = {ONESTOPBIT, ONE5STOPBITS, TWOSTOPBITS};
const BYTE  ParityTbl[4] = {NOPARITY, ODDPARITY, EVENPARITY, MARKPARITY};



void CGPRS_TESTDlg::OnBnClickedOpenCom()
{
	// TODO: 在此添加控件通知处理程序代码
	DWORD IDThread;
	HANDLE hRecvThread;												/* 接收线程句柄 */
	UpdateData(TRUE);

	CString strPort = PorTbl[m_ComboPort.GetCurSel()];				/* 查表获取参数值 */
	DWORD baud		= BaudTbl[m_ComboBaud.GetCurSel()];
	DWORD databit   = DataBitTbl[m_ComboData.GetCurSel()];
	BYTE stopbit    = StopBitTbl[m_ComboStop.GetCurSel()];
	BYTE parity		= ParityTbl[m_ComboParity.GetCurSel()];

	BOOL ret = OpenPort(strPort, baud, databit, stopbit, parity);	/* 打开串口 */
	if (ret == FALSE)
		return;

	m_ExitThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);		/* 创建串口接收线程退出事件*/

	// 创建串口接收线程
	hRecvThread = CreateThread(0, 0, CommRecvTread, this, 0, &IDThread);
	if (hRecvThread == NULL) 
	{
		MessageBox(_T("创建接收线程失败!"));
		return;
	}	
	CloseHandle(hRecvThread);

	m_ButOpen.EnableWindow(FALSE);									/* 打开端口按键禁止 */				
	m_ButClose.EnableWindow(TRUE);									/* 关闭端口按键使能 */	
	MessageBox(_T("打开") + strPort + _T("成功!"));
}

void CGPRS_TESTDlg::OnBnClickedCloseCom()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_ExitThreadEvent != NULL)
	{
		SetEvent(m_ExitThreadEvent);					/* 通知线程退出 */
		Sleep(1000);
		CloseHandle(m_ExitThreadEvent);
		m_ExitThreadEvent = NULL;
	}

	m_ButOpen.EnableWindow(TRUE);						/* 打开端口按键禁止 */				
	m_ButClose.EnableWindow(FALSE);						/* 关闭端口按键使能 */	
	ClosePort();
}

void CGPRS_TESTDlg::OnBnClickedClearRec()
{
	// TODO: 在此添加控件通知处理程序代码
	m_strRecDisp = _T("");					
	SetDlgItemText(IDC_REC_DISP,m_strRecDisp);		/* 清除接收区的字符 */
}

void CGPRS_TESTDlg::OnBnClickedCall()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_hComm == INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("串口未打开."));
		return ;
	}

	UpdateData(TRUE);

	DWORD dwactlen;
	if(m_number.GetLength()!=11)
	{
		MessageBox(_T("电话号码长度错误！."));
		return ;
	}
	char *psendbuf = new char[11];

	for(int i = 0; i < 11;i++)
	{
		psendbuf[i] = (char)m_number.GetAt(i);		 /* 转换为单字节字符 */
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
	// TODO: 在此添加控件通知处理程序代码
	if(m_hComm == INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("串口未打开."));
		return ;
	}
		UpdateData(TRUE);

	DWORD dwactlen;

    WriteFile(m_hComm,"ATA\r\n", strlen("ATA\r\n"),&dwactlen,NULL); 

	Sleep(10);
	
	
}

void CGPRS_TESTDlg::OnBnClickedHold()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_hComm == INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("串口未打开."));
		return ;
	}
		UpdateData(TRUE);

	DWORD dwactlen;

    WriteFile(m_hComm,"ATH\r\n", strlen("ATH\r\n"),&dwactlen,NULL); 

	Sleep(10);


}

void CGPRS_TESTDlg::OnBnClickedSend()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_hComm == INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("串口未打开."));
		return ;
	}
	UpdateData(TRUE);

	DWORD dwactlen;
	if(m_number.GetLength()!=11)
	{
		MessageBox(_T("电话号码长度错误！."));
		return ;
	}
	char ctl[]={26,0};
	char *pnumbuf = new char[11];
	int len = m_strSendEdit.GetLength();				 /* 取得输入字符串长度 */
	len++;
	char *psendbuf = new char[len];
	

	for(int i = 0; i < 11;i++)
		pnumbuf[i] = (char)m_number.GetAt(i);		 /* 转换为单字节字符 */
	for(int i = 0; i < len;i++)
		psendbuf[i] = (char)m_strSendEdit.GetAt(i);		 /* 转换为单字节字符 */

	WriteFile(m_hComm,"AT+CMGF=1\r\n", strlen("AT+CMGF=1\r\n"),&dwactlen,NULL); //AT指令发送短消息
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
	// TODO: 在此添加控件通知处理程序代码
		m_strSendEdit = _T("");							/* 清除发送区的字符 */
	UpdateData(FALSE);
}
void CGPRS_TESTDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	if (m_ExitThreadEvent != NULL)
	{
		SetEvent(m_ExitThreadEvent);				/* 通知串口接收线程退出 */
		Sleep(1000);
		CloseHandle(m_ExitThreadEvent);				/* 关闭接收线程退出事件句柄 */
		m_ExitThreadEvent = NULL;
	}

	ClosePort();									/* 关闭串口 */
}
void CGPRS_TESTDlg::OnBnClickedBtOk()//const DWORD Baud_GPRSl[7] = {0,4800, 9600, 19200, 38400, 57600,115200};	
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_hComm == INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("串口未打开."));
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
