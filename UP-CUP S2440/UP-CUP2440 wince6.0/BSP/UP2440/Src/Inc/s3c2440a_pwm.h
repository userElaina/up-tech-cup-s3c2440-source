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
//  Header: s3c2440a_pwm.h
//
//  Defines the PWM Timer register layout and associated types and constants.
//
#ifndef __S3C2440A_PWM_H
#define __S3C2440A_PWM_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type:  S3C2440A_PWM_REG
//
//  Defines the PWM Timer control register layout. This register bank is 
//  located by the constant S3C2440A_BASE_REG_XX_PWM in the configuration file
//  s3c2440a_base_reg_cfg.h.
//
typedef struct  {
    UINT32 TCFG0;
    UINT32 TCFG1;
    UINT32 TCON;
    UINT32 TCNTB0;
    UINT32 TCMPB0;
    UINT32 TCNTO0;
    UINT32 TCNTB1;
    UINT32 TCMPB1;
    UINT32 TCNTO1;
    UINT32 TCNTB2;
    UINT32 TCMPB2;
    UINT32 TCNTO2;
    UINT32 TCNTB3;
    UINT32 TCMPB3;
    UINT32 TCNTO3;
    UINT32 TCNTB4;
    UINT32 TCNTO4;

} S3C2440A_PWM_REG, *PS3C2440A_PWM_REG;

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif 
