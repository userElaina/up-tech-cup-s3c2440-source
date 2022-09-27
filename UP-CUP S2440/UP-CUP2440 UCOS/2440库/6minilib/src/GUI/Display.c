/*-----------------------------------------------------------------------------
    #说明: 绘图函数，包括与硬件无关的缓冲区绘图函数
    #接口函数
		#define SetPixel(pdc, x, y, color)	  Buffer_SetPixel(pdc, x, y, color) 
		#define ClearScreen()                        Buffer_ClearScreen()
		#define MoveTo( pdc,  x,  y)              Buffer_MoveTo( pdc, x,  y)
		#define LineTo( pdc,x,  y)                  Buffer_LineTo( pdc,x,  y)
		#define DrawSBresenham_Line( pdc,  x1,  y1, x2,  y2)                  Buffer_DrawSBresenham_Line( pdc,  x1,  y1, x2,  y2)
		#define  FillRect(pdc, left,top , right,  bottom,DrawMode, color)   Buffer_FillRect(pdc, left,top , right,  bottom,DrawMode, color)
		#define  FillRect2( pdc, rect,DrawMode,color)                                Buffer_FillRect2( pdc, rect,DrawMode,color)
		#define  Circle( pdc,x0,y0, r)                                                           Buffer_Circle( pdc,x0,y0, r)
		#define  CharactorOut( pdc, x, y, ch, bunicode,  fnt)                      Buffer_CharactorOut( pdc, x, y, ch, bunicode,  fnt)
		#define  CharactorOutRect( pdc, x, y, prect, ch, bunicode,  fnt)     Buffer_CharactorOutRect( pdc, x, y, prect, ch, bunicode,  fnt)
		#define  TextOut(pdc, x,  y,  ch,  bunicode,  fnt)                             Buffer_TextOut(pdc, x,  y,  ch,  bunicode,  fnt)
		#define  DrawRectFrame( pdc, left, top , right,  bottom)                Buffer_DrawRectFrame( pdc, left, top , right,  bottom)
		#define  DrawRectFrame2(pdc, rect)                                              Buffer_DrawRectFrame2(pdc, rect)
		#define  Draw3DRect( pdc, left, top, right,bottom,color1,color2)                Buffer_Draw3DRect( pdc, left, top, right,bottom,color1,color2)
		#define  ArcTo1(pdc,  x1, y1, R)                                                      Buffer_ArcTo1(pdc,  x1, y1, R)
		#define  ArcTo2(pdc,  x1, y1, R)                                                      Buffer_ArcTo2(pdc,  x1, y1, R)
		#define  ArcTo(pdc, x1,  y1,  arctype,  R)                                        Buffer_ArcTo(pdc, x1,  y1,  arctype,  R)         
		#define  ShowBmp( pdc,  filename, x,  y)                                        Buffer_ShowBmp( pdc,  filename, x,  y)

	#依赖关系:
	#include "../inc/drv/figure.h"

	#include "..\ucos-ii\includes.h"
	#include "..\inc\drv\lcd320.h"
	#include "..\inc\drv\loadfile.h"
	#include "..\inc\drv\display.h"
	#include "..\inc\maro.h"
	#include "..\inc\OSFile.h"
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2003-6-13	函数通过宏定义实现与底层绘图(是否使用Direct)分隔；
				
	------------------------------------------------------------------------------
	
-----------------------------------------------------------------------------------*/



#include "../ucos-ii/includes.h"
#include "../inc/drv/lcd.h"
#include "../inc/sys/loadfile.h"
#include "display.h"
#include "../inc/macro.h"
#include "../src/fs/OSFile.h"
#include "../inc/sys/lib.h"
#include <math.h>
#include <stdio.h>

#if USE_MINIGUI==0

extern U8 UFont12[256][12];	//半字宽12x12字符
extern U8 UCFont12[21312][24];	//全字宽12x12字符
extern U8 UFont16[256][16];	//半字宽16x16字符
extern U8 UCFont16[21312][32];	//全字宽16x16字符
extern U8 UFont24[256][48];	//半字宽24x24字符
extern U8 UCFont24[21312][72];	//全字宽24x24字符

extern U32 sucloadedfile;
extern OS_EVENT *Lcd_Disp_Sem;
extern OS_EVENT *LCDFresh_MBox;

int OSFontSize[]={0,12,16,24};

#define DCMemPartBlk	(sizeof(DC)+4)

OS_MEM *pDCMem;
INT8U DCMemPart[100][DCMemPartBlk];

#if (DIRECT_DISPLAY==0)
U32 LCDBuffer[LCDHEIGHT][LCDWIDTH];
#endif

void initOSDC()
{
	INT8U err;

	pDCMem=OSMemCreate(DCMemPart,100, DCMemPartBlk, &err);
	if(pDCMem==NULL){
		printk("Failed to Create DC quote\n");
		LCD_printf("Failed to Create DC quote\n");
	}
}

PDC CreateDC()
{
	INT8U err;
	PDC pdc;
	pdc=(PDC)OSMemGet(pDCMem,&err);
	
	pdc->DrawPointx=pdc->DrawPointy=0;//绘图所使用的坐标点
	pdc->PenWidth=1;	//画笔宽度
	pdc->PenMode=GRAPH_MODE_NORMAL; //画笔模式
	pdc->PenColor=COLOR_BLACK;	//画笔的颜色

	pdc->DrawOrgx=0;	//绘图的坐标原点位置
	pdc->DrawOrgy=0;

	pdc->WndOrgx=pdc->WndOrgy=0;

	pdc->DrawRangex=LCDWIDTH;	//绘图的区域范围
	pdc->DrawRangey=LCDHEIGHT;

	SetRect(&pdc->DrawRect, 0,0,LCDWIDTH-1,LCDHEIGHT-1);

	pdc->bUpdataBuffer=TRUE;	//是否更新后台缓冲区及显示
	pdc->Fontcolor=COLOR_BLACK;

	return pdc;
}

void DestoryDC(PDC pdc)
{
	OSMemPut(pDCMem, pdc);
}

void Buffer_SetPixel(PDC pdc, int x, int y, COLORREF color)
{
	static int oldx=0,oldy=0;
	static U32 oldpenMode=0xffffffff;
	x*=LCDWIDTH;
	x/=pdc->DrawRangex;
	y*=LCDHEIGHT;
	y/=pdc->DrawRangey;
	
	x+=pdc->DrawOrgx+pdc->WndOrgx;
	y+=pdc->DrawOrgy+pdc->WndOrgy;

	if(oldy==y && oldx==x && oldpenMode==pdc->PenMode)
		return;
	oldy=y;
	oldx=x;
	oldpenMode=pdc->PenMode;
	
	if(IsInRect(&pdc->DrawRect, x, y)){
		switch(pdc->PenMode){
		case GRAPH_MODE_NORMAL:
			LCDBuffer[y][x]=color;
			break;
		case GRAPH_MODE_OR:
			LCDBuffer[y][x]|=color;
			break;
		case GRAPH_MODE_AND:
			LCDBuffer[y][x]&=color;
			break;
		case GRAPH_MODE_XOR:
			LCDBuffer[y][x]^=color;
			break;
		case GRAPH_MODE_NOR:
			LCDBuffer[y][x]=~(LCDBuffer[y][x]|color);
			break;
		}
	}
}

