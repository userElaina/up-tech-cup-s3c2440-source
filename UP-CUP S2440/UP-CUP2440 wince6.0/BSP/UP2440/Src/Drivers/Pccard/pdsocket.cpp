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

Module Name:  

Abstract:

    Platform dependent PCMCIA initialization functions

Notes: 
--*/
#include <windows.h>
#include <types.h>
#include <socksv2.h>
#include <memory.h>
#include <ceddk.h>
#include <nkintr.h>

#include "PDSocket.h"
//
// @doc DRIVERS
//


#define REG_CHIP_REVISION                    0x00
#define REG_INTERFACE_STATUS                 0x01
#define REG_POWER_CONTROL                    0x02
#define REG_INTERRUPT_AND_GENERAL_CONTROL    0x03
#define REG_CARD_STATUS_CHANGE               0x04
#define REG_STATUS_CHANGE_INT_CONFIG         0x05
#define REG_GENERAL_CONTROL					 0x16
#define REG_GLOBAL_CONTROL                   0x1E
#define REG_FIFO_CTRL						 0x17

//
// Interface status register 0x01
//
#define STS_BVD1                             0x01
#define STS_BVD2                             0x02
#define STS_CD1                              0x04
#define STS_CD2                              0x08
#define STS_WRITE_PROTECT                    0x10
#define STS_CARD_READY                       0x20
#define STS_CARD_POWER_ON                    0x40
#define STS_GPI                              0x80

//
// Power and RESETDRV control register 0x02
//
#define PWR_VPP1_BIT0							0x01
#define PWR_VPP1_BIT1							0x02
#define PWR_VPP2_BIT0							0x04
#define PWR_VPP2_BIT1							0x08
#define PWR_VCC_POWER							0x10
#define PWR_AUTO_POWER 							0x20
#define PWR_RESUME_RESET						0x40
#define PWR_OUTPUT_ENABLE 						0x80

//
// Interrupt and general control register 0x03
//
#define INT_IRQ_BIT0                         0x01
#define INT_IRQ_BIT1                         0x02
#define INT_IRQ_BIT2                         0x04
#define INT_IRQ_BIT3                         0x08
#define INT_ENABLE_MANAGE_INT                0x10
#define INT_CARD_IS_IO                       0x20
#define INT_CARD_NOT_RESET                   0x40
#define INT_RING_INDICATE_ENABLE             0x80

//
// Card Status change register 0x04
//
#define CSC_BATTERY_DEAD_OR_STS_CHG          0x01
#define CSC_BATTERY_WARNING                  0x02
#define CSC_READY_CHANGE                     0x04
#define CSC_DETECT_CHANGE                    0x08
#define CSC_GPI_CHANGE                       0x10

//
// Card Status change interrupt configuration register 0x05
//
#define CFG_BATTERY_DEAD_ENABLE              0x01
#define CFG_BATTERY_WARNING_ENABLE           0x02
#define CFG_READY_ENABLE                     0x04
#define CFG_CARD_DETECT_ENABLE               0x08
#define CFG_MANAGEMENT_IRQ_BIT0              0x10
#define CFG_MANAGEMENT_IRQ_BIT1              0x20
#define CFG_MANAGEMENT_IRQ_BIT2              0x40
#define CFG_MANAGEMENT_IRQ_BIT3              0x80


//
// MISC Control 1 register 0x16
//
#define MISC1_5V_DETECT							0x01
#define MISC1_VCC_33							0x02
#define MISC1_PM_IRQ							0x04
#define MISC1_PS_IRQ							0x08
#define MISC1_SPK_ENABLE						0x10
#define MISC1_INPACK_ENABLE						0x80

//
// FIFO Control register 0x17
//
#define FIFO_EMPTY_WRITE						0x80

//
// MISC Control 2 register  0x1E
//
#define MISC2_BFS								0x01
#define MISC2_LOW_POWER_MODE					0x02
#define MISC2_SUSPEND							0x04
#define MISC2_5V_CORE							0x08
#define MISC2_LED_ENABLE						0x10
#define MISC2_3STATE_BIT7						0x20
#define MISC2_IRQ15_RIOUT						0x80

//
// Global control register 0x1E
//
#define GCR_POWER_DOWN                       0x01
#define GCR_LEVEL_MODE_INT                   0x02
#define GCR_WRITE_BACK_ACK                   0x04
#define GCR_PULSE_IRQ14                      0x08

// more PD6710 specific flags
#define REG_CARD_IO_MAP0_OFFSET_L 				0x36
#define REG_CARD_IO_MAP0_OFFSET_H 				0x37
#define REG_CARD_IO_MAP1_OFFSET_L 				0x38
#define REG_CARD_IO_MAP1_OFFSET_H 				0x39
#define REG_SETUP_TIMING0						0x3a
#define REG_CMD_TIMING0							0x3b
#define REG_RECOVERY_TIMING0					0x3c
#define REG_SETUP_TIMING1						0x3d
#define REG_CMD_TIMING1							0x3e
#define REG_RECOVERY_TIMING1					0x3f
#define REG_LAST_INDEX 							REG_RECOVERY_TIMING1

