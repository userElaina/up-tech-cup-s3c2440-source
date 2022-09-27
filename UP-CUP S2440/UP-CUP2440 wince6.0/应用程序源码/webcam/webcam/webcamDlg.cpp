// webcamDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "webcam.h"
#include "webcamDlg.h"
#include "../SDK/inc/zc030xlib.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ARMV4I 1
// PLS select a lib
#ifdef ARMV4
#pragma comment(lib, "../SDK/lib/armv4/zc030xlib.lib")
#elif defined(ARMV4I)
#pragma comment(lib, "../SDK/lib/armv4i/zc030xlib.lib")
#elif defined(_X86_)
#pragma comment(lib, "../SDK/lib/x86/zc030xlib.lib")
#elif defined(MIPSII)
#pragma comment(lib, "../SDK/lib/mipsii/zc030xlib.lib")
#elif defined(SH4)
#pragma comment(lib, "../SDK/lib/sh4/zc030xlib.lib")
#endif


// CwebcamDlg 对话框

CwebcamDlg::CwebcamDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CwebcamDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hDisplay = NULL;
	m_hThread = NULL;
	m_fIsStop = TRUE;
	m_hStopEvent = NULL;
	m_fIsSave = FALSE;
}

void CwebcamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SAVE_TO_FILE, m_ctrlSave2File);
	DDX_Control(pDX, IDC_FULLSCREEN, m_ctrlFullScreen);
	DDX_Control(pDX, IDC_EXIT, m_ctrlExit);
	DDX_Control(pDX, IDC_STOP, m_ctrlStop);
	DDX_Control(pDX, IDC_START, m_ctrlStart);
	DDX_Control(pDX, IDC_CAM_LIST, m_ctrlCamList);
}

BEGIN_MESSAGE_MAP(CwebcamDlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_START, &CwebcamDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_STOP, &CwebcamDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_EXIT, &CwebcamDlg::OnBnClickedExit)
	ON_BN_CLICKED(IDC_FULLSCREEN, &CwebcamDlg::OnBnClickedFullscreen)
	ON_BN_CLICKED(IDC_SAVE_TO_FILE, &CwebcamDlg::OnBnClickedSaveToFile)
END_MESSAGE_MAP()


// CwebcamDlg 消息处理程序

BOOL CwebcamDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	
	int i = capInitCamera();
	if (i > 0)
	{
		for (int j = 0; j < i; j ++)
		{
			CString strCam;
			strCam.Format(TEXT("Num %d"), j);

			m_ctrlCamList.AddString(strCam);
		}

		m_ctrlCamList.SetCurSel(0);

		TCHAR szText[128];

		memset(szText, 0, sizeof (szText));		
		if (capGetCurrentVersion(0, (unsigned char *)szText, sizeof (szText)))
		{
			MessageBox(szText, TEXT("Driver Version"), MB_OK | MB_TOPMOST);
		}

		// Create video window
		WNDCLASS wc;
		LPTSTR lpszClassName = TEXT("Display Window Class");

		memset(&wc, 0, sizeof (wc));
		wc.lpszClassName = lpszClassName;
		wc.lpfnWndProc = CaptureWindowProc;
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		
		RegisterClass(&wc);
		// Create a display window
		m_hDisplay = CreateWindow(lpszClassName, TEXT("Capture Window"), WS_VISIBLE, 40, 45, 320, 240, this->GetSafeHwnd(), NULL, NULL, this);
		if (m_hDisplay != NULL)
		{		
			::SetWindowPos(m_hDisplay, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

			// Enable start button
			m_ctrlStart.EnableWindow();

			m_hStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		}
	}
	else
	{
		TCHAR szText[128];
		switch (i)
		{
		case INVALID_MACHINE_ID:
			wsprintf(szText, TEXT("Machine Id error."));
			break;
		case ERROR_LOAD_LIBRARY:
			wsprintf(szText, TEXT("Load Iphlplib error."));
			break;
		case ERROR_GET_ADDR:
			wsprintf(szText, TEXT("Get address error."), i);
			break;
		case INVALID_DEVICE_ID:
			wsprintf(szText, TEXT("Device Id error."));
			break;
		default:
			wsprintf(szText, TEXT("Init camera error %d."), i);
			break;			
		}
		
		::MessageBox(NULL, szText, TEXT("Notice"), MB_OK | MB_TOPMOST);		
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
void CwebcamDlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_WEBCAM_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_WEBCAM_DIALOG));
	}
}
#endif


void CwebcamDlg::OnBnClickedStart()
{
	// TODO: 在此添加控件通知处理程序代码
	m_fIsStop = FALSE;
	ResetEvent(m_hStopEvent);
	m_ctrlStop.EnableWindow();
	m_ctrlStop.SetFocus();
	m_ctrlStart.EnableWindow(FALSE);	
	m_ctrlExit.EnableWindow(FALSE);
	m_ctrlFullScreen.EnableWindow();
	m_ctrlSave2File.EnableWindow();

	m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) CaptureThreadProc, this, 0, NULL);
}
void CwebcamDlg::OnOK() 
{
	// TODO: Add extra validation here
	if (m_ctrlStop.IsWindowEnabled()) return;

	this->OnBnClickedExit();
	CDialog::OnOK();
}

void CwebcamDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	//CDialog::OnCancel();
}
void CwebcamDlg::OnBnClickedStop()
{
	// TODO: 在此添加控件通知处理程序代码
	m_ctrlStart.EnableWindow();
	m_ctrlStart.SetFocus();
	m_ctrlStop.EnableWindow(FALSE);
	m_ctrlExit.EnableWindow();
	m_ctrlFullScreen.EnableWindow(FALSE);
	m_ctrlSave2File.EnableWindow(FALSE);

	if (m_hThread == NULL) return;

	m_fIsStop = TRUE;
	
	DWORD dwRtn = 0;
	dwRtn = WaitForSingleObject(m_hStopEvent, 500);
	if (dwRtn == WAIT_TIMEOUT)
	{
		TerminateThread(m_hThread, 0);
	}

	CloseHandle(m_hThread);
	m_hThread = NULL;
}

void CwebcamDlg::OnBnClickedExit()
{
	// TODO: 在此添加控件通知处理程序代码
	capCloseCamera();

	if (m_hDisplay)
	{
		::DestroyWindow(m_hDisplay);
		m_hDisplay = NULL;
	}

	if (m_hStopEvent)
	{
		CloseHandle(m_hStopEvent);
		m_hStopEvent = NULL;
	}

	EndDialog(0);
}
DWORD WINAPI CwebcamDlg::CaptureThreadProc(LPVOID lpParameter)
{
	CwebcamDlg *pThis = (CwebcamDlg *)lpParameter;
	if (pThis == NULL)
		return 0;

	int index = pThis->m_ctrlCamList.GetCurSel();
	// 
	if (0 != capSetVideoFormat(index, VIDEO_PALETTE_RGB24, VIDEO_SIZE_SIF))
	{
		return 0;
	}

	DWORD dwSize = 320 * 240 * 3;
	DWORD dwJpg = 40960;
	//DWORD dwTime;
	DWORD dwRtnSize[2]; /* 0 - for bmp, 1 - for jpeg */

	LPBYTE lpFrameBuffer = (LPBYTE) malloc (dwSize);
	LPBYTE lpJpgBuffer = (LPBYTE) malloc (dwJpg);
	if (lpFrameBuffer == NULL || lpJpgBuffer == NULL) goto finish;

	if (capStartCamera(index) != 0) 
		goto finish;

	while (!pThis->m_fIsStop)
	{		
		memset(lpFrameBuffer, 0, dwSize);
		memset(lpJpgBuffer, 0, dwJpg);
		dwRtnSize[0] = dwRtnSize[1] = 0;

		//dwTime = GetTickCount();
		if (capGetPicture(index, lpFrameBuffer, dwSize, lpJpgBuffer, dwJpg, dwRtnSize) == 0)
		{
			//dwTime = GetTickCount() - dwTime;
			//RETAILMSG( 1, (TEXT("Get picture take time %dms\r\n"), dwTime));

			/* save ? */
			if (pThis->m_fIsSave)
			{
				pThis->m_fIsSave = FALSE;

				pThis->CreateBitmapFromMem(TEXT("my.bmp"), 320, 240, 24, lpFrameBuffer);
				pThis->MakeJpeg(TEXT("out.jpg"), lpJpgBuffer, dwRtnSize[1]);
			}

			/* display */
			BITMAPINFOHEADER BmpHdr;
			HDC         hdc;
			RECT rc;
			
			memset(&BmpHdr, 0, sizeof (BmpHdr));			
			hdc = ::GetDC(pThis->m_hDisplay);			
			BmpHdr.biSize = sizeof (BmpHdr);
			BmpHdr.biWidth = 320;
			BmpHdr.biHeight = -240;
			BmpHdr.biBitCount = 24;
			BmpHdr.biPlanes = 1;
			BmpHdr.biCompression = BI_RGB;	
			
			::GetClientRect(pThis->m_hDisplay, &rc);
			//dwTime = GetTickCount();
			StretchDIBits(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, 0, 0, 320, 240, lpFrameBuffer, (BITMAPINFO *)&BmpHdr, DIB_RGB_COLORS, SRCCOPY);
			//dwTime = GetTickCount() - dwTime;
			//RETAILMSG( 1, (TEXT("StretchDIBits take time %dms\r\n"), dwTime));
			
			::ReleaseDC(pThis->m_hDisplay, hdc);
		}
		else
		{
			Sleep(15);
		}		
	}
	
finish:
	capStopCamera(index);

	if (lpFrameBuffer)
	{
		free (lpFrameBuffer);
		lpFrameBuffer = NULL;
	}
	
	if (lpJpgBuffer)
	{
		free (lpJpgBuffer);
		lpJpgBuffer = NULL;
	}

	SetEvent(pThis->m_hStopEvent);
	return 0;
}
void CwebcamDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	if (m_ctrlStop.IsWindowEnabled()) return;

	this->OnBnClickedExit();
	
	CDialog::OnClose();
}
void CwebcamDlg::OnBnClickedFullscreen()
{
	// TODO: 在此添加控件通知处理程序代码
	DWORD width = 0, height = 0;
	HDC hdc = ::GetDC(NULL);
	
	width = ::GetDeviceCaps(hdc, HORZRES);
	height = ::GetDeviceCaps(hdc, VERTRES);
	
	::ReleaseDC(NULL, hdc);
	hdc = NULL;

	::SetParent(m_hDisplay, NULL);
	::SetWindowPos(m_hDisplay, HWND_TOPMOST, 0, 0, width, height, NULL);
}