void SetPixelOR(PDC pdc,int x, int y, COLORREF color)
{
	x*=LCDWIDTH;
	x/=pdc->DrawRangex;
	y*=LCDHEIGHT;
	y/=pdc->DrawRangey;

	x+=pdc->DrawOrgx+pdc->WndOrgx;
	y+=pdc->DrawOrgy+pdc->WndOrgy;
	if(IsInRect(&pdc->DrawRect, x, y))
		LCDBuffer[y][x]|=color;
}

void SetPixelAND(PDC pdc,int x, int y, COLORREF color)
{
	x*=LCDWIDTH;
	x/=pdc->DrawRangex;
	y*=LCDHEIGHT;
	y/=pdc->DrawRangey;

	x+=pdc->DrawOrgx+pdc->WndOrgx;
	y+=pdc->DrawOrgy+pdc->WndOrgy;
	if(IsInRect(&pdc->DrawRect, x, y))
		LCDBuffer[y][x]&=color;
}

void SetPixelXOR(PDC pdc, int x, int y, COLORREF color)
{
	x*=LCDWIDTH;
	x/=pdc->DrawRangex;
	y*=LCDHEIGHT;
	y/=pdc->DrawRangey;

	x+=pdc->DrawOrgx+pdc->WndOrgx;
	y+=pdc->DrawOrgy+pdc->WndOrgy;
	if(IsInRect(&pdc->DrawRect, x, y))
		LCDBuffer[y][x]^=color;
}

int GetFontHeight(U8 fnt)
{
	return OSFontSize[fnt&0x03];
}

int GetFontWidth(U16 ch, U8 fnt)
{
	if(ch<=0xff)
		return OSFontSize[fnt&0x03]/2;
	return OSFontSize[fnt&0x03];
}

int GetUStrFontWidth(U16 str[], U8 fnt)
{
	int length=0;
	for(;(*str)!=0;str++){
		length+=GetFontWidth(*str,fnt);
	}
	return length;
}

///////////获得指向字符首地址的指针////////////////
/////////////pChfont返回的指向字符首地址的指针
/////////////size字符的大小(点数)
/////////////ch字符编码
/////////////bunicode编码类型,是否是Unicode编码
////////////fnt 字体的大型号为FONTSIZE_SMALL FONTSIZE_MIDDLE FONTSIZE_BIG
void GetChPointer(U8** pChfont, structSIZE* size, U16 ch, U8 bunicode,U8 fnt)
{
	if(bunicode){	//显示UNICODE字符集
		if(ch<256){	//ASCII字符
			size->cy=OSFontSize[fnt&0x03];
			size->cx=size->cy/2;
			switch(fnt&0x03){
			case FONTSIZE_SMALL:
				if(sucloadedfile|LOADU12FONT){
					*pChfont=UFont12[ch];
				}
				return;
			case FONTSIZE_MIDDLE:
				if(sucloadedfile|LOADU16FONT){
					*pChfont=UFont16[ch];
				}
				return;
			case FONTSIZE_BIG:
				if(sucloadedfile|LOADU24FONT){
					*pChfont=UFont24[ch];
				}
			}
			return;
		}
		//全宽度字符
		if(ch<0x2680)//特殊字符1
			ch-=0x2600;
		else if(ch<0x27c0)//特殊字符2
			ch-=0x2700-0x80;
		else if(ch<0xa000)//汉字
			ch-=0x4e00-0x80-0xc0;
		else//未定义字符
			ch=0x9fff-0x4e00+0x80+0xc0;

		size->cx=size->cy=OSFontSize[fnt&0x03];

		switch(fnt&0x03){
		case FONTSIZE_SMALL:
			if(sucloadedfile|LOADU12FONT){
				*pChfont=UCFont12[ch];
			}
			return;
		case FONTSIZE_MIDDLE:
			if(sucloadedfile|LOADU16FONT){
				*pChfont=UCFont16[ch];
			}
			return;
		case FONTSIZE_BIG:
			if(sucloadedfile|LOADU24FONT){
				*pChfont=UCFont24[ch];
			}
		}
	}
}

void Buffer_CharactorOut(PDC pdc, int* x, int* y, U16 ch, U8 bunicode, U8 fnt) //显示单个字符
{
	U8 *pfont;
	U8 nxbyte;//字符的水平占用的字节数
	U32 i,j,k,fntclr;
	INT8U err;
	structSIZE size;
	GetChPointer(&pfont, &size, ch, bunicode, fnt);

	nxbyte=size.cx/8;
	if(size.cx%8)
		nxbyte++;

	OSSemPend(Lcd_Disp_Sem,0, &err);
	switch(fnt&0xc){
	case FONT_TRANSPARENT:	//透明背景
		for(i=0;i<size.cy;i++){
			k=7;
			for(j=0;j<size.cx;j++){
				if((pfont[i*nxbyte+j/8]>>k)&0x1)
					fntclr=pdc->Fontcolor;
				else
					fntclr=COLOR_WHITE;
				SetPixelOR(pdc, j+(*x),i+(*y),fntclr);
				k--;
				k&=0x7;
			}
		}
		break;
	case FONT_BLACKBK:	//黑底白字
		for(i=0;i<size.cy;i++){
			k=7;
			for(j=0;j<size.cx;j++){
				if((~(pfont[i*nxbyte+j/8]>>k))&0x01)
					fntclr=pdc->Fontcolor;
				else
					fntclr=COLOR_WHITE;
				SetPixel(pdc,j+(*x),i+(*y),fntclr);
				k--;
				k&=0x7;
			}
		}
		break;
	default:	//正常模式
		for(i=0;i<size.cy;i++){
			k=7;
			for(j=0;j<size.cx;j++){
				if((pfont[i*nxbyte+j/8]>>k)&0x01)
					fntclr=pdc->Fontcolor;
				else
					fntclr=COLOR_WHITE;
				SetPixel(pdc,j+(*x),i+(*y),fntclr);
				k--;
				k&=0x7;
			}
		}
	}
	OSSemPost(Lcd_Disp_Sem);
	(*x)+=size.cx;
}

