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
// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//  
// -----------------------------------------------------------------------------
#include <windows.h>
#include <bsp.h>
#include <kitl_cfg.h>

OAL_KITL_DEVICE g_kitlDevice;

const OAL_KITL_SERIAL_DRIVER *GetKitlSerialDriver (void);
const OAL_KITL_SERIAL_DRIVER *GetKitlUSBSerialDriver (void);
static OAL_KITL_ETH_DRIVER g_kitlEthCS8900A = OAL_ETHDRV_CS8900A;

volatile S3C2440A_MEMCTRL_REG *g_pMEMCTRLReg;
volatile S3C2440A_IOPORT_REG *g_pIOPortReg;

BOOL USBSerKitl_POLL = FALSE;

BOOL InitKitlEtherArgs (OAL_KITL_ARGS *pKitlArgs)
{
   	// init flags
   	pKitlArgs->flags = OAL_KITL_FLAGS_ENABLED | OAL_KITL_FLAGS_VMINI;
#ifdef CS8900A_KITL_POLLMODE
   	pKitlArgs->flags |= OAL_KITL_FLAGS_POLL;
#endif //CS8900A_KITL_POLLMODE
#ifdef CS8900A_KITL_DHCP
    pKitlArgs->flags |= OAL_KITL_FLAGS_DHCP;
#endif //CS8900A_KITL_DHCP

    pKitlArgs->devLoc.IfcType    	= Internal;
   	pKitlArgs->devLoc.BusNumber    	= 0;
   	pKitlArgs->devLoc.LogicalLoc    = BSP_BASE_REG_PA_CS8900A_IOBASE;  // base address
   	pKitlArgs->devLoc.Pin           = 0;
    
   	OALKitlStringToMAC(CS8900A_MAC,pKitlArgs->mac);

#ifndef CS8900A_KITL_DHCP
   	pKitlArgs->ipAddress            = OALKitlStringToIP(CS8900A_IP_ADDRESS);
   	pKitlArgs->ipMask            	= OALKitlStringToIP(CS8900A_IP_MASK);
   	pKitlArgs->ipRoute            	= OALKitlStringToIP(CS8900A_IP_ROUTER);
#endif CS8900A_KITL_DHCP

	g_kitlDevice.ifcType			= Internal;
   	g_kitlDevice.type               = OAL_KITL_TYPE_ETH;
   	g_kitlDevice.pDriver            = (void *)&g_kitlEthCS8900A;

	//configure nCS3 for cs8900a
   	g_pMEMCTRLReg = (S3C2440A_MEMCTRL_REG *)OALPAtoVA(S3C2440A_BASE_REG_PA_MEMCTRL, FALSE);
	g_pMEMCTRLReg->BWSCON = (g_pMEMCTRLReg->BWSCON & ~(0xf<<12)) | (0xd<<12);

	//setting EINT9 as IRQ_LAN
	if (!(pKitlArgs->flags & OAL_KITL_FLAGS_POLL))
	{
    	g_pIOPortReg = (S3C2440A_IOPORT_REG *)OALPAtoVA(S3C2440A_BASE_REG_PA_IOPORT, FALSE);
		g_pIOPortReg->GPGCON = (g_pIOPortReg->GPGCON & ~(0x3<<2)) | (0x2<<2);
		g_pIOPortReg->GPGUP |= 0x1<<1;
		g_pIOPortReg->EXTINT1 = (g_pIOPortReg->EXTINT1 & ~(0x7<<4)) | (0x1<<4);
	}
    return TRUE;
}

BOOL InitKitlSerialArgs (OAL_KITL_ARGS *pKitlArgs)
{
	DWORD dwIoBase = UART_Kitl;

    // init flags
    pKitlArgs->flags = OAL_KITL_FLAGS_ENABLED | OAL_KITL_FLAGS_POLL;

    pKitlArgs->devLoc.LogicalLoc    = dwIoBase;
    pKitlArgs->devLoc.Pin           = OAL_INTR_IRQ_UNDEFINED;
    pKitlArgs->baudRate             = CBR_115200;
    pKitlArgs->dataBits             = DATABITS_8;
    pKitlArgs->parity               = PARITY_NONE;
    pKitlArgs->stopBits             = STOPBITS_10;

    g_kitlDevice.type               = OAL_KITL_TYPE_SERIAL;
    g_kitlDevice.pDriver            = (VOID*) GetKitlSerialDriver ();
    
    return TRUE;
}

BOOL InitKitlUSBSerialArgs (OAL_KITL_ARGS *pKitlArgs)
{
	DWORD dwIoBase = S3C2440A_BASE_REG_PA_USBD + 0x140;

    // init flags
    pKitlArgs->flags = OAL_KITL_FLAGS_ENABLED;
#ifdef USBSER_KITL_POLL
    pKitlArgs->flags |= OAL_KITL_FLAGS_POLL;
    USBSerKitl_POLL = TRUE;
#endif

    pKitlArgs->devLoc.LogicalLoc    = dwIoBase;
    pKitlArgs->devLoc.Pin           = IRQ_USBD;
 
    g_kitlDevice.type               = OAL_KITL_TYPE_SERIAL;
    g_kitlDevice.pDriver            = (VOID*) GetKitlUSBSerialDriver ();
    
    return TRUE;
}

//------------------------------------------------------------------------------
//
// Platform entry point for KITL. Called when KITLIoctl (IOCTL_KITL_STARTUP, ...) is called.
//

