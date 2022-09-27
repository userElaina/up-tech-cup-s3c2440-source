
#include <windows.h>

#include <nkintr.h>
//#include <oalintr.h>
#include <pm.h>
#include "pmplatform.h"
#include <ceddk.h>

#include <s3c2440a.h>
#include <bsp.h>
#include "camif.h"
#include "camera.h"
#include "s3c2440a_lcd.h"

//Ìí¼ÓCIS_OV9650 Ö§³Ö
#if (CIS_TYPE == CIS_OV9650) 
	#include "ov9650.h"
#else
	#include "s5x532.h"
#endif

#define PM_MSG			0
#define I2C_MSG			0
#define MSG_EN_1		0
#define MSG_EN_2		0

#define CAPTURE_TIME		30

#define CAM_CLOCK_OFF_ON_IDLE	1

#if (CAM_CLOCK_OFF_ON_IDLE == 1)
#define DISPLAY_THREAD_TIMEOUT	3000
#else
#define DISPLAY_THREAD_TIMEOUT	INFINITE
#endif

#define U8	unsigned char
#define U16	unsigned short
#define U32	unsigned int

#define	PORT_A 1
#define	PORT_B 0

#define	YCbCrtoR(Y,Cb,Cr)	(1000*Y + 1540*(Cr-128))/1000
#define	YCbCrtoG(Y,Cb,Cr)	(1000*Y - 459*(Cb-128) - 183*(Cr-128))/1000
#define	YCbCrtoB(Y,Cb,Cr)	(1000*Y + 1856*(Cb-128))/1000

volatile S3C2440A_IOPORT_REG *s2440IOP = NULL;
volatile S3C2440A_CAM_REG    *s2440CAM = NULL;
volatile S3C2440A_INTR_REG	*s2440INT = NULL;
volatile S3C2440A_CLKPWR_REG *s2440PWR = NULL;

unsigned int dwDisplayTimeout = DISPLAY_THREAD_TIMEOUT;

BOOL bIdlePwrDown = FALSE;

unsigned char buffer_num=0xff;		// ping pong buffer

unsigned char image_size = 2;				// 1: QCIF, 2:CIF (default)
//unsigned char image_size = 1;				// 1: QCIF, 2:CIF (default)

//#define DRIVER_PREVIEW_ENABLE		2	// 0: onTimeTek, 1: Preview, 2:Samsung Camcorder
unsigned char DRIVER_PREVIEW_ENABLE = 1;	// 0: onTimeTek, 1: Preview, 2:Samsung Camcorder	

unsigned int frame_count=0;			// for MPEG4
DWORD Tick_GET_FRAME_CUR;
DWORD Tick_GET_FRAME_PREV;
DWORD Tick_COPY_FRAME;

CAMINFO sCAMINFO;

UINT32 g_CamIrq = IRQ_CAM;	// Determined by SMDK2440 board layout.
UINT32 g_CamSysIntr = SYSINTR_UNDEFINED;

HANDLE CameraThread;
HANDLE CameraEvent;
static BOOL mInitialized = FALSE;

unsigned char codec_flag=0;
unsigned char rgb_flag=0;
unsigned int y_address,cb_address,cr_address;
unsigned int rgb_address;

// PingPong Memory Physical Address 
PHYSICAL_ADDRESS g_PhysPreviewAddr;
PHYSICAL_ADDRESS g_PhysCodecAddr;

int CAM_WriteBlock(void);

void Virtual_Alloc();						// Virtual allocation
void Camera_Clock(unsigned char divide);	// set default value
void Camif_Capture(int cap_a, int cap_b);	// A port, B port
void Display_Cam_Image(U32 pos_x, U32 pos_y, U32 size_x, U32 size_y, U8 port);
void Display_Cam_Image2(U32 size_x, U32 size_y, U8 port);
void Copy_Cam_Image(U8 * pBufOut, U32 size_x, U32 size_y, U8 port);
void Samsung_camcoder(U8 *pBufOut);
void Samsung_camcoder_pr(U8 *pBufOut);
void Buffer_codec_info_update();
void Buffer_preview_info_update();
BOOL Cam_Init();
BOOL CamClockOn(BOOL bOnOff);

void CamInit(U32 CoDstWidth, U32 CoDstHeight, U32 PrDstWidth, U32 PrDstHeight, 
			U32 WinHorOffset, U32 WinVerOffset,  U32 CoFrameBuffer, U32 PrFrameBuffer);

DWORD CameraCaptureThread(void);
BOOL InitInterruptThread();

CEDEVICE_POWER_STATE m_Dx;

#ifdef DEBUG
#define ZONE_ERR        DEBUGZONE(15)
#define ZONE_FUNCTION   DEBUGZONE(13)
#define ZONE_THREAD     0

DBGPARAM dpCurSettings = {
        TEXT("CAMERA"), {
        TEXT("0"),TEXT("1"),TEXT("2"),TEXT("3"),
        TEXT("4"),TEXT("5"),TEXT("6"),TEXT("7"),
        TEXT("8"),TEXT("9"),TEXT("10"),TEXT("11"),
        TEXT("12"),TEXT("Function"),TEXT("Init"),TEXT("Error")},
        0x8000  // Errors only, by default
}; 
#endif


PCIS_CONTEXT pCIS;

CRITICAL_SECTION m_Lock;

void Lock()   {EnterCriticalSection(&m_Lock);}
void Unlock() {LeaveCriticalSection(&m_Lock);}


static void Delay(USHORT count)
{
	volatile int i, j = 0;
	volatile static int loop = S3C2440A_FCLK/100000;
	
	for(;count > 0;count--)
		for(i=0;i < loop; i++) { j++; }
}


// 
// Read PCF50606 registers directly into our cache
//
DWORD
HW_ReadRegisters(
    PCIS_CONTEXT pCIS,
    PUCHAR pBuff,       // Optional buffer
    UCHAR StartReg,     // Start Register
    DWORD nRegs         // Number of Registers
    )
{
	DWORD dwErr;


	EnterCriticalSection(&pCIS->RegCS);

	// use the driver-to-driver call
	dwErr = pCIS->fc.I2CRead(pCIS->fc.Context,
	                         CAMERA_READ, 	// SlaveAddress
	                         StartReg,      // WordAddress
	                         !pBuff ? (PUCHAR)&pCIS->Reg[StartReg] : pBuff,
	                         nRegs);
	
	if ( !dwErr ) {


	} else {        
		RETAILMSG(1,(TEXT("I2CRead ERROR: %u \r\n"), dwErr));
		//DEBUGMSG(ZONE_ERR,(TEXT("I2CRead ERROR: %u \r\n"), dwErr));
	}            

	LeaveCriticalSection(&pCIS->RegCS);
	
	return dwErr;
}


// 
// Write PCF50606 registers directly from our cache
//
DWORD
HW_WriteRegisters(
    PCIS_CONTEXT pCIS,
    PUCHAR pBuff,   // Optional buffer
    UCHAR StartReg, // start register
    DWORD nRegs     // number of registers
    )
{
	DWORD dwErr;


	EnterCriticalSection(&pCIS->RegCS);

	// use the driver-to-driver call
	dwErr = pCIS->fc.I2CWrite(pCIS->fc.Context,
	                          CAMERA_WRITE,   	// SlaveAddress
	                          StartReg,         // WordAddress
	                          !pBuff ? (PUCHAR)&pCIS->Reg[StartReg] : pBuff,
	                          nRegs);

	if ( dwErr ) {
		RETAILMSG(1, (TEXT("I2CWrite ERROR: %u \r\n"), dwErr));
		//DEBUGMSG(ZONE_ERR, (TEXT("I2CWrite ERROR: %u \r\n"), dwErr));
	}            

	LeaveCriticalSection(&pCIS->RegCS);
	
	return dwErr;
}

int CAM_WriteBlock(void)
{
	int i;
	unsigned char rdata[256] = { 0, }; 
	//int err = 0;
#if (CIS_TYPE == CIS_S5X3A1)
	for(i=0; i<(sizeof(S5X3A1_YCbCr8bit)/2); i++)
	{
		//HW_WriteRegisters(&S5X3A1_YCbCr8bit[i][1], S5X3A1_YCbCr8bit[i][0], 1);
		HW_WriteRegisters(pCIS, &S5X3A1_YCbCr8bit[i][1], S5X3A1_YCbCr8bit[i][0], 1);
	}
#elif (CIS_TYPE == CIS_S5X3AA) 
	for(i=0; i<(sizeof(S5X3AA_YCbCr8bit)/2); i++)
	{
		//HW_WriteRegisters(&S5X3AA_YCbCr8bit[i][1], S5X3AA_YCbCr8bit[i][0], 1);
		HW_WriteRegisters(pCIS, &S5X3AA_YCbCr8bit[i][1], S5X3AA_YCbCr8bit[i][0], 1);
	}
#elif (CIS_TYPE == CIS_OV9650) 
	for(i=0; i<(sizeof(OV9650Reg)/2); i++)
	{
		if (OV9650Reg[i][0] == CHIP_DELAY) Delay(OV9650Reg[i][1]);
		else HW_WriteRegisters(pCIS, &OV9650Reg[i][1], OV9650Reg[i][0], 1);
	}
	i = 0;
	HW_ReadRegisters(pCIS, (PUCHAR)&i, 0xB, 1);
	HW_ReadRegisters(pCIS, (PUCHAR)&i+1, 0xA, 1);
	RETAILMSG(1,(TEXT("Camera::Product id is 0x%x\r\n"),i));
#endif
	
	RETAILMSG(MSG_EN_1,(TEXT("Camera::Block TX Ended...\r\n")));
	return i;
}