void Buffer_CharactorOutRect(PDC pdc, int* x, int* y, structRECT* prect ,U16 ch, U8 bunicode, U8 fnt) //在指定矩形的范围内显示单个字符
{
	U8 *pfont;
	U8 nxbyte;//字符的水平占用的字节数
	U32 i,j,k,fntclr;
	INT8U err;
	structSIZE size;
	GetChPointer(&pfont, &size, ch, bunicode, fnt);

	nxbyte=size.cx/8;
	if(size.cx%8)
		nxbyte++;

	OSSemPend(Lcd_Disp_Sem,0, &err);
	switch(fnt&0xc){
	case FONT_TRANSPARENT:	//透明背景
		for(i=0;i<size.cy;i++){
			k=7;
			for(j=0;j<size.cx;j++){
				if(IsInRect(prect, j+(*x), i+(*y))){
					if((pfont[i*nxbyte+j/8]>>k)&0x01)
						fntclr=pdc->Fontcolor;
					else
						fntclr=COLOR_WHITE;
					SetPixelOR(pdc,j+(*x),i+(*y),fntclr);
				}
				k--;
				k&=0x7;
			}
		}
		break;
	case FONT_BLACKBK:	//黑底白字
		for(i=0;i<size.cy;i++){
			k=7;
			for(j=0;j<size.cx;j++){
				if(IsInRect(prect, j+(*x), i+(*y))){
					if((~(pfont[i*nxbyte+j/8]>>k))&0x01)
						fntclr=pdc->Fontcolor;
					else
						fntclr=COLOR_WHITE;
					SetPixel(pdc,j+(*x),i+(*y),fntclr);
				}
				k--;
				k&=0x7;
			}
		}
		break;
	default:	//正常模式
		for(i=0;i<size.cy;i++){
			k=7;
			for(j=0;j<size.cx;j++){
				if(IsInRect(prect, j+(*x), i+(*y))){
					if((pfont[i*nxbyte+j/8]>>k)&0x01)
						fntclr=pdc->Fontcolor;
					else
						fntclr=COLOR_WHITE;
					SetPixel(pdc,j+(*x),i+(*y),fntclr);
				}
				k--;
				k&=0x7;
			}
		}
	}
	OSSemPost(Lcd_Disp_Sem);
	(*x)+=size.cx;

/*	U8 *pfont, *plcdbuf=LCDBuffer+LCDWIDTH/8*(*y);
	U8 nbit,//需要由移动的位数
		nxbyte,//字符的水平占用的字节数
		ncutxbyte, //剪切以后字符的水平占用的字节数
		cutMask[]={0xff,0xff,0xff},//截取的运算掩码
		tmpfont;
	U32 i,j,*ptmp;
	int nCutbit;	//边缘截取字符的水平点数
	INT8U err;
	structSIZE size;

	GetChPointer(&pfont, &size, ch, bunicode, fnt);
	if((*x)>=prect->right){
		(*x)+=size.cx;
		return;
	}

	plcdbuf+=*x/8;
	nbit=*x%8;
	nxbyte=size.cx/8;
	if(size.cx%8)
		nxbyte++;
	ncutxbyte=nxbyte;
	
	nCutbit=(*x)+size.cx-prect->right;
	if(nCutbit>0){	//字符在右边框以外
		ptmp=(U32*)cutMask;
		(*ptmp)>>=nCutbit+24-size.cx;
		ncutxbyte=(size.cx-nCutbit)/8;
	}

	OSSemPend(Lcd_Disp_Sem,0, &err);
	switch(fnt&0xc){
	case FONT_TRANSPARENT:	//透明背景
		for(i=0;i<size.cy;i++){
			for(j=0;j<ncutxbyte;j++){
				tmpfont=pfont[i*nxbyte+j]&cutMask[j];
				*plcdbuf|=(tmpfont>>nbit);
				plcdbuf++;
				*plcdbuf|=tmpfont&(~(0xff<<nbit));
			}
			plcdbuf+=LCDWIDTH/8-ncutxbyte;
		}
		break;
	case FONT_BLACKBK:	//黑底白字
		for(i=0;i<size.cy;i++){
			for(j=0;j<ncutxbyte;j++){
				tmpfont=pfont[i*nxbyte+j]&cutMask[j];
				*plcdbuf|=(0xff>>nbit);
				*plcdbuf&=~(tmpfont>>nbit);
				plcdbuf++;
				*plcdbuf|=(0xff<<(8-nbit));
				*plcdbuf&=~(tmpfont<<(8-nbit));
			}
			plcdbuf+=LCDWIDTH/8-ncutxbyte;
		}
		break;
	default:	//正常模式
		for(i=0;i<size.cy;i++){
			for(j=0;j<ncutxbyte;j++){
				tmpfont=pfont[i*nxbyte+j]&cutMask[j];
				*plcdbuf&=~(0xff>>nbit);
				*plcdbuf|=(tmpfont>>nbit);
				plcdbuf++;
				*plcdbuf&=~(0xff<<(8-nbit));
				*plcdbuf|=(tmpfont<<(8-nbit));
			}
			plcdbuf+=LCDWIDTH/8-ncutxbyte;
		}
	}
	(*x)+=size.cx;
	OSSemPost(Lcd_Disp_Sem);*/
}

void Buffer_TextOut(PDC pdc, int x, int y, U16* ch, U8 bunicode, U8 fnt)	//显示文字
{
	int i;
	for(i=0;ch[i]!=0;i++){
		CharactorOut(pdc, &x, &y, ch[i], bunicode, fnt);
	}
	if(pdc->bUpdataBuffer)
		OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
}

void TextOutRect(PDC pdc, structRECT* prect, U16* ch, U8 bunicode, U8 fnt, U32 outMode)	//在指定矩形的范围内显示文字
{
	int i;
	int x,y,tmp;
	x=prect->left;
	y=prect->top;

	if(outMode&TEXTOUT_MID_Y){
		tmp=(prect->bottom-prect->top-GetFontHeight(fnt))/2;
		y+=tmp;
	}
	if(outMode&TEXTOUT_MID_X){
		tmp=(prect->right-prect->left-GetUStrFontWidth(ch, fnt))/2;
		x+=tmp;
	}
	
	for(i=0;ch[i]!=0;i++){
		CharactorOutRect(pdc,&x, &y, prect, ch[i], bunicode, fnt);
	}
	if(pdc->bUpdataBuffer)
		OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
}

void Buffer_DrawSBresenham_Line(PDC pdc, int x11, int y11,int x22, int y22)
{
	int x, y,dx,dy;
	int e,i;
	dx=x22-x11;
	dy=y22-y11;
	x=x11;	y=y11;

	if ( ABS(dx) >= ABS(dy) ){
		if ( dx >= 0 &&  dy>=0 ){
			e = -dx;
			for ( i = 0; i <= dx; i++ )	{
				SetPixel(pdc,x++,y,pdc->PenColor);
				e += 2 * dy;
				if (e >= 0)	{
					y = y + 1;
					e -= 2 * dx;
				}
			}
		}
		else if( dx < 0 &&  dy<0 ){
			e = -dx;
			for ( i = 0; i >= dx; i-- ){
				SetPixel(pdc,x--,y,pdc->PenColor);
				e += 2 * dy;
				if (e <= 0)	{
					y = y - 1;
					e -=  2 * dx;
				}
			}
		}
		else if(dx>=0 && dy<=0){
			e = dx;
			for ( i = 0; i <= dx; i++ ){
				SetPixel(pdc,x++,y,pdc->PenColor);
				e +=  2 * dy;
				if (e <= 0)	{
					y = y - 1;
					e +=  2 * dx;
				}
			}
		}
		else{	//dx<0 && dy>0
			e = dx;
			for ( i = 0; i >= dx; i-- ){
				SetPixel(pdc,x--,y,pdc->PenColor);
				e +=  2 * dy;
				if (e >= 0)	{
					y = y + 1;
					e +=  2 * dx;
				}
			}
		}
	}
	else{
		if ( dx >= 0 &&  dy>=0 ){
			e = -dy;
			for ( i = 0; i <= dy; i++ )	{
				SetPixel(pdc,x,y++,pdc->PenColor);
				e +=  2 * dx;
				if (e >= 0)	{
					x = x + 1;
					e -=  2 * dy;
				}
			}
		}
		else if( dx < 0 &&  dy<0 ){
			e = -dy;
			for ( i = 0; i >= dy; i-- ){
				SetPixel(pdc,x,y--,pdc->PenColor);
				e +=  2 * dx;
				if (e <= 0)	{
					x = x - 1;
					e -=  2 * dy;
				}
			}
		}
		else if(dx<=0 && dy>=0){
			e = dy;
			for ( i = 0; i <= dy; i++ ){
				SetPixel(pdc,x,y++,pdc->PenColor);
				e +=  2 * dx;
				if (e <= 0)	{
					x = x - 1;
					e +=  2 * dy;
				}
			}
		}
		else{	//dx>0 && dy<0
			e = dy;
			for ( i = 0; i >= dy; i-- ){
				SetPixel(pdc,x,y--,pdc->PenColor);
				e +=  2 * dx;
				if (e >= 0)	{
					x = x + 1;
					e +=  2 * dy;
				}
			}
		}
	}
}

