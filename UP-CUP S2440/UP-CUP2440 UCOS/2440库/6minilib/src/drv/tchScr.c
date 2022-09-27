#include "../inc/drivers.h"
#include "../inc/sys/lib.h"
#include <includes.h>
#include "../ucos-ii/add/osaddition.h"
#include "../src/miniGUI/uhal/ucos2input.h"
#include <stdio.h>
#include "inc/drv/reg2410.h"

//#define DPRINTF		printf
#define DPRINTF(...)		//printfNULL

#if OS_Touch_Screen_Task==1

#define IRQ_ADS7843				IRQ_EINT5	//IRQ_EINT5
#define ADS7843_PIN_PEN		(GPIO_MODE_IN | GPIO_PULLUP_EN | GPIO_F5)

#define TS_OPEN_INT()		rINTMSK&=0<<31
#define TS_CLOSE_INT()		rINTMSK|=1<<31

#define ADS7843_PIN_CS		(GPIO_MODE_OUT | GPIO_PULLUP_DIS | GPIO_G12)

/*���ù��ܹܽŶ����*/
#define nYPON 0x3
#define YMON 0x3
#define nXPON 0x3
#define XMON 0x3
/*ACDCON��*/
#define ECFLG_END 1                 // End of A/D conversion
#define PRSCEN_Enable 1			//prescaler Enable
#define PRSCVL_TC 49 					//A/D converter in process
#define SEL_MUX 7 				// xp
#define STDBM_NORMAL 0				//normal mode
#define STDBM_STANDBY 1			//Standby mode
#define READ_START 0			    //Disable start by read operation
#define ENABLE_START 0					//No operation
/*ADC touch screen control register��*/
#define YM_SEN_Hi_Z 0 				//YM=Hi-Z
#define YM_SEN_GND 1 			    //YM=GND
#define YP_SEN_External_voltage 0   //YP=External voltage
#define YP_SEN_AIN5 1       		//conntected with AIN5
#define XM_SEN_Hi_Z 0				
#define XM_SEN_GND  1
#define XP_SEN_External_voltage 0
#define xp_SEN_AIN7 1
#define PULL_UP_ENABLE 0  			//ENABLE
#define PULL_UP_DISABLE 1			//DISABLE
#define AUTO_PST_Normal 0			//Normal ADC conversion
#define AUTO_PST_AUTO   1			//Auto(sequential) x/y positioin conversion mode
#define XY_PST 0					//No operation mode
#define XY_PST_INT 0x3				//waiting for interrupt Mode

/*ADC start or interval delay register*/
#define DELAY 
/*ADC conversion data0 register*/
#define UPDOWN_DOWN_0 0
#define UPDOWN_UP_0 1
#define AUTO_PST_sequencing_0 1
#define XY_PST_0 0
/*ADC conversion data1 register*/
#define UPDOWN_DOWN_1 0
#define UPDOWN_UP_1 1
#define AUTO_PST_sequencing_1 1
#define XY_PST_1 0

//#define LCDWIDTH 640
//#define LCDHEIGHT 480
int TchScr_Xmin=38,TchScr_Xmax=997,
    TchScr_Ymin=23,TchScr_Ymax=974; 

static OS_EVENT* pTouchBeginSem=NULL;

static int penStatus=TCHSCR_ACTION_NULL;
//�����������

//   static int TchScr_Xmax=874,TchScr_Xmin=61, TchScr_Ymax=923,TchScr_Ymin=85; //��������ֵ�������


#define TCHSCR_PenDown()		(!(rADCDAT0&(1<<15)))||(!(rADCDAT1&(1<<15)))
//#define enable_7843()	write_gpio_bit(ADS7843_PIN_CS, 0)
//#define disable_7843()	write_gpio_bit(ADS7843_PIN_CS, 1)

//#define ADS7843_SPI_CHANNEL		0

