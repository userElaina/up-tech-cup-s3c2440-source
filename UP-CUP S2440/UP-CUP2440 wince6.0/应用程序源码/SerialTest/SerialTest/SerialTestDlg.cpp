// SerialTestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SerialTest.h"
#include "SerialTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSerialTestDlg 对话框

CSerialTestDlg::CSerialTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSerialTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_strSendEdit = _T("");
	//groupbox1.BackColor = Color.Transparent；

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


// CSerialTestDlg 消息处理程序

BOOL CSerialTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_ComboBaud.SetCurSel(5);			/* 默认波特率为: 115200 */
	m_ComboData.SetCurSel(1);			/* 默认数据位为: 8位 */
	m_ComboParity.SetCurSel(0);			/* 默认校验为: 无 */
	m_ComboPort.SetCurSel(0);			/* 默认端口为: COM1 */
	m_ComboStop.SetCurSel(0);			/* 默认停止位为: 1位 */

	m_ButClose.EnableWindow(FALSE);		/* "关闭端口"按键无效 */
	m_hComm = INVALID_HANDLE_VALUE;		/* 串口操作句柄无效 */
	m_ExitThreadEvent = NULL;			/* 串口接收线程退出事件无效 */
	m_strRecDisp = _T("");				/* 接收显示字符为空 */

	UpdateData(FALSE);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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
		
	PurgeComm(m_hComm, PURGE_TXCLEAR | PURGE_RXCLEAR);	 /* 清除收/发缓冲区 */		
	return TRUE;		
}


BOOL CSerialTestDlg::ClosePort(void)
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


void CALLBACK CSerialTestDlg::OnCommRecv(CWnd* pWnd, char *buf, int buflen)
{
	CString tmp;

	CSerialTestDlg * pDlg = (CSerialTestDlg*)pWnd;
	CEdit *pRecvStrEdit = (CEdit*)pDlg->GetDlgItem(IDC_REC_DISP);
														/* 取得控件指针 */
	for (int i = 0; i < buflen; i++, buf++)
	{
		if((*buf)!='\0')
		{
			tmp.Format(_T("%c"), *buf);						/* 将字符转换为字符串 */
			pDlg->m_strRecDisp += tmp;
		}
	}


	pRecvStrEdit->SetWindowText(pDlg->m_strRecDisp);	/* 显示在窗口上 */
	pRecvStrEdit->SetWindowText(pDlg->m_strRecDisp);	/* 显示在窗口上 */
	int   nLen   =   pRecvStrEdit->GetWindowTextLength(); 
	pRecvStrEdit->SetSel(nLen,   nLen);
}


DWORD CSerialTestDlg::CommRecvTread(LPVOID lparam)
{
	DWORD dwLength;
	char *recvBuf = new char[1024];
	CSerialTestDlg *pDlg = (CSerialTestDlg*)lparam;

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


void CSerialTestDlg::OnBnClickedClearRec()
{
	// TODO: 在此添加控件通知处理程序代码
	m_strRecDisp = _T("");					
	SetDlgItemText(IDC_REC_DISP,m_strRecDisp);		/* 清除接收区的字符 */
}

void CSerialTestDlg::OnBnClickedSend()
{
	// TODO: 在此添加控件通知处理程序代码
	DWORD dwactlen;

	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("串口未打开!"));
		return;
	}

	UpdateData(TRUE);
	int len = m_strSendEdit.GetLength();				 /* 取得输入字符串长度 */
	len++;
	char *psendbuf = new char[len];

	for(int i = 0; i < len;i++)
		psendbuf[i] = (char)m_strSendEdit.GetAt(i);		 /* 转换为单字节字符 */

	WriteFile(m_hComm, psendbuf, len, &dwactlen, NULL);	 /* 从串口发送数据 */
	
	delete[] psendbuf;
}

void CSerialTestDlg::OnBnClickedClearSend()
{
	// TODO: 在此添加控件通知处理程序代码
	m_strSendEdit = _T("");							/* 清除发送区的字符 */
	UpdateData(FALSE);
}

void CSerialTestDlg::OnBnClickedOpenCom()
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

void CSerialTestDlg::OnBnClickedCloseCom()
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


void CSerialTestDlg::OnDestroy() 
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