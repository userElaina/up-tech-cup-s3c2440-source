// Digital_DotDlg.h : ͷ�ļ�
//

#pragma once

// CDigital_DotDlg �Ի���
class CDigital_DotDlg : public CDialog
{
// ����
public:
	CDigital_DotDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_DIGITAL_DOT_DIALOG };
	int		m_show;
	UINT	m_arrayshow;

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
	afx_msg void OnBnClickedTubrStart();
	afx_msg void OnBnClickedDotStart();
	afx_msg void OnBnClickedTubrStop();
	afx_msg void OnBnClickedDotStop();
};
