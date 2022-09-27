/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 2002  Microsoft Corporation

Module Name:	FMD.CPP

Abstract:		FLASH Media Driver Interface Samsung K9F2808UOB NAND Flash Chip
                on AT-Rise development board.

Notes:			Currently, *only* the CE 3.0 (and earlier) power management logic
				is implemented (i.e. the PowerUp() and PowerDown() APIs).

Environment:	As noted, this media driver works on behalf of the FAL to directly
				access the underlying FLASH hardware.  Consquently, this module
				needs to be linked with FAL.LIB to produce the device driver
				named FLASHDRV.DLL.

-----------------------------------------------------------------------------*/
#include <fmd.h>
#include <s3c2440a.h>
#include "cfnand.h"
#include "args.h"
#include "image_cfg.h"

#define NAND_BASE 0xB0E00000
#define IOP_BASE  0xB1600000

#define BADBLOCKMARK                0x00

#define MAX_REGIONS 16
#if defined (MAGNETO)
static FlashRegion g_pRegionTable[MAX_REGIONS];
static FlashInfo g_flashInfo;
#endif
static DWORD g_dwNumRegions;

// TODO: Make sector size generic
static BYTE g_pFLSBuffer[SECTOR_SIZE];
#define DEFAULT_COMPACTION_BLOCKS 4

//  Registers
volatile PUSHORT pNFReg;
volatile PUSHORT pNFCONF;
volatile PUSHORT pNFCONT;
volatile PUSHORT pNFCMD;
volatile PUSHORT pNFADDR;
volatile PULONG  pNFDATA;
volatile PUSHORT pNFSTAT;
volatile PULONG  pNFMECCD0;
volatile PULONG  pNFMECCD1;
volatile PULONG  pNFESTAT0;
volatile PULONG  pNFESTAT1;
volatile PULONG  pNFMECC0;
volatile PULONG  pNFMECC1;
volatile PULONG  pNFECC;
volatile PULONG  pNFSBLK;
volatile PULONG  pNFEBLK;

#define NFDATA 0x4E000010

S3C2440A_IOPORT_REG *v_pIOPregs;

BSP_ARGS *pBSPArgs;

#if defined (MAGNETO)
static BOOL DefineLayout();
#endif
//void NANDTest();

//  External function
extern "C" {
BOOL ECC_CorrectData(LPBYTE pData, LPBYTE pExistingECC, LPBYTE pNewECC);

}

#define LOW	0
#define HIGH 1

void SetChipSelect(int mode, int status)
{
		if ( status == LOW )
			NF_CE_L();
		else // if ( status == HIGH )
			NF_CE_H();
}

//  Reset the chip
//
void NF_Reset()
{
    BOOL bLastMode = SetKMode(TRUE);

// Do not anything...

//	if ()
//	{
//		LB_NF_Reset(USE_NFCE);
//		SB_NF_Reset(USE_GPIO);
//	}
//	else
//	{
//		LB_NF_Reset(USE_GPIO);
//		SB_NF_Reset(USE_NFCE);
//	}

    SetKMode(bLastMode);
}

/*
 *  NAND_ReadSectorInfo
 *
 *  Read SectorInfo out of the spare area. The current implementation only handles
 *  one sector at a time.
 */
void NAND_ReadSectorInfo(SECTOR_ADDR sectorAddr, PSectorInfo pInfo)
{
	RETAILMSG(1, (TEXT("FMD::NAND_ReadSectorInfo ++ \r\n")));
	if ( sectorAddr < (unsigned)wPRIMARY_NAND_BLOCKS*PAGES_PER_BLOCK )
	{
		if ( astNandSpec[dwPrimaryNandDevice].nSctsPerPg == 4 )
			NAND_LB_ReadSectorInfo(sectorAddr, pInfo, USE_NFCE);
		else
			NAND_SB_ReadSectorInfo(sectorAddr, pInfo, USE_NFCE);
	}
	else
	{
		if ( astNandSpec[dwSecondaryNandDevice].nSctsPerPg == 4 )
			NAND_LB_ReadSectorInfo(sectorAddr-wPRIMARY_NAND_BLOCKS*PAGES_PER_BLOCK, pInfo, USE_GPIO);
		else
			NAND_SB_ReadSectorInfo(sectorAddr-wPRIMARY_NAND_BLOCKS*PAGES_PER_BLOCK, pInfo, USE_GPIO);
	}
	RETAILMSG(1, (TEXT("FMD::NAND_ReadSectorInfo -- \r\n")));
}

/*
 *  NAND_WriteSectorInfo
 *
 *  Write SectorInfo out to the spare area. The current implementation only handles
 *  one sector at a time.
 */
BOOL NAND_WriteSectorInfo(SECTOR_ADDR sectorAddr, PSectorInfo pInfo)
{
    BOOL    bRet = TRUE;

	if ( sectorAddr < (unsigned)wPRIMARY_NAND_BLOCKS*PAGES_PER_BLOCK )
	{
		if ( astNandSpec[dwPrimaryNandDevice].nSctsPerPg == 4 )
			bRet = NAND_LB_WriteSectorInfo(sectorAddr, pInfo, USE_NFCE);
		else
			bRet = NAND_SB_WriteSectorInfo(sectorAddr, pInfo, USE_NFCE);
	}
	else
	{
		if ( astNandSpec[dwSecondaryNandDevice].nSctsPerPg == 4 )
			bRet = NAND_LB_WriteSectorInfo(sectorAddr-wPRIMARY_NAND_BLOCKS*PAGES_PER_BLOCK, pInfo, USE_GPIO);
		else
			bRet = NAND_SB_WriteSectorInfo(sectorAddr-wPRIMARY_NAND_BLOCKS*PAGES_PER_BLOCK, pInfo, USE_GPIO);
	}
    return bRet;
}

/*-----------------------------------------------------------------------------
 *  FMD Interface functions
 *
 *----------------------------------------------------------------------------*/
//  FMD_Init
//
//  Initialize the flash chip
//
//  Note: Presently, the Flash size characteristics are hardcoded in CFNAND.H
//		  and are NOT stored in the registry.  Refer to the StratFlash FMD in
//		  %WINCEROOT%\PUBLIC\COMMON\OAK\DRIVERS\BLOCK\... for an example of how
//		  to use the registry for storing this information.
//
PVOID FMD_Init(LPCTSTR lpActiveReg, PPCI_REG_INFO pRegIn, PPCI_REG_INFO pRegOut)
{
    pBSPArgs = ((BSP_ARGS *) IMAGE_SHARE_ARGS_UA_START);

//	RETAILMSG(1, (TEXT("FMD::FMD_Init\r\n")));
    //  0. Create the Mutex for shared access between the kernel and MSFLASH

//	RETAILMSG(1, (TEXT("FlashDrv!FMD!FMD_Init: pNFReg = %x \r\n"), pNFReg));
    pNFReg = (PUSHORT) (NAND_BASE);
//#if defined SMALLBLOCK_FIRST || defined LARGEBLOCK_FIRST
    v_pIOPregs = (S3C2440A_IOPORT_REG*)(IOP_BASE);
//#endif

    pNFCONF		= pNFReg;
    pNFCONT		= (PUSHORT) ((PBYTE) pNFReg + 0x04);
    pNFCMD		= (PUSHORT) ((PBYTE) pNFReg + 0x08);
    pNFADDR		= (PUSHORT) ((PBYTE) pNFReg + 0x0C);
    pNFDATA		= (PULONG)  ((PBYTE) pNFReg + 0x10);
	pNFMECCD0	= (PULONG)  ((PBYTE) pNFReg + 0x14);
	pNFMECCD1	= (PULONG)  ((PBYTE) pNFReg + 0x18);
    pNFSTAT		= (PUSHORT) ((PBYTE) pNFReg + 0x20);
	pNFESTAT0	= (PULONG)  ((PBYTE) pNFReg + 0x24);
	pNFESTAT1	= (PULONG)  ((PBYTE) pNFReg + 0x28);
	pNFMECC0	= (PULONG)  ((PBYTE) pNFReg + 0x2C);
	pNFMECC1	= (PULONG)  ((PBYTE) pNFReg + 0x30);
    pNFECC		= (PULONG)  ((PBYTE) pNFReg + 0x2C);
	pNFSBLK		= (PULONG)  ((PBYTE) pNFReg + 0x38);
	pNFEBLK		= (PULONG)  ((PBYTE) pNFReg + 0x3C);

    //  Now we need enable the NAND Flash controller
    BOOL bLastMode = SetKMode(TRUE);
    WRITE_REGISTER_USHORT(pNFCONF, (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4)|(0<<0));
	WRITE_REGISTER_USHORT(pNFCONT, (0<<13)|(0<<12)|(0<<10)|(0<<9)|(0<<8)|(1<<6)|(1<<5)|(1<<4)|(1<<1)|(1<<0));
    WRITE_REGISTER_USHORT(pNFSTAT, 0);

#if defined (MAGNETO)
	DefineLayout();
#endif

    WRITE_REGISTER_ULONG(pNFEBLK, 0xFFFFE0);	// Maximum Number
#ifdef ULDR
    WRITE_REGISTER_ULONG(pNFSBLK, 0x0);			// Minimum Number
#else

	RETAILMSG(1, (TEXT("FMD::FMD_Init - pBSPArgs->nfsblk = 0x%x \r\n"), pBSPArgs->nfsblk));
	RETAILMSG(1, (TEXT("FMD::FMD_Init - READ_REGISTER_BYTE(pNFSBLK) = 0x%x \r\n"), READ_REGISTER_ULONG(pNFSBLK)));
	
	if ( (READ_REGISTER_ULONG(pNFSBLK) >> 5) == 0 )		// soft reset or IPL
	{
//#if defined (MAGNETO)
		if ( pBSPArgs->nfsblk == 0 )	// IPL
		{
			RETAILMSG(1, (TEXT("FMD::FMD_Init IPL ..... \r\n")));
		}
		else
//#endif
		{
			RETAILMSG(1, (TEXT("FMD::FMD_Init Softreset ..... \r\n")));

			if ( astNandSpec[dwPrimaryNandDevice].nSctsPerPg == 4 )		// Large Block
			{
			    WRITE_REGISTER_ULONG(pNFSBLK, pBSPArgs->nfsblk << 6);				// Minimum Number
			}
			else
			{
			    WRITE_REGISTER_ULONG(pNFSBLK, pBSPArgs->nfsblk << 5);				// Minimum Number
			}
			WRITE_REGISTER_USHORT(pNFCONT, READ_REGISTER_USHORT(pNFCONT) | (1<<13));
		}
	}
	else
	{
		RETAILMSG(1, (TEXT("FMD::FMD_Init First Booting IN OS .... \r\n")));

		if ( astNandSpec[dwPrimaryNandDevice].nSctsPerPg == 4 )		// Large Block
		{
			WRITE_REGISTER_ULONG(pNFSBLK, pBSPArgs->nfsblk << 6);				// Minimum Number
		}
		else
		{
			WRITE_REGISTER_ULONG(pNFSBLK, pBSPArgs->nfsblk << 5);				// Minimum Number
		}
//		WRITE_REGISTER_ULONG(pNFSBLK, pBSPArgs->nfsblk << 5);				// Minimum Number
		WRITE_REGISTER_USHORT(pNFCONT, READ_REGISTER_USHORT(pNFCONT) | (1<<13));
	}
#endif

//#if defined SMALLBLOCK_FIRST || defined LARGEBLOCK_FIRST
	//v_pIOPregs->GPACON &= ~(1<<16);
	//v_pIOPregs->GPADAT |= (1<<16);
//#endif

    SetKMode(bLastMode);

//	RETAILMSG(1, (TEXT("FMD::FMD_Init Done\r\n")));
	
//	NANDTest();

    return (PVOID)pNFCONF;
}

//  FMD_Deinit
//
//  De-initialize the flash chip
//
BOOL    FMD_Deinit(PVOID hFMD)
{
    return TRUE;
}


