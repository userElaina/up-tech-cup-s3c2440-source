// LEDDlg.h : 头文件
//

#pragma once

// CLEDDlg 对话框
class CLEDDlg : public CDialog
{
// 构造
public:
	HANDLE gpiodriver;			//设备文件句柄
	CLEDDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_LED_DIALOG };


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
	afx_msg void OnBnClickedLed1On();
	afx_msg void OnBnClickedLed2On();
	afx_msg void OnBnClickedLed3On();
	afx_msg void OnBnClickedLedallOn();
	afx_msg void OnBnClickedLed1Off();
	afx_msg void OnBnClickedLed2Off();
	afx_msg void OnBnClickedLed3Off();
	afx_msg void OnBnClickedLedallOff();
};