void Buffer_LineTo(PDC pdc, int x, int y)
{
	int i,j;
	int x11,x22,y11,y22;
	INT8U err;

	OSSemPend(Lcd_Disp_Sem,0, &err);
	if(pdc->DrawPointx==x){	//画垂直线
		if(pdc->DrawPointy<y){
			for(i=pdc->DrawPointy-(pdc->PenWidth-1)/2;i<=y+pdc->PenWidth/2;i++){
				for(j=-(pdc->PenWidth-1)/2;j<=pdc->PenWidth/2;j++){
					SetPixel(pdc,x+j,i,pdc->PenColor);
				}
			}
		}
		else{
			for(i=pdc->DrawPointy+pdc->PenWidth/2;i>=y-(pdc->PenWidth-1)/2;i--){
				for(j=-(pdc->PenWidth-1)/2;j<=pdc->PenWidth/2;j++){
					SetPixel(pdc,x+j,i,pdc->PenColor);
				}
			}
		}
		pdc->DrawPointy=y;
		if(pdc->bUpdataBuffer){
			OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
		}
		OSSemPost(Lcd_Disp_Sem);
		return;
	}

	if(pdc->DrawPointy==y){	//画水平线
		if(pdc->DrawPointx<x){
			for(i=pdc->DrawPointx-(pdc->PenWidth-1)/2;i<=x+pdc->PenWidth/2;i++){
				for(j=-(pdc->PenWidth-1)/2;j<=pdc->PenWidth/2;j++){
					SetPixel(pdc,i,y+j,pdc->PenColor);
				}
			}
		}
		else{
			for(i=pdc->DrawPointx+pdc->PenWidth/2;i>=x-(pdc->PenWidth-1)/2;i--){
				for(j=-(pdc->PenWidth-1)/2;j<=pdc->PenWidth/2;j++){
					SetPixel(pdc,i,y+j,pdc->PenColor);
				}
			}
		}
		pdc->DrawPointx=x;
		if(pdc->bUpdataBuffer){
			OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
		}
		OSSemPost(Lcd_Disp_Sem);
		return;
	}
	
	for(i=-(pdc->PenWidth-1)/2;i<=pdc->PenWidth/2;i++){
		x11=pdc->DrawPointx+(i+1)/2;
		x22=x+(i+1)/2;
		y11=(pdc->DrawPointx-x)*i/2/(y-pdc->DrawPointy)+pdc->DrawPointy;
		y22=(pdc->DrawPointx-x)*i/2/(y-pdc->DrawPointy)+y;
		DrawSBresenham_Line(pdc,x11,y11,x22,y22);
	}

	pdc->DrawPointx=x;
	pdc->DrawPointy=y;
	if(pdc->bUpdataBuffer)
		OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD

	OSSemPost(Lcd_Disp_Sem);
}

void Buffer_MoveTo(PDC pdc, int x, int y)
{
	pdc->DrawPointx=x;
	pdc->DrawPointy=y;
}

U8 SetPenWidth(PDC pdc, U8 width)
{
	U8 oldpenwidth=pdc->PenWidth;
	pdc->PenWidth=width;
	return oldpenwidth;
}

void Buffer_DrawRectFrame(PDC pdc, int left,int top ,int right, int bottom)
{
	U8 oldupMode=pdc->bUpdataBuffer;

	pdc->bUpdataBuffer=FALSE;
	MoveTo(pdc,left,top);
	LineTo(pdc,left,bottom);
	LineTo(pdc,right,bottom);
	LineTo(pdc,right,top);
	pdc->bUpdataBuffer=oldupMode;
	LineTo(pdc,left,top);
}

void Buffer_DrawRectFrame2(PDC pdc, structRECT *rect)
{
	DrawRectFrame(pdc,rect->left,rect->top ,rect->right,rect->bottom);
}


void Buffer_FillRect(PDC pdc, int left,int top ,int right, int bottom,U32 DrawMode, U32 color)
{
	int i,j;
	INT8U err;

	OSSemPend(Lcd_Disp_Sem,0, &err);
	for(i=left;i<=right;i++){
		for(j=top;j<=bottom;j++)
			SetPixel(pdc,i,j,color);
	}
	OSSemPost(Lcd_Disp_Sem);

	if(pdc->bUpdataBuffer)
		OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
}

void Buffer_FillRect2(PDC pdc, structRECT *rect,U32 DrawMode, U32 color)
{
	FillRect(pdc, rect->left,rect->top ,rect->right,rect->bottom,DrawMode,color);
}

void Buffer_ClearScreen()
{
	PDC pdc;
	pdc=CreateDC();

	FillRect(pdc,0,0,LCDWIDTH-1,LCDHEIGHT-1,GRAPH_MODE_NORMAL,COLOR_WHITE);

	DestoryDC(pdc);
}

U32 SetPenMode(PDC pdc, U32 mode)
{
	U32 oldMode=pdc->PenMode;
	pdc->PenMode=mode;
	return oldMode;
}

void Buffer_Circle(PDC pdc, int x0, int y0, int r)
{

	int x,y,delta,delta1,delta2,direction;
	INT8U err;
	x=0;	y=r;
	delta=2*(1-r);

	OSSemPend(Lcd_Disp_Sem, 0, &err);
	while(y>=0){
		SetPixel(pdc, x0+x,y0+y, pdc->PenColor);
		SetPixel(pdc, x0+x,y0-y, pdc->PenColor);
		SetPixel(pdc, x0-x,y0+y, pdc->PenColor);
		SetPixel(pdc, x0-x,y0-y, pdc->PenColor);
		if(delta<0){
			delta1=2*(delta+y)-1;
			if(delta1<=0)
				direction=1;
			else
				direction=2;
		}
		else if(delta>0){
			delta2=2*(delta-x)-1;
			if(delta2<=0)
				direction=2;
			else
				direction=3;
		}
		else
			direction=2;
		switch(direction){
		case 1:
			x++;
			delta+=2*x+1;
			break;
		case 2:
			x++;
			y--;
			delta+=2*(x-y+1);
			break;
		case 3:
			y--;
			delta+=-2*y+1;
			break;
		}
	}
	OSSemPost(Lcd_Disp_Sem);
	
	if(pdc->bUpdataBuffer)
		OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
}

int Getdelta1(int x0,int y0, int R)
{
	int delta;

	if(x0>0 && y0>0)	//第一象限
		delta = x0-y0+1;
	else if(x0<0 && y0>0)	//第二象限
		delta = x0+y0+1;
	else if(x0<0 && y0<0)	//第三象限
		delta = -x0+y0+1;
	else if(x0>0 && y0<0)	//第四象限
		delta = -x0-y0+1;
	else
		delta = -ABS(R)+1;

	delta*=2;

	return delta;
}

int Getdelta2(int x0,int y0, int R)
{
	int delta;

	if(x0>0 && y0>0)	//第一象限
		delta = -x0+y0+1;
	else if(x0<0 && y0>0)	//第二象限
		delta = -x0-y0+1;
	else if(x0<0 && y0<0)	//第三象限
		delta = x0-y0+1;
	else if(x0>0 && y0<0)	//第四象限
		delta = x0+y0+1;
	else
		delta = -ABS(R)+1;

	delta*=2;

	return delta;
}

