#include <windows.h>
#include <nkintr.h>
#include <pm.h>
#include "pmplatform.h"
#include "Pkfuncs.h"
#include "BSP.h"

#include "GPIODriver.h"

volatile S3C2440A_IOPORT_REG *v_pIOPregs ;

BOOL mInitialized;
bool InitializeAddresses(VOID);						// Virtual allocation


bool InitializeAddresses(VOID)
{
	bool	RetValue = TRUE;

	/* IO Register Allocation */
	v_pIOPregs = (volatile S3C2440A_IOPORT_REG *)VirtualAlloc(0, sizeof(S3C2440A_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (v_pIOPregs == NULL) 
	{
		ERRORMSG(1,(TEXT("For IOPregs : VirtualAlloc faiGPIO!\r\n")));
		RetValue = FALSE;
	}
	else 
	{
		if (!VirtualCopy((PVOID)v_pIOPregs, (PVOID)(S3C2440A_BASE_REG_PA_IOPORT >> 8), sizeof(S3C2440A_IOPORT_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) 
		{
			ERRORMSG(1,(TEXT("For IOPregs: VirtualCopy faiGPIO!\r\n")));
			RetValue = FALSE;
		}
	}

	if (!RetValue) 
	{
		if (v_pIOPregs) 
		{
			VirtualFree((PVOID) v_pIOPregs, 0, MEM_RELEASE);
		}

		v_pIOPregs = NULL;
	}

	return(RetValue);

}


BOOL WINAPI  
DllEntry(HANDLE	hinstDLL, 
			DWORD dwReason, 
			LPVOID /* lpvReserved */)
{
	switch(dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DEBUGREGISTER((HINSTANCE)hinstDLL);
		return TRUE;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
#ifdef UNDER_CE
	case DLL_PROCESS_EXITING:
		break;
	case DLL_SYSTEM_STARTED:
		break;
#endif
	}

	return TRUE;
}


BOOL GIO_Deinit(DWORD hDeviceContext)
{
	BOOL bRet = TRUE;

	RETAILMSG(1,(TEXT("GPIO_Control: GIO_Deinit\r\n")));

	return TRUE;
} 


DWORD GIO_Init(DWORD dwContext)
{

	RETAILMSG(1,(TEXT("GPIO Initialize ...")));

	if (!InitializeAddresses())
		return (FALSE);

	v_pIOPregs->GPCCON  = (v_pIOPregs->GPCCON  &~(3 << 10)) | (1<< 10);	// GPC5 == OUTPUT.
	v_pIOPregs->GPCCON  = (v_pIOPregs->GPCCON  &~(3 << 12)) | (1<< 12);	// GPC6 == OUTPUT.
	v_pIOPregs->GPCCON  = (v_pIOPregs->GPCCON  &~(3 << 14)) | (1<< 14);	// GPC7 == OUTPUT.
	//v_pIOPregs->GPBCON  = (v_pIOPregs->GPBCON  &~(3 << 16)) | (1<< 16);	// GPB8 == OUTPUT.
	
	mInitialized = TRUE;
	RETAILMSG(1,(TEXT("OK !!!\n")));
	return TRUE;
}

BOOL GIO_IOControl(DWORD hOpenContext, 
				   DWORD dwCode, 
				   PBYTE pBufIn, 
				   DWORD dwLenIn, 
				   PBYTE pBufOut, 
				   DWORD dwLenOut, 
				   PDWORD pdwActualOut)
{
	switch(dwCode)
	{
	case IO_CTL_GPIO_1_ON:
		v_pIOPregs->GPCDAT=v_pIOPregs->GPCDAT&~(0x1<<5);
		break;
	case IO_CTL_GPIO_2_ON:
		v_pIOPregs->GPCDAT=v_pIOPregs->GPCDAT&~(0x1<<6);
		break;
	case IO_CTL_GPIO_3_ON:
		v_pIOPregs->GPCDAT=v_pIOPregs->GPCDAT&~(0x1<<7);
		break;
	
	case IO_CTL_GPIO_ALL_ON:
		v_pIOPregs->GPCDAT=v_pIOPregs->GPCDAT&~(0x7<<5);
		break;
	case IO_CTL_GPIO_1_OFF:
		v_pIOPregs->GPCDAT=v_pIOPregs->GPCDAT|(0x1<<5);
		break;
	case IO_CTL_GPIO_2_OFF:
		v_pIOPregs->GPCDAT=v_pIOPregs->GPCDAT|(0x1<<6);
		break;
	case IO_CTL_GPIO_3_OFF:
		v_pIOPregs->GPCDAT=v_pIOPregs->GPCDAT|(0x1<<7);
		break;
	
	case IO_CTL_GPIO_ALL_OFF:
		v_pIOPregs->GPCDAT=v_pIOPregs->GPCDAT|(0x7<<5);
		break;
	default:
		break;
	}
    
	RETAILMSG(1,(TEXT("GPIO_Control:Ioctl code = 0x%x\r\n"), dwCode));
	return TRUE;
} 

DWORD GIO_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
	v_pIOPregs->GPCDAT=v_pIOPregs->GPCDAT|(0x7<<5);

	RETAILMSG(1,(TEXT("GPIO_Control: GPIO_Open\r\n")));
	return TRUE;
} 

BOOL GIO_Close(DWORD hOpenContext)
{
	v_pIOPregs->GPCDAT=v_pIOPregs->GPCDAT&(0x7<<5);

	RETAILMSG(1,(TEXT("GPIO_Control: GPIO_Close\r\n")));
	return TRUE;
}

void GIO_PowerDown(DWORD hDeviceContext)
{
	RETAILMSG(1,(TEXT("GPIO_Control: GPIO_PowerDown\r\n")));
}

void GIO_PowerUp(DWORD hDeviceContext)
{
	RETAILMSG(1,(TEXT("GPIO_Control: GPIO_PowerUp\r\n")));
}

DWORD GIO_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
	RETAILMSG(1,(TEXT("GPIO_Control: GPIO_Read\r\n")));
	return TRUE;
}

DWORD GIO_Seek(DWORD hOpenContext, long Amount, DWORD Type)
{
	RETAILMSG(1,(TEXT("GPIO_Control: GPIO_Seek\r\n")));
	return 0;
}

DWORD GIO_Write(DWORD hOpenContext, LPCVOID pSourceBytes, DWORD NumberOfBytes)
{
	RETAILMSG(1,(TEXT("GPIO_Control: GPIO_Write\r\n")));
	return 0;
}




