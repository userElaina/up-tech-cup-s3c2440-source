// dcmotorDlg.h : ͷ�ļ�
//

#pragma once

// CdcmotorDlg �Ի���
class CdcmotorDlg : public CDialog
{
// ����
public:
	CdcmotorDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_DCMOTOR_DIALOG };


	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
		afx_msg void OnClockwise();
	afx_msg void OnEastern();
	afx_msg void OnStop();
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	afx_msg void OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/);
#endif
	DECLARE_MESSAGE_MAP()
public:
	
};
