/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.4.26
\***************************************************************************/
	

/***************************************************************************\
    #说明: miniGUI的Sample程序
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  ----------------------------------

	----------------------------------修正--------------------------------------
	2004-8-12	创建，测试通过

	----------------------------------使用说明--------------------------------
	注意:	只把此文件加入工程编译，不要把sample目录下的其他.c
			加入工程

\***************************************************************************/
#include "../inc/sysconfig.h"
#if USE_MINIGUI==1

#include "Sample.h"

#if SAMPLE==SAMPLE_MYHELLO
#include "sample/myhello.c"
#endif

#if SAMPLE==SAMPLE_BitBlt
#include "sample/bitblt.c"
#endif

#if SAMPLE==SAMPLE_BmpBkGnd
#include "sample/bmpbkgnd.c"
#endif

#if SAMPLE==SAMPLE_Button
#include "sample/button.c"
#endif

#if SAMPLE==SAMPLE_Capture
#include "sample/Capture.c"
#endif

#if SAMPLE==SAMPLE_caretdemo
#include "sample/caretdemo.c"
#endif

#if SAMPLE==SAMPLE_combobox
#include "sample/combobox.c"
#endif

#if SAMPLE==SAMPLE_coolbar
#include "sample/coolbar.c"
#endif

#if SAMPLE==SAMPLE_createicon
#include "sample/createicon.c"
#endif

#if SAMPLE==SAMPLE_cursordemo
#include "sample/cursordemo.c"
#endif

#if SAMPLE==SAMPLE_dialogbox
#include "sample/dialogbox.c"
#endif

#if SAMPLE==SAMPLE_drawicon
#include "sample/drawicon.c"
#endif

#if SAMPLE==SAMPLE_edit
#include "sample/edit.c"
#endif

#if SAMPLE==SAMPLE_helloworld
#include "sample/helloword.c"
#endif

#if SAMPLE==SAMPLE_input	
#include "sample/input.c"
#endif

#if SAMPLE==SAMPLE_listbox
#include "sample/listbox.c"
#endif

#if SAMPLE==SAMPLE_listview
#include "sample/listview.c"
#endif

#if SAMPLE==SAMPLE_loadbmp
#include "sample/loadbmp.c"
#endif

#if SAMPLE==SAMPLE_menubutton
#include "sample/menubutton.c"
#endif

#if SAMPLE==SAMPLE_mginit
#include "sample/mginit.c"
#endif

#if SAMPLE==SAMPLE_monthcal
#include "sample/monthcal.c"
#endif

#if SAMPLE==SAMPLE_mycontrol
#include "sample/mycontrol.c"
#endif

#if SAMPLE==SAMPLE_newtoolbar
#include "sample/newtoolbar.c"
#endif

#if SAMPLE==SAMPLE_painter
#include "sample/painter.c"
#endif

#if SAMPLE==SAMPLE_progressbar
#include "sample/progressbar.c"
#endif

#if SAMPLE==SAMPLE_propsheet
#include "sample/propsheet.c"
#endif

#if SAMPLE==SAMPLE_scrnsaver
#include "sample/scrnsaver.c"
#endif

#if SAMPLE==SAMPLE_scrollbar
#include "sample/scrollbar.c"
#endif

#if SAMPLE==SAMPLE_simplekey
#include "sample/simplekey.c"
#endif

#if SAMPLE==SAMPLE_skindemo
#include "sample/skindemo.c"
#endif

#if SAMPLE==SAMPLE_spinbox
#include "sample/spinbox.c"
#endif

#if SAMPLE==SAMPLE_static
#include "sample/static.c"
#endif

#if SAMPLE==SAMPLE_stretchblt
#include "sample/stretchblt.c"
#endif

#if SAMPLE==SAMPLE_timeeditor
#include "sample/timeeditor.c"
#endif

#if SAMPLE==SAMPLE_timezone
#include "sample/timezone.c"
#endif

#if SAMPLE==SAMPLE_trackbar
#include "sample/trackbar.c"
#endif

#if SAMPLE==SAMPLE_treeview
#include "sample/treeview.c"
#endif

#if SAMPLE==SAMPLE_worldtime
#include "sample/worldtime.c"
#endif

#endif //#if USE_MINIGUI==1
