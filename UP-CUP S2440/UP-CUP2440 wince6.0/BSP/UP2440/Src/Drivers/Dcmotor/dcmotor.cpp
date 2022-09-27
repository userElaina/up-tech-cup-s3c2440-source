#include "windows.h"
#include "s3c2440a.h"



#define DCMOTOR_IOCTRL_RUN	1
#define DCMOTOR_IOCTRL_ANTIRUN 2
#define DCMOTOR_IOCTRL_STOP	3

#define DCM_TCNTB0			16384

volatile S3C2440A_PWM_REG *g_pPWMreg=NULL;
volatile S3C2440A_IOPORT_REG *g_pIOPregs = NULL;



bool   
DllEntry(HANDLE hinstdll, 
              DWORD dwreason, 
              LPVOID  lpvreserved )
{
	
    return true;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool DCM_Close(DWORD hopencontext)
{
    bool bret = true;
    
    return bret;
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool DCM_Deinit(DWORD hdevicecontext)
{
    bool bret = true;
    
    return bret;
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
DWORD DCM_Init(DWORD dwcontext)
{
	g_pIOPregs = (volatile S3C2440A_IOPORT_REG *)VirtualAlloc(0, sizeof(S3C2440A_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);

		if (g_pIOPregs == NULL) 
		{
				
			RETAILMSG(1,(TEXT("For SPI_IOPregs : VirtualAlloc failed!\r\n")));
			return 0;
		}
		else 
		{
			if (!VirtualCopy((PVOID)g_pIOPregs, (PVOID)(S3C2440A_BASE_REG_PA_IOPORT>>8), sizeof(S3C2440A_IOPORT_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) 
			{
				RETAILMSG(1,(TEXT("For SPI_IOPregs2: VirtualCopy failed!\r\n")));
				return 0;
			}
		}



	
	g_pPWMreg=(S3C2440A_PWM_REG *)VirtualAlloc(0,sizeof(S3C2440A_PWM_REG),MEM_RESERVE, PAGE_NOACCESS);
	if(!g_pPWMreg)
	{
		RETAILMSG(1,(TEXT("SPIDRV.DLL: PWMreg virualalloc failed\r\n")));
		return 0;
		
	}
	if(!VirtualCopy((PVOID)g_pPWMreg,(PVOID)(S3C2440A_BASE_REG_PA_PWM>>8),sizeof(S3C2440A_PWM_REG),PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE ))
	{
		RETAILMSG(1,(TEXT("SPIDRV.DLL:PWMreg virtualcopy failed! \r\n")));
		return 0;
	}

	g_pIOPregs->GPBCON &= ~0xF; //配置GPB0 GPB1为输出方式
	g_pIOPregs->GPBCON |= 0x5;
	g_pIOPregs->GPBDAT=g_pIOPregs->GPBDAT&~(0x1<<0);//同时输出1确保点击初始不转
	g_pIOPregs->GPBDAT=g_pIOPregs->GPBDAT&~(0x1<<1);



    return 1;
    
} 

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
DWORD DCM_IOControl(
    DWORD Handle,
    DWORD dwIoControlCode,
    PBYTE pInBuf,
    DWORD nInBufSize,
    PBYTE pOutBuf,
    DWORD nOutBufSize,
    PDWORD pBytesReturned
    )
{
	
    switch(dwIoControlCode)
    {
		case DCMOTOR_IOCTRL_RUN://正转
				RETAILMSG(1,(TEXT("DCMOTOR DLL IOCONTROL\r\n")));
				g_pPWMreg->TCFG0 &= ~(0x00ff0000);
				g_pPWMreg->TCFG0 |= 2;
				g_pPWMreg->TCFG1 &= ~(0xF);
				g_pPWMreg->TCNTB0=16384;
				g_pPWMreg->TCMPB0=16384/2;
				g_pPWMreg->TCON &= ~(0xf);
				g_pPWMreg->TCON |= 0x2;
				g_pPWMreg->TCON &= ~(0xf);
				g_pPWMreg->TCON |= 0x19;

				g_pIOPregs->GPBCON &= ~0xF;
				g_pIOPregs->GPBCON |= 0xA;//GPB0/1配成TOUT方式
				g_pPWMreg->TCMPB0 = DCM_TCNTB0/2 + (*((int *)pInBuf))*(DCM_TCNTB0/1024);

				break;
		case DCMOTOR_IOCTRL_ANTIRUN://反转
				g_pPWMreg->TCFG0 &= ~(0x00ff0000);
				g_pPWMreg->TCFG0 |= 2;
				g_pPWMreg->TCFG1 &= ~(0xF);
				g_pPWMreg->TCNTB0=16384;
				g_pPWMreg->TCMPB0=16384/2;
				g_pPWMreg->TCON &= ~(0xf);
				g_pPWMreg->TCON |= 0x2;
				g_pPWMreg->TCON &= ~(0xf);
				g_pPWMreg->TCON |= 0x19;

				g_pIOPregs->GPBCON &= ~0xF;
				g_pIOPregs->GPBCON |= 0xA;
				g_pPWMreg->TCMPB0 = DCM_TCNTB0/2 + (*((int *)pInBuf))*(DCM_TCNTB0/1024);
				break;
		case DCMOTOR_IOCTRL_STOP ://停止
				g_pIOPregs->GPBCON &= ~0xF;
				g_pIOPregs->GPBCON |= 0x5;
				g_pIOPregs->GPBDAT=g_pIOPregs->GPBDAT&~(0x1<<0);
				g_pIOPregs->GPBDAT=g_pIOPregs->GPBDAT&~(0x1<<1);
				g_pPWMreg->TCMPB0=DCM_TCNTB0/2+0;
				
				break;
				
		default :
		      return 0;
    }
    return 1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
DWORD DCM_Open(DWORD hdevicecontext, DWORD accesscode, DWORD sharemode)
{
    DWORD dwret = 0;
    

    dwret = 1; // have to make non-zero for this call to succeed.
    return dwret;
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void DCM_Powerdown(DWORD hdevicecontext)
{
    
} 

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void DCM_Powerup(DWORD hdevicecontext)
{
    
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
DWORD DCM_Read(DWORD hopencontext,LPVOID pbuffer, DWORD count)
{
    DWORD dwret = 0;
    
    return dwret;
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
DWORD DCM_Seek(DWORD hopencontext, long amount, DWORD type)
{
    DWORD dwret = 0;
    
    return dwret;
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
DWORD DCM_Write(DWORD hopencontext, LPVOID psourcebytes, DWORD numberofbytes)
{
    DWORD dwret = 0;
    

    return dwret;
}

