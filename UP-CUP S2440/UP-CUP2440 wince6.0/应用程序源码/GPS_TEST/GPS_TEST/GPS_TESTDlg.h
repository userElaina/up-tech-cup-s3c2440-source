// GPS_TESTDlg.h : 头文件
//


#pragma once

typedef struct{
	int year;  
	int month; 
	int day;
	int hour;
	int minute;
	int second;
}date_time;

typedef struct{
	 date_time D;//时间
	 char status;  		//接收状态
	 double	latitude;   //纬度
	 double longitude;  //经度
	 char NS;           //南北极
	 char EW;           //东西
	 double speed;      //速度
	 double high;       //高度
}GPS_INFO;

//UINT CommRcv(LPVOID ra);
UINT ShowGpsInfo(LPVOID ra);
BOOL gps_parse(char *line,GPS_INFO *GPS);
static double get_double_number(char *s);
static int GetComma(int num,char *str);
// CGPS_TESTDlg 对话框
class CGPS_TESTDlg : public CDialog
{
// 构造
public:
	CGPS_TESTDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_GPS_TEST_DIALOG };
	CButton	m_ButClose;
	CButton	m_ButOpen;
	CComboBox	m_ComboStop;
	CComboBox	m_ComboParity;
	CComboBox	m_ComboPort;
	CComboBox	m_ComboData;
	CComboBox	m_ComboBaud;
	//CString	m_strSendEdit;


	//CButton	m_BtStart;
	//CEdit	m_EditControl;
	//CEdit	m_EditRcvControl;
	int		m_year;
	int		m_month;
	int		m_day;
	int		m_hour;
	int		m_minute;
	int		m_second;
	double	m_latitude;
	double	m_longitude;
	double	m_speed;
	double	m_high;
	CString	m_number;
	CString	m_NS;
	CString	m_EW;
	CString	m_status;
	//CString	m_cMessage;

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
	afx_msg void OnBnClickedOpenCom();
	afx_msg void OnBnClickedCloseCom();
	afx_msg void OnBnClickedClearRec();
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
