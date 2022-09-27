/*
** $Id: menubutton.c,v 1.3 2003/06/13 06:50:39 weiym Exp $
** 
** Listing 11.1
**
** menubutton.c: Sample program for MiniGUI Programming Guide
**      The usage of MENUBUTTON control.
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

#define IDC_HOUR   100
#define IDC_MINUTE 110
#define IDC_SECOND 120
#define IDL_DAXIA  200

#define IDC_PROMPT 300

static DLGTEMPLATE DlgMyDate =
{
    WS_BORDER | WS_CAPTION,
    WS_EX_NONE,
    100, 100, 304, 135,
    "Լ�����",
    0, 0,
    9, NULL,
    0
};

static CTRLDATA CtrlMyDate[] =
{ 
    {
        "static",
        WS_CHILD | SS_RIGHT | WS_VISIBLE,
         10, 20, 50, 20,
        IDC_STATIC,
        "�Ҵ�����",
        0
    },
    {
        CTRL_COMBOBOX,
        WS_CHILD | WS_VISIBLE | 
            CBS_READONLY | CBS_AUTOSPIN | CBS_AUTOLOOP | CBS_EDITBASELINE,
        60, 18, 40, 20,
        IDC_HOUR, 
        "",
        0
    },
    {
        "static",
        WS_CHILD | SS_CENTER | WS_VISIBLE,
        100, 20, 20, 20,
        IDC_STATIC,
        "ʱ",
        0
    },
    {
        CTRL_COMBOBOX,
        WS_CHILD | WS_VISIBLE | 
            CBS_READONLY | CBS_AUTOSPIN | CBS_AUTOLOOP | CBS_EDITBASELINE,
        120, 18, 40, 20,
        IDC_MINUTE,
        "",
        0
    },
    {
        "static",
        WS_CHILD | SS_CENTER | WS_VISIBLE,
        160, 20, 30, 20,
        IDC_STATIC,
        "ȥ��",
        0
    },
    {
        CTRL_MENUBUTTON,
        WS_CHILD | WS_VISIBLE,
        190, 20, 100, 20,
        IDL_DAXIA,
        "",
        0
    },
    {
        "static",
        WS_CHILD | SS_RIGHT | WS_VISIBLE,
        10, 50, 280, 20,
        IDC_PROMPT,
        "This is",
        0
    },
    {
        CTRL_BUTTON,
        WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP | WS_GROUP,
        10, 70, 130, 25,
        IDOK, 
        "ȷ��",
        0
    },
    {
        "button",
        WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        150, 70, 130, 25,
        IDCANCEL,
        "ȡ��",
        0
    },
};

static const char* daxia [] =
{
    "��ҩʦ",
    "ŷ����",
    "�λ�ү",
    "���߹�",
    "�ܲ�ͨ",
    "����",
    "����",
};

static const char* daxia_char [] =
{
    "��Ƨ",
    "��",
    "�ٴȱ�",
    "һ������",
    "��Ƥ����������",
    "ɵ����",
    "����",
};

static void daxia_notif_proc (HWND hwnd, int id, int nc, DWORD add_data)
{
    if (nc == CBN_SELCHANGE) {
        int cur_sel = SendMessage (hwnd, MBM_GETCURITEM, 0, 0);
        if (cur_sel >= 0) {
                SetWindowText (GetDlgItem (GetParent(hwnd), IDC_PROMPT), daxia_char [cur_sel]);
        }
    }
}

static void prompt (HWND hDlg)
{
    char date [1024] = "���Լ�����ݣ�\n";

    int hour = SendDlgItemMessage(hDlg, IDC_HOUR, CB_GETSPINVALUE, 0, 0);
    int min = SendDlgItemMessage(hDlg, IDC_MINUTE, CB_GETSPINVALUE, 0, 0);
    int sel = SendDlgItemMessage(hDlg, IDL_DAXIA, MBM_GETCURITEM, 0, 0);

    sprintf (date, "�������ڽ��� %02d:%02d ȥ���Ǹ�%s��%s", hour, min, 
                    daxia_char [sel], daxia [sel]);

    MessageBox (hDlg, date, "Լ������", MB_OK | MB_ICONINFORMATION);
}

static int MyDateBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    int i;
    switch (message) {
    case MSG_INITDIALOG:
        SendDlgItemMessage(hDlg, IDC_HOUR, CB_SETSPINFORMAT, 0, (LPARAM)"%02d");
        SendDlgItemMessage(hDlg, IDC_HOUR, CB_SETSPINRANGE, 0, 23);
        SendDlgItemMessage(hDlg, IDC_HOUR, CB_SETSPINVALUE, 20, 0);
        SendDlgItemMessage(hDlg, IDC_HOUR, CB_SETSPINPACE, 1, 1);

        SendDlgItemMessage(hDlg, IDC_MINUTE, CB_SETSPINFORMAT, 0, (LPARAM)"%02d");
        SendDlgItemMessage(hDlg, IDC_MINUTE, CB_SETSPINRANGE, 0, 59);
        SendDlgItemMessage(hDlg, IDC_MINUTE, CB_SETSPINVALUE, 0, 0);
        SendDlgItemMessage(hDlg, IDC_MINUTE, CB_SETSPINPACE, 1, 2);

        for (i = 0; i < 7; i++) {
            MENUBUTTONITEM mbi;
            mbi.text = daxia[i];
            mbi.bmp = NULL;
            mbi.data = 0;
            SendDlgItemMessage(hDlg, IDL_DAXIA, MBM_ADDITEM, -1, (LPARAM)&mbi);
        }

        SetNotificationCallback (GetDlgItem (hDlg, IDL_DAXIA), daxia_notif_proc);
        SendDlgItemMessage(hDlg, IDL_DAXIA, MBM_SETCURITEM, 0, 0);
        SetWindowText (GetDlgItem (hDlg, IDC_PROMPT), daxia_char [0]);
        return 1;
        
    case MSG_COMMAND:
        switch (wParam) {
        case IDOK:
            prompt (hDlg);
        case IDCANCEL:
            EndDialog (hDlg, wParam);
            break;
        }
        break;
        
    }
    
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

int MiniGUIMain (int argc, const char* argv[])
{
#ifdef _LITE_VERSION
    SetDesktopRect(0, 0, 1024, 768);
#endif
    
    DlgMyDate.controls = CtrlMyDate;
    
    DialogBoxIndirectParam (&DlgMyDate, HWND_DESKTOP, MyDateBoxProc, 0L);

    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

