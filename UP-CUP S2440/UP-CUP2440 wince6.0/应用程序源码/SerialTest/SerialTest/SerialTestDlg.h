// SerialTestDlg.h : 头文件
//

#pragma once

// CSerialTestDlg 对话框
class CSerialTestDlg : public CDialog
{
// 构造
public:
	CSerialTestDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SERIALTEST_DIALOG };
	CButton	m_ButClose;
	CButton	m_ButOpen;
	CComboBox	m_ComboStop;
	CComboBox	m_ComboParity;
	CComboBox	m_ComboPort;
	CComboBox	m_ComboData;
	CComboBox	m_ComboBaud;
	CString	m_strSendEdit;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	afx_msg void OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/);
#endif
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedClearRec();
	afx_msg void OnBnClickedSend();
	afx_msg void OnBnClickedClearSend();
	afx_msg void OnBnClickedOpenCom();
	afx_msg void OnBnClickedCloseCom();
	
	afx_msg void OnDestroy();
	DCB dcb;						/* 串口参数结构体 */
	HANDLE m_hComm;					/* 串口操作句柄 */
	HANDLE m_ExitThreadEvent;		/* 串口接收线程退出事件 */
	CString m_strRecDisp;			/* 接收区显示字符 */
	
	BOOL ClosePort(void);			/* 关闭串口 */

	// 打开串口 
	BOOL OpenPort(LPCTSTR Port, int BaudRate, int DataBits, int StopBits, int Parity);

	// 串口接收线程
	static DWORD CommRecvTread(LPVOID lparam);

	// 串口接收数据成功回调函数
	typedef void (CALLBACK *ONCOMMRECV)(CWnd* pWnd, char *buf, int buflen);
	static void CALLBACK OnCommRecv(CWnd* pWnd, char *buf, int buflen);
};