//画顺圆
void Buffer_ArcTo1(PDC pdc, int x1, int y1, int R)
{	
	int	delta, delta1, delta2, direction,x0,y0,dAB2,x2,y2;
	int tmpx,tmpy;
	double tmp;

	x2=pdc->DrawPointx;
	y2=pdc->DrawPointy;

	tmpx=x1-x2;
	tmpy=y1-y2;

	dAB2=tmpx*tmpx+tmpy*tmpy;

	if(dAB2==0)
		return;
	if(R*2>dAB2)
		return;

	tmp=sqrt(((double)R*R)/((double)dAB2)-0.25);

	if(R>0){//劣弧
		x0=(int)(0.5*(x1-x2)+tmp*(y1-y2)+x2);	//圆心坐标
		y0=(int)(0.5*(y1-y2)-tmp*(x1-x2)+y2);
	}
	else{	//优弧
		x0=(int)(0.5*(x1-x2)-tmp*(y1-y2)+x2);	//圆心坐标
		y0=(int)(0.5*(y1-y2)+tmp*(x1-x2)+y2);
	}
	
	x2-=x0;

	y2-=y0;
	x1-=x0;
	y1-=y0;

	delta=Getdelta1(x2,y2, R);

	while ( ABS(x1-x2)>1 || ABS(y1-y2)>1){
		SetPixel(pdc, x2+x0, y2+y0,pdc->PenColor);
		if(x2>0 && y2>0){	//第一象限
			if ( delta < 0 ){
				delta1 = 2 * ( delta + y2) - 1;
				if ( delta1 <= 0 )
					direction = 1;//选择H点
				else
					direction = 2;//选择D点
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta - x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 2;//选择D点
				else
					direction = 3;//选择V点
			}
			else	//选择D点
				direction = 2;
		}
		else if(x2<0 && y2>0){	//第二象限
			if ( delta < 0 ){
				delta1 = 2 * ( delta + y2) - 1;
				if ( delta1 <= 0 )
					direction = 1;//选择H点
				else
					direction = 8;//选择U点
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta + x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 8;//选择U点
				else
					direction = 7;//选择T点
			}
			else	//选择U点
				direction = 8;
		}
		else if(x2<0 && y2<0){	//第三象限
			if ( delta < 0 ){
				delta1 = 2 * ( delta - y2) - 1;
				if ( delta1 <= 0 )
					direction = 5;//选择R点
				else
					direction = 6;//选择S点
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta + x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 6;//选择S点
				else
					direction = 7;//选择T点
			}
			else	//选择S点
				direction = 6;
		}
		else if(x2>0 && y2<0){	//第四象限
			if ( delta < 0 ){
				delta1 = 2 * ( delta - y2) - 1;
				if ( delta1 <= 0 )
					direction = 5;//选择R点
				else
					direction = 4;//选择Q点
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta - x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 4;//选择Q点
				else
					direction = 3;//选择V点
			}
			else	//选择Q点
				direction = 4;
		}
		else{
			if(x2==0){
				if(y2>0)
					x2++;
				else
					x2--;
			}
			else{	//y2==0
				if(x2>0)
					y2--;
				else
					y2++;
			}
			delta=Getdelta1(x2,y2, R);
		}

		switch (direction)	{
		case 1:
			x2++;
			delta += 2 * ABS(x2) + 1;
			break;
		case 2:

			x2++;
			y2--;
			delta += 2 * ( ABS(x2) - ABS(y2) + 1);
			break;
		case 3:
			y2--;
			delta += ( -2 * ABS(y2) + 1 );
			break;
		case 4:
			x2--;
			y2--;
			delta += 2 * ( ABS(x2) - ABS(y2) + 1);
			break;
		case 5:
			x2--;
			delta += 2 * ABS(x2) + 1;
			break;
		case 6:
			x2--;
			y2++;
			delta += 2 * ( ABS(x2) - ABS(y2) + 1);
			break;
		case 7:
			y2++;
			delta += ( -2 * ABS(y2) + 1 );
			break;
		case 8:
			x2++;
			y2++;
			delta += 2 * ( ABS(x2) - ABS(y2) + 1);
			break;
		}
	}
}

//画逆圆
void Buffer_ArcTo2(PDC pdc, int x1, int y1, int R)
{	
	int	delta, delta1, delta2, direction,x0,y0,dAB2,x2,y2;
	int tmpx,tmpy;

	double tmp;

	x2=pdc->DrawPointx;
	y2=pdc->DrawPointy;

	tmpx=x1-x2;
	tmpy=y1-y2;

	dAB2=tmpx*tmpx+tmpy*tmpy;

	if(dAB2==0)
		return;
	if(R*2>dAB2)
		return;

	tmp=sqrt(((double)R*R)/((double)dAB2)-0.25);

	if(R>0){//劣弧
		x0=(int)(0.5*(x1-x2)-tmp*(y1-y2)+x2);	//圆心坐标
		y0=(int)(0.5*(y1-y2)+tmp*(x1-x2)+y2);
	}
	else{	//优弧
		x0=(int)(0.5*(x1-x2)+tmp*(y1-y2)+x2);	//圆心坐标
		y0=(int)(0.5*(y1-y2)-tmp*(x1-x2)+y2);
	}
	
	x2-=x0;
	y2-=y0;
	x1-=x0;
	y1-=y0;

	delta=Getdelta2(x2,y2, R);
	while ( ABS(x1-x2)>1 || ABS(y1-y2)>1){
		SetPixel(pdc, x2+x0, y2+y0,pdc->PenColor);
		if(x2>0 && y2>0){	//第一象限
			if ( delta < 0 ){
				delta1 = 2 * ( delta + y2) - 1;
				if ( delta1 <= 0 )
					direction = 5;//选择R点
				else
					direction = 6;//选择S点
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta - x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 6;//选择S点
				else
					direction = 7;//选择T点
			}
			else	//选择S点
				direction = 6;
		}
		else if(x2<0 && y2>0){	//第二象限
			if ( delta < 0 ){
				delta1 = 2 * ( delta + y2) - 1;
				if ( delta1 <= 0 )
					direction = 5;//选择R点
				else
					direction = 4;//选择Q点
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta + x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 4;//选择Q点
				else
					direction = 3;//选择V点
			}
			else	//选择Q点
				direction = 4;
		}
		else if(x2<0 && y2<0){	//第三象限
			if ( delta < 0 ){
				delta1 = 2 * ( delta - y2) - 1;
				if ( delta1 <= 0 )
					direction = 1;//选择H点
				else
					direction = 2;//选择D点
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta + x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 2;//选择D点
				else
					direction = 3;//选择V点
			}
			else	//选择D点
				direction = 2;
		}
		else if(x2>0 && y2<0){	//第四象限
			if ( delta < 0 ){
				delta1 = 2 * ( delta - y2) - 1;
				if ( delta1 <= 0 )
					direction = 1;//选择H点
				else
					direction = 8;//选择U点
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta - x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 8;//选择U点
				else
					direction = 7;//选择T点
			}
			else	//选择U点
				direction = 8;
		}
		else{
			if(x2==0){
				if(y2>0)
					x2--;
				else
					x2++;
			}
			else{	//y2==0
				if(x2>0)
					y2++;
				else
					y2--;
			}
			delta=Getdelta2(x2,y2, R);
		}
		switch (direction)	{
		case 1:
			x2++;
			delta += 2 * ABS(x2) + 1;
			break;
		case 2:
			x2++;
			y2--;
			delta += 2 * ( ABS(x2) - ABS(y2) + 1);
			break;
		case 3:
			y2--;
			delta += ( -2 * ABS(y2) + 1 );
			break;
		case 4:
			x2--;
			y2--;
			delta += 2 * ( ABS(x2) - ABS(y2) + 1);
			break;
		case 5:
			x2--;
			delta += 2 * ABS(x2) + 1;
			break;
		case 6:
			x2--;
			y2++;
			delta += 2 * ( ABS(x2) - ABS(y2) + 1);
			break;
		case 7:
			y2++;
			delta += ( -2 * ABS(y2) + 1 );
			break;

		case 8:
			x2++;
			y2++;
			delta += 2 * ( ABS(x2) - ABS(y2) + 1);
			break;
		}
//		OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
//		OSTimeDly(100);
	}
}

