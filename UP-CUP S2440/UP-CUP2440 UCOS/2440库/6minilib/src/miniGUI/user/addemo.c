/*
** $Id: helloworld.c,v 1.1.1.1 2003/06/03 06:57:15 weiym Exp $
**
** Listing 2.1
**
** helloworld.c: Sample program for MiniGUI Programming Guide
** The first MiniGUI Application
**
** Copyright (C) 2003 Feynman Software.
**
** License: GPL
*/
#include "../inc/sysconfig.h"
#include "../src/minigui/sample.h"

#if (USE_MINIGUI==1 && SAMPLE==SAMPLE_NULL)

#include <stdio.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#include "../inc/drivers.h"

#define ID_ADTIMER	100

static BITMAP bmp;

static int HelloWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    int i;
    HDC hdc;
    pthread_t th;
    extern void* buttonMain (void* data);
    extern void* editMain (void* data);
    extern void* comboboxMain (void* data);
    extern void* gdidemo (void* data);
    extern void* painterMain (void* data);
    
    switch (message) {
        case MSG_NCCREATE:
            printf ("Creating helloworld window in MSG_NCCREATE.\n");
            break;

        case MSG_CREATE:
            printf ("Creating helloworld window.\n");
//            pthread_create (&th, NULL,buttonMain, NULL);
  			//pthread_create (&th, NULL,comboboxMain, NULL);
//  			pthread_create (&th, NULL,editMain, NULL); 	
  			//pthread_create (&th, NULL,gdidemo, NULL); 			
  			//pthread_create (&th, NULL, painterMain, NULL);
  		SetTimer(hWnd, ID_ADTIMER, 100);
		init_ADdevice();
	            if (LoadBitmap (HDC_SCREEN, &bmp, "/sys/up-tech.gif")){
					printf("Can't find picture\n");
	            }
            break;

        case MSG_KEYDOWN:
            printf ("KEYDOWN: %d.\n", wParam);
            break;
            
        case MSG_LBUTTONDOWN:
        {
        	int x, y;
        	x = LOWORD(lParam);
        	y = HIWORD(lParam);
        	printf ("mouse down: x = %d, y = %d\n", x, y);
        }
        break;
        
        case MSG_LBUTTONUP:
        {
        	int x, y;
        	x = LOWORD(lParam);
        	y = HIWORD(lParam);
        	printf ("mouse up: x = %d, y = %d\n", x, y);
        }
        break;

        case MSG_PAINT:
            hdc = BeginPaint (hWnd);
            FillBoxWithBitmap (hdc, 10, 10, bmp.bmWidth, bmp.bmHeight, &bmp);
/*
            for (i = 0; i<5; i++) {
                DrawIcon (hdc, i*16, 0, 0, 0, GetSmallSystemIcon (i));
                DrawIcon (hdc, i*32, 32, 0, 0, GetLargeSystemIcon (i));
            }
*/
            EndPaint (hWnd, hdc);
            return 0;

        case MSG_CLOSE:
            UnloadBitmap (&bmp);
  		KillTimer(hWnd, ID_ADTIMER);
            DestroyMainWindow (hWnd);
            PostQuitMessage (hWnd);
            return 0;

	case MSG_TIMER:
		{
			static char str[100];
			int i;
			hdc=GetDC(hWnd);
			for(i=0;i<4;i++){
				sprintf(str, "Í¨µÀ%d: %4.4d", i, AD_GetResult(i));
				TextOut (hdc, 100, 100+i*20, str);
			}
			ReleaseDC(hdc);
		}
		break;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

#include "includes.h"

int MiniGUIMain (int argc, const char* argv[])
{
    MSG Msg;
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;

    CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "Hello, world!";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = HelloWinProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = 320;
    CreateInfo.by = 240;
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = HWND_DESKTOP;
    
    hMainWnd = CreateMainWindow (&CreateInfo);
    
    printf ("create main window\n");
    if (hMainWnd == HWND_INVALID) {
    	printf ("invalid window handle\n");
        return -1;
    }

    ShowWindow (hMainWnd, SW_SHOWNORMAL);

    while (GetMessage(&Msg, hMainWnd)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    MainWindowThreadCleanup (hMainWnd);

    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

#endif //#if (USE_MINIGUI==1)

