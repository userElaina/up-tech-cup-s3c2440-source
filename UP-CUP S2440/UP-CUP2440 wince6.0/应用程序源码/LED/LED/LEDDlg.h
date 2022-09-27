// LEDDlg.h : ͷ�ļ�
//

#pragma once

// CLEDDlg �Ի���
class CLEDDlg : public CDialog
{
// ����
public:
	HANDLE gpiodriver;			//�豸�ļ����
	CLEDDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_LED_DIALOG };


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
	afx_msg void OnBnClickedLed1On();
	afx_msg void OnBnClickedLed2On();
	afx_msg void OnBnClickedLed3On();
	afx_msg void OnBnClickedLedallOn();
	afx_msg void OnBnClickedLed1Off();
	afx_msg void OnBnClickedLed2Off();
	afx_msg void OnBnClickedLed3Off();
	afx_msg void OnBnClickedLedallOff();
};