void Buffer_ArcTo(PDC pdc, int x1, int y1, U8 arctype, int R)
{
	INT8U err;

	OSSemPend(Lcd_Disp_Sem, 0, &err);

	if(arctype){//逆圆
		ArcTo2(pdc,x1,y1,R);
	}
	else{	//顺圆
		ArcTo1(pdc,x1,y1,R);
	}
	pdc->DrawPointx=x1;
	pdc->DrawPointy=y1;
	
	OSSemPost(Lcd_Disp_Sem);

	if(pdc->bUpdataBuffer)
		OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
}

U8 SetLCDUpdata(PDC pdc, U8 isUpdata)
{
	U8 old=pdc->bUpdataBuffer;
	pdc->bUpdataBuffer=isUpdata;

	return old;
}

void Buffer_Draw3DRect(PDC pdc, int left,int top, int right, int bottom, COLORREF color1, COLORREF color2)
{
	int i;
	INT8U err;

	OSSemPend(Lcd_Disp_Sem, 0, &err);
	////绘制左右边框
	for(i=top;i<=bottom;i++){
		SetPixel(pdc, left,i, color1);
		SetPixel(pdc,right,i, color2);
	}
	////绘制上下边框
	for(i=left;i<=right;i++){
		SetPixel(pdc, i,top,color1);
		SetPixel(pdc,i,bottom, color2);
	}
	OSSemPost(Lcd_Disp_Sem);

	if(pdc->bUpdataBuffer)
		OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
}

void Draw3DRect2(PDC pdc, structRECT *rect, COLORREF color1, COLORREF color2)
{
	Draw3DRect(pdc, rect->left,rect->top,rect->right,rect->bottom,color1,color2);
}

U8 GetPenWidth(PDC pdc)
{
	return pdc->PenWidth;
}

U32 GetPenMode(PDC pdc)
{
	return pdc->PenMode;
}

U32 SetPenColor(PDC pdc, U32 color)
{
	U32 old=pdc->PenColor;
	pdc->PenColor=color;
	return old;
}

U32 GetPenColor(PDC pdc)
{
	return pdc->PenColor;
}

void GetBmpSize(char filename[], int* Width, int *Height)
{
	BITMAPFILEHEADER bmpfileheader;
	BITMAPINFOHEADER bmpinfoheader;
	static char bmp[1024];
	FILE* pfile;
	if((pfile=fopen(filename, "r"))==NULL)
		return;

	fread(bmp, 2, 1, pfile);

//	if((bmp[0]&0xffff)!='MB' )	//不是bmp文件
	if(bmp[0]!='B' ||bmp[1] !='M')
		return;

	fread(&bmpfileheader, sizeof(BITMAPFILEHEADER), 1, pfile);
	fread(&bmpinfoheader, sizeof(BITMAPINFOHEADER), 1, pfile);

	*Width=bmpinfoheader.biWidth;
	*Height=bmpinfoheader.biHeight;
}


void Buffer_ShowBmp(PDC pdc, char filename[], int x, int y)
{
	int i,j,k,nbyte;
	U32 cx,cy;
	U32 color;
	FILE* pfile;
	U8 *pbmp;
	static U8 bmp[4096];

	INT8U err;
	BITMAPFILEHEADER bmpfileheader;
	BITMAPINFOHEADER bmpinfoheader;

	if((pfile=fopen(filename, "r"))==NULL)
		return;

	fread(bmp, 2, 1, pfile);

//	if((bmp[0]&0xffff)!='MB' )	//不是bmp文件
	if(bmp[0]!='B' ||bmp[1] !='M')
		return;

	fread(&bmpfileheader, sizeof(BITMAPFILEHEADER), 1, pfile);
	fread(&bmpinfoheader, sizeof(BITMAPINFOHEADER), 1, pfile);

	cx=bmpinfoheader.biWidth;
	cy=bmpinfoheader.biHeight;

	nbyte=bmpinfoheader.biBitCount/8;

	OSSemPend(Lcd_Disp_Sem, 0, &err);
	for(i=cy-1;i>=0;i--){
		pbmp=bmp;
		if(!fread(bmp,cx*nbyte+((cx*nbyte)%2), 1, pfile))
			break;
		for(j=0;j<cx;j++){
			color=*pbmp;
			for(k=0;k<nbyte-1;k++){
				color<<=8;
				pbmp++;
				color|=*pbmp;
			}
			pbmp++;
			SetPixel(pdc,x+j, y+i, color);
		}
	}
	OSSemPost(Lcd_Disp_Sem);

	fclose(pfile);

	if(pdc->bUpdataBuffer)
		OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD

}

void SetDrawOrg(PDC pdc, int x,int y, int* oldx, int *oldy)	//设置绘图的原点
{
	*oldx=pdc->DrawOrgx;
	*oldy=pdc->DrawOrgy;
	pdc->DrawOrgx=x;
	pdc->DrawOrgy=y;
}

void SetDrawRange(PDC pdc, int x,int y, int* oldx, int *oldy)	//设置绘图的范围
{
	*oldx=pdc->DrawRangex;
	*oldy=pdc->DrawRangey;

	if(x==0 && y==0)
		return;

	pdc->DrawRangex=x;
	pdc->DrawRangey=y;

	if(ABS(x)<=1)
		pdc->DrawRangex=y*LCDWIDTH/LCDHEIGHT;

	if(x==-1)	//左右镜像
		pdc->DrawRangex=-pdc->DrawRangex;

	if(ABS(y)<=1)
		pdc->DrawRangey=x*LCDHEIGHT/LCDWIDTH;
	if(y==-1)	//上下镜像
		pdc->DrawRangey=-pdc->DrawRangey;
}

////////////////////////有延时的绘图///////////////////////////////

