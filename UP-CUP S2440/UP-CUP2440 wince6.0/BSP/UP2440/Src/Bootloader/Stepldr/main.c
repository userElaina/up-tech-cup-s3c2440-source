#include <windows.h>
#include <pehdr.h>
#include <romldr.h>
#include "option.h"
#include "s2440addr.h"

// Constants.
//
#define LED_ON	                0xa
#define LED_OFF	                0x0

#define NAND_BLOCK_SIZE_BYTES   0x00020000//0x4000//0x00020000
#define NAND_PAGE_SIZE_BYTES    0x00000200
#define NAND_PAGES_PER_BLOCK    (NAND_BLOCK_SIZE_BYTES / NAND_PAGE_SIZE_BYTES)

    // NOTE: we assume that this Steppingstone loader occupies *part* the first (good) NAND flash block.  More
    // specifically, the loader takes up 4096 bytes (or 8 NAND pages) of the first block.  We'll start our image
    // copy on the very next page.
#define NAND_COPY_PAGE_OFFSET   4*NAND_PAGES_PER_BLOCK		//eboot的位置在块4，

#define LOAD_ADDRESS_PHYSICAL   0x30038000
#define LOAD_SIZE_BYTES         0x00040000		//这个地方应该是0x00080000
#define LOAD_SIZE_PAGES         (LOAD_SIZE_BYTES / NAND_PAGE_SIZE_BYTES)

//---------------------------------------------
#define RUNADDR  0x30038000

// Globals variables.
//
ROMHDR * volatile const pTOC = (ROMHDR *)-1;

// Function prototypes.
//
void MMU_EnableICache(void);
//void Led_Display(int);
void Port_Init(void);
void ChangeClockDivider(int hdivn, int pdivn);
void ChangeMPllValue(int mdiv, int pdiv, int sdiv);
void NF_Init(void);
int NF_ReadPage(UINT32 block, UINT32 page, volatile BYTE *buffer);

// 
typedef void (*PFN_IMAGE_LAUNCH)();



/*
    @func   BOOLEAN | SetupCopySection | Copies the IPL image's copy section data (initialized globals) to the correct fix-up location.  Once completed, the IPLs initialized globals are valid.
    @rdesc  TRUE == Success and FALSE == Failure.
    @comm
    @xref
*/
static BOOLEAN SetupCopySection(ROMHDR *const pTOC)
{
    // This code doesn't make use of global variables so there are no copy sections.  To reduce code size, this is a stub function...
    //
    return(TRUE);
}


/*
    @func   void | main | C entrypoint function for the Steppingstone loader.
    @rdesc  None.
    @comm    
    @xref   
*/
void main(void)
{
	register nBlock;
	register nPage;
	register nBadBlocks;
	volatile BYTE *pCopyPtr;
	int UartRec = 0;
	unsigned int i;
	unsigned int FileLend = 0xf5;
	unsigned int bSDUpload = 0;
	UINT8 NAND_ID ;
	int rNAND_BLOCK_SIZE_BYTES;
	int rNAND_PAGE_SIZE_BYTES;
	int rNAND_PAGES_PER_BLOCK;
	int rNAND_COPY_PAGE_OFFSET;

	// Set up copy section (initialized globals).
	//
	// NOTE: after this call, globals become valid.
	//
	SetupCopySection(pTOC);

	// Enable the ICache.
	//
	MMU_EnableICache();

	// Set up clock and PLL.
	//

#if 0
	ChangeClockDivider(3, 1);	      // 1:3:6.
//	ChangeMPllValue(0x6e, 0x3, 0x1);  // Fin=16MHz FCLK=399.65MHz.
	ChangeMPllValue(0x3e, 0x2, 0x1);  // Fin=16MHz FCLK=296.352MHz.
//	ChangeMPllValue(229, 0x5, 0x0);  // Fin=12Mhz
#endif

	// Set up all GPIO ports.
	//
	Port_Init();

	Uart_Init();
//	Uart_SendString("\n\n##### EmbedSky Boot #####\r\n");

NANDSTART:
	// Initialize the NAND flash interface.
	//
	NF_Init();

	NF_Reset();

	// Turn the LEDs off.
	//
	//Led_Display(LED_OFF);

	// Copy image from NAND flash to RAM.
	//
	pCopyPtr = (BYTE *)LOAD_ADDRESS_PHYSICAL;

	// NOTE: we assume that this Steppingstone loader occupies *part* the first (good) NAND flash block.  More
	// specifically, the loader takes up 4096 bytes (or 8 NAND pages) of the first block.  We'll start our image
	// copy on the very next page.

	NAND_ID = (UINT8)NF_ReadID();
	if (NAND_ID == FALSE)
	{
		Uart_SendString("ID ERR");
		while(1)
		{
			;
		}
	}
	switch(NAND_ID)
	{
		case 0x76:
			rNAND_BLOCK_SIZE_BYTES=0x00004000;
			rNAND_PAGE_SIZE_BYTES=0x00000200;
			rNAND_PAGES_PER_BLOCK=(rNAND_BLOCK_SIZE_BYTES / rNAND_PAGE_SIZE_BYTES);
			rNAND_COPY_PAGE_OFFSET =32*rNAND_PAGES_PER_BLOCK;
			break;
		case 0xf1:
		case 0xda:
		case 0xdc:
		case 0xd3:
			rNAND_BLOCK_SIZE_BYTES=0x00020000;
			rNAND_PAGE_SIZE_BYTES=0x00000200;
			rNAND_PAGES_PER_BLOCK=(rNAND_BLOCK_SIZE_BYTES / rNAND_PAGE_SIZE_BYTES);
			rNAND_COPY_PAGE_OFFSET =4*rNAND_PAGES_PER_BLOCK;
			break;
	}
	nBadBlocks = 0;
	for (nPage = rNAND_COPY_PAGE_OFFSET ; nPage < (LOAD_SIZE_PAGES + rNAND_COPY_PAGE_OFFSET) ; nPage++)
	{
		nBlock = ((nPage / rNAND_PAGES_PER_BLOCK) + nBadBlocks);

		if (!NF_ReadPage(nBlock, (nPage % rNAND_PAGES_PER_BLOCK), pCopyPtr))
		{
			if ((nPage % rNAND_PAGES_PER_BLOCK) != 0)
			{
				//Led_Display(0x9);    // real ECC Error.

				// Spin forever...
				while(1)
				{
				}
			}

			// ECC error on a block boundary is (likely) a bad block - retry the page 0 read on the next block.
			nBadBlocks++;
			nPage--;
			continue;
		}

		pCopyPtr += rNAND_PAGE_SIZE_BYTES;
	}

	// Turn the LEDs on.
	//Led_Display(LED_ON);

	// Jump to the image...
	//
	((PFN_IMAGE_LAUNCH)(LOAD_ADDRESS_PHYSICAL))();

}