#define EVENT_MASK_WRITE_PROTECT    0x0001 // write protect change
#define EVENT_MASK_CARD_LOCK        0x0002 // card lock change
#define EVENT_MASK_EJECT_REQ        0x0004 // ejection request
#define EVENT_MASK_INSERT_REQ       0x0008 // insertion request
#define EVENT_MASK_BATTERY_DEAD     0x0010 // battery dead
#define EVENT_MASK_BATTERY_LOW      0x0020 // battery low
#define EVENT_MASK_CARD_READY       0x0040 // ready change
#define EVENT_MASK_CARD_DETECT      0x0080 // card detect change
#define EVENT_MASK_POWER_MGMT       0x0100 // power management change
#define EVENT_MASK_RESET            0x0200 // card resets
#define EVENT_MASK_STATUS_CHANGE    0x0400 // card generated status change interrupts

//////////////////////////////////////////////////////////////////////////

//#define PCMCIA_DRIVER_KEY TEXT("Drivers\\PCMCIA")
#define IRQ_VALUE_NAME TEXT("Irq")
#define CHIPSET_VALUE_NAME TEXT("Chipset")
#define DEVICEID_VALUE_NAME TEXT("DeviceID")
#define POLLEDDEVICES_VALUE_NAME TEXT("PolledDevices")
#define CSC_SYSINTR_VALUE_NAME TEXT("CSCSysIntr")
#define CSC_IRQ_VALUE_NAME TEXT("CSCIrq")
#define POLL_TIMEOUT_NAME TEXT("PollTimeout")
#define POLLING_MODE_NAME TEXT("PollingMode")
#define DISABLE_SOCKET_NAME TEXT("DisableSocket")

#define CHIPSET_ID_CNT 32
#define CHIPSET_ID_LEN ((CHIPSET_ID_CNT * 9) + 1)

const SS_POWER_ENTRY CPcmciaCardSocket::m_rgPowerEntries[NUM_POWER_ENTRIES] =
{
    { 0,    PWR_SUPPLY_VCC | PWR_SUPPLY_VPP1 | PWR_SUPPLY_VPP2 },
    { 33,   PWR_SUPPLY_VCC                                     },
    { 50,   PWR_SUPPLY_VCC | PWR_SUPPLY_VPP1 | PWR_SUPPLY_VPP2 },
    { 120,                   PWR_SUPPLY_VPP1 | PWR_SUPPLY_VPP2 }
};
DWORD CPcmciaCardSocket::ms_dwSocketLastIndex = 1;

CPcmciaCardSocket::CPcmciaCardSocket( CPcmciaBusBridge* pBridge )
{
    m_pBridge = pBridge;
    m_dwSocketIndex = 0;
    while( m_dwSocketIndex == 0 )
    {
        m_dwSocketIndex = ( DWORD )
                          InterlockedIncrement( ( LONG * ) &ms_dwSocketLastIndex ); 
        //Make it is it does not exist.
        CPcmciaCardSocket* pSocket = GetSocket( ( HANDLE ) m_dwSocketIndex );
        if( pSocket != NULL )
        {
            // Duplicated , Retry.
            m_dwSocketIndex = 0;
            pSocket->DeRef();
        }
    }

    DEBUGCHK( m_pBridge );
    while( m_pBridge->LockOwner( 1000 ) != TRUE )
    {
        DEBUGCHK( FALSE );
    }
    DEBUGMSG( ZONE_INIT,
              ( TEXT( "CARDBUS: CPcmciaCardSocket (Socket=%d Index=%d) Created\r\n" ),
                GetSlotNumber(),
                GetSocketHandle() ) );
}

CPcmciaCardSocket::~CPcmciaCardSocket()
{
    m_pBridge->ReleaseOwner();
    DEBUGMSG( ZONE_INIT,
              ( TEXT( "CARDBUS: CPcmciaCardSocket (Socket=%d Index=%d) Deleted\r\n" ),
                GetSlotNumber(),
                GetSocketHandle() ) );
}

STATUS CPcmciaCardSocket::GetPowerEntry( PDWORD pdwNumOfEnery,
                                         PSS_POWER_ENTRY pPowerEntry )
{
    STATUS status = CERR_BAD_ARGS;
    if( pdwNumOfEnery != NULL && pPowerEntry != NULL )
    {
        DWORD dwNumOfCopied = min( *pdwNumOfEnery, NUM_POWER_ENTRIES );
        if( dwNumOfCopied != 0 )
        {
            memcpy( pPowerEntry,
                    m_rgPowerEntries,
                    dwNumOfCopied * sizeof( SS_POWER_ENTRY ) );
            *pdwNumOfEnery = dwNumOfCopied;
            status = CERR_SUCCESS;
        }
    }
    return status;
}

UINT16 CPcmciaCardSocket::GetSocketNo()
{
    return m_pBridge->GetSocketNo( GetSlotNumber() );
};

