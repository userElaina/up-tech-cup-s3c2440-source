/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 2003. Samsung Electronics, co. ltd  All rights reserved.

Module Name:  

Abstract:

	This file implements the S3C2440 LCD headers

rev:
	2004.12.13	: First S3C2440 version of DirectDraw

Notes: 
--*/

#ifndef __S3C2440DISP_H__
#define __S3C2440DISP_H__


class S3C2440DISP : public DDGPE
{
protected:
	GPEModeEx       	m_ModeInfoEx;           // local mode info
    	ULONG           	m_nSurfaceBitsAlign;    // Surface bits alignments

private:
	GPEMode			m_ModeInfo;
	DWORD			m_cbScanLineLength;
	DWORD			m_cxPhysicalScreen;
	DWORD			m_cyPhysicalScreen;
	DWORD			m_colorDepth;
	DWORD			m_VirtualFrameBuffer;
	DWORD			m_FrameBufferSize;
	SurfaceHeap		*m_pVideoMemoryHeap;     // Base entry representing all video memory
	BOOL			m_CursorDisabled;
	BOOL			m_CursorVisible;
	BOOL			m_CursorForcedOff;
	RECTL			m_CursorRect;
	POINTL			m_CursorSize;
	POINTL			m_CursorHotspot;
	UCHAR			*m_CursorBackingStore;
	UCHAR			*m_CursorXorShape;
	UCHAR			*m_CursorAndShape;
	ULONG			m_VideoPowerState;
	DWORD			m_dwLCDPhysicalFrameBase;
	DWORD			m_bClearAlternateVideoBCR;

public:
	S3C2440DISP(void);
	~S3C2440DISP();
	virtual INT	NumModes(void);
	virtual SCODE	SetMode(INT modeId,	HPALETTE *palette);
	virtual INT	InVBlank(void);
	virtual SCODE	SetPalette(const PALETTEENTRY *source, USHORT firstEntry,
								USHORT numEntries);
	virtual SCODE	GetModeInfo(GPEMode *pMode,	INT modeNumber);
	virtual SCODE	SetPointerShape(GPESurf *mask, GPESurf *colorSurface,
									INT xHot, INT yHot, INT cX, INT cY);
	virtual SCODE	MovePointer(INT xPosition, INT yPosition);
	virtual void	WaitForNotBusy(void);
	virtual INT	    IsBusy(void);
	virtual void	GetPhysicalVideoMemory(unsigned long *physicalMemoryBase, unsigned long *videoMemorySize);

	void    GetVirtualVideoMemory(unsigned long * virtualMemoryBase, unsigned long *videoMemorySize);

	virtual SCODE	Line(GPELineParms *lineParameters, EGPEPhase phase);
	virtual SCODE	BltPrepare(GPEBltParms *blitParameters);
	virtual SCODE	BltComplete(GPEBltParms *blitParameters);
	int GetBpp(void);
	virtual SCODE S3C2440DISP::GetModeInfoEx(GPEModeEx *pModeEx, int modeNo);
//	virtual ULONG	GetGraphicsCaps();
	virtual ULONG DrvEscape(
                        SURFOBJ *pso,
                        ULONG    iEsc,
                        ULONG    cjIn,
                        PVOID    pvIn,
                        ULONG    cjOut,
                        PVOID    pvOut);
	int GetGameXInfo(ULONG, ULONG, VOID*, ULONG, VOID*);  
	int GetRotateModeFromReg();
	void SetRotateParams();
	LONG DynRotate(int angle);
	SCODE RotateCursorShape(int angle);
//#endif 
	SCODE		WrappedEmulatedLine (GPELineParms *lineParameters);
	void			CursorOn (void);
	void			CursorOff (void);

	virtual SCODE	AllocSurface(GPESurf **surface, int width, int height, 
					EGPEFormat format, int surfaceFlags);
	virtual SCODE AllocSurface(DDGPESurf **ppSurf, int width, int height, 
					EGPEFormat format, EDDGPEPixelFormat pixelFormat, int surfaceFlags);
	
    	virtual void	SetVisibleSurface( GPESurf *pSurf, BOOL bWaitForVBlank = FALSE);

	void			InitializeHardware (void);
	void			SetDisplayPower(ULONG);

	friend void 		buildDDHALInfo( LPDDHALINFO lpddhi, DWORD modeidx );
 };


class S3C2440Surf : public DDGPESurf
{
private:
	SurfaceHeap *m_pHeap;
	HANDLE m_hSurface;
	UCHAR *m_pSurface;

public:

	S3C2440Surf(int, int, DWORD, VOID*, int, EGPEFormat, EDDGPEPixelFormat pixelFormat, SurfaceHeap*);
	virtual ~S3C2440Surf();

	VOID* SurfaceAddress() { return m_pSurface; }
	VOID  WriteBack();
	VOID  Discard();
	BOOL  SurfaceOk() { return m_pHeap != NULL || m_pSurface != NULL; }
};

#endif __S3C2440DISP_H__

