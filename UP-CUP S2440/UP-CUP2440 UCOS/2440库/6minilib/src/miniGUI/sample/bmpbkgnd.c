/* 
** $Id: bmpbkgnd.c,v 1.6 2003/12/29 10:39:25 weiym Exp $
**
** Listing 5.2
**
** bmpbkgnd.c: Sample program for MiniGUI Programming Guide
** 	    Create a dialog box filling background with bitmap.
**
** Copyright (C) 2003 Feynman Software.
**
** License: GPL
*/

#include <stdio.h>
#include <stdlib.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

static DLGTEMPLATE DlgBoxInputLen =
{
    WS_BORDER | WS_CAPTION, 
    WS_EX_NONE,
    10, 10, 300, 220, 
    "�����볤��",
    0, 0,
    4, NULL,
    0
};

#define IDC_SIZE_MM     100
#define IDC_SIZE_INCH   110

static CTRLDATA CtrlInputLen [] =
{ 
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE,
        40, 10, 200, 18, 
        IDC_STATIC, 
        "�����볤�ȣ���λ�����ף�",
        0
    },
    {
        CTRL_EDIT,
        WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        40, 40, 200, 24,
        IDC_SIZE_MM,
        NULL,
        0
    },
    {
        CTRL_STATIC,
        WS_VISIBLE | SS_SIMPLE,
        40, 70, 200, 18, 
        IDC_SIZE_INCH, 
        "�൱�� 0.00 Ӣ��",
        0
    },
    {
        CTRL_BUTTON,
        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
        40, 100, 60, 25,
        IDOK, 
        "ȷ��",
        0
    }
};

static void my_notif_proc (HWND hwnd, int id, int nc, DWORD add_data)
{
    if (id == IDC_SIZE_MM && nc == EN_CHANGE) {
        char buff [60];
        double len;

        GetWindowText (hwnd, buff, 32);
        len = (double) atoi (buff);
        len = len / 25.4;

        sprintf (buff, "�൱�� %.5f Ӣ��", len);
        SetDlgItemText (GetParent (hwnd), IDC_SIZE_INCH, buff);
    }
}       

static BITMAP bmp_bkgnd;
static int InputLenDialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case MSG_INITDIALOG:
        SetWindowAdditionalData (hDlg, lParam);
        SetNotificationCallback (GetDlgItem (hDlg, IDC_SIZE_MM), my_notif_proc);
        return 1;
        
    case MSG_ERASEBKGND:
	{
	    HDC hdc = (HDC)wParam;
	    const RECT* clip = (const RECT*) lParam;
	    BOOL fGetDC = FALSE;
	    RECT rcTemp;
		    
	    if (hdc == 0) {
		hdc = GetClientDC (hDlg);
		fGetDC = TRUE;
	    }       
		    
	    if (clip) {
		rcTemp = *clip;
		ScreenToClient (hDlg, &rcTemp.left, &rcTemp.top);
		ScreenToClient (hDlg, &rcTemp.right, &rcTemp.bottom);
		IncludeClipRect (hdc, &rcTemp);
	    }

	    FillBoxWithBitmap (hdc, rcTemp.left, rcTemp.top, 0, 0, &bmp_bkgnd);

	    if (fGetDC)
		ReleaseDC (hdc);
	    return 0;
	}

    case MSG_COMMAND:
        switch (wParam) {
        case IDOK:
        {
            char buff [40];
            double* length = (double*) GetWindowAdditionalData (hDlg);
            GetWindowText (GetDlgItem (hDlg, IDC_SIZE_MM), buff, 32);
            *length = (double)atoi (buff);
        }
        case IDCANCEL:
            EndDialog (hDlg, wParam);
            break;
        }
        break;
    }
    
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

static void InputLenDialogBox (HWND hWnd, double* length)
{
    DlgBoxInputLen.controls = CtrlInputLen;
    
    DialogBoxIndirectParam (&DlgBoxInputLen, hWnd, InputLenDialogBoxProc, (LPARAM)length);
}

int MiniGUIMain (int argc, const char* argv[])
{
    double length;

#ifdef _LITE_VERSION
    SetDesktopRect(0, 0, 1024, 768);
#endif
    
//    if (LoadBitmap (HDC_SCREEN, &bmp_bkgnd, "bkgnd.jpg"))
//         return 1;

    InputLenDialogBox (HWND_DESKTOP, &length);
    printf ("The length is %.5f mm.\n", length);

    UnloadBitmap (&bmp_bkgnd);
    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

