/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.4.26
\***************************************************************************/


/***************************************************************************\
    #说明: C库函数等定义
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2004-11-16	为yaffs文件系统添加了更多的处理函数，ReadSparePage，ReadPage512，
			WriteSparePage，WritePage512， CheckPageEreased等

	2004-5-2	1、创建

\***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/drv/reg2410.h"
#include "../inc/drv/flash.h"
#include "../inc/macro.h"
#include "../inc/sys/lib.h"

#define BAD_CHECK	(1)
#define ECC_CHECK	(1)
#define WRITEVERIFY  (0)  //Verifing is enable when writing flash
#define RAM_DISK		0

#define fNFCONF_TWRPH1			Fld(3,0)
#define NFCONF_TWRPH1			FMsk(fNFCONF_TWRPH1)
#define NFCONF_TWRPH1_0		FInsrt(0x0, fNFCONF_TWRPH1) /* 0 */
#define fNFCONF_TWRPH0			Fld(3,4)
#define NFCONF_TWRPH0			FMsk(fNFCONF_TWRPH0)
#define NFCONF_TWRPH0_3		FInsrt(0x3, fNFCONF_TWRPH0) /* 3 */
#define fNFCONF_TACLS			Fld(3,8)
#define NFCONF_TACLS			FMsk(fNFCONF_TACLS)
#define NFCONF_TACLS_0			FInsrt(0x0, fNFCONF_TACLS) /* 0 */
#define fNFCONF_nFCE			Fld(1,11)
#define NFCONF_nFCE				FMsk(fNFCONF_nFCE)
#define NFCONF_nFCE_LOW		FInsrt(0x0, fNFCONF_nFCE) /* active */
#define NFCONF_nFCE_HIGH		FInsrt(0x1, fNFCONF_nFCE) /* inactive */
#define fNFCONF_ECC				Fld(1,12)
#define NFCONF_ECC				FMsk(fNFCONF_ECC)
#define NFCONF_ECC_NINIT		FInsrt(0x0, fNFCONF_ECC) /* not initialize */
#define NFCONF_ECC_INIT		FInsrt(0x1, fNFCONF_ECC)    /* initialize */
#define fNFCONF_ADDRSTEP		Fld(1,13)                 /* Addressing Step */
#define NFCONF_ADDRSTEP		FMsk(fNFCONF_ADDRSTEP)
#define fNFCONF_PAGESIZE		Fld(1,14)
#define NFCONF_PAGESIZE		FMsk(fNFCONF_PAGESIZE)
#define NFCONF_PAGESIZE_256	FInsrt(0x0, fNFCONF_PAGESIZE) /* 256 bytes */
#define NFCONF_PAGESIZE_512	FInsrt(0x1, fNFCONF_PAGESIZE) /* 512 bytes */
#define fNFCONF_FCTRL			Fld(1,15)  /* Flash controller enable/disable */
#define NFCONF_FCTRL			FMsk(fNFCONF_FCTRL)
#define NFCONF_FCTRL_DIS		FInsrt(0x0, fNFCONF_FCTRL) /* Disable */
#define NFCONF_FCTRL_EN		FInsrt(0x1, fNFCONF_FCTRL) /* Enable */

#define NFSTAT_RnB				(1 << 0)
#define NFSTAT_nFWE			(1 << 8)
#define NFSTAT_nFRE				(1 << 9)
#define NFSTAT_ALE				(1 << 10)
#define NFSTAT_CLE				(1 << 11)
#define NFSTAT_AUTOBOOT		(1 << 15)

// by sprife
int stop;

#define NF_CMD(cmd)				{rNFCMD=cmd;}
#define NF_ADDR(addr)			{rNFADDR=addr;}	
#define NF_nFCE_L()				do{rNFCONF&=~NFCONF_nFCE_HIGH;stop = 0;while(stop < 100) stop++;}while(0)
#define NF_nFCE_H()				do{rNFCONF|=NFCONF_nFCE_HIGH;stop = 0;while(stop < 100) stop++;}while(0)

