/*****************************************
  NAME: Camif.h
  DESC: header file for Camera Interface test codes 
  HISTORY:  2002.03.13:draft ver 0.0
 *****************************************/

#ifndef __CAMIF_H__
#define __CAMIF_H__

#ifdef __cplusplus
extern "C"{
#endif

#define U32	unsigned int

//2440 camif.h/////////////////////////////////////////////////////

#define CAM_CCIR420						(0)
#define CAM_CCIR422						(1)
#define CAM_RGB16B						0
#define CAM_RGB24B						1

#define CAM_ITU601						(1)
#define CAM_ITU656						(0)

#define CAM_ORDER_YCBYCR				(0)
#define CAM_ORDER_YCRYCB				(1)
#define CAM_ORDER_CBYCRY				(2)
#define CAM_ORDER_CRYCBY				(3)

#define CAM_CODEC_IN_420				(0)
#define CAM_CODEC_IN_422				(1)

#define CAM_CODEC_OUT_420				(0)
#define CAM_CODEC_OUT_422				(1)

#define CAM_PVIEW_PINGPONG				FALSE
#define CAM_CODEC_PINGPONG				FALSE

#define CAM_FLIP_NORMAL					(0)
#define CAM_FLIP_XAXIS					(1)
#define CAM_FLIP_YAXIS					(2)
#define CAM_FLIP_180						(3)

#define CAM_SCALER_BYPASS_ON			(1)
#define CAM_SCALER_BYPASS_OFF			(0)

#define CAM_CODEC_SACLER_START_BIT			(1<<15)
#define CAM_PVIEW_SACLER_START_BIT			(1<<15)

#define CAM_CAMIF_GLOBAL_CAPTURE_ENABLE_BIT		(1<<31)
#define CAM_CODEC_SCALER_CAPTURE_ENABLE_BIT		(1<<30)
#define CAM_PVIEW_SCALER_CAPTURE_ENABLE_BIT		(1<<29)

//camTestMode
#define CAM_TEST_MODE_PVIEW				(1<<0)
#define CAM_TEST_MODE_CODEC				(1<<1)

//camCodecStatus, camPviewStatus
#define CAM_STARTED						(0)
#define CAM_STOP_ISSUED					(1)
#define CAM_LAST_CAPTURING				(2)
#define CAM_STOPPED						(3)
#define CAM_CODEC_SCALER_BYPASS_STATE	(4)


#define CAPTURE_ON	(1)
#define CAPTURE_OFF	(0)
#define HIGH		(1)
#define LOW			(0)

#define CAM_A	(0)
#define CAM_B	(1)

#define Y_BURST1_XGA		(16)
#define Y_BURST2_XGA		(16)
#define CB_BURST1_XGA		(16)
#define CB_BURST2_XGA		(16)
#define CR_BURST1_XGA		(16)
#define CR_BURST2_XGA		(16)

#define Y_BURST1_VGA		(16)
#define Y_BURST2_VGA		(16)
#define CB_BURST1_VGA		(16)
#define CB_BURST2_VGA		(16)
#define CR_BURST1_VGA		(16)
#define CR_BURST2_VGA		(16)

#define Y_BURST1_CIF		(16)
#define Y_BURST2_CIF		(8)
#define CB_BURST1_CIF		(8)
#define CB_BURST2_CIF		(4)
#define CR_BURST1_CIF		(8)
#define CR_BURST2_CIF		(4)

#define Y_BURST1_PQVGA		(8)
#define Y_BURST2_PQVGA		(4)
#define CB_BURST1_PQVGA		(4)
#define CB_BURST2_PQVGA		(2)
#define CR_BURST1_PQVGA		(4)
#define CR_BURST2_PQVGA		(2)

// JunKim, added 041004
#define CIS_S5X532           1   // OLD VGA CIS
#define CIS_S5X532_Rev36     2   //VGA CIS
#define CIS_S5X3A1           3   //megapixel CIS
#define CIS_S5X3AA	     4 	 
#define CIS_OV9650           5


#define CIS_TYPE             CIS_OV9650//CIS_S5X3A1//CIS_S5X3A1
#define S5X3A1_CamMode       0  //0:VGA   1:Mega

//Image Type of Camera Interface
#define CAMERA_XGA_VGA_XGA		(0x9121)//SRC 4:2:2 YCbCr 1024*768, DST 4:2:0 YCbCr A: 640*480 B:1024*768
#define CAMERA_XGA_PQVGA_XGA		(0x9131)//SRC 4:2:2 YCbCr 1024*768, DST 4:2:0 YCbCr A:240*320 B:1024*768
#define CAMERA_XGA_PQVGA_VGA		(0x9132)//SRC 4:2:2 YCbCr 1024*768, DST 4:2:0 YCbCr A:240*320 B:640*480
#define CAMERA_VGA_PQVGA_VGA		(0x9232)//SRC 4:2:2 YCbCr 640*480, DST 4:2:0 YCbCr A:240*320 B:640*480

// 1st Ping-pong Address
//#define COPIFRAMEBUFFER_A 	0x30088000	// 176*144 * 2(RGB 16bit) * 4 frames = 202752(0x31800)
// 0x30088000 + 0x31800 = 0x300B9800
//#define COPIFRAMEBUFFER_B 	0x300B9800	// Max buffer size of Port : 0x360000 (1024*768+256*192*2)*4
//#define COPIFRAMEBUFFER_B 	0x303B0000	// Max buffer size of Port : 0x360000 (1024*768+256*192*2)*4

#define COPIFRAMEBUFFER_A 	0x33BEAA00	// Preview RGB(MAX 240*180): 240*180 * 2(RGB 16bit) * 4 frames = 345600(0x54600)

#define COPIFRAMEBUFFER_B 	0x33C40000	// Codec YUV(MAX 1280*1024): 0x3C0000 (1280*1024*3/2)*2frame


#define VIRTUAL_OFFSET		0x70000000
#define VIRTUAL_ADDR_OFFSET	VIRTUAL_OFFSET // for MPEG4

//Image Size
#define SXGA_XSIZE			(1280)
#define SXGA_YSIZE			(1024)
#define XGA_XSIZE			(1024)
#define XGA_YSIZE			(768)
#define SVGA_XSIZE			(800)
#define SVGA_YSIZE			(600)
#define VGA_XSIZE			(640)
#define VGA_YSIZE			(480)
#define QVGA_XSIZE			(320)
#define QVGA_YSIZE			(240)
#define PQVGA_XSIZE			(240)
#define PQVGA_YSIZE			(320)
#define CIF_XSIZE			(352)
#define CIF_YSIZE			(288)

#define QCIF_XSIZE			(176)
#define QCIF_YSIZE			(144)
#define PREVIEW_X			(240)
#define PREVIEW_Y			(180)

#define XGA					(1)
#define SVGA				(2)
#define VGA					(3)
#define CIF					(4)
#define PQVGA				(5)
#define QCIF				(6)

#define CAMCLK48000000		(1)
#define CAMCLK32000000		(2)
#define CAMCLK24000000		(3)
#define CAMCLK16000000		(4)
#define CAMCLK12000000		(5)
#define CAMCLK9600000		(6)
#define CAMCLK8000000		(7)

#define IN1PINGPONG			(1)
#define IN2PINGPONG			(2)
#define IN4PINGPONG			(4)

#define OUT1PINGPONG		(1)
#define OUT2PINGPONG		(2)
#define OUT4PINGPONG		(4)
//////////////////////////////////////////////////////////////////


// Current modes
#define USED_CAM_TYPE				CIS_TYPE
#define USED_LCD_TYPE  				MODE_TFT_16BIT_240320  // refer to Lcdlib.h 
//AU70H camera resolution
#define AU70H_VIDEO_SIZE			(1152)	  // 1152:(1152x864) or 640:(640x480)	
#define CAM_CODEC_OUTPUT		CAM_CCIR420
#define CAM_PVIEW_OUTPUT		CAM_RGB16B

#define CAM_CODEC_4PP	(1) // 0:all equal, 1:4 pingpong
#define CAM_PVIEW_4PP	(1) // 0:all equal, 1:4 pingpong

/*
#if USED_CAM_TYPE==CAM_AU70H && AU70H_VIDEO_SIZE==1152
#define CAM_SRC_HSIZE				(1152)
#define CAM_SRC_VSIZE					(864)
#elif USED_CAM_TYPE==CAM_S5X3A1
#define CAM_SRC_HSIZE				(1280)
#define CAM_SRC_VSIZE					(1024)
#else
#define CAM_SRC_HSIZE				(640)
#define CAM_SRC_VSIZE					(480)
#endif
*/
#if (S5X3A1_CamMode == 1)
#define CAM_SRC_HSIZE				(1280)
#define CAM_SRC_VSIZE				(1024)
#else
#define CAM_SRC_HSIZE				(640)
#define CAM_SRC_VSIZE				(480)
#endif

//#define Preview_Mem_Size   CAM_SRC_HSIZE*CAM_SRC_VSIZE*2*4
//#define Codec_Mem_Size	   (((CAM_SRC_HSIZE*CAM_SRC_VSIZE) + (CAM_SRC_HSIZE*CAM_SRC_VSIZE*2/4))*4)//1280*1024*3/2
#define Preview_Mem_Size   240*180*2*4
#define Codec_Mem_Size	   1280*1024*3/2

//void __irq Camif_Int_S(void);
//void __irq Camif_Int_C(void);
void Camif_Int_Mask_S(void);
void Camif_Int_Mask_C(void);
void Camif_Int_Unmask_S(void);
void Camif_Int_Unmask_C(void);
void Camif_Init_Old(int type);

// 2003.05.12
void Camif_Init(U32 src_xsize, U32 src_ysize, U32 dst_xsize_a, U32 dst_ysize_a, U32 dst_xsize_b, U32 dst_ysize_b, U32 FrameBufferA, U32 FrameBufferB, U32 camclk);
void Camif_Capture(int cap_a );
void Camif_If_Reset(void);
void Camera_Processor_Reset(void);
void SetCAMClockDivider(int divn);// 2003.05.12
void Camif_Init_640480(U32 camclk);
void Camif_Init_VGA_CIF(U32 camclk);
void Camera_Test(void);
void Test_Cam_Post_Lcd_One(void);
void Test_Cam_VGA_CIF(void);
void Test_Cam_CIF(void);
void Test_Img_Cam_24bit_240320(void);
void Test_Img_Cam_24bit_640480(void);
void Move_Memory(U32 SrcMemory, U32 SrcXsize, U32 SrcYsize, U32 DstMemory, U32 DstXsize, U32 DstYsize);


//#endif /*__POST_H__*/

#if (CIS_TYPE == CIS_OV9650) 
#define CAM_CLK_DIV			0	
#else
#define CAM_CLK_DIV			1			// 96M / (DIV + 1) - 0:96M, 1:48M, 2:32M, 3:24M, 4:19.2M, 5:16M
#endif


// for MPEG4
typedef struct _YUVINFO
{
	int frame_width;
	int frame_height;
	unsigned int frame_stamp;
} YUVINFO;

// Availabe ping-pong address
typedef struct PINGPONG
{
	unsigned int y_address;
	unsigned int cb_address;
	unsigned int cr_address;
	unsigned char flag;
} PINGPONG;

typedef struct PINGPONG_PR
{
	unsigned int rgb_address;
	unsigned char flag;		
} PINGPONG_PR;

#ifdef __cplusplus
}
#endif

#endif /* __CAMIF_H__ */
