//#include"uhal.h"
//#include"myuart.h"
//#include "..\inc\44blib.h"
//#include "..\inc\drv\flash.h"
#include"osmessage.h"
#include "tchScr.h"
#include "calibrate.h"
#include "LCD.h"
#include"lib.h"
#include"directdisplay.h"
int TchScr_Xmin1=145,TchScr_Xmax1=902,
    TchScr_Ymin1=142,TchScr_Ymax1=902; 

//#pragma import(__use_no_semihosting_swi)  // ensure no functions that use semihosting 

//POINT displaySample[] ={ { 30,  30 }, { 210, 120 }, { 120, 210 }} ;


extern MATRIX  calibrate_matrix;

static int getmatrix(void) ;
//static int getdspdata(POINT1 *ptdsp, POINT1 *pttch) ;
//static void tchscr_calibrate_compare() ;
void tchscr_calibrate_test();


//static unsigned char* fb_mem = ((unsigned char *)0xc000000) ;

typedef struct fb_surface {
	int width;
	int height;
	int pixel_size;
	int bpp;
	int line_size;
	int buffer_size;
//	struct fb_fix_screeninfo finfo;
//	struct fb_var_screeninfo vinfo;
}fb_surface;

fb_surface fb_screen = {
	320,
	240,
	NULL,
	NULL,
	320,
	320*240
	};

//#define printf	Uart_Printf

static int endian_inv[] = {3,1,-1,-3} ; // 0 1 2 3 ---> 3,1,-1,-3
/*static void setpixel(int x, int y, int color)
{
//	char * tmp;
	if(x<0 || x>fb_screen.width || y<0 || y>fb_screen.height)
		return;
//	tmp = (char)(&fb_mem+y*fb_screen.line_size+x) ;
	x+=endian_inv[x%4] ;
	*(fb_mem+y*fb_screen.line_size+x)=color;
//	*tmp = color ;
}*/

#define POINTSIZE		30
#define POINTCOLOR		0xff
static int displayPoint(POINT1 *pt)
{
	int i,j;
	PDC pdc;	
//	memset(fb_mem, 0 , fb_screen.buffer_size);
	pdc=CreateDC();//创建绘图设备上下文
	j=pt->y;
	for(i=pt->x-POINTSIZE/2; i<pt->x+POINTSIZE/2; i++){
		//SetPixel(pdc,i, j, POINTCOLOR);
		SetPixel(pdc,i, j, pdc->PenColor);
	}
	OSTimeDly(300);
	i=pt->x;
	for(j=pt->y-POINTSIZE/2; j<pt->y+POINTSIZE/2; j++){
		SetPixel(pdc,i, j, pdc->PenColor);
	}	
	OSTimeDly(300);	
}
 int getdspdata(POINT1 *ptdsp, POINT1 *pttch,int j)
//static int getdspdata(POINT1 *ptdsp, POINT1 *pttch)
{
//	struct ts_event event;
//	int bytes_read;
	POSMSG pMsg;//定义消息结构
	PDC pdc;	
	//displayPoint(ptdsp);
			printk("\nPlease touch screen x=%d, y=%d\n", (int)ptdsp->x, (int)ptdsp->y);
			pdc=CreateDC();//创建绘图设备上下文
			MoveTo(pdc, ptdsp->x-POINTSIZE/2, ptdsp->y);
			LineTo(pdc,ptdsp->x+POINTSIZE/2,ptdsp->y);
			MoveTo(pdc, ptdsp->x, ptdsp->y-POINTSIZE/2);
			LineTo(pdc,ptdsp->x,ptdsp->y+POINTSIZE/2);
	loop:  pMsg=WaitMessage(0);//等待消息
	       switch(pMsg->Message){
			case OSM_TOUCH_SCREEN://OSM_TOUCH_SCREEN:
				switch(pMsg->LParam){
					case TCHSCR_ACTION_DOWN:
					  //printk("\nPlease touch screen x=%d, y=%d\n", (int)ptdsp->x, (int)ptdsp->y);
						pttch->x=pMsg->WParam&0x0000ffff;
						pttch->y=pMsg->WParam>>16;
						printk("Touch sreen raw x=%d, y=%d\n", (int)pttch->x, (int)pttch->y);
                        //return j=j;
                        break;
					default: goto loop; //return j=j-1;                
	               		//break;
				}
				break;            
           }
     DeleteMessage(pMsg);//删除消息 
    // TchScr_GetOSXY(pttch->x,pttch->y);
     DestoryDC(pdc);//删除绘图设备上下文  
  	// read a data point 
  	//WaitForScrDown(&(pttch->x), &(pttch->y));
/*	do{
		bytes_read = read(pd_fd, &event, sizeof(event));
		if(bytes_read != sizeof(event)) {
			if(bytes_read == -1) {
				printf("[%s] Error reading from touch panel\n", TS_DEVICE);
				return -1;
			}
			printf("[%s] Wrong number of bytes %d read from touch panel "
				"(expected %d)\n", TS_DEVICE, bytes_read, sizeof(event));
			return -1;
		}
	}while(event.pressure!=0);
*/
//	pttch->x = event.x;
//	pttch->y = event.y;
	return 0;
}


/*     the LCD screen will be divided into two areas, the left and the right
  *     In the left, coordinate of the touch point won`t be calibrated.
  *     In the right, coordinate will be calibrated.
  *     Draw lines along two rectangles, and you will see the comparison.
  *
  */
  
typedef struct {
	int left;
	int top;
	int right;
	int bottom;
}structRECT1;

/*    Draw rect
  */
//static void direct_draw_rect(structRECT1 rc, U16 color)
static void direct_draw_rect(structRECT1 rc)
{
	POINT1 tmp;
	PDC pdc;
	pdc=CreateDC();//创建绘图设备上下文
	for(tmp.x=rc.left , tmp.y=rc.top; tmp.x<rc.right; tmp.x++){		//draw horizontal line

		SetPixel(pdc,tmp.x, tmp.y, pdc->PenColor);
		SetPixel(pdc,tmp.x, tmp.y+(rc.bottom-rc.top),pdc->PenColor);
	}
	
	for( tmp.x=rc.left , tmp.y=rc.top; tmp.y<rc.bottom; tmp.y++){	//draw vertical line

		SetPixel(pdc,tmp.x, tmp.y, pdc->PenColor);
		SetPixel(pdc,tmp.x+(rc.right-rc.left), tmp.y,pdc->PenColor);
	}
}

/*static void clear_screen()
{
	int i;
	for(i=0;i<80*240;i++)
		*(pLCDBuffer16+i)=0x00000000;
}*/

/*    Draw sample rect
  */