void CwebcamDlg::OnBnClickedSaveToFile()
{
	// TODO: 在此添加控件通知处理程序代码
	m_fIsSave = TRUE;
}
#define ESC_HOT_KEY 0x0000

LRESULT CALLBACK CwebcamDlg::CaptureWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CwebcamDlg *pThis = NULL;
		
	switch(uMsg) 
	{
	case WM_CREATE:
		pThis = (CwebcamDlg *)((LPCREATESTRUCT) lParam)->lpCreateParams;

		if (pThis == NULL) return -1;

		RegisterHotKey(hWnd, ESC_HOT_KEY, NULL, VK_ESCAPE);
		
		return 0;
	case WM_HOTKEY:		
		if ((UINT) HIWORD(lParam) == VK_ESCAPE)
		{						
			::SetParent(hWnd, pThis->GetSafeHwnd());
			::SetWindowPos(hWnd, HWND_TOP, 15, 15, 320, 240, NULL);			
		}
		
		return 0;
	case WM_DESTROY:
		UnregisterHotKey(hWnd, ESC_HOT_KEY);
		break;;
		
	default: break;
	}
	
	return (::DefWindowProc(hWnd, uMsg, wParam, lParam));
}

BOOL CwebcamDlg::CreateBitmapFromMem(LPCTSTR bmpFileName, DWORD width, DWORD height, WORD bitCount, LPBYTE pBmpData)
{
	BOOL rtn = FALSE;
	HANDLE hFile = NULL;
	DWORD dwSize = (width * height * bitCount) >> 3;
	LPBYTE tmpBuffer = NULL;
	
	do 
	{
		BITMAPFILEHEADER bmpFileHeader;
		BITMAPINFOHEADER bmpInfoHeader;
		DWORD dwWritten = 0;
		DWORD i, lineData;

		if (bmpFileName == NULL ||
			pBmpData == NULL)
		{
			break;
		}	
		
		/* Alloc temp buffer */
		tmpBuffer = (unsigned char *)malloc (dwSize);
		if (tmpBuffer == NULL) break;
		
		/* */
		lineData = (width * bitCount) >> 3;
		
		for (i = 0; i < height; i ++)
		{
			memcpy(tmpBuffer + i * lineData, pBmpData + dwSize - (i + 1) * lineData, lineData);
		}
		
		/* Write bitmap header */
		hFile = CreateFile(bmpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) break;	
		
		memset(&bmpFileHeader, 0, sizeof (bmpFileHeader));
		memset(&bmpInfoHeader, 0, sizeof (bmpInfoHeader));
		
		bmpFileHeader.bfType = 0x4D42;
		bmpFileHeader.bfOffBits = sizeof (bmpFileHeader) + sizeof (bmpInfoHeader);
		bmpFileHeader.bfSize = bmpFileHeader.bfOffBits + dwSize;
		
		bmpInfoHeader.biSize = sizeof (bmpInfoHeader);
		bmpInfoHeader.biWidth = width;
		bmpInfoHeader.biHeight = height;
		bmpInfoHeader.biBitCount = bitCount;
		bmpInfoHeader.biPlanes = 1;
		bmpInfoHeader.biCompression = BI_RGB;
				
		WriteFile(hFile, &bmpFileHeader, sizeof (bmpFileHeader), &dwWritten, NULL);
		if (dwWritten != sizeof (bmpFileHeader)) break;
		
		WriteFile(hFile, &bmpInfoHeader, sizeof (bmpInfoHeader), &dwWritten, NULL);
		if (dwWritten != sizeof (bmpInfoHeader)) break;
		
		WriteFile(hFile, tmpBuffer, dwSize, &dwWritten, NULL);
		if (dwWritten != dwSize) break;
		
		rtn = TRUE;		
	} while(0);
	
	if (hFile)
	{
		CloseHandle(hFile);
		hFile = NULL;
	}
	
	if (tmpBuffer)
	{
		free (tmpBuffer);
		tmpBuffer = NULL;
	}
	
	return rtn;
}

BOOL CwebcamDlg::MakeJpeg(LPCTSTR jpgFileName, LPBYTE pJpgData, DWORD size)
{
	if (jpgFileName == NULL || pJpgData == NULL || size <= 0) return FALSE;

	HANDLE hFile = NULL;
	DWORD dwWritten = 0;

	hFile = CreateFile(jpgFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		WriteFile(hFile, pJpgData, size, &dwWritten, NULL);
		CloseHandle(hFile);	
		
		return TRUE;
	}		

	return FALSE;
}