/*****************************************************************************/
/*                                                                           */
/* NAME                                                                      */
/*      _ReadXID                                                             */
/* DESCRIPTION                                                               */
/*      This function reads ManufavturerID and DeviceID.                     */
/* PARAMETERS                                                                */
/*      pMID                                                                 */
/*          NAND Flash Manufacturer ID                                       */
/*          NOW, only detect SAMSUNG (0xEC)                                  */
/*      pDID                                                                 */
/*          NAND Flash device ID                                             */
/* RETURN VALUES                                                             */
/*      FAIL                                                     */
/*          In case of ID Finding Failure                                    */
/*      TRUE                                                          */
/*          Operation success                                                */
/* NOTES                                                                     */
/*                                                                           */
/*****************************************************************************/
INT32
_ReadXID(UINT8* pMID, UINT8* pDID, int mode)
{
    UINT32  nCnt, nRet;
    UINT8   n4thcycle, nBuff;
	int i;
    
//	RETAILMSG(1, (TEXT("[S2440:  IN] ++_ReadXID()\r\n")));
    n4thcycle = nBuff = 0;
    BOOL bLastMode = SetKMode(TRUE);

	SetChipSelect(mode, LOW);
	NF_CLEAR_RB();
    NF_CMD  (CMD_READ_ID);
    NF_ADDR (0x00);
	for ( i = 0; i < 100; i++ );

    /* tREA is necessary to Get a MID. */
    for (nCnt = 0; nCnt < 5; nCnt++)
    {
        *pMID = (BYTE) NF_DATA_R();
        if (0xEC == *pMID)
            break;
    }

    *pDID = (BYTE) NF_DATA_R();
    
    nBuff     = (BYTE) NF_DATA_R();
    n4thcycle = (BYTE) NF_DATA_R();
	SetChipSelect(mode, HIGH);

    SetKMode(bLastMode);

    if (*pMID != (UINT8)0xEC)
    {
        RETAILMSG(1, (TEXT("[S2440 : ERR] ID Finding Failure\r\n")));
        nRet = FALSE;
    }
    else
    {
//        RETAILMSG(1, (TEXT("MID = 0x%x, DID = 0x%x 4th Cycle : 0x%x\n"),*pMID, *pDID, n4thcycle));
        nRet = TRUE;
    }
//	RETAILMSG(1, (TEXT("[S2440 : OUT] --_ReadXID()\r\n")));
    return (nRet);
}

//  FMD_GetInfo
//
//  Return the Flash information
//
BOOL    FMD_GetInfo(PFlashInfo pFlashInfo)
{
 	UINT32  nCnt;
 	UINT8 nMID, nDID;

	if (!pFlashInfo)
		return(FALSE);

	pFlashInfo->flashType = NAND;

	if (_ReadXID(&nMID, &nDID, USE_NFCE) != TRUE)
	{
		RETAILMSG(1, (TEXT("[FMD_GetInfo : ERR] _ReadXID() Error\r\n")));
		return (FALSE);
	}

	for (nCnt = 0; astNandSpec[nCnt].nMID != 0; nCnt++)
	{
		if (nDID == astNandSpec[nCnt].nDID)
		{
			break;
		}
	}

	dwPrimaryNandDevice = nCnt;
	wPRIMARY_REAL_NAND_BLOCKS = astNandSpec[dwPrimaryNandDevice].nNumOfBlks;
	wSECONDARY_NAND_BLOCKS = 0;
	wSECONDARY_REAL_NAND_BLOCKS = 0;

	RETAILMSG(1, (TEXT("dwPrimaryNandDevice : %d(0x%x) \r\n"), dwPrimaryNandDevice, dwPrimaryNandDevice));
	RETAILMSG(1, (TEXT("astNandSpec[dwPrimaryNandDevice].nSctsPerPg: %d(0x%x) \r\n"), astNandSpec[dwPrimaryNandDevice].nSctsPerPg, astNandSpec[dwPrimaryNandDevice].nSctsPerPg));
	RETAILMSG(1, (TEXT("wPRIMARY_REAL_NAND_BLOCKS : %d(0x%x) \r\n"), wPRIMARY_REAL_NAND_BLOCKS, wPRIMARY_REAL_NAND_BLOCKS));

	if ( astNandSpec[dwPrimaryNandDevice].nSctsPerPg == 4 )	// Primary NAND is Large Block...
	{
		wPRIMARY_NAND_BLOCKS = wPRIMARY_REAL_NAND_BLOCKS ;
	}
	else
	{
		wPRIMARY_NAND_BLOCKS = wPRIMARY_REAL_NAND_BLOCKS / SB_BLOCK_LOOP;
	}
	wNUM_BLOCKS = wPRIMARY_NAND_BLOCKS + wSECONDARY_NAND_BLOCKS;

	//  OK, instead of reading it from the chip, we use the hardcoded
	//  numbers here.

	pFlashInfo->dwNumBlocks		= wNUM_BLOCKS;
	pFlashInfo->wSectorsPerBlock	= PAGES_PER_BLOCK;
	pFlashInfo->wDataBytesPerSector = SECTOR_SIZE;
	pFlashInfo->dwBytesPerBlock     = (pFlashInfo->wSectorsPerBlock * pFlashInfo->wDataBytesPerSector);

	RETAILMSG(1, (TEXT("NUMBLOCKS : %d(0x%x), SECTORSPERBLOCK = %d(0x%x), BYTESPERSECTOR = %d(0x%x) \r\n"), pFlashInfo->dwNumBlocks, pFlashInfo->dwNumBlocks, pFlashInfo->wSectorsPerBlock, pFlashInfo->wSectorsPerBlock, pFlashInfo->wDataBytesPerSector, pFlashInfo->wDataBytesPerSector));

	return TRUE;
}

#if defined (MAGNETO)
BOOL  FMD_GetInfoEx(PFlashInfoEx pFlashInfo, PDWORD pdwNumRegions)
{
	// Temp
	RETAILMSG(1, (L"FMD_GetInfoEx enter.\r\n"));

	if (!pdwNumRegions) 
	{
		return FALSE;
	}
    
	if (!pFlashInfo)
	{
		// Return required buffer size to caller
		*pdwNumRegions = g_dwNumRegions;
		return TRUE;
	}

	if (*pdwNumRegions < g_dwNumRegions)
	{
		*pdwNumRegions = g_dwNumRegions;
		DEBUGMSG (1, (TEXT("FMD_GetInfoEx: Insufficient buffer for number of regions")));
		return FALSE;
	}

	memcpy (pFlashInfo->region, g_pRegionTable, g_dwNumRegions * sizeof(FlashRegion));

	// Temp
	for (DWORD iRegion = 0; iRegion < g_dwNumRegions; iRegion++)
	{
		RETAILMSG(1, (L"Type=%d, StartP=0x%x, NumP=0x%x, NumL=0x%x, Sec/Blk=0x%x, B/Blk=0x%x, Compact=%d.\r\n", 
		g_pRegionTable[iRegion].regionType,
		g_pRegionTable[iRegion].dwStartPhysBlock,
		g_pRegionTable[iRegion].dwNumPhysBlocks,
		g_pRegionTable[iRegion].dwNumLogicalBlocks,
		g_pRegionTable[iRegion].dwSectorsPerBlock,
		g_pRegionTable[iRegion].dwBytesPerBlock,
		g_pRegionTable[iRegion].dwCompactBlocks));

	}

	*pdwNumRegions = g_dwNumRegions;

	pFlashInfo->cbSize					= sizeof(FlashInfoEx);
	pFlashInfo->flashType				= NAND;
	pFlashInfo->dwNumBlocks				= wNUM_BLOCKS;
	pFlashInfo->dwDataBytesPerSector		= SECTOR_SIZE;
	pFlashInfo->dwNumRegions			= g_dwNumRegions;

 	return(TRUE);
}

static BOOL DefineLayout()
{
	PFlashRegion pRegion = NULL;
	DWORD dwBlock = 0;

	if (!FMD_GetInfo (&g_flashInfo))
	{
		return FALSE;
	}
    
	// Find the MBR to determine if there is a flash layout sector
	g_dwNumRegions = 0;


	// Find the first usuable block
	while (dwBlock < g_flashInfo.dwNumBlocks)
	{
		if (!(FMD_GetBlockStatus(dwBlock) & (BLOCK_STATUS_BAD | BLOCK_STATUS_RESERVED)))
		{
			break;
		}
		dwBlock++;
	}

	RETAILMSG(1, (TEXT("DefineLayout: dwBlock = 0x%x \r\n"), dwBlock));

	DWORD dwSector = dwBlock * g_flashInfo.wSectorsPerBlock;
	if (!FMD_ReadSector (dwSector, g_pFLSBuffer, NULL, 1))
	{
		return FALSE;
	}

	// compare the signatures
	if (IS_VALID_BOOTSEC(g_pFLSBuffer)) 
	{
		if (!FMD_ReadSector (dwSector+1, g_pFLSBuffer, NULL, 1))
		{
			return FALSE;
		}
		if (IS_VALID_FLS(g_pFLSBuffer)) 
		{           
			PFlashLayoutSector pFLS = (PFlashLayoutSector)(g_pFLSBuffer);
            
			// Cache the flash layout sector information
			g_dwNumRegions = pFLS->cbRegionEntries / sizeof(FlashRegion);
			//RETAILMSG(1, (TEXT("DefineLayout: g_dwNumRegions = 0x%x \r\n"), g_dwNumRegions));
           
			// FlashRegion table starts after the ReservedEntry table. 
			if (g_dwNumRegions)
			{
				pRegion = (PFlashRegion)((LPBYTE)pFLS + sizeof(FlashLayoutSector) + pFLS->cbReservedEntries); 
			}
		}
	}
   
	if (!g_dwNumRegions) 
	{
		g_dwNumRegions = 1;
	}

	if (g_dwNumRegions > MAX_REGIONS)
		return FALSE;

	if (pRegion)
	{
		memcpy (g_pRegionTable, pRegion, g_dwNumRegions * sizeof(FlashRegion));
	}
	else
	{
		g_pRegionTable[0].dwStartPhysBlock = 0;
		g_pRegionTable[0].dwNumPhysBlocks = g_flashInfo.dwNumBlocks;
		g_pRegionTable[0].dwNumLogicalBlocks = FIELD_NOT_IN_USE;        
		g_pRegionTable[0].dwBytesPerBlock = g_flashInfo.dwBytesPerBlock;
		g_pRegionTable[0].regionType = FILESYS;
		g_pRegionTable[0].dwSectorsPerBlock = g_flashInfo.wSectorsPerBlock;
		g_pRegionTable[0].dwCompactBlocks = DEFAULT_COMPACTION_BLOCKS;    
	}


	RETAILMSG(1, (TEXT("DefineLayout: g_pRegionTable[0].dwStartPhysBlock = 0x%x , g_pRegionTable[0].dwNumPhysBlocks = 0x%x \r\n"), g_pRegionTable[0].dwStartPhysBlock, g_pRegionTable[0].dwNumPhysBlocks));

	return TRUE;
}
#endif

//  FMD_ReadSector
//
//  Read the content of the sector.
//
//  startSectorAddr: Starting page address
//  pSectorBuff  : Buffer for the data portion
//  pSectorInfoBuff: Buffer for Sector Info structure
//  dwNumSectors : Number of sectors
//
BOOL FMD_ReadSector(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff,
                        PSectorInfo pSectorInfoBuff, DWORD dwNumSectors)
{
	BOOL bRet;

//	RETAILMSG(1, (TEXT("FMD::FMD_ReadSector 0x%x \r\n"), startSectorAddr));

	if((startSectorAddr/PAGES_PER_BLOCK) < 1024)
	{
		return TRUE;
	}



	if ( startSectorAddr < (unsigned)wPRIMARY_NAND_BLOCKS*PAGES_PER_BLOCK )
	{
		if ( astNandSpec[dwPrimaryNandDevice].nSctsPerPg == 4 )
			bRet = FMD_LB_ReadSector(startSectorAddr, pSectorBuff, pSectorInfoBuff, dwNumSectors, USE_NFCE);
		else
			bRet = FMD_SB_ReadSector(startSectorAddr, pSectorBuff, pSectorInfoBuff, dwNumSectors, USE_NFCE);
	}
	else
	{
		if ( astNandSpec[dwSecondaryNandDevice].nSctsPerPg == 4 )
			bRet = FMD_LB_ReadSector(startSectorAddr-wPRIMARY_NAND_BLOCKS*PAGES_PER_BLOCK, pSectorBuff, pSectorInfoBuff, dwNumSectors, USE_GPIO);
		else
			bRet = FMD_SB_ReadSector(startSectorAddr-wPRIMARY_NAND_BLOCKS*PAGES_PER_BLOCK, pSectorBuff, pSectorInfoBuff, dwNumSectors, USE_GPIO);
	}

//	RETAILMSG(1, (TEXT("FMD::FMD_ReadSector -- \r\n")));

	return bRet;
}

