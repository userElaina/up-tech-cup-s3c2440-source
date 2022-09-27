// webcamDlg.h : 头文件
//

#pragma once

// CwebcamDlg 对话框
class CwebcamDlg : public CDialog
{
// 构造
public:
	CwebcamDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_WEBCAM_DIALOG };
	CButton	m_ctrlSave2File;
	CButton	m_ctrlFullScreen;
	CButton	m_ctrlExit;
	CButton	m_ctrlStop;
	CButton	m_ctrlStart;
	CComboBox	m_ctrlCamList;

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
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedExit();
	afx_msg void OnBnClickedFullscreen();
	afx_msg void OnBnClickedSaveToFile();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnClose();
private:
	HWND m_hDisplay;
	HANDLE m_hThread;
	BOOL m_fIsStop;
	HANDLE m_hStopEvent;
	BOOL m_fIsSave;
	static DWORD WINAPI CaptureThreadProc(LPVOID lpParameter);
	static LRESULT CALLBACK CaptureWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL CreateBitmapFromMem(LPCTSTR bmpFileName, DWORD width, DWORD height, WORD bitCount, LPBYTE pBmpData);
	BOOL MakeJpeg(LPCTSTR jpgFileName, LPBYTE pJpgData, DWORD size);
};
