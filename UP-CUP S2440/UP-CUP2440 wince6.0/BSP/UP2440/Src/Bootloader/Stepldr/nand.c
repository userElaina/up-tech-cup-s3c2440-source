#include <windows.h>
#include "s2440addr.h"
#include "option.h"

#define NF_CMD(cmd)		{rNFCMD=cmd;}
#define NF_ADDR(addr)		{rNFADDR=addr;}	

//  For flash chip that is bigger than 32 MB, we need to have 4 step address
#define NEED_EXT_ADDR		1

#define NF_nFCE_L()		{rNFCONT &= ~(1 << 1);}
#define NF_nFCE_H()		{rNFCONT |=  (1 << 1);}
#define NF_RSTECC()		{rNFCONT |=  (1 << 4);}

#define NF_MECC_UnLock()	{rNFCONT &= ~(1<<5);}
#define NF_MECC_Lock()		{rNFCONT |= (1<<5);}

#define NF_CLEAR_RB()		{rNFSTAT |=  (1 << 2);}
#define NF_DETECT_RB()		{while(!(rNFSTAT&(1<<2)));}
#define NF_WAITRB()     	{while (!(rNFSTAT & (1 << 2)));} 


#define NF_RDDATA()		(rNFDATA)
#define NF_RDDATA32()		(rNFDATA32)
#define NF_WRDATA(data)		{rNFDATA=data;}

#define NF_RDMECC0()		(rNFMECC0)
#define NF_RDMECC1()		(rNFMECC1)

#define NF_RDMECCD0()		(rNFMECCD0)
#define NF_RDMECCD1()		(rNFMECCD1)

#define NF_WRMECCD0(data)	{rNFMECCD0 = (data);}
#define NF_WRMECCD1(data)	{rNFMECCD1 = (data);}


#define ID_K9S1208U0C		0xec76
#define ID_K9S1G08U0A		0xecF1
#define ID_K9S2G08U0A		0xecDA
#define ID_K9S4G08U0A		0xecDC
#define ID_K9S8G08U0M		0xecD3

// HCLK=133Mhz
#define TACLS			0 
#define TWRPH0			4 // 5
#define TWRPH1			2 // 3

BOOL bLARGEBLOCK;

void __RdPagedata(BYTE *bufPt); 

UINT8 NF_ReadID()
{
	UINT8 pMID;
	UINT8 pDID;
	UINT32  nCnt;
	UINT32  nBAddr, nRet;
	UINT16  nBuff;
	UINT8   n4thcycle;
	int i;

	bLARGEBLOCK = FALSE;
	n4thcycle = nBuff = 0;

	NF_nFCE_L();    
	NF_CLEAR_RB();
	NF_CMD  (0x90);	// read id command
	NF_ADDR (0x00);
	for ( i = 0; i < 100; i++ );

	/* tREA is necessary to Get a MID. */
	for (nCnt = 0; nCnt < 5; nCnt++)
	{
		pMID = (BYTE) NF_RDDATA();
		if (0xEC == pMID)
			break;
	}

	pDID = (BYTE) NF_RDDATA();

//	Uart_SendDWORD(pMID,1);
//	Uart_SendDWORD(pDID,1);
    
	nBuff     = (BYTE) NF_RDDATA();
	n4thcycle = (BYTE) NF_RDDATA();
	NF_nFCE_H();

	if (pMID != (UINT8)0xEC)
		nRet = FALSE;
	else
		nRet = TRUE;

	if (pDID >= 0xA0)
	{
		bLARGEBLOCK = TRUE;
//		Uart_SendString("LARGBLOCK ! \r\n");
	}

	return (pDID);
}


int NF_ReadPage(UINT32 block,UINT32 page,UINT8 *buffer)
{
//    volatile int i;
	register UINT8 * bufPt=buffer;
	unsigned int blockPage;
	ULONG MECC;
	int NewDataAddr;

	if ( bLARGEBLOCK == TRUE )
	{
		blockPage = (block<<6) + page/4;
		NewDataAddr = 512*(page%4);
		NF_RSTECC();    // Initialize ECC
		NF_MECC_UnLock();
    
		NF_nFCE_L();    
		NF_CMD(0x00);   // Read command

		NF_ADDR((NewDataAddr)&0xff);
		NF_ADDR(((NewDataAddr)>>8)&0xff);
		NF_ADDR(blockPage&0xff);	    //
		NF_ADDR((blockPage>>8)&0xff);   // Block & Page num.
		if (LB_NEED_EXT_ADDR)
		{
			NF_ADDR((blockPage>>16)&0xff);   // Block & Page num.
		}
	}
	else
	{
		blockPage = (block<<5) + page;      // k9s1208 1block = 32page = 2^5 pages
		NewDataAddr=0;
		NF_RSTECC();    // Initialize ECC
		NF_MECC_UnLock();
    
		NF_nFCE_L();    
		NF_CMD(0x00);   // Read command

		NF_ADDR((NewDataAddr)&0xff);
		NF_ADDR(blockPage&0xff);	    //
		NF_ADDR((blockPage>>8)&0xff);   // Block & Page num.
		if (SB_NEED_EXT_ADDR)
		{
			NF_ADDR((blockPage>>16)&0xff);   // Block & Page num.
		}
	}

	NF_CLEAR_RB();
	NF_CMD(0x30);
	NF_DETECT_RB();

	__RdPagedata(bufPt);
	NF_MECC_Lock();

	// no ecc check
	return 1;

	MECC = NF_RDDATA32();
	MECC = NF_RDDATA32();
	    
	MECC = NF_RDDATA32();
	NF_WRMECCD0( ((MECC&0xff00    )<<8) |  (MECC&0xff    )      );
	NF_WRMECCD1( ((MECC&0xff000000)>>8) | ((MECC&0xff0000)>>16) );
	NF_nFCE_H();    

	if(rNFESTAT0 & 0x3)
	{
		Uart_SendString("ECC ERROR block");
		Uart_SendDWORD(block,0);
		Uart_SendString("page");
		Uart_SendDWORD(page,1);
		return 0;
	}
	else
		return 1;

}


void NF_Reset(void)
{
	volatile int i;

	NF_nFCE_L();

	NF_CMD(0xFF);              // reset command.

	for(i=0 ; i<10 ; i++);     // tWB = 100ns. 

	NF_WAITRB();               // wait 200~500us.

	NF_nFCE_H();
}


void NF_Init(void)
{

	rNFCONF = (TACLS  <<  12) | /* CLE & ALE = HCLK * (TACLS  + 1)   */
                          (TWRPH0 <<  8) | /* TWRPH0    = HCLK * (TWRPH0 + 1)   */
                          (TWRPH1 <<  4) |
			  (0xE<<0);  // 0x0000842E

	rNFCONT = (0<<13)|(0<<12)|(0<<10)|(0<<9)|(0<<8)|(0<<6)|(0<<5)|(1<<4)|(1<<1)|(1<<0);

	rNFSTAT = 0x4;

	NF_Reset();
}
