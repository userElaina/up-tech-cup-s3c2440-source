#ifndef __CAMERA_H_
#define __CAMERA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c.h"

typedef enum _CIS_STATE {

    UNINITIALIZED,
    INITIALIZE,
    RUN,
    SUSPEND,
    RESUME,
    //ADC_ERROR,    

} CIS_STATE, *PCIS_STATE;


#define CIS_SIG ' SIC'

#define VALID_CONTEXT( p ) \
   ( p && p->Sig && CIS_SIG == p->Sig )

//
// CIS_CONTEXT
//
#pragma pack(1)

typedef struct _CIS_CONTEXT {

    DWORD   Sig;    // Signature
    
    //
    // PWR50606 register cache
    //
    //volatile UCHAR Reg[PCF50606_REG_CACHE];
    volatile UCHAR Reg[128];
    
    // Saved our OOCS & INTx registers during init/reset/resume 
    // because they are cleared on read, so we loose them.
    UCHAR ResetReg[4];
    BOOL  ResetRegReady;

	volatile S3C2440A_IOPORT_REG *pIORegs;
	volatile S3C2440A_CAM_REG *pCAMRegs;
	volatile S3C2440A_INTR_REG *pINTRegs;
	volatile S3C2440A_CLKPWR_REG *pPWRRegs;

    CIS_STATE   State;

    CRITICAL_SECTION RegCS;

    HANDLE          hI2C;   // I2C Bus Driver
    I2C_FASTCALL    fc;     // I2C Fastcall driver-to-driver entrypoints

    //HANDLE          hUtil;   // UTIL Driver to map in VA
    //UTL_FASTCALL    utlFc;
        
    HANDLE IST;
    HANDLE ISTEvent;

    //HANDLE hBATT_FLT_IST;
    //HANDLE hBATT_FLT_Event;

    DWORD OpenCount;

    //
    // System Global Events
    //
    //HANDLE hADC;        // A-to-D Conversion Done
    //HANDLE hTSCPRES;    // Touch Pressed

    CEDEVICE_POWER_STATE    Dx;

} CIS_CONTEXT, *PCIS_CONTEXT;

#pragma pack()



BOOL CIS_Close(DWORD hOpenContext);
BOOL CIS_Deinit(DWORD hDeviceContext);
DWORD CIS_Init(DWORD dwContext);
DWORD CIS_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode);
void CIS_PowerUp(DWORD hDeviceContext);
void CIS_PowerDown(DWORD hDeviceContext);
DWORD CIS_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count);
DWORD CIS_Seek(DWORD hOpenContext, long Amount, DWORD Type);
DWORD CIS_Write(DWORD hOpenContext, LPCVOID pSourceBytes, DWORD NumberOfBytes);

BOOL CIS_IOControl(DWORD hOpenContext, 
				   DWORD dwCode, 
				   PBYTE pBufIn, 
				   DWORD dwLenIn, 
				   PBYTE pBufOut, 
				   DWORD dwLenOut, 
				   PDWORD pdwActualOut);


//void CalculateBurstSize(unsigned int hSize,unsigned int *mainBurstSize,unsigned int *remainedBurstSize);
//void CalculatePrescalerRatioShift(unsigned int SrcSize, unsigned int DstSize, unsigned int *ratio,unsigned int *shift);


#ifdef __cplusplus
}
#endif

#define MIN_ALPHA		0
#define MAX_ALPHA		8
#define MIN_HUE			0
#define MAX_HUE			90
#define MIN_BRIGHT		-128
#define MAX_BRIGHT		127
#define MIN_CBGAIN		0
#define MAX_CBGAIN		255
#define MIN_CRGAIN		0
#define MAX_CRGAIN		255

#define	COARSE_SCALE	3
#define	FINE_SCALE		0

#define	OVL_NORMAL		1
#define	OVL_ALPHA		4
#define	OVL_COLKEY		5

typedef struct _CAMINFO
{
	unsigned int nDestWidth;		
	unsigned int nDestHeight;
	unsigned int nZoomIndex;
} CAMINFO, *LPCAMINFO;

typedef struct _CAM_GAMMATABLE
{
	short	gamma_table[1024];
} CAM_GAMMATABLE, *LPCAM_GAMMATABLE;

typedef struct _CAM_WBTABLE
{
	WORD	WBTable[16];
} CAM_WBTABLE, *LPCAM_WBTABLE;

// show Camera 
//		dwLenIn = 1 : Normal overlay
//		dwLenIn = 4 : Alphablending 
//		dwLenIn = 5 : Colorkey overlay
#define IOCTL_CAM_SHOW			CTL_CODE( FILE_DEVICE_VIDEO, 1, METHOD_NEITHER,FILE_ANY_ACCESS)

// hide Camera 
#define IOCTL_CAM_HIDE			CTL_CODE( FILE_DEVICE_VIDEO, 2, METHOD_NEITHER,FILE_ANY_ACCESS)