/********************************************************
 CalculateBurstSize - Calculate the busrt lengths
 
 Description:	
 - dstHSize: the number of the byte of H Size.
 
*/
void CalculateBurstSize(unsigned int hSize,unsigned int *mainBurstSize,unsigned int *remainedBurstSize)
{
	unsigned int tmp;	
	tmp=(hSize/4)%16;
	switch(tmp) {
		case 0:
			*mainBurstSize=16;
			*remainedBurstSize=16;
			break;
		case 4:
			*mainBurstSize=16;
			*remainedBurstSize=4;
			break;
		case 8:
			*mainBurstSize=16;
			*remainedBurstSize=8;
			break;
		default: 
			tmp=(hSize/4)%8;
			switch(tmp) {
				case 0:
					*mainBurstSize=8;
					*remainedBurstSize=8;
					break;
				case 4:
					*mainBurstSize=8;
					*remainedBurstSize=4;
				default:
					*mainBurstSize=4;
					tmp=(hSize/4)%4;
					*remainedBurstSize= (tmp) ? tmp: 4;
					break;
			}
			break;
	}		    	    		
}

/********************************************************
 CalculatePrescalerRatioShift - none
 
 Description:	
 - none
 
*/
void CalculatePrescalerRatioShift(unsigned int SrcSize, unsigned int DstSize, unsigned int *ratio,unsigned int *shift)
{
	if(SrcSize>=64*DstSize) {
//		Uart_Printf("ERROR: out of the prescaler range: SrcSize/DstSize = %d(< 64)\r\n",SrcSize/DstSize);
		while(1);
	}
	else if(SrcSize>=32*DstSize) {
		*ratio=32;
		*shift=5;
	}
	else if(SrcSize>=16*DstSize) {
		*ratio=16;
		*shift=4;
	}
	else if(SrcSize>=8*DstSize) {
		*ratio=8;
		*shift=3;
	}
	else if(SrcSize>=4*DstSize) {
		*ratio=4;
		*shift=2;
	}
	else if(SrcSize>=2*DstSize) {
		*ratio=2;
		*shift=1;
	}
	else {
		*ratio=1;
		*shift=0;
	}    	
}

void Camera_Clock(unsigned char divide)
{
	unsigned int camclk;

	// Set camera clock for camera processor
	s2440PWR->CAMDIVN = (s2440PWR->CAMDIVN & ~(0x1f)) | (1<<4) | (divide & 0xf); // CAMCLK is divided..
	camclk = 48000000/(((s2440PWR->CAMDIVN&0xf) + 1)*2);

	RETAILMSG(1,(TEXT("CAM clock:%d\r\n"),camclk));
}


void Camif_Capture(int cap_a, int cap_b)
{
	//RETAILMSG(1,(_T("Camif_Capture(%d, %d)\r\n"), cap_a, cap_b));
	// S3C2440A

	s2440CAM->CIIMGCPT &= ~(CAM_CAMIF_GLOBAL_CAPTURE_ENABLE_BIT);

	if(cap_b == CAPTURE_ON) 
	{
		//Codec capture start
		s2440CAM->CICOSCCTRL |=(CAM_CODEC_SACLER_START_BIT);
		s2440CAM->CIIMGCPT |=(CAM_CAMIF_GLOBAL_CAPTURE_ENABLE_BIT)|(CAM_CODEC_SCALER_CAPTURE_ENABLE_BIT);
	} 
	else if (cap_b == CAPTURE_OFF)
	{
		s2440CAM->CICOSCCTRL &= ~(CAM_CODEC_SACLER_START_BIT);
		s2440CAM->CIIMGCPT &= ~(CAM_CODEC_SCALER_CAPTURE_ENABLE_BIT);
	}

	if(cap_a == CAPTURE_ON) 
	{  
		// Preview capture Start
		s2440CAM->CIPRSCCTRL |=(CAM_PVIEW_SACLER_START_BIT);
		s2440CAM->CIIMGCPT |=(CAM_CAMIF_GLOBAL_CAPTURE_ENABLE_BIT)|(CAM_PVIEW_SCALER_CAPTURE_ENABLE_BIT);
	}
	else if (cap_a == CAPTURE_OFF)
	{
		s2440CAM->CIPRSCCTRL &= ~(CAM_PVIEW_SACLER_START_BIT);
		s2440CAM->CIIMGCPT &= ~(CAM_PVIEW_SCALER_CAPTURE_ENABLE_BIT);
	}
	
	if ((cap_a == CAPTURE_ON) || (cap_b == CAPTURE_ON))
	{
		s2440CAM->CIIMGCPT |= (CAM_CAMIF_GLOBAL_CAPTURE_ENABLE_BIT);
	}
	
}



void CamInterface_PowerDown(void)
{
	RETAILMSG(PM_MSG, (_T("CamInterface_PowerDown()\r\n")));

	// Camera clock off
	CamClockOn(FALSE);

	// GPIO control
	// for power down mode, gpio goes to input mode
	s2440IOP->GPJCON = (s2440IOP->GPJCON & ~(0xffffff)) | 0x555555; // CAM IO setup except CAMRESET
	s2440IOP->GPJDAT = s2440IOP->GPJDAT & ~(0xfff); 	// CAM IO DATA pin setup except CAMRESET
	s2440IOP->GPJUP = (s2440IOP->GPJUP | 0xfff); 		// CAM IO PullUp setup except CAMRESET
	//s2440IOP->GPJCON = (s2440IOP->GPJCON & ~(0x3<<22)) | (0x0<<22); // CAMCLKOUT -> input
	
}

void CamInterface_PowerUp(void)
{
	RETAILMSG(PM_MSG, (_T("CamInterface_PowerUp()\r\n")));

	// GPIO control
	s2440IOP->GPJCON = (s2440IOP->GPJCON & ~(0xffffff)) | 0xaaaaaa; // CAM IO setup except CAMRESET
	s2440IOP->GPJDAT = s2440IOP->GPJDAT & ~(0xfff); 	// CAM IO DATA pin setup except CAMRESET
	s2440IOP->GPJUP = (s2440IOP->GPJUP | 0xfff); 		// CAM IO PullUp setup except CAMRESET
	//s2440IOP->GPJCON = (s2440IOP->GPJCON & ~(0x3<<22)) | (0x2<<22); // CAMCLKOUT -> input

	Delay(10);

	// Camera clock on
	CamClockOn(TRUE);
	
	Delay(100);
}


DWORD CameraCaptureThread(void)
{
	unsigned char tmp=0;
	static unsigned int time,old_time;
	static unsigned int cam_intr;
	
	DWORD	dwCause;

	//dwDisplayTimeout = INFINITE;

	SetProcPermissions((DWORD)-1);
	
	while(TRUE)
	{
		RETAILMSG(0,(TEXT("[CAM_HW] InterruptThread : Waiting For a Single Object\n\r")));
		
		dwCause = WaitForSingleObject(CameraEvent, dwDisplayTimeout);
		
		RETAILMSG(MSG_EN_1,(_T("CameraCaptureThread(%d)++\r\n"), frame_count));

#if 0
		if (frame_count <= 2) {
			//frame_count++;
			// Enable camera interrupt
			s2440INT->INTSUBMSK &= ~(( 1 << IRQ_SUB_CAM_P )|( 1 << IRQ_SUB_CAM_C ));
			s2440INT->INTMSK &= ~( 1 << IRQ_CAM );
			
			continue;
		}
#endif
		
		if (dwCause == WAIT_OBJECT_0)
		{
			Lock();

			__try
			{

				if (s2440INT->INTSUBMSK & ( 1 << IRQ_SUB_CAM_C ))
				{
					frame_count++;
					cam_intr |= ( 1 << IRQ_SUB_CAM_C );
				    
					s2440INT->SUBSRCPND  =  (1<<IRQ_SUB_CAM_C);
					s2440INT->INTSUBMSK &= ~(1<<IRQ_SUB_CAM_C);
					//RETAILMSG(1,(_T("CAM_C, ts %d\r\n"), GetTickCount()));
				}

				if (s2440INT->INTSUBMSK & ( 1 << IRQ_SUB_CAM_P ))
				{
					cam_intr |= ( 1 << IRQ_SUB_CAM_P );
					
					s2440INT->SUBSRCPND  =  (1<<IRQ_SUB_CAM_P);
					s2440INT->INTSUBMSK &= ~(1<<IRQ_SUB_CAM_P);
					//RETAILMSG(1,(_T("CAM_P, ts %d\r\n"), GetTickCount()));
				}

				if (((s2440INT->INTSUBMSK & ( 1 << IRQ_SUB_CAM_C )) == 0) && ((s2440INT->INTSUBMSK & ( 1 << IRQ_SUB_CAM_P )) == 0))
				{
					RETAILMSG(MSG_EN_1,(_T("[CAM]NOP\r\n")));					
				}
			
				InterruptDone(g_CamSysIntr);				
			
				//time = GetTickCount();
				//RETAILMSG(1,(TEXT("+time:%d\r\n"),(time - old_time)));
	
				// Handle any interrupts on the input source
				if (cam_intr & ( 1 << IRQ_SUB_CAM_P ))
				{
					// display the image	
					if (DRIVER_PREVIEW_ENABLE == 1)
					Display_Cam_Image(LCD_XSIZE_TFT-PREVIEW_X,0,PREVIEW_X, PREVIEW_Y, PORT_A);

					Buffer_preview_info_update();
					cam_intr &= ~( 1 << IRQ_SUB_CAM_P );
				}
					
				if (cam_intr & ( 1 << IRQ_SUB_CAM_C ))
				{
					Buffer_codec_info_update();
					cam_intr &= ~( 1 << IRQ_SUB_CAM_C );
				}

				// Enable camera interrupt
				//s2440INT->INTSUBMSK &= ~(( 1 << IRQ_SUB_CAM_P )|( 1 << IRQ_SUB_CAM_C ));
				//s2440INT->INTMSK &= ~( 1 << IRQ_CAM );

				//old_time = GetTickCount();
				//RETAILMSG(1,(TEXT("-time:%d\r\n"),(old_time-time)));

			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				RETAILMSG(PM_MSG, (TEXT("Camera.DLL:InterruptThread() - EXCEPTION: %d"), GetExceptionCode()));
			}
				
			Unlock();
			
		}
		else if (dwCause == WAIT_TIMEOUT)
		{
			Lock();
			
			RETAILMSG(PM_MSG,(_T("[CAM_HW] InterruptThread Timeout : %d msec\r\n"), dwDisplayTimeout));
			
			dwDisplayTimeout = INFINITE;				// reset timeout until Camera Interrupt occurs
			bIdlePwrDown = TRUE;					// Codec is off

			CamInterface_PowerDown();
			RETAILMSG(PM_MSG, (TEXT("[CAM_HW] InterruptThread : bIdlePwrDown = TRUE\r\n")));

			Unlock();
		}
		else
		{
			RETAILMSG(PM_MSG, (TEXT("[CAM_HW] InterruptThread : Exit %d, Cause %d\r\n"), GetLastError(), dwCause));
		}
	}

	return 0;
}