void draw_sample_rect()
//static void draw_sample_rect()
{	
	PDC pdc;
	//structRECT1 rect_left = {40, 50, 140, 150},
		//	   rect_right = {180, 50, 280, 150} ; 
	ClearScreen();
	OSTimeDly(300);		
	pdc=CreateDC();//创建绘图设备上下文
	//direct_draw_rect(rect_left, POINTCOLOR) ;
	//direct_draw_rect(rect_right, POINTCOLOR) ;
	MoveTo(pdc,40, 50);
	LineTo(pdc,140,50);
	LineTo(pdc,140,150);
	LineTo(pdc,40,150);
	LineTo(pdc,40,50);
	MoveTo(pdc,180, 50);
	LineTo(pdc,280,50);
	LineTo(pdc,280,150);
	LineTo(pdc,180,150);
	LineTo(pdc,180,50);
	DestoryDC(pdc);//删除绘图设备上下文 
	//direct_draw_rect(rect_left) ;
	//direct_draw_rect(rect_right) ;
}
/*void tchscr_calibrate_compare()
//static void tchscr_calibrate_compare()
{
	POINT1 dsppt, tchpt;
	POSMSG pMsg;//定义消息结构
	PDC pdc;
	pdc=CreateDC();//创建绘图设备上下文
	 // read a data point
	//TchScr_GetOSXY(&(tchpt.x), &(tchpt.y));
		pMsg=WaitMessage(0);//等待消息
	       switch(pMsg->Message){
			case OSM_TOUCH_SCREEN://OSM_TOUCH_SCREEN:
				switch(pMsg->LParam){
					case TCHSCR_ACTION_DOWN:
						tchpt.x=pMsg->WParam&0x0000ffff;
						tchpt.y=pMsg->WParam>>16;
                            		break;
					case TCHSCR_ACTION_MOVE:
						tchpt.x=pMsg->WParam&0x0000ffff;
						tchpt.y=pMsg->WParam>>16;                      
	               		break;
				}
				break;
	                     
           }	
        getDisplayPoint(&dsppt, &tchpt, &calibrate_matrix);

	if(dsppt.x < (fb_screen.width / 2)){
		
		tchpt.x=(tchpt.x-TchScr_Xmin1)*LCDWIDTH/(TchScr_Xmax1-TchScr_Xmin1);
		tchpt.y=(tchpt.y-TchScr_Ymin1)*LCDHEIGHT/(TchScr_Ymax1-TchScr_Ymin1);
		//SetPixel(pdc,tchpt.x, tchpt.y, POINTCOLOR);
		SetPixel(pdc,tchpt.x, tchpt.y, pdc->PenColor);
	}			
	else
		//SetPixel(pdc,dsppt.x, dsppt.y, POINTCOLOR);
		SetPixel(pdc,dsppt.x, dsppt.y, pdc->PenColor);
}

/*void tchscr_calibrate_test()
{
	int i;
	int index;//used in Set_UartLoopFunc
	
	for(i=0;i<SAMPLE_NUM;i++) {
		ClearScreen();
		OSTimeDly(300);
		getdspdata(&displaySample[i], &tchSample[i]);
		OSTimeDly(20000) ;
	}

	setCalibrationMatrix( &displaySample[0], &tchSample[0], &calibrate_matrix);

	/*printf("Matrix: An=%ld, Bn=%ld, Cn=%ld\n Dn=%ld, En=%ld, Fn=%ld\nDivider=%ld\n", calibrate_matrix.An,
		calibrate_matrix.Bn, calibrate_matrix.Cn, calibrate_matrix.Dn, calibrate_matrix.En, calibrate_matrix.Fn,
		calibrate_matrix.Divider);*/
	
	//TchScr_Test();
	
	/*#define WAIT_COUNT		(3)
	
	printf("\nTouch pad calibrated.");		
	for(i=0; i<WAIT_COUNT; i++) {
		printf("\nTouch pad calibrate comparison will begin after %d ... ", WAIT_COUNT-i);
		OSTimeDly(10000);
	}	
	printf("\n\nIn the left, touch points are not calibrated.");
	printf("\nIn the right, touch points are calibrated by this method.");
	printf("\nPlease draw along the rectangle, and press 'q' to return.");

	draw_sample_rect() ;	//draw sample rectangle and make the comparison.
	tchscr_calibrate_compare();
	//index=Set_UartLoopFunc(tchscr_calibrate_compare);
	//Uart_Getch(0);
	//Clear_UartLoopFunc(index);	

	printf("\n\nIs set touch pad OK? y/n.");
	
	/*if(Uart_Getch(0) == 'y') {		
		ReadPage(0,0, (unsigned char*)tx_tmpbuf);
		memcpy(tx_tmpbuf+0x24/4, &calibrate_matrix, sizeof(calibrate_matrix));
		Erase_Cluster(0);
		WritePage(0,0, (U8*)tx_tmpbuf);
		
		printf("\nTouch pad set saved.");
	}else
		printf("\nTouch pad set is not saved.");*/
	/*ClearScreen();
	OSTimeDly(300);		
}*/

	
