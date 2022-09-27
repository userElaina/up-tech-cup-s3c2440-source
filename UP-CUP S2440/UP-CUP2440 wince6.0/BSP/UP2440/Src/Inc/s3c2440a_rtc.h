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
//  Header: s3c2440a_rtc.h
//
//  Defines the Real Time Clock (RTC) register layout and associated 
//  types and constants.
//
#ifndef __S3C2440A_RTC_H__
#define __S3C2440A_RTC_H__

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type: S3C2440A_RTC_REG    
//
//  RTC control registers. This register bank is located by the constant 
//  S3C2440A_BASE_REG_XX_RTC in the configuration file s3c2440a_base_reg_cfg.h.
//

typedef struct {
    UINT32 PAD1[16];                // pad to - 0x3C
    UINT32 RTCCON;                  // control reg
    UINT32 TICNT;                   // tick time count reg
    UINT32 PAD2[2];                 // pad
    UINT32 RTCALM;                  // alarm control reg
    UINT32 ALMSEC;                  // alarm sec data reg
    UINT32 ALMMIN;                  // alarm min data reg
    UINT32 ALMHOUR;                 // alarm hour data reg
    UINT32 ALMDATE;                 // alarm date (day) data reg
    UINT32 ALMMON;                  // alarm month data reg
    UINT32 ALMYEAR;                 // alarm year data reg
    UINT32 RTCLBAT;
    UINT32 BCDSEC;                  // BCD values...
    UINT32 BCDMIN;             
    UINT32 BCDHOUR;
    UINT32 BCDDAY;
    UINT32 BCDDATE;
    UINT32 BCDMON;
    UINT32 BCDYEAR;

} S3C2440A_RTC_REG, *PS3C2440A_RTC_REG;    

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif 