// Set Diplay position 
//		dwLenIn = MAKELONG(left, top)
//		dwLenOut = MAKELONG(width, height)
#define IOCTL_CAM_SETPOS		CTL_CODE( FILE_DEVICE_VIDEO, 3, METHOD_NEITHER,FILE_ANY_ACCESS)

// play 
#define IOCTL_CAM_CONT			CTL_CODE( FILE_DEVICE_VIDEO, 4, METHOD_NEITHER,FILE_ANY_ACCESS)

// stop
#define IOCTL_CAM_STOP			CTL_CODE( FILE_DEVICE_VIDEO, 5, METHOD_NEITHER,FILE_ANY_ACCESS)

// copy camera image to pBufOut (RGB565 format)
//		dwLenIn = 0 : top-down image
//		dwLenIn = 1 : bottom-up image
#define IOCTL_CAM_COPY			CTL_CODE( FILE_DEVICE_VIDEO, 6, METHOD_OUT_DIRECT,FILE_ANY_ACCESS)

// set colorkey
//		dwLenIn = RGB(R-value, G-value, B-value)
#define IOCTL_CAM_SETCOLKEY		CTL_CODE( FILE_DEVICE_VIDEO, 7, METHOD_NEITHER,FILE_ANY_ACCESS)

// set alpha value
//		dwLenIn = alpha value[0-8]
#define IOCTL_CAM_SETALPHA		CTL_CODE( FILE_DEVICE_VIDEO, 8, METHOD_NEITHER,FILE_ANY_ACCESS)

// get camera information
//		pBufOut = LPCAMINFO;
#define IOCTL_CAM_GETINFO		CTL_CODE( FILE_DEVICE_VIDEO, 9, METHOD_OUT_DIRECT,FILE_ANY_ACCESS)

// set scale for image_scaler
//		dwLenIn = scalemode (0:FINE, 3:COARSE)
//		pBufIn = (float*)&scale;
#define IOCTL_CAM_SETSCALE		CTL_CODE( FILE_DEVICE_VIDEO, 10, METHOD_IN_DIRECT,FILE_ANY_ACCESS)

// set hue value
//		dwLenIn = MAKELONG(hue_value[0-90], coord.[0-4]) ; if coord==0 then all coord. set
#define IOCTL_CAM_SETHUE		CTL_CODE( FILE_DEVICE_VIDEO, 11, METHOD_NEITHER,FILE_ANY_ACCESS)

// set gamma
//		gamma_off : dwLenIn = 0
//		gamma_on  : pBufIn = gamma_table; dwLenIn = sizeof(gamma_table=2048); 
#define IOCTL_CAM_SETGAMMA		CTL_CODE( FILE_DEVICE_VIDEO, 12, METHOD_IN_DIRECT,FILE_ANY_ACCESS)

// set White balance coordinate
//		pBufIn = WBtable;
#define IOCTL_CAM_SETWBCOORD	CTL_CODE( FILE_DEVICE_VIDEO, 13, METHOD_IN_DIRECT,FILE_ANY_ACCESS)

// set Auto Exposure accumulation limit
//		dwLenIn = MAKELONG(LowerLimit[0-255], UpperLimit[0-255]);
#define IOCTL_CAM_SETAELIMIT	CTL_CODE( FILE_DEVICE_VIDEO, 14, METHOD_NEITHER,FILE_ANY_ACCESS)

// set ADC offset value
//		dwLenIn = RGB((BYTE)R-offset[-127 - 128], (BYTE)G-offset[-127 - 128], (BYTE)B-offset[-127 - 128]);
#define IOCTL_CAM_SETADCOFS		CTL_CODE( FILE_DEVICE_VIDEO, 15, METHOD_NEITHER,FILE_ANY_ACCESS)

// set ADC offset value
//		Decimal_X_gain[5.4] = (unsigned char)(fGain[0.0-8.0]*0x20);
//		dwLenIn = RGB(Decimal_R_gain, Decimal_G_gain, Decimal_B_gain);
#define IOCTL_CAM_SETWBGAIN		CTL_CODE( FILE_DEVICE_VIDEO, 16, METHOD_NEITHER,FILE_ANY_ACCESS)

// set CbGain & CrGain
//		Decimal_Cx_gain[7.6] = (unsigned char)(fGain[0.0-2.0]*0x80);
//		dwLenIn = MAKELONG(Decimal_Cr_gain, Decimal_Cb_gain);
#define IOCTL_CAM_SETCBCRGAIN	CTL_CODE( FILE_DEVICE_VIDEO, 17, METHOD_NEITHER,FILE_ANY_ACCESS)