//
//  IsBlockBad
//
//  Check to see if the given block is bad. A block is bad if the 517th byte on
//  the first or second page is not 0xff.
//
//  blockID:    The block address. We need to convert this to page address
//
//
BOOL IsBlockBad(BLOCK_ID blockID)
{
	BOOL bRet = FALSE;
	int i;

	if ( blockID < wPRIMARY_NAND_BLOCKS )
	{
		if ( astNandSpec[dwPrimaryNandDevice].nSctsPerPg == 4 )
		{
			for ( i = 0; i < LB_BLOCK_LOOP; i++ )
			{
				bRet = LB_IsBlockBad(blockID*(LB_BLOCK_LOOP) + i, USE_NFCE);
				if ( bRet == TRUE ) break;
			}
		}
		else
		{
			for ( i = 0; i < SB_BLOCK_LOOP; i++ )
			{
				bRet = SB_IsBlockBad(blockID*(SB_BLOCK_LOOP) + i, USE_NFCE);
				if ( bRet == TRUE ) break;
			}
		}
	}
	else
	{
		if ( astNandSpec[dwSecondaryNandDevice].nSctsPerPg == 4 )
		{
			for ( i = 0; i < LB_BLOCK_LOOP; i++ )
			{
				bRet = LB_IsBlockBad((blockID-wPRIMARY_NAND_BLOCKS)*(LB_BLOCK_LOOP) + i, USE_GPIO);
				if ( bRet == TRUE ) break;
			}
		}
		else
		{
			for ( i = 0; i < SB_BLOCK_LOOP; i++ )
			{
				bRet = SB_IsBlockBad((blockID-wPRIMARY_NAND_BLOCKS)*(SB_BLOCK_LOOP) + i, USE_GPIO);
				if ( bRet == TRUE ) break;
			}
		}
	}

	return bRet;
}

//
//  FMD_GetBlockStatus
//
//  Returns the status of a block.  The status information is stored in the spare area of the first sector for
//  the respective block.
//
//  A block is BAD if the bBadBlock byte on the first page is not equal to 0xff.
//
DWORD FMD_GetBlockStatus(BLOCK_ID blockID)
{
	DWORD dwResult = 0;
	int i;

	if ( blockID < wPRIMARY_NAND_BLOCKS )
	{
		if ( astNandSpec[dwPrimaryNandDevice].nSctsPerPg == 4 )
		{
			for ( i = 0; i < LB_BLOCK_LOOP; i++ )
			{
				dwResult |= FMD_LB_GetBlockStatus(blockID*(LB_BLOCK_LOOP) + i, USE_NFCE);
			}
		}
		else
		{
			for ( i = 0; i < SB_BLOCK_LOOP; i++ )
			{
				dwResult |= FMD_SB_GetBlockStatus(blockID*(SB_BLOCK_LOOP) + i, USE_NFCE);
			}
		}
	}
	else
	{
		if ( astNandSpec[dwSecondaryNandDevice].nSctsPerPg == 4 )
		{
			for ( i = 0; i < LB_BLOCK_LOOP; i++ )
			{
				dwResult |= FMD_LB_GetBlockStatus((blockID-wPRIMARY_NAND_BLOCKS)*(LB_BLOCK_LOOP) + i, USE_GPIO);
			}
		}
		else
		{
			for ( i = 0; i < SB_BLOCK_LOOP; i++ )
			{
				dwResult |= FMD_SB_GetBlockStatus((blockID-wPRIMARY_NAND_BLOCKS)*(SB_BLOCK_LOOP) + i, USE_GPIO);
			}
		}
	}

	return dwResult;
}

//  FMD_EraseBlock
//
//  Erase the given block
//
BOOL FMD_EraseBlock(BLOCK_ID blockID)
{
	BOOL    bRet = TRUE;
	int i;
	
	if(blockID < 1024)
		return TRUE;

	RETAILMSG(1,(TEXT("FMD_EreaseBlock %d\r\n"), blockID));
	if ( blockID < wPRIMARY_NAND_BLOCKS )
	{
		if ( astNandSpec[dwPrimaryNandDevice].nSctsPerPg == 4 )
		{
			for ( i = 0; i < LB_BLOCK_LOOP; i++ )
			{
				bRet = FMD_LB_EraseBlock(blockID*(LB_BLOCK_LOOP) + i, USE_NFCE);
				if ( bRet == FALSE ) break;
			}
		}
		else
		{
			for ( i = 0; i < SB_BLOCK_LOOP; i++ )
			{
				bRet = FMD_SB_EraseBlock(blockID*(SB_BLOCK_LOOP) + i, USE_NFCE);
				if ( bRet == FALSE ) break;
			}
		}
	}
	else
	{
		if ( astNandSpec[dwSecondaryNandDevice].nSctsPerPg == 4 )
		{
			for ( i = 0; i < LB_BLOCK_LOOP; i++ )
			{
				bRet = FMD_LB_EraseBlock((blockID-wPRIMARY_NAND_BLOCKS)*(LB_BLOCK_LOOP) + i, USE_GPIO);
				if ( bRet == FALSE ) break;
			}
		}
		else
		{
			for ( i = 0; i < SB_BLOCK_LOOP; i++ )
			{
				bRet = FMD_SB_EraseBlock((blockID-wPRIMARY_NAND_BLOCKS)*(SB_BLOCK_LOOP) + i, USE_GPIO);
				if ( bRet == FALSE ) break;
			}
		}
	}

	return bRet;
}

//  FMD_WriteSector
//
//  Write dwNumPages pages to the startSectorAddr
//
BOOL FMD_WriteSector(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, PSectorInfo pSectorInfoBuff,
                        DWORD dwNumSectors)
{
	BOOL    bRet = TRUE;

//	RETAILMSG(1, (TEXT("FMD::FMD_WriteSector 0x%x \r\n"), startSectorAddr/PAGES_PER_BLOCK));

	if((startSectorAddr/PAGES_PER_BLOCK) < 1024)
	{
		return FALSE;
	}

	

	if ( startSectorAddr < (unsigned)wPRIMARY_NAND_BLOCKS*PAGES_PER_BLOCK )
	{
		if ( astNandSpec[dwPrimaryNandDevice].nSctsPerPg == 4 )
			bRet = FMD_LB_WriteSector(startSectorAddr, pSectorBuff, pSectorInfoBuff, dwNumSectors, USE_NFCE);
		else
			bRet = FMD_SB_WriteSector(startSectorAddr, pSectorBuff, pSectorInfoBuff, dwNumSectors, USE_NFCE);
	}
	else	// if ( PRIMARY_NAND == SMALL_BLOCK_NAND )
	{
		if ( astNandSpec[dwSecondaryNandDevice].nSctsPerPg == 4 )
			bRet = FMD_LB_WriteSector(startSectorAddr-wPRIMARY_NAND_BLOCKS*PAGES_PER_BLOCK, pSectorBuff, pSectorInfoBuff, dwNumSectors, USE_GPIO);
		else
			bRet = FMD_SB_WriteSector(startSectorAddr-wPRIMARY_NAND_BLOCKS*PAGES_PER_BLOCK, pSectorBuff, pSectorInfoBuff, dwNumSectors, USE_GPIO);
	}

	return bRet;
}

/*
 *  MarkBlockBad
 *
 *  Mark the block as a bad block. We need to write a 00 to the 517th byte
 */

BOOL MarkBlockBad(BLOCK_ID blockID)
{
	BOOL    bRet = TRUE;
	int i;

	if ( blockID < wPRIMARY_NAND_BLOCKS )
	{
		if ( astNandSpec[dwPrimaryNandDevice].nSctsPerPg == 4 )
		{
			for ( i = 0; i < LB_BLOCK_LOOP; i++ )
			{
				bRet = LB_MarkBlockBad(blockID*(LB_BLOCK_LOOP) + i, USE_NFCE);
				if ( bRet == FALSE ) break;
			}
		}
		else
		{
			for ( i = 0; i < SB_BLOCK_LOOP; i++ )
			{
				bRet = SB_MarkBlockBad(blockID*(SB_BLOCK_LOOP) + i, USE_NFCE);
				if ( bRet == FALSE ) break;
			}
		}
	}
	else	// if ( PRIMARY_NAND == SMALL_BLOCK_NAND )
	{
		if ( astNandSpec[dwSecondaryNandDevice].nSctsPerPg == 4 )
		{
			for ( i = 0; i < LB_BLOCK_LOOP; i++ )
			{
				bRet = LB_MarkBlockBad((blockID-wPRIMARY_NAND_BLOCKS)*(LB_BLOCK_LOOP) + i, USE_GPIO);
				if ( bRet == FALSE ) break;
			}
		}
		else
		{
			for ( i = 0; i < SB_BLOCK_LOOP; i++ )
			{
				bRet = SB_MarkBlockBad((blockID-wPRIMARY_NAND_BLOCKS)*(SB_BLOCK_LOOP) + i, USE_GPIO);
				if ( bRet == FALSE ) break;
			}
		}
	}

	return bRet;
}

//
//  FMD_SetBlockStatus
//
//  Sets the status of a block.  Only implement for bad blocks for now.
//  Returns TRUE if no errors in setting.
//
BOOL FMD_SetBlockStatus(BLOCK_ID blockID, DWORD dwStatus)
{
	BOOL    bRet = TRUE;
	int i;

	if ( blockID < wPRIMARY_NAND_BLOCKS )
	{
		if ( astNandSpec[dwPrimaryNandDevice].nSctsPerPg == 4 )
		{
			for ( i = 0; i < LB_BLOCK_LOOP; i++ )
			{
				bRet = FMD_LB_SetBlockStatus(blockID*(LB_BLOCK_LOOP) + i, dwStatus, USE_NFCE);
				if ( bRet == FALSE ) break;
			}
		}
		else
		{
			for ( i = 0; i < SB_BLOCK_LOOP; i++ )
			{
				bRet = FMD_SB_SetBlockStatus(blockID*(SB_BLOCK_LOOP) + i, dwStatus, USE_NFCE);
				if ( bRet == FALSE ) break;
			}
		}
	}
	else	// if ( PRIMARY_NAND == SMALL_BLOCK_NAND )
	{
		if ( astNandSpec[dwSecondaryNandDevice].nSctsPerPg == 4 )
		{
			for ( i = 0; i < LB_BLOCK_LOOP; i++ )
			{
				bRet = FMD_LB_SetBlockStatus((blockID-wPRIMARY_NAND_BLOCKS)*(LB_BLOCK_LOOP) + i, dwStatus, USE_GPIO);
				if ( bRet == FALSE ) break;
			}
		}
		else
		{
			for ( i = 0; i < SB_BLOCK_LOOP; i++ )
			{
				bRet = FMD_SB_SetBlockStatus((blockID-wPRIMARY_NAND_BLOCKS)*(SB_BLOCK_LOOP) + i, dwStatus, USE_GPIO);
				if ( bRet == FALSE ) break;
			}
		}
	}

	return bRet;
}

#ifndef NOSYSCALL
//  We don't have to build the following interface functions for the
//  bootloader.
//

//  FMD_PowerUp
//
//  Performs any necessary powerup procedures...
//
VOID FMD_PowerUp(VOID)
{
}

//  FMD_PowerDown
//
//  Performs any necessary powerdown procedures...
//
VOID FMD_PowerDown(VOID)
{
}

