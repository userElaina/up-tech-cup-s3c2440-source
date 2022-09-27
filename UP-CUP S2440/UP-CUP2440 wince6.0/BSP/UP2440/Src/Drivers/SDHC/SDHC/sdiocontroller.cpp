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

// Copyright (c) 2002 BSQUARE Corporation.  All rights reserved.
// DO NOT REMOVE --- BEGIN EXTERNALLY DEVELOPED SOURCE CODE ID 40973--- DO NOT REMOVE

#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <s3c2440a.h>
#include <creg.hxx>
#include <ddkreg.h>
#include <SDCardDDK.h>
#include <SDHCD.h>

#include "SDIOController.h"

///////////////////////////////////////////////////////////////////////////////
//  CSDIOController::CSDIOController - CSDIOController constructor
//  Input:  pHCContext - hardware context
//  Notes:  
//          
///////////////////////////////////////////////////////////////////////////////
CSDIOController::CSDIOController( PSDCARD_HC_CONTEXT pHCContext )
: CSDIOControllerBase( pHCContext )
{
}

///////////////////////////////////////////////////////////////////////////////
//  CSDIOController::~CSDIOController - CSDIOController destructor
//  Input:  
//  Notes:  
//          
///////////////////////////////////////////////////////////////////////////////
CSDIOController::~CSDIOController()
{
}

#define CARD_DETECT_GPIO_TEXT TEXT("CardDetectGPIO")
#define CARD_DETECT_MASK_TEXT TEXT("CardDetectMask")
#define CARD_DETECT_FLAG_TEXT TEXT("CardDetectFlag")
#define CARD_DETECT_CONTROL_MASK_TEXT TEXT("CardDetectControlMask")
#define CARD_DETECT_CONTROL_FLAG_TEXT TEXT("CardDetectControlFlag")
#define CARD_DETECT_PULLUP_MASK_TEXT TEXT("CardDetectPullupMask")
#define CARD_DETECT_PULLUP_FLAG_TEXT TEXT("CardDetectPullupFlag")

#define CARD_READWRITE_GPIO_TEXT TEXT("CardReadWriteGPIO")
#define CARD_READWRITE_MASK_TEXT TEXT("CardReadWriteMask")
#define CARD_READWRITE_FLAG_TEXT TEXT("CardReadWriteFlag")
#define CARD_READWRITE_CONTROL_MASK_TEXT TEXT("CardReadWriteControlMask")
#define CARD_READWRITE_CONTROL_FLAG_TEXT TEXT("CardReadWriteControlFlag")
#define CARD_READWRITE_PULLUP_MASK_TEXT TEXT("CardReadWritePullupMask")
#define CARD_READWRITE_PULLUP_FLAG_TEXT TEXT("CardReadWritePullupFlag")

