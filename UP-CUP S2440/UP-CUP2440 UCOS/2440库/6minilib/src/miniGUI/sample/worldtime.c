/*
** $Id: worldtime.c,v 1.3 2003/11/20 05:04:46 weiym Exp $
**
** worldtime.c: worldtime for FHAS.
**
** Copyright (C) 2002, 2003 Feynman Software, all rights reserved.
**
** Use of this source package is subject to specific license terms
** from Beijing Feynman Software Technology Co., Ltd.
**
** URL: http://www.minigui.com
**
** Current maintainer: Wei Yongming (ymwei@minigui.org).
**
*/

#include "worldtime.h"

static const char* timezones [] =
{
    "GMT-12 ����ֵ�",
    "GMT-11 ��;��������Ħ��",
    "GMT-10 ������",
    "GMT-9 ����˹��",
    "GMT-8 ��ɼ���ɽ�ɽ",
    "GMT-8 �¸绪",
    "GMT-7 ����ɣ��",
    "GMT-6 ʥ·��˹��֥�Ӹ�",
    "GMT-6 ī����ǣ�������",
    "GMT-6 ���Ხ",
    "GMT-6 ������",
    "GMT-5 ��ʢ�٣�ŦԼ����ʿ��",
    "GMT-5 ���׶�",
    "GMT-5 ���������������",
    "GMT-4 ������˹������˹",
    "GMT-4 ʥ���Ǹ�",
    "GMT-3:30 Ŧ����",
    "GMT-3 ʥ���ޣ���Լ����¬",
    "GMT-3 ����ŵ˹����˹",
    "GMT-3 ������",
    "GMT-2 �������в�",
    "GMT-1 ���ٶ�Ⱥ��",
    "GMT+0 �׶أ�������",
    "GMT+0 ����������",
    "GMT+1 ��ķ˹�ص�����������",
	"GMT+1 ���������£�����",
	"GMT+1 ��³������������˹",
	"GMT+1 �籾������������",
    "GMT+1 ��˹������������",
    "GMT+1 ����˹�¸��Ħ",
    "GMT+1 �з�����",
    "GMT+2 �ŵ䣬������˹��",
    "GMT+2 ���ޣ��ն�����",
    "GMT+2 ��˹̹������Ү·����",
    "GMT+3 �͸�������",
    "GMT+3 Ī˹�ƣ�Ľ���",
    "GMT+3 ���ŵã����ޱ�",
    "GMT+3:30 �º���",
    "GMT+4 �������ȣ��ϰ�",
    "GMT+4:30 ������",
    "GMT+5 ��˹����",
	"GMT+5:30 �µ������",
    "GMT+5:30 ˹������",
    "GMT+5:45 �ӵ�����",
    "GMT+6 ��˹̹�ɣ��￨",
    "GMT+6:30 ����",
    "GMT+7 ���ȣ����ڣ��żӴ�",
    "GMT+8 ����������ر�������",
    "GMT+8 ̨��������",
    "GMT+8 ��¡�£��¼���",
    "GMT+8 ������",
    "GMT+8 ��������",
    "GMT+9 ���ǣ�ƽ��",
    "GMT+9 ����������",
    "GMT+9:30 �����",
    "GMT+10 Ϥ�ᣬī����",
    "GMT+10 ������������˹��",
    "GMT+10 ��������˹�п�",
    "GMT+10 �ص�",
    "GMT+11 ��ӵ�",
    "GMT+12 ������",
    "GMT+13 ����",
};

static void on_create (HWND hWnd)
{
    int i;
    HWND wnd_tzs;

    CreateWindow (CTRL_STATIC,
        "ϵͳʱ�䣺",
        WS_VISIBLE | SS_SIMPLE,
        IDC_STATIC,
        ORG_X, ORG_Y,
        WIDTH_LABEL, HEIGHT_LABEL,
        hWnd, 0);

    CreateWindow (CTRL_STATIC,
        "����ʱ�䣺",
        WS_VISIBLE | SS_SIMPLE,
        IDC_STATIC,
        ORG_X, ORG_Y + HEIGHT_TIME,
        WIDTH_LABEL, HEIGHT_LABEL,
        hWnd,0);

    CreateWindow (CTRL_STATIC,
        "��������ʱ�䣺",
        WS_VISIBLE | SS_SIMPLE,
        IDC_STATIC,
        ORG_X, 
        ORG_Y + HEIGHT_TIME * 2,
        WIDTH_LABEL, HEIGHT_LABEL,
        hWnd, 0);

    CreateWindow (CTRL_STATIC,
        "��ѡ������ʱ����",
        WS_VISIBLE | SS_SIMPLE,
        IDC_STATIC,
        ORG_X, 
        ORG_Y + HEIGHT_TIME * 3,
        WIDTH_LABEL, HEIGHT_LABEL,
        hWnd,0);

    wnd_tzs = CreateWindow (CTRL_COMBOBOX,
        "δѡ��ʱ��",
        WS_VISIBLE | CBS_DROPDOWNLIST | CBS_NOTIFY | CBS_READONLY,
        IDC_TIMEZONE,
        ORG_X, 
        ORG_Y + HEIGHT_TIME * 3 + HEIGHT_LABEL,
        MAINWINDOW_RX - ORG_X*2, HEIGHT_LABEL,
        hWnd, HEIGHT_LISTBOX);

    for (i = 0; i < TABLESIZE (timezones); i++) {
        SendMessage (wnd_tzs, CB_ADDSTRING, 0, (LPARAM)timezones [i]);
    }
}