CPcmciaBusBridge::CPcmciaBusBridge( LPCTSTR RegPath ) : CPCCardBusBridgeBase( RegPath ),
                                                        CMiniThread( 0,
                                                                     TRUE )
{
    m_fPCICCritSecInitialized = false;
    m_pCardBusResource = NULL;
    m_hISTEvent = NULL;
    m_dwSocketLowBound = 0;
    m_dwSocketHighBound = NUM_SLOTS - 1;
    for( UINT8 nSocket = m_dwSocketLowBound;
         nSocket <= m_dwSocketHighBound;
         nSocket++ )
    {
        m_rguSocketNum[nSocket] = ( WORD ) - 1;
        m_prgCardSocket[nSocket] = NULL;
        m_rgfPowerCycleEvent[nSocket] = FALSE;
        m_rgfCardInjectEvent[nSocket] = FALSE;
    }
    m_fPollingMode = TRUE;
};


CPcmciaBusBridge::~CPcmciaBusBridge()
{
    // Terminate IST
    m_bTerminated = TRUE;
    if( m_hISTEvent )
    {
        SetEvent( m_hISTEvent );
        ThreadTerminated( 1000 );
        if( !m_fPollingMode )
        {
            InterruptDisable( m_dwCSCSysIntr );
        }
        CloseHandle( m_hISTEvent );
    };

    for( UINT8 nSocket = GetSocketLowBound();
         nSocket <= GetSocketHighBound();
         nSocket++ )
    {
        if( m_rguSocketNum[nSocket] != ( UINT16 ) - 1 )
        {
            GetSocketNumberFromCS( nSocket, FALSE );
            m_rguSocketNum[nSocket] = ( UINT16 ) - 1;
        }

        RemovePcmciaCardSocket( nSocket );
    }
    if( m_pCardBusResource != NULL )
    {
        delete  m_pCardBusResource;
    }
    if( m_fPCICCritSecInitialized )
    {
        DeleteCriticalSection( &m_PCICCritSec );
    }
}

BOOL CPcmciaBusBridge::InstallIsr()
{
    UINT8 tmp;

    for( UINT8 nSocket = GetSocketLowBound();
         nSocket <= GetSocketHighBound();
         nSocket++ )
    {
        //
        // Disable interrupts
        //
        WritePCICRegister( nSocket, REG_INTERRUPT_AND_GENERAL_CONTROL, 0 );

	    // Management int -> edge triggering(PULSE), System int -> LEVEL triggering 
        WritePCICRegister( nSocket, REG_GENERAL_CONTROL, MISC1_VCC_33|MISC1_PM_IRQ|MISC1_SPK_ENABLE );
        UINT8 bPat = ReadPCICRegister( nSocket, REG_GENERAL_CONTROL );
	    // 25Mhz_bypass,low_power_dynamic,IRQ12=drive_LED
	    WritePCICRegister( nSocket, REG_GLOBAL_CONTROL, MISC2_LOW_POWER_MODE|MISC2_LED_ENABLE);

	    // before configuring timing register, FIFO should be cleared.
	    WritePCICRegister( nSocket, REG_FIFO_CTRL, FIFO_EMPTY_WRITE);    //Flush FIFO

	    //default access time is 300ns
	    WritePCICRegister( nSocket, REG_SETUP_TIMING0, 5);                   //80ns(no spec)
	    WritePCICRegister( nSocket, REG_CMD_TIMING0, 20);                  //320ns(by spec,25Mhz clock)
	    WritePCICRegister( nSocket, REG_RECOVERY_TIMING0, 5);                   //80ns(no spec)

	    //default access time is 300ns
	    WritePCICRegister( nSocket, REG_SETUP_TIMING1, 2);                   //80ns(no spec)
	    WritePCICRegister( nSocket, REG_CMD_TIMING1, 8);                   //320ns(by spec,25Mhz clock)
	    WritePCICRegister( nSocket, REG_RECOVERY_TIMING1, 2);                   //80ns(no spec)

        if( !m_fPollingMode )
        {
            tmp = ( UCHAR ) ( m_dwCSCIrq << 4 );
            tmp |= CFG_CARD_DETECT_ENABLE | CFG_READY_ENABLE;
            WritePCICRegister( nSocket, REG_STATUS_CHANGE_INT_CONFIG, tmp );
        }
        else
        {
            WritePCICRegister( nSocket, REG_STATUS_CHANGE_INT_CONFIG, 0 );
        }

        // PD6710 specific code to enable management interrupt(routed to -INTR)
        if( !m_fPollingMode )
        {
            WritePCICRegister( nSocket, REG_STATUS_CHANGE_INT_CONFIG, CFG_CARD_DETECT_ENABLE );
        }
        else
        {
            WritePCICRegister( nSocket, REG_STATUS_CHANGE_INT_CONFIG, 0 );
        }

        if( !m_fPollingMode )
        {
            // Enable Manage Interrupt
            tmp = ReadPCICRegister( nSocket, REG_INTERRUPT_AND_GENERAL_CONTROL );
            tmp |= INT_ENABLE_MANAGE_INT;
            WritePCICRegister( nSocket, REG_INTERRUPT_AND_GENERAL_CONTROL, tmp );
        }
    }

    // CreateIST Event
    m_hISTEvent = CreateEvent( 0, FALSE, FALSE, NULL );

    if( !m_fPollingMode )
    {
        // Run IST
        BOOL r = InterruptInitialize( m_dwCSCSysIntr, m_hISTEvent, 0, 0 );
        ASSERT( r );
    }

    return TRUE;
}