void Virtual_Alloc()
{

	PBYTE pVirtPreviewAddr = NULL;
	PBYTE pVirtCodecAddr = NULL;
	DMA_ADAPTER_OBJECT Adapter1, Adapter2;

	memset(&Adapter1, 0, sizeof(DMA_ADAPTER_OBJECT));
	Adapter1.InterfaceType = Internal;
	Adapter1.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

	memset(&Adapter2, 0, sizeof(DMA_ADAPTER_OBJECT));
	Adapter2.InterfaceType = Internal;
	Adapter2.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

	// Allocate a block of virtual memory (physically contiguous) for the DMA buffers.
	//
	pVirtPreviewAddr = (PBYTE)HalAllocateCommonBuffer(&Adapter1, Preview_Mem_Size, &g_PhysPreviewAddr, FALSE);
	if (pVirtPreviewAddr == NULL)
	{
		RETAILMSG(TRUE, (TEXT("Camera:Virtual_Alloc() - Failed to allocate DMA buffer for Preview.\r\n")));
	}

	pVirtCodecAddr = (PBYTE)HalAllocateCommonBuffer(&Adapter2, Codec_Mem_Size, &g_PhysCodecAddr, FALSE);
	if (pVirtCodecAddr == NULL)
	{
		RETAILMSG(TRUE, (TEXT("Camera:Virtual_Alloc() - Failed to allocate DMA buffer for Codec.\r\n")));
	}

	// GPIO Virtual alloc
	s2440IOP = (volatile S3C2440A_IOPORT_REG *) VirtualAlloc(0,sizeof(S3C2440A_IOPORT_REG),MEM_RESERVE, PAGE_NOACCESS);
	if(s2440IOP == NULL) {
		RETAILMSG(1,(TEXT("For s2440IOP: VirtualAlloc failed!\r\n")));
	}
	else {
		if(!VirtualCopy((PVOID)s2440IOP,(PVOID)(S3C2440A_BASE_REG_PA_IOPORT >> 8),sizeof(S3C2440A_IOPORT_REG),PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE )) {
			RETAILMSG(1,(TEXT("For s2440IOP: VirtualCopy failed!\r\n")));
		}
	}
	
	// Camera Virtual alloc
	s2440CAM = (volatile S3C2440A_CAM_REG *) VirtualAlloc(0,sizeof(S3C2440A_CAM_REG),MEM_RESERVE, PAGE_NOACCESS);
	if(s2440CAM == NULL) {
		RETAILMSG(1,(TEXT("For s2440CAM: VirtualAlloc failed!\r\n")));
	}
	else {
		if(!VirtualCopy((PVOID)s2440CAM,(PVOID)(S3C2440A_BASE_REG_PA_CAM >> 8),sizeof(S3C2440A_CAM_REG),PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE )) {
			RETAILMSG(1,(TEXT("For s2440CAM: VirtualCopy failed!\r\n")));
		}
	}

	// Interrupt Virtual alloc
	s2440INT = (volatile S3C2440A_INTR_REG *) VirtualAlloc(0,sizeof(S3C2440A_INTR_REG),MEM_RESERVE, PAGE_NOACCESS);
	if(s2440INT == NULL) {
		RETAILMSG(1,(TEXT("For s2440INT: VirtualAlloc failed!\r\n")));
	}
	else {
		if(!VirtualCopy((PVOID)s2440INT,(PVOID)(S3C2440A_BASE_REG_PA_INTR >> 8),sizeof(S3C2440A_INTR_REG),PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE )) {
			RETAILMSG(1,(TEXT("For s2440INT: VirtualCopy failed!\r\n")));
		}
	}
	// PWM clock Virtual alloc
	s2440PWR = (volatile S3C2440A_CLKPWR_REG *) VirtualAlloc(0,sizeof(S3C2440A_CLKPWR_REG),MEM_RESERVE, PAGE_NOACCESS);
	if(s2440PWR == NULL) {
		RETAILMSG(1,(TEXT("For s2440PWR: VirtualAlloc failed!\r\n")));
	}
	else {
		if(!VirtualCopy((PVOID)s2440PWR,(PVOID)(S3C2440A_BASE_REG_PA_CLOCK_POWER >>8),sizeof(S3C2440A_CLKPWR_REG),PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE )) {
			RETAILMSG(1,(TEXT("For s2440PWR: VirtualCopy failed!\r\n")));
		}
	}
	
	//InitializeCriticalSection(&m_Lock);


	pCIS->pIORegs = s2440IOP;
	pCIS->pCAMRegs = s2440CAM;
	pCIS->pINTRegs = s2440INT;
	pCIS->pPWRRegs = s2440PWR;

	
}

void Display_Cam_Image(U32 pos_x, U32 pos_y, U32 size_x, U32 size_y, U8 port)
{
	U8 *buffer_rgb;
	U32 y;
	int temp;
	
	//unsigned short *ptr = (unsigned short *)(FRAMEBUF_BASE+0x5dc0);
	
	//static unsigned short transfer_data[QCIF_XSIZE*QCIF_YSIZE];
	static unsigned int time,old_time;
	
	RETAILMSG(MSG_EN_1,(_T("Display_Cam_Image()\r\n")));
	
//	if (port)
	{
		temp = (s2440CAM->CIPRSTATUS>>26)&3;
		temp = (temp + 2) % 4;
		
		switch (temp)
		{
		case 0:
			buffer_rgb = (U8 *)s2440CAM->CIPRCLRSA1;
			break;
		case 1:
			buffer_rgb = (U8 *)s2440CAM->CIPRCLRSA2;
			break;
		case 2:
			buffer_rgb = (U8 *)s2440CAM->CIPRCLRSA3;
			break;
		case 3:
			buffer_rgb = (U8 *)s2440CAM->CIPRCLRSA4;
			break;
		default :
			buffer_rgb = (U8 *)s2440CAM->CIPRCLRSA1;
			break;
		}
	}
	RETAILMSG(MSG_EN_1,(_T("preview buf index = %d\r\n"), temp));

	
	buffer_rgb += VIRTUAL_OFFSET;

//	time = GetTickCount();
//	RETAILMSG(1,(TEXT("+:%d\r\n"),(time - old_time)));


#if 1
SetKMode(TRUE);
	for (y=0; y<size_y; y++) // YCbCr 4:2:0 format
	{
		memcpy((void *)(IMAGE_FRAMEBUFFER_UA_BASE+ (LCD_XSIZE_TFT*pos_y + pos_x) + y*LCD_XSIZE_TFT*2),(void *)buffer_rgb,(PREVIEW_X)*2);//czx
		buffer_rgb += (PREVIEW_X*2);
	}
SetKMode(FALSE);

#endif

}