void DrawSBresenham_LineDelay(PDC pdc, int x1, int y1,int x2, int y2, int ticks)
{
	INT8U err;
	int x, y,dx,dy;
	int e,i;
	dx=x2-x1;
	dy=y2-y1;
	x=x1;	y=y1;

	if ( ABS(dx) >= ABS(dy) ){
		if ( dx >= 0 &&  dy>=0 ){
			e = -dx;
			for ( i = 0; i <= dx; i++ )	{
				OSSemPend(Lcd_Disp_Sem,0, &err);
				SetPixel(pdc,x++,y,pdc->PenColor);
				e += 2 * dy;
				if (e >= 0)	{
					y = y + 1;
					e -= 2 * dx;
				}
				OSTimeDly(ticks);
				OSSemPost(Lcd_Disp_Sem);
				OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
			}
		}
		else if( dx < 0 &&  dy<0 ){
			e = -dx;
			for ( i = 0; i >= dx; i-- ){
				OSSemPend(Lcd_Disp_Sem,0, &err);
				SetPixel(pdc,x--,y,pdc->PenColor);
				e += 2 * dy;
				if (e <= 0)	{
					y = y - 1;
					e -=  2 * dx;
				}
				OSTimeDly(ticks);
				OSSemPost(Lcd_Disp_Sem);
				OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
			}
		}
		else if(dx>=0 && dy<=0){
			e = dx;
			for ( i = 0; i <= dx; i++ ){
				OSSemPend(Lcd_Disp_Sem,0, &err);
				SetPixel(pdc,x++,y,pdc->PenColor);
				e +=  2 * dy;
				if (e <= 0)	{
					y = y - 1;
					e +=  2 * dx;
				}
				OSTimeDly(ticks);
				OSSemPost(Lcd_Disp_Sem);
				OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
			}
		}
		else{	//dx<0 && dy>0
			e = dx;
			for ( i = 0; i >= dx; i-- ){
				OSSemPend(Lcd_Disp_Sem,0, &err);
				SetPixel(pdc,x--,y,pdc->PenColor);
				e +=  2 * dy;
				if (e >= 0)	{
					y = y + 1;
					e +=  2 * dx;
				}
				OSTimeDly(ticks);
				OSSemPost(Lcd_Disp_Sem);
				OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
			}
		}
	}
	else{
		if ( dx >= 0 &&  dy>=0 ){
			e = -dy;
			for ( i = 0; i <= dy; i++ )	{
				OSSemPend(Lcd_Disp_Sem,0, &err);
				SetPixel(pdc,x,y++,pdc->PenColor);
				e +=  2 * dx;
				if (e >= 0)	{
					x = x + 1;
					e -=  2 * dy;
				}
				OSTimeDly(ticks);
				OSSemPost(Lcd_Disp_Sem);
				OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
			}
		}
		else if( dx < 0 &&  dy<0 ){
			e = -dy;
			for ( i = 0; i >= dy; i-- ){
				OSSemPend(Lcd_Disp_Sem,0, &err);
				SetPixel(pdc,x,y--,pdc->PenColor);
				e +=  2 * dx;
				if (e <= 0)	{
					x = x - 1;
					e -=  2 * dy;
				}
				OSTimeDly(ticks);
				OSSemPost(Lcd_Disp_Sem);
				OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
			}
		}
		else if(dx<=0 && dy>=0){
			e = dy;
			for ( i = 0; i <= dy; i++ ){
				OSSemPend(Lcd_Disp_Sem,0, &err);
				SetPixel(pdc,x,y++,pdc->PenColor);
				e +=  2 * dx;
				if (e <= 0)	{
					x = x - 1;
					e +=  2 * dy;
				}
				OSTimeDly(ticks);
				OSSemPost(Lcd_Disp_Sem);
				OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
			}
		}
		else{	//dx>0 && dy<0
			e = dy;
			for ( i = 0; i >= dy; i-- ){
				OSSemPend(Lcd_Disp_Sem,0, &err);
				SetPixel(pdc,x,y--,pdc->PenColor);
				e +=  2 * dx;
				if (e >= 0)	{
					x = x + 1;
					e +=  2 * dy;
				}
				OSTimeDly(ticks);
				OSSemPost(Lcd_Disp_Sem);
				OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
			}
		}
	}
}

void LineToDelay(PDC pdc, int x, int y, int ticks)
{
	int i,j;
	int x1,x2,y1,y2;
	INT8U err;

	if(pdc->DrawPointx==x){	//画垂直线
		if(pdc->DrawPointy<y){
			for(i=pdc->DrawPointy-(pdc->PenWidth-1)/2;i<=y+pdc->PenWidth/2;i++){
				OSSemPend(Lcd_Disp_Sem,0, &err);
				for(j=-(pdc->PenWidth-1)/2;j<=pdc->PenWidth/2;j++){
					SetPixel(pdc,x+j,i,pdc->PenColor);
				}
				OSTimeDly(ticks);
				OSSemPost(Lcd_Disp_Sem);
				OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
			}
		}
		else{
			for(i=pdc->DrawPointy+pdc->PenWidth/2;i>=y-(pdc->PenWidth-1)/2;i--){
				OSSemPend(Lcd_Disp_Sem,0, &err);
				for(j=-(pdc->PenWidth-1)/2;j<=pdc->PenWidth/2;j++){
					SetPixel(pdc, x+j,i,pdc->PenColor);
				}
				OSTimeDly(ticks);
				OSSemPost(Lcd_Disp_Sem);
				OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
			}
		}
		pdc->DrawPointy=y;
		return;
	}

	if(pdc->DrawPointy==y){	//画水平线
		if(pdc->DrawPointx<x){
			for(i=pdc->DrawPointx-(pdc->PenWidth-1)/2;i<=x+pdc->PenWidth/2;i++){
				OSSemPend(Lcd_Disp_Sem,0, &err);
				for(j=-(pdc->PenWidth-1)/2;j<=pdc->PenWidth/2;j++){
					SetPixel(pdc,i,y+j,pdc->PenColor);
				}
				OSTimeDly(ticks);
				OSSemPost(Lcd_Disp_Sem);
				OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
			}
		}
		else{
			for(i=pdc->DrawPointx+pdc->PenWidth/2;i>=x-(pdc->PenWidth-1)/2;i--){
				OSSemPend(Lcd_Disp_Sem,0, &err);
				for(j=-(pdc->PenWidth-1)/2;j<=pdc->PenWidth/2;j++){
					SetPixel(pdc, i,y+j,pdc->PenColor);
				}
				OSTimeDly(ticks);
				OSSemPost(Lcd_Disp_Sem);
				OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
			}
		}
		pdc->DrawPointx=x;
		return;
	}
	
	for(i=-(pdc->PenWidth-1)/2;i<=pdc->PenWidth/2;i++){
		x1=pdc->DrawPointx+(i+1)/2;
		x2=x+(i+1)/2;
		y1=(pdc->DrawPointx-x)*i/2/(y-pdc->DrawPointy)+pdc->DrawPointy;
		y2=(pdc->DrawPointx-x)*i/2/(y-pdc->DrawPointy)+y;
		DrawSBresenham_LineDelay(pdc,x1,y1,x2,y2,ticks);
	}

	pdc->DrawPointx=x;
	pdc->DrawPointy=y;
}

