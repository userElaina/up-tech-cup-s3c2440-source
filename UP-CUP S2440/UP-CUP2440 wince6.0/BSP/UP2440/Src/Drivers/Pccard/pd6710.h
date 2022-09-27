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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 2002. Samsung Electronics, co. ltd  All rights reserved.

Module Name:  

Abstract:

Notes: 
--*/

#ifndef __PD6710_H__
#define __PD6710_H__

#define B6710_Tacs	(0x3)	// 4clk
#define B6710_Tcos	(0x3)	// 4clk
#define B6710_Tacc	(0x7)	// 14clk
#define B6710_Tcoh	(0x3)	// 1clk
#define B6710_Tah	(0x3)	// 4clk
#define B6710_Tacp	(0x3)	// 6clk
#define B6710_PMC	(0x3)	// normal(1data)

#define PD6710_MEM_BASE_ADDRESS     (0xA4000000)	//(0x04000000)//nGCS2 0x04000000
#define PD6710_IO_BASE_ADDRESS      (0xA5000000)	//(0x05000000)//      0x05000000


#endif /*__PD6710_H__*/
