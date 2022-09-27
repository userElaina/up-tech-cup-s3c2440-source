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
//------------------------------------------------------------------------------
//
//  File:  image_cfg.h
//
//  Defines configuration parameters used to create the NK and Bootloader
//  program images.
//
#ifndef __IMAGE_CFG_H
#define __IMAGE_CFG_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//  RESTRICTION
//
//  This file is a configuration file. It should ONLY contain simple #define 
//  directives defining constants. This file is included by other files that 
//  only support simple substitutions.
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  NAMING CONVENTION
//
//  The IMAGE_ naming convention ...
//
//  IMAGE_<NAME>_<SECTION>_<MEMORY_DEVICE>_[OFFSET|SIZE|START|END]
//
//      <NAME>          - WINCE, BOOT, SHARE
//      <SECTION>       - section name: user defined
//      <MEMORY_DEVICE> - the memory device the block resides on
//      OFFSET          - number of bytes from memory device start address
//      SIZE            - maximum size of the block
//      START           - start address of block    (device address + offset)
//      END             - end address of block      (start address  + size - 1)
//
//------------------------------------------------------------------------------

#define IMAGE_SHARE_ARGS_UA_START		0xA0020000
#define IMAGE_SHARE_ARGS_CA_START		0x80020000
#define IMAGE_SHARE_ARGS_SIZE			0x00000800

//------------------------------------------------------------------------------

#define IMAGE_FRAMEBUFFER_UA_BASE		0xA0060000
#define IMAGE_FRAMEBUFFER_DMA_BASE		0x30060000
#define IMAGE_FRAMEBUFFER_UA_BASE_eboot		0xA0101000
#define IMAGE_FRAMEBUFFER_DMA_BASE_eboot	0x30101000


//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif 