#ifdef DEBUG
VOID CPcmciaBusBridge::DumpAllRegisters()
{
    DEBUGMSG( ZONE_FUNCTION, ( TEXT( "Dumping all PCIC registers\r\n" ) ) );
    for( UINT8 nRegNum = 0; nRegNum < 0x40; nRegNum++ )
    {
        UINT8 val = ReadPCICRegister( 0, nRegNum );
        UINT8 val2 = ReadPCICRegister( 1, nRegNum );
        DEBUGMSG( ZONE_FUNCTION,
                  ( TEXT( "%02x: %02x %02x\r\n" ), nRegNum, val, val2 ) );
    }
    DEBUGMSG( ZONE_FUNCTION, ( TEXT( "Dump completed.\r\n" ) ) );
}
#endif

//
// Function to set the PCIC index register
//
VOID CPcmciaBusBridge::PCICIndex( UINT socket_num, UINT8  register_num )
{
    WRITE_PORT_UCHAR( m_PCICIndex,
                      ( UINT8 )( ( socket_num == 0 ? 0 : 0x40 ) | register_num ) );
}

//
// Function to write to the PCIC data register
//
VOID CPcmciaBusBridge::PCICDataWrite( UINT8 value )
{
    WRITE_PORT_UCHAR( m_PCICData, value );
}

//
// Function to read the PCIC data register
//
UINT8 CPcmciaBusBridge::PCICDataRead( VOID )
{
    return READ_PORT_UCHAR( m_PCICData );
}


//
// Verify the PCIC's REG_CHIP_REVISION
//
// This bit of code looks in the 82365 chip revision register (PCIC index 0)
// to see if a valid 82365 is in the system.  The original code only
// recognized the 83h silicon revision.  This indicates REV C silicon from
// Intel.  However, Intel also had a very popular rev B version, and that's
// what the integrated PCMCIA controller on the AMD ElanSC400 emulated.  The
// silicon revision register for that version returned 82h.
//
BOOL CPcmciaBusBridge::IsValidPCICSig( void )
{
    switch( m_vRevision = ReadPCICRegister( 0, REG_CHIP_REVISION ) )
    {
      case 0x82:
      case 0x83:
      case 0x84:
        // for newer chip - can handle 3.3v
        DEBUGMSG( 1,
                  ( TEXT( "PCMCIA:IsValidPCICSig Valid CHIP_REVISION detected = 0x%x at 0x%x\r\n" ),
                    m_vRevision,
                    m_PCICIndex ) );
        return TRUE;
    }
    DEBUGMSG( 1,
              ( TEXT( "PCMCIA:IsValidPCICSig Invalid CHIP_REVISION = 0x%x at 0x%x!!!\r\n" ),
                m_vRevision,
                m_PCICIndex ) );
    return FALSE;
}

//
// Function to get the initial settings from the registry
//
// NOTE: lpRegPath is assumed to be under HKEY_LOCAL_MACHINE
//
// Returns ERROR_SUCCESS on success or a Win32 error code on failure
//
DWORD CPcmciaBusBridge::GetRegistryConfig()
{
    DWORD dwRet = 1;
    DWORD dwSize, dwType, dwData;

    // get the PCMCIA windows configuration
    if( !LoadWindowsSettings() )
    {
        dwRet = ERROR_INVALID_DATA;
        goto grc_fail;
    }

    // get the polling mode value
    dwSize = sizeof( DWORD );
    if( !RegQueryValueEx( POLLING_MODE_NAME,
                          &dwType,
                          ( PUCHAR ) & dwData,
                          &dwSize ) )
    {
        m_fPollingMode = TRUE; // RegQueryValueEx failed, default to TRUE
    }
    else
    {
        m_fPollingMode = dwData ? TRUE : FALSE;
    }

    // get function interrupt routing configuration
    dwSize = sizeof( DWORD );
    if( !RegQueryValueEx( IRQ_VALUE_NAME,
                          &dwType,
                          ( PUCHAR ) & dwData,
                          &dwSize ) )
    {
        DEBUGMSG( ZONE_PDD,
                  ( TEXT( "PCMCIA:GetRegistyConfig RegQueryValueEx(%s) failed\r\n" ),
                    IRQ_VALUE_NAME ) );
        goto grc_fail;
    }
    m_dwIrq = dwData;

    m_dwCSCSysIntr = 0;
    m_dwCSCIrq = 0;
    if( !m_fPollingMode )
    {
        // get card status change interrupt routing configuration
        dwSize = sizeof( DWORD );
        if( !RegQueryValueEx( CSC_IRQ_VALUE_NAME,
                              &dwType,
                              ( PUCHAR ) & dwData,
                              &dwSize ) )
        {
            m_fPollingMode = TRUE;
        }
        else
        {
            m_dwCSCIrq = dwData;
        }
        dwSize = sizeof( DWORD );
        if( !RegQueryValueEx( CSC_SYSINTR_VALUE_NAME,
                              &dwType,
                              ( PUCHAR ) & dwData,
                              &dwSize ) )
        {
            m_fPollingMode = TRUE;
        }
        else
        {
            m_dwCSCSysIntr = dwData;
        }
    }

    // get the polling timeout value
    dwSize = sizeof( DWORD );
    if( !RegQueryValueEx( POLL_TIMEOUT_NAME,
                          &dwType,
                          ( PUCHAR ) & dwData,
                          &dwSize ) )
    {
        // RegQueryValueEx failed; if polling, set the timeout to 0.5 sec, otherwise set to INFINITE
        m_dwPollTimeout = m_fPollingMode ? 500 : INFINITE;
    }
    else
    {
        m_dwPollTimeout = dwData;
    }

    dwRet = ERROR_SUCCESS;

    grc_fail : return dwRet;
}   // GetRegistryConfig