//  FMD_OEMIoControl
//
//  Used for any OEM defined IOCTL operations
//
BOOL  FMD_OEMIoControl(DWORD dwIoControlCode, PBYTE pInBuf, DWORD nInBufSize,
                       PBYTE pOutBuf, DWORD nOutBufSize, PDWORD pBytesReturned)
{
	BSP_ARGS *pBSPArgs = ((BSP_ARGS *) IMAGE_SHARE_ARGS_UA_START);
	BlockLockInfo * pLockInfo;

	switch(dwIoControlCode)
	{
		case IOCTL_FMD_GET_INTERFACE:
		{
			if (!pOutBuf || nOutBufSize < sizeof(FMDInterface))
			{
				DEBUGMSG(1, (TEXT("FMD_OEMIoControl: IOCTL_FMD_GET_INTERFACE bad parameter(s).\r\n")));
				return(FALSE);
			}    

			PFMDInterface pInterface = (PFMDInterface)pOutBuf;
			pInterface->cbSize = sizeof(FMDInterface);
			pInterface->pInit = FMD_Init;
			pInterface->pDeInit = FMD_Deinit;
			pInterface->pGetInfo = FMD_GetInfo;        
#if defined (MAGNETO)
			pInterface->pGetInfoEx = FMD_GetInfoEx;
#endif
			pInterface->pGetBlockStatus = FMD_GetBlockStatus;     
			pInterface->pSetBlockStatus = FMD_SetBlockStatus;
			pInterface->pReadSector = FMD_ReadSector;
			pInterface->pWriteSector = FMD_WriteSector;
			pInterface->pEraseBlock = FMD_EraseBlock;
			pInterface->pPowerUp = FMD_PowerUp;
			pInterface->pPowerDown = FMD_PowerDown;
			pInterface->pGetPhysSectorAddr = NULL;            
	
			break;
		}
		case IOCTL_FMD_LOCK_BLOCKS:
			pLockInfo = (BlockLockInfo *)pInBuf;
			RETAILMSG(1, (TEXT("IOCTL_FMD_LOCK_BLOCKS!!!!(0x%x,0x%x) \r\n"), pLockInfo->StartBlock, pLockInfo->NumBlocks));

			if ( astNandSpec[dwPrimaryNandDevice].nSctsPerPg == 4 )		// Large Block
			{
				if ( READ_REGISTER_BYTE(pNFSBLK) >> 6 < (ULONG)(pLockInfo->StartBlock + pLockInfo->NumBlocks) )
					WRITE_REGISTER_USHORT(pNFSBLK, (pLockInfo->StartBlock + pLockInfo->NumBlocks)<<6);
			}
			else	// Small Block
			{
				if ( READ_REGISTER_BYTE(pNFSBLK) >> 5 < (ULONG)(pLockInfo->StartBlock + pLockInfo->NumBlocks)*8 )
				{
					//RETAILMSG(1, (TEXT("Write value (0x%x) \r\n"), ((ULONG)(pLockInfo->StartBlock + pLockInfo->NumBlocks)*8)<<5));
					WRITE_REGISTER_ULONG(pNFSBLK, ((ULONG)(pLockInfo->StartBlock + pLockInfo->NumBlocks)*8)<<5);
					//RETAILMSG(1, (TEXT("Read value  (0x%x) \r\n"), READ_REGISTER_ULONG(pNFSBLK)));
				}
			}
			pBSPArgs->nfsblk = pLockInfo->StartBlock + pLockInfo->NumBlocks;

			break;
		case IOCTL_FMD_UNLOCK_BLOCKS:
			RETAILMSG(1, (TEXT("IOCTL_FMD_UNLOCK_BLOCKS!!!!(0x%x,0x%x) \r\n"), pLockInfo->StartBlock, pLockInfo->NumBlocks));
			RETAILMSG(1, (TEXT("S3C2440 Does not support IOCTL_FMD_UNLOCK_BLOCKS !!!! \r\n")));
			return(FALSE);
			/*
			pLockInfo = (BlockLockInfo *)pInBuf;
			RETAILMSG(1, (TEXT("IOCTL_FMD_UNLOCK_BLOCKS!!!!(0x%x,0x%x) \r\n"), pLockInfo->StartBlock, pLockInfo->NumBlocks));

			if ( astNandSpec[dwPrimaryNandDevice].nSctsPerPg == 4 )		// Large Block
			{
				if ( READ_REGISTER_BYTE(pNFSBLK) >> 5 > (ULONG)(pLockInfo->StartBlock - 1) )
					WRITE_REGISTER_USHORT(pNFSBLK, (pLockInfo->StartBlock - 1)<<5 );
			}
			else	// Small Block
			{
				if ( READ_REGISTER_BYTE(pNFSBLK) >> 5 > (ULONG)(pLockInfo->StartBlock - 1)*8 )
					WRITE_REGISTER_USHORT(pNFSBLK, ((ULONG)(pLockInfo->StartBlock - 1)*8)<<5 );
			}
			pBSPArgs->nfsblk = pLockInfo->StartBlock - 1;
			*/

			break;
			/*
		case IOCTL_FMD_READ_RESERVED:
			RETAILMSG(1,(TEXT("IOCTL_FMD_READ_RESERVED\r\n")));
			return(FALSE);	
			break;
		case IOCTL_FMD_WRITE_RESERVED:
			RETAILMSG(1,(TEXT("IOCTL_FMD_WRITE_RESERVED\r\n")));
			return(FALSE);	
			break;
		case IOCTL_FMD_GET_RESERVED_TABLE:
			RETAILMSG(1,(TEXT("IOCTL_FMD_GET_RESERVED_TABLE\r\n")));
			return(FALSE);	
			break;
		case IOCTL_FMD_SET_REGION_TABLE:
			RETAILMSG(1,(TEXT("IOCTL_FMD_SET_REGION_TABLE\r\n")));
			return(FALSE);	
			break;
		case IOCTL_FMD_SET_SECTORSIZE:
			RETAILMSG(1,(TEXT("IOCTL_FMD_SET_SECTORSIZE\r\n")));
			return(FALSE);	
			break;			
		case IOCTL_FMD_RAW_WRITE_BLOCKS:
			RETAILMSG(1,(TEXT("IOCTL_FMD_RAW_WRITE_BLOCKS\r\n")));
			return(FALSE);	
			break;	
		case IOCTL_FMD_GET_RAW_BLOCK_SIZE:
			RETAILMSG(1,(TEXT("IOCTL_FMD_GET_RAW_BLOCK_SIZE\r\n")));
			return(FALSE);	
			break;	
		case IOCTL_FMD_GET_INFO:
			RETAILMSG(1,(TEXT("IOCTL_FMD_GET_INFO\r\n")));
			return(FALSE);	
			break;	
			*/																		
	    default:
			RETAILMSG(1, (TEXT("FMD_OEMIoControl: unrecognized IOCTL (0x%x).\r\n"), dwIoControlCode));
			return(FALSE);
	}

	return TRUE; 
}

#endif // NOSYSCALL

//------------------------------- Private Interface (NOT used by the FAL) --------------------------

//  FMD_GetOEMReservedByte
//
//  Retrieves the OEM reserved byte (for metadata) for the specified physical sector.
//
//
BOOL FMD_GetOEMReservedByte(SECTOR_ADDR physicalSectorAddr, PBYTE pOEMReserved)
{
	if ( physicalSectorAddr < (unsigned)wPRIMARY_NAND_BLOCKS*PAGES_PER_BLOCK )		// First NAND Flash
	{
		if ( astNandSpec[dwPrimaryNandDevice].nSctsPerPg == 4 )
			FMD_LB_GetOEMReservedByte( physicalSectorAddr,  pOEMReserved, USE_NFCE);
		else
			FMD_SB_GetOEMReservedByte( physicalSectorAddr,  pOEMReserved, USE_NFCE);
	}
	else
	{
		if ( astNandSpec[dwSecondaryNandDevice].nSctsPerPg == 4 )
			FMD_LB_GetOEMReservedByte( physicalSectorAddr-wPRIMARY_NAND_BLOCKS*PAGES_PER_BLOCK,  pOEMReserved, USE_GPIO);
		else
			FMD_SB_GetOEMReservedByte( physicalSectorAddr-wPRIMARY_NAND_BLOCKS*PAGES_PER_BLOCK,  pOEMReserved, USE_GPIO);
	}
	return TRUE;
}

//  FMD_SetOEMReservedByte
//
//  Sets the OEM reserved byte (for metadata) for the specified physical sector.
//
BOOL FMD_SetOEMReservedByte(SECTOR_ADDR physicalSectorAddr, BYTE bOEMReserved)
{
    BOOL    bRet = TRUE;
	if ( physicalSectorAddr < (unsigned)wPRIMARY_NAND_BLOCKS*PAGES_PER_BLOCK )		// First NAND Flash
	{
		if ( astNandSpec[dwPrimaryNandDevice].nSctsPerPg == 4 )
			bRet = FMD_LB_SetOEMReservedByte(physicalSectorAddr, bOEMReserved, USE_NFCE);
		else
			bRet = FMD_SB_SetOEMReservedByte(physicalSectorAddr, bOEMReserved, USE_NFCE);
	}
	else
	{
		if ( astNandSpec[dwSecondaryNandDevice].nSctsPerPg == 4 )
			bRet = FMD_LB_SetOEMReservedByte(physicalSectorAddr-wPRIMARY_NAND_BLOCKS*PAGES_PER_BLOCK, bOEMReserved, USE_GPIO);
		else
			bRet = FMD_SB_SetOEMReservedByte(physicalSectorAddr-wPRIMARY_NAND_BLOCKS*PAGES_PER_BLOCK, bOEMReserved, USE_GPIO);
	}
    return bRet;
}

//---------------------------------------- Helper Functions ----------------------------------------

//  Interface function for testing purpose.
//
BOOL FMD_ReadSpare(DWORD dwStartPage, LPBYTE pBuff, DWORD dwNumPages)
{
	RETAILMSG(1, (TEXT("FMD_ReadSpare Not Support !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n")));
    return FALSE;
}

//  Reset the chip
//
void SB_NF_Reset(int mode)
{
	int i;
    SetChipSelect(mode,LOW);
	NF_CLEAR_RB();
    NF_CMD(CMD_RESET);
	for(i=0;i<10;i++);  //tWB = 100ns. //??????
    SetChipSelect(mode,HIGH);
}

/*
 *  NAND_SB_ReadSectorInfo
 *
 *  Read SectorInfo out of the spare area. The current implementation only handles
 *  one sector at a time.
 */
void NAND_SB_ReadSectorInfo(SECTOR_ADDR sectorAddr, PSectorInfo pInfo, int mode)
{
    BOOL bLastMode = SetKMode(TRUE);
    
    //  Chip enable
    SetChipSelect(mode,LOW);
	NF_CLEAR_RB();

    //  Write the command
    NF_CMD(CMD_READ2);

    //  Write the address
    NF_ADDR(0x00);
    NF_ADDR(sectorAddr & 0xff);
    NF_ADDR((sectorAddr >> 8) & 0xff);

    if (SB_NEED_EXT_ADDR) {
        NF_ADDR((sectorAddr >> 16) & 0xff);
    }

    //  Wait for the Ready bit
	NF_DETECT_RB();	 // Wait tR(max 12us)

    //  Read the SectorInfo data (we only need to read first 8 bytes)
    pInfo->dwReserved1  = NF_DATA_R4();

    //  OEM byte
    pInfo->bOEMReserved = (BYTE) NF_DATA_R();

    //  Read the bad block mark
    pInfo->bBadBlock = (BYTE) NF_DATA_R();

    //  Second reserved field (WORD)
    pInfo->wReserved2 = ((BYTE) NF_DATA_R() << 8);
    pInfo->wReserved2 |= ((BYTE) NF_DATA_R());

    SetChipSelect(mode,HIGH);

    SetKMode(bLastMode);
}

/*
 *  NAND_SB_WriteSectorInfo
 *
 *  Write SectorInfo out to the spare area. The current implementation only handles
 *  one sector at a time.
 */
BOOL NAND_SB_WriteSectorInfo(SECTOR_ADDR sectorAddr, PSectorInfo pInfo, int mode)
{
    BOOL    bRet = TRUE;

    BOOL bLastMode = SetKMode(TRUE);

    //  Chip enable
    SetChipSelect(mode,LOW);
	NF_CLEAR_RB();

    //  Write the command
    //  First, let's point to the spare area
    NF_CMD(CMD_READ2);
    NF_CMD(CMD_WRITE);

    //  Write the address
    NF_ADDR(0x00);
    NF_ADDR(sectorAddr & 0xff);
    NF_ADDR((sectorAddr >> 8) & 0xff);

    if (SB_NEED_EXT_ADDR) {
        NF_ADDR((sectorAddr >> 16) & 0xff);
    }

    //  Now let's write the SectorInfo data
    //
    //  Write the first reserved field (DWORD)
    NF_DATA_W4( pInfo->dwReserved1 );

    //  Write OEM reserved flag
    NF_DATA_W( (pInfo->bOEMReserved) );

    //  Write the bad block flag
    NF_DATA_W( (pInfo->bBadBlock) );

    //  Write the second reserved field
    NF_DATA_W( (pInfo->wReserved2 >> 8) & 0xff );
    NF_DATA_W( (pInfo->wReserved2) );

    //  Issue the write complete command
    NF_CMD(CMD_WRITE2);

    //  Check ready bit
	NF_DETECT_RB();	 // Wait tR(max 12us)

	if ( READ_REGISTER_USHORT(pNFSTAT) & STATUS_ILLACC )
	{
		RETAILMSG(1, (TEXT("NAND_WriteSectorInfo() ######## Error Programming page (Illigar Access) %d!\n"), sectorAddr));
		WRITE_REGISTER_USHORT(pNFSTAT, STATUS_ILLACC);	// Write 1 to clear.
        bRet = FALSE;
	}
	else
	{
		//  Check the status of program
		NF_CMD(CMD_STATUS);

		if(NF_DATA_R() & STATUS_ERROR) {
			RETAILMSG(1, (TEXT("NAND_WriteSectorInfo() ######## Error Programming page %d!\n"), sectorAddr));
			bRet = FALSE;
		}
	}

    SetChipSelect(mode,HIGH);

    SetKMode(bLastMode);
    return bRet;
}

//  FMD_SB_ReadSector
//
//  Read the content of the sector.
//
//  startSectorAddr: Starting page address
//  pSectorBuff  : Buffer for the data portion
//  pSectorInfoBuff: Buffer for Sector Info structure
//  dwNumSectors : Number of sectors
//
typedef union _ECCRegVal
{
    DWORD   dwECCVal;
    BYTE    bECCBuf[4];
} ECCRegVal;