void Display_Cam_Image2(U32 size_x, U32 size_y, U8 port)
{
	U8 *buffer_rgb;
	U32 x, y;
	int temp;
	
	//unsigned short *ptr = (unsigned short *)(FRAMEBUF_BASE+0x5dc0);
	
	//static unsigned short transfer_data[QCIF_XSIZE*QCIF_YSIZE];
	static unsigned int time,old_time;

	RETAILMSG(MSG_EN_1,(_T("Display_Cam_Image()\r\n")));
	
//	if (port)
	{
		temp = (s2440CAM->CIPRSTATUS>>26)&3;
		temp = (temp + 2) % 4;
		
		switch (temp)
		{
		case 0:
			buffer_rgb = (U8 *)s2440CAM->CIPRCLRSA1;
			break;
		case 1:
			buffer_rgb = (U8 *)s2440CAM->CIPRCLRSA2;
			break;
		case 2:
			buffer_rgb = (U8 *)s2440CAM->CIPRCLRSA3;
			break;
		case 3:
			buffer_rgb = (U8 *)s2440CAM->CIPRCLRSA4;
			break;
		default :
			buffer_rgb = (U8 *)s2440CAM->CIPRCLRSA1;
			break;
		}
	}
	RETAILMSG(MSG_EN_1,(_T("preview buf index = %d\r\n"), temp));

	RETAILMSG(MSG_EN_1,(_T("buffer_rgb = 0x%x\r\n"), buffer_rgb));

	
	buffer_rgb += VIRTUAL_OFFSET;

	RETAILMSG(MSG_EN_1,(_T("buffer_rgb = 0x%x\r\n"), buffer_rgb));

//	time = GetTickCount();
//	RETAILMSG(1,(TEXT("+:%d\r\n"),(time - old_time)));


#if 1
SetKMode(TRUE);
	  for(y=0;y<240;y++)
  	{
		for (x=0; x<320; x++) // YCbCr 4:2:0 format
		{
		*(U16*)((U8*)(IMAGE_FRAMEBUFFER_UA_BASE+y*2+240*(319-x)*2))=*(U16*)((U8*)(buffer_rgb+y*320*2+x*2));
		//memcpy((U8 *)(FRAMEBUF_BASE + (y*2)+240*(319-x)*2),(U8 *)(buffer_rgb+y*320*2+x*2),2);
		}
  	}
SetKMode(FALSE);

#endif

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
		DEBUGMSG(ZONE_INIT,(TEXT("CAMERA: DLL_PROCESS_ATTACH\r\n")));
		return TRUE;
	case DLL_THREAD_ATTACH:
		DEBUGMSG(ZONE_THREAD,(TEXT("CAMERA: DLL_THREAD_ATTACH\r\n")));
		break;
	case DLL_THREAD_DETACH:
		DEBUGMSG(ZONE_THREAD,(TEXT("CAMERA: DLL_THREAD_DETACH\r\n")));
		break;
	case DLL_PROCESS_DETACH:
		DEBUGMSG(ZONE_INIT,(TEXT("CAMERA: DLL_PROCESS_DETACH\r\n")));
		break;
#ifdef UNDER_CE
	case DLL_PROCESS_EXITING:
		DEBUGMSG(ZONE_INIT,(TEXT("CAMERA: DLL_PROCESS_EXITING\r\n")));
		break;
	case DLL_SYSTEM_STARTED:
		DEBUGMSG(ZONE_INIT,(TEXT("CAMERA: DLL_SYSTEM_STARTED\r\n")));
		break;
#endif
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL CIS_Deinit(DWORD hDeviceContext)
{
	BOOL bRet = TRUE;
	
	RETAILMSG(1,(TEXT("CAMERA: CIS_Deinit\r\n")));

	s2440INT->INTMSK |= ( 1 << IRQ_CAM );
	s2440INT->INTSUBMSK |= (( 1 << IRQ_SUB_CAM_P )|( 1 << IRQ_SUB_CAM_C ));
	Camif_Capture(CAPTURE_OFF, CAPTURE_OFF);
	//DisplayEnable = 0;
	DRIVER_PREVIEW_ENABLE = 2;
	
	CloseHandle(CameraThread);
	
	VirtualFree((void*)s2440IOP, 0, MEM_RELEASE);
	VirtualFree((void*)s2440CAM, 0, MEM_RELEASE);
	VirtualFree((void*)s2440INT, 0, MEM_RELEASE);
	VirtualFree((void*)s2440PWR, 0, MEM_RELEASE);

	return TRUE;
} 



BOOL InitInterruptThread()
{
	DWORD         threadID;                         // thread ID
	BOOL bSuccess;

	CameraEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	if (!CameraEvent)
	{
		return FALSE;
	}

	bSuccess = InterruptInitialize(g_CamSysIntr, CameraEvent, NULL, 0);
	if (!bSuccess) 
	{
		RETAILMSG(1,(TEXT("Fail to initialize camera interrupt event\r\n")));
		return FALSE;
	}	
	
	CameraThread = CreateThread(NULL,
	                            0,
	                            (LPTHREAD_START_ROUTINE)CameraCaptureThread,
	                            0,
	                            0,
	                            &threadID);
    
	if (NULL == CameraThread ) {
		RETAILMSG(1,(TEXT("Create Camera Thread Fail\r\n")));
	}
	
	RETAILMSG(1,(_T("CAMERA.DLL::InterruptThread Initialized.\r\n")));
	return TRUE;
}


BOOL CamClockOn(BOOL bOnOff)
{
	// Camera clock
	if (!bOnOff)
	{
		s2440PWR->CLKCON &= ~(1<<19); // Camera clock disable
	}
	else 
	{
		Camera_Clock(CAM_CLK_DIV);
		s2440PWR->CLKCON |= (1<<19); // Camera clock enable
	}
	RETAILMSG(1,(_T("CamClockOn = %d\r\n"), bOnOff));
	
	Delay(1000);

	return TRUE;
}

BOOL CamGpioInit()
{
	s2440IOP->GPJCON = 0x2aaaaaa;
	s2440IOP->GPJDAT = 0;
	s2440IOP->GPJUP = 0;//0x1fff;
	Delay(1000);
	
	return TRUE;
}

void CAM_IF_Reset()
{
	// This functin is used on power handler operation.
	// So, you should not use Kernel API functions as like as "Sleep()".

	//
	// Camera (FIMC2.0) I/F Reset
	//
		
	//s2440CAM->CTRL_C = (1<<19); // 2440A
	s2440CAM->CIGCTRL |= (1<<31);
	// Don't modify this delay time
	//RETAILMSG(1,(TEXT("Camera I/F Reset\r\n")));
	Delay(1000);
	//s2440CAM->CTRL_C = (0<<19);
	s2440CAM->CIGCTRL &= ~(1<<31);
	// Wait for Camera module initialization
	Delay(1000);
}	

void Camera_Module_Reset()
{
	//s2440CAM->CTRL_C = (1<<19); // 2440A
	s2440CAM->CIGCTRL |= (1<<30);
	// Don't modify this delay time
	//RETAILMSG(1,(TEXT("Camera Module Reset\r\n")));
	Delay(100);

	//s2440CAM->CTRL_C = (0<<19);
	s2440CAM->CIGCTRL &= ~(1<<30);
	// Wait for Camera module initialization
	Delay(1000);

	// Samsung Camera need delay time between camera clock enable and camera reset.
	//RETAILMSG(1,(TEXT("You need delay time\r\n")));
	//Delay(1000);
	//Sleep(10);
}

BOOL Cam_Init()
{
	int id;
	sCAMINFO.nDestWidth=352;
	sCAMINFO.nDestHeight=288;
	sCAMINFO.nZoomIndex=1;

	// 1. Camera i/f reset
	CAM_IF_Reset();
	//
	// 2. Camera IO setup
	//
	CamGpioInit();

	//
	// 3. Camera Clock setup
	//
	CamClockOn(TRUE);

	// 4. camera module reset
	Camera_Module_Reset();
	

	// 5. set register of camera module through iic 
	//camera_initialize();
	// use iic for initialization
	id = CAM_WriteBlock();
	if((id == 0x9652) || (id == 0xff52))
		RETAILMSG(1,(TEXT("Camera:: OV9650 initialize is done\r\n")));
	else
		RETAILMSG(1,(TEXT("Camera:: OV9650 initialize is faile\r\n")));

	// to check time
	//s2440IOP->GPGCON &= ~(0x3<<22);
	//s2440IOP->GPGCON |= (0x1<<22);		// EINT19

 	CamInit(sCAMINFO.nDestWidth, sCAMINFO.nDestHeight, PREVIEW_X, PREVIEW_Y, 120, 100, (U32)(g_PhysCodecAddr.LowPart), (U32)(g_PhysPreviewAddr.LowPart));

	//RETAILMSG(1,(TEXT("PhysPreviewAddress = %x, PhysCodecAddress= %x \r\n"),(U32)(g_PhysPreviewAddr.LowPart),(U32)(g_PhysCodecAddr.LowPart)));
	return TRUE;
}


DWORD CIS_Init(DWORD dwContext)
{
	DWORD dwErr = ERROR_SUCCESS, bytes;

	RETAILMSG(MSG_EN_1, (TEXT("CIS::CIS_Init() \r\n")));

	// Allocate for our main data structure and one of it's fields.
	pCIS = (PCIS_CONTEXT)LocalAlloc( LPTR, sizeof(CIS_CONTEXT) );
	if ( !pCIS )
		return( NULL );
	
	pCIS->Sig = CIS_SIG;	

	// 1. Virtual Alloc
	Virtual_Alloc();

	RETAILMSG(MSG_EN_1, (TEXT("CIS::Virtual_Alloc \r\n")));	

	//RETAILMSG(1, (TEXT("CIS_Init : IOCTL_HAL_REQUEST_SYSINTR \r\n")));
	if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &g_CamIrq, sizeof(UINT32), &g_CamSysIntr, sizeof(UINT32), NULL))
	{
		RETAILMSG(1, (TEXT("ERROR: CIS_INIT: Failed to request sysintr value for Camera interrupt.\r\n")));
		return(0);
	}
	
	InitializeCriticalSection(&pCIS->RegCS);

	RETAILMSG(MSG_EN_1, (TEXT("CIS::InitializeCriticalSection \r\n")));

	//
	// Init I2C
	//
	pCIS->hI2C = CreateFile( L"I2C0:",
	                         GENERIC_READ|GENERIC_WRITE,
	                         FILE_SHARE_READ|FILE_SHARE_WRITE,
	                         NULL, OPEN_EXISTING, 0, 0);
   
	if ( INVALID_HANDLE_VALUE == pCIS->hI2C ) {
		dwErr = GetLastError();
		//DEBUGMSG(ZONE_ERR, (TEXT("Error %d opening device '%s' \r\n"), dwErr, L"I2C0:" ));
		RETAILMSG(I2C_MSG, (TEXT("Error %d opening device '%s' \r\n"), dwErr, L"I2C0:" ));
		goto _error_exit;
	}

	RETAILMSG(MSG_EN_1, (TEXT("CIS::CreateFile(\"I2C0\") \r\n")));

	//memset((void*)pCIS->eg, 0, sizeof(pCIS->eg));

	//
	// Gat Fastcall driver-to-driver entrypoints
	//
	if ( !DeviceIoControl(pCIS->hI2C,
	                      IOCTL_I2C_GET_FASTCALL, 
	                      NULL, 0, 
	                      &pCIS->fc, sizeof(pCIS->fc),
	                      &bytes, NULL) ) 
	{
		dwErr = GetLastError();
		DEBUGMSG(ZONE_ERR,(TEXT("IOCTL_I2C_GET_FASTCALL ERROR: %u \r\n"), dwErr));
		goto _error_exit;
	}    

	RETAILMSG(MSG_EN_1, (TEXT("CIS::DeviceIoControl \r\n")));

	// Init H/W
	pCIS->State = INITIALIZE;

	Cam_Init();

	if (!InitInterruptThread())
	{
		RETAILMSG(1,(TEXT("Fail to initialize camera interrupt event\r\n")));
		return FALSE;
	}

	pCIS->Dx = D0;

	m_Dx = (_CEDEVICE_POWER_STATE)D0;
	DevicePowerNotify(_T("CIS1:"),(_CEDEVICE_POWER_STATE)D0, POWER_NAME);

	mInitialized = TRUE;
	return TRUE;

