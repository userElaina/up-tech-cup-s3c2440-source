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
//  Header: s3c2440a.h
//
//  This header file defines the S3C2440A processor.
//
//  The s3C2440A is a System on Chip (SoC) part consisting of an ARM920T core. 
//  This header file is comprised of component header files that define the 
//  register layout of each component.
//  
//------------------------------------------------------------------------------
#ifndef __S3C2440A_H
#define __S3C2440A_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

// Base Definitions
#include "s3c2440a_base_regs.h"

// SoC Components
#include "s3c2440a_adc.h"
#include "s3c2440a_clkpwr.h"
#include "s3c2440a_dma.h"
#include "s3c2440a_iicbus.h"
#include "s3c2440a_iisbus.h"
#include "s3c2440a_intr.h"
#include "s3c2440a_ioport.h"
#include "s3c2440a_lcd.h"
#include "s3c2440a_memctrl.h"
#include "s3c2440a_nand.h"
#include "s3c2440a_pwm.h"
#include "s3c2440a_rtc.h"
#include "s3c2440a_sdi.h"
#include "s3c2440a_spi.h"
#include "s3c2440a_uart.h"
#include "s3c2440a_usbd.h"
#include "s3c2440a_wdog.h"
#include "s3c2440a_cam.h"
//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif 
