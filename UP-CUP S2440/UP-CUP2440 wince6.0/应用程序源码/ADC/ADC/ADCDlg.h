// ADCDlg.h : 头文件
//

#pragma once

// CADCDlg 对话框
class CADCDlg : public CDialog
{
// 构造
public:
	CADCDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_ADC_DIALOG };
	
	float	m_fCH1;
	float	m_fCH2;
	float	m_fCH3;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	 
	afx_msg void OnButtonStart();
	afx_msg void OnButtonStop();
	afx_msg void OnDestroy();

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	afx_msg void OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/);
#endif
	DECLARE_MESSAGE_MAP()

};
