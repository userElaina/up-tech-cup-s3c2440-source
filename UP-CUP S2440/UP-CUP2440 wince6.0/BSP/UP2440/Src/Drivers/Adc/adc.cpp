// ADC.cpp : Defines the entry point for the DLL application.
//

#include <windows.h>
#include <tchar.h>
#include <nkintr.h>
#include <string.h>
//#include "d:\WINCE420\PUBLIC\COMMON\OAK\INC\pkfuncs.h"
//#include "d:\WINCE420\PLATFORM\UT2410X\INC\s2410.h"
//#include "d:\WINCE420\PLATFORM\UT2410X\INC\oalintr.h"
#include "pkfuncs.h"
#include "s3c2440a.h"
#include "oal_intr.h"
//#include "adc.h"


HANDLE g_hevInterrupt;//中断事件句柄
HANDLE g_htIST;
DWORD dwThreadID;
//volatile INTreg *pINTreg;
volatile S3C2440A_INTR_REG *pINTreg;
struct ADC_DEV{

	CRITICAL_SECTION	ADC_CS;//临界区
	volatile S3C2440A_ADC_REG 		*pADCreg;// 指向ADC寄存器的指针
	char					channel;//通道选择
	char					prescale;//预分频
	DWORD				OpenCnt;		// @field Protects use of this port 
	
}adcdev;
struct ADC_DEV *pADC_Dev;



//necessary entry in every DLL file
BOOL WINAPI DllEntry ( HANDLE hinstDLL, 
							DWORD  dwReason, 
							LPVOID lpvReserved)
{	
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
	//		RETAILMSG(1,(TEXT("ADC:DLL_PROCESS_ATTACH\t\n")));
			return TRUE;
		case DLL_THREAD_ATTACH:
	//		RETAILMSG(1,(TEXT("ADC:DLL_THREAD_ATTACH\t\n")));
			break;
		case DLL_THREAD_DETACH:
	//		RETAILMSG(1,(TEXT("ADC:DLL_THREAD_DETACH\t\n")));
			break;
		case DLL_PROCESS_DETACH:
	//		RETAILMSG(1,(TEXT("ADC:DLL_PROCESS_DETACH\t\n")));
			break;
	#ifdef UNDER_CE
		case DLL_PROCESS_EXITING:
	//		RETAILMSG(1,(TEXT("ADC:DLL_PROCESS_EXITING\t\n")));
			break;
		case DLL_SYSTEM_STARTED:
	//		RETAILMSG(1,(TEXT("ADC:DLL_SYSTEM_STARTED\t\n")));
			break;
	#endif
		default:
			break;
	}
    return TRUE;
}

DWORD WINAPI AdcEventHandler(LPVOID a)//
{
		RETAILMSG(1,(TEXT("ADCEvent Handler,Waiting for Interrupution!\r\n")));
		////////////////////////Only for Test////////////////////////////
//		TCHAR RegValue1[20];
//		TCHAR RegValue2[20];
//		TCHAR MSGString[60];

		WaitForSingleObject(g_hevInterrupt,INFINITE);
//		RETAILMSG(1,(TEXT("Interruption has happend! Exit ADC Handler\r\n")));
		InterruptDone(IRQ_ADC);
		
		return 0;

}