BOOL CSDIOController::CustomSetup( LPCTSTR pszRegistryPath )
{
    BOOL fRetVal = TRUE;
    CReg regDevice; // encapsulated device key
    HKEY hKeyDevice = OpenDeviceKey(pszRegistryPath);
    if ( (hKeyDevice == NULL) || !regDevice.Open(hKeyDevice, NULL) ) {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("CSDIOControllerBase::InterpretCapabilities: Failed to open device key\r\n")));
        fRetVal = FALSE;
        goto FUNCTION_EXIT;
    }

    // read the card detect GPIO settings
    LPCTSTR pszCardDetectGPIO = regDevice.ValueSZ( CARD_DETECT_GPIO_TEXT );
    if( pszCardDetectGPIO && pszCardDetectGPIO[0] >= TEXT('a') && pszCardDetectGPIO[0] <= TEXT('h') )
    {
        m_chCardDetectGPIO = (char)pszCardDetectGPIO[0] - ('a'-'A');
    }
    else if( pszCardDetectGPIO && pszCardDetectGPIO[0] >= TEXT('A') && pszCardDetectGPIO[0] <= TEXT('H') )
    {
        m_chCardDetectGPIO = (char)pszCardDetectGPIO[0];
    }
    else
    {
        // invalid SDIO SYSINTR value!
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("invalid SDIO SYSINTR value!\r\n")));
        fRetVal = FALSE;
        goto FUNCTION_EXIT;
    }
    m_dwCardDetectMask = regDevice.ValueDW( CARD_DETECT_MASK_TEXT );
    m_dwCardDetectFlag = regDevice.ValueDW( CARD_DETECT_FLAG_TEXT );
    m_dwCardDetectControlMask = regDevice.ValueDW( CARD_DETECT_CONTROL_MASK_TEXT );
    m_dwCardDetectControlFlag = regDevice.ValueDW( CARD_DETECT_CONTROL_FLAG_TEXT );
    m_dwCardDetectPullupMask = regDevice.ValueDW( CARD_DETECT_PULLUP_MASK_TEXT );
    m_dwCardDetectPullupFlag = regDevice.ValueDW( CARD_DETECT_PULLUP_FLAG_TEXT );

    // read the card read/write GPIO settings
    LPCTSTR pszCardReadWriteGPIO = regDevice.ValueSZ( CARD_READWRITE_GPIO_TEXT );
    if( pszCardReadWriteGPIO && pszCardReadWriteGPIO[0] >= TEXT('a') && pszCardReadWriteGPIO[0] <= TEXT('h') )
    {
        m_chCardReadWriteGPIO = (char)pszCardReadWriteGPIO[0] - ('a'-'A');
    }
    else if( pszCardReadWriteGPIO || pszCardReadWriteGPIO[0] >= TEXT('A') && pszCardReadWriteGPIO[0] <= TEXT('H') )
    {
        m_chCardReadWriteGPIO = (char)pszCardReadWriteGPIO[0];
    }
    else
    {
        // invalid SDIO SYSINTR value!
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("invalid SDIO SYSINTR value!\r\n")));
        if (hKeyDevice) RegCloseKey(hKeyDevice);
        fRetVal = FALSE;
        goto FUNCTION_EXIT;
    }
    m_dwCardReadWriteMask = regDevice.ValueDW( CARD_READWRITE_MASK_TEXT );
    m_dwCardReadWriteFlag = regDevice.ValueDW( CARD_READWRITE_FLAG_TEXT );
    m_dwCardReadWriteControlMask = regDevice.ValueDW( CARD_READWRITE_CONTROL_MASK_TEXT );
    m_dwCardReadWriteControlFlag = regDevice.ValueDW( CARD_READWRITE_CONTROL_FLAG_TEXT );
    m_dwCardReadWritePullupMask = regDevice.ValueDW( CARD_READWRITE_PULLUP_MASK_TEXT );
    m_dwCardReadWritePullupFlag = regDevice.ValueDW( CARD_READWRITE_PULLUP_FLAG_TEXT );

FUNCTION_EXIT:
    if (hKeyDevice) RegCloseKey(hKeyDevice);
    return fRetVal;
}

