//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
#ifndef __NAND_H__
#define __NAND_H__

#define LARGE_BLOCK_NAND		0
#define SMALL_BLOCK_NAND		1
#define USE_NFCE			0
#define USE_GPIO			1

#define LB_BLOCK_LOOP			1
#define SB_BLOCK_LOOP			8

#define SECTOR_SIZE			512
#define SPARE_SIZE			16
#define PAGES_PER_BLOCK			256						// Phisical 64 * logical 4

#define NAND_PAGE_CNT			PAGES_PER_BLOCK		/* Each Block has 32 Pages      */
#define NAND_PAGE_SIZE			SECTOR_SIZE			/* Each Page has 512 Bytes      */
#define NAND_BLOCK_SIZE			(NAND_PAGE_CNT * NAND_PAGE_SIZE)

#endif    // __NAND_H_.