BOOL FMD_SB_ReadSector(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff,
                        PSectorInfo pSectorInfoBuff, DWORD dwNumSectors, int mode)
{
    DWORD       i;
    BYTE        eccBuf[8];
    ECCRegVal   eccRegVal;

	RETAILMSG(1, (TEXT("FMD::FMD_SB_ReadSector 0x%x \r\n"), startSectorAddr));
	RETAILMSG(1, (TEXT("startSectorAddr = 0x%x \r\n"), startSectorAddr));

	//  Sanity check
    if (!pSectorBuff && !pSectorInfoBuff || dwNumSectors > 1) {
        RETAILMSG(1, (TEXT("Invalid parameters!\n")));
        return FALSE;
    }

    if(!pSectorBuff) {
        //  We are reading spare only
        NAND_SB_ReadSectorInfo(startSectorAddr, pSectorInfoBuff, mode);

        //  There is no ECC for the sector info, so the read always succeed.
        return TRUE;
    }

	BOOL bLastMode = SetKMode(TRUE);

	//  Initialize ECC register
	NF_RSTECC();
	NF_MECC_UnLock();

	//  Enable the chip
	SetChipSelect(mode,LOW);
	NF_CLEAR_RB();

	//  Issue command
	NF_CMD(CMD_READ);

	//  Set up address
	NF_ADDR(0x00);
	NF_ADDR((startSectorAddr) & 0xff);
	NF_ADDR((startSectorAddr >> 8) & 0xff);

	if (SB_NEED_EXT_ADDR) {
		NF_ADDR((startSectorAddr >> 16) & 0xff);
	}

//	RETAILMSG(1, (TEXT("1 \r\n")));
	NF_DETECT_RB();	 // Wait tR(max 12us)
//	RETAILMSG(1, (TEXT("2 \r\n")));

	//  BUGBUG, because Media Player for Pocket PC sometimes pass us un-aligned buffer
	//  we have to waste cycle here to work around this problem
	if( ((DWORD) pSectorBuff) & 0x3) {
		for(i=0; i<SECTOR_SIZE; i++) {
			pSectorBuff[i] = (BYTE) NF_DATA_R();
		}
	}
	else {
		//  The right way.
		ReadPage512(pSectorBuff, pNFDATA);
    }

    //  Do the ECC thing here
    //  We read the ECC value from the ECC register pFNECC
	NF_MECC_Lock();
    eccRegVal.dwECCVal = NF_ECC();

    //  Read the SectorInfo data
    if(pSectorInfoBuff) {
        //  Read the SectorInfo data (we only need to read first 8 bytes)
        pSectorInfoBuff->dwReserved1  = NF_DATA_R4();

        //  OEM byte
        pSectorInfoBuff->bOEMReserved = (BYTE) NF_DATA_R();

        //  Read the bad block mark
        pSectorInfoBuff->bBadBlock = (BYTE) NF_DATA_R();

        //  Second reserved field (WORD)
        pSectorInfoBuff->wReserved2 = ((BYTE) NF_DATA_R() << 8);
        pSectorInfoBuff->wReserved2 |= ((BYTE) NF_DATA_R());
    }
    else {
        //  Advance the read pointer
        for(i=0; i<sizeof(SectorInfo); i++) {
            eccBuf[i] = (BYTE) NF_DATA_R();
        }
    }

    //  Verify the ECC values
    //
    //  Read the ECC buffer
    for(i=0; i<3; i++) {
        eccBuf[i] = (BYTE) NF_DATA_R();
    }

    SetChipSelect(mode,HIGH);

    //  Copmare with the ECC generated from the HW

    if(eccBuf[0] != eccRegVal.bECCBuf[0] ||
       eccBuf[1] != eccRegVal.bECCBuf[1]  ||
       eccBuf[2] != eccRegVal.bECCBuf[2] ) {
//        RETAILMSG(1, (TEXT("FMD(FMD_SB_ReadSector): ECC ERROR - Page #: %d\r\n"), startSectorAddr));
        //  Now try to correct them
        if(!ECC_CorrectData(pSectorBuff, eccBuf, eccRegVal.bECCBuf)) {
 //           RETAILMSG(1, (TEXT("FMD: Unable to correct the ECC error - Page #: %d\r\n"), startSectorAddr));
            SetKMode(bLastMode);
			return FALSE;
        }
    }

//	RETAILMSG(1, (TEXT("FMD::FMD_SB_ReadSector -- \r\n")));

    SetKMode(bLastMode);
    return TRUE;
}

//
//  SB_IsBlockBad
//
//  Check to see if the given block is bad. A block is bad if the 517th byte on
//  the first or second page is not 0xff.
//
//  blockID:    The block address. We need to convert this to page address
//
//
BOOL SB_IsBlockBad(BLOCK_ID blockID, int mode)
{
    DWORD   dwPageID = blockID << SB_NAND_LOG_2_PAGES_PER_BLOCK;
    BOOL    bRet = FALSE;
    BYTE    wFlag;

    BOOL bLastMode = SetKMode(TRUE);

    //  Enable the chip
    SetChipSelect(mode,LOW);
	NF_CLEAR_RB();

    //  Issue the command
    NF_CMD(CMD_READ2);

    //  Set up address
    NF_ADDR(VALIDADDR);
    NF_ADDR((dwPageID) & 0xff);
    NF_ADDR((dwPageID >> 8) & 0xff);

    if (SB_NEED_EXT_ADDR) {
        NF_ADDR((dwPageID >> 16) & 0xff);
    }

    //  Wait for Ready bit
	NF_DETECT_RB();	 // Wait tR(max 12us)

    //  Now get the byte we want
    wFlag = (BYTE) NF_DATA_R();

    if(wFlag != 0xff) {
        bRet = TRUE;
    }

    //  Disable the chip
    SetChipSelect(mode,HIGH);

    SetKMode(bLastMode);
    return bRet;
}

//
//  FMD_SB_GetBlockStatus
//
//  Returns the status of a block.  The status information is stored in the spare area of the first sector for
//  the respective block.
//
//  A block is BAD if the bBadBlock byte on the first page is not equal to 0xff.
//
DWORD FMD_SB_GetBlockStatus(BLOCK_ID blockID, int mode)
{
    SECTOR_ADDR sectorAddr = blockID << SB_NAND_LOG_2_PAGES_PER_BLOCK;
    SectorInfo SI;
    DWORD dwResult = 0;

	BOOL bLastMode = SetKMode(TRUE);

//	RETAILMSG(1, (TEXT("FMD_SB_GetBlockStatus (0x%x)0x%x \r\n"), blockID, sectorAddr));

	if(!FMD_SB_ReadSector(sectorAddr, NULL, &SI, 1, mode))
	{
        return BLOCK_STATUS_UNKNOWN;
	}

    if(!(SI.bOEMReserved & OEM_BLOCK_READONLY))
	{
        dwResult |= BLOCK_STATUS_READONLY;
	}
	
    if( blockID/8 < pBSPArgs->nfsblk )
	{
		RETAILMSG(1,(TEXT("pBSPArgs->nfsblk=%d\r\n"),pBSPArgs->nfsblk));
        dwResult |= BLOCK_STATUS_READONLY;
	}
	
    if(SI.bBadBlock != 0xFF)
	{
        dwResult |= BLOCK_STATUS_BAD;
	}
	
	SetKMode(bLastMode);
	
    return dwResult;
}




//  FMD_EraseBlock
//
//  Erase the given block
//
BOOL FMD_SB_EraseBlock(BLOCK_ID blockID, int mode)
{
    BOOL    bRet = TRUE;
    DWORD   dwPageID = blockID << SB_NAND_LOG_2_PAGES_PER_BLOCK;

//	RETAILMSG(1, (TEXT("FMD::FMD_SB_EraseBlock 0x%x \r\n"), dwPageID));

    BOOL bLastMode = SetKMode(TRUE);

    //  Enable the chip
    SetChipSelect(mode,LOW);
	NF_CLEAR_RB();

    //  Issue command
    NF_CMD(CMD_ERASE);

    //  Set up address
    NF_ADDR((dwPageID) & 0xff);
    NF_ADDR((dwPageID >> 8) & 0xff);

    if (SB_NEED_EXT_ADDR) {
        NF_ADDR((dwPageID >> 16) & 0xff);
    }

    //  Complete erase operation
    NF_CMD(CMD_ERASE2);

    //  Wait for ready bit
	NF_DETECT_RB();	 // Wait tR(max 12us)

	if ( READ_REGISTER_USHORT(pNFSTAT) & STATUS_ILLACC )
	{
        RETAILMSG(1, (TEXT("SB######## Error Erasing block (Illigar Access) %d!\n"), blockID));
		WRITE_REGISTER_USHORT(pNFSTAT, STATUS_ILLACC);	// Write 1 to clear.
        bRet = FALSE;
	}
	else
	{
		//  Check the status
		NF_CMD(CMD_STATUS);

		if(NF_DATA_R() & STATUS_ERROR) {
//			RETAILMSG(1, (TEXT("SB######## Error Erasing block %d!\n"), blockID));
			bRet = FALSE;
		}
	}

    SetChipSelect(mode,HIGH);
    SetKMode(bLastMode);

    return bRet;
}



//  FMD_SB_WriteSector
//
//  Write dwNumPages pages to the startSectorAddr
//
BOOL FMD_SB_WriteSector(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, PSectorInfo pSectorInfoBuff,
                        DWORD dwNumSectors, int mode)
{
    DWORD   i;
    BOOL    bRet = TRUE;
    DWORD   dwECCVal;
    BYTE    eccBuf[4];

//	RETAILMSG(1, (TEXT("FMD::FMD_SB_WriteSector 0x%x \r\n"), startSectorAddr));

    //  Sanity check
    //  BUGBUGBUG: I need to come back to support dwNumSectors > 1
    //
    if((!pSectorBuff && !pSectorInfoBuff) || dwNumSectors != 1) {
        RETAILMSG(1, (TEXT("Invalid parameters!\n")));
        return FALSE;
    }

    if(!pSectorBuff) {
        //  If we are asked just to write the SectorInfo, we will do that separately
        bRet = NAND_SB_WriteSectorInfo(startSectorAddr, pSectorInfoBuff, mode);
		return bRet;			// Do not write the actual sector information...
    }

    BOOL bLastMode = SetKMode(TRUE);

    //  Initialize ECC register
    NF_RSTECC();
	NF_MECC_UnLock();

    //  Enable Chip
    SetChipSelect(mode,LOW);

    //  Issue command
    NF_CMD(CMD_READ);
    NF_CMD(CMD_WRITE);

    //  Setup address
    NF_ADDR(0x00);
    NF_ADDR((startSectorAddr) & 0xff);
    NF_ADDR((startSectorAddr >> 8) & 0xff);

    if (SB_NEED_EXT_ADDR) {
        NF_ADDR((startSectorAddr >> 16) & 0xff);
    }

    //  Special case to handle un-aligned buffer pointer.
    //
    if( ((DWORD) pSectorBuff) & 0x3) {
        //  Write the data
        for(i=0; i<SECTOR_SIZE; i++) {
            NF_DATA_W(pSectorBuff[i]);
        }
    }
    else {
        WritePage512(pSectorBuff, pNFDATA);
    }

    //  Read out the ECC value generated by HW
	NF_MECC_Lock();
    dwECCVal = NF_ECC();

	// Write the SectorInfo data to the media
	// NOTE: This hardware is odd: only a byte can be written at a time and it must reside in the
	//       upper byte of a USHORT.
	if(pSectorInfoBuff)
	{
        //  Write the first reserved field (DWORD)
        NF_DATA_W4(pSectorInfoBuff->dwReserved1);

        //  Write OEM reserved flag
        NF_DATA_W( (pSectorInfoBuff->bOEMReserved) );

        //  Write the bad block flag
        NF_DATA_W( (pSectorInfoBuff->bBadBlock) );

        //  Write the second reserved field
        NF_DATA_W( (pSectorInfoBuff->wReserved2 >> 8) & 0xff );
        NF_DATA_W( (pSectorInfoBuff->wReserved2) & 0xff );

	}else
	{
		// Make sure we advance the Flash's write pointer (even though we aren't writing the SectorInfo data)
		for(i=0; i<sizeof(SectorInfo); i++)
		{
            NF_DATA_W(0xff);
		}
	}

    //  ECC stuff should be here
    eccBuf[0] = (BYTE) ((dwECCVal) & 0xff);
    eccBuf[1] = (BYTE) ((dwECCVal >> 8) & 0xff);
    eccBuf[2] = (BYTE) ((dwECCVal >> 16) & 0xff);

    //  Write the ECC value to the flash
    for(i=0; i<3; i++) {
        NF_DATA_W(eccBuf[i]);
    }

    for(i=0; i<5; i++) {
        NF_DATA_W(0xff);
    }

	NF_CLEAR_RB();
    //  Finish up the write operation
    NF_CMD(CMD_WRITE2);

    //  Wait for RB
	NF_DETECT_RB();	 // Wait tR(max 12us)

	if ( READ_REGISTER_USHORT(pNFSTAT) & STATUS_ILLACC )
	{
		RETAILMSG(1, (TEXT("FMD_WriteSector() ######## Error Programming page (Illigar Access) %d!\n"), startSectorAddr));
		WRITE_REGISTER_USHORT(pNFSTAT, STATUS_ILLACC);	// Write 1 to clear.
        bRet = FALSE;
	}
	else
	{
		//  Check the status
		NF_CMD(CMD_STATUS);

		if(NF_DATA_R() & STATUS_ERROR) {
			RETAILMSG(1, (TEXT("FMD_WriteSector() ######## Error Programming page %d!\n"), startSectorAddr));
			bRet = FALSE;
		}
	}

    //  Disable the chip
    SetChipSelect(mode,HIGH);

    SetKMode(bLastMode);
    return bRet;
}

