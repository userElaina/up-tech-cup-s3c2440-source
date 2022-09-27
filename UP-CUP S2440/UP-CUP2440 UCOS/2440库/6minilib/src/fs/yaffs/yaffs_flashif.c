/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system. 
 * yaffs_ramdisk.c: yaffs ram disk component
 *
 * Copyright (C) 2002 Aleph One Ltd.
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 * 
 * 2004-10-26 Edit by threewater<threewater@up-tech.com>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

// This provides a rma disk under yaffs.
// NB this is not intended for NAND emulation.
// Use this with dev->useNANDECC enabled, then ECC overheads are not required.

const char *yaffs_flashif_c_version = "$Id: yaffs_flashif.c,v 1.1 2003/01/21 03:32:17 charles Exp $";

#include "../inc/drv/flash.h"
#include "yportenv.h"

#include "yaffs_flashif.h"
#include "yaffs_guts.h"
#include "devextras.h"


//#define BLOCK_SIZE (32 * 528)
//#define BLOCKS_PER_MEG ((1024*1024)/(32 * 512))

typedef struct 
{
	__u8 data[528]; // Data + spare
} yflash_Page;

typedef struct
{
	yflash_Page page[32]; // The pages in the block
	
} yflash_Block;



typedef struct
{
	yflash_Block **block;
	int nBlocks;
} yflash_Device;

static yflash_Device ramdisk;

static int  CheckInit(yaffs_Device *dev)
{
	static int initialised = 0;
	
	
	if(initialised) 
	{
		return YAFFS_OK;
	}

	initialised = 1;

	;
	if(NandFlash_init()<0){
		T(YAFFS_TRACE_ALWAYS,("Unsuported flash id\n"));
		return YAFFS_FAIL;
	}
	
	return YAFFS_OK;
}

int yflash_WriteChunkToNAND(yaffs_Device *dev,int chunkInNAND,const __u8 *data, yaffs_Spare *spare)
{
	int blk;
	int pg;
	

	CheckInit(dev);
	
	blk = chunkInNAND/32;
	pg = chunkInNAND%32;
	
	
	if(data){
		WritePage512(blk, pg, data);
	}
	
	if(spare)
	{
		WriteSparePage(blk, pg, (unsigned char*)spare);
	}

	return YAFFS_OK;	

}


int yflash_ReadChunkFromNAND(yaffs_Device *dev,int chunkInNAND, __u8 *data, yaffs_Spare *spare)
{
	int blk;
	int pg;

	
	CheckInit(dev);
	
	blk = chunkInNAND/32;
	pg = chunkInNAND%32;
	
	if(data)
	{
		ReadPage512(blk, pg, data);
	}
	
	
	if(spare)
	{
		ReadSparePage(blk, pg, (unsigned char*)spare);
		//fixed me! how to use ECC?
		if(dev->useNANDECC)
		{
			struct yaffs_NANDSpare *nsp = (struct yaffs_NANDSpare *)spare;
			nsp->eccres1 = 0;
			nsp->eccres2 = 0;
		}
	}

	return YAFFS_OK;
}


int yflash_CheckChunkErased(yaffs_Device *dev,int chunkInNAND)
{
	int blk;
	int pg;
	
	CheckInit(dev);
	
	blk = chunkInNAND/32;
	pg = chunkInNAND%32;	

	return (CheckPageEreased(blk, pg)==0) ? YAFFS_OK: YAFFS_FAIL;
}

int yflash_EraseBlockInNAND(yaffs_Device *dev, int blockNumber)
{
	
	CheckInit(dev);

	
	if(blockNumber < dev->startBlock|| blockNumber >  dev->endBlock)
	{
		T(YAFFS_TRACE_ALWAYS,("Attempt to erase non-existant block %d\n",blockNumber));
		return YAFFS_FAIL;
	}
	else
	{
		Erase_Block(blockNumber);
		return YAFFS_OK;
	}
	
}

int yflash_InitialiseNAND(yaffs_Device *dev)
{
	dev->useNANDECC = 1; // force on useNANDECC which gets faked. 
						 // This saves us doing ECC checks.
	
	return YAFFS_OK;
}