BOOL CSDIOController::InitializeHardware( BOOL bOnPowerUp )
{
    // Configure GPIO pin to detect WRITE-PROTECT status
    switch( m_chCardReadWriteGPIO )
    {
    case 'A':
        vm_pIOPreg->GPACON  = ( vm_pIOPreg->GPACON & m_dwCardReadWriteControlMask ) | m_dwCardReadWriteControlFlag;
        break;
    case 'B':
        vm_pIOPreg->GPBCON  = ( vm_pIOPreg->GPBCON & m_dwCardReadWriteControlMask ) | m_dwCardReadWriteControlFlag;
	    vm_pIOPreg->GPBUP   = ( vm_pIOPreg->GPBUP & m_dwCardReadWritePullupMask ) | m_dwCardReadWritePullupFlag;
        break;
    case 'C':
        vm_pIOPreg->GPCCON  = ( vm_pIOPreg->GPCCON & m_dwCardReadWriteControlMask ) | m_dwCardReadWriteControlFlag;
	    vm_pIOPreg->GPCUP   = ( vm_pIOPreg->GPCUP & m_dwCardReadWritePullupMask ) | m_dwCardReadWritePullupFlag;
        break;
    case 'D':
        vm_pIOPreg->GPDCON  = ( vm_pIOPreg->GPDCON & m_dwCardReadWriteControlMask ) | m_dwCardReadWriteControlFlag;
	    vm_pIOPreg->GPDUP   = ( vm_pIOPreg->GPDUP & m_dwCardReadWritePullupMask ) | m_dwCardReadWritePullupFlag;
        break;
    case 'E':
        vm_pIOPreg->GPECON  = ( vm_pIOPreg->GPECON & m_dwCardReadWriteControlMask ) | m_dwCardReadWriteControlFlag;
	    vm_pIOPreg->GPEUP   = ( vm_pIOPreg->GPEUP & m_dwCardReadWritePullupMask ) | m_dwCardReadWritePullupFlag;
        break;
    case 'F':
        vm_pIOPreg->GPFCON  = ( vm_pIOPreg->GPFCON & m_dwCardReadWriteControlMask ) | m_dwCardReadWriteControlFlag;
	    vm_pIOPreg->GPFUP   = ( vm_pIOPreg->GPFUP & m_dwCardReadWritePullupMask ) | m_dwCardReadWritePullupFlag;
        break;
    case 'G':
        vm_pIOPreg->GPGCON  = ( vm_pIOPreg->GPGCON & m_dwCardReadWriteControlMask ) | m_dwCardReadWriteControlFlag;
	    vm_pIOPreg->GPGUP   = ( vm_pIOPreg->GPGUP & m_dwCardReadWritePullupMask ) | m_dwCardReadWritePullupFlag;
        break;
    case 'H':
        vm_pIOPreg->GPHCON  = ( vm_pIOPreg->GPHCON & m_dwCardReadWriteControlMask ) | m_dwCardReadWriteControlFlag;
	    vm_pIOPreg->GPHUP   = ( vm_pIOPreg->GPHUP & m_dwCardReadWritePullupMask ) | m_dwCardReadWritePullupFlag;
        break;
    default:
        ASSERT(0); // invalid GPIO! We should never get here!
        return FALSE;
    }

    // Configure GPIO pin to detect CARD PRESENT status
    switch( m_chCardDetectGPIO )
    {
    case 'A':
        vm_pIOPreg->GPACON  = ( vm_pIOPreg->GPACON & m_dwCardDetectControlMask ) | m_dwCardDetectControlFlag;
        break;
    case 'B':
        vm_pIOPreg->GPBCON  = ( vm_pIOPreg->GPBCON & m_dwCardDetectControlMask ) | m_dwCardDetectControlFlag;
	    vm_pIOPreg->GPBUP   = ( vm_pIOPreg->GPBUP & m_dwCardDetectPullupMask ) | m_dwCardDetectPullupFlag;
        break;
    case 'C':
        vm_pIOPreg->GPCCON  = ( vm_pIOPreg->GPCCON & m_dwCardDetectControlMask ) | m_dwCardDetectControlFlag;
	    vm_pIOPreg->GPCUP   = ( vm_pIOPreg->GPCUP & m_dwCardDetectPullupMask ) | m_dwCardDetectPullupFlag;
        break;
    case 'D':
        vm_pIOPreg->GPDCON  = ( vm_pIOPreg->GPDCON & m_dwCardDetectControlMask ) | m_dwCardDetectControlFlag;
	    vm_pIOPreg->GPDUP   = ( vm_pIOPreg->GPDUP & m_dwCardDetectPullupMask ) | m_dwCardDetectPullupFlag;
        break;
    case 'E':
        vm_pIOPreg->GPECON  = ( vm_pIOPreg->GPECON & m_dwCardDetectControlMask ) | m_dwCardDetectControlFlag;
	    vm_pIOPreg->GPEUP   = ( vm_pIOPreg->GPEUP & m_dwCardDetectPullupMask ) | m_dwCardDetectPullupFlag;
        break;
    case 'F':
        vm_pIOPreg->GPFCON  = ( vm_pIOPreg->GPFCON & m_dwCardDetectControlMask ) | m_dwCardDetectControlFlag;
	    vm_pIOPreg->GPFUP   = ( vm_pIOPreg->GPFUP & m_dwCardDetectPullupMask ) | m_dwCardDetectPullupFlag;
        break;
    case 'G':
        vm_pIOPreg->GPGCON  = ( vm_pIOPreg->GPGCON & m_dwCardDetectControlMask ) | m_dwCardDetectControlFlag;
	    vm_pIOPreg->GPGUP   = ( vm_pIOPreg->GPGUP & m_dwCardDetectPullupMask ) | m_dwCardDetectPullupFlag;
        break;
    case 'H':
        vm_pIOPreg->GPHCON  = ( vm_pIOPreg->GPHCON & m_dwCardDetectControlMask ) | m_dwCardDetectControlFlag;
	    vm_pIOPreg->GPHUP   = ( vm_pIOPreg->GPHUP & m_dwCardDetectPullupMask ) | m_dwCardDetectPullupFlag;
        break;
    default:
        ASSERT(0); // invalid GPIO! We should never get here!
        return FALSE;
    }

    return TRUE;
}