/*
 *  MarkBlockBad
 *
 *  Mark the block as a bad block. We need to write a 00 to the 517th byte
 */

BOOL SB_MarkBlockBad(BLOCK_ID blockID, int mode)
{
    DWORD   dwStartPage = blockID << SB_NAND_LOG_2_PAGES_PER_BLOCK;
    BOOL    bRet = TRUE;

	RETAILMSG(1, (TEXT("SB_MarkBlockBad 0x%x \r\n"), dwStartPage));

    BOOL bLastMode = SetKMode(TRUE);

    //  Enable chip
    SetChipSelect(mode,LOW);
	NF_CLEAR_RB();

    //  Issue command
    //  We are dealing with spare area
    NF_CMD(CMD_READ2);
    NF_CMD(CMD_WRITE);

    //  Set up address
    NF_ADDR(VALIDADDR);
    NF_ADDR((dwStartPage) & 0xff);
    NF_ADDR((dwStartPage >> 8) & 0xff);
    if (SB_NEED_EXT_ADDR) {
        NF_ADDR((dwStartPage >> 16) & 0xff);
    }

    NF_DATA_W(BADBLOCKMARK);

    //  Copmlete the write
    NF_CMD(CMD_WRITE2);

    //  Wait for RB
	NF_DETECT_RB();	 // Wait tR(max 12us)

	if ( READ_REGISTER_USHORT(pNFSTAT) & STATUS_ILLACC )
	{
		RETAILMSG(1, (TEXT("######## Failed to mark the block bad (Illigar Access) ! %d \n"), blockID));
		WRITE_REGISTER_USHORT(pNFSTAT, STATUS_ILLACC);	// Write 1 to clear.
        bRet = FALSE;
	}
	else
	{
		//  Get the status
		NF_CMD(CMD_STATUS);

		if(NF_DATA_R() &  STATUS_ERROR) {
			RETAILMSG(1, (TEXT("######## Failed to mark the block bad!\n")));
			bRet = FALSE;
		}
	}

    //  Disable chip select
    SetChipSelect(mode,HIGH);

    SetKMode(bLastMode);
    return bRet;
}

//
//  FMD_SB_SetBlockStatus
//
//  Sets the status of a block.  Only implement for bad blocks for now.
//  Returns TRUE if no errors in setting.
//
BOOL FMD_SB_SetBlockStatus(BLOCK_ID blockID, DWORD dwStatus, int mode)
{
    SECTOR_ADDR sectorAddr = blockID << SB_NAND_LOG_2_PAGES_PER_BLOCK;
	BYTE bStatus = 0;

    if(dwStatus & BLOCK_STATUS_BAD)
	{
        if(!SB_MarkBlockBad (blockID, mode))
        {
            return FALSE;
        }
    }

    // We don't currently support setting a block to read-only, so fail if request is
    // for read-only and block is not currently read-only.
    if(dwStatus & BLOCK_STATUS_READONLY)
	{
        if(!(FMD_SB_GetBlockStatus(blockID, mode) & BLOCK_STATUS_READONLY))
        {
            return FALSE;
        }
    }

    return TRUE;
}


//------------------------------- Private Interface (NOT used by the FAL) --------------------------

//  FMD_SB_GetOEMReservedByte
//
//  Retrieves the OEM reserved byte (for metadata) for the specified physical sector.
//
//
BOOL FMD_SB_GetOEMReservedByte(SECTOR_ADDR physicalSectorAddr, PBYTE pOEMReserved, int mode)
{
    BOOL bLastMode = SetKMode(TRUE);
    
    //  Enable chip select
    SetChipSelect(mode,LOW);
	NF_CLEAR_RB();

    //  Issue command
    NF_CMD(CMD_READ2);

    //  Set up address
    NF_ADDR(OEMADDR);
    NF_ADDR((physicalSectorAddr) & 0xff);
    NF_ADDR((physicalSectorAddr >> 8) & 0xff);

    if (SB_NEED_EXT_ADDR) {
        NF_ADDR((physicalSectorAddr >> 16) & 0xff);
    }

    //  Wait for the ready bit
	NF_DETECT_RB();	 // Wait tR(max 12us)

    //  Read the data
    *pOEMReserved = (BYTE) NF_DATA_R();

    //  Disable chip select
    SetChipSelect(mode,HIGH);

    SetKMode(bLastMode);
	return TRUE;

}


//  FMD_SB_SetOEMReservedByte
//
//  Sets the OEM reserved byte (for metadata) for the specified physical sector.
//
BOOL FMD_SB_SetOEMReservedByte(SECTOR_ADDR physicalSectorAddr, BYTE bOEMReserved, int mode)
{
    BOOL    bRet = TRUE;

    BOOL bLastMode = SetKMode(TRUE);

    //  Enable chip select
    SetChipSelect(mode,LOW);
	NF_CLEAR_RB();

    //  Issue command
    NF_CMD(CMD_READ2);
    NF_CMD(CMD_WRITE);

    //  Set up address
    NF_ADDR(OEMADDR);
    NF_ADDR((physicalSectorAddr) & 0xff);
    NF_ADDR((physicalSectorAddr >> 8) & 0xff);

    if (SB_NEED_EXT_ADDR) {
        NF_ADDR((physicalSectorAddr >> 16) & 0xff);
    }

    //  Write the data
    NF_DATA_W(bOEMReserved);

    //  Complete the write
    NF_CMD(CMD_WRITE2);

    //  Wait for the ready bit
	NF_DETECT_RB();	 // Wait tR(max 12us)

	if ( READ_REGISTER_USHORT(pNFSTAT) & STATUS_ILLACC )
	{
		RETAILMSG(1, (TEXT("######## Failed to set OEM Reserved byte (Illigar Access) ! %d \n"), physicalSectorAddr));
		WRITE_REGISTER_USHORT(pNFSTAT, STATUS_ILLACC);	// Write 1 to clear.
        bRet = FALSE;
	}
	else
	{
		//  Read the status
		NF_CMD(CMD_STATUS);

		//  Check the status
		if(NF_DATA_R() & STATUS_ERROR) {
			RETAILMSG(1, (TEXT("######## Failed to set OEM Reserved byte!\n")));
			bRet = FALSE;
		}
	}

    //  Disable chip select
    SetChipSelect(mode,HIGH);

    SetKMode(bLastMode);
    return bRet;
}


//  Reset the chip
//
void LB_NF_Reset(int mode)
{
	int i;
    SetChipSelect(mode,LOW);
	NF_CLEAR_RB();
    NF_CMD(CMD_RESET);
	for(i=0;i<10;i++);  //tWB = 100ns. //??????
    SetChipSelect(mode,HIGH);
}

/*
 *  NAND_LB_ReadSectorInfo
 *
 *  Read SectorInfo out of the spare area. The current implementation only handles
 *  one sector at a time.
 */
void NAND_LB_ReadSectorInfo(SECTOR_ADDR sectorAddr, PSectorInfo pInfo, int mode)
{
	volatile DWORD rddata;
	int NewSpareAddr = 2048 + 16*(sectorAddr%4);
	int NewSectorAddr = sectorAddr/4;

//	RETAILMSG(1, (TEXT("FMD::NAND_LB_ReadSectorInfo 0x%x \r\n"), sectorAddr));

    BOOL bLastMode = SetKMode(TRUE);
    
    //  Chip enable
    SetChipSelect(mode,LOW);
	NF_CLEAR_RB();

    //  Write the command
    NF_CMD(CMD_READ);

    //  Write the address
    NF_ADDR((NewSpareAddr)&0xff);
    NF_ADDR((NewSpareAddr>>8)&0xff);
    NF_ADDR((NewSectorAddr)&0xff);
    NF_ADDR((NewSectorAddr>>8) & 0xff);

    if (LB_NEED_EXT_ADDR) {
        NF_ADDR((NewSectorAddr >> 16) & 0xff);
    }

    NF_CMD(CMD_READ3);

    //  Wait for the Ready bit
	NF_DETECT_RB();	 // Wait tR(max 12us)

    //  Read the SectorInfo data (we only need to read first 8 bytes)
    pInfo->dwReserved1  = NF_DATA_R4();

	rddata = NF_DATA_R4();

    //  OEM byte
    pInfo->bOEMReserved = (BYTE) (rddata & 0xff);

    //  Read the bad block mark
    pInfo->bBadBlock = (BYTE) ((rddata>>8) & 0xff);

    //  Second reserved field (WORD)
    pInfo->wReserved2 = (WORD) ((rddata>>16) & 0xffff);

    SetChipSelect(mode,HIGH);

    SetKMode(bLastMode);
}

/*
 *  NAND_LB_WriteSectorInfo
 *
 *  Write SectorInfo out to the spare area. The current implementation only handles
 *  one sector at a time.
 */
BOOL NAND_LB_WriteSectorInfo(SECTOR_ADDR sectorAddr, PSectorInfo pInfo, int mode)
{
	volatile DWORD	wrdata;
    BOOL    bRet = TRUE;
	int NewSpareAddr = 2048 + 16*(sectorAddr%4);
	int NewSectorAddr = sectorAddr/4;

    BOOL bLastMode = SetKMode(TRUE);

    //  Chip enable
    SetChipSelect(mode,LOW);
	NF_CLEAR_RB();

    //  Write the command
    //  First, let's point to the spare area
    NF_CMD(CMD_WRITE);

    //  Write the address
    NF_ADDR((NewSpareAddr)&0xff);
    NF_ADDR((NewSpareAddr>>8)&0xff);
    NF_ADDR(NewSectorAddr & 0xff);
    NF_ADDR((NewSectorAddr >> 8) & 0xff);

    if (LB_NEED_EXT_ADDR) {
        NF_ADDR((NewSectorAddr >> 16) & 0xff);
    }

    //  Now let's write the SectorInfo data
    //
    //  Write the first reserved field (DWORD)
    NF_DATA_W4( pInfo->dwReserved1 );

	wrdata = (DWORD)(pInfo->bOEMReserved) | (((DWORD)(pInfo->bBadBlock) << 8)&0x0000ff00) | (((DWORD)(pInfo->wReserved2) << 16)&0xffff0000);

    NF_DATA_W4( wrdata );

    //  Issue the write complete command
    NF_CMD(CMD_WRITE2);

    //  Check ready bit
	NF_DETECT_RB();	 // Wait tR(max 12us)

	if ( READ_REGISTER_USHORT(pNFSTAT) & STATUS_ILLACC )
	{
		RETAILMSG(1, (TEXT("NAND_LB_WriteSectorInfo() ######## Error Programming page (Illigar Access) %d!\n"), sectorAddr));
		WRITE_REGISTER_USHORT(pNFSTAT, STATUS_ILLACC);	// Write 1 to clear.
        bRet = FALSE;
	}
	else
	{
		//  Check the status of program
		NF_CMD(CMD_STATUS);

		if(NF_DATA_R() & STATUS_ERROR) {
			RETAILMSG(1, (TEXT("NAND_LB_WriteSectorInfo() ######## Error Programming page %d!\n"), sectorAddr));
			bRet = FALSE;
		}
	}

    SetChipSelect(mode,HIGH);

    SetKMode(bLastMode);
    return bRet;
}

//  FMD_LB_ReadSector
//
//  Read the content of the sector.
//
//  startSectorAddr: Starting page address
//  pSectorBuff  : Buffer for the data portion
//  pSectorInfoBuff: Buffer for Sector Info structure
//  dwNumSectors : Number of sectors
//

