/*
 * zc030xlib.h
 *
 * zc030x static/dynamic library support
 *
 * USB Camera driver for Windows CE
 *
 * Copyright (c) 2005-2006 for Cyansoft Studio (www.cyansoft.com.cn).
 * All Rights Reserved.
 *
 *
 * Contributor(s): ______________________________________.
 *
 * $Cyansoft: projects/zc030x/wince/dev/zc030xlib/zc030xlib.h,v 1.0.0.1 2006/05/20 15:47:02 jin.bai Exp $
 */
#ifndef _ZC030X_LIB_INC_
#define _ZC030X_LIB_INC_

#ifdef __cplusplus
extern "C" {
#endif

/* Error code */
#define INVALID_PARAMETER		-2		
#define INVALID_MACHINE_ID		-3		
#define INVALID_CAMERA_INDEX		-4		
#define ERROR_LOAD_LIBRARY		-5		
#define ERROR_GET_ADDR			-6	
#define INVALID_DEVICE_ID		-7

/* Supported video format and size list */
enum {
	VIDEO_PALETTE_RGB565 = 0,	/* R:5, G:6, B:5 */
	VIDEO_PALETTE_RGB24,		/* R:8, G:8, B:8 */
	VIDEO_PALETTE_RGB32,		/* R:8, G:8, B:8, A:8 */
	VIDEO_PALETTE_YUV420P,		/* Y:4, U:1, V:1 */
	VIDEO_PALETTE_JPEG,		/* JPEG */	
};

enum {
	VIDEO_SIZE_VGA = 0,		/* 640 x 480 */
	VIDEO_SIZE_SIF,			/* 320 x 240 */
};

/* Function list */
#ifdef ZC030XLIB_EXPORTS
#define ZC030XLIB_API __declspec(dllexport)
#else
#define ZC030XLIB_API
#endif

/* Name : capInitCamera
 * Initialize driver and get current available cameras number.
 * Return : The number, of the available cameras, indicates success.
 */
ZC030XLIB_API int capInitCamera(void); 

/* Name : capGetCurrentVersion
 * Get the current driver version information.
 * Return : The length, in characters, of the copied string, 
 *			not including the terminating null character, indicates success.
 */
ZC030XLIB_API 
int capGetCurrentVersion(
						 int index,			/* [IN] Camera index, 0 - first */
						 unsigned char *pBufOut,	/* [OUT] Long pointer to the buffer that will receive the text */
						 int lenOut			/* [IN] Specifies the maximum number of characters to copy to the buffer, 
											including the NULL character. If the text exceeds this limit, it is truncated. */
						 );				

/* 
 * Name : capGetVideoFormat
 * Get the current video format.
 * Return : Zero indicates success.
 */
ZC030XLIB_API 
int capGetVideoFormat(
					  int index,		/* [IN] Camera index */
					  int *pFormat,		/* [OUT] Long pointer to the buffer that will receive the video format, VIDEO_PALETTE_RGB565 - VIDEO_PALETTE_JPEG */
					  int *pSizeMode	/* [OUT] Long pointer to the buffer that will receive the size mode, VIDEO_SIZE_VGA - VIDEO_SIZE_SIF */
					  );

/* 
 * Name : capSetVideoFormat
 * Set video format.
 * Return : Zero indicates success.
 */
ZC030XLIB_API 
int capSetVideoFormat(
					  int index,		/* [IN] Camera index */
					  int format,		/* [IN] Video format, VIDEO_PALETTE_RGB565 - VIDEO_PALETTE_JPEG */
					  int sizeMode		/* [IN] Size mode, VIDEO_SIZE_VGA - VIDEO_SIZE_SIF */
					  );

/* 
 * Name : capGrabFrame
 * Grab a frame from driver.
 * Return : The length, in bytes, of the copied video frame data, indicates success.
 */
ZC030XLIB_API 
int capGrabFrame(
				 int index,				/* [IN] Camera index */
				 unsigned char *pFrameBuf,		/* [OUT] Long pointer to the buffer that will receive the video frame */
				 unsigned int bufferLen			/* [IN] Specifies the maximum number of bytes to copy to the buffer */												
				 );

/* 
 * Name : capGetLastJpeg
 * Get a last JPEG frame from driver.
 * Return : The length, in bytes, of the copied JPEG frame data, indicates success.
 */
ZC030XLIB_API 
int capGetLastJpeg(
				   int index,				/* [IN] Camera index */
				   unsigned char *pFrameBuf,		/* [OUT] Long pointer to the buffer that will receive the video frame */
				   unsigned int bufferLen		/* [IN] Specifies the maximum number of bytes to copy to the buffer */
				   );

/* 
 * Name : capStartCamera
 * Start camera to capture video.
 * Return : Zero indicates success.
 */
ZC030XLIB_API 
int capStartCamera(
				   int index			/* [IN] Camera index */
				   ); 

/* 
 * Name : capStopCamera
 * Stop camera to capture video.
 * Return : Zero indicates success.
 */
ZC030XLIB_API 
int capStopCamera(
				  int index				/* [IN] Camera index */
				  );

/* 
 * Name : capCloseCamera
 * Close all available camera.
 * Return : Void.
 */
ZC030XLIB_API void capCloseCamera(void);

/*
 * Name : capGetPciture
 * Get a last bitmap and jpeg from driver
 * Return : Zero indicates success.
 */
ZC030XLIB_API 
int capGetPicture(
				  int index,			/* [IN] Camera index */
				  unsigned char *pFrameBuf, 	/* [OUT] Long pointer to the buffer that will receive the video frame */
				  unsigned int bufferLen,	/* [IN] Size, in bytes, of the buffer pointed to by pFrameBuf */
				  unsigned char *pJpgBuf,	/* [OUT] Long pointer to the buffer that will receive the jpeg picture */
				  unsigned int jpgLen,		/* [IN] Size, in bytes, of the buffer pointed to by pJpgBuf */
				  PDWORD pdwActualOut		/* [OUT] Pointer to an array of the return lengths */
				  );


#ifdef __cplusplus
}
#endif

#endif /* _ZC030X_LIB_INC_ */
/* End of file */
