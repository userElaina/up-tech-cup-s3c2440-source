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
//  Header: s3c2440a_dma.h
//
//  Defines the DMA controller CPU register layout and definitions.
//
#ifndef __S3C2440A_DMA_H
#define __S3C2440A_DMA_H

#if __cplusplus
    extern "C" 
    {
#endif


//------------------------------------------------------------------------------
//  Type: S3C2440A_DMA_REG    
//
//  DMA control registers. This register bank is located by the constant 
//  CPU_BASE_REG_DMA in the configuration file cpu_base_reg_cfg.h.
//

typedef struct 
{
    // DMA 0

    UINT32    DISRC0;         // initial source reg               - 0x0
    UINT32    DISRCC0;        // initial source control reg       - 0x04
    UINT32    DIDST0;         // initial destination reg          - 0x08
    UINT32    DIDSTC0;        // initial destination control reg  - 0x0C
    UINT32    DCON0;          // control reg                      - 0x10
    UINT32    DSTAT0;         // count reg                        - 0x14
    UINT32    DCSRC0;         // current source reg               - 0x18
    UINT32    DCDST0;         // current destination reg          - 0x1C
    UINT32    DMASKTRIG0;     // mask trigger reg                 - 0x20
    UINT32    PAD1[7];        // pad                              - 0x24 - 0x3C

    // DMA 1                           offset

    UINT32    DISRC1;         // 0x40
    UINT32    DISRCC1;        // 0x44
    UINT32    DIDST1;         // 0x48
    UINT32    DIDSTC1;        // 0x4C
    UINT32    DCON1;          // 0x50
    UINT32    DSTAT1;         // 0x54
    UINT32    DCSRC1;         // 0x58
    UINT32    DCDST1;         // 0x5C
    UINT32    DMASKTRIG1;     // 0x60
    UINT32    PAD2[7];        // 0x64 - 0x7C

    // DMA 2

    UINT32    DISRC2;         // 0x80
    UINT32    DISRCC2;        // 0x84
    UINT32    DIDST2;         // 0x88
    UINT32    DIDSTC2;        // 0x8C
    UINT32    DCON2;          // 0x90
    UINT32    DSTAT2;         // 0x94
    UINT32    DCSRC2;         // 0x98
    UINT32    DCDST2;         // 0x9C
    UINT32    DMASKTRIG2;     // 0xA0
    UINT32    PAD3[7];        // 0xA4 - 0xBC

    // DMA 3

    UINT32    DISRC3;         // 0xC0
    UINT32    DISRCC3;        // 0xC4
    UINT32    DIDST3;         // 0xC8
    UINT32    DIDSTC3;        // 0xCC
    UINT32    DCON3;          // 0xD0
    UINT32    DSTAT3;         // 0xD4
    UINT32    DCSRC3;         // 0xD8
    UINT32    DCDST3;         // 0xDC
    UINT32    DMASKTRIG3;     // 0xE0

} S3C2440A_DMA_REG, *S3C2440A_PDMA_REG;


#if __cplusplus
    }
#endif

#endif 