_error_exit:
	return dwErr;	
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL CIS_IOControl(DWORD hOpenContext, 
				   DWORD dwCode, 
				   PBYTE pBufIn, 
				   DWORD dwLenIn, 
				   PBYTE pBufOut, 
				   DWORD dwLenOut, 
				   PDWORD pdwActualOut)
{
	BOOL RetVal = TRUE;
	DWORD dwErr = ERROR_SUCCESS;
	static unsigned int time=0,old_time=0;

	switch (dwCode)
	{

//-----------------------------------------------------------------------------------------
		case IOCTL_POWER_CAPABILITIES: 
		{
			PPOWER_CAPABILITIES ppc;
			RETAILMSG(1, (TEXT("CIS: IOCTL_POWER_CAPABILITIES\r\n")));   
	
			if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(POWER_CAPABILITIES)) ) {
				RetVal = FALSE;
				dwErr = ERROR_INVALID_PARAMETER;
				break;
			}
			
			ppc = (PPOWER_CAPABILITIES)pBufOut;
		
			memset(ppc, 0, sizeof(POWER_CAPABILITIES));

			// support D0, D4 
			ppc->DeviceDx = 0x11;

			// Report our power consumption in uAmps rather than mWatts. 
			ppc->Flags = POWER_CAP_PREFIX_MICRO | POWER_CAP_UNIT_AMPS;
			
			// 25 m = 25000 uA
			// TODO: find out a more accurate value
			ppc->Power[D0] = 25000;
			
			*pdwActualOut = sizeof(POWER_CAPABILITIES);
		} break;

		case IOCTL_POWER_SET: 
		{
			CEDEVICE_POWER_STATE NewDx;

			if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(CEDEVICE_POWER_STATE)) ) {
				RetVal = FALSE;
				dwErr = ERROR_INVALID_PARAMETER;
				break;
			}
			
			NewDx = *(PCEDEVICE_POWER_STATE)pBufOut;

			if ( VALID_DX(NewDx) ) {
				switch ( NewDx ) {
				case D0:
					if (m_Dx != D0) {
						CIS_PowerUp(hOpenContext);
						m_Dx = D0;
					}
					break;

				default:
					if (m_Dx != (_CEDEVICE_POWER_STATE)D4) {
						CIS_PowerDown(hOpenContext);
						m_Dx = (_CEDEVICE_POWER_STATE)D4;
					}
					break;
				}

				// return our state
				*(PCEDEVICE_POWER_STATE)pBufOut = m_Dx;

				RETAILMSG(1, (TEXT("CIS: IOCTL_POWER_SET: D%u \r\n"), NewDx));

				*pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
			} else {
				RetVal = FALSE;
				dwErr = ERROR_INVALID_PARAMETER;
			}
			
		} break;

		case IOCTL_POWER_GET: 

			if ( !pdwActualOut || !pBufOut || (dwLenOut < sizeof(CEDEVICE_POWER_STATE)) ) {
				RetVal = FALSE;
				dwErr = ERROR_INVALID_PARAMETER;
				break;
			}

			*(PCEDEVICE_POWER_STATE)pBufOut = m_Dx;

			RETAILMSG(1, (TEXT("CIS: IOCTL_POWER_GET: D%u \r\n"), m_Dx));

			*pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
			break;

//-----------------------------------------------------------------------------------------

		case IOCTL_CAM_SHOW :
			// Charlie. Show Menu
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_SHOW(%x)\r\n"),dwLenIn));
			break;

		case IOCTL_CAM_HIDE :
			// Charlie. Close display window
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_HIDE\r\n")));
			break;

		case IOCTL_CAM_SETPOS : 
			time = GetTickCount();
			RETAILMSG(MSG_EN_1,(TEXT("Capture time:%d msec\r\n"), (time)));
			old_time = time;
			break;
		
		case CAM_IOCTL_MOVIE_START:			// for MPEG4
		case IOCTL_CAM_CONT : 
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_CONT(MOVIE_START)\r\n")));
			// Charlie. Play
			//DisplayEnable = 1;

			if (bIdlePwrDown == TRUE)
			{
				RETAILMSG(PM_MSG, (_T("[CAM_HW] IOControl CAM_START : Turning Codec On\r\n")));

				CamInterface_PowerUp();
	
				bIdlePwrDown = FALSE;
			}

			// Enable camera interrupt
			s2440INT->INTMSK &= ~( 1 << IRQ_CAM );
			s2440INT->INTSUBMSK &= ~(( 1 << IRQ_SUB_CAM_P )|( 1 << IRQ_SUB_CAM_C ));

			Camif_Capture(CAPTURE_ON, CAPTURE_ON);

			dwDisplayTimeout = DISPLAY_THREAD_TIMEOUT;
			//DisplayTime = DISPLAY_THREAD_TIMEOUT;				// polling mode
			//SetEvent(CameraEvent);

			//s2440IOP->GPGCON &= ~(0x3<<24);
			//s2440IOP->GPGCON |= (0x1<<24);		// EINT20
			frame_count = 0;		// for MPEG4
			break;
		
		case CAM_IOCTL_MOVIE_STOP:		// for MPEG4
		case IOCTL_CAM_STOP : 
			// Charlie. Stop
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_STOP\r\n")));

			// Disable camera interrupt
			s2440INT->INTMSK |= ( 1 << IRQ_CAM );
			s2440INT->INTSUBMSK |= (( 1 << IRQ_SUB_CAM_P )|( 1 << IRQ_SUB_CAM_C ));
			if (s2440INT->INTPND & ( 1 << IRQ_CAM )) s2440INT->INTPND |= ( 1 << IRQ_CAM );
			s2440INT->SRCPND |= ( 1 << IRQ_CAM );
			s2440INT->SUBSRCPND |= (( 1 << IRQ_SUB_CAM_P )|( 1 << IRQ_SUB_CAM_C ));

			/*
			s2440CAM->CICOCTRL |= (1<<2); // CODEC Last IRQ Enable
			s2440CAM->CIPRCTRL |= (1<<2); // PREVIEW Last IRQ Enable
			*/
			Camif_Capture(CAPTURE_OFF, CAPTURE_OFF);
			
			//DisplayEnable = 0;
			// 2004.05.18 jylee
			DRIVER_PREVIEW_ENABLE = 2; 
			//dwDisplayTimeout = INFINITE;
			break;

		// for MPEG4
		case CAM_IOCTL_GET_LATEST_FRAME:
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:CAM_IOCTL_GET_LATEST_FRAME\r\n")));
			//RETAILMSG(MSG_EN_1,(TEXT("pBufIn = 0x%x\r\n"), pBufIn));
			//RETAILMSG(MSG_EN_1,(TEXT("no function available, TBD\r\n")));
			if (frame_count < 2) {
				RETAILMSG(1,(TEXT("CAMERA:CAM_IOCTL_GET_LATEST_FRAME - frame not available!!!\r\n")));
				return FALSE;
			}
			Tick_GET_FRAME_PREV = Tick_GET_FRAME_CUR;
			Tick_GET_FRAME_CUR = GetTickCount();
			Copy_Cam_Image(pBufOut, QCIF_XSIZE, QCIF_YSIZE, PORT_A);
			break;

		case CAM_IOCTL_SAMSUNG_CAM:		// ID=0x520
			RETAILMSG(MSG_EN_1,(_T("CAM_IOCTL_SAMSUNG_CAM\r\n")));
			Samsung_camcoder(pBufOut);
			break;

		case CAM_IOCTL_SAMSUNG_CAM_PR:	// ID=0x522
			RETAILMSG(MSG_EN_1,(_T("CAM_IOCTL_SAMSUNG_CAM_PR\r\n")));
			Samsung_camcoder_pr(pBufOut);
			break;

		case CAM_IOCTL_SAMSUNG_PREVIEW_START : // ID=0x523
			DRIVER_PREVIEW_ENABLE = 1;
			RETAILMSG(MSG_EN_1,(_T("CAM_IOCTL_SAMSUNG_PREVIEW_START(%x)\r\n"),*pBufOut, DRIVER_PREVIEW_ENABLE));
			break;

		case CAM_IOCTL_SAMSUNG_PREVIEW_STOP : // ID=0x524
			DRIVER_PREVIEW_ENABLE = 2;
			RETAILMSG(MSG_EN_1,(_T("CAM_IOCTL_SAMSUNG_PREVIEW_STOP(%x)\r\n"),*pBufOut, DRIVER_PREVIEW_ENABLE));
			break;
			
		case IOCTL_CAM_SETGAMMA :
			if( *pBufOut == 1 ){
				DRIVER_PREVIEW_ENABLE = 1;
			}
			else if( *pBufOut == 2 ){
				DRIVER_PREVIEW_ENABLE = 2;
			}
			RETAILMSG(MSG_EN_1,(_T("IOCTL_CAM_SETGAMMA:*pBufOut(%x):DRIVER_PREVIEW_ENABLE(%x)\r\n"),*pBufOut, DRIVER_PREVIEW_ENABLE));
			break;