/*
#define NF_CMD(cmd)				{rNFCMD=cmd;}
#define NF_ADDR(addr)			{rNFADDR=addr;}	
#define NF_nFCE_L()				{rNFCONF&=~NFCONF_nFCE_HIGH;}
#define NF_nFCE_H()				{rNFCONF|=NFCONF_nFCE_HIGH;}
*/
#define NF_RSTECC()				{rNFCONF|=NFCONF_ECC_INIT;}
#define NF_RDDATA()				(rNFDATA)
#define NF_WRDATA(data)			{rNFDATA=data;}

#define NF_WAITRB()				while(!(rNFSTAT&NFSTAT_RnB));

flashinfo nand_flashinfo;

typedef struct _NandInfo
{
	unsigned char NandID;
	int totalBlock;		//整个flash的块数
	int PagePerBlock;	//每个块的page数
} NandInfo;

typedef struct _NandMaunfacture
{
	unsigned char ManuID;
	unsigned char ManuStr[10];
} NandManufacture;

#define K9F2808_ID (0xec73)

static NandInfo NandType[]= {
	{ 0xe3,  256, 16},
	{ 0xe5,  512, 16},
	{ 0xe6, 1024, 16},
	{ 0x73, 1024, 32},
	{ 0x75, 2048, 32},
	{ 0x76, 4096, 32},
	{ 0x79, 8192, 32},};
	
static NandManufacture NandManuId[] = {
	{ 0xec, "SamSung"},
	{ 0x98, "Toshiba"}};

//define nand flash command
#define NAND_SEQDATAINPUT	0x80
#define NAND_READ0			0x00
#define NAND_READ1			0x01
#define NAND_READ2			0x50
#define NAND_RESET			0xff
#define NAND_PAGEPROG		0x10
#define NAND_ERASE0		0x60
#define NAND_ERASE1		0xd0
#define NAND_STATUS		0x70
#define NAND_IDREAD		0x90

#define MANUID(id)		(((id)&0xff00)>>8)
#define TYPEID(id)		((id)&0xff)

#if(RAM_DISK==1)
	#define RAM_DISK_SIZE		(4*1024*1024)
	#define Page_Size			(528)
	#define Cluster_Size			(32*Page_Size)
	unsigned char *RamDisk=(unsigned char *)(0xC080000+1*1024*1024);
#endif

static int Flash_Reset(void);//flash reset
static unsigned int Get_Flash_Id(void);

int NandFlash_init(void)
{
	U32 flashid;
	int i;

	// set NAND Flash controller
	rNFCONF = NFCONF_FCTRL_EN | NFCONF_nFCE_HIGH | NFCONF_TWRPH0_3 |NFCONF_ECC_INIT;

	if(Flash_Reset()!=OK){
		printk("Failed to reset Nand Flash!\n");
		for(;;);
		return FAIL;
	}

	if((flashid=Get_Flash_Id())==FAIL){
		printk("Failed to get Nand Flash ID!\n");
		return FAIL;
	}

	printk("Nand Flash ID is 0x%x, ", flashid);
	for(i=0;i<NumberOfArray(NandManuId);i++){
		if(NandManuId[i].ManuID==MANUID(flashid)){
			printk("Manufacture is %s. ", NandManuId[i].ManuStr);
			break;
		}
	}

	for(i=0;i<NumberOfArray(NandType);i++){
		if(NandType[i].NandID==TYPEID(flashid)){
			nand_flashinfo.PagePerBlock=NandType[i].PagePerBlock;
			nand_flashinfo.totalBlock=NandType[i].totalBlock;
			nand_flashinfo.totalsize=nand_flashinfo.totalBlock*
				nand_flashinfo.PagePerBlock*NandPageSize;
			nand_flashinfo.blocksize = NandType[i].PagePerBlock*NandPageSize;
			printk("Size is %dMByte\n", nand_flashinfo.totalsize/1024/1024);
			break;
		}
	}

//	if(totalsize>32)	//nand flash larger than 32MByte
//		rSmcConf|=BigCardEn;
	return OK;
}

