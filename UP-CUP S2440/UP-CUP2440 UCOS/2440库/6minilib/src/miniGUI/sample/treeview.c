/*
** $Id: treeview.c,v 1.2 2003/11/23 13:35:26 weiym Exp $
**
** treeview.c: Sample program for MiniGUI Programming Guide
**      Usage of TREEVIEW control.
**
** Copyright (C) 2003 Feynman Software.
**
** License: GPL
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mgext.h>


#define IDC_TREEVIEW  100


#define CHAPTER_NUM	5

static const char *chapter[] = 
{
    "��ʮ���� ���Ϳؼ�",
    "��ʮ���� �б��Ϳؼ�",
    "��ʮ���� �����ؼ�",
    "��ʮ���� ��ť�ؼ�",
    "�ڶ�ʮ�� �Ṥ�����ؼ�",
};

static const char *section[] =
{
    "�ؼ����",
    "�ؼ���Ϣ",
    "�ؼ�֪ͨ��"
};

static int BookProc(HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{

    switch (message) {
        case MSG_INITDIALOG:
        {
            TVITEMINFO tvItemInfo;
	    int item;
	    int i, j;

	    for (i = 0; i < CHAPTER_NUM; i++) {
		tvItemInfo.text = (char*)chapter[i];
	        item = SendMessage (GetDlgItem(hDlg, IDC_TREEVIEW), TVM_ADDITEM, 
			    0, (LPARAM)&tvItemInfo);
		for (j = 0; j < 3; j++) {
		    tvItemInfo.text = (char*)section[j];
	            SendMessage (GetDlgItem(hDlg, IDC_TREEVIEW), TVM_ADDITEM, 
			    item, (LPARAM)&tvItemInfo);
		}
	    }
        }
        break;

        case MSG_COMMAND:
        {
            //int id   = LOWORD(wParam);
            //int code = HIWORD(wParam);
        }
        break;
        
        case MSG_CLOSE:
	    EndDialog (hDlg, 0);
            return 0;
    }

    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

static TVITEMINFO bookInfo =
{
    "MiniGUI���ָ��"
};

static DLGTEMPLATE DlgBook =
{
    WS_BORDER | WS_CAPTION,
    WS_EX_NONE,
    0, 0, 240, 320,
    "Book Content",
    0, 0,
    1, NULL,
    0
};

static CTRLDATA CtrlBook[] =
{
    {
        "treeview",
        WS_BORDER | WS_CHILD | WS_VISIBLE | 
		WS_VSCROLL | WS_HSCROLL,
         10, 10, 220, 280,
        IDC_TREEVIEW,
        "treeview control",
        (LPARAM)&bookInfo
    }
};

int MiniGUIMain (int argc, const char* argv[])
{
#ifdef _LITE_VERSION
    SetDesktopRect(0, 0, 1024, 768);
#endif
    
    if (!InitMiniGUIExt()) {
        return 2;
    }

    DlgBook.controls = CtrlBook;
    
    DialogBoxIndirectParam (&DlgBook, HWND_DESKTOP, BookProc, 0L);

    MiniGUIExtCleanUp ();

    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