/*					
		case CAM_IOCTL_IMAGE_SIZE:		// ID=0x521
		
			RETAILMSG(MSG_EN_1,(TEXT("CAM_IOCTL_IMAGE_SIZE\r\n")));

			if (bIdlePwrDown == TRUE)
			{
				RETAILMSG(PM_MSG, (_T("[CAM_HW] IOControl CAM_START : Turning Codec On\r\n")));

				CamInterface_PowerUp();
	
				bIdlePwrDown = FALSE;
			}

			if( *pBufOut == 1 ){
				image_size = 1;
			}
			else if( *pBufOut == 2 ){
				image_size = 2;
			}
			RETAILMSG(MSG_EN_1,(TEXT("image_size:%d,0x%x,0x%x\r\n"),image_size,pBufOut,*pBufOut));			

			s2440INT->INTMSK |= ( 1 << IRQ_CAM );
			s2440INT->INTSUBMSK |= (( 1 << IRQ_SUB_CAM_P )|( 1 << IRQ_SUB_CAM_C ));

			Camif_Capture(CAPTURE_OFF, CAPTURE_OFF);
			
			if (image_size == 1)
				CamInit(QCIF_XSIZE, QCIF_YSIZE, QCIF_XSIZE, QCIF_YSIZE, 120, 100, COPIFRAMEBUFFER_B, COPIFRAMEBUFFER_A);
			else if (image_size == 2)
				CamInit(CIF_XSIZE, CIF_YSIZE, QCIF_XSIZE, QCIF_YSIZE, 120, 100, COPIFRAMEBUFFER_B, COPIFRAMEBUFFER_A);
			
			break;
*/
		case CAM_IOCTL_IMAGE_CHANGE:		// ID=0x521
		
			RETAILMSG(MSG_EN_1,(TEXT("CAM_IOCTL_IMAGE_CHANGE\r\n")));

			if (bIdlePwrDown == TRUE)
			{
				RETAILMSG(PM_MSG, (_T("[CAM_HW] IOControl CAM_START : Turning Codec On\r\n")));

				CamInterface_PowerUp();
	
				bIdlePwrDown = FALSE;
			}

			memcpy(&sCAMINFO, pBufOut, sizeof(sCAMINFO));
						
			RETAILMSG(MSG_EN_1,(TEXT("image_Xsize:%d, image_YSize:%d, ZoomIndex:%d\r\n"), sCAMINFO.nDestWidth, sCAMINFO.nDestHeight, sCAMINFO.nZoomIndex));			

			s2440INT->INTMSK |= ( 1 << IRQ_CAM );
			s2440INT->INTSUBMSK |= (( 1 << IRQ_SUB_CAM_P )|( 1 << IRQ_SUB_CAM_C ));

			Camif_Capture(CAPTURE_OFF, CAPTURE_OFF);

			if((sCAMINFO.nZoomIndex == 2) ||(sCAMINFO.nZoomIndex == 3))
				CamInit(sCAMINFO.nDestWidth, sCAMINFO.nDestHeight, PREVIEW_X, PREVIEW_Y, sCAMINFO.nZoomIndex*80, sCAMINFO.nZoomIndex*60, (U32)(g_PhysCodecAddr.LowPart), (U32)(g_PhysPreviewAddr.LowPart));
			else if(sCAMINFO.nZoomIndex == 1)
				CamInit(sCAMINFO.nDestWidth, sCAMINFO.nDestHeight, PREVIEW_X, PREVIEW_Y, 120, 100, (U32)(g_PhysCodecAddr.LowPart), (U32)(g_PhysPreviewAddr.LowPart));				
			else
				CamInit(CIF_XSIZE, CIF_YSIZE, PREVIEW_X, PREVIEW_Y, 120, 100, (U32)(g_PhysCodecAddr.LowPart), (U32)(g_PhysPreviewAddr.LowPart));				
			break;
				
		case IOCTL_CAM_COPY :
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_COPY(dwLenIn:%x)\r\n"), dwLenIn));
			break;

		case IOCTL_CAM_SETCOLKEY :
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_SETCOLKEY\r\n")));
			break;
			
		case IOCTL_CAM_SETALPHA :
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_SETALPHA\r\n")));
			break;

		case IOCTL_CAM_GETINFO :
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_GETINFO\r\n")));
			break;

		case IOCTL_CAM_SETSCALE :
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_SETSCALE\r\n")));
			break;

		case IOCTL_CAM_SETHUE :
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_SETHUE\r\n")));
			break;
#if 0
		case IOCTL_CAM_SETGAMMA :
			// Get which ping -pong buffer have data
			//DisplayEnable = 0;
			Sleep(200);
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_BUFFER:%d\r\n"),buffer_num));
			break;
