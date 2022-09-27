/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system.
 * yaffscfg.c  The configuration for the "direct" use of yaffs.
 *
 * This file is intended to be modified to your requirements.
 * There is no need to redistribute this file.
 */
#include "../inc/drivers.h"
#include "../inc/sys/lib.h"
#include "bon.h"
#include <includes.h>

#include "yaffscfg.h"
#include "yaffsfs.h"
#include <errno.h>

#define SYSDEV_SIZE		(6*1024*1024)
#define USERDEV_SIZE		(8*1024*1024)

#define nandBLOCK_SIZE	(16*1024)

unsigned yaffs_traceMask = YAFFS_TRACE_BAD_BLOCKS|YAFFS_TRACE_ALWAYS|YAFFS_TRACE_BUG;

void yaffsfs_SetError(int err)
{
	//Do whatever to set error
	errno = err;
}

__u32 yaffsfs_CurrentTime(void)
{
	return 0;
}

#if defined(__UCOSII__)

static OS_EVENT *yaffs_Sem;

void yaffsfs_Lock(void)
{
	INT8U err;
	OSSemPend(yaffs_Sem,0,&err);
}

void yaffsfs_Unlock(void)
{
	OSSemPost(yaffs_Sem);
}

void yaffsfs_LocalInitialisation(void)
{
	// Define locking semaphore.

	yaffs_Sem=OSSemCreate(1);
}

#elif defined(__ADSONLY__)

void yaffsfs_Lock(void)
{
}

void yaffsfs_Unlock(void)
{
}

void yaffsfs_LocalInitialisation(void)
{
	// Define locking semaphore.
}
#endif

// Configuration for:
// /ram  2MB ramdisk
// /boot 2MB boot disk (flash)
// /flash 14MB flash disk (flash)
// NB Though /boot and /flash occupy the same physical device they
// are still disticnt "yaffs_Devices. You may think of these as "partitions"
// using non-overlapping areas in the same device.
// 

//#include "yaffs_ramdisk.h"
#include "yaffs_flashif.h"

static yaffs_Device sysDev;

static yaffsfs_DeviceConfiguration yaffsfs_config[] = {

	{ "/sys", &sysDev},
	{(void *)0,(void *)0}
};


int yaffs_StartUp(void)
{
	int i=0;
/*	for(i=128;i<1024;i++){
		Erase_Cluster(i);
	}*/

	NandFlash_init();

	// Stuff to configure YAFFS
	// Stuff to initialise anything special (eg lock semaphore).
	yaffsfs_LocalInitialisation();

	if(read_bon_partition())
		return -1;

	for(i=0; i< bon_part.num_part; i++){
		if(bon_part.parts[i].flag == BON_YAFFS_PART){
			sysDev.startBlock = bon_part.parts[i].offset/nand_flashinfo.blocksize;
			sysDev.endBlock= sysDev.startBlock
				+bon_part.parts[i].size/nand_flashinfo.blocksize;

			printk("find a yaffs parttion: offset=0x%lx, size=0x%lx\n", 
				bon_part.parts[i].offset, bon_part.parts[i].size);
			break;
		}
	}

	if( i == bon_part.num_part ){
		printk("No yaffs partition found!\n");
		return -1;
	}
	
	// Set up devices

		// /sysDev
	sysDev.nBytesPerChunk = YAFFS_BYTES_PER_CHUNK;
	sysDev.nChunksPerBlock = YAFFS_CHUNKS_PER_BLOCK;
	sysDev.nReservedBlocks = 5;
	sysDev.useNANDECC = 1; // use YAFFS's ECC
	sysDev.nShortOpCaches = 10; // Use caches
	sysDev.genericDevice = (void *) 2;	// Used to identify the device in fstat.
	sysDev.writeChunkToNAND = yflash_WriteChunkToNAND;
	sysDev.readChunkFromNAND = yflash_ReadChunkFromNAND;
	sysDev.eraseBlockInNAND = yflash_EraseBlockInNAND;
	sysDev.initialiseNAND = yflash_InitialiseNAND;

#if 0
		// /userDev
	userDev.nBytesPerChunk = YAFFS_BYTES_PER_CHUNK;
	userDev.nChunksPerBlock = YAFFS_CHUNKS_PER_BLOCK;
	userDev.nReservedBlocks = 5;
	userDev.startBlock = block; // First block 
	block+=(USERDEV_SIZE/nandBLOCK_SIZE-1);
	userDev.endBlock = block; // Last block 
	userDev.useNANDECC = 1; // use YAFFS's ECC
	userDev.nShortOpCaches = 10; // Use caches
	userDev.genericDevice = (void *) 2;	// Used to identify the device in fstat.
	userDev.writeChunkToNAND = yflash_WriteChunkToNAND;
	userDev.readChunkFromNAND = yflash_ReadChunkFromNAND;
	userDev.eraseBlockInNAND = yflash_EraseBlockInNAND;
	userDev.initialiseNAND = yflash_InitialiseNAND;
#endif

	yaffs_initialise(yaffsfs_config);

	i=0;
	while(yaffsfs_config[i].prefix)
		yaffs_mount(yaffsfs_config[i++].prefix);
	
	return 0;
}




