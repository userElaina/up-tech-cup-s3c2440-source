#include <windows.h>
#include <types.h>
#include <excpt.h>
#include <tchar.h>
#include <cardserv.h>
#include <cardapi.h>
#include <tuple.h>
#include <devload.h>
#include <diskio.h>
#include <nkintr.h>
#include <windev.h>
#include <pm.h>

#include "BSP.h"

#include "pmplatform.h"
#include "Pkfuncs.h"

static volatile S3C2440A_IOPORT_REG * v_pIOPregs;
volatile S3C2440A_INTR_REG * v_pINTRregs;

//UINT32 g_KeySysIntr[4];
UINT32 g_KeySysIntr[1];
HANDLE IntThread;
HANDLE IntEvent;

void Virtual_Alloc();						// Virtual allocation
DWORD IntProcessThread(void);



DWORD IntProcessThread(void)
{
	RETAILMSG(1,(TEXT("USERKEY: IntProcessThread\r\n")));
	UINT32 IRQ;

	IntEvent = CreateEvent(NULL, FALSE, FALSE, NULL);    
	if (!IntEvent)
	{
		RETAILMSG(1, (TEXT("ERROR: kEYBD: Failed to create event.\r\n")));
		return FALSE;
	}

	IRQ = 33; //IRQ_EINT5;
	if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &IRQ, sizeof(UINT32), &g_KeySysIntr[0], sizeof(UINT32), NULL))
	{
		RETAILMSG(1, (TEXT("ERROR: kEYBD: Failed to request sysintr value.\r\n")));
		return FALSE;
	}

	if (!InterruptInitialize(g_KeySysIntr[0], IntEvent, NULL, 0))
	{
		RETAILMSG(1,(TEXT("Fail to initialize userkey interrupt event\r\n")));
		return FALSE;
	}

	while(1)
	{
		v_pINTRregs->INTMSK &= ~(1<<4);

	v_pIOPregs->EINTMASK  &= (0x0 << 5);
		RETAILMSG(1,(L"INTPND=%X, SRCPND=%X,EINTPEND=%X, GPFCON=%X\r\n", v_pINTRregs->INTPND, v_pINTRregs->SRCPND,v_pIOPregs->EINTPEND, v_pIOPregs->GPFCON));
		WaitForSingleObject(IntEvent, INFINITE);
		RETAILMSG(1,(L"_INTMASK=%X, EINTMASK=%X, GPFCON=%X\r\n", v_pINTRregs->INTMSK, v_pIOPregs->EINTMASK, v_pIOPregs->GPFCON));



		
#if 1
		if(v_pIOPregs->EINTPEND & (1<<5))
		{
			RETAILMSG(1,(TEXT("[Key Down]\r\n")));
			/*keybd_event(VK_UP ,0x26, 0, 0);
			Sleep(30);
			keybd_event(VK_UP ,0x26, KEYEVENTF_KEYUP, 0);*/
			v_pIOPregs->GPCDAT=v_pIOPregs->GPCDAT&~(0x1<<5);
			Sleep(300);
			v_pIOPregs->GPCDAT=v_pIOPregs->GPCDAT|(0x1<<5);
			Sleep(300);

			v_pIOPregs->GPCDAT=v_pIOPregs->GPCDAT&~(0x1<<6);
			Sleep(300);
			v_pIOPregs->GPCDAT=v_pIOPregs->GPCDAT|(0x1<<6);
			Sleep(30);

			v_pIOPregs->GPCDAT=v_pIOPregs->GPCDAT&~(0x1<<7);
			Sleep(300);
			v_pIOPregs->GPCDAT=v_pIOPregs->GPCDAT|(0x1<<7);
	
			InterruptDone(g_KeySysIntr[0]);
		}
#endif
		

	}
}