#endif
		case IOCTL_CAM_SETWBCOORD : 
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_SETWBCOORD\r\n")));
			break;

		case IOCTL_CAM_SETAELIMIT :
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_SETAELIMIT\r\n")));
			break;

		case IOCTL_CAM_SETADCOFS :	
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_SETADCOFS\r\n")));
			break;

		case IOCTL_CAM_SETWBGAIN :	
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_SETWBGAIN\r\n")));
			break;

		case IOCTL_CAM_SETCBCRGAIN :
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_SETCBCRGAIN\r\n")));
			break;

		case IOCTL_CAM_SETLEGAIN :	
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_SETLEGAIN\r\n")));
			break;

		case IOCTL_CAM_SETBRIGHTNESS :
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_SETBRIGHTNESS\r\n")));
			break;

		case IOCTL_CAM_SETCLIP :
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_SETCLIP\r\n")));
			break;

		case IOCTL_CAM_SETSLICELMT :
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_SETSLICELMT\r\n")));
			break;

		case IOCTL_CAM_WBACCLMT :
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_WBACCLMT\r\n")));
			break;

		case IOCTL_CAM_CSEDGELMT :
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_CSEDGELMT\r\n")));
			break;

		case IOCTL_CAM_ENVINT :
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_ENVINT\r\n")));
			break;

		case IOCTL_CAM_DISVINT :
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_DISVINT\r\n")));
			break;

		case IOCTL_CAM_SETCAMERA :
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:IOCTL_CAM_SETCAMERA\r\n")));
			break;

		default : 
			RETAILMSG(MSG_EN_1,(TEXT("CAMERA:Ioctl code = 0x%x\r\n"), dwCode));
			return FALSE;

	}
	return TRUE;
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
DWORD CIS_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
	RETAILMSG(1,(TEXT("CAMERA: CIS_Open\r\n")));

	DRIVER_PREVIEW_ENABLE = 2;

	return TRUE;
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL CIS_Close(DWORD hOpenContext)
{
	RETAILMSG(1,(TEXT("CAMERA: CIS_Close\r\n")));
	return TRUE;
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CIS_PowerDown(DWORD hDeviceContext)
{
	RETAILMSG(1,(TEXT("CAMERA: CIS_PowerDown\r\n")));

	m_Dx = (_CEDEVICE_POWER_STATE)D4;

	Camif_Capture(CAPTURE_OFF, CAPTURE_OFF);

#if 1
	bIdlePwrDown = TRUE;
	CamInterface_PowerDown();
#else
	CamClockOn(FALSE);
#endif

	//RETAILMSG(1,(TEXT("CAMERA: CIS_PowerDown, m_Dx = D%u, init %d \r\n"), m_Dx, mInitialized));
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CIS_PowerUp(DWORD hDeviceContext)
{
	RETAILMSG(1,(TEXT("CAMERA: CIS_PowerUp\r\n")));
	
	m_Dx = (_CEDEVICE_POWER_STATE)D0;
	
	if (bIdlePwrDown == FALSE)
	{
		//
		// 1. Camera IO setup
		//
		CamGpioInit();
	
		//
		// 2. Camera Clock setup
		//
		CamClockOn(TRUE);
	
		// 3. Camera i/f reset
		CAM_IF_Reset();

		/*
		// 4. camera module reset
		Camera_Module_Reset();
	

		// 5. set register of camera module through iic 
		//camera_initialize();
		// use iic for initialization
		CAM_WriteBlock();
		*/

		if (image_size == 1)
			CamInit(QCIF_XSIZE, QCIF_YSIZE, PREVIEW_X, PREVIEW_Y, 120, 100, (U32)(g_PhysCodecAddr.LowPart), (U32)(g_PhysPreviewAddr.LowPart));
		else if (image_size == 2)
			CamInit(CIF_XSIZE, CIF_YSIZE, PREVIEW_X, PREVIEW_Y, 120, 100, (U32)(g_PhysCodecAddr.LowPart), (U32)(g_PhysPreviewAddr.LowPart));
		else
 			CamInit(sCAMINFO.nDestWidth, sCAMINFO.nDestHeight, PREVIEW_X, PREVIEW_Y, 120, 100, (U32)(g_PhysCodecAddr.LowPart), (U32)(g_PhysPreviewAddr.LowPart));


	} else 
	{
		//CamInterface_PowerUp();
	}		
	RETAILMSG(1,(TEXT("CAMERA: CIS_PowerUp, m_Dx = D%u\r\n"), m_Dx));
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
DWORD CIS_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
	RETAILMSG(1,(TEXT("CAMERA: CIS_Read\r\n")));
	return TRUE;
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
DWORD CIS_Seek(DWORD hOpenContext, long Amount, DWORD Type)
{
	RETAILMSG(1,(TEXT("CAMERA: CIS_Seek\r\n")));
	return 0;
} 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
DWORD CIS_Write(DWORD hOpenContext, LPCVOID pSourceBytes, DWORD NumberOfBytes)
{
	RETAILMSG(1,(TEXT("CAMERA: CIS_Write\r\n")));
	return 0;
}

// for MPEG4
void Copy_Cam_Image(U8 *pBufOut, U32 size_x, U32 size_y, U8 port)
{
	U8 *buffer_y, *buffer_cb, *buffer_cr;
	U32 Y_size, C_size, P_size;
	int temp;
	U8 *pImage;
	static unsigned int frame_timestamp_prev = GetTickCount();
	static int frame_bank_prev = -1;
	YUVINFO yuvinfo;

	RETAILMSG(MSG_EN_1,(_T("Copy_Cam_Image\r\n")));

	//s2440IOP->GPFDAT &= ~(1<<6);	

	pImage = pBufOut;

	if (image_size == 1) // QCIF	
		Y_size = QCIF_XSIZE*QCIF_YSIZE;
	else if (image_size == 2) // CIF
		Y_size = CIF_XSIZE*CIF_YSIZE;
			
	C_size = Y_size/4;	
	P_size = Y_size + C_size*2;

	{
		temp = (s2440CAM->CICOSTATUS>>26)&3;
		temp = (temp + 2) % 4;

		RETAILMSG(MSG_EN_1,(_T("Copy_Cam_Image %d\r\n"), temp));

		switch (temp)
		{
		case 0:
			buffer_y = (U8 *)s2440CAM->CICOYSA1;
			buffer_cb = (U8 *)s2440CAM->CICOCBSA1;
			buffer_cr = (U8 *)s2440CAM->CICOCRSA1;
			break;
		case 1:
			buffer_y = (U8 *)s2440CAM->CICOYSA2;
			buffer_cb = (U8 *)s2440CAM->CICOCBSA2;
			buffer_cr = (U8 *)s2440CAM->CICOCRSA2;
			break;
		case 2:
			buffer_y = (U8 *)s2440CAM->CICOYSA3;
			buffer_cb = (U8 *)s2440CAM->CICOCBSA3;
			buffer_cr = (U8 *)s2440CAM->CICOCRSA3;
			break;
		case 3:
			buffer_y = (U8 *)s2440CAM->CICOYSA4;
			buffer_cb = (U8 *)s2440CAM->CICOCBSA4;
			buffer_cr = (U8 *)s2440CAM->CICOCRSA4;
			break;
		default :
			buffer_y = (U8 *)s2440CAM->CICOYSA1;
			buffer_cb = (U8 *)s2440CAM->CICOCBSA1;
			buffer_cr = (U8 *)s2440CAM->CICOCRSA1;
			break;
		}
	} 		

	buffer_y += VIRTUAL_ADDR_OFFSET;
	buffer_cb += VIRTUAL_ADDR_OFFSET;
	buffer_cr += VIRTUAL_ADDR_OFFSET;

	RETAILMSG(MSG_EN_1,(_T("buffer_y = 0x%x\r\n"), buffer_y));
	RETAILMSG(MSG_EN_1,(_T("buffer_cb = 0x%x\r\n"), buffer_cb));
	RETAILMSG(MSG_EN_1,(_T("buffer_cr = 0x%x\r\n"), buffer_cr));

	if (image_size == 1) // QCIF	
	{
		yuvinfo.frame_width = QCIF_XSIZE;
		yuvinfo.frame_height = QCIF_YSIZE;
	}
	else if (image_size == 2) // CIF	
	{
		yuvinfo.frame_width = CIF_XSIZE;
		yuvinfo.frame_height = CIF_YSIZE;
	}
	
	if (temp == frame_bank_prev) yuvinfo.frame_stamp = frame_timestamp_prev;
	else 
	{
		yuvinfo.frame_stamp = GetTickCount();
		frame_timestamp_prev = yuvinfo.frame_stamp;
		frame_bank_prev = temp;
	}

	RETAILMSG(MSG_EN_1,(_T("pBufOut 0 offset = 0x%x\r\n"), pImage));
	memcpy(pImage, &yuvinfo, sizeof(YUVINFO));
	pImage += sizeof(YUVINFO);	
	
	RETAILMSG(MSG_EN_1,(_T("pBufOut Y = 0x%x\r\n"), pImage));
	memcpy(pImage, buffer_y, Y_size);
	pImage += (Y_size);
	
	RETAILMSG(MSG_EN_1,(_T("pBufOut cb = 0x%x\r\n"), pImage));
	memcpy(pImage, buffer_cb, C_size);
	pImage += C_size;
	RETAILMSG(MSG_EN_1,(_T("pBufOut cr = 0x%x\r\n"), pImage));
	memcpy(pImage, buffer_cr, C_size);
//	pBufOut += (size_x*size_y)/4;




//	RETAILMSG(MSG_EN_1,(TEXT("1:buffer_y:0x%x,buffer_cb:0x%x,buffer_cr:0x%x\r\n"),buffer_y,buffer_cb,buffer_cr));

	//s2440IOP->GPFDAT |= (1<<6);	
	
	Tick_COPY_FRAME = GetTickCount();

	RETAILMSG(MSG_EN_1,(_T("COPY_Tick = %d\r\n"), (Tick_COPY_FRAME-Tick_GET_FRAME_CUR)));
	RETAILMSG(MSG_EN_1,(_T("Frame_Tick = %d\r\n"), (Tick_GET_FRAME_CUR-Tick_GET_FRAME_PREV)));
	
//	RETAILMSG(1,(_T("Tick_COPY_FRAME = %d\r\n"), Tick_COPY_FRAME));
//	RETAILMSG(1,(_T("Tick_GET_FRAME_PREV = %d\r\n"), Tick_GET_FRAME_PREV));
//	RETAILMSG(1,(_T("Tick_GET_FRAME_CUR = %d\r\n"), Tick_GET_FRAME_CUR));	


}

void Samsung_camcoder(U8 *pBufOut)
{
	U8 *pImage;
	PINGPONG caminfo;

	pImage = pBufOut;

	if( codec_flag )
	{
		caminfo.flag = 1;
		caminfo.y_address = y_address;
		caminfo.cb_address = cb_address;
		caminfo.cr_address = cr_address;
		
		memcpy(pImage, &caminfo, sizeof(PINGPONG));
		codec_flag = 0;
	}
}

void Samsung_camcoder_pr(U8 *pBufOut)
{
	U8 *pImage;
	PINGPONG_PR prinfo;

	pImage = pBufOut;

	if ( rgb_flag )
	{
		prinfo.flag = 1;
		prinfo.rgb_address = rgb_address;
		memcpy(pImage, &prinfo, sizeof(PINGPONG_PR));
		rgb_flag = 0;
	}	
}

void Buffer_codec_info_update()
{
	U32 Y_size, C_size, P_size;
	int temp;
	unsigned int buffer_y, buffer_cb, buffer_cr;
	
	if ( image_size == 1 )
		Y_size = QCIF_XSIZE*QCIF_YSIZE;
	else if ( image_size == 2 )
		Y_size = CIF_XSIZE*CIF_YSIZE;

	C_size = Y_size/4;	
	P_size = Y_size + C_size*2;

	temp = (s2440CAM->CICOSTATUS>>26)&3;
	temp = (temp + 2) % 4;

	RETAILMSG(MSG_EN_2,(_T("codec index = %d, size %d\r\n"), temp, image_size));

	switch (temp)
	{
	case 0:
		buffer_y = s2440CAM->CICOYSA1;
		buffer_cb = s2440CAM->CICOCBSA1;
		buffer_cr = s2440CAM->CICOCRSA1;
		break;
	case 1:
		buffer_y = s2440CAM->CICOYSA2;
		buffer_cb = s2440CAM->CICOCBSA2;
		buffer_cr = s2440CAM->CICOCRSA2;
		break;
	case 2:
		buffer_y = s2440CAM->CICOYSA3;
		buffer_cb = s2440CAM->CICOCBSA3;
		buffer_cr = s2440CAM->CICOCRSA3;
		break;
	case 3:
		buffer_y = s2440CAM->CICOYSA4;
		buffer_cb = s2440CAM->CICOCBSA4;
		buffer_cr = s2440CAM->CICOCRSA4;
		break;
	default :
		buffer_y = s2440CAM->CICOYSA1;
		buffer_cb = s2440CAM->CICOCBSA1;
		buffer_cr = s2440CAM->CICOCRSA1;
		break;
	}

	buffer_y += VIRTUAL_ADDR_OFFSET;
	buffer_cb += VIRTUAL_ADDR_OFFSET;
	buffer_cr += VIRTUAL_ADDR_OFFSET;
#if 0
	RETAILMSG(MSG_EN_1,(_T("buffer_y = 0x%x\r\n"), buffer_y));
	RETAILMSG(MSG_EN_1,(_T("buffer_cb = 0x%x\r\n"), buffer_cb));
	RETAILMSG(MSG_EN_1,(_T("buffer_cr = 0x%x\r\n"), buffer_cr));
#endif
	if( codec_flag )	RETAILMSG(MSG_EN_1,(_T("Buffer is not read\r\n")));
	codec_flag = 1;
	y_address = buffer_y;
	cb_address = buffer_cb;
	cr_address = buffer_cr;
}


void Buffer_preview_info_update()
{
//	U32 Y_size;
	int temp;
	unsigned int buffer_rgb;
	
/*
	if ( image_size == 1 )
		Y_size = QCIF_XSIZE*QCIF_YSIZE;
	else if ( image_size == 2 )
		Y_size = CIF_XSIZE*CIF_YSIZE;
*/

	temp = (s2440CAM->CIPRSTATUS>>26)&3;
	temp = (temp + 2) % 4;

	RETAILMSG(MSG_EN_2,(_T("preview index = %d, size %d\r\n"), temp, image_size));

	switch (temp)
	{
	case 0:
		buffer_rgb = s2440CAM->CIPRCLRSA1;
		break;
	case 1:
		buffer_rgb = s2440CAM->CIPRCLRSA2;
		break;
	case 2:
		buffer_rgb = s2440CAM->CIPRCLRSA3;
		break;
	case 3:
		buffer_rgb = s2440CAM->CIPRCLRSA4;
		break;
	default :
		buffer_rgb = s2440CAM->CIPRCLRSA1;
		break;
	}

	RETAILMSG(MSG_EN_1,(_T("buffer_rgb[PHY] = 0x%x\r\n"), buffer_rgb));
	buffer_rgb += VIRTUAL_ADDR_OFFSET;
#if 1
	RETAILMSG(MSG_EN_1,(_T("buffer_rgb = 0x%x\r\n"), buffer_rgb));
#endif
	if( rgb_flag )	RETAILMSG(MSG_EN_1,(_T("Prev Buffer is not read\r\n")));
	rgb_flag = 1;
	rgb_address = buffer_rgb;
}


void CamInit(U32 CoDstWidth, U32 CoDstHeight, U32 PrDstWidth, U32 PrDstHeight, 
			U32 WinHorOffset, U32 WinVerOffset,  U32 CoFrameBuffer, U32 PrFrameBuffer)
{
	U32 WinOfsEn;
	U32 divisor, multiplier;
	U32 MainBurstSizeY, RemainedBurstSizeY, MainBurstSizeC, RemainedBurstSizeC, MainBurstSizeRGB, RemainedBurstSizeRGB;
	U32 H_Shift, V_Shift, PreHorRatio, PreVerRatio, MainHorRatio, MainVerRatio;
	U32 SrcWidth, SrcHeight;
	U32 ScaleUp_H_Co, ScaleUp_V_Co, ScaleUp_H_Pr, ScaleUp_V_Pr;

	//constant for calculating codec dma address
	if(CAM_CODEC_OUTPUT)
		divisor=2; //CCIR-422
	else
		divisor=4; //CCIR-420
		
	//constant for calculating preview dma address
	if(CAM_PVIEW_OUTPUT)
		multiplier=4;
	else
		multiplier=2;
	
	if(WinHorOffset==0 && WinVerOffset==0)
		WinOfsEn=0;
	else
		WinOfsEn=1;

	SrcWidth=CAM_SRC_HSIZE-WinHorOffset*2;
	SrcHeight=CAM_SRC_VSIZE-WinVerOffset*2;

	if(SrcWidth>=CoDstWidth) ScaleUp_H_Co=0; //down
	else ScaleUp_H_Co=1;		//up

	if(SrcHeight>=CoDstHeight) ScaleUp_V_Co=0;
	else ScaleUp_V_Co=1;		

	if(SrcWidth>=PrDstWidth) ScaleUp_H_Pr=0; //down
	else ScaleUp_H_Pr=1;		//up

	if(SrcHeight>=PrDstHeight) ScaleUp_V_Pr=0;
	else ScaleUp_V_Pr=1;		

	////////////////// common control setting
	s2440CAM->CIGCTRL |= (1<<26)|(0<<27); // inverse PCLK, test pattern
	s2440CAM->CIWDOFST = (1<<30)|(0xf<<12); // clear overflow 
	s2440CAM->CIWDOFST = 0;	
	s2440CAM->CIWDOFST=(WinOfsEn<<31)|(WinHorOffset<<16)|(WinVerOffset);

//#if(CIS_TYPE == CIS_S5X3A1)
	s2440CAM->CISRCFMT=(CAM_ITU601<<31)|(0<<30)|(0<<29)|(CAM_SRC_HSIZE<<16)|(CAM_ORDER_YCBYCR<<14)|(CAM_SRC_VSIZE);
//#elif(CIS_TYPE == CIS_S5X3AA)
//	s2440CAM->CISRCFMT=(CAM_ITU601<<31)|(0<<30)|(0<<29)|(CAM_SRC_HSIZE<<16)|(CAM_ORDER_CBYCRY<<14)|(CAM_SRC_VSIZE); 
//#endif


	////////////////// codec port setting
	s2440CAM->CICOYSA1=CoFrameBuffer;
	s2440CAM->CICOYSA2=s2440CAM->CICOYSA1+CoDstWidth*CoDstHeight+2*CoDstWidth*CoDstHeight/divisor;
	if(CoDstWidth*CoDstHeight < 655360)   // Codec Max. Size = 1280*1024   Above sXGA, use only 2 memory of the 4 pingpong memory
	{
		s2440CAM->CICOYSA3=s2440CAM->CICOYSA2+CoDstWidth*CoDstHeight+2*CoDstWidth*CoDstHeight/divisor;
		s2440CAM->CICOYSA4=s2440CAM->CICOYSA3+CoDstWidth*CoDstHeight+2*CoDstWidth*CoDstHeight/divisor;
	}
	else
	{
		s2440CAM->CICOYSA3=s2440CAM->CICOYSA1;
		s2440CAM->CICOYSA4=s2440CAM->CICOYSA2;		
	}
	
	s2440CAM->CICOCBSA1=s2440CAM->CICOYSA1+CoDstWidth*CoDstHeight;
	s2440CAM->CICOCBSA2=s2440CAM->CICOYSA2+CoDstWidth*CoDstHeight;
	s2440CAM->CICOCBSA3=s2440CAM->CICOYSA3+CoDstWidth*CoDstHeight;
	s2440CAM->CICOCBSA4=s2440CAM->CICOYSA4+CoDstWidth*CoDstHeight;

	s2440CAM->CICOCRSA1=s2440CAM->CICOCBSA1+CoDstWidth*CoDstHeight/divisor;
	s2440CAM->CICOCRSA2=s2440CAM->CICOCBSA2+CoDstWidth*CoDstHeight/divisor;
	s2440CAM->CICOCRSA3=s2440CAM->CICOCBSA3+CoDstWidth*CoDstHeight/divisor;
	s2440CAM->CICOCRSA4=s2440CAM->CICOCBSA4+CoDstWidth*CoDstHeight/divisor;

	s2440CAM->CICOTRGFMT=(CAM_CODEC_IN_422<<31)|(CAM_CODEC_OUTPUT<<30)|(CoDstWidth<<16)|(CAM_FLIP_NORMAL<<14)|(CoDstHeight);

	CalculateBurstSize(CoDstWidth, &MainBurstSizeY, &RemainedBurstSizeY);
	CalculateBurstSize(CoDstWidth/2, &MainBurstSizeC, &RemainedBurstSizeC);
	s2440CAM->CICOCTRL=(MainBurstSizeY<<19)|(RemainedBurstSizeY<<14)|(MainBurstSizeC<<9)|(RemainedBurstSizeC<<4);

	CalculatePrescalerRatioShift(SrcWidth, CoDstWidth, &PreHorRatio, &H_Shift);
	CalculatePrescalerRatioShift(SrcHeight, CoDstHeight, &PreVerRatio, &V_Shift);
	MainHorRatio=(SrcWidth<<8)/(CoDstWidth<<H_Shift);
	MainVerRatio=(SrcHeight<<8)/(CoDstHeight<<V_Shift);

	s2440CAM->CICOSCPRERATIO=((10-H_Shift-V_Shift)<<28)|(PreHorRatio<<16)|(PreVerRatio);
	s2440CAM->CICOSCPREDST=((SrcWidth/PreHorRatio)<<16)|(SrcHeight/PreVerRatio); 
	s2440CAM->CICOSCCTRL=(CAM_SCALER_BYPASS_OFF<<31)|(ScaleUp_H_Co<<30)|(ScaleUp_V_Co<<29)|(MainHorRatio<<16)|(MainVerRatio);

	s2440CAM->CICOTAREA=CoDstWidth*CoDstHeight;

	///////////////// preview port setting
	s2440CAM->CIPRCLRSA1=PrFrameBuffer;
	s2440CAM->CIPRCLRSA2=s2440CAM->CIPRCLRSA1+PrDstWidth*PrDstHeight*multiplier;
	s2440CAM->CIPRCLRSA3=s2440CAM->CIPRCLRSA2+PrDstWidth*PrDstHeight*multiplier;
	s2440CAM->CIPRCLRSA4=s2440CAM->CIPRCLRSA3+PrDstWidth*PrDstHeight*multiplier;
/*
	if(CIS_TYPE == CIS_S5X3A1)
		s2440CAM->CIPRTRGFMT=(PrDstWidth<<16)|(CAM_FLIP_YAXIS<<14)|(PrDstHeight);
	else
		s2440CAM->CIPRTRGFMT=(PrDstWidth<<16)|(CAM_FLIP_NORMAL<<14)|(PrDstHeight);*/
	s2440CAM->CIPRTRGFMT=(PrDstWidth<<16)|(CAM_FLIP_XAXIS<<14)|(PrDstHeight);
	
	if (CAM_PVIEW_OUTPUT==CAM_RGB24B)
		CalculateBurstSize(PrDstWidth*4, &MainBurstSizeRGB, &RemainedBurstSizeRGB);
	else // RGB16B
		CalculateBurstSize(PrDstWidth*2, &MainBurstSizeRGB, &RemainedBurstSizeRGB);
   	s2440CAM->CIPRCTRL=(MainBurstSizeRGB<<19)|(RemainedBurstSizeRGB<<14);

	CalculatePrescalerRatioShift(SrcWidth, PrDstWidth, &PreHorRatio, &H_Shift);
	CalculatePrescalerRatioShift(SrcHeight, PrDstHeight, &PreVerRatio, &V_Shift);
	MainHorRatio=(SrcWidth<<8)/(PrDstWidth<<H_Shift);
	MainVerRatio=(SrcHeight<<8)/(PrDstHeight<<V_Shift);
	s2440CAM->CIPRSCPRERATIO=((10-H_Shift-V_Shift)<<28)|(PreHorRatio<<16)|(PreVerRatio);		 
	s2440CAM->CIPRSCPREDST=((SrcWidth/PreHorRatio)<<16)|(SrcHeight/PreVerRatio);
	s2440CAM->CIPRSCCTRL=(1<<31)|(CAM_RGB16B<<30)|(ScaleUp_H_Pr<<29)|(ScaleUp_V_Pr<<28)|(MainHorRatio<<16)|(MainVerRatio);

	s2440CAM->CIPRTAREA= PrDstWidth*PrDstHeight;

	// LJY, added 040615	
	// initialization for buffer addresses
	y_address = s2440CAM->CICOYSA1;
	cb_address = s2440CAM->CICOCBSA1;
	cr_address = s2440CAM->CICOCRSA1;
	rgb_address = s2440CAM->CIPRCLRSA1;
}


