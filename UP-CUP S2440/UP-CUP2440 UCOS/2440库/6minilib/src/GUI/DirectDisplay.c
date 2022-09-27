#include "../ucos-ii/includes.h"
#include "../inc/sys/loadfile.h"
#include "../inc/macro.h"
#include "../src/fs/OSFile.h"
#include <math.h>

#include "DirectDisplay.h"

#if USE_MINIGUI==0

void Direct_DrawSBresenham_Line(PDC pdc, int x1, int y1,int x2, int y2);

U32* pDirectLCDBuffer=(U32*)0xc000000;

#if  DIRECT_DISPLAY==1

//#ifdef COLORDEPTH_GRAY

void Direct_SetPixel(PDC pdc, int x, int y, COLORREF color)
{

	U32  temp=0x00000000;
	U32*  p;
	x*=LCDWIDTH;
	x/=pdc->DrawRangex;
	y*=LCDHEIGHT;
	y/=pdc->DrawRangey;
	
	x+=pdc->DrawOrgx+pdc->WndOrgx;
	y+=pdc->DrawOrgy+pdc->WndOrgy;	

      #if  COLORDEPTH_GRAY==1		




	if(x%8==0){
          p=pDirectLCDBuffer+LCDWIDTH/8*y+x/8-1;
	   temp=color;  
		}
	else	{
	   p=pDirectLCDBuffer+LCDWIDTH/8*y+x/8;	
	   temp=color<<(8-x%8)*4;
		}
	
	if(IsInRect(&pdc->DrawRect, x, y)){
		switch(pdc->PenMode){
		case GRAPH_MODE_NORMAL:		
			if(color>0x00000000)
			*p=*p|temp;
			else if(color==0x00000000){
				switch(x%8){
				case  0:
                                   temp=temp+0xfffffff0;
					break;
				case  1:
					temp=temp+0x0fffffff;					
					break;
				case  2:
                                   temp=temp+0xf0ffffff;	
					break;
				case  3:
                                   temp=temp+0xff0fffff;	
					break;
				case  4:
                                   temp=temp+0xfff0ffff;
					break;
				case  5:
                                   temp=temp+0xffff0fff;
					break;
				case  6:
                                   temp=temp+0xfffff0ff;
					break;
				case  7:
                                   temp=temp+0xffffff0f;
					break;				
				}
				*p=*p&temp;	
				}
				
			
			break;
			
		case GRAPH_MODE_OR:
		         *p=*p|temp;
			break;

		case GRAPH_MODE_AND:
			switch(x%8){
				case  0:
                                   temp=temp+0xfffffff0;
					break;
				case  1:
					temp=temp+0x0fffffff;					
					break;
				case  2:
                                   temp=temp+0xf0ffffff;	
					break;
				case  3:
                                   temp=temp+0xff0fffff;	
					break;
				case  4:
                                   temp=temp+0xfff0ffff;
					break;
				case  5:
                                   temp=temp+0xffff0fff;
					break;
				case  6:
                                   temp=temp+0xfffff0ff;
					break;
				case  7:
                                   temp=temp+0xffffff0f;
					break;				
				}
		
		       *p=*p&temp;			   
			break;
		case GRAPH_MODE_XOR:
			*p=*p^temp;		
			break;
		case GRAPH_MODE_NOR:
		       switch(x%8){
				case  0:
                                   temp=temp+0x0000000f;
					break;
				case  1:
					temp=temp+0xf0000000;					
					break;
				case  2:
                                   temp=temp+0x0f000000;	
					break;
				case  3:
                                   temp=temp+0x00f00000;	
					break;
				case  4:
                                   temp=temp+0x000f0000;
					break;
				case  5:
                                   temp=temp+0x0000f000;
					break;
				case  6:
                                   temp=temp+0x00000f00;
					break;
				case  7:
                                   temp=temp+0x000000f0;
					break;				
				}
			   *p=*p^temp;
			break;
		}
	}
	#endif

	#if   COLORDEPTH_GRAY ==0    //��ɫģʽ
	if(IsInRect(&pdc->DrawRect, x, y)){
	if(x%32==0){////xλ��32��������λ��
          p=pDirectLCDBuffer+LCDWIDTH/32*y+x/32-1;
	   if(color==0){///��ɫ
	   
	      switch(pdc->PenMode){
			case GRAPH_MODE_NORMAL:	
		                   *p=*p&0xfffffffe;
				   break;
			case GRAPH_MODE_OR:
				    *p=*p|0x00000000;
				   break;
			case GRAPH_MODE_AND:
				   *p=*p&0xfffffffe;	
				   break;
			case GRAPH_MODE_XOR:
				    *p=*p^0x00000000;		
				    break;
			case GRAPH_MODE_NOR:
				   *p=*p^0x00000000;		
				    break;
	   	}
	   	}
	    else{

			
	      switch(pdc->PenMode){
			case GRAPH_MODE_NORMAL:	
		                   *p=*p|0x00000001;
				   break;
			case GRAPH_MODE_OR:
				    *p=*p|0x00000001;
				   break;
			case GRAPH_MODE_AND:
				   *p=*p;	
				   break;
			case GRAPH_MODE_XOR:
				    *p=*p^0x00000001;		
				    break;
			case GRAPH_MODE_NOR:
				   *p=*p^0x00000001;		
				    break;
	   	}
		
	    	}
	
		}
	else	{////xλ��32�ķ�������λ��
	   p=pDirectLCDBuffer+LCDWIDTH/32*y+x/32;	
	
	   if(color==0){
	   	   temp=~(0x00000001<<(32-x%32));
            switch(pdc->PenMode){
			case GRAPH_MODE_NORMAL:	
		                   *p=*p&temp;
				   break;
			case GRAPH_MODE_OR:
				    *p=*p|0x00000000;
				   break;
			case GRAPH_MODE_AND:
				   *p=*p&temp;
				   break;
			case GRAPH_MODE_XOR:
				    *p=*p^0x00000000;		
				    break;
			case GRAPH_MODE_NOR:
				   *p=*p^0x00000000;		
				    break;
            	}
		
	   	}
	   else{
	   	temp=0x00000001<<(32-x%32);
            switch(pdc->PenMode){
			case GRAPH_MODE_NORMAL:	
		                   *p=*p|temp;
				   break;
			case GRAPH_MODE_OR:
				    *p=*p|temp;
				   break;
			case GRAPH_MODE_AND:
				   *p=*p;	
				   break;
			case GRAPH_MODE_XOR:
				    *p=*p^temp;		
				    break;
			case GRAPH_MODE_NOR:
				   *p=*p^temp;		
				    break;

		
	   	}
	   	
		}

		}
		}
	
	#endif
	
}

