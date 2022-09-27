#include <stdarg.h>
#include <stdio.h>
#include "inc/lcd320.h"
#include "inc/macro.h"
#include "inc/reg2440.h"

#define LCDCON1_CLKVAL		(1<<8)
#define LCDCON1_MMODE		(0<<7)
#define LCDCON1_PNRMODE	(0x3<<5)
#define LCDCON1_BPPMODE		(0xc<<1)
#define LCDCON1_ENVID		(1)

#define LCDCON2_VBPD 32//32
#define LCDCON2_LINEVAL 479
#define LCDCON2_VFPD 9//9
#define LCDCON2_VSPW 1//
#define LCDCON3_HBPD 47//49
#define LCDCON3_HOZVAL 639
#define LCDCON3_HFPD 15//14
#define LCDCON4_HSPW 95//95
#define LCDCON5_FRM565 1//
#define LCDCON5_INVVCLK 0
#define LCDCON5_INVVLINE 1
#define LCDCON5_INVVFRAME 1
#define LCDCON5_INVVD 0
#define LCDCON5_INVVDEN 0
#define LCDCON5_INVPWREN 0
#define LCDCON5_INVLEND 0
#define LCDCON5_PWREN 1
#define LCDCON5_ENLEND 0
#define LCDCON5_BSWP 0
#define LCDCON5_HWSWP 1
#define BPP24BL 0
#define TPALEN 1
#define LPC_EN 1
#define FIWSEL 0
#define INT_FrSyn 1
#define INT_FiCnt 1
#define MVAL 13
U16* pLCDBuffer16I1=(U16*)0x32000000;
U16* pLCDBuffer16I2=(U16*)0x32096000;
U32 LCDBufferII2[LCDHEIGHT][LCDWIDTH];
U16 LCDBufferII1[307200];
void LCD_Init()
{
	U32 i;
	U32 LCDBASEU,LCDBASEL,LCDBANK;

    rGPCUP=0xffffffff; // Disable Pull-up register
    rGPCCON=0xaaaaaaaa; //Initialize VD[7:0],LCDVF[2:0],VM,VFRAME,VLINE,VCLK,LEND 
    rGPDUP=0xffffffff; // Disable Pull-up register
    rGPDCON=0xaaaaaaaa; //Initialize VD[23:8]
      
    rLCDCON1=0|LCDCON1_BPPMODE|LCDCON1_PNRMODE|LCDCON1_MMODE|LCDCON1_CLKVAL;//|LCDCON1_MMODE;    
	    // disable
	rLCDCON2=(LCDCON2_VBPD<<24)|(LCDCON2_LINEVAL<<14)|(LCDCON2_VFPD<<6)|LCDCON2_VSPW;
	    //320x240LCD  LINEBLANK=15 (without any calculation) 
	rLCDCON3=(LCDCON3_HBPD<<19)|(LCDCON3_HOZVAL<<8)|LCDCON3_HFPD;
    rLCDCON4=LCDCON4_HSPW|(MVAL<<8);
	rLCDCON5=(BPP24BL<<12)|(LCDCON5_FRM565<<11)|(LCDCON5_INVVCLK<<10)|(LCDCON5_INVVLINE<<9)|(LCDCON5_INVVFRAME<<8)|(LCDCON5_INVVD<<7)|\
		     (LCDCON5_INVVDEN<<6)|(LCDCON5_INVPWREN<<5)|(LCDCON5_INVLEND<<4)|(LCDCON5_PWREN<<3)|(LCDCON5_ENLEND<<2)|(LCDCON5_BSWP<<1)|LCDCON5_HWSWP;
	
	LCDBANK=0x32000000>>22;
	LCDBASEU=0x0;	  
	LCDBASEL=LCDBASEU+(480)*640;
	//rLCDADDR1=0x33000000>>1;
	rLCDADDR1= (LCDBANK<<21)|LCDBASEU;    
	rLCDADDR2=LCDBASEL;	
	rLCDADDR3= (640)|(0<<11);
	rLCDINTMSK=(INT_FrSyn<<1)|INT_FiCnt;//|(FIWSEL<<2);
	rLCDLPCSEL=0;
	rTPAL=(0<<24);	    
	for(i=0;i<640*480;i++)
		//*(pLCDBuffer161+i)=0xffff;
		*(pLCDBuffer16I1+i)=0x0;					
	//hudelay(5000);	
    rLCDCON1+=LCDCON1_ENVID;	
  
}

void LCD_Refresh()
{
	int i,j;
	U32 lcddata;
	U16 pixcolor;	//一个像素点的颜色	
	U8* pbuf=(U8*)LCDBufferII2[0];
	U32 LCDBASEU,LCDBASEL,LCDBANK;
	for(i=0;i<LCDWIDTH*LCDHEIGHT;i++){	
			pixcolor=((pbuf[0]&0xf8)<<11)|((pbuf[1]&0xfc)<<6)|(pbuf[2]&0xf8);	//变换RGB		
			pbuf+=4;	
		    //LCDBufferII1[i]=pixcolor;
		    *(pLCDBuffer16I2+i)=pixcolor;
	}
	
	//for(i=0;i<LCDWIDTH*LCDHEIGHT;i++)
	//*(pLCDBuffer16I2+i)=LCDBufferII1[i];
	
	LCDBANK=0x32096000>>22;
	LCDBASEU=(0x32096000<<9)>>10;	  
	LCDBASEL=LCDBASEU+(480)*640;
	//rLCDADDR1=0x33000000>>1;
	rLCDADDR1= (LCDBANK<<21)|LCDBASEU;    
	rLCDADDR2=LCDBASEL;	
	rLCDADDR3= (640)|(0<<11);
	
	
}