BOOL CPcmciaBusBridge::InitCardBusBridge( void )
{
    DWORD dwRet;

    // Create critical section protecting the PCIC registers
    __try
    {
        InitializeCriticalSection( &m_PCICCritSec );
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        DEBUGMSG( ZONE_PDD,
                  ( TEXT( "PDCardInitServices InitializeCriticalSection failed %d\r\n" ) ) );
        return FALSE;
    }
    m_fPCICCritSecInitialized = true;

    // Get registry information
    if( ( dwRet = GetRegistryConfig() ) != ERROR_SUCCESS )
    {
        DEBUGMSG( ZONE_PDD,
                  ( TEXT( "PDCardInitServices GetRegistryConfig failed %d\r\n" ),
                    dwRet ) );
        return FALSE;
    }

    // Convert the data interrupt to a logical sysintr value.
    //
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &m_dwIrq, sizeof(DWORD), &m_dwSysIntr, sizeof(DWORD), NULL))
    {
        RETAILMSG(1, (TEXT("PCMCIA: Failed to obtain sysintr value for data interrupt.\r\n")));
        m_dwSysIntr = SYSINTR_UNDEFINED;
        return FALSE;
    }

	// Allocate PCMCIA buffers.
	m_vpIOPRegs = (S3C2440A_IOPORT_REG*)VirtualAlloc(0, sizeof(S3C2440A_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (m_vpIOPRegs == NULL) 
	{
		DEBUGMSG (1,(TEXT("m_vpIOPRegs is not allocated\n\r")));
		goto pcis_fail;
	}
	if (!VirtualCopy((PVOID)m_vpIOPRegs, (PVOID)(S3C2440A_BASE_REG_PA_IOPORT >> 8), sizeof(S3C2440A_IOPORT_REG), PAGE_PHYSICAL|PAGE_READWRITE|PAGE_NOCACHE)) {
		DEBUGMSG (1,(TEXT("m_vpIOPRegs is not mapped\n\r")));
		goto pcis_fail;
	}
	DEBUGMSG (1,(TEXT("m_vpIOPRegs is mapped to %x\n\r"), m_vpIOPRegs));
	
	m_vpMEMRegs = (S3C2440A_MEMCTRL_REG*)VirtualAlloc(0,sizeof(S3C2440A_MEMCTRL_REG), MEM_RESERVE,PAGE_NOACCESS);
	if(m_vpMEMRegs == NULL) 
	{
		DEBUGMSG (1,(TEXT("m_vpMEMRegs is not allocated\n\r")));
		goto pcis_fail;
	}
	if(!VirtualCopy((PVOID)m_vpMEMRegs,(PVOID)(S3C2440A_BASE_REG_PA_MEMCTRL >> 8),sizeof(S3C2440A_MEMCTRL_REG), PAGE_PHYSICAL|PAGE_READWRITE|PAGE_NOCACHE)) {

		DEBUGMSG (1,(TEXT("m_vpMEMRegs is not mapped\n\r")));
		goto pcis_fail;
	}    
	DEBUGMSG (1,(TEXT("m_vpMEMRegs is mapped to %x\n\r"), m_vpMEMRegs));

	m_vpPCMCIAPort = (PUCHAR*)VirtualAlloc(0, 0x0400, MEM_RESERVE,PAGE_NOACCESS);
	if(m_vpPCMCIAPort == NULL) 
	{
		DEBUGMSG (1,(TEXT("m_vpPCMCIAPort is not allocated\n\r")));
		goto pcis_fail;
	}
	if(!VirtualCopy((PVOID)m_vpPCMCIAPort,(PVOID)PD6710_IO_BASE_ADDRESS, 0x0400, PAGE_READWRITE|PAGE_NOCACHE)) {
		DEBUGMSG (1,(TEXT("m_vpPCMCIAPort is not mapped\n\r")));
		goto pcis_fail;
	}    
	DEBUGMSG (1,(TEXT("m_vpPCMCIAPort is mapped to %x\n\r"), m_vpPCMCIAPort));
	
	// Initialize S3C2440 for PD6710           
	// EINT3(GPF3) is enabled.
	m_vpIOPRegs->GPFCON = (m_vpIOPRegs->GPFCON & ~(0x3<<6)) | (0x2<<6); 
	// EINT3 is pulldown enabled.
	m_vpIOPRegs->GPFUP = (m_vpIOPRegs->GPFUP & ~(0x1<<3));

	// EINT8(GPG0) is enabled.
	m_vpIOPRegs->GPGCON = (m_vpIOPRegs->GPGCON & ~(0x3<<0)) | (0x2<<0); 
	// EINT8 is *not* pulldown enabled.
	m_vpIOPRegs->GPGUP = (m_vpIOPRegs->GPGUP | (0x1<<0));

	
	// nGCS2=nUB/nLB(nSBHE),nWAIT,16-bit
	m_vpMEMRegs->BWSCON = (m_vpMEMRegs->BWSCON & ~(0xf<<8)) | (0xd<<8); 

	// BANK2 access timing
	m_vpMEMRegs->BANKCON2 = ((B6710_Tacs<<13)+(B6710_Tcos<<11)+(B6710_Tacc<<8)+(B6710_Tcoh<<6)\
		+(B6710_Tah<<4)+(B6710_Tacp<<2)+(B6710_PMC));

	
	// EINT8=Level-high triggered, IRQ3.
	// EINT3=Falling Edge triggering -> connected INTR(controller)
	m_vpIOPRegs->EXTINT1=(m_vpIOPRegs->EXTINT1 & ~(0xf<<0)) | (0x1<<0); 
	m_vpIOPRegs->EXTINT0=(m_vpIOPRegs->EXTINT0 & ~(0xf<<12)) | (0x2<<12); 
		
	m_PCICIndex = ((PUCHAR)((ULONG)m_vpPCMCIAPort+0x3e0));
	m_PCICData = ((PUCHAR)((ULONG)m_vpPCMCIAPort+0x3e1));

	DEBUGMSG(1, (TEXT("PDCardInitServices m_PCICIndex = 0x%x, m_PCICData = 0x%x\r\n"),
		          m_PCICIndex, m_PCICData));

    if( !IsValidPCICSig() )
    {
        return FALSE;
    }

    DEBUGMSG( ZONE_PDD,
              ( TEXT( "PDCardInitServices m_PCICIndex = 0x%x, m_PCICData = 0x%x\r\n" ),
                m_PCICIndex,
                m_PCICData ) );

    UINT8 tmp = ReadPCICRegister( 0, REG_CHIP_REVISION );
    DEBUGMSG( ZONE_PDD,
              ( TEXT( "PDCardInitServices REG_CHIP_REVISION = 0x%x\r\n" ),
                tmp ) );

    DDKISRINFO ddi;
    if( GetIsrInfo( &ddi ) != ERROR_SUCCESS )
    {
        return FALSE;
    }

    InstallIsr();

    return TRUE;

pcis_fail:
    return FALSE;
}

#define DEFAULT_PRIORITY 101

BOOL CPcmciaBusBridge::Init()
{
    if( !loadPcCardEntry() )
    {
        return FALSE;
    }
    if( !InitCardBusBridge() )
    {
        return FALSE;
    }

    for( UINT8 nSocket = GetSocketLowBound();
         nSocket <= GetSocketHighBound();
         nSocket++ )
    {
        if( !GetSocketNumberFromCS( nSocket, TRUE ) )
        {
            for( UINT nTmp = GetSocketLowBound(); nTmp < nSocket; nTmp++ )
            {
                GetSocketNumberFromCS( nSocket, FALSE );
                m_rguSocketNum[nSocket] = ( UINT16 ) - 1;
            }
            return FALSE;
        }
    }

    // Set Tread Priority.
    m_uPriority = DEFAULT_PRIORITY;
    DWORD dwRegPriority = 0;
    if( GetRegValue( RegPriority256,
                     ( PBYTE ) & dwRegPriority,
                     sizeof( dwRegPriority ) ) )
    {
        m_uPriority = ( UINT ) dwRegPriority;
    }
    // Get Thread Priority from Registry and Set it.
    CeSetPriority( m_uPriority );
    m_bTerminated = FALSE;

    ThreadStart();
    return TRUE;
}
BOOL CPcmciaBusBridge::GetRegPowerOption( PDWORD pOption )
{
    if( pOption )
    {
        return GetRegValue( RegPowerOption,
                            ( PBYTE ) pOption,
                            sizeof( DWORD ) );
    }
    return TRUE;
}
void CPcmciaBusBridge::PowerMgr( BOOL bPowerDown )
{
    if( bPowerDown )
    {
        for( UINT8 nSocket = GetSocketLowBound();
            nSocket <= GetSocketHighBound();
            nSocket++ )
        {
            if( m_prgCardSocket[nSocket] )
            {
                // Disable client interrupts
                WritePCICRegister( nSocket, REG_INTERRUPT_AND_GENERAL_CONTROL,
                    ReadPCICRegister( nSocket, REG_INTERRUPT_AND_GENERAL_CONTROL ) & 0xf0 );
                // Shut down socket any way.
                WritePCICRegister( nSocket, REG_POWER_CONTROL, 0 );
            }
        }
    }
    else
    {
        // Initialize S3C2440 for PD6710           
        // EINT3(GPF3) is enabled.
        m_vpIOPRegs->GPFCON = (m_vpIOPRegs->GPFCON & ~(0x3<<6)) | (0x2<<6); 

        // EINT3 is pulldown enabled.
        m_vpIOPRegs->GPFUP = (m_vpIOPRegs->GPFUP & ~(0x1<<3));

        // EINT8(GPG0) is enabled.
        m_vpIOPRegs->GPGCON = (m_vpIOPRegs->GPGCON & ~(0x3<<0)) | (0x2<<0); 
        // EINT8 is *not* pulldown enabled.
        m_vpIOPRegs->GPGUP = (m_vpIOPRegs->GPGUP | (0x1<<0));


        // nGCS2=nUB/nLB(nSBHE),nWAIT,16-bit
        m_vpMEMRegs->BWSCON = (m_vpMEMRegs->BWSCON & ~(0xf<<8)) | (0xd<<8); 

        // BANK2 access timing
        m_vpMEMRegs->BANKCON2 = ((B6710_Tacs<<13)+(B6710_Tcos<<11)+(B6710_Tacc<<8)+(B6710_Tcoh<<6)\
            +(B6710_Tah<<4)+(B6710_Tacp<<2)+(B6710_PMC));


        // EINT8=Level-high triggered, IRQ3.
        // EINT3=Falling Edge triggering -> connected INTR(controller)
        m_vpIOPRegs->EXTINT1=(m_vpIOPRegs->EXTINT1 & ~(0xf<<0)) | (0x1<<0); 
        m_vpIOPRegs->EXTINT0=(m_vpIOPRegs->EXTINT0 & ~(0xf<<12)) | (0x2<<12); 

        for( UINT8 nSocket = GetSocketLowBound();
            nSocket <= GetSocketHighBound();
            nSocket++ )
        {
            if( m_prgCardSocket[nSocket] )
            {
                CardInjectEvent( nSocket );
            }
        }
    } 
}

BOOL CPcmciaBusBridge::NeedPowerResuming()
{
    m_bResumeFlag = TRUE;
    SetInterruptEvent( m_dwCSCSysIntr );
    return TRUE;
}

void CPcmciaBusBridge::PowerCycleEvent( UINT8 nSocket )
{
    Lock();
    m_rgfPowerCycleEvent[nSocket] = TRUE;
    SetEvent( m_hISTEvent );
    Unlock();
}

void CPcmciaBusBridge::CardInjectEvent( UINT8 nSocket )
{
    Lock();
    m_rgfCardInjectEvent[nSocket] = TRUE;
    SetEvent( m_hISTEvent );
    Unlock();
}

DWORD CPcmciaBusBridge::ThreadRun() // THIS is CardBusBridge IST.
{
    // Test if a PCMCIA card is already inserted in one of the sockets
    bool fCardAlreadyInserted = false;
    DWORD dwOldCardStatus[NUM_SLOTS];
    bool frgCardAlreadyInserted[NUM_SLOTS];
    for( int nSocket = GetSocketLowBound();
         nSocket <= GetSocketHighBound();
         nSocket++ )
    {
        frgCardAlreadyInserted[nSocket] = false;
        if( ( ReadPCICRegister( nSocket, REG_INTERFACE_STATUS ) 
                & ( STS_CD1 | STS_CD2 ) ) == ( STS_CD1 | STS_CD2 ) )
        {
            frgCardAlreadyInserted[nSocket] = true;
            fCardAlreadyInserted = true;
            dwOldCardStatus[nSocket] = ( STS_CD1 | STS_CD2 );
        }
        else
        {
            dwOldCardStatus[nSocket] = 0;
        }
    }

    // run until signalled to terminate
    while( !m_bTerminated )
    {
        DEBUGCHK(m_hISTEvent!=NULL);
        BOOL bInterrupt;
        if( !fCardAlreadyInserted )
        {
            bInterrupt = (WaitForSingleObject(m_hISTEvent,m_dwPollTimeout)!=WAIT_TIMEOUT);
            if( m_fPollingMode ) // we are polling, fake an interrupt event
            {
                bInterrupt = true;
            }
        }
        else
        {
            bInterrupt = true;
        }

        if (!bInterrupt) { // we have reached a timeout in non-polling mode or something bad has occured.
            continue;
        }

        Lock();

        // check each card slot for changes
        for( UINT8 nSocket = GetSocketLowBound();
             nSocket <= GetSocketHighBound();
             nSocket++ )
        {
            UINT8 BSocketState, bCardStatusChange;
            UINT8 fNotifyEvents = 0;

            if( !m_fPollingMode )
            {
                // See what changed and acknowledge any status change interrupts
                bCardStatusChange = ReadPCICRegister( nSocket,
                                                REG_CARD_STATUS_CHANGE );
            }
            else
            {
                // Get the current card status and compare it to the previous status
                DWORD newStatus = ReadPCICRegister( nSocket, REG_INTERFACE_STATUS ) & ( STS_CD1 | STS_CD2 );
                bCardStatusChange = ( newStatus != dwOldCardStatus[nSocket] ) ? CSC_DETECT_CHANGE : 0;
                dwOldCardStatus[nSocket] = newStatus;
            }

            // if nothing has changed, continue to the next slot
            if( !frgCardAlreadyInserted[nSocket] &&
                !m_rgfPowerCycleEvent[nSocket] &&
                !m_rgfCardInjectEvent[nSocket] &&
                bCardStatusChange == 0x0 )
            {
                continue;
            }

            //
            // Figure out the socket state
            //
            BSocketState = ReadPCICRegister( nSocket, REG_INTERFACE_STATUS );

            if( frgCardAlreadyInserted[nSocket] )
            {
                bCardStatusChange |= CSC_DETECT_CHANGE;
                frgCardAlreadyInserted[nSocket] = false;
            }

            if( bCardStatusChange & CSC_DETECT_CHANGE )
            {
                fNotifyEvents |= EVENT_MASK_CARD_DETECT;
            }

            if( m_rgfCardInjectEvent[nSocket] )
            {
                fNotifyEvents |= EVENT_MASK_CARD_DETECT;
                m_rgfCardInjectEvent[nSocket] = FALSE;
            }

            if( m_rgfPowerCycleEvent[nSocket] )
            {
                fNotifyEvents |= SPS_POWERCYCLE;
                m_rgfPowerCycleEvent[nSocket] = FALSE;
            }

            if( fNotifyEvents & EVENT_MASK_CARD_DETECT )
            {
                // we're only processing Card Detection Signal.
                // If this happens, Something has been changed. 
                // Send out Card revmoval before doing any more process.
                RemovePcmciaCardSocket( nSocket );
                ProcessCDD( nSocket, BSocketState );
            }
            else
            {
                // Other Event left for type specific socket
                if( m_prgCardSocket[nSocket] )
                    m_prgCardSocket[nSocket]->SocketEventHandle( bCardStatusChange,
                                                                 fNotifyEvents );
            }
        } // for

        Unlock();
        fCardAlreadyInserted = false;

        if( !m_fPollingMode )
        {
            DEBUGMSG( ZONE_FUNCTION,
                      ( L"CardBus: PCCardBus Call InterruptDone\r\n" ) );
            InterruptDone( m_dwCSCSysIntr );
        }
    } // while(!m_bTerminated) 
    return 0;
}

void CPcmciaBusBridge::ProcessCDD( UINT nSocketNumber, DWORD dwPStateReg )
{
    switch( dwPStateReg & ( STS_CD1 | STS_CD2 ) )
    {
      case 0x0:
        // Complete removal
        if( m_prgCardSocket[nSocketNumber] )
        {
            RemovePcmciaCardSocket( nSocketNumber );
        }
        // Socket Detached.

        break;
      case (STS_CD1|STS_CD2):
        // Complete Insertion.
        if( m_prgCardSocket[nSocketNumber] == NULL )
        {
            InsertPcmciaCardSocket( CreatePCMCIASocket( this, nSocketNumber ),
                                    nSocketNumber );
        }
        break;
      default:
        break;
    }
}

void CPcmciaBusBridge::CallBackToCardService( UINT8 nSocket,
                                              HANDLE hSocket,
                                              PSS_SOCKET_STATE pSocketState )
{
    if( m_pCallBackToCardService )
    {
        __try
        {
            m_pCallBackToCardService( hSocket,
                                      m_rguSocketNum[nSocket],
                                      pSocketState );
        } __except( EXCEPTION_EXECUTE_HANDLER )
        {
            DEBUGCHK( FALSE );
        }
    }
    else
    {
        DEBUGCHK( FALSE );
    }
}
BOOL CPcmciaBusBridge::GetSocketNumberFromCS( UINT8 nSocket, BOOL bGet )
{
    STATUS status = CERR_BAD_ARGS;
    if( m_pRequestSocketNumber && m_pDeleteSocket )
    {
        __try
        {
            status = ( bGet ?
                       m_pRequestSocketNumber( &m_rguSocketNum[nSocket],
                                               sizeof( SMDK2440SocketServiceStatic ),
                                               &SMDK2440SocketServiceStatic,
                                               GetSocketName() ) :
                       m_pDeleteSocket( m_rguSocketNum[nSocket] ) );
        } __except( EXCEPTION_EXECUTE_HANDLER )
        {
            DEBUGCHK( FALSE );
            status = CERR_BAD_ARGS;
        }
    }
    DEBUGCHK( status == CERR_SUCCESS );
    return ( status == CERR_SUCCESS );
}