static unsigned int Get_Flash_Id(void)
{
#if(RAM_DISK==1)	//for ramdisk
	return K9F2808_ID;
#else	//for flash
	int i;
	U16 id;

	NF_nFCE_L();

	NF_CMD(NAND_IDREAD);
	NF_ADDR(0x0);

	for(i=0;i<10;i++); //wait tWB(100ns)////?????

	id=NF_RDDATA()<<8;	// Maker code(K9S1208V:0xec)
	id|=NF_RDDATA();	// Devide code(K9S1208V:0x76)

	NF_nFCE_H();

	return id;

#endif
}

static int Flash_Reset()		//flash reset
{
#if(RAM_DISK==0)	//for nand flash
	int i;
    
	NF_nFCE_L();
	NF_CMD(NAND_RESET);	//reset command

	for(i=0;i<10;i++);  //tWB = 100ns. //??????
	NF_WAITRB();      //wait 200~500us;

	NF_nFCE_H();

#endif

	return OK;
}

static U8 seBuf[16]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

//检测是否是坏块，是返回1
static int NF_IsBadBlock(U32 block)
{
	int i;
	unsigned int blockPage;
	U8 data;

	blockPage=(block<<5);	// For 2'nd cycle I/O[7:5] 

	NF_nFCE_L();    
	NF_CMD(NAND_READ2);	// Spare array read command
	NF_ADDR(517&0xf);		// Read the mark of bad block in spare array(M addr=5) 
	NF_ADDR(blockPage&0xff);	// The mark of bad block is in 0 page
	NF_ADDR((blockPage>>8)&0xff);   // For block number A[24:17]
	NF_ADDR((blockPage>>16)&0xff);  // For block number A[25]

	for(i=0;i<10;i++);	// wait tWB(100ns) //?????

	NF_WAITRB();	// Wait tR(max 12us)

	data=NF_RDDATA();

	NF_nFCE_H();    

	if(data!=0xff)
	{
		printk("[block %d has been marked as a bad block(%x)]\n",block,data);
		return 1;
	}

	return 0;
}


static int NF_MarkBadBlock(U32 block)
{
	int i;
	U32 blockPage=(block<<5);

	seBuf[0]=0xff;
	seBuf[1]=0xff;    
	seBuf[2]=0xff;    
	seBuf[5]=0x44;   // Bad blcok mark=0

	NF_nFCE_L(); 
	NF_CMD(NAND_READ2);		//????
	NF_CMD(NAND_SEQDATAINPUT);	// Write 1st command

	NF_ADDR(0x0);		// The mark of bad block is 
	NF_ADDR(blockPage&0xff);	    // marked 5th spare array 
	NF_ADDR((blockPage>>8)&0xff);   // in the 1st page.
	NF_ADDR((blockPage>>16)&0xff);  //

	for(i=0;i<16;i++){
		NF_WRDATA(seBuf[i]);	// Write spare array
	}

	NF_CMD(NAND_PAGEPROG);   // Write 2nd command

	for(i=0;i<10;i++);  //tWB = 100ns. ///???????

	NF_WAITRB();      // Wait tPROG(200~500us)

	NF_CMD(NAND_STATUS);

	for(i=0;i<3;i++);  //twhr=60ns////??????

	if (NF_RDDATA()&0x1){ // Spare arrray write error
		NF_nFCE_H();
		printf("[Program error is occurred but ignored]\n");
	}
	else{
		NF_nFCE_H();
	}

	printf("[block #%d is marked as a bad block]\n",block);
	return OK;
}


