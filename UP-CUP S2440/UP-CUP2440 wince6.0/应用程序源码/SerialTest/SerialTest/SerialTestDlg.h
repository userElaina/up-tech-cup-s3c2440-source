// SerialTestDlg.h : ͷ�ļ�
//

#pragma once

// CSerialTestDlg �Ի���
class CSerialTestDlg : public CDialog
{
// ����
public:
	CSerialTestDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
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
	afx_msg void OnBnClickedClearRec();
	afx_msg void OnBnClickedSend();
	afx_msg void OnBnClickedClearSend();
	afx_msg void OnBnClickedOpenCom();
	afx_msg void OnBnClickedCloseCom();
	
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