BOOL FMD_LB_ReadSector(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff,
                        PSectorInfo pSectorInfoBuff, DWORD dwNumSectors, int mode)
{
    DWORD       i;
    BYTE        eccBuf[8];
	volatile DWORD		rddata;
	int NewSpareAddr = 2048 + 16*(startSectorAddr%4);
	int NewDataAddr = 512*(startSectorAddr%4);
	int NewSectorAddr = startSectorAddr/4;
//	DWORD	Mecc0, Mecc1;

//	RETAILMSG(1, (TEXT("FMD::FMD_LB_ReadSector 0x%x \r\n"), startSectorAddr));
//	RETAILMSG(1, (TEXT("startSectorAddr = 0x%x \r\n"), startSectorAddr));
//	RETAILMSG(1, (TEXT("NewSpareAddr = 0x%x \r\n"), NewSpareAddr));
//	RETAILMSG(1, (TEXT("NewDataAddr = 0x%x \r\n"), NewDataAddr));
//	RETAILMSG(1, (TEXT("NewSectorAddr = 0x%x \r\n"), NewSectorAddr));

    //  Sanity check
    if (!pSectorBuff && !pSectorInfoBuff || dwNumSectors > 1) {
        RETAILMSG(1, (TEXT("Invalid parameters!\n")));
        return FALSE;
    }

    if(!pSectorBuff) {
        //  We are reading spare only
//		RETAILMSG(1, (TEXT("FMD::FMD_LB_ReadSector **** Read Spare Only **** \r\n")));
        NAND_LB_ReadSectorInfo(startSectorAddr, pSectorInfoBuff, mode);

        //  There is no ECC for the sector info, so the read always succeed.
        return TRUE;
    }

//	RETAILMSG(1, (TEXT("FMD::FMD_LB_ReadSector 0x%x \r\n"), startSectorAddr));

	BOOL bLastMode = SetKMode(TRUE);

	//  Initialize ECC register
	NF_RSTECC();
	NF_MECC_UnLock();

	//  Enable the chip
	SetChipSelect(mode,LOW);
	NF_CLEAR_RB();

	//  Issue command
	NF_CMD(CMD_READ);

	//  Set up address
    NF_ADDR((NewDataAddr)&0xff);
    NF_ADDR(((NewDataAddr)>>8)&0xff);
	NF_ADDR((NewSectorAddr) & 0xff);
	NF_ADDR((NewSectorAddr >> 8) & 0xff);

	if (LB_NEED_EXT_ADDR) {
		NF_ADDR((NewSectorAddr >> 16) & 0xff);
	}

	NF_CMD(CMD_READ3);	// 2nd command

	NF_DETECT_RB();	 // Wait tR(max 12us)

	//  BUGBUG, because Media Player for Pocket PC sometimes pass us un-aligned buffer
	//  we have to waste cycle here to work around this problem
	if( ((DWORD) pSectorBuff) & 0x3) {
		for(i=0; i<SECTOR_SIZE/sizeof(DWORD); i++) {
			rddata = (DWORD) NF_DATA_R4();
			pSectorBuff[i*4+0] = (BYTE)(rddata & 0xff);
			pSectorBuff[i*4+1] = (BYTE)(rddata>>8 & 0xff);
			pSectorBuff[i*4+2] = (BYTE)(rddata>>16 & 0xff);
			pSectorBuff[i*4+3] = (BYTE)(rddata>>24 & 0xff);
		}
	}
	else {
		ReadPage512(pSectorBuff, pNFDATA);
    }
    //  Do the ECC thing here
    //  We read the ECC value from the ECC register pFNECC
	NF_MECC_Lock();

//	Mecc0 = READ_REGISTER_ULONG(pNFMECC0);
//	Mecc1 = READ_REGISTER_ULONG(pNFMECC1);

	NF_CMD(CMD_RDO);
	//  Set up address
    NF_ADDR((NewSpareAddr)&0xff);
    NF_ADDR(((NewSpareAddr)>>8)&0xff);
	NF_CMD(CMD_RDO2);

    //  Read the SectorInfo data
    if(pSectorInfoBuff) {
        //  Read the SectorInfo data (we only need to read first 8 bytes)
        pSectorInfoBuff->dwReserved1  = NF_DATA_R4();
		rddata = NF_DATA_R4();
		//  OEM byte
		pSectorInfoBuff->bOEMReserved = (BYTE) (rddata & 0xff);

		//  Read the bad block mark
		pSectorInfoBuff->bBadBlock = (BYTE) ((rddata>>8) & 0xff);

		//  Second reserved field (WORD)
		pSectorInfoBuff->wReserved2 = (WORD) ((rddata>>16) & 0xffff);
    }
    else {
        //  Advance the read pointer
        for(i=0; i<sizeof(SectorInfo)/sizeof(DWORD); i++) {
            rddata = (DWORD) NF_DATA_R4();		// read and trash the data
        }
    }

//	RETAILMSG(1, (TEXT("3 \r\n")));
    //  Verify the ECC values
    //
    //  Read the ECC buffer 8bytes
	for(i=0; i<2; i++) {
		rddata = (DWORD) NF_DATA_R4();
//		RETAILMSG(1, (TEXT("rddata 0x%x \r\n"), rddata));
		eccBuf[i*4+0] = (BYTE)(rddata & 0xff);
		eccBuf[i*4+1] = (BYTE)(rddata>>8 & 0xff);
		eccBuf[i*4+2] = (BYTE)(rddata>>16 & 0xff);
		eccBuf[i*4+3] = (BYTE)(rddata>>24 & 0xff);
    }

//	Mecc0 = READ_REGISTER_ULONG(pNFMECC0);

    //  Copmare with the ECC generated from the HW
	WRITE_REGISTER_ULONG(pNFMECCD0, (DWORD)((DWORD)(eccBuf[1]<<16) | (DWORD)(eccBuf[0]&0xff)));
	WRITE_REGISTER_ULONG(pNFMECCD1, (DWORD)((DWORD)(eccBuf[3]<<16) | (DWORD)(eccBuf[2]&0xff)));
//	WRITE_REGISTER_ULONG(pNFMECCD0, (DWORD)((eccBuf[5])<<24) | (DWORD)((eccBuf[1])<<16) | (DWORD)((eccBuf[4])<<8) | (DWORD)((eccBuf[0]&0xff)));
//	WRITE_REGISTER_ULONG(pNFMECCD1, (DWORD)((eccBuf[7])<<24) | (DWORD)((eccBuf[3])<<16) | (DWORD)((eccBuf[6])<<8) | (DWORD)((eccBuf[2]&0xff)));

	SetChipSelect(mode,HIGH);

//	RETAILMSG(1, (TEXT("4 \r\n")));
	if ((READ_REGISTER_ULONG(pNFESTAT0)&0x3) == 0x0){
	}
	else {
//		RETAILMSG(1, (TEXT("FMD(FMD_LB_ReadSector): ECC ERROR - Page #: 0x%x \r\n"), startSectorAddr));
		return FALSE;
	}

	SetKMode(bLastMode);

//	RETAILMSG(1, (TEXT("FMD::FMD_LB_ReadSector -- \r\n")));

	return TRUE;
}

//
//  LB_IsBlockBad
//
//  Check to see if the given block is bad. A block is bad if the 517th byte on
//  the first or second page is not 0xff.
//
//  blockID:    The block address. We need to convert this to page address
//
//
BOOL LB_IsBlockBad(BLOCK_ID blockID, int mode)
{
    DWORD   dwPageID = blockID << LB_NAND_LOG_2_PAGES_PER_BLOCK;
    BOOL    bRet = FALSE;
    BYTE    wFlag;

    BOOL bLastMode = SetKMode(TRUE);

    //  Enable the chip
    SetChipSelect(mode,LOW);
	NF_CLEAR_RB();

    //  Issue the command
    NF_CMD(CMD_READ);

    //  Set up address
    NF_ADDR((2048+VALIDADDR)&0xff);
    NF_ADDR(((2048+VALIDADDR)>>8)&0xff);
    NF_ADDR((dwPageID) & 0xff);
    NF_ADDR((dwPageID >> 8) & 0xff);

    if (LB_NEED_EXT_ADDR) {
        NF_ADDR((dwPageID >> 16) & 0xff);
    }

	NF_CMD(CMD_READ3);

    //  Wait for Ready bit
	NF_DETECT_RB();	 // Wait tR(max 12us)

    //  Now get the byte we want
    wFlag = (BYTE)(NF_DATA_R());

    if(wFlag != 0xff) {
		RETAILMSG(1, (TEXT("FMD: IsBlockBad - Page #: 0x%x \r\n"), dwPageID));
        bRet = TRUE;
    }

    //  Disable the chip
    SetChipSelect(mode,HIGH);

    SetKMode(bLastMode);
    return bRet;
}

//
//  FMD_LB_GetBlockStatus
//
//  Returns the status of a block.  The status information is stored in the spare area of the first sector for
//  the respective block.
//
//  A block is BAD if the bBadBlock byte on the first page is not equal to 0xff.
//
DWORD FMD_LB_GetBlockStatus(BLOCK_ID blockID, int mode)
{
    SECTOR_ADDR sectorAddr = blockID << LB_NAND_LOG_2_PAGES_PER_BLOCK;
    SectorInfo SI;
    DWORD dwResult = 0;

	//RETAILMSG(1, (TEXT("FMD_LB_GetBlockStatus (0x%x)0x%x \r\n"), blockID, sectorAddr));
	BOOL bLastMode = SetKMode(TRUE);
	
	if(!FMD_LB_ReadSector(sectorAddr<<2, NULL, &SI, 1, mode))
	{
        return BLOCK_STATUS_UNKNOWN;
	}

    if(!(SI.bOEMReserved & OEM_BLOCK_READONLY))
	{
        dwResult |= BLOCK_STATUS_READONLY;
	}
	
    if( blockID < pBSPArgs->nfsblk )
	{
        dwResult |= BLOCK_STATUS_READONLY;
	}
	
    if(SI.bBadBlock != 0xFF)
	{
        dwResult |= BLOCK_STATUS_BAD;
	}
	SetKMode(bLastMode);

    return dwResult;
}




//  FMD_LB_EraseBlock
//
//  Erase the given block
//
BOOL FMD_LB_EraseBlock(BLOCK_ID blockID, int mode)
{
    BOOL    bRet = TRUE;
    DWORD   dwPageID = blockID << LB_NAND_LOG_2_PAGES_PER_BLOCK;

//	RETAILMSG(1, (TEXT("FMD_LB_EraseBlock 0x%x \r\n"), blockID));

    BOOL bLastMode = SetKMode(TRUE);

    //  Enable the chip
    SetChipSelect(mode,LOW);
	NF_CLEAR_RB();

    //  Issue command
    NF_CMD(CMD_ERASE);

    //  Set up address
    NF_ADDR((dwPageID) & 0xff);
    NF_ADDR((dwPageID >> 8) & 0xff);

    if (LB_NEED_EXT_ADDR) {
        NF_ADDR((dwPageID >> 16) & 0xff);
    }

    //  Complete erase operation
    NF_CMD(CMD_ERASE2);

    //  Wait for ready bit
	NF_DETECT_RB();	 // Wait tR(max 12us)

	if ( READ_REGISTER_USHORT(pNFSTAT) & STATUS_ILLACC )
	{
        RETAILMSG(1, (TEXT("LB######## Error Erasing block (Illigar Access) %d!\n"), blockID));
		WRITE_REGISTER_USHORT(pNFSTAT, STATUS_ILLACC);	// Write 1 to clear.
        bRet = FALSE;
	}
	else
	{
		//  Check the status
		NF_CMD(CMD_STATUS);

		if(NF_DATA_R() & STATUS_ERROR) {
			RETAILMSG(1, (TEXT("LB######## Error Erasing block %d!\n"), blockID));
			bRet = FALSE;
		}
	}

    SetChipSelect(mode,HIGH);

    SetKMode(bLastMode);

    return bRet;
}