void CSDIOController::DeinitializeHardware( BOOL bOnPowerDown )
{
    // nothing to do
}

BOOL CSDIOController::IsCardWriteProtected()
{
    switch( m_chCardReadWriteGPIO )
    {
    case 'A':
        return ( ( vm_pIOPreg->GPADAT & m_dwCardReadWriteMask ) == m_dwCardReadWriteFlag ) ? TRUE  : FALSE;
    case 'B':
        return ( ( vm_pIOPreg->GPBDAT & m_dwCardReadWriteMask ) == m_dwCardReadWriteFlag ) ? TRUE  : FALSE;
    case 'C':
        return ( ( vm_pIOPreg->GPCDAT & m_dwCardReadWriteMask ) == m_dwCardReadWriteFlag ) ? TRUE  : FALSE;
    case 'D':
        return ( ( vm_pIOPreg->GPDDAT & m_dwCardReadWriteMask ) == m_dwCardReadWriteFlag ) ? TRUE  : FALSE;
    case 'E':
        return ( ( vm_pIOPreg->GPEDAT & m_dwCardReadWriteMask ) == m_dwCardReadWriteFlag ) ? TRUE  : FALSE;
    case 'F':
        return ( ( vm_pIOPreg->GPFDAT & m_dwCardReadWriteMask ) == m_dwCardReadWriteFlag ) ? TRUE  : FALSE;
    case 'G':
        return ( ( vm_pIOPreg->GPGDAT & m_dwCardReadWriteMask ) == m_dwCardReadWriteFlag ) ? TRUE  : FALSE;
    case 'H':
        return ( ( vm_pIOPreg->GPHDAT & m_dwCardReadWriteMask ) == m_dwCardReadWriteFlag ) ? TRUE  : FALSE;
    default:
        ASSERT(0); // invalid GPIO!  We should never get here
        return TRUE;
    }
}

BOOL CSDIOController::IsCardPresent()
{
    switch( m_chCardDetectGPIO )
    {
    case 'A':
        return ( ( vm_pIOPreg->GPADAT & m_dwCardDetectMask ) == m_dwCardDetectFlag ) ? TRUE  : FALSE;
    case 'B':
        return ( ( vm_pIOPreg->GPBDAT & m_dwCardDetectMask ) == m_dwCardDetectFlag ) ? TRUE  : FALSE;
    case 'C':
        return ( ( vm_pIOPreg->GPCDAT & m_dwCardDetectMask ) == m_dwCardDetectFlag ) ? TRUE  : FALSE;
    case 'D':
        return ( ( vm_pIOPreg->GPDDAT & m_dwCardDetectMask ) == m_dwCardDetectFlag ) ? TRUE  : FALSE;
    case 'E':
        return ( ( vm_pIOPreg->GPEDAT & m_dwCardDetectMask ) == m_dwCardDetectFlag ) ? TRUE  : FALSE;
    case 'F':
        return ( ( vm_pIOPreg->GPFDAT & m_dwCardDetectMask ) == m_dwCardDetectFlag ) ? TRUE  : FALSE;
    case 'G':
        return ( ( vm_pIOPreg->GPGDAT & m_dwCardDetectMask ) == m_dwCardDetectFlag ) ? TRUE  : FALSE;
    case 'H':
        return ( ( vm_pIOPreg->GPHDAT & m_dwCardDetectMask ) == m_dwCardDetectFlag ) ? TRUE  : FALSE;
    default:
        ASSERT(0); // invalid GPIO!  We should never get here
    }
    return (vm_pIOPreg->GPGDAT & (1<<10))  ? FALSE : TRUE;
}

CSDIOControllerBase* CreateSDIOController( PSDCARD_HC_CONTEXT pHCContext )
{
    return new CSDIOController( pHCContext );
}

// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE
