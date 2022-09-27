#include "../inc/lib.h"
#include <stdio.h>
#include "inc/tchScr.h"
#include "uhal/uhal.h"
#pragma import(__use_no_semihosting_swi)  // ensure no functions that use semihosting 
U32 TchScr_GetOSXY(int *x, int *y)
{//��ô��������겢���ش�������
	static U32 mode=0;
	static int oldx,oldy;
	int i,j;
	for(;;){
	if((mode!=TCHSCR_ACTION_DOWN) && (mode!=TCHSCR_ACTION_MOVE)){	           
			if(!(rADCDAT0&(1<<15))){//�д�������			   
				TchScr_GetScrXY(x, y);//�õ�����������
				oldx=*x;
	            oldy=*y;				
				for(i=0;i<40;i++){				    
					if(rADCDAT0&(1<<15)){//̧��					    					    
						break;
					}						
					hudelay(20);
				}
				if(i<40){	//�ڹ涨��˫��ʱ��֮��̧�𣬼���ǲ��Ǽ�ʱ����
					for(i=0;i<60;i++){					    
						if(!(rADCDAT0&(1<<15))){				
						    SUBSRCPND|=(1<<9);						    
                            if (i<10) {i=60;break;}//���������ܶ�ʱ���ڰ��£�����Ϊ˫��                             
							mode=TCHSCR_ACTION_DBCLICK;
							for(j=0;j<40;j++) hudelay(50);//��⵽˫������ʱ����ֹ��β
                            break;
						}
					hudelay(20);	
                     
					}
					if(i==60)		//û���ڹ涨��ʱ���ڰ���,��Ϊ����
						mode=TCHSCR_ACTION_CLICK;
				}
				else{	//û���ڹ涨��ʱ����̧����Ϊ����
					mode=TCHSCR_ACTION_DOWN;
				}
			  	
              break;
			}
		}
		else{
		    TchScr_GetScrXY(x, y);//�õ�����������
			if(rADCDAT0&(1<<15)){	//̧��
				mode=TCHSCR_ACTION_UP;
				break;
			}
			else{				
				if(ABS(oldx-*x)>25 ||ABS(oldy-*y)>25){//���ƶ�����
					mode=TCHSCR_ACTION_MOVE;
					break;
				}
			}
		}
		
		hudelay(10);
	}
	oldx=*x;
	oldy=*y;
	return mode;
}

void TchScr_Test()
{
	U32 mode;
	int x,y;
	
	Uart_Printf(0,"\nplease touch the screen\n");

	for(;;){
		mode=TchScr_GetOSXY(&x, &y);
		switch(mode){
		case TCHSCR_ACTION_CLICK:			
			Uart_Printf(0,"Action=click:x=%d,\ty=%d\n",x,y);			
			break;
		case TCHSCR_ACTION_DBCLICK:			
			Uart_Printf(0,"Action=double click:x=%d,\ty=%d\n",x,y);
			break;
		case TCHSCR_ACTION_DOWN:			
			Uart_Printf(0,"Action=down:x=%d,\ty=%d\n",x,y);
			break;
		case TCHSCR_ACTION_UP:			
			Uart_Printf(0,"Action=up:x=%d,\ty=%d\n",x,y);
			break;
		case TCHSCR_ACTION_MOVE:			
			Uart_Printf(0,"Action=move:x=%d,\ty=%d\n",x,y);
			break;
		}
		
		hudelay(1000);
	}
}


///*****************�¼�����*****************///


/////////////////////////////////////////////////////
//                  Main function.                //
////////////////////////////////////////////////////
int main(void)
{

	ARMTargetInit();        // do target (uHAL based ARM system) initialisation //
	TchScr_init();
    TchScr_Test();
}    

