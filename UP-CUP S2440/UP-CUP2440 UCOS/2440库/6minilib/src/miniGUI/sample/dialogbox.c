/* 
** $Id: dialogbox.c,v 1.3 2003/06/13 06:50:39 weiym Exp $
**
** Listing 4.1
**
** dialogbox.c: Sample program for MiniGUI Programming Guide
** 	Usage of DialogBoxIndirectParam
**
** Copyright (C) 2003 Feynman Software.
**
** License: GPL
*/

#include <stdio.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

static DLGTEMPLATE DlgInitProgress =
{
    WS_BORDER | WS_CAPTION, 
    WS_EX_NONE,
    10, 50, 300, 130, 
    "VAM-CNC ���ڳ�ʼ��",
    0, 0,
    3, NULL,
    0
};

#define IDC_PROMPTINFO	100
#define IDC_PROGRESS  	110

static CTRLDATA CtrlInitProgress [] =
{ 
    {
        "static",
        WS_VISIBLE | SS_SIMPLE,
        10, 10, 180, 16, 
        IDC_PROMPTINFO, 
        "����...",
        0
    },
    {
        "progressbar",
        WS_VISIBLE,
        10, 40, 180, 20,
        IDC_PROGRESS,
        NULL,
        0
    },
    {
        "button",
        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
        220, 70, 60, 25,
        IDOK, 
        "ȷ��",
        0
    }
};

static int InitDialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case MSG_INITDIALOG:
        return 1;
        
    case MSG_COMMAND:
        switch (wParam) {
        case IDOK:
        case IDCANCEL:
            EndDialog (hDlg, wParam);
            break;
        }
        break;
        
    }
    
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

static void InitDialogBox (HWND hWnd)
{
    DlgInitProgress.controls = CtrlInitProgress;
    
    DialogBoxIndirectParam (&DlgInitProgress, hWnd, InitDialogBoxProc, 0L);
}

int MiniGUIMain (int argc, const char* argv[])
{
#ifdef _LITE_VERSION
    SetDesktopRect(0, 0, 1024, 768);
#endif
    
    InitDialogBox (HWND_DESKTOP);

    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