/* Main WindProc function */
static int WorldTimeWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    static char timezone [20];
    static const char *pdate;
    switch (message) {
        case MSG_CREATE:
            on_create (hWnd);
            SetTimer (hWnd, TIMER_CLOCK, 10);
        break;

        break;

        case MSG_COMMAND:
        {
            int id   = LOWORD (wParam);
            int code = HIWORD (wParam);
            if (id == IDC_TIMEZONE && code == LBN_SELCHANGE) {
                int selected = SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
                if (selected >= 0) {

                    int space;
                    HDC hdc;

                    strcpy (timezone, "TZ=");
                    space = strchr (timezones [selected], ' ') - timezones [selected];
                    strncat (timezone, timezones [selected], space);
                    if (timezone [6] == '+')
                        timezone [6] = '-';
                    else
                        timezone [6] = '+';

                    hdc = GetClientDC (hWnd);
                    pdate = getdate (timezone);
                    TextOut (hdc, ORG_X, ORG_Y + HEIGHT_TIME * 4 + 10, pdate);
                    ReleaseDC (hdc);
                }
            }

            break;
        }

        case MSG_TIMER:
                if (wParam == TIMER_CLOCK) {
                    HDC hdc;
                    char *London;
                    char *BeiJing;
    
                    hdc = GetClientDC(hWnd);
                    SetBkColor (hdc, COLOR_lightwhite);

                    TextOut (hdc, ORG_X,
                                  ORG_Y + HEIGHT_LABEL,
                                  getdatetime ("TZ=GMT-8"));

                    BeiJing = "TZ=GMT-8";
                    TextOut (hdc, ORG_X , 
                                  ORG_Y + HEIGHT_TIME + HEIGHT_LABEL,
                                  gettime(BeiJing));

                    London = "TZ=GMT+0";
                    TextOut (hdc, ORG_X,
                                  ORG_Y + HEIGHT_TIME * 2 + HEIGHT_LABEL,
                                  gettime(London));

                    if (pdate != NULL) {
                        pdate = getdate (timezone);
                        TextOut (hdc, ORG_X, ORG_Y + HEIGHT_TIME * 4 + 10, pdate);
                    }

                    ReleaseDC (hdc);
                }
            break;
    
       case MSG_MAXIMIZE:
            break;

       case MSG_KEYDOWN:
            break;

       case MSG_CLOSE:
            pdate = NULL;
            KillTimer (hWnd, TIMER_CLOCK);
            DestroyAllControls (hWnd);
            DestroyMainWindow (hWnd);
            PostQuitMessage (hWnd);
            return 0;
    }

    return DefaultMainWinProc (hWnd, message, wParam, lParam);
}

static void InitCreateInfo(PMAINWINCREATE pCreateInfo)
{
    pCreateInfo->dwStyle = WS_CAPTION | WS_MAXIMIZEBOX | WS_VISIBLE;
    pCreateInfo->dwExStyle = WS_EX_NONE;
    pCreateInfo->spCaption = "����ʱ" ;
    pCreateInfo->hMenu = 0;
    pCreateInfo->hCursor = GetSystemCursor(0);
    pCreateInfo->hIcon = 0;
    pCreateInfo->MainWindowProc = WorldTimeWinProc;
    pCreateInfo->lx = MAINWINDOW_LX; 
    pCreateInfo->ty = MAINWINDOW_TY;
    pCreateInfo->rx = MAINWINDOW_RX;
    pCreateInfo->by = MAINWINDOW_BY;
    pCreateInfo->iBkColor = COLOR_lightwhite; 
    pCreateInfo->dwAddData = 0;
    pCreateInfo->hHosting = HWND_DESKTOP;
}

int MiniGUIMain (int args, const char* arg[])
{
    MSG Msg;
    MAINWINCREATE CreateInfo;
    HWND hMainWnd;

#ifdef _LITE_VERSION
    SetDesktopRect(0,0,1024,768);
#endif

    InitCreateInfo(&CreateInfo);
    hMainWnd = CreateMainWindow(&CreateInfo);
    
    if (hMainWnd == HWND_INVALID)
        return 3;

    ShowWindow(hMainWnd, SW_SHOWNORMAL);

    while(GetMessage(&Msg, hMainWnd)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }    
    
    MainWindowThreadCleanup(hMainWnd);

    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

