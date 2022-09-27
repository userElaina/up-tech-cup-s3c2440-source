// GPS_TESTDlg.h : ͷ�ļ�
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
	 date_time D;//ʱ��
	 char status;  		//����״̬
	 double	latitude;   //γ��
	 double longitude;  //����
	 char NS;           //�ϱ���
	 char EW;           //����
	 double speed;      //�ٶ�
	 double high;       //�߶�
}GPS_INFO;

//UINT CommRcv(LPVOID ra);
UINT ShowGpsInfo(LPVOID ra);
BOOL gps_parse(char *line,GPS_INFO *GPS);
static double get_double_number(char *s);
static int GetComma(int num,char *str);
// CGPS_TESTDlg �Ի���
class CGPS_TESTDlg : public CDialog
{
// ����
public:
	CGPS_TESTDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
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
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
	DCB dcb;						/* ���ڲ����ṹ�� */
	HANDLE m_hComm;					/* ���ڲ������ */
	HANDLE m_ExitThreadEvent;		/* ���ڽ����߳��˳��¼� */
	CString m_strRecDisp;			/* ��������ʾ�ַ� */
	
	BOOL ClosePort(void);			/* �رմ��� */

	// �򿪴��� 
	BOOL OpenPort(LPCTSTR Port, int BaudRate, int DataBits, int StopBits, int Parity);

	// ���ڽ����߳�
	static DWORD CommRecvTread(LPVOID lparam);

	// ���ڽ������ݳɹ��ص�����
	typedef void (CALLBACK *ONCOMMRECV)(CWnd* pWnd, char *buf, int buflen);
	static void CALLBACK OnCommRecv(CWnd* pWnd, char *buf, int buflen);
};