void Direct_ClearScreen()
{
    #if   COLORDEPTH_GRAY ==1
	memset(pDirectLCDBuffer,0,LCDWIDTH*LCDHEIGHT/2);
	#endif
	
    #if   COLORDEPTH_GRAY ==0
	memset(pDirectLCDBuffer,0,LCDWIDTH*LCDHEIGHT/8);
	#endif
}

void Direct_SetPixelOR(PDC pdc, int x, int y, COLORREF color)
{
       U32  temp=0x00000000;
	U32*  p;
	x*=LCDWIDTH;
	x/=pdc->DrawRangex;
	y*=LCDHEIGHT;
	y/=pdc->DrawRangey;
	
	x+=pdc->DrawOrgx+pdc->WndOrgx;
	y+=pdc->DrawOrgy+pdc->WndOrgy;	
	
#if   COLORDEPTH_GRAY ==1    //�Ҷ�ģʽ
	p=pDirectLCDBuffer+LCDWIDTH/8*y+x/8;
	temp=color<<(8-x%8)*4;
	*p=*p|temp;
#endif



#if   COLORDEPTH_GRAY ==0    //��ɫģʽ

	if(x%32==0){////xλ��32��������λ��
          p=pDirectLCDBuffer+LCDWIDTH/32*y+x/32-1;
	   if(color==0)///��ɫ  
	      *p=*p|0x00000000;	 	
	    else	      
	      *p=*p|0x00000001;	   	
		}
	else	{////xλ��32�ķ�������λ��
	   p=pDirectLCDBuffer+LCDWIDTH/32*y+x/32;	
	
	   if(color==0)
	   	    *p=*p|0x00000000;		   	
	   else{
	   	temp=0x00000001<<(32-x%32);         
		 *p=*p|temp;
		}
	   	
		}		
		
#endif
	
}
void Direct_SetPixelAND(PDC pdc, int x, int y, COLORREF color)
{
       U32  temp=0x00000000;
	U32*  p;
	x*=LCDWIDTH;
	x/=pdc->DrawRangex;
	y*=LCDHEIGHT;
	y/=pdc->DrawRangey;
	
	x+=pdc->DrawOrgx+pdc->WndOrgx;
	y+=pdc->DrawOrgy+pdc->WndOrgy;	
	
#if   COLORDEPTH_GRAY ==1    //�Ҷ�ģʽ

	p=pDirectLCDBuffer+LCDWIDTH/8*y+x/8;
       temp=color<<(8-x%8)*4;
	switch(x%8){
		case  0:
	                       temp=temp+0xfffffff0;
			break;
		case  1:
			temp=temp+0x0fffffff;					
			break;
		case  2:
	                       temp=temp+0xf0ffffff;	
			break;
		case  3:
	                       temp=temp+0xff0fffff;	
			break;
		case  4:
	                       temp=temp+0xfff0ffff;
			break;
		case  5:
	                       temp=temp+0xffff0fff;
			break;
		case  6:
	                       temp=temp+0xfffff0ff;
			break;
		case  7:
	                       temp=temp+0xffffff0f;
			break;				
		}

	   *p=*p&temp;	
#endif


#if   COLORDEPTH_GRAY ==0    //��ɫģʽ

	if(x%32==0){////xλ��32��������λ��
          p=pDirectLCDBuffer+LCDWIDTH/32*y+x/32-1;
	   if(color==0)///��ɫ  
	         *p=*p&0xfffffffe;	
	    else	      
	         *p=*p;	
		}
	else	{////xλ��32�ķ�������λ��
	   p=pDirectLCDBuffer+LCDWIDTH/32*y+x/32;	
	
	   if(color==0){
	   	   temp=~(0x00000001<<(32-x%32));  
	   	    *p=*p&temp;	
	   	}
	   else{   
	   	temp=0x00000001<<(32-x%32);  
		  *p=*p;  	
		}

		}
		
#endif   


}
void Direct_SetPixelXOR(PDC pdc, int x, int y, COLORREF color)
{
       U32  temp=0x00000000;
	U32*  p;
	x*=LCDWIDTH;
	x/=pdc->DrawRangex;
	y*=LCDHEIGHT;
	y/=pdc->DrawRangey;
	
	x+=pdc->DrawOrgx+pdc->WndOrgx;
	y+=pdc->DrawOrgy+pdc->WndOrgy;	

	
#if   COLORDEPTH_GRAY ==1    //�Ҷ�ģʽ


	p=pDirectLCDBuffer+LCDWIDTH/8*y+x/8;
       temp=color<<(8-x%8)*4;
	*p=*p^temp;	

#endif

#if   COLORDEPTH_GRAY ==0    //��ɫģʽ

	if(x%32==0){////xλ��32��������λ��
          p=pDirectLCDBuffer+LCDWIDTH/32*y+x/32-1;
	   if(color==0)///��ɫ  
	         *p=*p^0x00000000;	
	    else	      
	         *p=*p^0x00000001;	
		}
	else	{////xλ��32�ķ�������λ��
	   p=pDirectLCDBuffer+LCDWIDTH/32*y+x/32;	
	
	     if(color==0){
	   	   temp=~(0x00000001<<(32-x%32));  
	   	    *p=*p&temp;	
	   	}
	   else{   
	   	temp=0x00000001<<(32-x%32);  
		  *p=*p^temp;  	
		}
		}		
		
#endif   

	
}