//  FMD_LB_WriteSector
//
//  Write dwNumPages pages to the startSectorAddr
//
BOOL FMD_LB_WriteSector(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, PSectorInfo pSectorInfoBuff, DWORD dwNumSectors, int mode)
{
    DWORD   i;
    BOOL    bRet = TRUE;
	volatile DWORD	wrdata;
	DWORD	Mecc0;
	int NewSpareAddr = 2048 + 16*(startSectorAddr%4);
	int NewDataAddr = 512*(startSectorAddr%4);
	int NewSectorAddr = startSectorAddr/4;

//	RETAILMSG(1, (TEXT("FMD::FMD_LB_WriteSector 0x%x \r\n"), startSectorAddr));

    //  Sanity check
    //  BUGBUGBUG: I need to come back to support dwNumSectors > 1
    //
    if((!pSectorBuff && !pSectorInfoBuff) || dwNumSectors != 1) {
        RETAILMSG(1, (TEXT("Invalid parameters!\n")));
        return FALSE;
    }

    if(!pSectorBuff) {
        //  If we are asked just to write the SectorInfo, we will do that separately
        bRet = NAND_LB_WriteSectorInfo(startSectorAddr, pSectorInfoBuff, mode);
		return bRet;			// Do not write the actual sector information...
    }

    BOOL bLastMode = SetKMode(TRUE);

    //  Initialize ECC register
    NF_RSTECC();
	NF_MECC_UnLock();

    //  Enable Chip
    SetChipSelect(mode,LOW);

    //  Issue command
    NF_CMD(CMD_WRITE);

    //  Setup address
    NF_ADDR((NewDataAddr)&0xff);
    NF_ADDR(((NewDataAddr)>>8)&0xff);
    NF_ADDR((NewSectorAddr) & 0xff);
    NF_ADDR((NewSectorAddr >> 8) & 0xff);

    if (LB_NEED_EXT_ADDR) {
        NF_ADDR((NewSectorAddr >> 16) & 0xff);
    }

    //  Special case to handle un-aligned buffer pointer.
    //
    if( ((DWORD) pSectorBuff) & 0x3) {
        //  Write the data
        for(i=0; i<SECTOR_SIZE/sizeof(DWORD); i++) {
			wrdata = pSectorBuff[i*4+0];
			wrdata |= pSectorBuff[i*4+1]<<8;
			wrdata |= pSectorBuff[i*4+2]<<16;
			wrdata |= pSectorBuff[i*4+3]<<24;
            NF_DATA_W4(wrdata);
        }
    }
    else {
#ifndef USENANDDMA
        WritePage512(pSectorBuff, pNFDATA);
#else	// USENANDDMA
#ifdef USESETKMODE
		SetKMode(TRUE);
		v_pINTregs->rSRCPND=BIT_DMA3;	// Init DMA src pending.
#endif // USESETKMODE
		memcpy(pDMABuffer, pSectorBuff, 512);
		// Nand to memory dma setting
	    v_pDMAregs->rDISRC3  = (int)SDI_DMA_BUFFER_PHYS; 	// Nand flash data register
	    v_pDMAregs->rDISRCC3 = (0<<1) | (0<<0); //arc=AHB,src_addr=inc
	    v_pDMAregs->rDIDST3  = (int)NFDATA;
	    v_pDMAregs->rDIDSTC3 = (0<<1) | (1<<0); //dst=AHB,dst_addr=fix;
	    v_pDMAregs->rDCON3   = (1<<31)|(1<<30)|(1<<29)|(0<<28)|(1<<27)|(0<<23)|(1<<22)|(2<<20)|(512/4);
		//  only unit transfer in writing!!!!
		//Handshake,AHB,interrupt,(unit),whole,S/W,no_autoreload,word,count=128;

		// DMA on and start.
	    v_pDMAregs->rDMASKTRIG3 = (1<<1)|(1<<0);

#ifndef USESETKMODE
		WaitForSingleObject(gDMA3IntrEvent, INFINITE);
		InterruptDone(SYSINTR_DMA3);
#else // USESETKMODE
		while(!(v_pINTregs->rSRCPND & BIT_DMA3));	// Wait until Dma transfer is done.
		v_pINTregs->rSRCPND=BIT_DMA3;
		SetKMode(FALSE);
#endif	// USESETKMODE
#endif	// USENANDDMA
    }

    //  Read out the ECC value generated by HW
	NF_MECC_Lock();

	Mecc0 = READ_REGISTER_ULONG(pNFMECC0);
//	Mecc1 = READ_REGISTER_ULONG(pNFMECC1);

    NF_CMD(CMD_RDI);
    NF_ADDR((NewSpareAddr)&0xff);
    NF_ADDR(((NewSpareAddr)>>8)&0xff);

	// Write the SectorInfo data to the media
	// NOTE: This hardware is odd: only a byte can be written at a time and it must reside in the
	//       upper byte of a USHORT.
	if(pSectorInfoBuff)
	{
        //  Write the first reserved field (DWORD)
        NF_DATA_W4(pSectorInfoBuff->dwReserved1);
		wrdata = (DWORD)(pSectorInfoBuff->bOEMReserved) | (((DWORD)(pSectorInfoBuff->bBadBlock) << 8)&0x0000ff00) | (((DWORD)(pSectorInfoBuff->wReserved2) << 16)&0xffff0000);
		NF_DATA_W4( wrdata );
	}else
	{
		// Make sure we advance the Flash's write pointer (even though we aren't writing the SectorInfo data)
		for(i=0; i<sizeof(SectorInfo)/sizeof(DWORD); i++)
		{
            NF_DATA_W4(0xffffffff);
		}
	}

    //  Write the ECC value to the flash
	NF_DATA_W4(Mecc0);
//	NF_DATA_W4(Mecc1);
//	NF_DATA_W4(Mecc0);

	NF_CLEAR_RB();

    //  Finish up the write operation
    NF_CMD(CMD_WRITE2);

    //  Wait for RB
	NF_DETECT_RB();	 // Wait tR(max 12us)

	if ( READ_REGISTER_USHORT(pNFSTAT) & STATUS_ILLACC )
	{
		RETAILMSG(1, (TEXT("FMD_WriteSector() ######## Error Programming page (Illigar Access) %d!\n"), startSectorAddr));
		WRITE_REGISTER_USHORT(pNFSTAT, STATUS_ILLACC);	// Write 1 to clear.
        bRet = FALSE;
	}
	else
	{
		//  Check the status
		NF_CMD(CMD_STATUS);

		if(NF_DATA_R() & STATUS_ERROR) {
			RETAILMSG(1, (TEXT("FMD_WriteSector() ######## Error Programming page %d!\n"), startSectorAddr));
			bRet = FALSE;
		}
	}

    //  Disable the chip
    SetChipSelect(mode,HIGH);

    SetKMode(bLastMode);
    return bRet;
}

/*
 *  LB_MarkBlockBad
 *
 *  Mark the block as a bad block. We need to write a 00 to the 517th byte
 */

BOOL LB_MarkBlockBad(BLOCK_ID blockID, int mode)
{
    DWORD   dwStartPage = blockID << LB_NAND_LOG_2_PAGES_PER_BLOCK;
    BOOL    bRet = TRUE;

	RETAILMSG(1, (TEXT("LB_MarkBlockBad 0x%x \r\n"), dwStartPage));
    BOOL bLastMode = SetKMode(TRUE);

    //  Enable chip
    SetChipSelect(mode,LOW);
	NF_CLEAR_RB();

    //  Issue command
    //  We are dealing with spare area
    NF_CMD(CMD_WRITE);

    //  Set up address
    NF_ADDR((2048+VALIDADDR)&0xff);
    NF_ADDR(((2048+VALIDADDR)>>8)&0xff);
    NF_ADDR((dwStartPage) & 0xff);
    NF_ADDR((dwStartPage >> 8) & 0xff);
    if (LB_NEED_EXT_ADDR) {
        NF_ADDR((dwStartPage >> 16) & 0xff);
    }

    NF_DATA_W(BADBLOCKMARK);

    //  Copmlete the write
    NF_CMD(CMD_WRITE2);

    //  Wait for RB
	NF_DETECT_RB();	 // Wait tR(max 12us)

	if ( READ_REGISTER_USHORT(pNFSTAT) & STATUS_ILLACC )
	{
		RETAILMSG(1, (TEXT("######## Failed to mark the block bad (Illigar Access) %d!\n"), blockID));
		WRITE_REGISTER_USHORT(pNFSTAT, STATUS_ILLACC);	// Write 1 to clear.
        bRet = FALSE;
	}
	else
	{
		//  Get the status
		NF_CMD(CMD_STATUS);

		if(NF_DATA_R() &  STATUS_ERROR) {
			RETAILMSG(1, (TEXT("######## Failed to mark the block bad!\n")));
			bRet = FALSE;
		}
	}

    //  Disable chip select
    SetChipSelect(mode,HIGH);

    SetKMode(bLastMode);
    return bRet;
}

//
//  FMD_SetBlockStatus
//
//  Sets the status of a block.  Only implement for bad blocks for now.
//  Returns TRUE if no errors in setting.
//
BOOL FMD_LB_SetBlockStatus(BLOCK_ID blockID, DWORD dwStatus, int mode)
{
	BYTE bStatus = 0;

    if(dwStatus & BLOCK_STATUS_BAD)
	{
        if(!LB_MarkBlockBad (blockID, mode))
        {
            return FALSE;
        }
    }

    // We don't currently support setting a block to read-only, so fail if request is
    // for read-only and block is not currently read-only.
    if(dwStatus & BLOCK_STATUS_READONLY)
	{
        if(!(FMD_LB_GetBlockStatus(blockID, mode) & BLOCK_STATUS_READONLY))
        {
            return FALSE;
        }
    }

    return TRUE;
}


//------------------------------- Private Interface (NOT used by the FAL) --------------------------

//  FMD_GetOEMReservedByte
//
//  Retrieves the OEM reserved byte (for metadata) for the specified physical sector.
//
//
BOOL FMD_LB_GetOEMReservedByte(SECTOR_ADDR physicalSectorAddr, PBYTE pOEMReserved, int mode)
{
	int NewSpareAddr = 2048 + 16*(physicalSectorAddr%4);
	int NewDataAddr = 512*(physicalSectorAddr%4);
	int NewSectorAddr = physicalSectorAddr/4;

	RETAILMSG(1, (TEXT("FMD_GetOEMReservedByte 0x%x \n"), physicalSectorAddr));
    BOOL bLastMode = SetKMode(TRUE);
    
    //  Enable chip select
    SetChipSelect(mode,LOW);
	NF_CLEAR_RB();

    //  Issue command
    NF_CMD(CMD_READ);

    //  Set up address
    NF_ADDR((NewSpareAddr+OEMADDR)&0xff);
    NF_ADDR(((NewSpareAddr+OEMADDR)>>8)&0xff);
    NF_ADDR((NewSectorAddr) & 0xff);
    NF_ADDR((NewSectorAddr >> 8) & 0xff);

    if (LB_NEED_EXT_ADDR) {
        NF_ADDR((NewSectorAddr >> 16) & 0xff);
    }

	NF_CMD(CMD_READ3);

    //  Wait for the ready bit
	NF_DETECT_RB();	 // Wait tR(max 12us)

    //  Read the data
    *pOEMReserved = (BYTE) (NF_DATA_R());		// read and discard

    //  Disable chip select
    SetChipSelect(mode,HIGH);

    SetKMode(bLastMode);
	return TRUE;

}


//  FMD_SetOEMReservedByte
//
//  Sets the OEM reserved byte (for metadata) for the specified physical sector.
//
BOOL FMD_LB_SetOEMReservedByte(SECTOR_ADDR physicalSectorAddr, BYTE bOEMReserved, int mode)
{
    BOOL    bRet = TRUE;
	int NewSpareAddr = 2048 + 16*(physicalSectorAddr%4);
	int NewDataAddr = 512*(physicalSectorAddr%4);
	int NewSectorAddr = physicalSectorAddr/4;

	RETAILMSG(1, (TEXT("FMD_SetOEMReservedByte 0x%x \n"), physicalSectorAddr));
    BOOL bLastMode = SetKMode(TRUE);

    //  Enable chip select
    SetChipSelect(mode,LOW);
	NF_CLEAR_RB();

    //  Issue command
    NF_CMD(CMD_WRITE);

    //  Set up address
    NF_ADDR((NewSpareAddr+OEMADDR)&0xff);
    NF_ADDR(((NewSpareAddr+OEMADDR)>>8)&0xff);
    NF_ADDR((NewSectorAddr) & 0xff);
    NF_ADDR((NewSectorAddr >> 8) & 0xff);

    if (LB_NEED_EXT_ADDR) {
        NF_ADDR((NewSectorAddr >> 16) & 0xff);
    }

    //  Write the data
    NF_DATA_W(bOEMReserved);

    //  Complete the write
    NF_CMD(CMD_WRITE2);

    //  Wait for the ready bit
	NF_DETECT_RB();	 // Wait tR(max 12us)

	if ( READ_REGISTER_USHORT(pNFSTAT) & STATUS_ILLACC )
	{
		RETAILMSG(1, (TEXT("######## Failed to set OEM Reserved byte (Illigar Access) %d!\n"), physicalSectorAddr));
		WRITE_REGISTER_USHORT(pNFSTAT, STATUS_ILLACC);	// Write 1 to clear.
        bRet = FALSE;
	}
	else
	{
		//  Read the status
		NF_CMD(CMD_STATUS);

		//  Check the status
		if(NF_DATA_R() & STATUS_ERROR) {
			RETAILMSG(1, (TEXT("######## Failed to set OEM Reserved byte!\n")));
			bRet = FALSE;
		}
	}

    //  Disable chip select
    SetChipSelect(mode,HIGH);

    SetKMode(bLastMode);
    return bRet;
}


