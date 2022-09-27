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
//  Header: s3c2440a_clkpwr.h
//
//  Defines the clock and power register layout and definitions.
//
#ifndef __S3C2440A_CLKPWR_H
#define __S3C2440A_CLKPWR_H

#if __cplusplus
    extern "C" 
    {
#endif


//------------------------------------------------------------------------------
//  Type: S3C2440A_CLKPWR_REG
//
//  Clock and Power Management registers.
//

typedef struct 
{
    UINT32   LOCKTIME;               // PLL lock time count register
    UINT32   MPLLCON;                // MPLL configuration register
    UINT32   UPLLCON;                // UPLL configuration register
    UINT32   CLKCON;                 // clock generator control register
    UINT32   CLKSLOW;                // slow clock control register
    UINT32   CLKDIVN;                // clock divider control register
    UINT32   CAMDIVN;                // camera clock divider register

} S3C2440A_CLKPWR_REG, *PS3C2440A_CLKPWR_REG;


#if __cplusplus
    }
#endif

#endif 
