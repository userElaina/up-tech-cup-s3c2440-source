/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.5.12
\***************************************************************************/
/***************************************************************************\
    #说明: mmu管理 函数
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2004-5-17	创建

\***************************************************************************/

#include "../inc/sys/lib.h"
#include "../inc/sys/mmu.h"

extern int L0PageTable;

void MMU_SetMTT(int vaddrStart,int vaddrEnd,int paddrStart,int attr)
{
    U32 *pTT;
    int i,nSec;
    pTT=(U32 *)(&L0PageTable+(vaddrStart>>20));
    nSec=(vaddrEnd>>20)-(vaddrStart>>20);
    for(i=0;i<=nSec;i++)*pTT++=attr |(((paddrStart>>20)+i)<<20);
}


int MMU_Init(void)
{
    int i,j;
    //========================== IMPORTANT NOTE =========================
    //The current stack and code area can't be re-mapped in this routine.
    //If you want memory map mapped freely, your own sophiscated MMU
    //initialization code is needed.
    //===================================================================

    MMU_DisableDCache();
    MMU_DisableICache();

    //If write-back is used,the DCache should be cleared.
    for(i=0;i<64;i++)
    	for(j=0;j<8;j++)
    	    MMU_CleanInvalidateDCacheIndex((i<<26)|(j<<5));
    MMU_InvalidateICache();
    
    MMU_DisableMMU();
    MMU_InvalidateTLB();

    //MMU_SetMTT(int vaddrStart,int vaddrEnd,int paddrStart,int attr)
    MMU_SetMTT(0x00000000,0x07f00000,0x00000000,RW_CNB);  //bank0
    MMU_SetMTT(0x08000000,0x0ff00000,0x08000000,RW_NCNB);  //bank1
//    MMU_SetMTT(0x08000000,0x0ff00000,0x08000000,RW_CNB);  //bank1
    MMU_SetMTT(0x10000000,0x17f00000,0x10000000,RW_NCNB); //bank2
    MMU_SetMTT(0x18000000,0x1ff00000,0x18000000,RW_NCNB); //bank3
    MMU_SetMTT(0x20000000,0x27f00000,0x20000000,RW_NCNB); //bank4
    MMU_SetMTT(0x28000000,0x2ff00000,0x28000000,RW_NCNB); //bank5

#if 0
    MMU_SetMTT(0x30000000,0x30f00000,0x30000000,RW_NCNB);	  //bank6-1
//    MMU_SetMTT(0x30000000,0x30f00000,0x30000000,RW_CB);	  //bank6-1
    MMU_SetMTT(0x31000000,0x33e00000,0x31000000,RW_NCNB); //bank6-2
    MMU_SetMTT(0x33f00000,0x33f00000,0x33f00000,RW_CB);   //bank6-3
#endif
    MMU_SetMTT(0x30000000,0x33f00000,0x30000000,RW_CB);	  //bank6

    MMU_SetMTT(0x38000000,0x3ff00000,0x38000000,RW_NCNB); //bank7
    
    MMU_SetMTT(0x40000000,0x5af00000,0x40000000,RW_NCNB);//SFR+StepSram    
    MMU_SetMTT(0x5b000000,0xfff00000,0x5b000000,RW_FAULT);//not used

    MMU_SetTTBase((int)&L0PageTable);
    MMU_SetDomain(0x55555550|DOMAIN1_ATTR|DOMAIN0_ATTR); 
    	//DOMAIN1: no_access, DOMAIN0,2~15=client(AP is checked)
    MMU_SetProcessId(0x0);
    MMU_EnableAlignFault();
    	
    MMU_EnableMMU();
    MMU_EnableICache();
    MMU_EnableDCache(); //DCache should be turned on after MMU is turned on.

	return 0;
}

#if 0
int virt2phys(int vaddr)
{
	int *pagetable = (int *)&L0PageTable;
	int base = pagetable[(vaddr>>20)&0xfff];
	return (base & 0xfff00000) | (vaddr & ~0xfff00000);
}

/**************************************************************\
	通过MMU重映象函数
	参数: 
			vaddr,	映象的虚拟地址
			phyaddr,	物理地址
			size,		映象大小，单位(1kbyte)
			flags,	映象的MMU标志
\**************************************************************/
void remap_pages(U32 vaddr, U32 phyaddr, U32 size, U32 flags)
{
	int i;
	int *pagetable = (int *)&L0PageTable;

	vaddr &= ~0x3fff;
	phyaddr &= ~0x3fff;
	flags &= 0x3fff;	//bit[13:0]
	size >>=10;		// 1Mbyte
	for(i=0; i<size; i++) {
		pagetable[vaddr>>20] = phyaddr | flags;

		__asm{
			mcr p15, 0, vaddr, c8, c6, 1		//Invalidate TLB single entry
		}

		vaddr+=0x100000;
		phyaddr+=0x100000;
	}
}
#endif