void Direct_MoveTo(PDC pdc, int x, int y)
{
	pdc->DrawPointx=x;
	pdc->DrawPointy=y;
}


void Direct_LineTo(PDC pdc, int x, int y)
{
       int i,j;
	int x1,x2,y1,y2;
	INT8U err;

//	OSSemPend(Lcd_Disp_Sem,0, &err);
	if(pdc->DrawPointx==x){	//����ֱ��
		if(pdc->DrawPointy<y){
			for(i=pdc->DrawPointy-(pdc->PenWidth-1)/2;i<=y+pdc->PenWidth/2;i++){
				for(j=-(pdc->PenWidth-1)/2;j<=pdc->PenWidth/2;j++){
					Direct_SetPixel(pdc,x+j,i,pdc->PenColor);
				}
			}
		}
		else{
			for(i=pdc->DrawPointy+pdc->PenWidth/2;i>=y-(pdc->PenWidth-1)/2;i--){
				for(j=-(pdc->PenWidth-1)/2;j<=pdc->PenWidth/2;j++){
					Direct_SetPixel(pdc,x+j,i,pdc->PenColor);
				}
			}
		}
		pdc->DrawPointy=y;

		return;
	}

	if(pdc->DrawPointy==y){	//��ˮƽ��
		if(pdc->DrawPointx<x){
			for(i=pdc->DrawPointx-(pdc->PenWidth-1)/2;i<=x+pdc->PenWidth/2;i++){
				for(j=-(pdc->PenWidth-1)/2;j<=pdc->PenWidth/2;j++){
				       Direct_SetPixel(pdc,i,y+j,pdc->PenColor);
				}
			}
		}
		else{
			for(i=pdc->DrawPointx+pdc->PenWidth/2;i>=x-(pdc->PenWidth-1)/2;i--){
				for(j=-(pdc->PenWidth-1)/2;j<=pdc->PenWidth/2;j++){
					Direct_SetPixel(pdc,i,y+j,pdc->PenColor);
				}
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
		Direct_DrawSBresenham_Line(pdc,x1,y1,x2,y2);
	}

	pdc->DrawPointx=x;
	pdc->DrawPointy=y;

}


void Direct_DrawSBresenham_Line(PDC pdc, int x1, int y1,int x2, int y2)
{
	int x, y,dx,dy;
	int e,i;
	dx=x2-x1;
	dy=y2-y1;
	x=x1;	y=y1;

	if ( ABS(dx) >= ABS(dy) ){
		if ( dx >= 0 &&  dy>=0 ){
			e = -dx;
			for ( i = 0; i <= dx; i++ )	{
				Direct_SetPixel(pdc,x++,y,pdc->PenColor);
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
				Direct_SetPixel(pdc,x--,y,pdc->PenColor);
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
				Direct_SetPixel(pdc,x++,y,pdc->PenColor);
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
				Direct_SetPixel(pdc,x--,y,pdc->PenColor);
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
				Direct_SetPixel(pdc,x,y++,pdc->PenColor);
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
				Direct_SetPixel(pdc,x,y--,pdc->PenColor);
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
				Direct_SetPixel(pdc,x,y++,pdc->PenColor);
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
				Direct_SetPixel(pdc,x,y--,pdc->PenColor);
				e +=  2 * dx;
				if (e >= 0)	{
					x = x + 1;
					e +=  2 * dy;
				}
			}
		}
	}
}

void Direct_FillRect(PDC pdc, int left,int top ,int right, int bottom,U32 DrawMode, U32 color)
{
	int i=0;
	int j=0;

	int  temp_left=0;
	int  temp_right=0;

	U32 temp_color=0x00000000;
	U32* p;

       pdc->PenMode=DrawMode;

	i=left;

	#if  COLORDEPTH_GRAY==1	

	
	while(i%8!=0&&i<=right){
		    for( j=top;j<=bottom;j++)
				Direct_SetPixel(pdc, i, j, color);
		    i++;
               }
	if(i==right+1)
		return;
	
	for( j=top;j<=bottom;j++)
				Direct_SetPixel(pdc, i, j, color);
	temp_left=i;

	i=right;
	while(i%8!=0){
		    for(j=top;j<=bottom;j++)
				Direct_SetPixel(pdc, i, j, color);
		    i--;
		}
			
	temp_right=i;

	if(temp_left==temp_right)
		return;
    temp_color=color|color<<4|color<<8|color<<12
	   	            |color<<16|color<<20|color<<24|color<<28;
	   
	p=pDirectLCDBuffer+LCDWIDTH/8*top+temp_left/8;
	for(i=0;i<(temp_right-temp_left)/8;i++){
		for(j=0;j<=(bottom-top);j++){
			switch(DrawMode){
				case   GRAPH_MODE_NORMAL:		
			         	if(color>0x00000000)
					*(p+LCDWIDTH/8*j+i)|=temp_color;
					else if(color==0x00000000)
					*(p+LCDWIDTH/8*j+i)&=temp_color;
					break;
					
                             case GRAPH_MODE_OR:
		                     *(p+LCDWIDTH/8*j+i)|=temp_color;
			              break;

				case GRAPH_MODE_AND:
					*(p+LCDWIDTH/8*j+i)&=temp_color;
					break;
					
                            case GRAPH_MODE_XOR:
			             *(p+LCDWIDTH/8*j+i)^=temp_color;
			             break;

				case GRAPH_MODE_NOR:
					*(p+LCDWIDTH/8*j+i)=~*(p+LCDWIDTH/8*j+i);
					break;
					
				
				}
			}		
		}
#endif


#if  COLORDEPTH_GRAY==0

	
	while(i%32!=0&&i<=right){
		    for( j=top;j<=bottom;j++)
				Direct_SetPixel(pdc, i, j, color);
		    i++;
               }
	if(i==right+1)
		return;
	
	for( j=top;j<=bottom;j++)
				Direct_SetPixel(pdc, i, j, color);
	temp_left=i;

	i=right;
	while(i%32!=0){
		    for(j=top;j<=bottom;j++)
				Direct_SetPixel(pdc, i, j, color);
		    i--;
		}
			
	temp_right=i;

	if(temp_left==temp_right)
		return;

  //     temp_color=color|color<<4|color<<8|color<<12
//	   	            |color<<16|color<<20|color<<24|color<<28;

	if(color>0)	
	       temp_color=0xffffffff;

   
	p=pDirectLCDBuffer+LCDWIDTH/32*top+temp_left/32;
	for(i=0;i<(temp_right-temp_left)/32;i++){
		for(j=0;j<=(bottom-top);j++){
			switch(DrawMode){
				case   GRAPH_MODE_NORMAL:		
			         	if(color>0x00000000)
					*(p+LCDWIDTH/32*j+i)|=temp_color;
					else if(color==0x00000000)
					*(p+LCDWIDTH/32*j+i)&=temp_color;
					break;
					
                             case GRAPH_MODE_OR:
		                     *(p+LCDWIDTH/32*j+i)|=temp_color;
			              break;

				case GRAPH_MODE_AND:
					*(p+LCDWIDTH/32*j+i)&=temp_color;
					break;
					
                            case GRAPH_MODE_XOR:
			             *(p+LCDWIDTH/32*j+i)^=temp_color;
			             break;

				case GRAPH_MODE_NOR:
					*(p+LCDWIDTH/32*j+i)=~*(p+LCDWIDTH/32*j+i);
					break;
					
				
				}
			}		
		}
   
#endif



}
void Direct_FillRect2(PDC pdc, structRECT *rect,U32 DrawMode, U32 color)
{
       Direct_FillRect( pdc, rect->left, rect->top, rect->right, rect->bottom, DrawMode, color);
}
void Direct_Circle(PDC pdc,int x0,int y0,int r)
{
 
	   	
	   int x,y;
          int y1,y2;
          int x1,x2;
          int detx,dety;

       
//////first draw the four special point
          for(x=x0;x>=x0-(int)(r*sqrt(2)/2)-1;x--)
          	{
          	y1=y0+(int)(sqrt(r*r-(x-x0)*(x-x0)));
		y2=y0-(int)(sqrt(r*r-(x-x0)*(x-x0)));

		detx=ABS(x0-x);//det����0��
		dety=ABS(y0-y2);
            			  
              Direct_SetPixel(pdc, x, y1, 0x0000000f);
              Direct_SetPixel(pdc, 2*x0-x, y1, 0x0000000f);
              Direct_SetPixel(pdc, x, y2, 0x0000000f);
              Direct_SetPixel(pdc, 2*x0-x, y2, 0x0000000f);	
			  
              Direct_SetPixel(pdc, x0-dety, y0+detx, 0x0000000f);
	       Direct_SetPixel(pdc, x0+dety, y0+detx, 0x0000000f);
		Direct_SetPixel(pdc, x0-dety, y0-detx, 0x0000000f);
		Direct_SetPixel(pdc, x0+dety, y0-detx, 0x0000000f);		
	       
          	}         
	   
}
/*void Direct_GetChPointer(U8** pChfont, structSIZE* size, U16 ch, U8 bunicode,U8 fnt)
{
	if(bunicode){	//��ʾUNICODE�ַ���
		if(ch<256){	//ASCII�ַ�
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
		//ȫ����ַ�
		if(ch<0x2680)//�����ַ�1
			ch-=0x2600;
		else if(ch<0x27c0)//�����ַ�2
			ch-=0x2700-0x80;
		else if(ch<0xa000)//����
			ch-=0x4e00-0x80-0xc0;
		else//δ�����ַ�
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
}*/
void Direct_CharactorOutRect(PDC pdc,int * x,int* y, structRECT* prect,U16 ch,U8 bunicode, U8 fnt)
{
       U8 *pfont;
	U8 nxbyte;//�ַ���ˮƽռ�õ��ֽ���
	U32 i,j,k,fntclr;
	INT8U err;
	structSIZE size;
	GetChPointer(&pfont, &size, ch, bunicode, fnt);

	nxbyte=size.cx/8;
	if(size.cx%8)
		nxbyte++;

//	OSSemPend(Lcd_Disp_Sem,0, &err);
	switch(fnt&0xc){
	case FONT_TRANSPARENT:	//͸������
		for(i=0;i<size.cy;i++){
			k=7;
			for(j=0;j<size.cx;j++){
				if(IsInRect(prect, j+(*x), i+(*y))){
					if((pfont[i*nxbyte+j/8]>>k)&0x01)
						fntclr=pdc->Fontcolor;
					else
						fntclr=COLOR_WHITE;
					Direct_SetPixelOR(pdc,j+(*x),i+(*y),fntclr);
				}
				k--;
				k&=0x7;
			}
		}
		break;
	case FONT_BLACKBK:	//�ڵװ���
		for(i=0;i<size.cy;i++){
			k=7;
			for(j=0;j<size.cx;j++){
				if(IsInRect(prect, j+(*x), i+(*y))){
					if((~(pfont[i*nxbyte+j/8]>>k))&0x01)
						fntclr=pdc->Fontcolor;
					else
						fntclr=COLOR_WHITE;
					Direct_SetPixel(pdc,j+(*x),i+(*y),fntclr);
				}
				k--;
				k&=0x7;
			}
		}
		break;
	default:	//����ģʽ
		for(i=0;i<size.cy;i++){
			k=7;
			for(j=0;j<size.cx;j++){
				if(IsInRect(prect, j+(*x), i+(*y))){
					if((pfont[i*nxbyte+j/8]>>k)&0x01)
						fntclr=pdc->Fontcolor;
					else
						fntclr=COLOR_WHITE;
					Direct_SetPixel(pdc,j+(*x),i+(*y),fntclr);
				}
				k--;
				k&=0x7;
			}
		}
	}
	(*x)+=size.cx;
}

void Direct_CharactorOut(PDC pdc, int* x, int* y, U16 ch, U8 bunicode, U8 fnt) //��ʾ�����ַ�
{
	U8 *pfont;
	U8 nxbyte;//�ַ���ˮƽռ�õ��ֽ���
	U32 i,j,k,fntclr;
	INT8U err;
	structSIZE size;
       GetChPointer(&pfont, &size, ch, bunicode, fnt);

	nxbyte=size.cx/8;
	if(size.cx%8)
		nxbyte++;

	//OSSemPend(Lcd_Disp_Sem,0, &err);
	switch(fnt&0xc){
	case FONT_TRANSPARENT:	//͸������
		for(i=0;i<size.cy;i++){
			k=7;
			for(j=0;j<size.cx;j++){
				if((pfont[i*nxbyte+j/8]>>k)&0x1)
					fntclr=pdc->Fontcolor;
				else
					fntclr=COLOR_WHITE;
				Direct_SetPixelOR(pdc, j+(*x),i+(*y),fntclr);
				k--;
				k&=0x7;
			}
		}
		break;
	case FONT_BLACKBK:	//�ڵװ���
		for(i=0;i<size.cy;i++){
			k=7;
			for(j=0;j<size.cx;j++){
				if((~(pfont[i*nxbyte+j/8]>>k))&0x01)
					fntclr=pdc->Fontcolor;
					
				else
					fntclr=COLOR_WHITE;
				Direct_SetPixel(pdc,j+(*x),i+(*y),fntclr);
				k--;
				k&=0x7;
			}
		}
		break;
	default:	//����ģʽ
		for(i=0;i<size.cy;i++){
			k=7;
			for(j=0;j<size.cx;j++){
				if((pfont[i*nxbyte+j/8]>>k)&0x01)
					fntclr=pdc->Fontcolor;
				    //   fntclr=0x0000000f;
				else
					fntclr=COLOR_WHITE;
			            
				Direct_SetPixel(pdc,j+(*x),i+(*y),fntclr);
				k--;
				k&=0x7;
			}
		}
	}
	(*x)+=size.cx;
}
void Direct_TextOut(PDC pdc, int x, int y, U16* ch, U8 bunicode, U8 fnt)	//��ʾ����
{
	int i;
	pdc->Fontcolor=0x0000008;
	for(i=0;ch[i]!=0;i++){
		pdc->Fontcolor+=i;
		Direct_CharactorOut(pdc, &x, &y, ch[i], bunicode, fnt);
		
	}
	
}
void Direct_DrawRectFrame(PDC pdc, int left,int top ,int right, int bottom)
{
        Direct_MoveTo(pdc, left, top);
	 Direct_LineTo(pdc, left, bottom);
	 Direct_LineTo(pdc, right, bottom);
	 Direct_LineTo(pdc, right, top);
	 Direct_LineTo(pdc, left, top);

}
void Direct_DrawRectFrame2(PDC pdc, structRECT *rect)
{
        Direct_DrawRectFrame(pdc,rect->left,rect->top,rect->right,rect->bottom);
}
void Direct_Draw3DRect(PDC pdc, int left, int top, int right, int bottom,COLORREF color1, COLORREF color2)
{
         int i;
         pdc->PenColor=color1;
	  Direct_DrawRectFrame(pdc,left,top,right,bottom);
/*	  Direct_MoveTo(pdc,right,top);
	  Direct_LineTo( pdc, left, top);
	  Direct_LineTo( pdc, left, bottom);*/
	  pdc->PenColor=color2;
	  Direct_MoveTo(pdc, right+pdc->PenWidth, top+pdc->PenWidth);
	  Direct_LineTo(pdc, right+pdc->PenWidth, bottom+pdc->PenWidth);
	   Direct_LineTo(pdc,left+pdc->PenWidth, bottom+pdc->PenWidth); 
/*	   Direct_LineTo( pdc, right, bottom);
          Direct_LineTo( pdc, right, top);*/
	  
	  
}
void Direct_ArcTo1(PDC pdc, int x1, int y1, int R)
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

	if(R>0){//�ӻ�
		x0=(int)(0.5*(x1-x2)+tmp*(y1-y2)+x2);	//Բ������
		y0=(int)(0.5*(y1-y2)-tmp*(x1-x2)+y2);
	}
	else{	//�Ż�
		x0=(int)(0.5*(x1-x2)-tmp*(y1-y2)+x2);	//Բ������
		y0=(int)(0.5*(y1-y2)+tmp*(x1-x2)+y2);
	}
	
	x2-=x0;
	y2-=y0;
	x1-=x0;
	y1-=y0;

	delta=Getdelta1(x2,y2, R);

	while ( ABS(x1-x2)>1 || ABS(y1-y2)>1){
		Direct_SetPixel(pdc, x2+x0, y2+y0,pdc->PenColor);
		if(x2>0 && y2>0){	//��һ����
			if ( delta < 0 ){
				delta1 = 2 * ( delta + y2) - 1;
				if ( delta1 <= 0 )
					direction = 1;//ѡ��H��
				else
					direction = 2;//ѡ��D��
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta - x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 2;//ѡ��D��
				else
					direction = 3;//ѡ��V��
			}
			else	//ѡ��D��
				direction = 2;
		}
		else if(x2<0 && y2>0){	//�ڶ�����
			if ( delta < 0 ){
				delta1 = 2 * ( delta + y2) - 1;
				if ( delta1 <= 0 )
					direction = 1;//ѡ��H��
				else
					direction = 8;//ѡ��U��
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta + x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 8;//ѡ��U��
				else
					direction = 7;//ѡ��T��
			}
			else	//ѡ��U��
				direction = 8;
		}
		else if(x2<0 && y2<0){	//��������
			if ( delta < 0 ){
				delta1 = 2 * ( delta - y2) - 1;
				if ( delta1 <= 0 )
					direction = 5;//ѡ��R��
				else
					direction = 6;//ѡ��S��
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta + x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 6;//ѡ��S��
				else
					direction = 7;//ѡ��T��
			}
			else	//ѡ��S��
				direction = 6;
		}
		else if(x2>0 && y2<0){	//��������
			if ( delta < 0 ){
				delta1 = 2 * ( delta - y2) - 1;
				if ( delta1 <= 0 )
					direction = 5;//ѡ��R��
				else
					direction = 4;//ѡ��Q��
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta - x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 4;//ѡ��Q��
				else
					direction = 3;//ѡ��V��
			}
			else	//ѡ��Q��
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

void Direct_ArcTo2(PDC pdc, int x1, int y1, int R)
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

	if(R>0){//�ӻ�
		x0=(int)(0.5*(x1-x2)-tmp*(y1-y2)+x2);	//Բ������
		y0=(int)(0.5*(y1-y2)+tmp*(x1-x2)+y2);
	}
	else{	//�Ż�
		x0=(int)(0.5*(x1-x2)+tmp*(y1-y2)+x2);	//Բ������
		y0=(int)(0.5*(y1-y2)-tmp*(x1-x2)+y2);
	}
	
	x2-=x0;
	y2-=y0;
	x1-=x0;
	y1-=y0;

	delta=Getdelta2(x2,y2, R);
	while ( ABS(x1-x2)>1 || ABS(y1-y2)>1){
		Direct_SetPixel(pdc, x2+x0, y2+y0,pdc->PenColor);
		if(x2>0 && y2>0){	//��һ����
			if ( delta < 0 ){
				delta1 = 2 * ( delta + y2) - 1;
				if ( delta1 <= 0 )
					direction = 5;//ѡ��R��
				else
					direction = 6;//ѡ��S��
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta - x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 6;//ѡ��S��
				else
					direction = 7;//ѡ��T��
			}
			else	//ѡ��S��
				direction = 6;
		}
		else if(x2<0 && y2>0){	//�ڶ�����
			if ( delta < 0 ){
				delta1 = 2 * ( delta + y2) - 1;
				if ( delta1 <= 0 )
					direction = 5;//ѡ��R��
				else
					direction = 4;//ѡ��Q��
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta + x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 4;//ѡ��Q��
				else
					direction = 3;//ѡ��V��
			}
			else	//ѡ��Q��
				direction = 4;
		}
		else if(x2<0 && y2<0){	//��������
			if ( delta < 0 ){
				delta1 = 2 * ( delta - y2) - 1;
				if ( delta1 <= 0 )
					direction = 1;//ѡ��H��
				else
					direction = 2;//ѡ��D��
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta + x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 2;//ѡ��D��
				else
					direction = 3;//ѡ��V��
			}
			else	//ѡ��D��
				direction = 2;
		}
		else if(x2>0 && y2<0){	//��������
			if ( delta < 0 ){
				delta1 = 2 * ( delta - y2) - 1;
				if ( delta1 <= 0 )
					direction = 1;//ѡ��H��
				else
					direction = 8;//ѡ��U��
			}
			else if ( delta > 0 ){
				delta2 = 2 * ( delta - x2 ) - 1;
				if ( delta2 <= 0 )
					direction = 8;//ѡ��U��
				else
					direction = 7;//ѡ��T��
			}
			else	//ѡ��U��
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

	}
}
void Direct_ArcTo(PDC pdc, int x1, int y1, U8 arctype, int R)
{
	INT8U err;

	if(arctype){//��Բ
		ArcTo2(pdc,x1,y1,R);
	}
	else{	//˳Բ
		ArcTo1(pdc,x1,y1,R);
	}
	pdc->DrawPointx=x1;
	pdc->DrawPointy=y1;
	
	
}

#if   COLORDEPTH_GRAY ==0    //��ɫģʽ
void mem8invcpy(U8 *dest, U8 *src, int n)
{
	for(;n>=0;n--){
		*((U8*)((((int)dest)&(~0x3))+3-(((int)src)&0x3)))=*src;
		dest++;
		src++;
	}
}
#endif

void Direct_ShowBmp(PDC pdc, char filename[], int x, int y)
{
	static U8 bmp[4096];
	int i,j,k,nbyte;
	U32 cx,cy;
	U32 color;
	OSFILE* pfile;
	U8 *pbmp;
	U8 *p;

	INT8U err;
	BITMAPFILEHEADER bmpfileheader;
	BITMAPINFOHEADER bmpinfoheader;

	if((pfile=OpenOSFile(filename, FILEMODE_READ))==NULL)
		return;

	ReadOSFile(pfile, (U8*)bmp, 2);

//	if((bmp[0]&0xffff)!='MB' )	//����bmp�ļ�
	if(bmp[0]!='B' ||bmp[1] !='M')
		return;

	ReadOSFile(pfile, (U8*)&bmpfileheader, sizeof(BITMAPFILEHEADER));
	ReadOSFile(pfile, (U8*)&bmpinfoheader, sizeof(BITMAPINFOHEADER));

	cx=bmpinfoheader.biWidth;
	cy=bmpinfoheader.biHeight;

#if   COLORDEPTH_GRAY ==0    //��ɫģʽ

	ReadOSFile(pfile, bmp, 8);

	p=(U8*)(pDirectLCDBuffer);
	p+=LCDWIDTH/8*y+x/8;
	if(bmpinfoheader.biBitCount==1){	//��ɫģʽλͼ
		for(i=cy-1;i>=0;i--){
			pbmp=bmp;
			if(!ReadOSFile(pfile, bmp,cx/8))
				break;

			if(x%8==0){////xλ��8��������λ��
				mem8invcpy(p+LCDWIDTH/8*i, bmp, cx/8);
			}
			else	{////xλ��8�ķ�������λ��

			}
		}
		goto showbmpend;
	}
	
#endif   

	nbyte=bmpinfoheader.biBitCount/8;

	for(i=cy-1;i>=0;i--){
		pbmp=bmp;
		if(!ReadOSFile(pfile, bmp,cx*nbyte+((cx*nbyte)%2)))
			break;
		for(j=0;j<cx;j++){
			color=*pbmp;
			
			for(k=0;k<nbyte-1;k++){
				pbmp++;
			       color+=*pbmp;
			}
			pbmp++;
                     color/=48;  
			
                     color=0x0000000f-color;
			

			 #if COLORDEPTH_GRAY==0
			 if(color>0x00000008)
			 	color=0x00000001;
			 else
			 	color=0;
		        #endif	 
			Direct_SetPixel(pdc,x+j, y+i, color);	
			

                    
		
		
			
		}
	}

showbmpend:
	CloseOSFile(pfile);
}

void Direct_Bmpcopy(U32 *pbmp,int width,int length)
{
       memcpy(pDirectLCDBuffer,pbmp,width*length/2);
}




void Direct_SetPixe_mono(PDC pdc, int x, int y, COLORREF color)
{


U32*  p;
p=pDirectLCDBuffer+x;
*p=1;



}
#endif

#endif //#if USE_MINIGUI==0