void Virtual_Alloc()
{

	/* IO Register Allocation */
	v_pIOPregs = (volatile S3C2440A_IOPORT_REG *) VirtualAlloc(0,sizeof(S3C2440A_IOPORT_REG),MEM_RESERVE, PAGE_NOACCESS);
	if(v_pIOPregs == NULL)
	{
		RETAILMSG(1,(TEXT("For IOPregs: VirtualAlloc failed!\r\n")));
	}
	else
	{
		if(!VirtualCopy((PVOID)v_pIOPregs,(PVOID)(S3C2440A_BASE_REG_PA_IOPORT >> 8),sizeof(S3C2440A_IOPORT_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE))
		{
			RETAILMSG(1,(TEXT("For IOPregs: VirtualCopy failed!\r\n")));
		}
	}
 
	/* INTR Register Allocation */
	v_pINTRregs = (volatile S3C2440A_INTR_REG *)VirtualAlloc(0, sizeof(S3C2440A_INTR_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (v_pINTRregs == NULL) 
	{
		ERRORMSG(1,(TEXT("For INTRregs : VirtualAlloc failed!\r\n")));
	}
	else 
	{
		if (!VirtualCopy((PVOID)v_pINTRregs, (PVOID)(S3C2440A_BASE_REG_PA_INTR >> 8), sizeof(S3C2440A_INTR_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) 
		{
			ERRORMSG(1,(TEXT("For INTRregs: VirtualCopy failed!\r\n")));
		}
	}

}

BOOL WINAPI  
DllEntry(HANDLE	hinstDLL, DWORD dwReason, LPVOID  Reserved/* lpvReserved */)
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


BOOL INT_Deinit(DWORD hDeviceContext)
{
	BOOL bRet = TRUE;
	
	RETAILMSG(1,(TEXT("USERKEY: INT_Deinit\r\n")));

	for (int i=0; i<1; i++)
		InterruptDisable(g_KeySysIntr[i]);

	CloseHandle(IntThread);
	CloseHandle(IntEvent);

	VirtualFree((void*)v_pIOPregs, sizeof(S3C2440A_IOPORT_REG), MEM_RELEASE);
	VirtualFree((void*)v_pINTRregs, sizeof(S3C2440A_INTR_REG), MEM_RELEASE);

	return TRUE;
} 

BOOL Eint_GPIO_Init()
{
	RETAILMSG(1,(TEXT("INT_GPIO_Setting----\r\n")));



	v_pIOPregs->GPFCON  &= ~(0x3 << 10);		/* Set EINT0(GPF5) as EINT4							*/
	v_pIOPregs->GPFCON  |=  (0x2 << 10);


	v_pINTRregs->INTMSK &= ~(1<<4);

	v_pIOPregs->EINTMASK  &= (0x0 << 5);

	v_pIOPregs->EXTINT0 &= ~(0x7 << 20);		/* Configure EINT5 as Falling Edge Mode				*/
	v_pIOPregs->EXTINT0 |=  (0x0 << 20);

	v_pIOPregs->GPCCON  = (v_pIOPregs->GPCCON  &~(3 << 10)) | (1<< 10);	// GPC5 == OUTPUT.
	v_pIOPregs->GPCCON  = (v_pIOPregs->GPCCON  &~(3 << 12)) | (1<< 12);	// GPC6 == OUTPUT.
	v_pIOPregs->GPCCON  = (v_pIOPregs->GPCCON  &~(3 << 14)) | (1<< 14);	// GPC7 == OUTPUT.
	Sleep(300);
	v_pIOPregs->GPCDAT=v_pIOPregs->GPCDAT|(0x7<<5);
	Sleep(300);
	v_pIOPregs->GPCDAT=v_pIOPregs->GPCDAT&~(0x7<<5);
	Sleep(300);
	v_pIOPregs->GPCDAT=v_pIOPregs->GPCDAT|(0x7<<5);

	return TRUE;
}

DWORD INT_Init(DWORD dwContext)
{
	DWORD       threadID;                         // thread ID
	RETAILMSG(1,(TEXT("INT_Init----\r\n")));

	// 1. Virtual Alloc
	Virtual_Alloc();

	Eint_GPIO_Init();

	IntThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)IntProcessThread, 0, 0, &threadID);

	if (NULL == IntThread )
	{
		RETAILMSG(1,(TEXT("ERROR: failed to Create Key Thread!\r\n")));
		return FALSE;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL INT_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
	return TRUE;
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
DWORD INT_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
	return TRUE;
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL INT_Close(DWORD hOpenContext)
{
	return TRUE;
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void INT_PowerDown(DWORD hDeviceContext)
{
	
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void INT_PowerUp(DWORD hDeviceContext)
{
	RETAILMSG(1,(TEXT("USERKEY: INT_PowerUp\r\n")));
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
DWORD INT_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
	RETAILMSG(1,(TEXT("USERKEY: INT_Read\r\n")));
	return TRUE;
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
DWORD INT_Seek(DWORD hOpenContext, long Amount, DWORD Type)
{
	return 0;
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
DWORD INT_Write(DWORD hOpenContext, LPCVOID pSourceBytes, DWORD NumberOfBytes)
{
	return 0;
}

