/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.4.26
\***************************************************************************/
	

/***************************************************************************\
    #˵��: miniGUI ����ں���
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  ----------------------------------

	----------------------------------����--------------------------------------
	2004-8-12	����������ͨ��

	----------------------------------ʹ��˵��--------------------------------

\***************************************************************************/
#include "../inc/sysconfig.h"

#include <stdio.h>
#include <minigui/minigui.h>
#include "../ucos-ii/ucos_ii.h"

#if USE_MINIGUI==1

static void* mg_main_task (void* args)
{
    OSTimeDly (400);

    minigui_entry (0, NULL);

    return NULL;
}

void minigui_app_entry (void)
{
    pthread_t main_thread;

//    if (ucos2_malloc_init ()) {
//        fprintf (stderr, "Can not init our own malloc implementation for uC/OS-II.\n");
//        return;
//    }
    if (ucos2_posix_pthread_init ()) {
        fprintf (stderr, "Can not init our own pthread implementation for uC/OS-II.\n");
        return;
    }

	pthread_create (&main_thread, NULL, mg_main_task, NULL);
}

#endif //#if USE_MINIGUI==1

//#ifndef _LITE_VERSION
//#include <minigui/dti.c>
//#endif