//画顺圆
void ArcToDelay1(PDC pdc, int x1, int y1, int R, int ticks)
{	
	int delta, delta1, delta2, direction,x0,y0,dAB2,x2,y2;
	int tmpx,tmpy;
	double tmp;

	INT8U err;

	x2=pdc->DrawPointx;
	y2=pdc->DrawPointy;


	tmpx=x1-x2;
	tmpy=y1-y2;

	dAB2=tmpx*tmpx+tmpy*tmpy;

	if(dAB2==0)
		return;
	if(R*2>dAB2)
		return;

	tmp=sqrt(((double)R*R)/((double)dAB2)-0.25);

	if(R>0){//劣弧
		x0=(int)(0.5*(x1-x2)+tmp*(y1-y2)+x2);	//圆心坐标
		y0=(int)(0.5*(y1-y2)-tmp*(x1-x2)+y2);
	}
	else{	//优弧
		x0=(int)(0.5*(x1-x2)-tmp*(y1-y2)+x2);	//圆心坐标
		y0=(int)(0.5*(y1-y2)+tmp*(x1-x2)+y2);
	}
	
	x2-=x0;
	y2-=y0;
	x1-=x0;
	y1-=y0;

	delta=Getdelta1(x2,y2, R);

	while ( ABS(x1-x2)>1 || ABS(y1-y2)>1){
		OSSemPend(Lcd_Disp_Sem, 0, &err);
		SetPixel(pdc,x2+x0, y2+y0,pdc->PenColor);
		if(x2>0 && y2>0){	//第一象限
			if ( delta < 0 ){
				delta1 = 2 * ( delta + y2) - 1;
				if ( delta1 <= 0 )
					direction = 1;//选择H点
				else
					direction = 2;//选择D点
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta - x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 2;//选择D点
				else
					direction = 3;//选择V点
			}
			else	//选择D点
				direction = 2;
		}
		else if(x2<0 && y2>0){	//第二象限
			if ( delta < 0 ){
				delta1 = 2 * ( delta + y2) - 1;
				if ( delta1 <= 0 )
					direction = 1;//选择H点
				else
					direction = 8;//选择U点
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta + x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 8;//选择U点
				else
					direction = 7;//选择T点
			}
			else	//选择U点
				direction = 8;
		}
		else if(x2<0 && y2<0){	//第三象限
			if ( delta < 0 ){
				delta1 = 2 * ( delta - y2) - 1;
				if ( delta1 <= 0 )
					direction = 5;//选择R点
				else
					direction = 6;//选择S点
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta + x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 6;//选择S点
				else
					direction = 7;//选择T点
			}
			else	//选择S点
				direction = 6;
		}
		else if(x2>0 && y2<0){	//第四象限
			if ( delta < 0 ){
				delta1 = 2 * ( delta - y2) - 1;
				if ( delta1 <= 0 )
					direction = 5;//选择R点
				else
					direction = 4;//选择Q点
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta - x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 4;//选择Q点
				else
					direction = 3;//选择V点
			}
			else	//选择Q点
				direction = 4;
		}
		else{
			if(x2==0){
				if(y2>0)
					x2++;
				else
					x2--;
			}
			else{	//y2==0
				if(x2>0)
					y2--;
				else
					y2++;
			}
			delta=Getdelta1(x2,y2, R);
		}

		switch (direction)	{
		case 1:
			x2++;
			delta += 2 * ABS(x2) + 1;
			break;
		case 2:
			x2++;
			y2--;
			delta += 2 * ( ABS(x2) - ABS(y2) + 1);
			break;
		case 3:
			y2--;
			delta += ( -2 * ABS(y2) + 1 );
			break;
		case 4:
			x2--;
			y2--;
			delta += 2 * ( ABS(x2) - ABS(y2) + 1);
			break;
		case 5:
			x2--;
			delta += 2 * ABS(x2) + 1;
			break;
		case 6:
			x2--;
			y2++;
			delta += 2 * ( ABS(x2) - ABS(y2) + 1);
			break;
		case 7:
			y2++;
			delta += ( -2 * ABS(y2) + 1 );
			break;
		case 8:
			x2++;
			y2++;
			delta += 2 * ( ABS(x2) - ABS(y2) + 1);
			break;
		}
		OSTimeDly(ticks);
		OSSemPost(Lcd_Disp_Sem);
		OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
	}
}


//画逆圆
void ArcToDelay2(PDC pdc, int x1, int y1, int R, int ticks)
{	
	int delta, delta1, delta2, direction,x0,y0,dAB2,x2,y2;
	int tmpx,tmpy;
	double tmp;
	INT8U err;

	x2=pdc->DrawPointx;
	y2=pdc->DrawPointy;

	tmpx=x1-x2;
	tmpy=y1-y2;

	dAB2=tmpx*tmpx+tmpy*tmpy;

	if(dAB2==0)
		return;
	if(R*2>dAB2)
		return;

	tmp=sqrt(((double)R*R)/((double)dAB2)-0.25);

	if(R>0){//劣弧
		x0=(int)(0.5*(x1-x2)-tmp*(y1-y2)+x2);	//圆心坐标
		y0=(int)(0.5*(y1-y2)+tmp*(x1-x2)+y2);
	}
	else{	//优弧
		x0=(int)(0.5*(x1-x2)+tmp*(y1-y2)+x2);	//圆心坐标
		y0=(int)(0.5*(y1-y2)-tmp*(x1-x2)+y2);
	}
	
	x2-=x0;
	y2-=y0;
	x1-=x0;
	y1-=y0;

	delta=Getdelta2(x2,y2, R);
	while ( ABS(x1-x2)>1 || ABS(y1-y2)>1){
		OSSemPend(Lcd_Disp_Sem, 0, &err);
		SetPixel(pdc,x2+x0, y2+y0,pdc->PenColor);
		if(x2>0 && y2>0){	//第一象限
			if ( delta < 0 ){
				delta1 = 2 * ( delta + y2) - 1;
				if ( delta1 <= 0 )
					direction = 5;//选择R点
				else
					direction = 6;//选择S点
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta - x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 6;//选择S点
				else
					direction = 7;//选择T点
			}
			else	//选择S点
				direction = 6;
		}
		else if(x2<0 && y2>0){	//第二象限
			if ( delta < 0 ){
				delta1 = 2 * ( delta + y2) - 1;
				if ( delta1 <= 0 )
					direction = 5;//选择R点
				else
					direction = 4;//选择Q点
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta + x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 4;//选择Q点
				else
					direction = 3;//选择V点
			}
			else	//选择Q点
				direction = 4;
		}
		else if(x2<0 && y2<0){	//第三象限
			if ( delta < 0 ){
				delta1 = 2 * ( delta - y2) - 1;
				if ( delta1 <= 0 )
					direction = 1;//选择H点
				else
					direction = 2;//选择D点
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta + x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 2;//选择D点
				else
					direction = 3;//选择V点
			}
			else	//选择D点
				direction = 2;
		}
		else if(x2>0 && y2<0){	//第四象限
			if ( delta < 0 ){
				delta1 = 2 * ( delta - y2) - 1;
				if ( delta1 <= 0 )
					direction = 1;//选择H点
				else
					direction = 8;//选择U点
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta - x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 8;//选择U点
				else
					direction = 7;//选择T点
			}
			else	//选择U点
				direction = 8;
		}
		else{
			if(x2==0){
				if(y2>0)
					x2--;
				else
					x2++;
			}
			else{	//y2==0
				if(x2>0)
					y2++;
				else
					y2--;
			}
			delta=Getdelta2(x2,y2, R);
		}
		switch (direction)	{
		case 1:
			x2++;
			delta += 2 * ABS(x2) + 1;
			break;
		case 2:
			x2++;
			y2--;
			delta += 2 * ( ABS(x2) - ABS(y2) + 1);
			break;
		case 3:
			y2--;
			delta += ( -2 * ABS(y2) + 1 );
			break;
		case 4:
			x2--;
			y2--;
			delta += 2 * ( ABS(x2) - ABS(y2) + 1);
			break;
		case 5:
			x2--;
			delta += 2 * ABS(x2) + 1;
			break;
		case 6:
			x2--;
			y2++;
			delta += 2 * ( ABS(x2) - ABS(y2) + 1);
			break;
		case 7:
			y2++;
			delta += ( -2 * ABS(y2) + 1 );
			break;
		case 8:
			x2++;
			y2++;
			delta += 2 * ( ABS(x2) - ABS(y2) + 1);
			break;
		}
		OSTimeDly(ticks);
		OSSemPost(Lcd_Disp_Sem);
		OSMboxPost(LCDFresh_MBox,(void*)1);	//刷新LCD
	}
}

void ArcToDelay(PDC pdc, int x1, int y1, U8 arctype, int R, int ticks)
{
	if(arctype){//逆圆
		ArcToDelay2(pdc,x1,y1,R,ticks);
	}
	else{	//顺圆
		ArcToDelay1(pdc,x1,y1,R,ticks);
	}
	pdc->DrawPointx=x1;
	pdc->DrawPointy=y1;
}

#endif //#if USE_MINIGUI==0