// set Luminance Enhancement Gain
//		Decimal_H_gain[4.3] = (unsigned char)(fHGain[0.0-8.0]*0x10);
//		Decimal_V_gain[4.3] = (unsigned char)(fVGain[0.0-8.0]*0x10);
//		Decimal_Main_gain[6.5] = (unsigned char)(fMainGain[0.0-4.0]*0x40);
//		dwLenIn = MAKELONG(Decimal_H_gain, Decimal_V_gain);
//		dwLenOut = Decimal_Main_Gain;
#define IOCTL_CAM_SETLEGAIN		CTL_CODE( FILE_DEVICE_VIDEO, 18, METHOD_NEITHER,FILE_ANY_ACCESS)

// set brightness
//		dwLenIn = (BYTE)brightness[-127 - 128];
#define IOCTL_CAM_SETBRIGHTNESS CTL_CODE( FILE_DEVICE_VIDEO, 19, METHOD_NEITHER,FILE_ANY_ACCESS)

// set Clip point & color supress 
//		dwLenIn = CSlantEn[0-1] << 31 | CSlant_val[0-1,[7.6]decimal] << 16 | Clip2[0-255] << 8 | Clip1[0-255];
#define IOCTL_CAM_SETCLIP		CTL_CODE( FILE_DEVICE_VIDEO, 20, METHOD_NEITHER,FILE_ANY_ACCESS)

// set Slice limit
//		dwLenIn = slice_limit[0-255];
#define IOCTL_CAM_SETSLICELMT	CTL_CODE( FILE_DEVICE_VIDEO, 21, METHOD_NEITHER,FILE_ANY_ACCESS)

// set White balance accumulation Limit
//		dwLenIn = MAKELONG(LowerLimit[0-255], UpperLimit[0-255]);
#define IOCTL_CAM_WBACCLMT		CTL_CODE( FILE_DEVICE_VIDEO, 22, METHOD_NEITHER,FILE_ANY_ACCESS)

// set Color supress Luma Edge limit
//		dwLenIn = MAKELONG(LowerLimit[0-255], UpperLimit[0-255]);
#define IOCTL_CAM_CSEDGELMT		CTL_CODE( FILE_DEVICE_VIDEO, 23, METHOD_NEITHER,FILE_ANY_ACCESS)

// Camera V-Sync Interrupt Handler Enable
#define IOCTL_CAM_ENVINT		CTL_CODE( FILE_DEVICE_VIDEO, 24, METHOD_NEITHER,FILE_ANY_ACCESS)

// Camera V-Sync Interrupt Handler Disable
#define IOCTL_CAM_DISVINT		CTL_CODE( FILE_DEVICE_VIDEO, 25, METHOD_NEITHER,FILE_ANY_ACCESS)

// set Camera
//		pBufIn = (LPTSTR)szCameraName;
#define IOCTL_CAM_SETCAMERA		CTL_CODE( FILE_DEVICE_VIDEO, 26, METHOD_IN_DIRECT,FILE_ANY_ACCESS)

#define CAM_IOCTL_GETJPEG			0x500
#define CAM_IOCTL_PRV_SETSIZE		0x501
#define CAM_IOCTL_PREVIEW_ON		0x502
#define CAM_IOCTL_PREVIEW_OFF		0x503
#define CAM_IOCTL_PRV_POSITION		0x504
#define CAM_IOCTL_ZOOM_ON			0x505
#define CAM_IOCTL_ZOOM_OFF			0x506
#define CAM_IOCTL_Y_FLIP			0x507
#define CAM_IOCTL_X_FLIP			0x508
#define CAM_IOCTL_MOVIE_START		0x509
#define CAM_IOCTL_MOVIE_STOP		0x50A
#define CAM_IOCTL_GET_LATEST_FRAME	0x50B
#define CAM_IOCTL_POWER_RESUME		0x50C
#define CAM_IOCTL_SET_RESOLUTION	0x50D
#define CAM_IOCTL_GET_RESOLUTION	0x50E
#define CAM_IOCTL_SET_BRIGHTNESS	0x50F
#define CAM_IOCTL_SET_SENSOR		0x510
#define CAM_IOCTL_GET_SENSOR		0x511
#define CAM_IOCTL_SET_ASIC			0x512
#define CAM_IOCTL_GET_ASIC			0x513
#define CAM_IOCTL_LM_PREVIEW		0x514
#define CAM_IOCTL_GET_QFACTOR		0x515
#define CAM_IOCTL_SET_WHITEBAL		0x516
#define CAM_IOCTL_SAMSUNG_CAM		0x520
//#define CAM_IOCTL_IMAGE_SIZE		0x521 
#define CAM_IOCTL_IMAGE_CHANGE	0x521 
#define CAM_IOCTL_SAMSUNG_CAM_PR	0x522
#define CAM_IOCTL_SAMSUNG_PREVIEW_START	0x523
#define CAM_IOCTL_SAMSUNG_PREVIEW_STOP	0x524


#endif /* __CAMERA_H_ */