/*__inline static void set7843toIRQmode(void)
{
	enable_7843();
	SPISend(ADS7843_CMD_X, ADS7843_SPI_CHANNEL);
	SPIRecv(ADS7843_SPI_CHANNEL);
	SPIRecv(ADS7843_SPI_CHANNEL);
	disable_7843();
}*/

static void TchScr_init()
{
	/*���ùܽŹ��ܶ���*/
	rGPGCON &= ~((0x03 << 30)|(0x03 << 28)|(0x03 << 26)|(0x03 << 24));
	rGPGCON|=(nYPON<<30)|(YMON<<28)|(nXPON<<26)|(XMON<<24);
	/*set ACDCON*/
	rADCCON=(PRSCEN_Enable<<14)|(PRSCVL_TC<<6)|(SEL_MUX<<3);
	/*ADC start or interval delay register*/
	rADCDLY=0x12ff;
	/*set ADC touch screen control register*/
	rADCTSC = (0<<8)|(1<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(0<<2)|(3);
}

#define CLOCK_DELAY()		do{int i; for(i=0; i<10; i++);}while(0)

void TchScr_GetScrXY(int *x, int *y)
{
#if OS_CRITICAL_METHOD == 3                  /* Allocate storage for CPU status register               */
    OS_CPU_SR  cpu_sr;
#endif    

	unsigned int temp;
	int x2,y2;
    static int oldx1,oldy1;

	OS_ENTER_CRITICAL();
    
    if((!(rADCDAT0&(1<<15)))||(!(rADCDAT1&(1<<15)))){ /*����д�������*/
       // rADCTSC&=~((1<<3)|(1<<2)|(0));
    	rADCTSC|=(1<<3)|(1<<2)|(0);					  /*�����Զ�ת��ģʽ*/
    	rADCCON|=1;    								  /*��ʼA/Dת��*/
    	while(!(SUBSRCPND&(1<<10)));				  /*�ȴ�A/Dת������*/
    	//while(!rADCCON&(1<<15));                    /*���A/Dת������*/
    	
    	x2=rADCDAT0&0x3ff;    						  /*�ɼ�x��A/Dת��ֵ*/
  
    	y2=rADCDAT1&0x3ff;  						  /*�ɼ�y��A/Dת��ֵ*/
    	

       *x=x2;
       *y=y2;
       oldx1=x2;									  /*�������β���ֵ*/
       oldy1=y2;
       
    }
     
    else{ 											 /*���û�д�����������Ϊ������������ֵ����*/
      *x=oldx1;
      *y=oldy1;
    }
     
    //rADCCON&=~1;
    //rADCTSC&= ~((0<<8)|(1<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(0<<2)|(3));
    rADCTSC = (0<<8)|(1<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(0<<2)|(3);/*�ȴ��ж�ģʽ*/
    SUBSRCPND|=(1<<9); /*INT_TC����*/
    SUBSRCPND|=(1<<10); /*INT_ADC����*/
    /*����ת��*/
    *x=(*x-TchScr_Xmin)*LCDWIDTH/(TchScr_Xmax-TchScr_Xmin);
    *y=(TchScr_Ymax-*y)*LCDHEIGHT/(TchScr_Ymax-TchScr_Ymin);
    //Uart_Printf(0,"\nx=%d",*x);  // by sprife
    //Uart_Printf(0,"y=%d\n",*y);

	OS_EXIT_CRITICAL();

#if 0
	{
		calibrate_POINT dsppt, tchppt;
		tchppt.x=*x;
		tchppt.y=*y;
		getDisplayPoint(&dsppt, &tchppt, &system_param.calibrate_matrix);
		*x=dsppt.x;
		*y=dsppt.y;
		//*x=(*x-TchScr_Xmin)*LCDWIDTH/(TchScr_Xmax-TchScr_Xmin);
	    //*y=(*y-TchScr_Ymin)*LCDHEIGHT/(TchScr_Ymax-TchScr_Ymin);
	}
#endif
}

static void Touch_Screen_ISR(int vector, void* data)
{
	DPRINTF("Touch screen Interrupt\n");

	TS_CLOSE_INT();
	penStatus=TCHSCR_ACTION_DOWN;
	OSSemPost(pTouchBeginSem);
}

extern OS_FLAG_GRP *Input_Flag;
tsdata tchscr_data;

#if USE_MINIGUI==1

#define SendTS(TchScrAction) do\
		{ 	tchscr_data.action = TchScrAction;								\
			OSFlagPost(Input_Flag, UCOS2_MOUSEINPUT, OS_FLAG_SET, &err);		\
		}while(0)
#else

#define SendTS(TchScrAction) do{\
		POSMSG pmsg=OSCreateMessage(NULL, OSM_TOUCH_SCREEN,(tchscr_data.y<<16)|(tchscr_data.x&0xffff), TchScrAction);\
		SendMessage(pmsg);	}while(0)

#endif


void Touch_Screen_Task(void *Id)             //����������
{
	INT8U err,sta=0;
    int i;
	TchScr_init();

	printk("begin TouchScreen Task\n");
	pTouchBeginSem=OSSemCreate(0);

	//���ô������жϴ�����
	TS_OPEN_INT();
	SetISR_Interrupt(IRQ_TC, Touch_Screen_ISR, NULL);//�޸����ж�����by��

	for(;;){
		OSSemPend(pTouchBeginSem, 0, &err);	//wait for Touch Screen Pen down
		OSTimeDly(10);
		for(;;){
			if (TCHSCR_PenDown()){	//�д����ʰ���
				TchScr_GetScrXY(&(tchscr_data.x), &(tchscr_data.y));
				/*SendTS(penStatus);
				penStatus=TCHSCR_ACTION_MOVE;*/
				switch(sta)
       	 	        {case  0: sta=1;break;                   
       	             case  1: sta=3;
       	                        penStatus=TCHSCR_ACTION_DOWN;//�������μ�⵽��������ΪDOWN
				                SendTS(penStatus);
				                break;
				     case  3:  penStatus=TCHSCR_ACTION_MOVE;//�������μ�⵽��������ΪMOVE
				                SendTS(penStatus);
				                break;               
                     default : sta=0;break;
       	 	         }
			}
			else{
//				if(penStatus==TCHSCR_ACTION_DOWN)
//					SendTS(TCHSCR_ACTION_CLICK);

				/*if(penStatus!=TCHSCR_ACTION_DOWN && penStatus!=TCHSCR_ACTION_MOVE){
					break;
				}

				penStatus=TCHSCR_ACTION_UP;
				SendTS(penStatus);
				TS_OPEN_INT();
				break;*/
				switch(sta)
               	         {case 1: sta=0;           // ����ͷ�ǰ�ü�⵽1�ΰ��£���ʼ������˫������
                                        for(i=0;i<7;i++)
                                          {OSTimeDly(10);
                                           if(TCHSCR_PenDown())  //�ڹ涨ʱ�����ּ�⵽��������Ϊ��˫��
                            	                {TchScr_GetScrXY(&(tchscr_data.x), &(tchscr_data.y));
                                                  penStatus=TCHSCR_ACTION_DBCLICK;
		                                          SendTS(penStatus);
                                                  OSTimeDly(60);
                              	                  break;
                                                } 
                                           }   
                                        if(i==7) {         //�ڹ涨ʱ����û�м�⵽�µİ��¶�����Ϊ�ǵ���
                                            penStatus=TCHSCR_ACTION_CLICK;
				                            SendTS(penStatus);}   
                                            break;
				          case 3: sta=0;           //���ͷ�ǰ��⵽2�����ϵİ��¶�����Ϊ��UP
				     	          penStatus=TCHSCR_ACTION_UP;
				                  SendTS(penStatus);
				                  break;
				          default: sta=0;break;
                       	 }
                 TS_OPEN_INT();           //�򿪴������жϲ��˳���ѯ״̬
                 break; 
			}
			OSTimeDly(100);
		}
	}
}

#endif	//OS_Touch_Screen_Task==1
