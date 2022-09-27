/*
** $Id: monthcal.c,v 1.4 2003/11/23 13:09:22 weiym Exp $
**
** monthcal.c: Sample program for MiniGUI Programming Guide
**      Usage of MONTHCALENDAR control.
**
** Copyright (C) 2003 Feynman Software.
**
** License: GPL
*/


#include <stdlib.h>
#include <time.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>


#define IDC_MC                100
#define IDC_OK                200


CTRLDATA CtrlTime[]=
{
        {
                "monthcalendar",
                WS_CHILD | WS_VISIBLE | MCS_NOTIFY | MCS_CHN,
                10, 10, 220, 180,
                IDC_MC,
                "",
                0
        },
        {
                "button",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10, 195, 50, 22,
                IDC_OK,
                "ȷ��",
                0
        }
};

DLGTEMPLATE DlgTime = 
{
        WS_VISIBLE | WS_CAPTION | WS_BORDER,
        WS_EX_NONE,
        0, 0, 240, 240,
        "Լ��ʱ��",
        0, 0,
        2, CtrlTime,
        0
};

/*******************************************************************************/

static int TimeWinProc(HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case MSG_INITDIALOG:
        break;

        case MSG_COMMAND:
        {
            int id = LOWORD(wParam);
	    if (id == IDC_OK) {
		    char info[100];
		    SYSTEMTIME date;
		    SendMessage (GetDlgItem(hDlg, IDC_MC), MCM_GETCURDATE, 0, (LPARAM)&date);
		    sprintf (info, "��������%d��%d��%d�հݼ�������ϯ��", date.year, date.month, date.day);
		    MessageBox (hDlg, info, "Լ��", MB_OK);
		    EndDialog (hDlg, 0);
	    }
        }
        break;

    case MSG_CLOSE:
        {
	    EndDialog (hDlg, 0);
        }
        return 0;
    }

    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

/*******************************************************************************/

int MiniGUIMain (int argc, const char* argv[])
{
#ifdef _LITE_VERSION
    SetDesktopRect(0, 0, 1024, 768);
#endif
    
    if (!InitMiniGUIExt()) {
        return 2;
    }

    DlgTime.controls = CtrlTime;
    
    DialogBoxIndirectParam (&DlgTime, HWND_DESKTOP, TimeWinProc, 0L);

    MiniGUIExtCleanUp ();

    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

