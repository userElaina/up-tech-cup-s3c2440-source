/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2005.3.9
\***************************************************************************/
/***************************************************************************\
    #说明: bon part
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2005-3-9	创建

\***************************************************************************/
#include "bon.h"
#include "../inc/drivers.h"
#include "../inc/sys/lib.h"
#include <stdio.h>

bon_part_info bon_part;

static const char BON_MAGIC[8] = {'M', 0, 0, 'I', 0, 'Z', 'I', 0};

int read_bon_partition(void)
{
	int i;
	unsigned char oobbuf[NandSparePageSize];
	char buf[NandPageSize];
	unsigned int *s;
	unsigned int offset = PARTITION_OFFSET;
	int ret;

	int block, page;

	if (offset > nand_flashinfo.totalsize - nand_flashinfo.blocksize)
		offset = nand_flashinfo.totalsize - nand_flashinfo.blocksize;

	block = offset/nand_flashinfo.blocksize;
	page = offset %nand_flashinfo.blocksize;

	ret = ReadSparePage(block, page, oobbuf);
	if (ret || oobbuf[5] != 0xFF){
		printk("bon partition in bad block!\n");
		return -2;
	}
	
	if (ReadPage(block,page,(unsigned char*)buf))
	   	return -3;

	if (strncmp(buf, BON_MAGIC, sizeof(BON_MAGIC)) != 0){
		printk("Can't find bon partition!\n");
		return -1;
	}

	s = (unsigned int *)(buf + 8);
	bon_part.num_part = *s++;

	for (i = 0; i < bon_part.num_part; i++) {
		bon_part.parts[i].offset = *s++;
		bon_part.parts[i].size = *s++;
		bon_part.parts[i].flag = *s++;

		//check partition info
		if(bon_part.parts[i].size + bon_part.parts[i].offset 
			>= nand_flashinfo.totalsize){
			printk("bon partition info error\n");
			return -1;
		}
	}

//we don't need to check bad block
/*	for (i = 0; i < bon_part.num_part; i++) {
		bon_part.parts[i].num_bad_block = *s++;
		if (bon_part.parts[i].num_bad_block) {
			bon_part.parts[i].bad_blocks = mmalloc(parts[i].num_bad_block * sizeof(unsigned int));
			for (k = 0; k < parts[i].num_bad_block;k++) {
				parts[i].bad_blocks[k] = *s++;
			}
		} else {
			parts[i].bad_blocks = 0;
		}
	}*/

	return 0;
}