BOOL OEMKitlStartup (void)
{
    OAL_KITL_ARGS   kitlArgs, *pArgs;
    BOOL            fRet = FALSE;
    UCHAR			*szDeviceId,buffer[OAL_KITL_ID_SIZE]="\0";

    OALMSG(OAL_KITL&&OAL_FUNC, (L"+OEMKitlStartup\r\n"));
    
    memset (&kitlArgs, 0, sizeof (kitlArgs));

    pArgs = (OAL_KITL_ARGS *)OALArgsQuery(OAL_ARGS_QUERY_KITL);
    szDeviceId = (CHAR*)OALArgsQuery(OAL_ARGS_QUERY_DEVID);

    // common parts
   // kitlArgs.devLoc.IfcType = g_kitlDevice.ifcType
   //                         = InterfaceTypeUndefined;
   // g_kitlDevice.name       = g_oalIoCtlPlatformType;


    
#ifdef KITL_SERIAL
    KITLOutputDebugString ("SERIAL\n");
	RETAILMSG(1,(TEXT("KITL_SERIAL!!\r\n")));
	fRet = InitKitlSerialArgs (&kitlArgs);
	strcpy(buffer,"2440SerKitl");
	szDeviceId = buffer;
    g_kitlDevice.id = kitlArgs.devLoc.LogicalLoc;
#endif //KITL_SERIAL
#ifdef KITL_USBSERIAL
    KITLOutputDebugString ("USB SERIAL\n");
	RETAILMSG(1,(TEXT("SERIAL!!\r\n")));
	fRet = InitKitlUSBSerialArgs (&kitlArgs);
	strcpy(buffer,"2440USBSerKitl");
	szDeviceId = buffer;
    g_kitlDevice.id = kitlArgs.devLoc.LogicalLoc;
#endif //KITL_USBSERIAL
#ifdef KITL_ETHERNET
    KITLOutputDebugString ("ETHERNET\n");
	RETAILMSG(1,(TEXT("KITL_ETHETNET!!\r\n")));
	if (pArgs->devLoc.LogicalLoc == 0)
	{	
		KITLOutputDebugString ("pArgs = NULL\n");
		fRet = InitKitlEtherArgs (&kitlArgs);
    	OALKitlCreateName(BSP_DEVICE_PREFIX, kitlArgs.mac, buffer);
		szDeviceId = buffer;
    	g_kitlDevice.id = kitlArgs.devLoc.LogicalLoc;
    }
    else
    {
    	KITLOutputDebugString ("Kitl args bring from argument setting of RAM\n");
		g_kitlDevice.ifcType			= Internal;
    	g_kitlDevice.id    				= BSP_BASE_REG_PA_CS8900A_IOBASE;  // base address
    	g_kitlDevice.type               = OAL_KITL_TYPE_ETH;
    	g_kitlDevice.pDriver            = (void *)&g_kitlEthCS8900A;

		memcpy(&kitlArgs, pArgs, sizeof (kitlArgs));
		OALKitlCreateName(BSP_DEVICE_PREFIX, kitlArgs.mac, buffer);
		szDeviceId = buffer;

		fRet = TRUE;
    }
    RETAILMSG(KITL_DBON, (
        L"DeviceId................. %hs\r\n", szDeviceId
    ));
    RETAILMSG(KITL_DBON, (
        L"kitlArgs.flags............. 0x%x\r\n", kitlArgs.flags
    ));
    RETAILMSG(KITL_DBON, (
        L"kitlArgs.devLoc.IfcType.... %d\r\n",   kitlArgs.devLoc.IfcType
    ));
    RETAILMSG(KITL_DBON, (
        L"kitlArgs.devLoc.LogicalLoc. 0x%x\r\n", kitlArgs.devLoc.LogicalLoc
    ));
    RETAILMSG(KITL_DBON, (
        L"kitlArgs.devLoc.PhysicalLoc 0x%x\r\n", kitlArgs.devLoc.PhysicalLoc
    ));
    RETAILMSG(KITL_DBON, (
        L"kitlArgs.devLoc.Pin........ %d\r\n",   kitlArgs.devLoc.Pin
    ));
    RETAILMSG(KITL_DBON, (
        L"kitlArgs.ip4address........ %s\r\n",   OALKitlIPtoString(kitlArgs.ipAddress)
    ));
    
#endif //KITL_ETHERNET

	if (fRet == FALSE)
	{
	    KITLOutputDebugString ("NONE\n");
		RETAILMSG(1, (TEXT("KITL Argument Setting INIT fail.\n")));
		return FALSE;
	}
	
	if ((kitlArgs.flags & OAL_KITL_FLAGS_ENABLED) == 0)
	RETAILMSG(1, (TEXT("KITL is disabled.\r\n")));
	else
	RETAILMSG(1, (TEXT("KITL is enabeld.\r\n")));
	KITLOutputDebugString ("Call OALKitlInit().\n");
	fRet = OALKitlInit (szDeviceId, &kitlArgs, &g_kitlDevice);
    return fRet;
}

DWORD OEMKitlGetSecs (void)
{
    SYSTEMTIME st;
    DWORD dwRet;
    static DWORD dwBias;
    static DWORD dwLastTime;

    OEMGetRealTime( &st );
    dwRet = ((60UL * (60UL * (24UL * (31UL * st.wMonth + st.wDay) + st.wHour) + st.wMinute)) + st.wSecond);
    dwBias = dwRet;

    if (dwRet < dwLastTime) {
        KITLOutputDebugString("! Time went backwards (or wrapped): cur: %u, last %u\n",
                              dwRet,dwLastTime);
    }
    dwLastTime = dwRet;
    return (dwRet);
}


//------------------------------------------------------------------------------
//
//  Function:  OALGetTickCount
//
//  This function is called by some KITL libraries to obtain relative time
//  since device boot. It is mostly used to implement timeout in network
//  protocol.
//

UINT32 OALGetTickCount()
{
    return OEMKitlGetSecs () * 1000;
}


