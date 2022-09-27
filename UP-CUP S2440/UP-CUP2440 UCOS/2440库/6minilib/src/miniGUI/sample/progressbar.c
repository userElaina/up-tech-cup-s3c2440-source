/* 
** $Id: progressbar.c,v 1.5 2003/08/15 09:20:35 weiym Exp $
**
** Listing 12.1
**
** progressbar.c: Sample program for MiniGUI Programming Guide
**      Usage of PORGRESSBAR control.
**
** Copyright (C) 2003 Feynman Software.
**
** License: GPL
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
//#include <stat.h>
#include <time.h>
//#include <types.h>
//#include <unistd.h>
#include <errno.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

static HWND createProgressWin (HWND hParentWnd, char * title, char * label,
        int id, int range)
{
    HWND hwnd;
    MAINWINCREATE CreateInfo;
    int ww, wh;
    HWND hStatic, hProgBar;

    ww = ClientWidthToWindowWidth (WS_CAPTION | WS_BORDER, 400);
    wh = ClientHeightToWindowHeight (WS_CAPTION | WS_BORDER, 
            (range > 0) ? 70 : 35, FALSE);
    
    CreateInfo.dwStyle = WS_ABSSCRPOS | WS_CAPTION | WS_BORDER | WS_VISIBLE;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = title;
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(IDC_WAIT);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = DefaultMainWinProc;
    CreateInfo.lx = (GetGDCapability (HDC_SCREEN, GDCAP_MAXX) - ww) >> 1;
    CreateInfo.ty = (GetGDCapability (HDC_SCREEN, GDCAP_MAXY) - wh) >> 1;
    CreateInfo.rx = CreateInfo.lx + ww;
    CreateInfo.by = CreateInfo.ty + wh;
    CreateInfo.iBkColor = COLOR_lightgray;
    CreateInfo.dwAddData = 0L;
    CreateInfo.hHosting = hParentWnd;

    hwnd = CreateMainWindow (&CreateInfo);
    if (hwnd == HWND_INVALID)
        return hwnd;

    hStatic = CreateWindowEx ("static", 
                  label, 
                  WS_VISIBLE | SS_SIMPLE, 
                  WS_EX_USEPARENTCURSOR,
                  IDC_STATIC, 
                  10, 10, 380, 16, hwnd, 0);
    
    if (range > 0) {
        hProgBar = CreateWindowEx ("progressbar", 
                  NULL, 
                  WS_VISIBLE,
                  WS_EX_USEPARENTCURSOR,
                  id,
                  10, 30, 380, 30, hwnd, 0);
        SendDlgItemMessage (hwnd, id, PBM_SETRANGE, 0, range);
    }
    else
        hProgBar = HWND_INVALID;

    UpdateWindow (hwnd, TRUE);

    return hwnd;
}

static void destroyProgressWin (HWND hwnd)
{
    DestroyAllControls (hwnd);
    DestroyMainWindow (hwnd);
    ThrowAwayMessages (hwnd);
    MainWindowThreadCleanup (hwnd);
}

int MiniGUIMain (int argc, const char* argv[])
{
    int i, sum;
    HCURSOR hOldCursor;
    HWND hwnd;

#ifdef _LITE_VERSION
    SetDesktopRect(0, 0, 1024, 768);
#endif

    hOldCursor = SetDefaultCursor (GetSystemCursor (IDC_WAIT));

    hwnd = createProgressWin (HWND_DESKTOP, "������",
	    "���ڼ��㣬���Ժ�...", 100, 2000);

    while (HavePendingMessage (hwnd)) {
         MSG msg;
         GetMessage (&msg, hwnd);
         DispatchMessage (&msg);
    }

    for (i = 0; i < 2000; i++) {
        unsigned long j;
        
        if (i % 100 == 0) {
            SendDlgItemMessage (hwnd, 100, PBM_SETPOS, i, 0L);
            while (HavePendingMessage (hwnd)) {
                MSG msg;
                GetMessage (&msg, hwnd);
                DispatchMessage (&msg);
            }
        }

        sum = i*5000;
        for (j = 0; j < 500000; j++)
            sum *= j;
        sum += sum;
    }

    destroyProgressWin (hwnd);
    SetDefaultCursor (hOldCursor);

    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