void ADC_Init (void)
{
	pADC_Dev=&adcdev;
	//pADC_Dev->pADCreg=(volatile ADCreg *)ADC_BASE;
	
	//////////////////////////////////////
	
	pADC_Dev->pADCreg = (volatile S3C2440A_ADC_REG *)VirtualAlloc(0, sizeof(S3C2440A_ADC_REG), MEM_RESERVE, PAGE_NOACCESS);
	//pADC_Dev->pADCreg=(volatile S3C2440A_ADC_REG *)S3C2440A_BASE_REG_PA_ADC;

		if (pADC_Dev->pADCreg != NULL) 
		{
			
			if (!VirtualCopy((PVOID)pADC_Dev->pADCreg, (PVOID)(S3C2440A_BASE_REG_PA_ADC>>8), sizeof(S3C2440A_ADC_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) 
			{
				VirtualFree((PVOID) pADC_Dev->pADCreg, 0, MEM_RELEASE);
				RETAILMSG(1,(TEXT("For ADC_ADCregs: VirtualCopy failed!\r\n")));
				
			}
			else
			{
				
				RETAILMSG(1,(TEXT("FOR ADC_ADCregs: VirtualCopy Success\r\n")));
				
			}
				
		}
	///////////////////////////////////////
	
	//pINTreg=(volatile INTreg*)INT_BASE;
	RETAILMSG(1,(TEXT("ADCDRV.DLL: InitAddINTreg2\r\n")));
		
	
		pINTreg = (volatile S3C2440A_INTR_REG *)VirtualAlloc(0, sizeof(S3C2440A_INTR_REG), MEM_RESERVE, PAGE_NOACCESS);
		//pINTreg = (volatile S3C2440A_INTR_REG *)S3C2440A_BASE_REG_PA_INTR;
		if (pINTreg != NULL) 
		{
			
			if (!VirtualCopy((PVOID)pINTreg, (PVOID)(S3C2440A_BASE_REG_PA_INTR>>8), sizeof(S3C2440A_INTR_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) 
			{
				VirtualFree((PVOID) pINTreg, 0, MEM_RELEASE);
				RETAILMSG(1,(TEXT("For ADC_INTregs: VirtualCopy failed!\r\n")));
				
			}
			else
			{
				
				RETAILMSG(1,(TEXT("FOR ADC_INTregs: VirtualCopy Success\r\n")));
				
			}
				
		}
	///////////////////////////////////////////////
	InitializeCriticalSection(&(pADC_Dev->ADC_CS));
	
	/////////////////////Set the Rigisters of ADC//////////////////////////////
	pADC_Dev->pADCreg->ADCCON&=0x8000;//Enable Prescale
	//pADC_Dev->pADCreg->rADCCON&=0xfff8;//Normal operation mode,Disable start by read operation,No operation;
	////////////////////Set the Rigisters of ADC Over//////////////////////////

		////////////////Initialize Event/////////////////
	g_hevInterrupt = CreateEvent(NULL,FALSE,FALSE,NULL);//create event

	if(g_hevInterrupt==NULL)
	{
		RETAILMSG(1,(TEXT("Event creation failed!\r\n")));
		return;
	}

	//注册中断
	if(!InterruptInitialize(IRQ_ADC,g_hevInterrupt,NULL,0))
	{
		RETAILMSG(1,(TEXT("InterruptInitialize failed!\r\n")));
		return;
	}
		////////////////Initialize Event Over//////////////

	
}
/////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////
DWORD ADC_Open(HANDLE hDeviceContext,
			   DWORD AccessCode,
			   DWORD ShareMode)
{
	RETAILMSG(1,(TEXT("ADC_DRV:OPEN SUCCESS!\r\n")));
	return(1);
}

BOOL ADC_Close(DWORD hOpenContext)
{
	return TRUE;
}


DWORD ADC_Deinit(DWORD dwDeviceContext)
{
	return FALSE;
}


BOOL ADC_IOControl(DWORD hOpenContext,
					DWORD dwCode,
					PBYTE pBufIn,
					DWORD dwLenIn,
					PBYTE pBufOut,
					DWORD dwLenOut,
					PDWORD pdwActualOut)
{
	return(TRUE);
}

////////////OK
DWORD ADC_Read(DWORD hOpenContext,
			   LPVOID pBuffer,
			   DWORD Count)
{
	//int m_nISTPriority;
	DWORD ReturnValue;
	//RETAILMSG(1,(TEXT("ADC Read!\r\n")));
	////////////////Initialize Thread/////////////////
	g_htIST= CreateThread(NULL,
						  0,
						  AdcEventHandler,
						  NULL,
						  CREATE_SUSPENDED,
						  &dwThreadID);
	//设置优先级
	if(!CeSetThreadPriority(g_htIST,251))
	{
		RETAILMSG(1,(TEXT("ADC_READ: Failed setting Thread Priority.\r\n")));
		return 0;
	}
	//恢复IST
	ResumeThread(g_htIST);
	///////////////Initialize Thread Over///////////////
	
	///////////////////Enable ADC//////////////////
	
	EnterCriticalSection(&(pADC_Dev->ADC_CS));
//	RETAILMSG(1,(TEXT("ADC_READ: Entered Critical Section!\r\n")));
//	RETAILMSG(1,(TEXT("ADC_READ: ADCCON '%x'\r\n"),pADC_Dev->pADCreg->rADCCON));
	
	pADC_Dev->pADCreg->ADCCON|= 1;//Enable Convertion

	LeaveCriticalSection(&(pADC_Dev->ADC_CS));
//	RETAILMSG(1,(TEXT("ADC_READ: AD Conversion Enabled! Leave Critical Section!\r\n ")));
 

//	RETAILMSG(1,(TEXT("ADC_READ: ADCCON '%x'\r\n"),pADC_Dev->pADCreg->rADCCON));

	
	///////////////////Enable ADC Over/////////////

	///////////////////Wait for the ThreadObject////////////////
//	RETAILMSG(1,(TEXT("ADC_READ: Waiting For Event!\r\n")));
	ReturnValue=WaitForSingleObject(g_htIST ,50);
	
	//////////////////Wait for the Thread Object Over////////////

	if(ReturnValue==WAIT_OBJECT_0)
	{
		///////////////////Write Buffer/////////////////////////
		
		EnterCriticalSection(&(pADC_Dev->ADC_CS));

		*((DWORD *)pBuffer)=pADC_Dev->pADCreg->ADCDAT0;
		(*((DWORD *)pBuffer))&=0x03ff;

		
		LeaveCriticalSection(&(pADC_Dev->ADC_CS));

		//////////////////Write Buffer Over/////////////////////

		return 4;
	}
	else if(ReturnValue==WAIT_TIMEOUT)
	{
//		RETAILMSG(1,(TEXT("ADC_READ: Read Timeout! ADCDAT0:%x\r\n"),pADC_Dev->pADCreg->rADCDAT0&0x3ff));
//		RETAILMSG(1,(TEXT("ADC_READ: Thread Terminated!")));
		*((DWORD *)pBuffer)=pADC_Dev->pADCreg->ADCDAT0;
		(*((DWORD *)pBuffer))&=0x03ff;
		TerminateThread(g_htIST,0);
		return 1;
	}
	else
	{
		RETAILMSG(1,(TEXT("ADC_READ: Do Not Know What Was Wrong!\r\n")));
		return 0;
	}
}

///////////OK
DWORD ADC_Write(DWORD hOpenContext,
				LPCVOID pSourceBytes,
				DWORD NumberOfBytes)
{
	DWORD *ADCControl;
	EnterCriticalSection(&(pADC_Dev->ADC_CS));//用于多个线程操作相同的数据 互斥变量

	
	ADCControl=(DWORD *)pSourceBytes;
	if(ADCControl)
	{
	pADC_Dev->pADCreg->ADCCON &= 0x8000;
	pADC_Dev->pADCreg->ADCCON|= 0x4000;
	pADC_Dev->pADCreg->ADCCON|=((*ADCControl)&0xff)<<6;
//	pADC_Dev->pADCreg->ADCCON|=((*(++ADCControl))&7)<<3;
	pADC_Dev->pADCreg->ADCCON|=(((*ADCControl)>>16)&0x7)<<3;
	}
	LeaveCriticalSection(&(pADC_Dev->ADC_CS));//用于多个线程操作相同的数据 互斥变量

	 
	return TRUE;
}

/////////OK
DWORD ADC_Seek(DWORD hOpenContext,
			   long Amount,
			   DWORD Type)
{
	return FALSE;
}
