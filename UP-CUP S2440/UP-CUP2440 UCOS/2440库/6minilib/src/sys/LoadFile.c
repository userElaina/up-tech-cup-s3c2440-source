#include  "../inc/drivers.h"
#include "../src/fs/OSFile.h"
#include "../inc/sys/loadfile.h"
#include "../inc/sys/lib.h"
#include <stdio.h>

//volatile unsigned char *downPt;
//extern unsigned int fileSize;

char U12font[]="/sys/ucos/u12x12.fnt";
char U16font[]="/sys/ucos/u16x16.fnt";
char U24font[]="/sys/ucos/u24x24.fnt";

U8 UFont12[256][12];	//°ë×Ö¿í12x12×Ö·û
U8 UCFont12[21312][24];	//È«×Ö¿í12x12×Ö·û
U8 UFont16[256][16];	//°ë×Ö¿í16x16×Ö·û
U8 UCFont16[21312][32];	//È«×Ö¿í16x16×Ö·û

#if LOAD_24FONT_EN==1
U8 UFont24[256][48];	//°ë×Ö¿í24x24×Ö·û
U8 UCFont24[21312][72];	//È«×Ö¿í24x24×Ö·û
#else
U8 UFont24[1][1];	//°ë×Ö¿í24x24×Ö·û
U8 UCFont24[1][1];	//È«×Ö¿í24x24×Ö·û
#endif

U32 sucloadedfile;

U8 Load16Font()
{
	FILE *pFile;

	pFile=fopen(U16font,"r");
	
	if(pFile==NULL){
		printk("16x16 font file is not found!\n");
		LCD_printf("16x16 font file is not found!\n");
		return 0;
	}

	printk("   Loading 16x16 font,please wait...\n");
	LCD_printf("   Loading 16x16 font,\n   please wait...\n");

	fread(UFont16,sizeof(UFont16), 1, pFile);
	fread(UCFont16,sizeof(UCFont16),1, pFile);

	fclose(pFile);
	
	printk("16x16 font loading finished\n");
	LCD_printf("16x16 font loading finished\n");

	sucloadedfile|=LOADU16FONT;
	return 1;
}

U8 Load12Font()
{
	FILE *pFile;

	pFile=fopen(U12font,"r");
	
	if(pFile==NULL){
		printk("12x12 font file is not found!\n");
		LCD_printf("12x12 font file is not found!\n");
		return 0;
	}

	printk("   Loading 12x12 font,please wait...\n");
	LCD_printf("   Loading 12x12 font,\n   please wait...\n");

	fread(UFont12,sizeof(UFont12), 1 , pFile);
	fread(UCFont12,sizeof(UCFont12), 1, pFile);

	fclose(pFile);
	
	printk("12x12 font loading finished\n");
	LCD_printf("12x12 font loading finished\n");

	sucloadedfile|=LOADU12FONT;
	return 1;
}

#if LOAD_24FONT_EN==1
U8 Load24Font()
{
	FILE *pFile;

	pFile=fopen(U24font,"r");
	
	if(pFile==NULL){
		printk("24x24 font file is not found!\n");
		LCD_printf("24x24 font file is not found!\n");
		return 0;
	}

	printk("   Loading 24x24 font,please wait...\n");
	LCD_printf("   Loading 24x24 font,\n   please wait...\n");

	fread(UFont24,sizeof(UFont24), 1, pFile);
	fread(UCFont24,sizeof(UCFont24), 1, pFile);

	fclose(pFile);
	
	printk("24x24 font loading finished\n");
	LCD_printf("24x24 font loading finished\n");

	sucloadedfile|=LOADU24FONT;
	return 1;
}
#endif

U8 LoadFont()
{
	U8 tmp=1;
	tmp&=Load12Font();
	tmp&=Load16Font();

#if LOAD_24FONT_EN==1
	tmp&=Load24Font();
#endif

	return tmp;
}

