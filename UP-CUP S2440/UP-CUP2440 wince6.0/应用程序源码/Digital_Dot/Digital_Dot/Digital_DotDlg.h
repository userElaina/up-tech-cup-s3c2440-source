// Digital_DotDlg.h : 头文件
//

#pragma once

// CDigital_DotDlg 对话框
class CDigital_DotDlg : public CDialog
{
// 构造
public:
	CDigital_DotDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_DIGITAL_DOT_DIALOG };
	int		m_show;
	UINT	m_arrayshow;

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
	afx_msg void OnBnClickedTubrStart();
	afx_msg void OnBnClickedDotStart();
	afx_msg void OnBnClickedTubrStop();
	afx_msg void OnBnClickedDotStop();
};