/*****************************************************************\
	功能：擦除FLASH的1Block
	参数	block:擦除的 块位置
	返回值	成功返回OK，否则返回FAIL
\*****************************************************************/
int Erase_Block(unsigned int block)
{

#if(RAM_DISK==1)	//for ramdisk
	memset(RamDisk+block*Cluster_Size,0xff,Cluster_Size);
#else	//for flash

    U32 blockPage=(block<<5);
    int i;

#if BAD_CHECK
    if(NF_IsBadBlock(block))
	return FAIL;
#endif

	NF_nFCE_L();
	
	NF_CMD(NAND_ERASE0);   // Erase one block 1st command

	NF_ADDR(blockPage&0xff);	    // Page number=0
	NF_ADDR((blockPage>>8)&0xff);   
	NF_ADDR((blockPage>>16)&0xff);

	NF_CMD(NAND_ERASE1);   // Erase one blcok 2nd command
	 
	for(i=0;i<10;i++); //wait tWB(100ns)//??????

	NF_WAITRB();    // Wait tBERS max 3ms.
	NF_CMD(0x70);   // Read status command

	if (NF_RDDATA()&0x1){ // Erase error
		NF_nFCE_H();
		printf("[ERASE_ERROR:block#=%d]\n",block);
		NF_MarkBadBlock(block);
		return FAIL;
	}

	NF_nFCE_H();
#endif

	return OK;
}

//检测一个页是否擦除，擦除则返回0，否则返回1
int CheckPageEreased(unsigned int block,unsigned int page)
{
	int ret=0;
	int i;
	unsigned int blockPage;

	page=page&0x1f;
	blockPage=(block<<5)+page;

	NF_nFCE_L();
	NF_CMD(NAND_READ0);   // Read command
	NF_ADDR(0);	    // Column = 0
	NF_ADDR(blockPage&0xff);	    //
	NF_ADDR((blockPage>>8)&0xff);   // Block & Page num.
	NF_ADDR((blockPage>>16)&0xff);  //

	for(i=0;i<10;i++); //wait tWB(100ns)/////??????

	NF_WAITRB();    // Wait tR(max 12us)
	for(i=0;i<NandPageSize;i++){	// Read one page
		if(0xff!=NF_RDDATA()){
			ret=1;
			break;
		}
	}

	NF_nFCE_H();    

	return ret;
}

/*****************************************************************\
	功能：读取FLASH的某个Block中的1 space page数据512byte
	参数	block: 读取的块位置
			page: 读取的page位置
			pPage: 返回的数据
	返回值	成功返回OK，否则返回FAIL
	注意: pPage指向的缓冲区的大小必须大于等于512 byte
\*****************************************************************/
int ReadPage512(unsigned int block,unsigned int page, unsigned char* pPage)
{
	int i;
	unsigned int blockPage;
//	U8 ecc0,ecc1,ecc2;
	U8 se[16];

	page=page&0x1f;
	blockPage=(block<<5)+page;
	NF_RSTECC();    // Initialize ECC

	NF_nFCE_L();
	NF_CMD(NAND_READ0);   // Read command
	NF_ADDR(0);	    // Column = 0
	NF_ADDR(blockPage&0xff);	    //
	NF_ADDR((blockPage>>8)&0xff);   // Block & Page num.
	NF_ADDR((blockPage>>16)&0xff);  //

	for(i=0;i<10;i++); //wait tWB(100ns)/////??????

	NF_WAITRB();    // Wait tR(max 12us)
	for(i=0;i<NandPageSize;i++){
		*pPage++=NF_RDDATA();	// Read one page
	}

/*	ecc0=rNFECC0;
	ecc1=rNFECC1;
	ecc2=rNFECC2;*/
	for(i=0;i<16;i++){
		se[i]=NF_RDDATA();	// Read spare array
	}
	NF_nFCE_H();    
/*
	if(ecc0==se[0] && ecc1==se[1] && ecc2==se[2])
	{
		//printf("[ECC OK:%x,%x,%x]\n",se[0],se[1],se[2]);
		return OK;
	}

	printf("[ECC ERROR(RD):read:%x,%x,%x, reg:%x,%x,%x]\n",se[0],se[1],se[2],ecc0,ecc1,ecc2);*/
	return OK;
}

