/*
** $Id: listview.c,v 1.4 2003/11/23 13:14:43 weiym Exp $
**
** listview.c: Sample program for MiniGUI Programming Guide
**      Usage of LISTVIEW control.
**
** Copyright (C) 2003 Feynman Software.
**
** License: GPL
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/stat.h>
//#include <sys/types.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mgext.h>


#define IDC_LISTVIEW	10
#define IDC_CTRL1       20
#define IDC_CTRL2       30


#define SCORE_NUM	4
#define SUB_NUM		3

static char * caption [SUB_NUM+1] =
{
    "����", "����", "��ѧ", "Ӣ��"
};

typedef struct _SCORE
{
    char *name;
    int scr[SUB_NUM];
} SCORE;


static SCORE scores[SCORE_NUM] =
{
	{"С��", {81, 96, 75}},
	{"Сǿ", {98, 62, 84}},
	{"С��", {79, 88, 89}},
	{"С��", {66, 73, 99}}
};

static int
ScoreProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    HWND hListView;

    hListView = GetDlgItem (hDlg, IDC_LISTVIEW);

    switch (message)
    {

    case MSG_INITDIALOG:
    {
        int i, j;
        LVSUBITEM subdata;
        LVITEM item;
        LVCOLUMN lvcol;

	for (i = 1; i <= SUB_NUM+1; i++) {
            lvcol.nCols = i;
	    lvcol.pszHeadText = caption[i-1];
	    lvcol.width = 50;
	    lvcol.pfnCompare = NULL;
	    lvcol.colFlags = 0;
            SendMessage (hListView, LVM_ADDCOLUMN, 0, (LPARAM) &lvcol);
	}

	for (i = 1; i <= SCORE_NUM; i++) {
            item.nItem = i;
            SendMessage (hListView, LVM_ADDITEM, 0, (LPARAM) & item);

	    for (j = 1; j <= 4; j++) {
		char buff[20];
                subdata.nItem = i;
                subdata.subItem = j;
		if (j == 1) {
		    subdata.pszText = scores[i-1].name;
		    subdata.nTextColor = 0;
		}
		else {
		    sprintf (buff, "%d", scores[i-1].scr[j-2]);
		    subdata.pszText = buff;
		    if (scores[i-1].scr[j-2] > 90)
			subdata.nTextColor = PIXEL_red;
		    else
			subdata.nTextColor = 0;
		}
		subdata.flags = 0;
		subdata.image = 0;
                SendMessage (hListView, LVM_SETSUBITEM, 0, (LPARAM) & subdata);
	    }
	}

      }
      break;

    case MSG_COMMAND:
    {
	int id = LOWORD (wParam);

	if (id == IDC_CTRL2) {
	    int i, j;
	    float average = 0;
	    char buff[20];
	    for (i = 0; i < SCORE_NUM; i++) {
		for (j = 0; j < SUB_NUM; j++) {
		    average += scores[i].scr[j];
		}
	    }
	    average = average / (SCORE_NUM * SUB_NUM);

	    sprintf (buff, "%4.1f", average);
	    SendDlgItemMessage (hDlg, IDC_CTRL1, MSG_SETTEXT, 0, (LPARAM)buff);
	}
    }
    break;

    case MSG_CLOSE:
    {
	    EndDialog (hDlg, 0);
    }
    break;

    }

    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

static DLGTEMPLATE DlgScore =
{
    WS_BORDER | WS_CAPTION,
    WS_EX_NONE,
    0, 0, 240, 240,
    "��ƽ����",
    0, 0,
    3, NULL,
    0
};

static CTRLDATA CtrlScore[] =
{
    {
        "listview",
        WS_BORDER | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL,
         10, 10, 220, 120,
        IDC_LISTVIEW,
        "score table",
        0
    },
    {
        "edit",
        WS_CHILD | WS_VISIBLE | WS_BORDER,
         10, 160, 50, 20,
        IDC_CTRL1,
        "",
        0
    },
    {
        "button",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
         80, 160, 80, 20,
        IDC_CTRL2,
        "����ƽ����",
        0
    },
};

int MiniGUIMain (int argc, const char* argv[])
{
#ifdef _LITE_VERSION
    SetDesktopRect(0, 0, 1024, 768);
#endif
    
    if (!InitMiniGUIExt()) {
        return 2;
    }

    DlgScore.controls = CtrlScore;
    
    DialogBoxIndirectParam (&DlgScore, HWND_DESKTOP, ScoreProc, 0L);

    MiniGUIExtCleanUp ();

    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

