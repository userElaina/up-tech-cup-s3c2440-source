// ADCDlg.h : ͷ�ļ�
//

#pragma once

// CADCDlg �Ի���
class CADCDlg : public CDialog
{
// ����
public:
	CADCDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_ADC_DIALOG };
	
	float	m_fCH1;
	float	m_fCH2;
	float	m_fCH3;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	 
	afx_msg void OnButtonStart();
	afx_msg void OnButtonStop();
	afx_msg void OnDestroy();

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	afx_msg void OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/);
#endif
	DECLARE_MESSAGE_MAP()

};