/*****************************************************************\
	功能：读取FLASH的某个Block中的1 space page数据16byte
	参数	block: 读取的块位置
			page: 读取的page位置
			pPage: 返回的数据
	返回值	成功返回OK，否则返回FAIL
	注意: pPage指向的缓冲区的大小必须大于等于16 byte
\*****************************************************************/
int ReadSparePage(unsigned int block,unsigned int page, unsigned char* pPage)
{
	int i;
	unsigned int blockPage;

	blockPage=(block<<5)+(page&0x1f);	// For 2'nd cycle I/O[7:5] 

	NF_nFCE_L();    
	NF_CMD(NAND_READ2);	// Spare array read command
	NF_ADDR(0);	
	NF_ADDR(blockPage&0xff);	// The mark of bad block is in 0 page
	NF_ADDR((blockPage>>8)&0xff);   // For block number A[24:17]
	NF_ADDR((blockPage>>16)&0xff);  // For block number A[25]

	for(i=0;i<10;i++);	// wait tWB(100ns) //?????

	NF_WAITRB();	// Wait tR(max 12us)

	for(i=0;i<16;i++)
		*pPage++=NF_RDDATA();

	NF_nFCE_H();    

	return OK;
}

/*****************************************************************\
	功能：写入FLASH的某个Block中的1 page数据, 大小512 byte
	参数	block: 写入的块位置
			page: 写入的page位置
			pPage: 返回的数据
	返回值	成功返回OK，否则返回FAIL
	注意: pPage指向的缓冲区的大小必须大于等于512 byte
\*****************************************************************/
int WritePage512(unsigned int block,unsigned int page, const unsigned char *pPage)
{
	int i;
	U32 blockPage=(block<<5)+page;

	NF_RSTECC();    // Initialize ECC

	NF_nFCE_L(); 
	NF_CMD(NAND_READ0);//??????
	NF_CMD(NAND_SEQDATAINPUT);   // Write 1st command
	NF_ADDR(0);			    // Column 0
	NF_ADDR(blockPage&0xff);	    //
	NF_ADDR((blockPage>>8)&0xff);   // Block & page num.
	NF_ADDR((blockPage>>16)&0xff);  //

	for(i=0;i<512;i++){
		NF_WRDATA(*pPage++);	// Write one page to NFM from buffer
	}  

/*	seBuf[0]=rNFECC0;
	seBuf[1]=rNFECC1;
	seBuf[2]=rNFECC2;
	seBuf[5]=0xff;		// Marking good block

	for(i=0;i<16;i++){
		NF_WRDATA(seBuf[i]);	// Write spare array(ECC and Mark)
	}  */

	NF_CMD(NAND_PAGEPROG);   // Write 2nd command

	for(i=0;i<10;i++);  //tWB = 100ns. ////??????

	NF_WAITRB();    //wait tPROG 200~500us;

	NF_CMD(NAND_STATUS);   // Read status command   

	for(i=0;i<3;i++);  //twhr=60ns

	if (NF_RDDATA()&0x1){ // Page write error
		NF_nFCE_H();
		printf("[PROGRAM_ERROR:block#=%d]\n",block);
		NF_MarkBadBlock(block);
		return FAIL;
	}

	NF_nFCE_H();
	return OK;
}

/*****************************************************************\
	功能：写入FLASH的某个Block中的1 spare page数据, 大小16 byte
	参数	block: 写入的块位置
			page: 写入的page位置
			pPage: 返回的数据
	返回值	成功返回OK，否则返回FAIL
	注意: pPage指向的缓冲区的大小必须大于等于16 byte
\*****************************************************************/
int WriteSparePage(unsigned int block,unsigned int page, const unsigned char *pPage)
{
	int i;
	U32 blockPage=(block<<5)+(page&0x1f);

	NF_nFCE_L();
	NF_CMD(NAND_READ2);		//????
	NF_CMD(NAND_SEQDATAINPUT);	// Write 1st command

	NF_ADDR(0x0);
	NF_ADDR(blockPage&0xff);	    // marked 5th spare array 
	NF_ADDR((blockPage>>8)&0xff);   // in the 1st page.
	NF_ADDR((blockPage>>16)&0xff);  //

	for(i=0;i<16;i++){
		NF_WRDATA(pPage[i]);	// Write spare array
	}

	NF_CMD(NAND_PAGEPROG);   // Write 2nd command

	for(i=0;i<10;i++);  //tWB = 100ns. ///???????

	NF_WAITRB();      // Wait tPROG(200~500us)

	NF_CMD(NAND_STATUS);

	for(i=0;i<3;i++);  //twhr=60ns////??????

	if (NF_RDDATA()&0x1){ // Spare arrray write error
		NF_nFCE_H();
		printf("[Program error is occurred but ignored]\n");
		return FAIL;
	}

	NF_nFCE_H();
	return FAIL;
}

