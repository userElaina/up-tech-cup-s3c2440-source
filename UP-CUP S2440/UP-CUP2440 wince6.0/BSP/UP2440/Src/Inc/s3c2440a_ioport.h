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
//  Header:  s3c2440a_ioport.h
//
//  Defines the Input/Output Ports (IOPORT) control registers and associated
//  types and constants.
//
#ifndef __S3C2440A_IOPORT_H
#define __S3C2440A_IOPORT_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

typedef struct {
    UINT32 GPACON;                  // Port A - offset 0
    UINT32 GPADAT;                  // Data
    UINT32 PAD1[2];

    UINT32 GPBCON;                  // Port B - offset 0x10
    UINT32 GPBDAT;                  // Data
    UINT32 GPBUP;                   // Pull-up disable
    UINT32 PAD2;

    UINT32 GPCCON;                  // Port C - offset 0x20
    UINT32 GPCDAT;                  // Data
    UINT32 GPCUP;                   // Pull-up disable
    UINT32 PAD3;
    
    UINT32 GPDCON;                  // Port D - offset 0x30
    UINT32 GPDDAT;                  // Data
    UINT32 GPDUP;                   // Pull-up disable
    UINT32 PAD4;
    
    UINT32 GPECON;                  // Port E - offset 0x40
    UINT32 GPEDAT;                  // Data
    UINT32 GPEUP;                   // Pull-up disable
    UINT32 PAD5;                 
    
    UINT32 GPFCON;                  // Port F - offset 0x50
    UINT32 GPFDAT;
    UINT32 GPFUP; 
    UINT32 PAD6;
    
    UINT32 GPGCON;                  // Port G - offset 0x60
    UINT32 GPGDAT;
    UINT32 GPGUP; 
    UINT32 PAD7;
    
    UINT32 GPHCON;                  // Port H - offset 0x70
    UINT32 GPHDAT;
    UINT32 GPHUP; 
    UINT32 PAD8;

    UINT32 MISCCR;                  // misc control reg - offset 0x80
    UINT32 DCLKCON;                 // DCLK0/1 control reg
    
    UINT32 EXTINT0;                 // external interrupt control reg 0
    UINT32 EXTINT1;                 // external interrupt control reg 1
    UINT32 EXTINT2;                 // external interrupt control reg 2
    
    UINT32 EINTFLT0;                // reserved
    UINT32 EINTFLT1;                // reserved
    UINT32 EINTFLT2;                // external interrupt filter reg 2
    UINT32 EINTFLT3;                // external interrupt filter reg 3

    UINT32 EINTMASK;                // external interrupt mask reg
    UINT32 EINTPEND;                // external interrupt pending reg

    UINT32 GSTATUS0;                // external pin status
    UINT32 GSTATUS1;                // chip ID
    UINT32 GSTATUS2;                // reset status
    UINT32 GSTATUS3;                // inform register
    UINT32 GSTATUS4;                // inform register

    UINT32 FLTOUT;					// C0
    UINT32 DSC0;
    UINT32 DSC1;
    UINT32 MSLCON;

    UINT32 GPJCON;					// D0
    UINT32 GPJDAT;
    UINT32 GPJUP;
    UINT32 PDA9;
    
} S3C2440A_IOPORT_REG, *PS3C2440A_IOPORT_REG;  

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif // __S3C2440A_IOPORT_H