/*******************************************************************\
 	功能：Nand Flash是否有效
 	返回值: TRUE or FALSE
\********************************************************************/
BOOL NandFlashOK(void)
{
	return ((nand_flashinfo.PagePerBlock==0)?FALSE:TRUE);
}

/*******************************************************************\
 	功能：显示FLASH的数据
\********************************************************************/
int Flash_Tools(int argc, char *argv[])
{
	static unsigned char bbb[NandPageSize];
	unsigned int i,block,page;

/*	for(i=0;i<8;i++)
		Erase_Block(i);

	for(i=0;i<256;i++){
		memset(bbb, i, sizeof(bbb));
		WritePage(i/32, i%32, bbb);
	}*/

	if(NandFlashOK()==FALSE)
	{
			printf("*               No Nand flash Found.Please check your hardware!         *\n");
			printf("*-----------------------------------------------------------------------*\n");
			return FAIL;
	}

	i = 0;
	block = 0;
	page = 0;

	while(1){
		char aa;
		printf("*                     C(c)>>Point to Configure zone(Block0)             *\n");
		printf("*                     A(a)>>Point to Application zone(Block1~)          *\n");
		printf("*                     B(b)>>Point to Next Block                         *\n");
		printf("*                     P(p)>>Point to Next Page                          *\n");
		printf("*                     Q(q)>>Quit                                          *\n");
		printf("*-----------------------------------------------------------------------*\n");

		aa = getchar();

		if (aa == 'q' || aa=='Q') break;
		switch(aa)
		{
		case 'c':
		case 'C':
			block = 0;
			page = 0;
			printf("*---------------------Configure Zone at Block %4d, Page %2d----------------*\n",block,page);
			break;
		case 'a':
		case 'A':
			block = 1;
			page = 0;
			printf("*---------------------Application Zone at Block %4d, Page %2d---------*\n",block,page);
			break;
	
		case 'b':
		case 'B':
			block++;
			if (block>=nand_flashinfo.totalBlock) 
				block = 0;
			page = 0;
			printf("*---------------------Block %4d, Page %2d---------------------------*\n",block,page);
			break;
		case 'p':
		case 'P':
		default:
			page++;
			if (page>=nand_flashinfo.PagePerBlock)
			{
				page = 0;
				block ++;
				if (block>=nand_flashinfo.totalBlock)	block = 0;
			}
			printf("*---------------------Block %4d, Page %2d---------------------------*\n",block,page);
		}

		if(ReadPage(block,page,bbb)==FAIL){
			printf("Failed to read page at block %4d, Page %4d", block,page);
			continue;
		}

		for(i=0;i<NandPageSize;i=i+16)
		{
			printf("BYTE%4x:%4x%4x%4x%4x%4x%4x%4x%4x%4x%4x%4x%4x%4x%4x%4x%4x\n",\
			i,bbb[i],bbb[i+1],bbb[i+2],bbb[i+3],bbb[i+4],bbb[i+5],bbb[i+6],bbb[i+7],\
			bbb[i+8],bbb[i+9],bbb[i+10],bbb[i+11],bbb[i+12],bbb[i+13],bbb[i+14],bbb[i+15]);
		}
	}

	return OK;
}
