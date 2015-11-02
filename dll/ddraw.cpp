// to do: duplicate EnumSurfaces(D) handling 
// fix Unlock duplicate hook in Judge Dredd Pinball

#define _CRT_SECURE_NO_WARNINGS
#define INITGUID
//#define FULLHEXDUMP

#include <windows.h>
#include <ddraw.h>
#include "dxwnd.h"
#include "dxhook.h"
#include "ddrawi.h"
#include "dxwcore.hpp"
#include "stdio.h" 
#include "hddraw.h"
#include "ddproxy.h"
#include "dxhelper.h"
#include "syslibs.h"

extern BOOL IsChangeDisplaySettingsHotPatched;
BOOL bDontReleaseBackBuffer = FALSE;
DWORD dwBackBufferCaps;
extern void TextureHandling(LPDIRECTDRAWSURFACE);
ColorConversion_Type pColorConversion = NULL;

// DirectDraw API
HRESULT WINAPI extDirectDrawCreate(GUID FAR *, LPDIRECTDRAW FAR *, IUnknown FAR *);
HRESULT WINAPI extDirectDrawCreateEx(GUID FAR *, LPDIRECTDRAW FAR *, REFIID, IUnknown FAR *);
HRESULT WINAPI extDirectDrawEnumerate(LPDDENUMCALLBACK, LPVOID);
HRESULT WINAPI extDirectDrawEnumerateEx(LPDDENUMCALLBACKEX, LPVOID, DWORD);

// DirectDraw
HRESULT WINAPI extQueryInterfaceD(void *, REFIID, LPVOID *);
ULONG WINAPI extReleaseD(LPDIRECTDRAW);
    /*** IDirectDraw methods ***/
HRESULT WINAPI extCreateClipper(LPDIRECTDRAW, DWORD, LPDIRECTDRAWCLIPPER FAR* , IUnknown FAR*);
HRESULT WINAPI extCreatePalette(LPDIRECTDRAW, DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE *, IUnknown *);
HRESULT WINAPI extCreateSurface1(LPDIRECTDRAW, DDSURFACEDESC *, LPDIRECTDRAWSURFACE *, void *);
HRESULT WINAPI extCreateSurface2(LPDIRECTDRAW, DDSURFACEDESC *, LPDIRECTDRAWSURFACE *, void *);
HRESULT WINAPI extCreateSurface4(LPDIRECTDRAW, DDSURFACEDESC2 *, LPDIRECTDRAWSURFACE *, void *);
HRESULT WINAPI extCreateSurface7(LPDIRECTDRAW, DDSURFACEDESC2 *, LPDIRECTDRAWSURFACE *, void *);
HRESULT WINAPI extDuplicateSurface(LPDIRECTDRAW, LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE *);
HRESULT WINAPI extFlipToGDISurface(LPDIRECTDRAW);
HRESULT WINAPI extGetDisplayMode(LPDIRECTDRAW, LPDDSURFACEDESC);
HRESULT WINAPI extGetGDISurface(LPDIRECTDRAW, LPDIRECTDRAWSURFACE *);
HRESULT WINAPI extEnumDisplayModes1(LPDIRECTDRAW, DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMMODESCALLBACK);
HRESULT WINAPI extEnumDisplayModes4(LPDIRECTDRAW, DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMMODESCALLBACK2);
HRESULT WINAPI extInitialize(LPDIRECTDRAW, FAR GUID *);
HRESULT WINAPI extSetCooperativeLevel(void *, HWND, DWORD);
HRESULT WINAPI extSetDisplayMode1(LPDIRECTDRAW, DWORD, DWORD, DWORD);
HRESULT WINAPI extSetDisplayMode2(LPDIRECTDRAW, DWORD, DWORD, DWORD, DWORD, DWORD);
HRESULT WINAPI extWaitForVerticalBlank(LPDIRECTDRAW, DWORD, HANDLE);
    /*** Added in the V2 Interface ***/
HRESULT WINAPI extGetAvailableVidMem2(LPDIRECTDRAW, LPDDSCAPS, LPDWORD, LPDWORD);
HRESULT WINAPI extGetAvailableVidMem4(LPDIRECTDRAW, LPDDSCAPS, LPDWORD, LPDWORD);
    /*** Added in the V4 Interface ***/
HRESULT WINAPI extTestCooperativeLevel(LPDIRECTDRAW);
//    STDMETHOD(StartModeTest)(THIS_ LPSIZE, DWORD, DWORD ) PURE;
//    STDMETHOD(EvaluateMode)(THIS_ DWORD, DWORD * ) PURE;
HRESULT WINAPI extGetCapsD(LPDIRECTDRAW, LPDDCAPS, LPDDCAPS);


// DirectDrawSurface
HRESULT WINAPI extQueryInterfaceS(void *, REFIID, LPVOID *);
HRESULT WINAPI extReleaseS(LPDIRECTDRAWSURFACE);

    /*** IDirectDrawSurface methods ***/
HRESULT WINAPI extAddAttachedSurface(LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE);
HRESULT WINAPI extBlt(LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDBLTFX);
HRESULT WINAPI extBltFast(LPDIRECTDRAWSURFACE, DWORD, DWORD, LPDIRECTDRAWSURFACE, LPRECT, DWORD);
HRESULT WINAPI extDeleteAttachedSurface(LPDIRECTDRAWSURFACE,  DWORD, LPDIRECTDRAWSURFACE);
HRESULT WINAPI extEnumAttachedSurfaces(LPDIRECTDRAWSURFACE, LPVOID, LPDDENUMSURFACESCALLBACK);
HRESULT WINAPI extFlip(LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE, DWORD);
HRESULT WINAPI extGetAttachedSurface1(LPDIRECTDRAWSURFACE, DDSCAPS *, LPDIRECTDRAWSURFACE *);
HRESULT WINAPI extGetAttachedSurface3(LPDIRECTDRAWSURFACE, DDSCAPS *, LPDIRECTDRAWSURFACE *);
HRESULT WINAPI extGetAttachedSurface4(LPDIRECTDRAWSURFACE, DDSCAPS *, LPDIRECTDRAWSURFACE *);
HRESULT WINAPI extGetAttachedSurface7(LPDIRECTDRAWSURFACE, DDSCAPS *, LPDIRECTDRAWSURFACE *);
HRESULT WINAPI extGetCaps1S(LPDIRECTDRAWSURFACE, LPDDSCAPS);
HRESULT WINAPI extGetCaps2S(LPDIRECTDRAWSURFACE, LPDDSCAPS);
HRESULT WINAPI extGetCaps3S(LPDIRECTDRAWSURFACE, LPDDSCAPS);
HRESULT WINAPI extGetCaps4S(LPDIRECTDRAWSURFACE, LPDDSCAPS2);
HRESULT WINAPI extGetCaps7S(LPDIRECTDRAWSURFACE, LPDDSCAPS2);
HRESULT WINAPI extGetColorKey(LPDIRECTDRAWSURFACE, DWORD, LPDDCOLORKEY);
HRESULT WINAPI extGetDC(LPDIRECTDRAWSURFACE, HDC FAR *);
HRESULT WINAPI extGetPalette(LPDIRECTDRAWSURFACE, LPDIRECTDRAWPALETTE *);
HRESULT WINAPI extGetPixelFormat(LPDIRECTDRAWSURFACE, LPDDPIXELFORMAT);
HRESULT WINAPI extGetSurfaceDesc1(LPDIRECTDRAWSURFACE lpdds, LPDDSURFACEDESC lpddsd);
HRESULT WINAPI extGetSurfaceDesc2(LPDIRECTDRAWSURFACE2 lpdds, LPDDSURFACEDESC2 lpddsd);
HRESULT WINAPI extGetSurfaceDesc7(LPDIRECTDRAWSURFACE2 lpdds, LPDDSURFACEDESC2 lpddsd);
//    STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW, LPDDSURFACEDESC2) PURE;
HRESULT WINAPI extLock(LPDIRECTDRAWSURFACE, LPRECT, LPDDSURFACEDESC, DWORD, HANDLE);
HRESULT WINAPI extLockDir(LPDIRECTDRAWSURFACE, LPRECT, LPDDSURFACEDESC, DWORD, HANDLE);
HRESULT WINAPI extReleaseDC(LPDIRECTDRAWSURFACE, HDC);
HRESULT WINAPI extSetClipper(LPDIRECTDRAWSURFACE, LPDIRECTDRAWCLIPPER);
HRESULT WINAPI extSetColorKey(LPDIRECTDRAWSURFACE, DWORD, LPDDCOLORKEY);
HRESULT WINAPI extSetPalette(LPDIRECTDRAWSURFACE, LPDIRECTDRAWPALETTE);
HRESULT WINAPI extUnlock4(LPDIRECTDRAWSURFACE, LPRECT);
HRESULT WINAPI extUnlock1(LPDIRECTDRAWSURFACE, LPVOID);
HRESULT WINAPI extUnlockDir4(LPDIRECTDRAWSURFACE, LPRECT);
HRESULT WINAPI extUnlockDir1(LPDIRECTDRAWSURFACE, LPVOID);

HRESULT WINAPI extCreateSurface(int, CreateSurface_Type, LPDIRECTDRAW, DDSURFACEDESC2 *, LPDIRECTDRAWSURFACE *, void *);
HRESULT WINAPI extSetSurfaceDesc(LPDIRECTDRAWSURFACE, LPDDSURFACEDESC, DWORD);

// DirectDrawClipper
HRESULT WINAPI extReleaseC(LPDIRECTDRAWCLIPPER);

// DirectDrawPalette
HRESULT WINAPI extReleaseP(LPDIRECTDRAWPALETTE);
//    STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW, DWORD, LPPALETTEENTRY) PURE;
HRESULT WINAPI extSetEntries(LPDIRECTDRAWPALETTE, DWORD, DWORD, DWORD, LPPALETTEENTRY);

// GammaRamp
HRESULT WINAPI extDDSetGammaRamp(LPDIRECTDRAWSURFACE, DWORD, LPDDGAMMARAMP);
HRESULT WINAPI extDDGetGammaRamp(LPDIRECTDRAWSURFACE, DWORD, LPDDGAMMARAMP);


HDC WINAPI extGDIGetDC(HWND);
HDC WINAPI extGDIGetWindowDC(HWND);
int WINAPI extGDIReleaseDC(HWND, HDC);

/* DirectDraw APIs */
DirectDrawCreate_Type pDirectDrawCreate = NULL;
DirectDrawCreateEx_Type pDirectDrawCreateEx = NULL;
DirectDrawEnumerate_Type pDirectDrawEnumerate = NULL;
DirectDrawEnumerateEx_Type pDirectDrawEnumerateEx = NULL;

/* DirectDraw hook pointers */
QueryInterface_Type pQueryInterfaceD;
AddRefD_Type pAddRefD;
ReleaseD_Type pReleaseD;
Compact_Type pCompact;
CreateClipper_Type pCreateClipper=NULL;
CreatePalette_Type pCreatePalette;
CreateSurface1_Type pCreateSurface1;
CreateSurface1_Type pCreateSurface2;
CreateSurface1_Type pCreateSurface3;
CreateSurface2_Type pCreateSurface4;
CreateSurface2_Type pCreateSurface7;
DuplicateSurface_Type pDuplicateSurface;
EnumDisplayModes1_Type pEnumDisplayModes1;
EnumDisplayModes4_Type pEnumDisplayModes4;
EnumSurfaces1_Type pEnumSurfaces1;
EnumSurfaces4_Type pEnumSurfaces4;
FlipToGDISurface_Type pFlipToGDISurface;
GetCapsD_Type pGetCapsD;
GetDisplayMode_Type pGetDisplayMode;
GetFourCCCodes_Type pGetFourCCCodes;
GetGDISurface_Type pGetGDISurface;
GetMonitorFrequency_Type pGetMonitorFrequency;
GetScanLine_Type pGetScanLine;
GetVerticalBlankStatus_Type pGetVerticalBlankStatus;
Initialize_Type pInitialize;
RestoreDisplayMode_Type pRestoreDisplayMode;
SetCooperativeLevel_Type pSetCooperativeLevel;
SetDisplayMode1_Type pSetDisplayMode1;
SetDisplayMode2_Type pSetDisplayMode2;
WaitForVerticalBlank_Type pWaitForVerticalBlank;
GetSurfaceFromDC_Type pGetSurfaceFromDC;
GetAvailableVidMem_Type pGetAvailableVidMem;
GetAvailableVidMem_Type pGetAvailableVidMem2;
GetAvailableVidMem_Type pGetAvailableVidMem4;
RestoreAllSurfaces_Type pRestoreAllSurfaces;
TestCooperativeLevel_Type pTestCooperativeLevel;
GetDeviceIdentifier_Type pGetDeviceIdentifier;

/* DirectDrawSurface hook pointers */
QueryInterface_Type pQueryInterfaceS;
AddRefS_Type pAddRefS;
ReleaseS_Type pReleaseS;
AddAttachedSurface_Type pAddAttachedSurface;
AddOverlayDirtyRect_Type pAddOverlayDirtyRect;
Blt_Type pBlt;
BltBatch_Type pBltBatch;
BltFast_Type pBltFast;
DeleteAttachedSurface_Type pDeleteAttachedSurface;
EnumAttachedSurfaces_Type pEnumAttachedSurfaces;
EnumOverlayZOrders_Type pEnumOverlayZOrders;
Flip_Type pFlip;
GetAttachedSurface_Type pGetAttachedSurface1;
GetAttachedSurface_Type pGetAttachedSurface3;
GetAttachedSurface_Type pGetAttachedSurface4;
GetAttachedSurface_Type pGetAttachedSurface7;
GetBltStatus_Type pGetBltStatus;
GetCapsS_Type pGetCaps1S;
GetCapsS_Type pGetCaps2S;
GetCapsS_Type pGetCaps3S;
GetCaps2S_Type pGetCaps4S;
GetCaps2S_Type pGetCaps7S;
GetClipper_Type pGetClipper;
GetColorKey_Type pGetColorKey;
GetDC_Type pGetDC;
GetFlipStatus_Type pGetFlipStatus;
GetOverlayPosition_Type pGetOverlayPosition;
GetPalette_Type pGetPalette;
GetPixelFormat_Type pGetPixelFormat;
GetSurfaceDesc_Type pGetSurfaceDesc1;
GetSurfaceDesc2_Type pGetSurfaceDesc4;
GetSurfaceDesc2_Type pGetSurfaceDesc7;

//Initialize
IsLost_Type pIsLost;
Lock_Type pLock;
ReleaseDC_Type pReleaseDC;
Restore_Type pRestore;
SetClipper_Type pSetClipper;
SetColorKey_Type pSetColorKey;
SetOverlayPosition_Type pSetOverlayPosition;
SetPalette_Type pSetPalette;
Unlock1_Type pUnlock1;
Unlock4_Type pUnlock4;
UpdateOverlay_Type pUpdateOverlay;
UpdateOverlayDisplay_Type pUpdateOverlayDisplay;
UpdateOverlayZOrder_Type pUpdateOverlayZOrder;
SetSurfaceDesc_Type pSetSurfaceDesc;

/* DirectDrawClipper hook pointers */
QueryInterface_Type pQueryInterfaceC;
AddRefC_Type pAddRefC;
ReleaseC_Type pReleaseC;
GetClipList_Type pGetClipList;
GetHWnd_Type pGetHWnd;
InitializeC_Type pInitializeC;
IsClipListChanged_Type pIsClipListChanged;
SetClipList_Type pSetClipList;
SetHWnd_Type pSetHWnd;

/* DirectDrawPalette hook pointers */
QueryInterfaceP_Type pQueryInterfaceP;
AddRefP_Type pAddRefP;
ReleaseP_Type pReleaseP;
    /*** IDirectDrawPalette methods ***/
GetCapsP_Type pGetCapsP;
GetEntries_Type pGetEntries;
//    STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW, DWORD, LPPALETTEENTRY) PURE;
SetEntries_Type pSetEntries;

// GammaRamp
GammaRamp_Type pDDGetGammaRamp;
GammaRamp_Type pDDSetGammaRamp;

// ddraw global variables, constants & so on

#define MAXBACKBUFFERS 4

LPDIRECTDRAWSURFACE lpDDSEmu_Prim=NULL;
LPDIRECTDRAWSURFACE lpDDSEmu_Back=NULL;
LPDIRECTDRAWSURFACE lpDDSBack=NULL;
LPDIRECTDRAWSURFACE lpDDZBuffer=NULL;
LPDIRECTDRAWSURFACE lpDDTexture=NULL;
// v2.1.87: lpPrimaryDD is the DIRECTDRAW object to which the primary surface and all 
// the service objects (emulated backbuffer, emulater primary, ....) are attached.
LPDIRECTDRAW lpPrimaryDD=NULL;
LPDIRECTDRAW lpBackBufferDD=NULL;
int iBakBufferVersion;
LPDIRECTDRAWPALETTE lpDDP=NULL;

// v2.02.37: globals to store requested main surface capabilities 
DDSURFACEDESC2 DDSD_Prim;
DDSURFACEDESC2 DDSD_Back;
DDSURFACEDESC2 DDSD_ZBuffer;

DWORD DDZBufferCaps;
DWORD PaletteEntries[256];
DWORD *Palette16BPP = NULL;
void *EmuScreenBuffer = NULL; // to implement pitch bug fix
DWORD rPitch = 0;
LPVOID rSurface = NULL;
static void SetPixFmt(LPDDSURFACEDESC2);
static void GetPixFmt(LPDDSURFACEDESC2);

static HookEntry_Type ddHooks[]={
	{HOOK_HOT_CANDIDATE, "DirectDrawCreate", (FARPROC)NULL, (FARPROC *)&pDirectDrawCreate, (FARPROC)extDirectDrawCreate},
	{HOOK_HOT_CANDIDATE, "DirectDrawCreateEx", (FARPROC)NULL, (FARPROC *)&pDirectDrawCreateEx, (FARPROC)extDirectDrawCreateEx},
	{HOOK_HOT_CANDIDATE, "DirectDrawEnumerateA", (FARPROC)NULL, (FARPROC *)&pDirectDrawEnumerate, (FARPROC)extDirectDrawEnumerate},
	{HOOK_HOT_CANDIDATE, "DirectDrawEnumerateExA", (FARPROC)NULL, (FARPROC *)&pDirectDrawEnumerateEx, (FARPROC)extDirectDrawEnumerateEx},
	//{HOOK_IAT_CANDIDATE, "DirectDrawEnumerateW", (FARPROC)NULL, (FARPROC *)&pDirectDrawEnumerateW, (FARPROC)extDirectDrawCreate},
	//{HOOK_IAT_CANDIDATE, "DirectDrawEnumerateExW", (FARPROC)NULL, (FARPROC *)&pDirectDrawEnumerateExW, (FARPROC)extDirectDrawCreate},
	{HOOK_IAT_CANDIDATE, 0, NULL, 0, 0} // terminator
};

FARPROC Remap_ddraw_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;
	if (addr=RemapLibrary(proc, hModule, ddHooks)) return addr;
	return NULL;
}

extern HRESULT WINAPI sBlt(char *, LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDBLTFX, BOOL);

/* ------------------------------------------------------------------------------ */
// auxiliary (static) functions
/* ------------------------------------------------------------------------------ */

DWORD gdwRefreshRate;
#define MAXREFRESHDELAYCOUNT 20
int iRefreshDelays[MAXREFRESHDELAYCOUNT]={16, 17};
int iRefreshDelayCount=2;

void SetVSyncDelays(LPDIRECTDRAW lpdd)
{
	DDSURFACEDESC2 ddsdRefreshRate;
	HRESULT res;

	memset(&ddsdRefreshRate, 0, sizeof(ddsdRefreshRate));
	ddsdRefreshRate.dwSize = sizeof(DDSURFACEDESC);
	res=(*pGetDisplayMode)(lpdd, (LPDDSURFACEDESC)&ddsdRefreshRate);
	if(res==DDERR_GENERIC){ // handling Win8 missing support for old ddraw interface
		ddsdRefreshRate.dwSize = sizeof(DDSURFACEDESC2);
		res=(*pGetDisplayMode)(lpdd, (LPDDSURFACEDESC)&ddsdRefreshRate);
	}
	if(res) return;
	dxw.SetVSyncDelays(ddsdRefreshRate.dwRefreshRate);
}

static void Stopper(char *s, int line)
{
	char sMsg[81];
	sprintf(sMsg,"break: \"%s\"", s);
	MessageBox(0, sMsg, "break", MB_OK | MB_ICONEXCLAMATION);
}

//#define STOPPER_TEST // comment out to eliminate
#ifdef STOPPER_TEST
#define STOPPER(s) Stopper(s, __LINE__)
#else
#define STOPPER(s)
#endif

static char *sFourCC(DWORD fcc)
{
	static char sRet[5];
	char c;
	int i;
	char *t=&sRet[0];
	for(i=0; i<4; i++){
		c = fcc & (0xFF);
		*t++ = isprint(c) ? c : '.';
		c = c >> 8;
	}
	*t = 0;
	return sRet;
}

static char *DumpPixelFormat(LPDDSURFACEDESC2 lpddsd)
{
	static char sBuf[512];
	char sItem[256];
	DWORD flags=lpddsd->ddpfPixelFormat.dwFlags;
	sprintf(sBuf, " PixelFormat flags=%x(%s) BPP=%d", 
		flags, ExplainPixelFormatFlags(flags), lpddsd->ddpfPixelFormat.dwRGBBitCount);
	if (flags & DDPF_RGB) {
		if (flags & DDPF_ALPHAPIXELS) {
			sprintf(sItem, " RGBA=(%x,%x,%x,%x)", 
				lpddsd->ddpfPixelFormat.dwRBitMask,
				lpddsd->ddpfPixelFormat.dwGBitMask,
				lpddsd->ddpfPixelFormat.dwBBitMask,
				lpddsd->ddpfPixelFormat.dwRGBAlphaBitMask);
		}
		else {
			sprintf(sItem, " RGB=(%x,%x,%x)", 
				lpddsd->ddpfPixelFormat.dwRBitMask,
				lpddsd->ddpfPixelFormat.dwGBitMask,
				lpddsd->ddpfPixelFormat.dwBBitMask);
		}
		strcat(sBuf, sItem);
	}
	if (flags & DDPF_YUV) {
		sprintf(sItem, " YUVA=(%x,%x,%x,%x)", 
			lpddsd->ddpfPixelFormat.dwYBitMask,
			lpddsd->ddpfPixelFormat.dwUBitMask,
			lpddsd->ddpfPixelFormat.dwVBitMask,
			lpddsd->ddpfPixelFormat.dwYUVAlphaBitMask);
		strcat(sBuf, sItem);
	}
	if (flags & DDPF_ZBUFFER) {
		sprintf(sItem, " SdZSbL=(%x,%x,%x,%x)", 
			lpddsd->ddpfPixelFormat.dwStencilBitDepth,
			lpddsd->ddpfPixelFormat.dwZBitMask,
			lpddsd->ddpfPixelFormat.dwStencilBitMask,
			lpddsd->ddpfPixelFormat.dwLuminanceAlphaBitMask);
		strcat(sBuf, sItem);
	}
	if (flags & DDPF_ALPHA) {
		sprintf(sItem, " LBdBlZ=(%x,%x,%x,%x)", 
			lpddsd->ddpfPixelFormat.dwLuminanceBitMask,
			lpddsd->ddpfPixelFormat.dwBumpDvBitMask,
			lpddsd->ddpfPixelFormat.dwBumpLuminanceBitMask,
			lpddsd->ddpfPixelFormat.dwRGBZBitMask);
		strcat(sBuf, sItem);
	}
	if (flags & DDPF_LUMINANCE) {
		sprintf(sItem, " BMbMF=(%x,%x,%x,%x)", 
			lpddsd->ddpfPixelFormat.dwBumpDuBitMask,
			lpddsd->ddpfPixelFormat.MultiSampleCaps.wBltMSTypes,
			lpddsd->ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes,
			lpddsd->ddpfPixelFormat.dwYUVZBitMask);
		strcat(sBuf, sItem);
	}
	if (flags & DDPF_BUMPDUDV) {
		sprintf(sItem, " O=(%x)", 
			lpddsd->ddpfPixelFormat.dwOperations);
		strcat(sBuf, sItem);
	}
	if (flags & DDPF_FOURCC) {
		sprintf(sItem, " FourCC=%x(%s)", 
			lpddsd->ddpfPixelFormat.dwFourCC, sFourCC(lpddsd->ddpfPixelFormat.dwFourCC));
		strcat(sBuf, sItem);
	}
	return sBuf;
}

static CHAR *LogSurfaceAttributes(LPDDSURFACEDESC lpddsd, char *label, int line)
{
	static char sInfo[1024];
	sprintf(sInfo, "SurfaceDesc: %s Flags=%x(%s)",
		label, 
		lpddsd->dwFlags, ExplainFlags(lpddsd->dwFlags));
	if (lpddsd->dwFlags & DDSD_BACKBUFFERCOUNT) sprintf(sInfo, "%s BackBufferCount=%d", sInfo, lpddsd->dwBackBufferCount);
	if (lpddsd->dwFlags & DDSD_WIDTH) sprintf(sInfo, "%s Width=%d", sInfo, lpddsd->dwWidth);
	if (lpddsd->dwFlags & DDSD_HEIGHT) sprintf(sInfo, "%s Height=%d", sInfo, lpddsd->dwHeight);
	if (lpddsd->dwFlags & DDSD_PITCH) sprintf(sInfo, "%s Pitch=%d", sInfo, lpddsd->lPitch);
	if (lpddsd->dwFlags & DDSD_MIPMAPCOUNT) sprintf(sInfo, "%s MipMapCount=%d", sInfo, lpddsd->dwMipMapCount);
	if (lpddsd->dwFlags & DDSD_CAPS) {
		sprintf(sInfo, "%s Caps=%x(%s)", sInfo, lpddsd->ddsCaps.dwCaps, ExplainDDSCaps(lpddsd->ddsCaps.dwCaps));
		if(lpddsd->dwSize==sizeof(DDSURFACEDESC2)){
			LPDDSURFACEDESC2 lpddsd2=(LPDDSURFACEDESC2)lpddsd;
			sprintf(sInfo, "%s Caps2=%x(%s)", sInfo, lpddsd2->ddsCaps.dwCaps2, ExplainDDSCaps2(lpddsd2->ddsCaps.dwCaps2));
			sprintf(sInfo, "%s Caps3=%x(%s)", sInfo, lpddsd2->ddsCaps.dwCaps3, ExplainDDSCaps3(lpddsd2->ddsCaps.dwCaps3));
		}
	}
	if (lpddsd->dwFlags & DDSD_CKDESTBLT ) sprintf(sInfo, "%s CKDestBlt=(%x,%x)", sInfo, lpddsd->ddckCKDestBlt.dwColorSpaceLowValue, lpddsd->ddckCKDestBlt.dwColorSpaceHighValue);
	if (lpddsd->dwFlags & DDSD_CKDESTOVERLAY ) sprintf(sInfo, "%s CKDestOverlay=(%x,%x)", sInfo, lpddsd->ddckCKDestOverlay.dwColorSpaceLowValue, lpddsd->ddckCKDestOverlay.dwColorSpaceHighValue);
	if (lpddsd->dwFlags & DDSD_CKSRCBLT ) sprintf(sInfo, "%s CKSrcBlt=(%x,%x)", sInfo, lpddsd->ddckCKSrcBlt.dwColorSpaceLowValue, lpddsd->ddckCKSrcBlt.dwColorSpaceHighValue);
	if (lpddsd->dwFlags & DDSD_CKSRCOVERLAY ) sprintf(sInfo, "%s CKSrcOverlay=(%x,%x)", sInfo, lpddsd->ddckCKSrcOverlay.dwColorSpaceLowValue, lpddsd->ddckCKSrcOverlay.dwColorSpaceHighValue);
	if (lpddsd->dwFlags & DDSD_PIXELFORMAT ) sprintf(sInfo, "%s %s", sInfo, DumpPixelFormat((LPDDSURFACEDESC2)lpddsd));
	if (lpddsd->dwFlags & DDSD_LPSURFACE) sprintf(sInfo, "%s Surface=%x", sInfo, lpddsd->lpSurface);
	if (lpddsd->dwFlags & DDSD_ZBUFFERBITDEPTH) sprintf(sInfo, "%s ZBufferBitDepth=%d", sInfo, lpddsd->dwZBufferBitDepth);
	if (lpddsd->dwFlags & DDSD_ALPHABITDEPTH) sprintf(sInfo, "%s AlphaBitDepth=%d", sInfo, lpddsd->dwAlphaBitDepth);
	if (lpddsd->dwFlags & DDSD_REFRESHRATE) sprintf(sInfo, "%s RefreshRate=%d", sInfo, lpddsd->dwRefreshRate);
	if (lpddsd->dwFlags & DDSD_LINEARSIZE) sprintf(sInfo, "%s LinearSize=%d", sInfo, lpddsd->dwLinearSize);
	if (lpddsd->dwSize == sizeof(DDSURFACEDESC2)){
		if (lpddsd->dwFlags & DDSD_TEXTURESTAGE) sprintf(sInfo, "%s TextureStage=%x", sInfo, ((LPDDSURFACEDESC2)lpddsd)->dwTextureStage);
		if (lpddsd->dwFlags & DDSD_FVF) sprintf(sInfo, "%s FVF=%x", sInfo, ((LPDDSURFACEDESC2)lpddsd)->dwFVF);
	}

	return sInfo;
}

static void DumpPixFmt(LPDDSURFACEDESC2 lpdds)
{
	OutTrace("PixelFormat: lpddsd=%x %s\n", DumpPixelFormat(lpdds));
}

void DescribeSurface(LPDIRECTDRAWSURFACE lpdds, int dxversion, char *label, int line)
{
	DDSURFACEDESC2 ddsd;
	HRESULT res;
	int dwSize = (dxversion<4)?sizeof(DDSURFACEDESC):sizeof(DDSURFACEDESC2);
	memset(&ddsd, 0, dwSize);
	ddsd.dwSize = dwSize;
	switch (dxversion){
	case 0:
	case 1:
	case 2:
	case 3:
		if (pGetSurfaceDesc1) 
			res=(*pGetSurfaceDesc1)(lpdds, (LPDDSURFACEDESC)&ddsd);
		else
			res=lpdds->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd);
		break;
	case 4:
	case 5:
	case 6:
		if (pGetSurfaceDesc4) 
			res=(*pGetSurfaceDesc4)((LPDIRECTDRAWSURFACE2)lpdds, &ddsd);
		else
			res=lpdds->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd);
		break;
	case 7:
		if (pGetSurfaceDesc7) 
			res=(*pGetSurfaceDesc7)((LPDIRECTDRAWSURFACE2)lpdds, &ddsd);
		else
			res=lpdds->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd);
		break;
	}
	if(res)return;
	OutTraceDW("Surface %s: ddsd=%x dxversion=%d %s\n", 
		label, lpdds, dxversion, LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, label, line));
}

/* ------------------------------------------------------------------------------ */
// auxiliary (static) functions for palette handling
/* ------------------------------------------------------------------------------ */

BOOL isPaletteUpdated;

void mySetPalette(int dwstart, int dwcount, LPPALETTEENTRY lpentries)
{
	int i;
	extern DXWNDSTATUS *pStatus;

	for(int idx=0; idx<dwcount; idx++)  
		pStatus->Palette[dwstart+idx]= lpentries[idx];

	for(i = 0; i < dwcount; i ++){
		PALETTEENTRY PalColor;
		PalColor = lpentries[i];
		if (dxw.dwFlags3 & BLACKWHITE){
			// (http://www.codeproject.com/Articles/66253/Converting-Colors-to-Gray-Shades):
			// gray = (red * 0.30) + (green * 0.59) + (blue * 0.11) 
			DWORD grayscale;
			//grayscale = ((DWORD)lpentries[i].peRed + (DWORD)lpentries[i].peGreen + (DWORD)lpentries[i].peBlue) / 3;
			grayscale = (((DWORD)PalColor.peRed * 30) + ((DWORD)PalColor.peGreen * 59) + ((DWORD)PalColor.peBlue) * 11) / 100;
			PalColor.peRed = PalColor.peGreen = PalColor.peBlue = (BYTE)grayscale;
		}
		switch (dxw.ActualPixelFormat.dwRGBBitCount){
		case 32:
			PaletteEntries[i + dwstart] =
				(((DWORD)PalColor.peRed) << 16) + (((DWORD)PalColor.peGreen) << 8) + ((DWORD)PalColor.peBlue);
			break;
		case 16:
			PaletteEntries[i + dwstart] = (dxw.ActualPixelFormat.dwGBitMask == 0x03E0) ?
				(((DWORD)PalColor.peRed & 0xF8) << 8) + (((DWORD)PalColor.peGreen & 0xFC) << 3) + (((DWORD)PalColor.peBlue &0xF8) >> 3)
				:
				(((DWORD)PalColor.peRed & 0xF8) << 8) + (((DWORD)PalColor.peGreen & 0xF8) << 3) + (((DWORD)PalColor.peBlue &0xF8) >> 3);
			break;
		default:
			OutTraceDW("ASSERT: unsupported Color BPP=%d\n", dxw.ActualPixelFormat.dwRGBBitCount);
			break;
		}
	}

	isPaletteUpdated = TRUE;
}

void InitDDScreenParameters(LPDIRECTDRAW lpdd)
{
	HRESULT res;
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize=sizeof(DDSURFACEDESC);
	res=(*pGetDisplayMode)(lpdd, (LPDDSURFACEDESC)&ddsd);
	if(res==DDERR_GENERIC){ // Win8 missing support for old ddraw interfaces
		ddsd.dwSize=sizeof(DDSURFACEDESC2);
		res=(*pGetDisplayMode)(lpdd, (LPDDSURFACEDESC)&ddsd);
	}
	if(res){
		OutTraceE("GetDisplayMode: ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return;
	}

	OutTraceDW("InitDDScreenParameters: Actual %s\n", DumpPixelFormat((LPDDSURFACEDESC2)&ddsd));
	dxw.ActualPixelFormat=ddsd.ddpfPixelFormat;
	if(dxw.VirtualPixelFormat.dwRGBBitCount==0) dxw.VirtualPixelFormat=ddsd.ddpfPixelFormat;
	SetBltTransformations();
	return;
}

void InitDSScreenParameters(LPDIRECTDRAWSURFACE lpdds)
{
	HRESULT res;
	DDPIXELFORMAT p;
	DDSURFACEDESC2 ddsd;
	p.dwSize=sizeof(DDPIXELFORMAT);
	if(res=(*pGetPixelFormat)(lpdds, &p)){
		OutTraceE("GetPixelFormat: ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return;
	}

	ddsd.ddpfPixelFormat = p;
	OutTraceDW("InitDSScreenParameters: Actual %s\n", DumpPixelFormat(&ddsd));
	dxw.ActualPixelFormat = p;
	SetBltTransformations();
	return;
}

void InitScreenParameters()
{
	DEVMODE CurrDevMode;
	static int DoOnce = FALSE;

	if(DoOnce) return;
	DoOnce = TRUE;

	// set default VGA mode 800x600
	// should I make it configurable ? (640x480, 800x600, 1024x768)
	dxw.SetScreenSize(); // 800 x 600 by default
	GetHookInfo()->Height=(short)dxw.GetScreenHeight();
	GetHookInfo()->Width=(short)dxw.GetScreenWidth();
	GetHookInfo()->ColorDepth=0; // unknown
	GetHookInfo()->DXVersion=0; // unknown
	GetHookInfo()->isLogging=(dxw.dwTFlags & OUTTRACE);

	if(!(*pEnumDisplaySettings)(NULL, ENUM_CURRENT_SETTINGS, &CurrDevMode)){
		OutTraceE("EnumDisplaySettings: ERROR err=%d at %d\n", GetLastError(), __LINE__);
		return;
	}
	memset(&dxw.ActualPixelFormat, 0, sizeof(DDPIXELFORMAT));
	// initialize to default null values, but dwRGBBitCount
	dxw.ActualPixelFormat.dwRGBBitCount=CurrDevMode.dmBitsPerPel;
	dxw.VirtualPixelFormat.dwRGBBitCount=CurrDevMode.dmBitsPerPel; // until set differently
	OutTraceDW("InitScreenParameters: RGBBitCount=%d\n", CurrDevMode.dmBitsPerPel);
	SetBltTransformations();

	return;
}

void FixPixelFormat(int ColorDepth, DDPIXELFORMAT *pf)
{
	pf->dwFlags = DDPF_RGB;
	switch(ColorDepth){
	case 8:
		pf->dwFlags |= DDPF_PALETTEINDEXED8;		
		pf->dwRGBBitCount = 8;
		pf->dwRBitMask = 0;
		pf->dwGBitMask = 0;
		pf->dwBBitMask = 0;
		pf->dwRGBAlphaBitMask = 0x0000;
		break;
	case 15: // v2.02.53: Hesperian Wars - so far the only game setting color depth to 15 BPP!
	case 16:
		pf->dwRGBBitCount = 16;
		if (dxw.dwFlags1 & USERGB565){
			pf->dwRBitMask = 0xf800; 
			pf->dwGBitMask = 0x07e0;
			pf->dwBBitMask = 0x001f;
			pf->dwRGBAlphaBitMask = 0x0000;
		}
		else {
			if(!(dxw.dwFlags4 & NOALPHACHANNEL) && (ColorDepth == 16)) pf->dwFlags |= DDPF_ALPHAPIXELS; // v2.02.33,40,53	
			pf->dwRBitMask = 0x7c00;
			pf->dwGBitMask = 0x03e0;
			pf->dwBBitMask = 0x001f;
			pf->dwRGBAlphaBitMask = 0x8000;
		}
		break;
	case 24:
		pf->dwRGBBitCount = 24;
		pf->dwRBitMask = 0x00FF0000;
		pf->dwGBitMask = 0x0000FF00;
		pf->dwBBitMask = 0x000000FF;
		pf->dwRGBAlphaBitMask = 0x00000000;
		break;
	case 32:
		if(!(dxw.dwFlags4 & NOALPHACHANNEL)) pf->dwFlags |= DDPF_ALPHAPIXELS; // v2.02.33
		pf->dwRGBBitCount = 32;
		pf->dwRBitMask = 0x00FF0000;
		pf->dwGBitMask = 0x0000FF00;
		pf->dwBBitMask = 0x000000FF;
		pf->dwRGBAlphaBitMask = 0xFF000000;
		break;
	}
}

static void ddSetCompatibility()
{
	typedef HRESULT (WINAPI *SetAppCompatData_Type)(DWORD, DWORD);
	SetAppCompatData_Type pSetAppCompatData;
	HRESULT res;
	HINSTANCE hinst;

	hinst=LoadLibrary("ddraw.dll");
	pSetAppCompatData=(SetAppCompatData_Type)(*pGetProcAddress)(hinst, "SetAppCompatData");
	if(pSetAppCompatData) {
		res=(*pSetAppCompatData)(2, 0);
		OutTraceDW("HookDirectDraw: SetAppCompatData(2,0) ret=%x(%s)\n", res, ExplainDDError(res));
	}
	else
		OutTraceDW("HookDirectDraw: missing SetAppCompatData call\n");
	FreeLibrary(hinst);
}

int HookDirectDraw(HMODULE module, int version)
{
	if(dxw.dwFlags2 & SETCOMPATIBILITY) {
		static BOOL AlreadyDone = FALSE;
		if(!AlreadyDone){
			ddSetCompatibility();
			AlreadyDone = TRUE;
		}
	}

	if(dxw.dwFlags4 & HOTPATCH) {
		// hot-patch all APIs and that's all folks!
		HookLibrary(module, ddHooks, "ddraw.dll");
		return TRUE;
	}

	//const GUID dd7 = {0x15e65ec0,0x3b9c,0x11d2,0xb9,0x2f,0x00,0x60,0x97,0x97,0xea,0x5b};
	HMODULE hinst;

	OutTraceB("HookDirectDraw version=%d\n", version); //GHO
	switch(version){
	case 0: // automatic
		HookLibrary(module, ddHooks, "ddraw.dll");
		break;
	case 1:
	case 2:
	case 3:
	case 5:
	case 6:
		HookLibrary(module, ddHooks, "ddraw.dll");
		if(!pDirectDrawCreate){ // required for IAT patching 
			hinst = LoadLibrary("ddraw.dll");
			pDirectDrawCreate = (DirectDrawCreate_Type)GetProcAddress(hinst, "DirectDrawCreate");
			pDirectDrawEnumerate = (DirectDrawEnumerate_Type)GetProcAddress(hinst, "DirectDrawEnumerateA");		
		}
		if(pDirectDrawCreate){
			LPDIRECTDRAW lpdd;
			BOOL res;
			HookLibrary(module, ddHooks, "ddraw.dll");
			res=extDirectDrawCreate(0, &lpdd, 0);
			if (res){
				OutTraceE("DirectDrawCreate: ERROR res=%x(%s)\n", res, ExplainDDError(res));
			}
			lpdd->Release();
		}
		break;
	case 7:
		//hinst = LoadLibrary("ddraw.dll");
		HookLibrary(module, ddHooks, "ddraw.dll");
		if(!pDirectDrawCreate){ // required for IAT patching in "Crimson skies"
			hinst = LoadLibrary("ddraw.dll");
			pDirectDrawEnumerate = (DirectDrawEnumerate_Type)GetProcAddress(hinst, "DirectDrawEnumerateA");
			pDirectDrawEnumerateEx = (DirectDrawEnumerateEx_Type)GetProcAddress(hinst, "DirectDrawEnumerateExA");
			pDirectDrawCreate = (DirectDrawCreate_Type)GetProcAddress(hinst, "DirectDrawCreate");
			pDirectDrawCreateEx = (DirectDrawCreateEx_Type)GetProcAddress(hinst, "DirectDrawCreateEx");
		}		
		if(pDirectDrawCreate){
			LPDIRECTDRAW lpdd;
			BOOL res;
			res=extDirectDrawCreate(0, &lpdd, 0);
			if (res) OutTraceE("DirectDrawCreate: ERROR res=%x(%s)\n", res, ExplainDDError(res));
			lpdd->Release();
		}
		//if(pDirectDrawCreateEx){
		//	LPDIRECTDRAW lpdd;
		//	BOOL res;
		//	HookLibrary(module, ddHooks, "ddraw.dll");
		//	res=extDirectDrawCreateEx(0, &lpdd, dd7, 0);
		//	if (res) OutTraceE("DirectDrawCreateEx: ERROR res=%x(%s)\n", res, ExplainDDError(res));
		//	lpdd->Release();
		//}
		break;
	}

	if(pDirectDrawCreate || pDirectDrawCreateEx) return 1;
	return 0;
}

Unlock4_Type pUnlockMethod(LPDIRECTDRAWSURFACE lpdds)
{
	// to do: return extUnlock for unhooked surfaces 

	char sMsg[81];
	void * extUnlock = NULL;
	if(lpdds){
		__try{ // v2.02.31: catch some possible exception (i.e. Abomination in EMULATION mode)
			extUnlock=(void *)*(DWORD *)(*(DWORD *)lpdds + 128);
		}
		__except (EXCEPTION_EXECUTE_HANDLER){
			OutTraceE("Exception at %d\n",__LINE__);
			extUnlock=NULL;
		};
		if(extUnlock==(void *)extUnlock1) return (Unlock4_Type)pUnlock1;
		if(extUnlock==(void *)extUnlock4) return (Unlock4_Type)pUnlock4;
		if(extUnlock==(void *)extUnlockDir1) return (Unlock4_Type)pUnlock1;
		if(extUnlock==(void *)extUnlockDir4) return (Unlock4_Type)pUnlock4;
	}
	sprintf_s(sMsg, 80, "pUnlockMethod: pUnlock(%x) can't match %x\n", lpdds, extUnlock);
	OutTraceDW(sMsg);
	if (IsAssertEnabled) MessageBox(0, sMsg, "pUnlockMethod", MB_OK | MB_ICONEXCLAMATION);
	if (pUnlock4) return pUnlock4;
	return (Unlock4_Type)pUnlock1;
}

#if 0
Lock_Type pLockMethod(LPDIRECTDRAWSURFACE lpdds)
{
	char sMsg[81];
	void * extLock = NULL;
	if(lpdds){
		__try{ // v2.02.31: catch some possible exception (i.e. Abomination in EMULATION mode)
			extLock=(void *)*(DWORD *)(*(DWORD *)lpdds + 128);
		}
		__except (EXCEPTION_EXECUTE_HANDLER){
			OutTraceE("Exception at %d\n",__LINE__);
			extLock=NULL;
		};
		if(extUnlock==(void *)extUnlock1) return pLock1;
		if(extUnlock==(void *)extUnlock4) return pLock4;
		if(extUnlock==(void *)extUnlockDir1) return pLock1;
		if(extUnlock==(void *)extUnlockDir4) return pLock4;
	}
	sprintf_s(sMsg, 80, "pLockMethod: pUnlock(%x) can't match %x\n", lpdds, extLock);
	OutTraceDW(sMsg);
	if (IsAssertEnabled) MessageBox(0, sMsg, "pLockMethod", MB_OK | MB_ICONEXCLAMATION);
	if (pLock4) return pLock4;
	return pLock1;
}
#endif

CreateSurface2_Type pCreateSurfaceMethod(LPDIRECTDRAWSURFACE lpdds)
{
	char sMsg[81];
	void * extUnlock = NULL;
	if(lpdds){
		__try{ // v2.02.31: catch some possible exception (i.e. Abomination in EMULATION mode)
			extUnlock=(void *)*(DWORD *)(*(DWORD *)lpdds + 128);	}
		__except (EXCEPTION_EXECUTE_HANDLER){
			OutTraceE("Exception at %d\n",__LINE__);
			extUnlock = NULL;
		};
		if(extUnlock==(void *)extUnlock1) return (CreateSurface2_Type)pCreateSurface1;
		if(extUnlock==(void *)extUnlock4) return pCreateSurface7 ? (CreateSurface2_Type)pCreateSurface7 : (CreateSurface2_Type)pCreateSurface4;
		if(extUnlock==(void *)extUnlockDir1) return (CreateSurface2_Type)pCreateSurface1;
		if(extUnlock==(void *)extUnlockDir4) return pCreateSurface7 ? (CreateSurface2_Type)pCreateSurface7 : (CreateSurface2_Type)pCreateSurface4;
	}
	sprintf_s(sMsg, 80, "pCreateSurfaceMethod: pUnlock(%x) can't match %x\n", lpdds, extUnlock);
	OutTraceDW(sMsg);
	if (IsAssertEnabled) MessageBox(0, sMsg, "pCreateSurfaceMethod", MB_OK | MB_ICONEXCLAMATION);
	if (pCreateSurface7) return pCreateSurface7;
	if (pCreateSurface4) return pCreateSurface4;
	return (CreateSurface2_Type)pCreateSurface1;
}

GetSurfaceDesc2_Type pGetSurfaceDescMethod(LPDIRECTDRAWSURFACE lpdds)
{
	char sMsg[81];
	void * extUnlock;
	extUnlock=(void *)*(DWORD *)(*(DWORD *)lpdds + 128);
	if(extUnlock==(void *)extUnlock1) return (GetSurfaceDesc2_Type)pGetSurfaceDesc1;
	if(extUnlock==(void *)extUnlock4) return pGetSurfaceDesc7 ? pGetSurfaceDesc7 : pGetSurfaceDesc4;
	if(extUnlock==(void *)extUnlockDir1) return (GetSurfaceDesc2_Type)pGetSurfaceDesc1;
	if(extUnlock==(void *)extUnlockDir4) return pGetSurfaceDesc7 ? pGetSurfaceDesc7 : pGetSurfaceDesc4;
	sprintf_s(sMsg, 80, "pGetSurfaceDescMethod: pUnlock(%x) can't match %x\n", lpdds, extUnlock);
	OutTraceDW(sMsg);
	if (IsAssertEnabled) MessageBox(0, sMsg, "pGetSurfaceDescMethod", MB_OK | MB_ICONEXCLAMATION);
	if (pGetSurfaceDesc4) return pGetSurfaceDesc4;
	return (GetSurfaceDesc2_Type)pGetSurfaceDesc1;
}

int SurfaceDescrSize(LPDIRECTDRAWSURFACE lpdds)
{
	char sMsg[81];
	void * extUnlock;
	extUnlock=(void *)*(DWORD *)(*(DWORD *)lpdds + 128);
	if(extUnlock==(void *)extUnlock1) return sizeof(DDSURFACEDESC);
	if(extUnlock==(void *)extUnlock4) return sizeof(DDSURFACEDESC2);
	if(extUnlock==(void *)extUnlockDir1) return sizeof(DDSURFACEDESC);
	if(extUnlock==(void *)extUnlockDir4) return sizeof(DDSURFACEDESC2);
	sprintf_s(sMsg, 80, "pCreateSurfaceMethod: pUnlock(%x) can't match %x\n", lpdds, extUnlock);
	OutTraceDW(sMsg);
	if (IsAssertEnabled) MessageBox(0, sMsg, "SurfaceDescrSize", MB_OK | MB_ICONEXCLAMATION);
	return sizeof(DDSURFACEDESC);
}

int lpddsHookedVersion(LPDIRECTDRAWSURFACE lpdds)
{
	char sMsg[81];
	void * extGetCaps = NULL;

	if(lpdds){
		__try{
		extGetCaps=(void *)*(DWORD *)(*(DWORD *)lpdds + 56);
		}
		__except (EXCEPTION_EXECUTE_HANDLER){
		extGetCaps=NULL;
		};	
		if(extGetCaps==(void *)extGetCaps1S) return 1;
		if(extGetCaps==(void *)extGetCaps2S) return 2;
		if(extGetCaps==(void *)extGetCaps3S) return 3;
		if(extGetCaps==(void *)extGetCaps4S) return 4;
		if(extGetCaps==(void *)extGetCaps7S) return 7;
	}
	sprintf_s(sMsg, 80, "lpddsHookedVersion(%x) can't match %x\n", lpdds, extGetCaps);
	OutTraceDW(sMsg);
	if (IsAssertEnabled) MessageBox(0, sMsg, "lpddsHookedVersion", MB_OK | MB_ICONEXCLAMATION);
	return 0;
}

int lpddHookedVersion(LPDIRECTDRAW lpdd)
{
	char sMsg[81];
	void *extCreateSurface = NULL;

	if(lpPrimaryDD){
		__try{
		extCreateSurface=(void *)*(DWORD *)(*(DWORD *)lpdd + 24);
		}
		__except (EXCEPTION_EXECUTE_HANDLER){
		extCreateSurface=NULL;
		};	
		if(extCreateSurface==(void *)extCreateSurface7) return 7;
		if(extCreateSurface==(void *)extCreateSurface4) return 4;
		if(extCreateSurface==(void *)extCreateSurface2) return 2;
		if(extCreateSurface==(void *)extCreateSurface1) return 1;
	}
	sprintf_s(sMsg, 80, "lpddHookedVersion(%x) can't match %x\n", lpdd, extCreateSurface);
	OutTraceDW(sMsg);
	if (IsAssertEnabled) MessageBox(0, sMsg, "lpddHookedVersion", MB_OK | MB_ICONEXCLAMATION);
	return 0;
}

void RegisterPixelFormat(LPDIRECTDRAWSURFACE lpdds)
{
	DDSURFACEDESC2 ddsdpix;
	memset((void *)&ddsdpix, 0, sizeof(DDSURFACEDESC2));
	ddsdpix.dwSize = SurfaceDescrSize(lpdds);
	ddsdpix.dwFlags = DDSD_PIXELFORMAT;
	(*pGetSurfaceDescMethod(lpdds))((LPDIRECTDRAWSURFACE2)lpdds, &ddsdpix);
	GetHookInfo()->pfd=ddsdpix.ddpfPixelFormat; // v2.02.88
	OutTraceB("RegisterPixelFormat: lpdds=%x %s\n", lpdds, DumpPixelFormat(&ddsdpix));
}

/* ------------------------------------------------------------------ */

// SetPixFmt: builds a pixel format descriptor when no one is specified, starting from the color depth, the current
// desktop pixel format (when the color depth is the same) or the config flags

static void SetPixFmt(LPDDSURFACEDESC2 lpdd)
{
	OutTraceDW("SetPixFmt: BPP=%d Use565=%d NoAlpha=%d\n", 
		dxw.VirtualPixelFormat.dwRGBBitCount, 
		dxw.dwFlags1 & USERGB565 ? 1:0,
		dxw.dwFlags4 & NOALPHACHANNEL ? 1:0);

	memset(&lpdd->ddpfPixelFormat,0,sizeof(DDPIXELFORMAT));
	lpdd->ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

	switch (dxw.VirtualPixelFormat.dwRGBBitCount)
	{
	case 8:
	case 15:
	case 16:
	case 24:
	case 32:
		FixPixelFormat(dxw.VirtualPixelFormat.dwRGBBitCount, &lpdd->ddpfPixelFormat);
		break;
	default:
		OutTraceE("CreateSurface ERROR: Unsupported resolution ColorBPP=%d\n", dxw.VirtualPixelFormat.dwRGBBitCount);
		break;
	}


	// remember current virtual settings
	dxw.VirtualPixelFormat=lpdd->ddpfPixelFormat;
	OutTraceDW("SetPixFmt: %s\n", DumpPixelFormat(lpdd));
}

// retrieves the stored pixel format

static void GetPixFmt(LPDDSURFACEDESC2 lpdd)
{
	lpdd->ddpfPixelFormat = dxw.VirtualPixelFormat;
	OutTraceDW("GetPixFmt: %s\n", DumpPixelFormat(lpdd));
}

/* ------------------------------------------------------------------ */
// hook query functions that determines the object versioning ....
/* ------------------------------------------------------------------ */

int Set_dwSize_From_Surface(LPDIRECTDRAWSURFACE lpdds)
{
	int dxversion;
	if (lpdds==NULL || 
		lpdds==lpDDSEmu_Prim || 
		lpdds==lpDDSEmu_Back
		)
		dxversion=lpddHookedVersion(lpPrimaryDD); // v2.01.87-v2.02.31 fix
	else
		dxversion=lpddsHookedVersion(lpdds);

	return (dxversion < 4) ? sizeof(DDSURFACEDESC) : sizeof(DDSURFACEDESC2);
}

int Set_dwSize_From_DDraw(LPDIRECTDRAW lpdd)
{
	return (lpddHookedVersion(lpdd) < 4) ? sizeof(DDSURFACEDESC) : sizeof(DDSURFACEDESC2);
}

void HookDDSession(LPDIRECTDRAW *lplpdd, int dxversion)
{
	OutTraceDW("Hooking directdraw session dd=%x dxversion=%d thread_id=%x\n", 
		*lplpdd, dxversion, GetCurrentThreadId());

	// IDIrectDraw::QueryInterface
	SetHook((void *)(**(DWORD **)lplpdd), extQueryInterfaceD, (void **)&pQueryInterfaceD, "QueryInterface(D)");
	// IDIrectDraw::Release
	SetHook((void *)(**(DWORD **)lplpdd + 8), extReleaseD, (void **)&pReleaseD, "Release(D)");
	// IDIrectDraw::CreateClipper
	SetHook((void *)(**(DWORD **)lplpdd + 16), extCreateClipper, (void **)&pCreateClipper, "CreateClipper(D)");
	// IDIrectDraw::CreatePalette
	SetHook((void *)(**(DWORD **)lplpdd + 20), extCreatePalette, (void **)&pCreatePalette, "CreatePalette(D)");
	// IDIrectDraw::CreateSurface
	switch(dxversion) {
	case 1:
		SetHook((void *)(**(DWORD **)lplpdd + 24), extCreateSurface1, (void **)&pCreateSurface1, "CreateSurface(S1)");
		break;
	case 2:
		SetHook((void *)(**(DWORD **)lplpdd + 24), extCreateSurface2, (void **)&pCreateSurface2, "CreateSurface(S2)");
		break;
	case 4:
		SetHook((void *)(**(DWORD **)lplpdd + 24), extCreateSurface4, (void **)&pCreateSurface4, "CreateSurface(S4)");
		break;
	case 7:
		SetHook((void *)(**(DWORD **)lplpdd + 24), extCreateSurface7, (void **)&pCreateSurface7, "CreateSurface(S7)");
		break;
	}
	// IDIrectDraw::DuplicateSurface
	SetHook((void *)(**(DWORD **)lplpdd + 28), extDuplicateSurface, (void **)&pDuplicateSurface, "DuplicateSurface(D)");
	// IDIrectDraw::EnumDisplayModes
	switch(dxversion) {
	case 1:
	case 2:
		SetHook((void *)(**(DWORD **)lplpdd + 32), extEnumDisplayModes1, (void **)&pEnumDisplayModes1, "EnumDisplayModes(D1)");
		break;
	case 4:
	case 7:
		SetHook((void *)(**(DWORD **)lplpdd + 32), extEnumDisplayModes4, (void **)&pEnumDisplayModes4, "EnumDisplayModes(D4)");
		break;
	}
	// IDIrectDraw::FlipToGDISurface
	SetHook((void *)(**(DWORD **)lplpdd + 40), extFlipToGDISurface, (void **)&pFlipToGDISurface, "FlipToGDISurface(D)");
	// IDIrectDraw::GetCaps
	SetHook((void *)(**(DWORD **)lplpdd + 44), extGetCapsD, (void **)&pGetCapsD, "GetCaps(D)");
	// IDIrectDraw::GetDisplayMode
	SetHook((void *)(**(DWORD **)lplpdd + 48), extGetDisplayMode, (void **)&pGetDisplayMode, "GetDisplayMode(D)");
	// IDIrectDraw::GetGDISurface
	SetHook((void *)(**(DWORD **)lplpdd + 56), extGetGDISurface, (void **)&pGetGDISurface, "GetGDISurface(D)");
	// IDIrectDraw::Initialize
	SetHook((void *)(**(DWORD **)lplpdd + 72), extInitialize, (void **)&pInitialize, "Initialize(D)");
	// IDIrectDraw::SetCooperativeLevel
	SetHook((void *)(**(DWORD **)lplpdd + 80), extSetCooperativeLevel, (void **)&pSetCooperativeLevel, "SetCooperativeLevel(D)");
	// IDIrectDraw::SetDisplayMode
	if (dxversion > 1)
		SetHook((void *)(**(DWORD **)lplpdd + 84), extSetDisplayMode2, (void **)&pSetDisplayMode2, "SetDisplayMode(D2)");
	else 
		SetHook((void *)(**(DWORD **)lplpdd + 84), extSetDisplayMode1, (void **)&pSetDisplayMode1, "SetDisplayMode(D1)");
	// IDIrectDraw::WaitForVerticalBlank
	SetHook((void *)(**(DWORD **)lplpdd + 88), extWaitForVerticalBlank, (void **)&pWaitForVerticalBlank, "WaitForVerticalBlank(D)");
	// IDIrectDraw::GetAvailableVidMem
	if (dxversion == 2)
		SetHook((void *)(**(DWORD **)lplpdd + 92), extGetAvailableVidMem2, (void **)&pGetAvailableVidMem2, "GetAvailableVidMem(D2)");
	if (dxversion >= 4){
		// IDIrectDraw::GetAvailableVidMem
			SetHook((void *)(**(DWORD **)lplpdd + 92), extGetAvailableVidMem4, (void **)&pGetAvailableVidMem4, "GetAvailableVidMem(D4)");
		// IDIrectDraw::TestCooperativeLevel
			SetHook((void *)(**(DWORD **)lplpdd + 104), extTestCooperativeLevel, (void **)&pTestCooperativeLevel, "TestCooperativeLevel(D)");
	}

	if (!(dxw.dwTFlags & OUTPROXYTRACE)) return;
	// Just proxed ...

	// IDIrectDraw::AddRef
	SetHook((void *)(**(DWORD **)lplpdd + 4), extAddRefDProxy, (void **)&pAddRefD, "AddRef(D)");
	// IDIrectDraw::Compact
	SetHook((void *)(**(DWORD **)lplpdd + 12), extCompactProxy, (void **)&pCompact, "Compact(D)");
	// IDIrectDraw::EnumSurfaces
	if (dxversion < 4)
		SetHook((void *)(**(DWORD **)lplpdd + 36), extEnumSurfacesProxy1, (void **)&pEnumSurfaces1, "EnumSurfaces(D1)");
	else
		SetHook((void *)(**(DWORD **)lplpdd + 36), extEnumSurfacesProxy4, (void **)&pEnumSurfaces4, "EnumSurfaces(D4)");
	// IDIrectDraw::GetFourCCCodes
	SetHook((void *)(**(DWORD **)lplpdd + 52), extGetFourCCCodesProxy, (void **)&pGetFourCCCodes, "GetFourCCCodes(D)");
	// IDIrectDraw::GetMonitorFrequency
	SetHook((void *)(**(DWORD **)lplpdd + 60), extGetMonitorFrequencyProxy, (void **)&pGetMonitorFrequency, "GetMonitorFrequency(D)");
	// IDIrectDraw::GetScanLine
	SetHook((void *)(**(DWORD **)lplpdd + 64), extGetScanLineProxy, (void **)&pGetScanLine, "GetScanLine(D)");
	// IDIrectDraw::GetVerticalBlankStatus
	SetHook((void *)(**(DWORD **)lplpdd + 68), extGetVerticalBlankStatusProxy, (void **)&pGetVerticalBlankStatus, "GetVerticalBlankStatus(D)");
	// IDIrectDraw::RestoreDisplayMode
	SetHook((void *)(**(DWORD **)lplpdd + 76), extRestoreDisplayModeProxy, (void **)&pRestoreDisplayMode, "RestoreDisplayMode(D)");
	if (dxversion >= 4){
		// IDIrectDraw::GetSurfaceFromDC
		SetHook((void *)(**(DWORD **)lplpdd + 96), extGetSurfaceFromDCProxy, (void **)&pGetSurfaceFromDC, "GetSurfaceFromDC(D)");
		// IDIrectDraw::RestoreAllSurfaces
		SetHook((void *)(**(DWORD **)lplpdd + 100), extRestoreAllSurfacesProxy, (void **)&pRestoreAllSurfaces, "RestoreAllSurfaces(D)");
		// IDIrectDraw::GetDeviceIdentifier
		SetHook((void *)(**(DWORD **)lplpdd + 108), extGetDeviceIdentifierProxy, (void **)&pGetDeviceIdentifier, "GetDeviceIdentifier(D)");
	}
}

static void HookDDClipper(LPDIRECTDRAWCLIPPER FAR* lplpDDClipper)
{
	OutTraceDW("Hooking directdraw clipper dd=%x\n", *lplpDDClipper);

	// IDirectDrawClipper::Release
	SetHook((void *)(**(DWORD **)lplpDDClipper + 8), extReleaseC, (void **)&pReleaseC, "Release(C)");

	if (!(dxw.dwTFlags & OUTPROXYTRACE)) return;
	// Just proxed ...

	// IDirectDrawClipper::QueryInterface
	SetHook((void *)(**(DWORD **)lplpDDClipper), extQueryInterfaceCProxy, (void **)&pQueryInterfaceC, "QueryInterface(C)");
	// IDirectDrawClipper::AddRef
	SetHook((void *)(**(DWORD **)lplpDDClipper + 4), extAddRefCProxy, (void **)&pAddRefC, "AddRef(C)");
	// IDirectDrawClipper::GetClipList
	SetHook((void *)(**(DWORD **)lplpDDClipper + 12), extGetClipListProxy, (void **)&pGetClipList, "GetClipList(C)");
	// IDirectDrawClipper::GetHWnd
	SetHook((void *)(**(DWORD **)lplpDDClipper + 16), extGetHWndProxy, (void **)&pGetHWnd, "GetHWnd(C)");
	// IDirectDrawClipper::Initialize
	SetHook((void *)(**(DWORD **)lplpDDClipper + 20), extInitializeCProxy, (void **)&pInitializeC, "Initialize(C)");
	// IDirectDrawClipper::IsClipListChanged
	SetHook((void *)(**(DWORD **)lplpDDClipper + 24), extIsClipListChangedProxy, (void **)&pIsClipListChanged, "IsClipListChanged(C)");
	// IDirectDrawClipper::SetClipList
	SetHook((void *)(**(DWORD **)lplpDDClipper + 28), extSetClipListProxy, (void **)&pSetClipList, "SetClipList(C)");
	// IDirectDrawClipper::SetHWnd
	SetHook((void *)(**(DWORD **)lplpDDClipper + 32), extSetHWndProxy, (void **)&pSetHWnd, "SetHWnd(C)");

	return;
}

static void HookDDPalette(LPDIRECTDRAWPALETTE FAR* lplpDDPalette)
{
	OutTraceDW("Hooking directdraw palette dd=%x\n", *lplpDDPalette);

    /*** IDirectDrawPalette methods ***/
	// IDirectDrawPalette::Release
	SetHook((void *)(**(DWORD **)lplpDDPalette + 8), extReleaseP, (void **)&pReleaseP, "Release(P)");
	// IDirectDrawPalette::SetEntries
	SetHook((void *)(**(DWORD **)lplpDDPalette + 24), extSetEntries, (void **)&pSetEntries, "SetEntries(P)");

	if (!(dxw.dwTFlags & OUTPROXYTRACE)) return;

	// IDirectDrawPalette::QueryInterface
	SetHook((void *)(**(DWORD **)lplpDDPalette), extQueryInterfacePProxy, (void **)&pQueryInterfaceP, "QueryInterface(P)");
	// IDirectDrawPalette::AddRef
	SetHook((void *)(**(DWORD **)lplpDDPalette + 4), extAddRefPProxy, (void **)&pAddRefP, "AddRef(P)");
	// IDirectDrawPalette::GetCaps
	SetHook((void *)(**(DWORD **)lplpDDPalette + 12), extGetCapsPProxy, (void **)&pGetCapsP, "GetCaps(P)");
	// IDirectDrawPalette::GetEntries
	SetHook((void *)(**(DWORD **)lplpDDPalette + 16), extGetEntriesProxy, (void **)&pGetEntries, "GetEntries(P)");

	return;
}

static void HookDDSurfacePrim(LPDIRECTDRAWSURFACE *lplpdds, int dxversion)
{
	OutTraceDW("Hooking surface as primary dds=%x dxversion=%d thread_id=%x\n", 
		*lplpdds, dxversion, GetCurrentThreadId());

	// IDirectDrawSurface::Query
	SetHook((void *)(**(DWORD **)lplpdds), extQueryInterfaceS, (void **)&pQueryInterfaceS, "QueryInterface(S)");
	// IDirectDrawSurface::Release
	SetHook((void *)(**(DWORD **)lplpdds + 8), extReleaseS, (void **)&pReleaseS, "Release(S)");
	// IDirectDrawSurface::AddAttachedSurface
	SetHook((void *)(**(DWORD **)lplpdds + 12), extAddAttachedSurface, (void **)&pAddAttachedSurface, "AddAttachedSurface(S)");
	// IDirectDrawSurface::Blt
	SetHook((void *)(**(DWORD **)lplpdds + 20), extBlt, (void **)&pBlt, "Blt(S)");
	// IDirectDrawSurface::BltFast
	SetHook((void *)(**(DWORD **)lplpdds + 28), extBltFast, (void **)&pBltFast, "BltFast(S)");
	// IDirectDrawSurface::DeleteAttachedSurface
	SetHook((void *)(**(DWORD **)lplpdds + 32), extDeleteAttachedSurface, (void **)&pDeleteAttachedSurface, "DeleteAttachedSurface(S)");
	// IDirectDrawSurface::EnumAttachedSurfaces
	SetHook((void *)(**(DWORD **)lplpdds + 36), extEnumAttachedSurfaces, (void **)&pEnumAttachedSurfaces, "EnumAttachedSurfaces(S)");
	// IDirectDrawSurface::Flip
	SetHook((void *)(**(DWORD **)lplpdds + 44), extFlip, (void **)&pFlip, "Flip(S)");
	// IDirectDrawSurface::GetAttachedSurface
	switch(dxversion) {
	case 1:
	case 2:
		SetHook((void *)(**(DWORD **)lplpdds + 48), extGetAttachedSurface1, (void **)&pGetAttachedSurface1, "GetAttachedSurface(S1)");
		break;
	case 3:
		SetHook((void *)(**(DWORD **)lplpdds + 48), extGetAttachedSurface3, (void **)&pGetAttachedSurface3, "GetAttachedSurface(S3)");
		break;
	case 4:
		SetHook((void *)(**(DWORD **)lplpdds + 48), extGetAttachedSurface4, (void **)&pGetAttachedSurface4, "GetAttachedSurface(S4)");
		break;
	case 7:
		SetHook((void *)(**(DWORD **)lplpdds + 48), extGetAttachedSurface7, (void **)&pGetAttachedSurface7, "GetAttachedSurface(S7)");
		break;
	}
	// IDirectDrawSurface::GetCaps
	switch(dxversion) {
	case 1:
		SetHook((void *)(**(DWORD **)lplpdds + 56), extGetCaps1S, (void **)&pGetCaps1S, "GetCaps(S1)");
		break;
	case 2:
		SetHook((void *)(**(DWORD **)lplpdds + 56), extGetCaps2S, (void **)&pGetCaps2S, "GetCaps(S2)");
		break;
	case 3:
		SetHook((void *)(**(DWORD **)lplpdds + 56), extGetCaps3S, (void **)&pGetCaps3S, "GetCaps(S3)");
		break;
	case 4:
		SetHook((void *)(**(DWORD **)lplpdds + 56), extGetCaps4S, (void **)&pGetCaps4S, "GetCaps(S4)");
		break;
	case 7:
		SetHook((void *)(**(DWORD **)lplpdds + 56), extGetCaps7S, (void **)&pGetCaps7S, "GetCaps(S7)");
		break;
	}
	// IDirectDrawSurface::GetColorKey
	SetHook((void *)(**(DWORD **)lplpdds + 64), extGetColorKey, (void **)&pGetColorKey, "GetColorKey(S)");
	// IDirectDrawSurface::GetPalette
	SetHook((void *)(**(DWORD **)lplpdds + 80), extGetPalette, (void **)&pGetPalette, "GetPalette(S)");
	// IDirectDrawSurface::GetPixelFormat
	SetHook((void *)(**(DWORD **)lplpdds + 84), extGetPixelFormat, (void **)&pGetPixelFormat, "GetPixelFormat(S)");
	// IDirectDrawSurface::GetSurfaceDesc
	if (dxversion < 4) {
		SetHook((void *)(**(DWORD **)lplpdds + 88), extGetSurfaceDesc1, (void **)&pGetSurfaceDesc1, "GetSurfaceDesc(S1)");
	}
	if((dxversion >= 4) && (dxversion < 7)) {
		SetHook((void *)(**(DWORD **)lplpdds + 88), extGetSurfaceDesc2, (void **)&pGetSurfaceDesc4, "GetSurfaceDesc(S4)");
	}
	if(dxversion ==  7) {
		SetHook((void *)(**(DWORD **)lplpdds + 88), extGetSurfaceDesc7, (void **)&pGetSurfaceDesc7, "GetSurfaceDesc(S7)");
	}
	// IDirectDrawSurface::SetClipper
	SetHook((void *)(**(DWORD **)lplpdds + 112), extSetClipper, (void **)&pSetClipper, "SetClipper(S)");
	// IDirectDrawSurface::SetColorKey
	SetHook((void *)(**(DWORD **)lplpdds + 116), extSetColorKey, (void **)&pSetColorKey, "SetColorKey(S)");
	// IDirectDrawSurface::SetPalette
	SetHook((void *)(**(DWORD **)lplpdds + 124), extSetPalette, (void **)&pSetPalette, "SetPalette(S)");
	// IDirectDrawSurface::GetDC
	SetHook((void *)(**(DWORD **)lplpdds + 68), extGetDC, (void **)&pGetDC, "GetDC(S)");
	// IDirectDrawSurface::ReleaseDC
	SetHook((void *)(**(DWORD **)lplpdds + 104), extReleaseDC, (void **)&pReleaseDC, "ReleaseDC(S)");
	if (dxw.dwFlags1 & (EMULATESURFACE|EMULATEBUFFER)){
		// IDirectDrawSurface::Lock
		SetHook((void *)(**(DWORD **)lplpdds + 100), extLock, (void **)&pLock, "Lock(S)");
		// IDirectDrawSurface::Unlock
		if (dxversion < 4)
			SetHook((void *)(**(DWORD **)lplpdds + 128), extUnlock1, (void **)&pUnlock1, "Unlock(S1)");
		else
			SetHook((void *)(**(DWORD **)lplpdds + 128), extUnlock4, (void **)&pUnlock4, "Unlock(S4)");
	}
	else {
		// IDirectDrawSurface::Lock
		SetHook((void *)(**(DWORD **)lplpdds + 100), extLockDir, (void **)&pLock, "Lock(S)");
		// IDirectDrawSurface::Unlock
		if (dxversion < 4)
			SetHook((void *)(**(DWORD **)lplpdds + 128), extUnlockDir1, (void **)&pUnlock1, "Unlock(S1)");
		else
			SetHook((void *)(**(DWORD **)lplpdds + 128), extUnlockDir4, (void **)&pUnlock4, "Unlock(S4)");
	}

	if (!(dxw.dwTFlags & OUTPROXYTRACE)) return;

	// Just proxed ...

	// IDirectDrawSurface::AddRef
	SetHook((void *)(**(DWORD **)lplpdds + 4), extAddRefSProxy, (void **)&pAddRefS, "AddRef(S)");
	// IDirectDrawSurface::AddOverlayDirtyRect
	SetHook((void *)(**(DWORD **)lplpdds + 16), extAddOverlayDirtyRectProxy, (void **)&pAddOverlayDirtyRect, "AddOverlayDirtyRect(S)");
	// IDirectDrawSurface::BltBatch
	SetHook((void *)(**(DWORD **)lplpdds + 24), extBltBatchProxy, (void **)&pBltBatch, "BltBatch(S)");
	// IDirectDrawSurface::EnumOverlayZOrders
	SetHook((void *)(**(DWORD **)lplpdds + 40), extEnumOverlayZOrdersProxy, (void **)&pEnumOverlayZOrders, "EnumOverlayZOrders(S)");
	// IDirectDrawSurface::GetBltStatus
	SetHook((void *)(**(DWORD **)lplpdds + 52), extGetBltStatusProxy, (void **)&pGetBltStatus, "GetBltStatus(S)");
	// IDirectDrawSurface::GetClipper
	SetHook((void *)(**(DWORD **)lplpdds + 60), extGetClipperProxy, (void **)&pGetClipper, "GetClipper(S)");
	// IDirectDrawSurface::GetFlipStatus
	SetHook((void *)(**(DWORD **)lplpdds + 72), extGetFlipStatusProxy, (void **)&pGetFlipStatus, "GetFlipStatus(S)");
	// IDirectDrawSurface::GetOverlayPosition
	SetHook((void *)(**(DWORD **)lplpdds + 76), extGetOverlayPositionProxy, (void **)&pGetOverlayPosition, "GetOverlayPosition(S)");
	// IDirectDrawSurface::IsLost
	SetHook((void *)(**(DWORD **)lplpdds + 96), extIsLostProxy, (void **)&pIsLost, "IsLost(S)");
	// IDirectDrawSurface::Restore
	SetHook((void *)(**(DWORD **)lplpdds + 108), extRestoreProxy, (void **)&pRestore, "Restore(S)");
	// IDirectDrawSurface::SetOverlayPosition
	SetHook((void *)(**(DWORD **)lplpdds + 120), extSetOverlayPositionProxy, (void **)&pSetOverlayPosition, "SetOverlayPosition(S)");
	// IDirectDrawSurface::UpdateOverlay
	SetHook((void *)(**(DWORD **)lplpdds + 132), extUpdateOverlayProxy, (void **)&pUpdateOverlay, "UpdateOverlay(S)");
	// IDirectDrawSurface::UpdateOverlayDisplay
	SetHook((void *)(**(DWORD **)lplpdds + 136), extUpdateOverlayDisplayProxy, (void **)&pUpdateOverlayDisplay, "UpdateOverlayDisplay(S)");
	// IDirectDrawSurface::UpdateOverlayZOrder
	SetHook((void *)(**(DWORD **)lplpdds + 140), extUpdateOverlayZOrderProxy, (void **)&pUpdateOverlayZOrder, "UpdateOverlayZOrder(S)");
}

static void HookDDSurfaceGeneric(LPDIRECTDRAWSURFACE *lplpdds, int dxversion)
{
	OutTraceDW("Hooking surface as generic dds=%x dxversion=%d thread_id=%x\n", 
		*lplpdds, dxversion, GetCurrentThreadId());

	// IDirectDrawSurface::QueryInterface
	SetHook((void *)(**(DWORD **)lplpdds), extQueryInterfaceS, (void **)&pQueryInterfaceS, "QueryInterface(S)");
	// IDirectDrawSurface::Release
	SetHook((void *)(**(DWORD **)lplpdds + 8), extReleaseS, (void **)&pReleaseS, "Release(S)");
	// IDirectDrawSurface::AddAttachedSurface
	SetHook((void *)(**(DWORD **)lplpdds + 12), extAddAttachedSurface, (void **)&pAddAttachedSurface, "AddAttachedSurface(S)");
	// IDirectDrawSurface::Flip
	SetHook((void *)(**(DWORD **)lplpdds + 44), extFlip, (void **)&pFlip, "Flip(S)");
	// IDirectDrawSurface::Blt
	SetHook((void *)(**(DWORD **)lplpdds + 20), extBlt, (void **)&pBlt, "Blt(S)");
	// IDirectDrawSurface::BltFast
	SetHook((void *)(**(DWORD **)lplpdds + 28), extBltFast, (void **)&pBltFast, "BltFast(S)");
	// IDirectDrawSurface::DeleteAttachedSurface
	SetHook((void *)(**(DWORD **)lplpdds + 32), extDeleteAttachedSurface, (void **)&pDeleteAttachedSurface, "DeleteAttachedSurface(S)");
	// IDirectDrawSurface::GetAttachedSurface
	switch(dxversion) {
	case 1:
	case 2: 
		SetHook((void *)(**(DWORD **)lplpdds + 48), extGetAttachedSurface1, (void **)&pGetAttachedSurface1, "GetAttachedSurface(S1)"); 
		break;
	case 3:
		SetHook((void *)(**(DWORD **)lplpdds + 48), extGetAttachedSurface3, (void **)&pGetAttachedSurface3, "GetAttachedSurface(S3)");
		break;
	case 4:
		SetHook((void *)(**(DWORD **)lplpdds + 48), extGetAttachedSurface4, (void **)&pGetAttachedSurface4, "GetAttachedSurface(S4)");
		break;
	case 7:
		SetHook((void *)(**(DWORD **)lplpdds + 48), extGetAttachedSurface7, (void **)&pGetAttachedSurface7, "GetAttachedSurface(S7)");
		break;
	}
	// IDirectDrawSurface::GetCaps
	switch(dxversion) {
	case 1:
		SetHook((void *)(**(DWORD **)lplpdds + 56), extGetCaps1S, (void **)&pGetCaps1S, "GetCaps(S1)");
		break;
	case 2:
		SetHook((void *)(**(DWORD **)lplpdds + 56), extGetCaps2S, (void **)&pGetCaps2S, "GetCaps(S2)");
		break;
	case 3:
		SetHook((void *)(**(DWORD **)lplpdds + 56), extGetCaps3S, (void **)&pGetCaps3S, "GetCaps(S3)");
		break;
	case 4:
		SetHook((void *)(**(DWORD **)lplpdds + 56), extGetCaps4S, (void **)&pGetCaps4S, "GetCaps(S4)");
		break;
	case 7:
		SetHook((void *)(**(DWORD **)lplpdds + 56), extGetCaps7S, (void **)&pGetCaps7S, "GetCaps(S7)");
		break;
	}
	// IDirectDrawSurface::GetDC
	SetHook((void *)(**(DWORD **)lplpdds + 68), extGetDC, (void **)&pGetDC, "GetDC(S)");	// IDirectDrawSurface::GetSurfaceDesc
	// IDirectDrawSurface::GetSurfaceDesc
	if (dxversion < 4) {
		SetHook((void *)(**(DWORD **)lplpdds + 88), extGetSurfaceDesc1, (void **)&pGetSurfaceDesc1, "GetSurfaceDesc(S1)");
	}
	else {
		SetHook((void *)(**(DWORD **)lplpdds + 88), extGetSurfaceDesc2, (void **)&pGetSurfaceDesc4, "GetSurfaceDesc(S4)");
	}
	// IDirectDrawSurface::ReleaseDC
	SetHook((void *)(**(DWORD **)lplpdds + 104), extReleaseDC, (void **)&pReleaseDC, "ReleaseDC(S)");

	if (dxw.dwFlags1 & (EMULATESURFACE|EMULATEBUFFER)){
		// IDirectDrawSurface::Lock
		SetHook((void *)(**(DWORD **)lplpdds + 100), extLock, (void **)&pLock, "Lock(S)");
		// IDirectDrawSurface::Unlock
		if (dxversion < 4)
			SetHook((void *)(**(DWORD **)lplpdds + 128), extUnlock1, (void **)&pUnlock1, "Unlock(S1)");
		else
			SetHook((void *)(**(DWORD **)lplpdds + 128), extUnlock4, (void **)&pUnlock4, "Unlock(S4)");
	}
	else {
		// IDirectDrawSurface::Lock
		SetHook((void *)(**(DWORD **)lplpdds + 100), extLockDir, (void **)&pLock, "Lock(S)");
		// IDirectDrawSurface::Unlock
		if (dxversion < 4)
			SetHook((void *)(**(DWORD **)lplpdds + 128), extUnlockDir1, (void **)&pUnlock1, "Unlock(S1)");
		else
			SetHook((void *)(**(DWORD **)lplpdds + 128), extUnlockDir4, (void **)&pUnlock4, "Unlock(S4)");
	}

	if (dxversion == 7)
		SetHook((void *)(**(DWORD **)lplpdds + 156), extSetSurfaceDesc, (void **)&pSetSurfaceDesc, "SetSurfaceDesc(S3)");

	if (!(dxw.dwTFlags & OUTPROXYTRACE)) return;

	// just proxed ....

	// IDirectDrawSurface::AddRef
	SetHook((void *)(**(DWORD **)lplpdds + 4), extAddRefSProxy, (void **)&pAddRefS, "AddRef(S)");
	// IDirectDrawSurface::AddOverlayDirtyRect
	SetHook((void *)(**(DWORD **)lplpdds + 16), extAddOverlayDirtyRectProxy, (void **)&pAddOverlayDirtyRect, "AddOverlayDirtyRect(S)");
	// IDirectDrawSurface::BltBatch
	SetHook((void *)(**(DWORD **)lplpdds + 24), extBltBatchProxy, (void **)&pBltBatch, "BltBatch(S)");
	// IDirectDrawSurface::EnumAttachedSurfaces
	SetHook((void *)(**(DWORD **)lplpdds + 36), extEnumAttachedSurfaces, (void **)&pEnumAttachedSurfaces, "EnumAttachedSurfaces(S)");
	// IDirectDrawSurface::EnumOverlayZOrders
	SetHook((void *)(**(DWORD **)lplpdds + 40), extEnumOverlayZOrdersProxy, (void **)&pEnumOverlayZOrders, "EnumOverlayZOrders(S)");
	// IDirectDrawSurface::GetBltStatus
	SetHook((void *)(**(DWORD **)lplpdds + 52), extGetBltStatusProxy, (void **)&pGetBltStatus, "GetBltStatus(S)");
	// IDirectDrawSurface::GetClipper
	SetHook((void *)(**(DWORD **)lplpdds + 60), extGetClipperProxy, (void **)&pGetClipper, "GetClipper(S)");
	// IDirectDrawSurface::GetFlipStatus
	SetHook((void *)(**(DWORD **)lplpdds + 72), extGetFlipStatusProxy, (void **)&pGetFlipStatus, "GetFlipStatus(S)");
	// IDirectDrawSurface::GetOverlayPosition
	SetHook((void *)(**(DWORD **)lplpdds + 76), extGetOverlayPositionProxy, (void **)&pGetOverlayPosition, "GetOverlayPosition(S)");
	// IDirectDrawSurface::IsLost
	SetHook((void *)(**(DWORD **)lplpdds + 96), extIsLostProxy, (void **)&pIsLost, "IsLost(S)");
	// IDirectDrawSurface::Lock
	SetHook((void *)(**(DWORD **)lplpdds + 100), extLock, (void **)&pLock, "Lock(S)");
	// IDirectDrawSurface::Restore
	SetHook((void *)(**(DWORD **)lplpdds + 108), extRestoreProxy, (void **)&pRestore, "Restore(S)");
	// IDirectDrawSurface::SetOverlayPosition
	SetHook((void *)(**(DWORD **)lplpdds + 120), extSetOverlayPositionProxy, (void **)&pSetOverlayPosition, "SetOverlayPosition(S)");
	// IDirectDrawSurface::Unlock
	if  (dxversion < 4)
		SetHook((void *)(**(DWORD **)lplpdds + 128), extUnlock1, (void **)&pUnlock1, "Unlock(S1)");
	else
		SetHook((void *)(**(DWORD **)lplpdds + 128), extUnlock4, (void **)&pUnlock4, "Unlock(S4)");
	// IDirectDrawSurface::UpdateOverlay
	SetHook((void *)(**(DWORD **)lplpdds + 132), extUpdateOverlayProxy, (void **)&pUpdateOverlay, "UpdateOverlay(S)");
	// IDirectDrawSurface::UpdateOverlayDisplay
	SetHook((void *)(**(DWORD **)lplpdds + 136), extUpdateOverlayDisplayProxy, (void **)&pUpdateOverlayDisplay, "UpdateOverlayDisplay(S)");
	// IDirectDrawSurface::UpdateOverlayZOrder
	SetHook((void *)(**(DWORD **)lplpdds + 140), extUpdateOverlayZOrderProxy, (void **)&pUpdateOverlayZOrder, "UpdateOverlayZOrder(S)");
}

static void HookGammaControl(LPVOID *obp)
{
	// IDirectDrawGammaControl::GetGammaRamp
	SetHook((void *)(**(DWORD **)obp + 12), extDDGetGammaRamp, (void **)&pDDGetGammaRamp, "GetGammaRamp(G)");
	// IDirectDrawGammaControl::SetGammaRamp
	SetHook((void *)(**(DWORD **)obp + 16), extDDSetGammaRamp, (void **)&pDDSetGammaRamp, "SetGammaRamp(G)");
}

/* ------------------------------------------------------------------------------ */
// CleanRect:
// takes care of a corrupted RECT struct where some elements are not valid pointers.
// In this case, the whole RECT * variable is set to NULL, a value that is interpreted
// by directdraw functions as the whole surface area.
/* ------------------------------------------------------------------------------ */

static void CleanRect(RECT **lprect, int line)
{
	__try {
		// normally unharmful statements
		if(*lprect){
			int i;
			i=(*lprect)->bottom;
			i=(*lprect)->top;
			i=(*lprect)->left;
			i=(*lprect)->right;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER){ 
		OutTraceE("Rectangle exception caught at %d: invalid RECT\n", __LINE__);
		if(IsAssertEnabled) MessageBox(0, "Rectangle exception", "CleanRect", MB_OK | MB_ICONEXCLAMATION);
        *lprect=NULL;
    }
}

static void MaskCapsD(LPDDCAPS c1, LPDDCAPS c2)
{
	FILE *capfile;
	//char sBuf[80+1];
	char token[20+1];
	DWORD val;
	OutTraceDW("MaskCaps\n");
	capfile=fopen("dxwnd.cap", "r");
	if(!capfile) return;
	if(c1) {
		while(TRUE){
			if(fscanf(capfile, "%s=%x", token, &val)!=2) break;
			if(!strcmp(token, "dwCaps")) c1->dwCaps &= val;
			if(!strcmp(token, "dwCaps2")) c1->dwCaps2 &= val;
			if(!strcmp(token, "dwCKeyCaps")) c1->dwCKeyCaps &= val;
			if(!strcmp(token, "dwFXCaps")) c1->dwFXCaps &= val;
		}
		OutTraceDW("MaskCaps(D-HW): caps=%x(%s) caps2=%x(%s) fxcaps=%x(%s) fxalphacaps=%x(%s) keycaps=%x(%s)\n", 
			c1->dwCaps, ExplainDDDCaps(c1->dwCaps),
			c1->dwCaps2, ExplainDDDCaps2(c1->dwCaps2),
			c1->dwFXCaps, ExplainDDFXCaps(c1->dwFXCaps),
			c1->dwFXAlphaCaps, ExplainDDFXALPHACaps(c1->dwFXAlphaCaps),
			c1->dwCKeyCaps, ExplainDDCKeyCaps(c1->dwCKeyCaps));
	}
	if(c2) {
		while(TRUE){
			if(fscanf(capfile, "%s=%x", token, &val)!=2) break;
			if(!strcmp(token, "dwCaps")) c2->dwCaps &= val;
			if(!strcmp(token, "dwCaps2")) c2->dwCaps2 &= val;
			if(!strcmp(token, "dwCKeyCaps")) c2->dwCKeyCaps &= val;
			if(!strcmp(token, "dwFXCaps")) c2->dwFXCaps &= val;
		}
		OutTraceDW("MaskCaps(D-HW): caps=%x(%s) caps2=%x(%s) fxcaps=%x(%s) fxalphacaps=%x(%s) keycaps=%x(%s)\n", 
			c2->dwCaps, ExplainDDDCaps(c2->dwCaps),
			c2->dwCaps2, ExplainDDDCaps2(c2->dwCaps2),
			c2->dwFXCaps, ExplainDDFXCaps(c2->dwFXCaps),
			c2->dwFXAlphaCaps, ExplainDDFXALPHACaps(c2->dwFXAlphaCaps),
			c2->dwCKeyCaps, ExplainDDCKeyCaps(c2->dwCKeyCaps));
	}	
	fclose(capfile);
}

/* ------------------------------------------------------------------------------ */
// directdraw method hooks
/* ------------------------------------------------------------------------------ */

static void HandleCapsD(char *sLabel, LPDDCAPS c)
{
	OutTraceDDRAW(
		"GetCaps(%s): caps=%x(%s) caps2=%x(%s) palcaps=%x(%s) "
		"fxcaps=%x(%s) fxalphacaps=%x(%s) keycaps=%x(%s)\n", 
		sLabel,
		c->dwCaps, ExplainDDDCaps(c->dwCaps),
		c->dwCaps2, ExplainDDDCaps2(c->dwCaps2),
		c->dwPalCaps, ExplainDDPalCaps(c->dwPalCaps),
		c->dwFXCaps, ExplainDDFXCaps(c->dwFXCaps),
		c->dwFXAlphaCaps, ExplainDDFXALPHACaps(c->dwFXAlphaCaps),
		c->dwCKeyCaps, ExplainDDCKeyCaps(c->dwCKeyCaps));
	OutTraceDDRAW("GetCaps(%s): VidMemTotal=%x VidMemFree=%x\n", 
		sLabel, c->dwVidMemTotal, c->dwVidMemFree);
	if(dxw.dwFlags2 & LIMITRESOURCES){ // check for memory value overflow
		const DWORD dwMaxMem = 0x70000000; 
		if(c->dwVidMemTotal > dwMaxMem) c->dwVidMemTotal = dwMaxMem;
		if(c->dwVidMemFree  > dwMaxMem) c->dwVidMemFree  = dwMaxMem;
	}
	if(dxw.dwFlags5 & STRESSRESOURCES){
		//const DWORD dwHugeMem = 0xF0000000; 
		const DWORD dwHugeMem = 0xFFFFFFFF; 
		c->dwVidMemTotal = dwHugeMem;		
		c->dwVidMemFree  = dwHugeMem;	
	}
	if((dxw.dwFlags5 & STRESSRESOURCES) || (dxw.dwFlags5 & STRESSRESOURCES))
		OutTraceDDRAW("GetCaps(%s): FIXED VidMemTotal=%x VidMemFree=%x\n", sLabel, c->dwVidMemTotal, c->dwVidMemFree);
}

HRESULT WINAPI extGetCapsD(LPDIRECTDRAW lpdd, LPDDCAPS c1, LPDDCAPS c2)
{
	HRESULT res;
	OutTraceDDRAW("GetCaps(D): lpdd=%x %s %s\n", lpdd, c1?"c1":"NULL", c2?"c2":"NULL");
	res=(*pGetCapsD)(lpdd, c1, c2);
	if(res) 
		OutTraceE("GetCaps(D): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	else {
		if (c1) HandleCapsD("D-HW", c1);
		if (c2) HandleCapsD("D-SW", c2);
	}

	if((dxw.dwFlags3 & FORCESHEL) && c1) {
		DDCAPS_DX7 swcaps; // DDCAPS_DX7 because it is the bigger in size
		int size;
		size=c1->dwSize;
		if (!c2) {
			memset(&swcaps, 0, sizeof(DDCAPS_DX7));
			swcaps.dwSize=size;
			c2=&swcaps;
			res=(*pGetCapsD)(lpdd, NULL, c2);
		}
		DWORD dwVidMemTotal=c1->dwVidMemTotal;
		DWORD dwVidMemFree=c1->dwVidMemFree;
		memcpy((void *)c1, (void *)c2, size);
		if(c1->dwVidMemTotal == 0) c1->dwVidMemTotal=dwVidMemTotal; 
		if(c1->dwVidMemFree  == 0) c1->dwVidMemFree =dwVidMemFree; 
		if(c1->dwVidMemTotal == 0) c1->dwVidMemTotal=0x20000000; // 500 MB
		if(c1->dwVidMemFree  == 0) c1->dwVidMemFree =0x20000000; // 500 MB
		if (c1) HandleCapsD("D-HW(fixed)", c1);
		if (c2) HandleCapsD("D-SW(fixed)", c2);
	}

	if(dxw.dwFlags3 & CAPMASK) MaskCapsD(c1, c2);

	return res;
}

HRESULT WINAPI extDirectDrawCreate(GUID FAR *lpguid, LPDIRECTDRAW FAR *lplpdd, IUnknown FAR *pu)
{
	HRESULT res;
	GUID FAR *lpPrivGuid = lpguid;

	OutTraceDDRAW("DirectDrawCreate: guid=%x(%s)\n", lpguid, ExplainGUID(lpguid));

	if(!pDirectDrawCreate){ // not hooked yet....
		HINSTANCE hinst;
		hinst = LoadLibrary("ddraw.dll");
		if(!hinst){
			OutTraceE("LoadLibrary ERROR err=%d at %d\n", GetLastError(), __LINE__);
		}
		pDirectDrawCreate =
			(DirectDrawCreate_Type)GetProcAddress(hinst, "DirectDrawCreate");
		if(pDirectDrawCreate)
			HookAPI(hinst, "ddraw.dll", pDirectDrawCreate, "DirectDrawCreate", extDirectDrawCreate); // v2.02.52
		else{
			char sMsg[81];
			sprintf_s(sMsg, 80, "DirectDrawCreate hook failed: error=%d\n", GetLastError());
			OutTraceDW(sMsg);
			if(IsAssertEnabled) MessageBox(0, sMsg, "Hook", MB_OK | MB_ICONEXCLAMATION);
			return DDERR_GENERIC; // is there a better one?
		}
	}

	if((dxw.dwFlags3 & FORCESHEL) && (lpguid==NULL)) lpPrivGuid=(GUID FAR *)DDCREATE_EMULATIONONLY;

	res = (*pDirectDrawCreate)(lpPrivGuid, lplpdd, pu);
	if(res) {
		OutTraceE("DirectDrawCreate: ERROR res=%x(%s)\n", res, ExplainDDError(res));
		return res;
	}

	if(dxw.dwFlags3 & COLORFIX) (*((DDRAWI_DIRECTDRAW_INT **)lplpdd))->lpLcl->dwAppHackFlags |= 0x800;

	dxw.dwDDVersion=1;
	char *mode;
	switch ((DWORD)lpPrivGuid){
		case 0: mode="NULL"; break;
		case DDCREATE_HARDWAREONLY: mode="DDCREATE_HARDWAREONLY"; break;
		case DDCREATE_EMULATIONONLY: mode="DDCREATE_EMULATIONONLY"; break;
		default:
			switch (*(DWORD *)lpguid){
				case 0x6C14DB80: dxw.dwDDVersion=1; mode="IID_IDirectDraw"; break;
				case 0xB3A6F3E0: dxw.dwDDVersion=2; mode="IID_IDirectDraw2"; break;
				case 0x9c59509a: dxw.dwDDVersion=4; mode="IID_IDirectDraw4"; break;
				case 0x15e65ec0: dxw.dwDDVersion=7; mode="IID_IDirectDraw7"; break;
				default: mode="unknown"; break;
			}
			break;
	}
	OutTraceDDRAW("DirectDrawCreate: lpdd=%x guid=%s DDVersion=%d\n", *lplpdd, mode, dxw.dwDDVersion);

	HookDDSession(lplpdd, dxw.dwDDVersion);

	// beware: old ddraw games seem to reuse a closed ddraw handle. The theory is that maybe whenever
	// you created a ddraw session, you got more than one reference count, so that releasing it once 
	// did not destroy the object.... but why? when (I mean, in which situations? Maybe not always...)
	// and for which releases? The fix here assumes that this fact is true and holds always for ddraw 1
	// sessions. 
	// update: this is no good for "Jet Moto" that created a ddraw session and expects to Release it 
	// with a refcount zero.

	//if(dxw.dwDDVersion==1) (*lplpdd)->AddRef(); // seems to fix problems in "Warhammer 40K Rites Of War"

	if(IsDebug && (dxw.dwTFlags & OUTPROXYTRACE)){
		DDCAPS DriverCaps, EmulCaps;
		memset(&DriverCaps, 0, sizeof(DriverCaps));
		DriverCaps.dwSize=sizeof(DriverCaps);
		memset(&EmulCaps, 0, sizeof(EmulCaps));
		EmulCaps.dwSize=sizeof(EmulCaps);
		(LPDIRECTDRAW)(*lplpdd)->GetCaps(&DriverCaps, &EmulCaps);
		//OutTrace("DirectDrawCreate: drivercaps=%x(%s) emulcaps=%x(%s)\n", DriverCaps.ddsCaps, "???", EmulCaps.ddsCaps, "???");
	}

	if(lpPrimaryDD==NULL) lpPrimaryDD=*lplpdd; // do not override the value set when creating the primary surface!
	return DD_OK;
}

HRESULT WINAPI extDirectDrawCreateEx(GUID FAR *lpguid,
	LPDIRECTDRAW FAR *lplpdd, REFIID iid, IUnknown FAR *pu)
{
	HRESULT res;
	GUID FAR *lpPrivGuid = lpguid;

	OutTraceDDRAW("DirectDrawCreateEx: guid=%x(%s) refiid=%x\n", lpguid, ExplainGUID(lpguid), iid);

	// v2.1.70: auto-hooking (just in case...)
	if(!pDirectDrawCreateEx){ // not hooked yet....
		HINSTANCE hinst;
		hinst = LoadLibrary("ddraw.dll");
		if(!hinst){
			OutTraceE("LoadLibrary ERROR err=%d at %d\n", GetLastError(), __LINE__);
		}
		pDirectDrawCreateEx =
			(DirectDrawCreateEx_Type)GetProcAddress(hinst, "DirectDrawCreateEx");
		if(pDirectDrawCreateEx)
			HookAPI(hinst, "ddraw.dll", pDirectDrawCreateEx, "DirectDrawCreateEx", extDirectDrawCreateEx); // v2.02.52
		else{
			char sMsg[81];
			sprintf_s(sMsg, 80, "DirectDrawCreateEx hook failed: error=%d\n", GetLastError());
			OutTraceDW(sMsg);
			if(IsAssertEnabled) MessageBox(0, sMsg, "Hook", MB_OK | MB_ICONEXCLAMATION);
			return DDERR_GENERIC; // is there a better one?
		}
	}

	if((dxw.dwFlags3 & FORCESHEL) && (lpguid==NULL)) lpPrivGuid=(GUID FAR *)DDCREATE_EMULATIONONLY;

	res = (*pDirectDrawCreateEx)(lpPrivGuid, lplpdd, iid, pu);
	if (res){
		OutTraceDW("DirectDrawCreateEx: res=%x(%s)\n",res, ExplainDDError(res));
		return res;
	}

	if(dxw.dwFlags3 & COLORFIX) (*((DDRAWI_DIRECTDRAW_INT **)lplpdd))->lpLcl->dwAppHackFlags |= 0x800;

	dxw.dwDDVersion=7;
	char *mode;
	switch ((DWORD)lpPrivGuid){
		case 0: mode="NULL"; break;
		case DDCREATE_HARDWAREONLY: mode="DDCREATE_HARDWAREONLY"; break;
		case DDCREATE_EMULATIONONLY: mode="DDCREATE_EMULATIONONLY"; break;
		default:
			switch (*(DWORD *)lpguid){
				case 0x6C14DB80: dxw.dwDDVersion=1; mode="IID_IDirectDraw"; break;
				case 0xB3A6F3E0: dxw.dwDDVersion=2; mode="IID_IDirectDraw2"; break;
				case 0x9c59509a: dxw.dwDDVersion=4; mode="IID_IDirectDraw4"; break;
				case 0x15e65ec0: dxw.dwDDVersion=7; mode="IID_IDirectDraw7"; break;
				default: mode="unknown"; break;
			}
			break;
	}
	OutTraceDDRAW("DirectDrawCreateEx: lpdd=%x guid=%s DDVersion=%d\n", *lplpdd, mode, dxw.dwDDVersion);

	HookDDSession(lplpdd, dxw.dwDDVersion);

	if(IsDebug && (dxw.dwTFlags & OUTPROXYTRACE)){
		DDCAPS DriverCaps, EmulCaps;
		memset(&DriverCaps, 0, sizeof(DriverCaps));
		DriverCaps.dwSize=sizeof(DriverCaps);
		memset(&EmulCaps, 0, sizeof(EmulCaps));
		EmulCaps.dwSize=sizeof(EmulCaps);
		(LPDIRECTDRAW)(*lplpdd)->GetCaps(&DriverCaps, &EmulCaps);
		//OutTrace("DirectDrawCreate: drivercaps=%x(%s) emulcaps=%x(%s)\n", DriverCaps.ddsCaps, "???", EmulCaps.ddsCaps, "???");
	}

	if(lpPrimaryDD==NULL) lpPrimaryDD=*lplpdd; // do not override the value set when creating the primary surface!
	return DD_OK;
}

HRESULT WINAPI extInitialize(LPDIRECTDRAW lpdd, GUID FAR *lpguid)
{
	HRESULT res;
	GUID FAR *lpPrivGuid = lpguid;

	OutTraceDDRAW("Initialize: lpdd=%x guid=%x(%s)\n", lpdd, lpguid, ExplainGUID(lpguid));

	if((dxw.dwFlags3 & FORCESHEL) && (lpguid==NULL)) lpPrivGuid=(GUID FAR *)DDCREATE_EMULATIONONLY;

	res=(*pInitialize)(lpdd, lpPrivGuid);

	if(dxw.dwFlags3 & COLORFIX) (((DDRAWI_DIRECTDRAW_INT *)lpdd))->lpLcl->dwAppHackFlags |= 0x800;

	if(res) OutTraceE("Initialize ERROR: res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extQueryInterfaceD(void *lpdd, REFIID riid, LPVOID *obp)
{
	HRESULT res;
	unsigned int dwLocalDDVersion;
	unsigned int dwLocalD3DVersion;

	res = (*pQueryInterfaceD)(lpdd, riid, obp);
	OutTraceDDRAW("QueryInterface(D): lpdd=%x REFIID=%x(%s) obp=%x ret=%x at %d\n",
		lpdd, riid.Data1, ExplainGUID((GUID *)&riid), *obp, res, __LINE__);	
	
	if(res) return res;

	dwLocalDDVersion=0;
	dwLocalD3DVersion=0;
	switch(riid.Data1){
	case 0x6C14DB80:		//DirectDraw1
		dwLocalDDVersion = 1;	
		break;
	case 0xB3A6F3E0:		//DirectDraw2
		dwLocalDDVersion = 2;
		break;
	case 0x9c59509a:		//DirectDraw4 
		dwLocalDDVersion = 4;
		break;
	case 0x15e65ec0:		//DirectDraw7
		dwLocalDDVersion = 7;
		break;
	case 0x3BBA0080:		//Direct3D
		dwLocalD3DVersion = 1;
		break;
	case 0x6aae1ec1:		//Direct3D2
		dwLocalD3DVersion = 2;
		break;
	case 0xbb223240:		//Direct3D3
		dwLocalD3DVersion = 3;
		break;
	case 0xf5049e77:		//Direct3D7
		dwLocalD3DVersion = 7;
		break;
	}
	if (! *obp){ 
		OutTraceDDRAW("QueryInterface(D): Interface for DX version %d not found\n", dwLocalDDVersion);
		return(0);
	}
	if(dwLocalDDVersion)  OutTraceDW("QueryInterface(D): Got interface for DX version %d\n", dwLocalDDVersion);
	if(dwLocalD3DVersion) OutTraceDW("QueryInterface(D): Got interface for D3D version %d\n", dwLocalD3DVersion);

	if (dwLocalDDVersion > dxw.dwMaxDDVersion) {
		*obp = NULL;
		OutTraceDW("QueryInterface(D): lpdd=%x REFIID=%x obp=(NULL) ret=%x at %d\n",
			lpdd, riid.Data1, res, __LINE__);
		return(0);
	}

	switch (dwLocalDDVersion){
	case 1: // you never know ....
	case 2:
	case 4:
		// it's not supposed to be written for DDVersion==7, but it works ....
	case 7:
		dxw.dwDDVersion=dwLocalDDVersion;
		HookDDSession((LPDIRECTDRAW *)obp, dxw.dwDDVersion);
		break;
	}

	extern void HookDirect3DSession(LPDIRECTDRAW *, int);
	switch (dwLocalD3DVersion){
	case 1: 
	case 2:
	case 3:
	case 7:
		HookDirect3DSession((LPDIRECTDRAW *)obp, dwLocalD3DVersion);
		break;
	}

	OutTraceDDRAW("QueryInterface(D): lpdd=%x REFIID=%x obp=%x DDVersion=%d ret=0\n",
		lpdd, riid.Data1, *obp, dxw.dwDDVersion);

	return 0;
}

// some unhandled interfaces in emulation mode:
// REFIID={84e63de0-46aa-11cf-816f-0000c020156e}: IID_IDirect3DHALDevice

HRESULT WINAPI extQueryInterfaceS(void *lpdds, REFIID riid, LPVOID *obp)
{
	HRESULT res;
	BOOL IsPrim;
	BOOL IsBack;
	BOOL IsGammaRamp;
	unsigned int dwLocalDDVersion;
	unsigned int dwLocalTexVersion;

	OutTraceDDRAW("QueryInterface(S): lpdds=%x REFIID=%x(%s) obp=%x\n",
		lpdds, riid.Data1, ExplainGUID((GUID *)&riid), *obp);	

	IsPrim=dxw.IsAPrimarySurface((LPDIRECTDRAWSURFACE)lpdds);
	IsBack=dxw.IsABackBufferSurface((LPDIRECTDRAWSURFACE)lpdds);
	IsGammaRamp=FALSE;

	dwLocalDDVersion=0;
	dwLocalTexVersion=0;
	switch(riid.Data1){
	case 0x6C14DB81:
		dwLocalDDVersion = 1;
		break;
	case 0x57805885:		//DDSurface2 - WIP (Dark Reign)
		dwLocalDDVersion = 2;
		break;
	case 0xDA044E00:		//DDSurface3
		dwLocalDDVersion = 3;
		break;
	case 0x0B2B8630:
		dwLocalDDVersion = 4;
		break;
	case 0x06675a80:
		dwLocalDDVersion = 7;
		break;
	case 0x84e63de0:
		OutTraceDW("QueryInterface: IID_IDirect3DHALDevice\n");
		break;
	case 0xA4665C60: //  IID_IDirect3DRGBDevice
		OutTraceDW("QueryInterface: IID_IDirect3DRGBDevice\n");
		break;
	case 0xF2086B20: //  IID_IDirect3DRampDevice
		OutTraceDW("QueryInterface: IID_IDirect3DRampDevice\n");
		break;
	case 0x881949a1: //  IID_IDirect3DMMXDevice
		OutTraceDW("QueryInterface: IID_IDirect3DMMXDevice\n");
		break;
	case 0x4B9F0EE0:
		OutTraceDW("QueryInterface: IID_IDirectDrawColorControl\n");
		break;
	case 0x69C11C3E:
		OutTraceDW("QueryInterface: IID_IDirectDrawGammaControl\n");
		IsGammaRamp=TRUE;
		break;
	// textures
	case 0x2CDCD9E0:
		OutTraceDW("QueryInterface: IID_IDirect3DTexture\n");
		dwLocalTexVersion = 1;
		break;
	case 0x93281502:
		OutTraceDW("QueryInterface: IID_IDirect3DTexture2\n");
		dwLocalTexVersion = 2;
		break;
	} 

	if (dwLocalDDVersion > dxw.dwMaxDDVersion) {
		*obp = NULL;
		OutTraceDW("QueryInterface(S): DDVersion=%d SUPPRESS lpdds=%x(%s) REFIID=%x obp=(NULL) ret=0 at %d\n",
			dwLocalDDVersion, lpdds, IsPrim?"":"(PRIM)", riid.Data1, __LINE__);
		return(0);
	}

	// fix the target for gamma ramp creation: if it is a primary surface, use the real one!!
	if(dxw.IsAPrimarySurface((LPDIRECTDRAWSURFACE)lpdds) && IsGammaRamp) lpdds = lpDDSEmu_Prim;

	res = (*pQueryInterfaceS)(lpdds, riid, obp);

	if(res) // added trace
	{
		OutTraceDW("QueryInterface(S): ERROR lpdds=%x%s REFIID=%x obp=%x ret=%x(%s) at %d\n",
			lpdds, IsPrim?"(PRIM)":"", riid.Data1, *obp, res, ExplainDDError(res), __LINE__);
		return res;
	}

	if (! *obp) {
		OutTraceDW("QueryInterface(S): Interface for DX version %d not found\n", dwLocalDDVersion);
		return 0;
	}

	// added trace
	OutTraceDW("QueryInterface(S): lpdds=%x%s REFIID=%x obp=%x DDVersion=%d TexVersion=%d GammaRamp=%d ret=0\n",
		lpdds, IsPrim?"(PRIM)":"", riid.Data1, *obp, dwLocalDDVersion, dwLocalTexVersion, IsGammaRamp);

	if (dwLocalDDVersion) {
		switch (dwLocalDDVersion){
			case 1: // added for The Sims
			case 2:
			case 3:
			case 4: 
			case 7:
			dxw.dwDDVersion=dwLocalDDVersion;
			if(IsPrim){
				OutTraceDW("QueryInterface(S): primary=%x new=%x\n", lpdds, *obp);
				dxw.MarkPrimarySurface((LPDIRECTDRAWSURFACE)*obp);
				HookDDSurfacePrim((LPDIRECTDRAWSURFACE *)obp, dxw.dwDDVersion);
			}
			else{
				if(IsBack) dxw.MarkBackBufferSurface((LPDIRECTDRAWSURFACE)*obp);
				else dxw.MarkRegularSurface((LPDIRECTDRAWSURFACE)*obp);
				// v2.02.13: seems that hooking inconditionally gives troubles. What is the proper safe hook condition?
				HookDDSurfaceGeneric((LPDIRECTDRAWSURFACE *)obp, dxw.dwDDVersion);
			}
			break;
		}
		if(lpdds == lpDDSBack) {
			// assume that you always use the newer interface version, if available.
			if(dwLocalDDVersion > (UINT)iBakBufferVersion){
				OutTraceDW("QueryInterface(S): switching backbuffer %x -> %x\n", lpDDSBack, *obp); 
				lpDDSBack = (LPDIRECTDRAWSURFACE)*obp;
				iBakBufferVersion = dwLocalDDVersion;
			}
		}
	}

	if(dwLocalTexVersion) {
		// Texture Handling on QueryInterface
		if(dxw.dwFlags5 & TEXTUREMASK) TextureHandling((LPDIRECTDRAWSURFACE)lpdds);
		HookTexture(obp, dwLocalTexVersion);
	}

	if(IsGammaRamp)	HookGammaControl(obp);

	return 0;
}

HRESULT WINAPI extSetDisplayMode(int version, LPDIRECTDRAW lpdd,
	DWORD dwwidth, DWORD dwheight, DWORD dwbpp, DWORD dwrefreshrate, DWORD dwflags)
{
	DDSURFACEDESC2 ddsd;
	HRESULT res = 0;

	if(IsTraceDDRAW){
		char sInfo[81];
		strcpy(sInfo, "");
		if (version==2) sprintf(sInfo, " dwRefresh=%i dwFlags=%x", dwrefreshrate, dwflags);
		OutTrace("SetDisplayMode: version=%d dwWidth=%i dwHeight=%i dwBPP=%i%s\n",
			version, dwwidth, dwheight, dwbpp, sInfo);
	}

	// binkplayer fix
	if((int)dwwidth < 0) dwwidth = dxw.GetScreenWidth();
	if((int)dwheight < 0) dwheight = dxw.GetScreenHeight();

	dxw.SetScreenSize(dwwidth, dwheight);
	GetHookInfo()->Height=(short)dxw.GetScreenHeight();
	GetHookInfo()->Width=(short)dxw.GetScreenWidth();
	GetHookInfo()->ColorDepth=(short)dwbpp;
	if(dxw.Windowize) AdjustWindowFrame(dxw.GethWnd(), dwwidth, dwheight);

	if(dxw.dwFlags1 & EMULATESURFACE){
		// in EMULATESURFACE mode, let SetPixFmt decide upon the PixelFormat
		dxw.VirtualPixelFormat.dwRGBBitCount = dwbpp;
		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize=sizeof(ddsd);
		SetPixFmt(&ddsd);
		SetBltTransformations();
		if(dxw.Windowize) {
			OutTraceDW("SetDisplayMode: mode=EMULATE %s ret=OK\n", DumpPixelFormat(&ddsd));
			SetVSyncDelays(lpdd);
			return DD_OK;
		}
	}
	else{
		OutTraceDW("SetDisplayMode: mode=STANDARD BPP=%d\n", dwbpp);
		dxw.VirtualPixelFormat.dwRGBBitCount = dwbpp;
		dxw.ActualPixelFormat.dwRGBBitCount = dwbpp;
	}

	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = Set_dwSize_From_DDraw(lpdd);
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_REFRESHRATE;
	ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB; 
	res=(*pGetDisplayMode)(lpdd, (LPDDSURFACEDESC)&ddsd);
	if(res==DDERR_GENERIC){ // handling Win8 missing support for old ddraw interface
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		res=(*pGetDisplayMode)(lpdd, (LPDDSURFACEDESC)&ddsd);
	}
	
	if(dxw.Windowize){
		if(!IsChangeDisplaySettingsHotPatched){
			dwwidth = ddsd.dwWidth;
			dwheight = ddsd.dwHeight;
		}
		OutTraceDW("SetDisplayMode: fixing colordepth current=%d required=%d size=(%dx%d)\n",
				ddsd.ddpfPixelFormat.dwRGBBitCount, dwbpp, dwwidth, dwheight);
	}

	if(dxw.dwFlags1 & EMULATESURFACE){
		dwbpp = ddsd.ddpfPixelFormat.dwRGBBitCount;
	}

	if (version==1)
		res = (*pSetDisplayMode1)(lpdd, dwwidth, dwheight, dwbpp);
	else
		res = (*pSetDisplayMode2)(lpdd, dwwidth, dwheight, dwbpp, ddsd.dwRefreshRate, 0);

	if(res) OutTraceE("SetDisplayMode: error=%x\n", res);

	SetVSyncDelays(lpdd);
	return DD_OK;
}

HRESULT WINAPI extSetDisplayMode2(LPDIRECTDRAW lpdd,
	DWORD dwwidth, DWORD dwheight, DWORD dwbpp, DWORD dwrefreshrate, DWORD dwflags)
{
	return extSetDisplayMode(2, lpdd, dwwidth, dwheight, dwbpp, dwrefreshrate, dwflags);
}

HRESULT WINAPI extSetDisplayMode1(LPDIRECTDRAW lpdd,
	DWORD dwwidth, DWORD dwheight, DWORD dwbpp)
{
	return extSetDisplayMode(1, lpdd, dwwidth, dwheight, dwbpp, 0, 0);
}

HRESULT WINAPI extGetDisplayMode(LPDIRECTDRAW lpdd, LPDDSURFACEDESC lpddsd)
{
	OutTraceDDRAW("GetDisplayMode: lpdd=%x lpddsd=%x\n", lpdd, lpddsd);

	(*pGetDisplayMode)(lpdd, lpddsd);
	if(dxw.dwFlags1 & EMULATESURFACE) {
		GetPixFmt((LPDDSURFACEDESC2)lpddsd);
		if(!lpddsd->ddpfPixelFormat.dwFlags) SetPixFmt((LPDDSURFACEDESC2)lpddsd);
	}

	if(dxw.Windowize){
		lpddsd->dwWidth = dxw.GetScreenWidth();
		lpddsd->dwHeight = dxw.GetScreenHeight();

		// v2.1.96: fake screen color depth
		if(dxw.dwFlags2 & (INIT8BPP|INIT16BPP)){ // v2.02.32 fix
			if(dxw.dwFlags2 & INIT8BPP) FixPixelFormat(8, &lpddsd->ddpfPixelFormat);
			if(dxw.dwFlags2 & INIT16BPP) FixPixelFormat(16, &lpddsd->ddpfPixelFormat);
			OutTraceDW("GetDisplayMode: fix RGBBitCount=%d\n", lpddsd->ddpfPixelFormat.dwRGBBitCount);
		}
	}

	//OutTraceDW("GetDisplayMode: returning WxH=(%dx%d) PixelFormat Flags=%x(%s) RGBBitCount=%d RGBAmask=(%x,%x,%x,%x) Caps=%x(%s)\n",
	//	lpddsd->dwWidth, lpddsd->dwHeight, 
	//	lpddsd->ddpfPixelFormat.dwFlags, ExplainPixelFormatFlags(lpddsd->ddpfPixelFormat.dwFlags),
	//	lpddsd->ddpfPixelFormat.dwRGBBitCount,
	//	lpddsd->ddpfPixelFormat.dwRBitMask, lpddsd->ddpfPixelFormat.dwGBitMask, lpddsd->ddpfPixelFormat.dwBBitMask,
	//	lpddsd->ddpfPixelFormat.dwRGBAlphaBitMask,
	//	lpddsd->ddsCaps.dwCaps, ExplainDDSCaps(lpddsd->ddsCaps.dwCaps));

	OutTraceDDRAW("GetDisplayMode: returning size=(%dx%d) %s\n", lpddsd->dwWidth, lpddsd->dwHeight, DumpPixelFormat((LPDDSURFACEDESC2)lpddsd));

	return 0;
}


HRESULT WINAPI extSetCooperativeLevel(void *lpdd, HWND hwnd, DWORD dwflags)
{
	HRESULT res;

	OutTraceDDRAW("SetCooperativeLevel: lpdd=%x hwnd=%x dwFlags=%x(%s)\n",
		lpdd, hwnd, dwflags,ExplainCoopFlags(dwflags));

	InitDDScreenParameters((LPDIRECTDRAW)lpdd);

	if(dxw.Windowize){
		if (dwflags & DDSCL_FULLSCREEN){
			// v2.01.82 fix:
			// WARN: Tomb Raider 4 demo is setting cooperative level against hwnd 0 (desktop)
			// so in this case better use the registered hWnd value. Same as GP500, who uses 
			// the desktop window handle.
			// v2.02.31 fix:
			// Hooligans - Storm over Europe wants to set cooperative level NORMAL to hwnd 0
			// that is legitimate, but setting against desktop window gives an error code - so
			// the zero hwnd redirection had to be moved within the FULLSCREEN if case.
			if(dxw.IsRealDesktop(hwnd)){
				OutTraceDW("SetCooperativeLevel: desktop hwnd=%x -> %x\n", hwnd, dxw.GethWnd());
				hwnd=dxw.GethWnd();
			}
			dxw.SetFullScreen(TRUE);
			dwflags &= ~(DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWMODEX);
			dwflags |= DDSCL_NORMAL;
			res=(*pSetCooperativeLevel)(lpdd, hwnd, dwflags);
			AdjustWindowFrame(hwnd, dxw.GetScreenWidth(), dxw.GetScreenHeight());
			if (dxw.dwFlags1 & FIXWINFRAME) dxw.FixWindowFrame(hwnd);
		}
		else{
			RECT client;
			(*pGetClientRect)(hwnd, &client);
			// v2.02.11:
			// Non fullscreen cooperative mode means windowed, unless the window occupies the whole desktop area
			dxw.SetFullScreen(client.right==dxw.iSizX && client.bottom==dxw.iSizY);
			//dxw.SetFullScreen(FALSE);
			res=(*pSetCooperativeLevel)(lpdd, hwnd, dwflags);
		}
	}
	else {
		res=(*pSetCooperativeLevel)(lpdd, hwnd, dwflags);
	}

	if(res)
		OutTraceE("SetCooperativeLevel: ERROR lpdd=%x hwnd=%x Flags=%x err=%x(%s) at %d\n", 
			lpdd, hwnd, dwflags, res, ExplainDDError(res), __LINE__);

	GetHookInfo()->IsFullScreen=dxw.IsFullScreen();

	// WARN: GP500 was setting cooperative level against the desktop! This can be partially
	// intercepted by hooking the GetDesktopWindow() call, but in windowed mode this can't be 
	// done, so better repeat the check here.

	if ((res==DD_OK) && (hwnd!=NULL)){
		if (hwnd==(*pGetDesktopWindow)()){
			OutTraceE("SetCooperativeLevel: attempt to work on desktop window\n");
		}
		else
			dxw.SethWnd(hwnd); // save the good one	
	}

	return res;
}

static void FixSurfaceCaps(LPDDSURFACEDESC2 lpddsd, int dxversion)
{	
	// rules of thumb:
	// 1) textures should be left untouched (switching to SYSTEMMEMORY when forcing HEL may even fail!)
	// 1.1) textures with no DDSD_PIXELFORMAT specification should have one when virtual color depth is not equal to real color depth?
	// 2) if a pixel format is specified, if DDSCAPS_SYSTEMMEMORY add DDSCAPS_OFFSCREENPLAY (if pixel formats are different?), otherwise do not touch anything.
	// 3) if the surface is used as a buffer (DDSD_WIDTH set, DDSD_HEIGHT unset) do not touch anything.
	// 4) zbuffer surfaces (DDSCAPS_ZBUFFER set) must have DDSCAPS_SYSTEMMEMORY 
	// 5) DDSCAPS_3DDEVICE surfaces should have a defined pixel format
	// 6) in all remaining cases, adjust the pixel format and ensure you have DDSCAPS_SYSTEMMEMORY|DDSCAPS_OFFSCREENPLAIN

	if(!(lpddsd->dwFlags & DDSD_CAPS)) lpddsd->ddsCaps.dwCaps = 0;

	OutTraceDW("FixSurfaceCaps: Flags=%x(%s) Caps=%x(%s)\n",
		lpddsd->dwFlags, ExplainFlags(lpddsd->dwFlags), lpddsd->ddsCaps.dwCaps, ExplainDDSCaps(lpddsd->ddsCaps.dwCaps));

	// DDSCAPS_TEXTURE surfaces must be left untouched, unless you set FORCESHEL: in this case switch VIDEOMEMORY to SYSTEMMEMORY
	if((lpddsd->dwFlags & DDSD_CAPS) && (lpddsd->ddsCaps.dwCaps & DDSCAPS_TEXTURE)){
		if (dxw.dwFlags3 & FORCESHEL) {
			lpddsd->ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
			lpddsd->ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		}
		if((dxw.dwFlags5 & TEXTUREFORMAT) && !(lpddsd->dwFlags & DDSD_PIXELFORMAT)){
			// TEXTURE: enforce PIXELFORMAT on MEMORY
			lpddsd->dwFlags |= DDSD_PIXELFORMAT;
			GetPixFmt(lpddsd); 
		}
		return;
	}

	// this is valid just in case the above block eliminated TEXTURE surfaces....
	if (lpddsd->dwFlags & DDSD_PIXELFORMAT){ // pixel format defined
		if (lpddsd->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) lpddsd->ddsCaps.dwCaps |= DDSCAPS_OFFSCREENPLAIN; // to allow for pixel format conversion (Quest for Glory 5 - GOG version)
		return;
	}

	if ((lpddsd->dwFlags & (DDSD_WIDTH|DDSD_HEIGHT)) == DDSD_WIDTH) { // buffer surface
		return;
	}

	if((lpddsd->dwFlags & DDSD_CAPS) && (lpddsd->ddsCaps.dwCaps & DDSCAPS_ZBUFFER)) { // z-buffer surface - set to memory
		lpddsd->ddsCaps.dwCaps = (DDSCAPS_SYSTEMMEMORY|DDSCAPS_ZBUFFER); 
		return;
	}

	if((lpddsd->dwFlags & DDSD_CAPS) && 
		(lpddsd->ddsCaps.dwCaps & DDSCAPS_3DDEVICE)) // v2.02.90: added for "Zoo Tycoon" textures
		{ // 3DDEVICE no TEXTURE: enforce PIXELFORMAT on MEMORY
		lpddsd->dwFlags |= DDSD_PIXELFORMAT;
		lpddsd->ddsCaps.dwCaps = (DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY|DDSCAPS_3DDEVICE); 
		GetPixFmt(lpddsd);
		return;
	}

	// default case: adjust pixel format
	OutTraceB("FixSurfaceCaps: suppress DDSCAPS_VIDEOMEMORY case\n");
	lpddsd->dwFlags |= (DDSD_CAPS|DDSD_PIXELFORMAT); 
	lpddsd->ddsCaps.dwCaps &= ~(DDSCAPS_VIDEOMEMORY|DDSCAPS_LOCALVIDMEM); // v2.02.43
	lpddsd->ddsCaps.dwCaps |= (DDSCAPS_SYSTEMMEMORY|DDSCAPS_OFFSCREENPLAIN);
	if(!(dxw.dwFlags3 & NOPIXELFORMAT)) GetPixFmt(lpddsd);
	return;
}

static void ClearSurfaceDesc(void *ddsd, int dxversion)
{
	int size;
	size = (dxversion < 4) ? sizeof(DDSURFACEDESC) : sizeof(DDSURFACEDESC2);
	memset(ddsd, 0, size); // Clean all
	((LPDDSURFACEDESC)ddsd)->dwSize = size;
}

static HRESULT BuildPrimaryEmu(LPDIRECTDRAW lpdd, CreateSurface_Type pCreateSurface, LPDDSURFACEDESC2 lpddsd, int dxversion, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	DDSURFACEDESC2 ddsd;
	HRESULT res;

	// emulated primary surface
	memcpy((void *)&ddsd, lpddsd, lpddsd->dwSize);

	// handle the surface attributes before the ddsd.dwFlags gets updated:
	// if a surface desc is NOT specified, build one
	if(!(ddsd.dwFlags & DDSD_PIXELFORMAT)) SetPixFmt((LPDDSURFACEDESC2)&ddsd);
	// then save it
	dxw.VirtualPixelFormat = ddsd.ddpfPixelFormat;

	OutTraceDW("DDSD_PIXELFORMAT: color=%d flags=%x\n", dxw.VirtualPixelFormat.dwRGBBitCount, dxw.VirtualPixelFormat.dwFlags);
	ddsd.dwFlags &= ~(DDSD_BACKBUFFERCOUNT|DDSD_REFRESHRATE);
	ddsd.dwFlags |= (DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT);
	ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_PRIMARYSURFACE|DDSCAPS_FLIP|DDSCAPS_COMPLEX|DDSCAPS_VIDEOMEMORY|DDSCAPS_LOCALVIDMEM);
	// DDSCAPS_OFFSCREENPLAIN seems required to support the palette in memory surfaces
	ddsd.ddsCaps.dwCaps |= (DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY);
	// on WinXP Fifa 99 doesn't like DDSCAPS_SYSTEMMEMORY cap, so better to leave a way to unset it....
	if(dxw.dwFlags5 & NOSYSTEMEMULATED) ddsd.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;
	ddsd.dwWidth = dxw.GetScreenWidth();
	ddsd.dwHeight = dxw.GetScreenHeight();

	// create Primary surface
	OutTraceDW("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[Primary]" , __LINE__));
	res=(*pCreateSurface)(lpdd, &ddsd, lplpdds, 0);
	if(res){
		if (res==DDERR_PRIMARYSURFACEALREADYEXISTS){
			LPDIRECTDRAWSURFACE lpPrim;
			OutTraceE("CreateSurface: CreateSurface DDERR_PRIMARYSURFACEALREADYEXISTS workaround\n");
			(*pGetGDISurface)(lpPrimaryDD, &lpPrim);
			while ((*pReleaseS)(lpPrim));
			res = (*pCreateSurface)(lpdd, &ddsd, lplpdds, 0);
		}
		/* fall through */
		if(res){
			OutTraceE("CreateSurface: ERROR on DDSPrim res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			if(res==DDERR_INVALIDPIXELFORMAT) DumpPixFmt(&ddsd);
			return res;
		}
	}

	OutTraceDW("CreateSurface: created PRIMARY DDSPrim=%x\n", *lplpdds);
    if(IsDebug) DescribeSurface(*lplpdds, dxversion, "DDSPrim", __LINE__);
	HookDDSurfacePrim(lplpdds, dxversion);
	// "Hoyle Casino Empire" opens a primary surface and NOT a backbuffer ....
	iBakBufferVersion=dxversion; // v2.03.01

	// set a global capability value for surfaces that have to blit to primary
	// DDSCAPS_OFFSCREENPLAIN seems required to support the palette in memory surfaces
	// DDSCAPS_SYSTEMMEMORY makes operations faster, but it is not always good...
	dwBackBufferCaps = (DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY);
	// on WinXP Fifa 99 doesn't like DDSCAPS_SYSTEMMEMORY cap, so better to leave a way to unset it....
	if(dxw.dwFlags5 & NOSYSTEMEMULATED) dwBackBufferCaps &= ~DDSCAPS_SYSTEMMEMORY;
	if(dxw.dwFlags5 & GSKYHACK) dwBackBufferCaps = (DDSCAPS_OFFSCREENPLAIN|DDSCAPS_VIDEOMEMORY|DDSCAPS_LOCALVIDMEM);

	if(dxw.dwFlags5 & GDIMODE) return DD_OK;

	if(lpDDSEmu_Prim==NULL){
		ClearSurfaceDesc((void *)&ddsd, dxversion);
		ddsd.dwFlags = DDSD_CAPS; 
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		OutTraceDW("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[EmuPrim]", __LINE__));
		res=(*pCreateSurface)(lpdd, &ddsd, &lpDDSEmu_Prim, 0);
		if(res==DDERR_PRIMARYSURFACEALREADYEXISTS){
			OutTraceDW("CreateSurface: ASSERT DDSEmu_Prim already exists\n");
			if(dxw.Windowize){
				// in Winowize mode, the desktop properties are untouched, then the current primary surface can be recycled
				res=(*pGetGDISurface)(lpdd, &lpDDSEmu_Prim); 
				(*pReleaseS)(lpDDSEmu_Prim);
			}
			else {
				// in non-Windowized mode, the primary surface must be released and rebuilt with the proper properties
				res=(*pGetGDISurface)(lpdd, &lpDDSEmu_Prim); 
				if (lpDDSEmu_Prim) while((*pReleaseS)(lpDDSEmu_Prim));
				res=(*pCreateSurface)(lpdd, &ddsd, &lpDDSEmu_Prim, 0);
			}
		}
		if(res){
			OutTraceE("CreateSurface: ERROR on DDSEmu_Prim res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			if(res==DDERR_INVALIDPIXELFORMAT) DumpPixFmt(&ddsd);
			return res;
		}
		OutTraceDW("CreateSurface: created new DDSEmu_Prim=%x\n",lpDDSEmu_Prim);
		if(IsDebug) DescribeSurface(lpDDSEmu_Prim, dxversion, "DDSEmu_Prim", __LINE__);
		InitDSScreenParameters(lpDDSEmu_Prim);
		dxw.MarkRegularSurface(lpDDSEmu_Prim);
		// can't hook lpDDSEmu_Prim as generic, since the Flip method is unimplemented for a PRIMARY surface!
		// better avoid it or hook just useful methods.
		//if (dxw.dwTFlags & OUTPROXYTRACE) HookDDSurfaceGeneric(&lpDDSEmu_Prim, dxw.dwDDVersion);
	}

	if(lpDDSEmu_Back==NULL){
		ClearSurfaceDesc((void *)&ddsd, dxversion);
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		ddsd.ddsCaps.dwCaps = dwBackBufferCaps;
		ddsd.dwWidth = dxw.GetScreenWidth();
		ddsd.dwHeight = dxw.GetScreenHeight();
		if(dxw.dwFlags4 & BILINEAR2XFILTER){
			// double backbuffer size
			ddsd.dwWidth = dxw.GetScreenWidth() << 1;
			ddsd.dwHeight = dxw.GetScreenHeight() << 1;
		}

		OutTraceDW("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[EmuBack]", __LINE__));

		res=(*pCreateSurface)(lpdd, &ddsd, &lpDDSEmu_Back, 0);
		if(res){
			OutTraceE("CreateSurface: CreateSurface ERROR on DDSEmuBack : res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			if(res==DDERR_INVALIDPIXELFORMAT) DumpPixFmt(&ddsd);
			return res;
		}
		OutTraceDW("CreateSurface: created new DDSEmu_Back=%x\n", lpDDSEmu_Back);
		if(IsDebug) DescribeSurface(lpDDSEmu_Back, dxversion, "DDSEmu_Back", __LINE__);
		dxw.MarkRegularSurface(lpDDSEmu_Back);
		if (dxw.dwTFlags & OUTPROXYTRACE) HookDDSurfaceGeneric(&lpDDSEmu_Back, dxversion);
	}

	return DD_OK;
}

static HRESULT BuildPrimaryDir(LPDIRECTDRAW lpdd, CreateSurface_Type pCreateSurface, LPDDSURFACEDESC2 lpddsd, int dxversion, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	DDSURFACEDESC2 ddsd;
	HRESULT res;

	// genuine primary surface
	memcpy((void *)&ddsd, lpddsd, lpddsd->dwSize);
	ddsd.dwFlags &= ~(DDSD_WIDTH|DDSD_HEIGHT|DDSD_BACKBUFFERCOUNT|DDSD_REFRESHRATE|DDSD_PIXELFORMAT);
	ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_FLIP|DDSCAPS_COMPLEX);
	// v2.02.93: don't move primary / backbuf surfaces on systemmemory when 3DDEVICE is requested
	// this impact also on capabilities for temporary surfaces for AERO optimized handling
	if ((lpddsd->dwFlags & DDSD_CAPS) && (lpddsd->ddsCaps.dwCaps & DDSCAPS_3DDEVICE)) ddsd.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;

	// create Primary surface
	OutTraceDW("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[Primary]", __LINE__));
	res=(*pCreateSurface)(lpdd, &ddsd, lplpdds, 0);
	if(res){
		if (res==DDERR_PRIMARYSURFACEALREADYEXISTS){
			LPDIRECTDRAWSURFACE lpPrim;
			OutTraceE("CreateSurface: CreateSurface DDERR_PRIMARYSURFACEALREADYEXISTS workaround\n");
			(*pGetGDISurface)(lpPrimaryDD, &lpPrim);
			while ((*pReleaseS)(lpPrim));
			res = (*pCreateSurface)(lpdd, &ddsd, lplpdds, 0);
		}
		/* fall through */
		if(res){
			OutTraceE("CreateSurface: ERROR on DDSPrim res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			if(res==DDERR_INVALIDPIXELFORMAT) DumpPixFmt(&ddsd);
			return res;
		}
	}

	OutTraceDW("CreateSurface: created PRIMARY DDSPrim=%x\n", *lplpdds);
    if(IsDebug) DescribeSurface(*lplpdds, dxversion, "DDSPrim", __LINE__);

	if(dxw.dwFlags1 & EMULATEBUFFER){
		lpDDSEmu_Prim = *lplpdds;
		dxw.MarkRegularSurface(lpDDSEmu_Prim);

		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		// warning: can't create zero sized backbuffer surface !!!!
		ddsd.dwWidth = dxw.GetScreenWidth();
		ddsd.dwHeight = dxw.GetScreenHeight();
		ddsd.ddsCaps.dwCaps = 0;
		if (dxversion >= 4) ddsd.ddsCaps.dwCaps |= DDSCAPS_OFFSCREENPLAIN;
		OutTraceDW("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[Dir FixBuf]", __LINE__));
		res=(*pCreateSurface)(lpdd, &ddsd, lplpdds, 0);
		if(res){
			OutTraceE("CreateSurface: ERROR on DDSPrim res=%x(%s) at %d\n",res, ExplainDDError(res), __LINE__);
			return res;
		}
		OutTraceDW("CreateSurface: created FIX DDSPrim=%x\n", *lplpdds);
		if(IsDebug) DescribeSurface(*lplpdds, dxversion, "DDSPrim(2)", __LINE__);
	}

	HookDDSurfacePrim(lplpdds, dxversion);
	if(dxw.dwFlags1 & CLIPCURSOR) dxw.SetClipCursor();

	return DD_OK;
}

static HRESULT BuildBackBufferEmu(LPDIRECTDRAW lpdd, CreateSurface_Type pCreateSurface, LPDDSURFACEDESC2 lpddsd, int dxversion, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	DDSURFACEDESC2 ddsd;
	HRESULT res;

	// create BackBuffer surface
	memcpy(&ddsd, lpddsd, lpddsd->dwSize);
	ddsd.dwFlags &= ~(DDSD_BACKBUFFERCOUNT|DDSD_REFRESHRATE);
	ddsd.dwFlags |= (DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT);
	ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_BACKBUFFER|DDSCAPS_PRIMARYSURFACE|DDSCAPS_FLIP|DDSCAPS_COMPLEX|DDSCAPS_VIDEOMEMORY|DDSCAPS_LOCALVIDMEM);
	// DDSCAPS_OFFSCREENPLAIN seems required to support the palette in memory surfaces
	ddsd.ddsCaps.dwCaps |= (DDSCAPS_SYSTEMMEMORY|DDSCAPS_OFFSCREENPLAIN);
	if(ddsd.ddsCaps.dwCaps & DDSCAPS_3DDEVICE) ddsd.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY; // necessary: Martian Gotic crashes otherwise
	// on WinXP Fifa 99 doesn't like DDSCAPS_SYSTEMMEMORY cap, so better to leave a way to unset it....
	if(dxw.dwFlags5 & NOSYSTEMMEMORY) ddsd.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;
	ddsd.dwWidth = dxw.GetScreenWidth();
	ddsd.dwHeight = dxw.GetScreenHeight();
	GetPixFmt(&ddsd);

	OutTraceDW("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[Backbuf]", __LINE__));
	res=(*pCreateSurface)(lpdd, &ddsd, lplpdds, 0);
	if(res) {
		OutTraceE("CreateSurface ERROR: res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		if(res==DDERR_INVALIDPIXELFORMAT) DumpPixFmt(&ddsd);
		return res;
	}

	OutTraceDW("CreateSurface: created BACK DDSBack=%x\n", *lplpdds);
    if(IsDebug) DescribeSurface(*lplpdds, dxversion, "DDSBack", __LINE__);
	HookDDSurfaceGeneric(lplpdds, dxversion); // added !!!
	lpBackBufferDD = lpdd; // v2.02.31
	iBakBufferVersion=dxversion; // v2.02.31

	return DD_OK;
}

static HRESULT BuildBackBufferDir(LPDIRECTDRAW lpdd, CreateSurface_Type pCreateSurface, LPDDSURFACEDESC2 lpddsd, int dxversion, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	DDSURFACEDESC2 ddsd;
	HRESULT res;

	// create BackBuffer surface
	// ClearSurfaceDesc((void *)&ddsd, dxversion);
	memcpy(&ddsd, lpddsd, lpddsd->dwSize);
	ddsd.dwFlags &= ~(DDSD_WIDTH|DDSD_HEIGHT|DDSD_BACKBUFFERCOUNT|DDSD_REFRESHRATE|DDSD_PIXELFORMAT);
	ddsd.dwFlags |= (DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH);
	ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_PRIMARYSURFACE|DDSCAPS_FLIP|DDSCAPS_COMPLEX);
	// v2.02.93: don't move primary / backbuf surfaces on systemmemory when 3DDEVICE is requested
	if(lpddsd->ddsCaps.dwCaps & DDSCAPS_3DDEVICE) 
		ddsd.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;
	else
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY; 
	if (dxversion >= 4) ddsd.ddsCaps.dwCaps |= DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwWidth = dxw.GetScreenWidth();
	ddsd.dwHeight = dxw.GetScreenHeight();
	if (dxw.dwFlags2 & BACKBUFATTACH) {
		LPDIRECTDRAWSURFACE lpPrim;
		DDSURFACEDESC2 prim;
		(*pGetGDISurface)(lpPrimaryDD, &lpPrim);
		memset(&prim, 0, sizeof(DDSURFACEDESC2));
		prim.dwSize = (dxversion >= 4) ? sizeof(DDSURFACEDESC2) : sizeof(DDSURFACEDESC);
		res=lpPrim->GetSurfaceDesc((DDSURFACEDESC *)&prim);
		(*pReleaseS)(lpPrim);
		ddsd.dwWidth = prim.dwWidth;
		ddsd.dwHeight = prim.dwHeight;
		OutTraceDW("BMX FIX: res=%x(%s) wxh=(%dx%d)\n", res, ExplainDDError(res),ddsd.dwWidth, ddsd.dwHeight);
	}
	OutTraceDW("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[Backbuf]", __LINE__));
	res=(*pCreateSurface)(lpdd, &ddsd, lplpdds, 0);
	if(res) {
		if ((dxw.dwFlags1 & SWITCHVIDEOMEMORY) && (res==DDERR_OUTOFVIDEOMEMORY)){
			OutTraceDW("CreateSurface: CreateSurface DDERR_OUTOFVIDEOMEMORY ERROR at %d, retry in SYSTEMMEMORY\n", __LINE__);
			ddsd.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY; 
			ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY; 
			res=(*pCreateSurface)(lpdd, &ddsd, lplpdds, 0);
		}
		if(res){
			OutTraceE("CreateSurface ERROR: res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			return res;
		}
	}

	OutTraceDW("CreateSurface: created BACK DDSBack=%x\n", *lplpdds);
    if(IsDebug) DescribeSurface(*lplpdds, dxversion, "DDSBack", __LINE__);
	HookDDSurfaceGeneric(lplpdds, dxversion); // added !!!
	lpBackBufferDD = lpdd; // v2.02.31
	iBakBufferVersion=dxversion; // v2.02.31

	return DD_OK;
}		

static HRESULT BuildGenericEmu(LPDIRECTDRAW lpdd, CreateSurface_Type pCreateSurface, LPDDSURFACEDESC2 lpddsd, int dxversion, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	DDSURFACEDESC2 ddsd;
	HRESULT res;

	memcpy(&ddsd, lpddsd, lpddsd->dwSize); // Copy over ....
	FixSurfaceCaps(&ddsd, dxversion);
	// It looks that DDSCAPS_SYSTEMMEMORY surfaces can perfectly be DDSCAPS_3DDEVICE as well. 
	// For "Risk II" it is necessary that both the primary surface and the offscreen surfaces are generated
	// with the same type, so that assuming an identical lPitch and memcopy-ing from one buffer to the 
	// other is a legitimate operation. 
	if(ddsd.ddsCaps.dwCaps & DDSCAPS_ZBUFFER) ddsd.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY; // v2.03.03 !!
	// on WinXP Fifa 99 doesn't like DDSCAPS_SYSTEMMEMORY cap, so better to leave a way to unset it....
	if(dxw.dwFlags5 & NOSYSTEMMEMORY) ddsd.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;

	res=(*pCreateSurface)(lpdd, &ddsd, lplpdds, pu);
	if ((dxw.dwFlags1 & SWITCHVIDEOMEMORY) && (res!=DD_OK)){
		OutTraceDW("CreateSurface ERROR: res=%x(%s) at %d, retry\n", res, ExplainDDError(res), __LINE__);
		ddsd.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		res=(*pCreateSurface)(lpdd, &ddsd, lplpdds, pu);
	}
	if (res) {
		OutTraceE("CreateSurface: ERROR on Emu_Generic res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	OutTraceDW("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[Emu Generic]", __LINE__));
		
	OutTraceDW("CreateSurface: created Emu_Generic dds=%x\n", *lplpdds);
	if(IsDebug) DescribeSurface(*lplpdds, dxversion, "DDSEmu_Generic", __LINE__);

	// v2.02.66: if 8BPP paletized surface and a primary palette exixts, apply.
	// fixes "Virtua Fighter PC" palette bug
	if(lpDDP && (ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)){
		res=(*pSetPalette)(*lplpdds, lpDDP);
		if(res)
			OutTraceE("SetPalette: ERROR on lpdds=%x(Emu_Generic) res=%x(%s) at %d\n", *lplpdds, res, ExplainDDError(res), __LINE__);
		else
			OutTraceDW("CreateSurface: applied lpddp=%x to lpdds=%x\n", lpDDP, *lplpdds);
	}

	// diagnostic hooks ....
	HookDDSurfaceGeneric(lplpdds, dxversion);

	return DD_OK;
}

static HRESULT BuildGenericDir(LPDIRECTDRAW lpdd, CreateSurface_Type pCreateSurface, LPDDSURFACEDESC2 lpddsd, int dxversion, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	HRESULT res;

	OutTraceDW("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)lpddsd, "[Dir Generic]", __LINE__));

	res = (*pCreateSurface)(lpdd, lpddsd, lplpdds, 0); 
	if(res){
		// v2.02.60: Ref. game Incoming GOG release, post by Marek, error DDERR_UNSUPPORTED while trying to create ZBUFFER surface 
		if ((dxw.dwFlags1 & SWITCHVIDEOMEMORY) && ((res==DDERR_OUTOFVIDEOMEMORY)||(res==DDERR_UNSUPPORTED))){
			OutTraceDW("CreateSurface ERROR: res=%x(%s) at %d, retry\n", res, ExplainDDError(res), __LINE__);
			lpddsd->ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
			lpddsd->ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
			res = (*pCreateSurface)(lpdd, lpddsd, lplpdds, 0); 
		}
		if(res){
			OutTraceE("CreateSurface: CreateSurface ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			return res;
		}
	}
	OutTraceDW("CreateSurface: created GENERIC surface dds=%x flags=%x(%s) caps=%x(%s)\n", 
		*lplpdds, lpddsd->dwFlags, ExplainFlags(lpddsd->dwFlags), lpddsd->ddsCaps.dwCaps, ExplainDDSCaps(lpddsd->ddsCaps.dwCaps));

	// hooks ....
	HookDDSurfaceGeneric(lplpdds, dxversion);

	OutTraceDW("CreateSurface: created lpdds=%x type=Generic ret=%x\n", *lplpdds, res);
	if(IsDebug) DescribeSurface(*lplpdds, dxversion, "Generic", __LINE__); //v2.02.44 bug fix

	return DD_OK;
}

static HRESULT WINAPI extCreateSurface(int dxversion, CreateSurface_Type pCreateSurface, LPDIRECTDRAW lpdd, DDSURFACEDESC2 *lpddsd,
	LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	HRESULT res;
	DDSURFACEDESC2 ddsd;
	LPDIRECTDRAWSURFACE lpDDSPrim;
	DWORD CurFlags;
	int TargetSize;
	typedef HRESULT (*BuildSurface_Type)(LPDIRECTDRAW, CreateSurface_Type, LPDDSURFACEDESC2, int, LPDIRECTDRAWSURFACE *, void *);
	BuildSurface_Type BuildPrimary;
	BuildSurface_Type BuildBackBuffer;
	BuildSurface_Type BuildGeneric;

	if (dxw.dwFlags1 & EMULATESURFACE){
		BuildPrimary = BuildPrimaryEmu;
		BuildBackBuffer = BuildBackBufferEmu;
		BuildGeneric = BuildGenericEmu;
	}
	else {
		BuildPrimary = BuildPrimaryDir;
		BuildBackBuffer = BuildBackBufferDir;
		BuildGeneric = BuildGenericDir;
	}

	OutTraceDDRAW("CreateSurface: Version=%d lpdd=%x %s\n", 
		dxversion, lpdd, LogSurfaceAttributes((LPDDSURFACEDESC)lpddsd, "[CreateSurface]", __LINE__));

	// check for lpddsd->dwSize value
	TargetSize=(dxversion<4)?sizeof(DDSURFACEDESC):sizeof(DDSURFACEDESC2);
	if(lpddsd->dwSize != TargetSize){
		char sMsg[81];
		sprintf_s(sMsg,80, "CreateSurface: ASSERT bad dwSize=%d dxversion=%d\n", 
			lpddsd->dwSize, dxversion);
		OutTraceDW(sMsg);
		if(IsAssertEnabled) MessageBox(0, sMsg, "CreateSurface", MB_OK | MB_ICONEXCLAMATION);
		return DDERR_INVALIDPARAMS;
	}

	//GHO workaround (needed for WarWind, Rogue Spear):
	if (lpddsd->dwFlags && !(lpddsd->dwFlags & 0x1)){
		OutTraceDW("CreateSurface: fixing illegal dwFlags value: %x -> %x\n",
			lpddsd->dwFlags, lpddsd->dwFlags+1);
		lpddsd->dwFlags++;
	}

	memcpy(&ddsd, lpddsd, lpddsd->dwSize); // Copy

	// v2.02.38: this is odd: in "Star Force Deluxe" there is no PRIMARY surface, but a surface with 
	// 0 flags and 0 capabilities serves for this purpose. Is it a side-effect of old ddraw releases?
	if((dxversion == 1) && (ddsd.dwFlags == 0)){ // Star Force Deluxe
		ddsd.dwFlags = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		//if(dxw.VirtualPixelFormat.dwRGBBitCount == 8) ddsd.ddsCaps.dwCaps |= DDSCAPS_PALETTE;
	}

	// creation of the primary surface....
	if(ddsd.dwFlags & DDSD_CAPS && ddsd.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE){
		SetVSyncDelays(lpdd);
		GetHookInfo()->Height=(short)dxw.GetScreenHeight();
		GetHookInfo()->Width=(short)dxw.GetScreenWidth();
		GetHookInfo()->ColorDepth=(short)dxw.VirtualPixelFormat.dwRGBBitCount;
		GetHookInfo()->DXVersion=dxversion;
		lpPrimaryDD = lpdd; // v2.1.87
		memcpy(&DDSD_Prim, lpddsd, sizeof(DDSD_Prim)); // v2.02.37

		// beware of the different behaviour between older and newer directdraw releases...
		if(dxversion >= 4){
			if (lpDDSEmu_Back) while(lpDDSEmu_Back->Release());
			if (lpDDSEmu_Prim) while(lpDDSEmu_Prim->Release());
			if (ddsd.dwFlags & DDSD_BACKBUFFERCOUNT) { // Praetorians !!!!
				if (lpDDSBack) while(lpDDSBack->Release());
				lpBackBufferDD = NULL;
			}
		}
		lpDDSEmu_Back=NULL;
		lpDDSEmu_Prim=NULL;

		int BBCount=0; // or 1 ??
		if (ddsd.dwFlags & DDSD_BACKBUFFERCOUNT) BBCount=ddsd.dwBackBufferCount;
		if ((BBCount > 0) && (iBakBufferVersion < 4)){ 
			lpDDSBack=NULL;
			OutTraceDW("CreateSurface: backbuffer cleared - BackBufferCount=%d\n", BBCount);
		}

		if (BBCount > MAXBACKBUFFERS){
			char sMsg[81];
			sprintf_s(sMsg, 80, "CreateSurface: BackBufferCount=%d\n", BBCount);
			OutTraceDW(sMsg);
			if (IsAssertEnabled) MessageBox(0, sMsg, "CreateSurface", MB_OK | MB_ICONEXCLAMATION);
			// recover ...
			BBCount = MAXBACKBUFFERS;
		}

		// build emulated primary surface, real primary and backbuffer surfaces
		CurFlags=ddsd.dwFlags;
		res=BuildPrimary(lpdd, pCreateSurface, lpddsd, dxversion, lplpdds, NULL);
		if(res) return res;
		lpDDSPrim = *lplpdds;
		dxw.MarkPrimarySurface(lpDDSPrim);
		RegisterPixelFormat(lpDDSPrim);

		if (BBCount){
			// build emulated backbuffer surface
			res=BuildBackBuffer(lpdd, pCreateSurface, lpddsd, dxversion, &lpDDSBack, NULL);
			if(res) return res;
			dxw.MarkBackBufferSurface(lpDDSBack);

			// V2.1.85/V2.2.34: tricky !!!!
			// When a real backbuffer is created, it has a reference to its frontbuffer.
			// some games (Monopoly 3D) may depend on this setting - i.e. they could close
			// the exceeding references - so this is better be replicated adding an initial
			// reference to the zero count. But you don't have to do this if the backbuffer
			// is created independently by the primary surface.
			lpDDSBack->AddRef(); // should it be repeated BBCount times????
		}

		if(IsTraceDDRAW){
			char sInfo[256+1];
			sprintf(sInfo, "CreateSurface: created DDSPrim=%x DDSBack=%x", lpDDSPrim, lpDDSBack);
			if(dxw.dwFlags1 & (EMULATESURFACE|EMULATEBUFFER)) sprintf(sInfo, "%s DDSEmu_Prim=%x", sInfo, lpDDSEmu_Prim);
			if(dxw.dwFlags1 & EMULATESURFACE) sprintf(sInfo, "%s DDSEmu_Back=%x", sInfo, lpDDSEmu_Back);
			strcat(sInfo, "\n");
			OutTrace(sInfo);
		}
 
		// rebuild the clipper area
		if(dxw.dwFlags1 & CLIPCURSOR) dxw.SetClipCursor();

		// v2.2.64: added extra ref needed to preserve ddraw session for later use. Is it a ddraw1 legacy?
		// seems to fix problems in "Warhammer 40K Rites Of War" that uses a ddraw session after reaching 0 refcount.
		// v2.2.84: avoid the extra referenced in non windowed mode since it causes the window shift reported by gsky916
		// for Wind Fantasy SP.
		if((dxw.dwDDVersion==1) && dxw.Windowize) lpdd->AddRef();

		return DD_OK;
	}

	// a request for a separate (not complex) backbuffer to attach later on, maybe.
	if ((ddsd.dwFlags & DDSD_CAPS) && (ddsd.ddsCaps.dwCaps & DDSCAPS_BACKBUFFER)){
		if (lpDDSBack) {
			OutTraceDW("CreateSurface: returning current DDSBack=%x\n", lpDDSBack);
			*lplpdds = lpDDSBack;
			return DD_OK;
		}

		res=BuildBackBuffer(lpdd, pCreateSurface, lpddsd, dxversion, lplpdds, NULL);
		lpDDSBack = *lplpdds;
		dxw.MarkBackBufferSurface(lpDDSBack);
		return res;
	}

	// if nothing else, it's a generic/zbuffer surface

	res=BuildGeneric(lpdd, pCreateSurface, lpddsd, dxversion, lplpdds, pu);
	if(!res) {
		dxw.MarkRegularSurface(*lplpdds);
		if(lpddsd->ddsCaps.dwCaps & DDSCAPS_ZBUFFER) {
			lpDDZBuffer=*lplpdds;
			DDZBufferCaps = lpddsd->ddsCaps.dwCaps;
			OutTraceDW("CreateSurface: lpDDZBuffer=%x save ZBUFFER caps=%x(%s)\n", lpDDZBuffer, DDZBufferCaps, ExplainDDSCaps(DDZBufferCaps));
		}
	}


	return res;
}


HRESULT WINAPI extCreateSurface1(LPDIRECTDRAW lpdd, DDSURFACEDESC *lpddsd, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	return extCreateSurface(1, (CreateSurface_Type)pCreateSurface1, lpdd, (DDSURFACEDESC2 *)lpddsd, lplpdds, pu);
}

HRESULT WINAPI extCreateSurface2(LPDIRECTDRAW lpdd, DDSURFACEDESC *lpddsd, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	return extCreateSurface(2, (CreateSurface_Type)pCreateSurface2, lpdd, (DDSURFACEDESC2 *)lpddsd, lplpdds, pu);
}

HRESULT WINAPI extCreateSurface4(LPDIRECTDRAW lpdd, DDSURFACEDESC2 *lpddsd, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	return extCreateSurface(4, (CreateSurface_Type)pCreateSurface4, lpdd, (DDSURFACEDESC2 *)lpddsd, lplpdds, pu);
}

HRESULT WINAPI extCreateSurface7(LPDIRECTDRAW lpdd, DDSURFACEDESC2 *lpddsd, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	return extCreateSurface(7, (CreateSurface_Type)pCreateSurface7, lpdd, (DDSURFACEDESC2 *)lpddsd, lplpdds, pu);
}


HRESULT WINAPI extGetAttachedSurface(int dxversion, GetAttachedSurface_Type pGetAttachedSurface,
	LPDIRECTDRAWSURFACE lpdds, LPDDSCAPS lpddsc, LPDIRECTDRAWSURFACE *lplpddas)
{
	HRESULT res;
	BOOL IsPrim;
	BOOL IsBack;

	IsPrim=dxw.IsAPrimarySurface(lpdds);
	IsBack=dxw.IsABackBufferSurface(lpdds);
	OutTraceDDRAW("GetAttachedSurface(%d): lpdds=%x%s caps=%x(%s)\n", 
		dxversion, lpdds, (IsPrim?"(PRIM)":(IsBack ? "(BACK)":"")), lpddsc->dwCaps, ExplainDDSCaps(lpddsc->dwCaps));

	// this is yet to be proven utility.....
	// v2.02.45: No - it prevents "Praetorians" to work!!! -> commented out.
	//
	//if (IsBack && (lpddsc->dwCaps & DDSCAPS_ZBUFFER) && lpDDZBuffer){ 
	//	*lplpddas = lpDDZBuffer;
	//	OutTraceDW("GetAttachedSurface(%d): emulating ZBUFFER attach on BACKBUFFER lpddsadd=%x\n", dxversion, *lplpddas); 
	//	return 0;
	//}

	// v2.1.81: fix to make "Silver" working: if the primary surface was created with 
	// backbuffercount == 2, the game expects some more surface to be attached to 
	// the attached backbuffer. Since there would be no use for it, just return
	// the attached backbuffer itself. Makes Silver working, anyway....
	// beware: "Snowboard Racer" fails if you return an attached surface anyhow! There,
	// the primary surface was created with back buffer count == 1.
	// v2.2.62 fix: a check to implement doublebuffer emulation only in case of DDSCAPS_BACKBUFFER
	// requests. A call to GetAttachedSurface can be made to retrieve DDSCAPS_ZBUFFER surfaces, and in 
	// this case the lpDDSBack surface can't be returned.

	if (IsBack && (DDSD_Prim.dwBackBufferCount > 1) && (lpddsc->dwCaps & DDSCAPS_BACKBUFFER)){ 
	//if (IsBack && (DDSD_Prim.dwBackBufferCount > 1)){ 
		*lplpddas = lpDDSBack;
		OutTraceDW("GetAttachedSurface(%d): DOUBLEBUFFER attached to BACK=%x\n", dxversion, *lplpddas); 
		return DD_OK;
	}

	// on primary surface return the lpDDSBack surface coming from either an explicit
	// AddAttachedSurface, or a primary complex surface creation otherwise....

	if(IsPrim && (lpddsc->dwCaps & (DDSCAPS_BACKBUFFER|DDSCAPS_FLIP))) { // v2.02.42 added DDSCAPS_FLIP for Empire Earth
		// in "Tomb Raider III" GOG release, the primary surface is queryed and has no attached
		// backbuffer, but a backbuffer does exist and has to be retrieved by GetBackBufferSurface.
		LPDIRECTDRAWSURFACE lpddsback = dxw.GetBackBufferSurface();
		if (lpddsback) {
			*lplpddas = lpddsback;
			OutTraceDW("GetAttachedSurface(%d): BACKBUFFER attached=%x\n", dxversion, *lplpddas); 
			return DD_OK;
		}
		else {
			*lplpddas = NULL;
			OutTraceDW("GetAttachedSurface(%d): no attached BACKBUFFER\n", dxversion); 
			return DDERR_NOTFOUND;
		}
	}

	// proxy the call...

	res=(*pGetAttachedSurface)(lpdds, lpddsc, lplpddas);
	if(res) 
		OutTraceE("GetAttachedSurface(%d): ERROR res=%x(%s) at %d\n", dxversion, res, ExplainDDError(res), __LINE__);
	else
		OutTraceDDRAW("GetAttachedSurface(%d): attached=%x\n", dxversion, *lplpddas); 

	return res;
	
}

HRESULT WINAPI extGetAttachedSurface1(LPDIRECTDRAWSURFACE lpdds, LPDDSCAPS lpddsc, LPDIRECTDRAWSURFACE *lplpddas)
{
	return extGetAttachedSurface(1, pGetAttachedSurface1, lpdds, lpddsc, lplpddas);
}

HRESULT WINAPI extGetAttachedSurface3(LPDIRECTDRAWSURFACE lpdds, LPDDSCAPS lpddsc, LPDIRECTDRAWSURFACE *lplpddas)
{
	return extGetAttachedSurface(3, pGetAttachedSurface3, lpdds, lpddsc, lplpddas);
}

HRESULT WINAPI extGetAttachedSurface4(LPDIRECTDRAWSURFACE lpdds, LPDDSCAPS lpddsc, LPDIRECTDRAWSURFACE *lplpddas)
{
	return extGetAttachedSurface(4, pGetAttachedSurface4, lpdds, lpddsc, lplpddas);
}

HRESULT WINAPI extGetAttachedSurface7(LPDIRECTDRAWSURFACE lpdds, LPDDSCAPS lpddsc, LPDIRECTDRAWSURFACE *lplpddas)
{
	return extGetAttachedSurface(7, pGetAttachedSurface7, lpdds, lpddsc, lplpddas);
}

void BlitError(HRESULT res, LPRECT lps, LPRECT lpd, int line)
{
	char sInfo[512];
	sprintf(sInfo, "Blt: ERROR %x(%s) at %d", res, ExplainDDError(res), line);
	if (res==DDERR_INVALIDRECT){
		if (lps)
			sprintf(sInfo, "%s src=(%d,%d)-(%d,%d)", sInfo, lps->left, lps->top, lps->right, lps->bottom);
		else
			sprintf(sInfo, "%s src=(NULL)", sInfo);
		if (lpd)
			sprintf(sInfo, "%s dest=(%d,%d)-(%d,%d)", sInfo, lpd->left, lpd->top, lpd->right, lpd->bottom);
		else
			sprintf(sInfo, "%s dest=(NULL)", sInfo);
	}
	strcat(sInfo, "\n");
	OutTrace(sInfo);
	return;
}

void BlitTrace(char *label, LPRECT lps, LPRECT lpd, int line)
{
	char sInfo[512];
	sprintf(sInfo, "Blt: %s", label);
	if (lps)
		sprintf(sInfo, "%s src=(%d,%d)-(%d,%d)", sInfo, lps->left, lps->top, lps->right, lps->bottom);
	else
		sprintf(sInfo, "%s src=(NULL)", sInfo);
	if (lpd)
		sprintf(sInfo, "%s dest=(%d,%d)-(%d,%d)", sInfo, lpd->left, lpd->top, lpd->right, lpd->bottom);
	else
		sprintf(sInfo, "%s dest=(NULL)", sInfo);
	sprintf(sInfo, "%s at %d\n", sInfo, line);
	OutTrace(sInfo);
	return;
}

HRESULT WINAPI PrimaryBlt(LPDIRECTDRAWSURFACE lpdds, LPRECT lpdestrect, LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect)
{
	return (*pBlt)(lpdds, lpdestrect, lpddssrc, lpsrcrect, DDBLT_WAIT, 0);
}

HRESULT WINAPI PrimaryFastBlt(LPDIRECTDRAWSURFACE lpdds, LPRECT lpdestrect, LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect)
{
	RECT client;
	int iXOffset, iYOffset; // offsets to center surface area to window
	(*pGetClientRect)(dxw.GethWnd(), &client);
	iXOffset = (client.right - dxw.GetScreenWidth()) >> 1;
	iYOffset = (client.bottom - dxw.GetScreenHeight()) >> 1;
	return (*pBltFast)(lpdds, iXOffset + lpdestrect->left, iYOffset + lpdestrect->top, lpddssrc, lpsrcrect, DDBLTFAST_WAIT);
}

HRESULT WINAPI PrimaryStretchBlt(LPDIRECTDRAWSURFACE lpdds, LPRECT lpdestrect, LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect)
{
	HRESULT res;
	DDSURFACEDESC2 ddsd; 
	RECT TmpRect;
	LPDIRECTDRAWSURFACE lpddsTmp;
	LPDIRECTDRAWSURFACE lpddsBak;
	DDSCAPS caps;
	CreateSurface1_Type pCreateSurface;
	int dwSize;

	switch(iBakBufferVersion){
		default:
		case 1: pCreateSurface=pCreateSurface1; dwSize = sizeof(DDSURFACEDESC); break;
		case 2: pCreateSurface=(CreateSurface1_Type)pCreateSurface2; dwSize = sizeof(DDSURFACEDESC); break;
		case 3: pCreateSurface=(CreateSurface1_Type)pCreateSurface3; dwSize = sizeof(DDSURFACEDESC); break;
		case 4: pCreateSurface=(CreateSurface1_Type)pCreateSurface4; dwSize = sizeof(DDSURFACEDESC2); break;
		case 7: pCreateSurface=(CreateSurface1_Type)pCreateSurface7; dwSize = sizeof(DDSURFACEDESC2); break;
	}
	caps.dwCaps = DDSCAPS_BACKBUFFER;
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = dwSize;
	if(lpddssrc==NULL){
		// blit from backbuffer
		lpdds->GetAttachedSurface(&caps, &(LPDIRECTDRAWSURFACE)lpddsBak);
		if(lpddsBak == NULL) {
			OutTraceE("PrimaryStretchBlt: skip - lpddsBak=%x at %d\n", lpddsBak, __LINE__);
			return DD_OK; // fake a success ...
		}
		lpddsBak->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd);
	}
	else{
		// blit from surface
		lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd);
	}
	TmpRect.left = TmpRect.top = 0;
	TmpRect.bottom = ddsd.dwHeight = lpdestrect->bottom - lpdestrect->top;
	TmpRect.right  = ddsd.dwWidth  = lpdestrect->right  - lpdestrect->left;
	if((TmpRect.bottom==0) || (TmpRect.right==0)) return DD_OK; // avoid blitting to null areas (Fifa 2000 D3D)
	ddsd.dwFlags = (DDSD_HEIGHT | DDSD_WIDTH | DDSD_CAPS);
	// capabilities must cope with primary / backbuffer surface capabilities to get speedy operations
	ddsd.ddsCaps.dwCaps = dwBackBufferCaps;
	res=(*pCreateSurface)(lpPrimaryDD, (LPDDSURFACEDESC)&ddsd, &lpddsTmp, NULL);
	if(res) {
		OutTraceE("CreateSurface: ERROR %x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		OutTraceB("CreateSurface: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[Gateway]", __LINE__));
		return res;
	}
	// stretch-blit to target size on OFFSCREENPLAIN temp surface
	res= (*pBlt)(lpddsTmp, &TmpRect, lpddssrc, lpsrcrect, DDBLT_WAIT, 0);
	if(res) {
		OutTraceE("Blt: ERROR %x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	}
	else {
		// fast-blit to primary
		res= (*pBltFast)(lpdds, lpdestrect->left, lpdestrect->top, lpddsTmp, &TmpRect, DDBLTFAST_WAIT);
		if(res) OutTraceE("Blt: ERROR %x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	}
	(*pReleaseS)(lpddsTmp);
	return res;
}

void *LoadFilter(char *apiname)
{
	HMODULE filterlib;
	#define MAX_FILE_PATH 512
	char sSourcePath[MAX_FILE_PATH+1];
	char *p;
	DWORD dwAttrib;	
	
	dwAttrib = GetFileAttributes("dxwnd.dll");
	if (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) return NULL;
	GetModuleFileName(GetModuleHandle("dxwnd"), sSourcePath, MAX_FILE_PATH);
	p=&sSourcePath[strlen(sSourcePath)-strlen("dxwnd.dll")];

	*p=0;
	SetDllDirectory(sSourcePath);

	strcpy(p, "filter.dll");
	filterlib=(*pLoadLibraryA)(sSourcePath);
	if(!filterlib) {
		OutTraceDW("DXWND: Load lib=\"%s\" failed err=%d\n", sSourcePath, GetLastError());
		return NULL;
	}
	return (*pGetProcAddress)(filterlib, apiname);
}

HRESULT WINAPI PrimaryBilinearBlt(LPDIRECTDRAWSURFACE lpdds, LPRECT lpdestrect, LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect)
{
	HRESULT res;
	typedef void (WINAPI *Resize_HQ_Type)( unsigned char*, RECT *, int, unsigned char*, RECT *, int);
	static Resize_HQ_Type pResize_HQ = NULL;
	/* to be implemented .... */

	DDSURFACEDESC2 ddsd; 
	RECT TmpRect, SrcRect;
	LPDIRECTDRAWSURFACE lpddsTmp;
	LPDIRECTDRAWSURFACE lpddsBak;
	LPDIRECTDRAWSURFACE lpddsCopy=NULL;
	DDSCAPS caps;
	BYTE *bSourceBuf, *bDestBuf;
	LONG dwWidth, dwHeight;
	int SrcPitch, DestPitch;
	CreateSurface1_Type pCreateSurface;
	int dwSize;

	switch(iBakBufferVersion){
		default:
		case 1: pCreateSurface=pCreateSurface1; dwSize = sizeof(DDSURFACEDESC); break;
		case 2: pCreateSurface=(CreateSurface1_Type)pCreateSurface2; dwSize = sizeof(DDSURFACEDESC); break;
		case 3: pCreateSurface=(CreateSurface1_Type)pCreateSurface3; dwSize = sizeof(DDSURFACEDESC); break;
		case 4: pCreateSurface=(CreateSurface1_Type)pCreateSurface4; dwSize = sizeof(DDSURFACEDESC2); break;
		case 7: pCreateSurface=(CreateSurface1_Type)pCreateSurface7; dwSize = sizeof(DDSURFACEDESC2); break;
	}
	caps.dwCaps = DDSCAPS_BACKBUFFER;
	memset(&ddsd, 0, sizeof(DDSURFACEDESC));
	ddsd.dwSize = dwSize;
	if(lpddssrc==NULL){
		// blit from backbuffer
		lpdds->GetAttachedSurface(&caps, &lpddsBak);
		if(lpddsBak) lpddsBak->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd);
	}
	else{
		// blit from surface
		lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd);
	}
	
	// assign source RECT values anyway....
	if(!lpsrcrect){
		lpsrcrect = &SrcRect;
		lpsrcrect->left = lpsrcrect->top = 0;
		lpsrcrect->right = dxw.GetScreenWidth();
		lpsrcrect->bottom = dxw.GetScreenHeight();
	}

	dwWidth = lpdestrect->right  - lpdestrect->left;
	dwHeight = lpdestrect->bottom - lpdestrect->top;
	TmpRect.left = TmpRect.top = 0;
	TmpRect.bottom = ddsd.dwHeight = dwHeight;
	TmpRect.right  = ddsd.dwWidth  = dwWidth;
	ddsd.dwFlags = (DDSD_HEIGHT | DDSD_WIDTH | DDSD_CAPS);
	// capabilities must cope with primary / backbuffer surface capabilities to get speedy operations
	ddsd.ddsCaps.dwCaps = dwBackBufferCaps;
	res=(*pCreateSurface)(lpPrimaryDD, (LPDDSURFACEDESC)&ddsd, &lpddsTmp, NULL);
	if(res) OutTraceE("CreateSurface: ERROR %x(%s) at %d\n", res, ExplainDDError(res), __LINE__);

	// get informations
	memset(&ddsd,0,dwSize);
	ddsd.dwSize = dwSize;
	ddsd.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	res=(*pLock)(lpddssrc, 0, (LPDDSURFACEDESC)&ddsd, DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY, 0);
	if(res) {
		OutTraceE("Lock: ERROR %x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return DD_OK;
	}
	bSourceBuf = (BYTE *)ddsd.lpSurface;
	SrcPitch = ddsd.lPitch;
	memset(&ddsd,0,dwSize);
	ddsd.dwSize = dwSize;
	ddsd.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	res=(*pLock)(lpddsTmp, 0, (LPDDSURFACEDESC)&ddsd, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY|DDLOCK_WAIT, 0);
	if(res) {
		OutTraceE("Lock: ERROR %x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return DD_OK;
	}
	bDestBuf = (BYTE *)ddsd.lpSurface;
	DestPitch = ddsd.lPitch;

	// do the filtering
	if(!pResize_HQ) {
		char *filter;
		HMODULE filterlib;
		switch(ddsd.ddpfPixelFormat.dwGBitMask)
		{
		default:
		case 0x00FF00:
			filter = "Resize_HQ_4ch";
			break;
		case 0x0007E0: // RGB565
			filter = "Resize_HQ_2ch565";
			break;
		case 0x0003E0: // RGB555
			filter = "Resize_HQ_2ch555";
			break;
		}

		filterlib=(*pLoadLibraryA)("filter.dll");
		if(!filterlib) {
			char sMsg[80+1];
			sprintf(sMsg, "DXWND: ERROR can't load lib=\"filter.dll\" err=%x\n", GetLastError());
			OutTraceE(sMsg);
			MessageBox(0, sMsg, "ERROR", MB_OK | MB_ICONEXCLAMATION);
			exit(0);
		}
		pResize_HQ = (Resize_HQ_Type)(*pGetProcAddress)(filterlib, filter);
		if(!pResize_HQ){
			char sMsg[80+1];
			sprintf(sMsg, "DXWND: ERROR can't load name=\"%s\"\n", filter);
			OutTraceE(sMsg);
			MessageBox(0, sMsg, "ERROR", MB_OK | MB_ICONEXCLAMATION);
			exit(0);
		}
	}
	(*pResize_HQ)(bSourceBuf, lpsrcrect, SrcPitch, bDestBuf, lpdestrect, DestPitch);

	// fast-blit to primary
	(*pUnlockMethod(lpddssrc))(lpddssrc, NULL);
	(*pUnlockMethod(lpddsTmp))(lpddsTmp, NULL);
	res= (*pBltFast)(lpdds, lpdestrect->left, lpdestrect->top, lpddsTmp, &TmpRect, DDBLTFAST_WAIT);
	if(res) OutTraceE("BltFast: ERROR %x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	(*pReleaseS)(lpddsTmp);	
	if(lpddsCopy) (*pReleaseS)(lpddsCopy);
	return res;
}

HRESULT WINAPI PrimaryNoBlt(LPDIRECTDRAWSURFACE lpdds, LPRECT lpdestrect, LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect)
{
	return DD_OK;
}

typedef HRESULT (WINAPI *ColorConversion_Type)(LPDIRECTDRAWSURFACE, RECT, LPDIRECTDRAWSURFACE *);

HRESULT WINAPI ColorConversionEmulated(LPDIRECTDRAWSURFACE lpdds, RECT emurect, LPDIRECTDRAWSURFACE *lpddssource)
{
	HRESULT res;
	res=(*pEmuBlt)(lpDDSEmu_Back, &emurect, lpdds, &emurect, DDBLT_WAIT, 0);
	if(res==DDERR_SURFACEBUSY){
		(*pUnlockMethod(lpdds))(lpdds, NULL);
		(*pUnlockMethod(lpDDSEmu_Back))(lpDDSEmu_Back, NULL);
		res=(*pEmuBlt)(lpDDSEmu_Back, &emurect, lpdds, &emurect, DDBLT_WAIT, 0);
	}
	if(res) {
		BlitError(res, &emurect, &emurect, __LINE__);
		if(dxw.dwFlags1 & SUPPRESSDXERRORS) res=0;
	}
	*lpddssource = lpDDSEmu_Back;
	return res;
}

HRESULT WINAPI ColorConversionGDI(LPDIRECTDRAWSURFACE lpdds, RECT emurect, LPDIRECTDRAWSURFACE *lpddssource)
{
	// GDICOLORCONV: use GDI capabilities to convert color depth by BitBlt-ting between different hdc
	HRESULT res;
	OutTrace("STEP: GDI Color Conversion\n");
	do {
		HDC hdc_source, hdc_dest;
		res=(*pGetDC)(lpdds, &hdc_source);
		if(res) {
			OutTraceE("GetDC ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			break;
		}
		res=(*pGetDC)(lpDDSEmu_Back, &hdc_dest);
		if(res) {
			OutTraceE("GetDC ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			break;
		}
		if(!BitBlt(hdc_dest, 0, 0, dxw.GetScreenWidth(), dxw.GetScreenHeight(), hdc_source, 0, 0, SRCCOPY)){
			OutTraceE("BitBlt ERROR: err=%d at %d\n", GetLastError(), __LINE__);
		}
		res=(*pReleaseDC)(lpdds, hdc_source);
		if(res) {
			OutTraceE("ReleaseDC ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			break;
		}
		res=(*pReleaseDC)(lpDDSEmu_Back, hdc_dest);
		if(res) {
			OutTraceE("ReleaseDC ERROR: err=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			break;
		}
	} while(FALSE);
	*lpddssource = lpDDSEmu_Back;
	return res;
}

HRESULT WINAPI ColorConversionDDRAW(LPDIRECTDRAWSURFACE lpdds, RECT emurect, LPDIRECTDRAWSURFACE *lpddssource)
{
	*lpddssource = lpdds;
	return DD_OK;
}

HRESULT WINAPI extFlip(LPDIRECTDRAWSURFACE lpdds, LPDIRECTDRAWSURFACE lpddssrc, DWORD dwflags)
{
	BOOL IsPrim;
	HRESULT res;
	DDSURFACEDESC2 ddsd;
	LPDIRECTDRAWSURFACE lpddsTmp;

	IsPrim=dxw.IsAPrimarySurface(lpdds);
	OutTraceDDRAW("Flip: lpdds=%x%s, src=%x, flags=%x(%s)\n", 
		lpdds, IsPrim?"(PRIM)":"", lpddssrc, dwflags, ExplainFlipFlags(dwflags));

	if (!IsPrim){
		if(lpddssrc){
			res=(*pFlip)(lpdds, lpddssrc, dwflags);
		}
		else{
			LPDIRECTDRAWSURFACE lpddsAttached;
			DDSCAPS ddsc;
			DDSURFACEDESC2 sd;

			sd.dwSize=Set_dwSize_From_Surface(lpdds);
			res=lpdds->GetSurfaceDesc((DDSURFACEDESC *)&sd);
			if (res) OutTraceDW("Flip: GetSurfaceDesc res=%x at %d\n",res, __LINE__);

			// replace these CAPS (good for seven kingdoms II) with same as lpdds surface
			//ddsc.dwCaps=DDSCAPS_OFFSCREENPLAIN+DDSCAPS_SYSTEMMEMORY;
			ddsc.dwCaps=sd.ddsCaps.dwCaps;

			res=lpdds->GetAttachedSurface(&ddsc, &lpddsAttached);
			if(res){
				OutTraceE("Flip: GetAttachedSurface ERROR %x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
				return res;
			}
			res=sBlt("Flip", lpdds, NULL, lpddsAttached, NULL, DDBLT_WAIT, 0, TRUE);
			if(res){
				OutTraceE("Flip: Blt ERROR %x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
				return res;
			}
		}
		if(res)	OutTraceE("Flip: ERROR %x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	// emulation to primary surface Flip - you can't flip to window surfaces,
	// so you have to replace it with Blt operations.

	if((dwflags & DDFLIP_WAIT) || (dxw.dwFlags1 & SAVELOAD)) lpPrimaryDD->WaitForVerticalBlank(DDWAITVB_BLOCKEND , 0);

	if(dxw.dwFlags4 & NOFLIPEMULATION){
		HRESULT res2;
		// create a temporary working surface
		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = SurfaceDescrSize(lpdds);
		// v2.02.80: the BackBuffer may not exist? see "HellCopter"
		if(lpDDSBack){
			(*pGetSurfaceDescMethod(lpdds))((LPDIRECTDRAWSURFACE2)lpDDSBack, &ddsd);
			ddsd.dwFlags &= ~DDSD_PITCH;
		}
		else{
			ddsd.dwFlags = (DDSD_HEIGHT | DDSD_WIDTH | DDSD_CAPS);
			ddsd.ddsCaps.dwCaps = (DDSCAPS_OFFSCREENPLAIN);
			ddsd.dwHeight = dxw.GetScreenHeight();
			ddsd.dwWidth = dxw.GetScreenWidth();
		}
		res2=(*pCreateSurfaceMethod(lpdds))(lpPrimaryDD, &ddsd, &lpddsTmp, NULL);
		if(res2) {
			OutTraceE("CreateSurface: ERROR %x(%s) at %d\n", res2, ExplainDDError(res2), __LINE__);
			OutTraceE("Size=%d lpPrimaryDD=%x lpDDSBack=%x %s\n", 
				ddsd.dwSize, lpPrimaryDD, lpDDSBack, LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[FlipBuf]", __LINE__));
		}
		//OutTrace("DEBUG: copied surface size=(%dx%d)\n", ddsd.dwWidth, ddsd.dwHeight);
		// copy front buffer 
		if(dxw.dwFlags1 & EMULATESURFACE){
			// in emulated mode, the primary surface is virtual and you can pick it all
			// needed for "Gruntz"
			res2= (*pBlt)(lpddsTmp, NULL, lpdds, NULL, DDBLT_WAIT, NULL);
			if(res2) BlitError(res2, NULL, NULL,  __LINE__);
		}
		else {
			// in no-emulated mode, the primary surface is the whole screen, so you have to pick...
			// needed for "Black Thorn"
			RECT clip;
			if(dxw.dwFlags1 & EMULATEBUFFER) 
				clip=dxw.GetScreenRect();
			else
			clip=dxw.GetUnmappedScreenRect();
			res2= (*pBlt)(lpddsTmp, NULL, lpdds, &clip, DDBLT_WAIT, NULL);
			if(res2) BlitError(res2, &clip, NULL,  __LINE__);
		}
	}

	if(lpddssrc){
		//res=lpdds->Blt(0, lpddssrc, 0, DDBLT_WAIT, 0);
		res=sBlt("Flip", lpdds, NULL, lpddssrc, NULL, DDBLT_WAIT, 0, TRUE);
	}
	else{
		//v2.02.38: look for a valid BackBuffer surface
		if(!lpDDSBack) lpDDSBack=dxw.GetBackBufferSurface();
		if(!lpDDSBack){
			OutTraceE("Flip: no backbuffer\n");
			return DDERR_INVALIDPARAMS;
		}
		if (dxw.dwFlags2 & BACKBUFATTACH){
			RECT NullArea;
			NullArea.left=NullArea.top=0;
			NullArea.bottom=dxw.GetScreenHeight();
			NullArea.right=dxw.GetScreenWidth();
			res=sBlt("Flip", lpdds, NULL, lpDDSBack, &NullArea, DDBLT_WAIT, 0, TRUE);
		}
		else
			res=sBlt("Flip", lpdds, NULL, lpDDSBack, NULL, DDBLT_WAIT, 0, TRUE);

		lpddssrc = lpDDSBack;
	}

	if(dxw.dwFlags4 & NOFLIPEMULATION){
		HRESULT res2;
		// restore flipped backbuffer and delete temporary surface
		res2= (*pBlt)(lpddssrc, NULL, lpddsTmp, NULL, DDBLT_WAIT, NULL);
		if(res2) OutTraceE("Blt: ERROR %x(%s) at %d\n", res2, ExplainDDError(res2), __LINE__);
		(*pReleaseS)(lpddsTmp);
	}

	if(res) OutTraceE("Flip: Blt ERROR %x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extBlt(LPDIRECTDRAWSURFACE lpdds, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPDDBLTFX lpddbltfx)
{
	if ((dxw.dwFlags2 & FULLRECTBLT) && dxw.IsAPrimarySurface(lpdds)){
		lpsrcrect=NULL;
		lpdestrect=NULL;
	}

	return sBlt("Blt", lpdds, lpdestrect, lpddssrc, lpsrcrect, dwflags, lpddbltfx, FALSE);
}

HRESULT WINAPI extBltFast(LPDIRECTDRAWSURFACE lpdds, DWORD dwx, DWORD dwy, 
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwtrans)
{
	// BltFast is supported just on screen surfaces, so it has to be replaced
	// by ordinary Blt operation in EMULATESURFACE mode.
	// Mind that screen surface doesn't necessarily mean PRIMARY surfaces!

	RECT srcrect, destrect;
	DWORD flags = 0;
	DDSURFACEDESC2 ddsd;
	HRESULT ret;
	BOOL ToPrim, FromPrim;

	ToPrim=dxw.IsAPrimarySurface(lpdds);
	FromPrim=dxw.IsAPrimarySurface(lpddssrc);

	CleanRect(&lpsrcrect,__LINE__);

	if(IsTraceDDRAW){
		char sRect[81];
		if (lpsrcrect) sprintf(sRect, "(%d,%d)-(%d,%d)", lpsrcrect->left, lpsrcrect->top, lpsrcrect->right, lpsrcrect->bottom);
		else strcpy(sRect, "(NULL)");
		OutTrace("BltFast: dest=%x%s src=%x%s dwTrans=%x(%s) (x,y)=(%d,%d) srcrect=%s\n", 
			lpdds, ToPrim?"(PRIM)":"", lpddssrc, FromPrim?"(PRIM)":"", dwtrans, ExplainBltFastFlags(dwtrans), dwx, dwy, sRect);
	}

	// consistency check ....
	if (lpsrcrect)
		if((lpsrcrect->left >= lpsrcrect->right) || (lpsrcrect->top >= lpsrcrect->bottom)) {
			OutTraceDW("BltFast: ASSERT bad rect at %d\n", __LINE__);
			return 0;
		}

	flags = 0;
	if(dwtrans & DDBLTFAST_WAIT) flags = DDBLT_WAIT;
	if(dwtrans & DDBLTFAST_DESTCOLORKEY) flags |= DDBLT_KEYDEST;
	if(dwtrans & DDBLTFAST_SRCCOLORKEY) flags |= DDBLT_KEYSRC;
	//if(dwtrans & DDBLTFAST_SRCCOLORKEY) flags |= DDBLT_COLORFILL;

	if ((dxw.dwFlags2 & FULLRECTBLT) && ToPrim){
		 return sBlt("BltFast", lpdds, NULL, lpddssrc, lpsrcrect, flags, NULL, FALSE);
	}

	destrect.left = dwx;
	destrect.top = dwy;
	if(lpsrcrect){
		destrect.right = destrect.left + lpsrcrect->right - lpsrcrect->left;
		destrect.bottom = destrect.top + lpsrcrect->bottom - lpsrcrect->top;
		// avoid altering pointed values....
		srcrect=*lpsrcrect;
		ret=sBlt("BltFast", lpdds, &destrect, lpddssrc, &srcrect, flags, NULL, FALSE);
	}
	else{
		// does it EVER goes through here? NULL is not a valid rect value for BltFast call....
		// yes, forced in FULLRECTBLT mode!
		// yes, when BltFast on DDBLTFAST_SRCCOLORKEY!! (Pax Corpus)
		if(lpddssrc){
			ddsd.dwSize=Set_dwSize_From_Surface(lpddssrc);
			ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
			ret=lpddssrc->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd);
			if (ret){
				OutTraceE("BltFast: GetSurfaceDesc ERROR %x at %d\n", ret, __LINE__);
				return 0;
			}
			destrect.right = destrect.left + ddsd.dwWidth;
			destrect.bottom = destrect.top + ddsd.dwHeight;
			ret=sBlt("BltFast", lpdds, &destrect, lpddssrc, NULL, flags, NULL, FALSE);
		}
		else{
			//ret=sBlt("BltFast", lpdds, NULL, lpddssrc, NULL, flags, NULL, FALSE);
			ret=DD_OK;
		}
	}

	return ret;
}

HRESULT WINAPI extWaitForVerticalBlank(LPDIRECTDRAW lpdd, DWORD dwflags, HANDLE hevent)
{
	if(dxw.dwFlags1 & SAVELOAD){
		dxw.VSyncWait();
		return 0;
	}
	return (*pWaitForVerticalBlank)(lpdd, dwflags, hevent);
}

#define DDPCAPS_INITIALIZE_LEGACY 0x00000008l

HRESULT WINAPI extCreatePalette(LPDIRECTDRAW lpdd, DWORD dwflags, LPPALETTEENTRY lpddpa,
	LPDIRECTDRAWPALETTE *lplpddp, IUnknown *pu)
{
	HRESULT res;

	OutTraceDDRAW("CreatePalette: dwFlags=%x(%s)\n", dwflags, ExplainCreatePaletteFlags(dwflags));
	if(IsDebug && (dwflags & DDPCAPS_8BIT)) dxw.DumpPalette(256, lpddpa);

	//if (dwflags & ~(DDPCAPS_PRIMARYSURFACE|DDPCAPS_8BIT|DDPCAPS_ALLOW256|DDPCAPS_INITIALIZE_LEGACY)) STOPPER("Palette flags");

	if(dxw.dwFlags1 & EMULATESURFACE) dwflags &= ~DDPCAPS_PRIMARYSURFACE;
	res = (*pCreatePalette)(lpdd, dwflags, lpddpa, lplpddp, pu);
	if (res) {
		OutTraceE("CreatePalette: ERROR res=%x(%s)\n", res, ExplainDDError(res));
		return res;
	}
	else OutTraceDDRAW("CreatePalette: OK lpddp=%x\n", *lplpddp);

	HookDDPalette(lplpddp);
	return 0;
}

HRESULT WINAPI extGetPalette(LPDIRECTDRAWSURFACE lpdds, LPDIRECTDRAWPALETTE *lplpddp)
{
	HRESULT res;
	BOOL isPrim, isBack;

	isPrim=dxw.IsAPrimarySurface(lpdds);
	isBack=dxw.IsABackBufferSurface(lpdds);
	OutTraceDDRAW("GetPalette: lpdds=%x%s%s\n", lpdds, isPrim?"(PRIM)":"", isBack?"(BACK)":"");

	res=(*pGetPalette)(lpdds, lplpddp);

	// v2.03.07: in "Die Hard Trilogy" the backbuffer surface is queryed for the palette
    // v2.03.08: in "Viper Racing" lpDDP is still NULL (how could it be?)
	if((dxw.dwFlags1 & EMULATESURFACE) && (res == DDERR_NOPALETTEATTACHED) && (isPrim||isBack) && lpDDP){
		OutTraceDW("GetPalette: retrieve PRIMARY palette for emulated surface lpDDP=%x\n", lpDDP);
		*lplpddp = lpDDP;
		lpDDP->AddRef();
		res=DD_OK;
	}

	if (res) OutTraceE("GetPalette: ERROR res=%x(%s)\n", res, ExplainDDError(res));
	else OutTraceDDRAW("GetPalette: OK\n");
	return res;
}

HRESULT WINAPI extSetPalette(LPDIRECTDRAWSURFACE lpdds, LPDIRECTDRAWPALETTE lpddp)
{
	PALETTEENTRY *lpentries;
	BOOL isPrim;
	HRESULT res;
	
	isPrim=dxw.IsAPrimarySurface(lpdds);
	OutTraceDDRAW("SetPalette: lpdds=%x%s lpddp=%x\n", lpdds, isPrim?"(PRIM)":"", lpddp);

	res=(*pSetPalette)(lpdds, lpddp);
	if(res)OutTraceE("SetPalette: ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	else OutTraceDDRAW("SetPalette: OK\n");
	res=DD_OK;

	if((dxw.dwFlags1 & EMULATESURFACE) && isPrim){
		OutTraceDW("SetPalette: register PRIMARY palette lpDDP=%x\n", lpddp);
		lpDDP = lpddp;
		if(lpddp){
			HRESULT res2;
			lpentries = (LPPALETTEENTRY)PaletteEntries;
			res2=lpddp->GetEntries(0, 0, 256, lpentries);
			if(res2) OutTraceE("SetPalette: GetEntries ERROR res=%x(%s)\n", res2, ExplainDDError(res2));
			mySetPalette(0, 256, lpentries); // v2.02.76: necessary for "Requiem Avenging Angel" in SURFACEEMULATION mode
		}
		// Apply palette to backbuffer surface. This is necessary on some games: "Duckman private dick", "Total Soccer 2000", ...
		if (lpDDSBack){
				OutTraceDW("SetPalette: apply PRIMARY palette lpDDP=%x to DDSBack=%x\n", lpddp, lpDDSBack);
				res=(*pSetPalette)(lpDDSBack, lpddp);
				if(res) OutTraceE("SetPalette: ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			}
		// add a reference to simulate what would happen in reality....
		lpdds->AddRef();
		res=DD_OK;
	}

	return res;
}

HRESULT WINAPI extSetEntries(LPDIRECTDRAWPALETTE lpddp, DWORD dwflags, DWORD dwstart, DWORD dwcount, LPPALETTEENTRY lpentries)
{
	HRESULT res;

	OutTraceDDRAW("SetEntries: lpddp=%x dwFlags=%x, start=%d, count=%d entries=%x\n", //GHO: added trace infos
		lpddp, dwflags, dwstart, dwcount, lpentries);
	if(IsDebug) dxw.DumpPalette(dwcount, &lpentries[dwstart]);

	res = (*pSetEntries)(lpddp, dwflags, dwstart, dwcount, lpentries);
	if(res) OutTraceE("SetEntries: ERROR res=%x(%s)\n", res, ExplainDDError(res));
	else OutTraceDDRAW("SetEntries: OK\n");

	if((dxw.dwFlags1 & EMULATESURFACE) && (lpDDP == lpddp)){
		OutTraceDW("SetEntries: update PRIMARY palette lpDDP=%x\n", lpddp);
		if ((dwstart + dwcount > 256) || (dwstart<0)){
			dwcount=256;
			dwstart=0;
			OutTraceDW("SetEntries: ASSERT start+count > 256\n");
		}

		mySetPalette(dwstart, dwcount, lpentries);

		// GHO: needed for fixed rect and variable palette animations, 
		// e.g. dungeon keeper loading screen, Warcraft II splash, ...
		// GHO: but refreshing cause flickering when GDI was used without updating the primary surface
		// e.g. Tomb Raider 2 intro titles, Virtua Fighter PC, ...
		// v2.03.10: do not blit also in case of GDI mode
		if ((dxw.dwFlags1 & EMULATESURFACE) && !(dxw.dwFlags2 & NOPALETTEUPDATE) && !(dxw.dwFlags5 & GDIMODE)) dxw.ScreenRefresh();
	}
	return res;
}

HRESULT WINAPI extSetClipper(LPDIRECTDRAWSURFACE lpdds, LPDIRECTDRAWCLIPPER lpddc)
{
	HRESULT res;
	BOOL isPrim;
	isPrim=dxw.IsAPrimarySurface(lpdds);
	OutTraceDDRAW("SetClipper: lpdds=%x%s lpddc=%x\n", lpdds, isPrim?"(PRIM)":"", lpddc);

	// v2.1.84: SUPPRESSCLIPPING flag - improves "Monopoly Edition 3D" where continuous
	// clipping ON & OFF affects blitting on primary surface.
	// Needed also to avoid "New Yourk Racer" intro movie clipping.
	if(dxw.dwFlags1 & SUPPRESSCLIPPING) return 0;

	if(dxw.dwFlags1 & (EMULATESURFACE|EMULATEBUFFER)){
		if ((isPrim && lpDDSEmu_Prim) || 
			((lpdds == lpDDSBack) && lpDDSEmu_Back)){
			OutTraceDW("SetClipper: skip primary/backbuffer lpdds=%x\n", lpdds);
			res=0;
		}
		else 
			res=(*pSetClipper)(lpdds, lpddc);
	}
	else
		res=(*pSetClipper)(lpdds, lpddc);

	if (res)
		OutTraceE("SetClipper: ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extLock(LPDIRECTDRAWSURFACE lpdds, LPRECT lprect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD flags, HANDLE hEvent)
{
	HRESULT res;

	CleanRect(&lprect, __LINE__);

	if(IsTraceDDRAW){
		BOOL IsPrim=dxw.IsAPrimarySurface(lpdds);
		char sRect[81];
		if (lprect) sprintf_s(sRect, 80, "(%d,%d)-(%d,%d)", lprect->left, lprect->top, lprect->right, lprect->bottom);
		else strcpy(sRect, "(NULL)");
		OutTrace("Lock: lpdds=%x%s flags=%x(%s) lpDDSurfaceDesc=%x rect=%s\n", 
			lpdds, (IsPrim ? "(PRIM)":""), flags, ExplainLockFlags(flags), lpDDSurfaceDesc, sRect);
	}

	res=(*pLock)(lpdds, lprect, lpDDSurfaceDesc, flags, hEvent);
	if(res==DDERR_SURFACEBUSY){ // v70: fix for "Ancient Evil"
		(*pUnlockMethod(lpdds))(lpdds, NULL);
		res = (*pLock)(lpdds, lprect, lpDDSurfaceDesc, flags, hEvent);
		OutTraceDW("Lock RETRY: ret=%x(%s)\n", res, ExplainDDError(res));
	}
	if(res) OutTraceE("Lock ERROR: ret=%x(%s)\n", res, ExplainDDError(res));
	OutTraceB("Lock: lPitch=%d lpSurface=%x %s\n", 
		lpDDSurfaceDesc->lPitch, lpDDSurfaceDesc->lpSurface, LogSurfaceAttributes(lpDDSurfaceDesc, "[Locked]", __LINE__));
	if(dxw.dwFlags1 & SUPPRESSDXERRORS) res=DD_OK;

	return res;
}

LPDIRECTDRAWSURFACE2 lpDDSBuffer = NULL;

HRESULT WINAPI extLockDir(LPDIRECTDRAWSURFACE lpdds, LPRECT lprect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD flags, HANDLE hEvent)
{
	HRESULT res, res2;
	static RECT client;
	POINT upleft={0,0};
	LPDIRECTDRAWSURFACE lpDDSPrim;

	// this hooker operates on 
	// Beware!!! for strange reason, the function gets hooked to ANY surface, also non primary ones!!!
	// to find out whether it is the primary or not, using lpdds==lpPrimaryDD->GetGDISurface(&lpDDSPrim);

	if(IsTraceDDRAW){
		BOOL IsPrim=dxw.IsAPrimarySurface(lpdds);
		char sRect[81];
		if (lprect) sprintf_s(sRect, 80, "(%d,%d)-(%d,%d)", lprect->left, lprect->top, lprect->right, lprect->bottom);
		else strcpy(sRect, "(NULL)");
		OutTrace("Lock: lpdds=%x%s flags=%x(%s) lpDDSurfaceDesc=%x rect=%s\n", 
			lpdds, (IsPrim ? "(PRIM)":""), flags, ExplainLockFlags(flags), lpDDSurfaceDesc, sRect);
	}

	// V2.02.43: Empire Earth does some test Lock operations apparently before the primary surface is created
	if(lpPrimaryDD){
		lpDDSPrim=0;
		res2=(*pGetGDISurface)(lpPrimaryDD, &lpDDSPrim);
		if(res2)
			OutTraceE("Lock: GetGDISurface ERROR res=%x(%s) at %d\n", res2, ExplainDDError(res2), __LINE__);
		else
			(*pReleaseS)(lpDDSPrim);
		if(lpdds==lpDDSPrim){
			if(dxw.dwFlags1 & LOCKEDSURFACE){
				DDSURFACEDESC2 ddsd;
				DDBLTFX fx;
				memset(&ddsd, 0, sizeof(ddsd));
				//ddsd.dwSize=SurfaceDescrSize(lpdds);
				ddsd.dwSize=sizeof(DDSURFACEDESC);
				ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
				ddsd.dwWidth = dxw.GetScreenWidth();
				ddsd.dwHeight = dxw.GetScreenHeight();
				ddsd.ddsCaps.dwCaps = 0;
				OutTraceB("Lock: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)&ddsd, "[Dir FixBuf]" , __LINE__));
				res=(*pCreateSurface1)(lpPrimaryDD, (DDSURFACEDESC *)&ddsd, (LPDIRECTDRAWSURFACE *)&lpDDSBuffer, 0);
				if(res){
					OutTraceE("CreateSurface: ERROR on DDSBuffer res=%x(%s) at %d\n",res, ExplainDDError(res), __LINE__);
					return res;
				}
				memset(&fx, 0, sizeof(fx));
				fx.dwSize=sizeof(DDBLTFX);
				fx.dwFillColor=0;
				res=(*pBlt)((LPDIRECTDRAWSURFACE)lpDDSBuffer, NULL, NULL, NULL, DDBLT_WAIT|DDBLT_COLORFILL, &fx);
				if(res){
					OutTraceE("Blt: ERROR on DDSBuffer res=%x(%s) at %d\n",res, ExplainDDError(res), __LINE__);
				}
				lpdds=(LPDIRECTDRAWSURFACE)lpDDSBuffer;
			}
			else{
				// since it can't scale, at least the updated rect is centered into the window.
				(*pGetClientRect)(dxw.GethWnd(), &client);
				(*pClientToScreen)(dxw.GethWnd(), &upleft);
				if (!lprect) lprect=&client;
				OffsetRect(lprect, 
					upleft.x+(client.right-dxw.GetScreenWidth())/2, 
					upleft.y+(client.bottom-dxw.GetScreenHeight())/2);
				OutTraceDW("Lock: NULL rect remapped to (%d,%d)-(%d,%d)\n", 
					lprect->left, lprect->top, lprect->right, lprect->bottom);
			}
		}
	}

	res=(*pLock)(lpdds, lprect, lpDDSurfaceDesc, flags, hEvent);

	if(res) OutTraceE("Lock ERROR: ret=%x(%s)\n", res, ExplainDDError(res));
	OutTraceB("Lock: %s\n", LogSurfaceAttributes((LPDDSURFACEDESC)lpDDSurfaceDesc, "[Locked]" , __LINE__));
	if(dxw.dwFlags1 & SUPPRESSDXERRORS) res=DD_OK;

	return res;
}

HRESULT WINAPI extUnlock(int dxversion, Unlock4_Type pUnlock, LPDIRECTDRAWSURFACE lpdds, LPRECT lprect)
{
	HRESULT res;
	//RECT screen, rect;
	BOOL IsPrim;

	IsPrim=dxw.IsAPrimarySurface(lpdds);

	if ((dxversion == 4) && lprect) CleanRect(&lprect,__LINE__);

	if(IsTraceDDRAW){
		char sRect[81];
		if (dxversion == 4){
			if (lprect) sprintf_s(sRect, 80, "rect=(%d,%d)-(%d,%d)", lprect->left, lprect->top, lprect->right, lprect->bottom);
			else strcpy(sRect, "rect=(NULL)");
		}
		else
			sprintf_s(sRect, 80, "lpvoid=%x", lprect);
		OutTrace("Unlock(%d): lpdds=%x%s %s\n", dxversion, lpdds, (IsPrim ? "(PRIM)":""), sRect);
	}

	res=(*pUnlock)(lpdds, lprect);
	if(res==DDERR_NOTLOCKED) res=DD_OK; // ignore not locked error
	if (res) OutTraceE("Unlock ERROR res=%x(%s) at %d\n",res, ExplainDDError(res), __LINE__);
	if (IsPrim && res==DD_OK) {
		if(dxversion == 1){
			res=sBlt("Unlock", lpdds, NULL, lpdds, NULL, NULL, 0, FALSE);
			if(IsPrim) (*pInvalidateRect)(dxw.GethWnd(), NULL, FALSE); // to fix "Deadlock II" mouse trails....
		}
		else {
			res=sBlt("Unlock", lpdds, lprect, lpdds, lprect, NULL, 0, FALSE);
			if(IsPrim) (*pInvalidateRect)(dxw.GethWnd(), lprect, FALSE); 
		}
	}

	if(dxw.dwFlags1 & SUPPRESSDXERRORS) res=DD_OK;

	if((dxw.dwFlags5 & TEXTUREMASK) && (!IsPrim)) {
		// Texture Handling on Unlock
		TextureHandling(lpdds);
	}
	return res;
}

HRESULT WINAPI extUnlock4(LPDIRECTDRAWSURFACE lpdds, LPRECT lprect)
{
	return extUnlock(4, pUnlock4, lpdds, lprect);
}

HRESULT WINAPI extUnlock1(LPDIRECTDRAWSURFACE lpdds, LPVOID lpvoid)
{
	return extUnlock(1, (Unlock4_Type)pUnlock1, lpdds, (LPRECT)lpvoid);
}

HRESULT WINAPI extUnlockDir(int dxversion, Unlock4_Type pUnlock, LPDIRECTDRAWSURFACE lpdds, LPRECT lprect)
{
	HRESULT res;
	//RECT screen, rect;
	BOOL IsPrim;
	LPDIRECTDRAWSURFACE lpDDSPrim;

	IsPrim=dxw.IsAPrimarySurface(lpdds);

	if ((dxversion >= 4) && lprect) CleanRect(&lprect,__LINE__);

	if(IsTraceDDRAW){
		char sRect[81];
		if (dxversion == 4){
			if (lprect) sprintf_s(sRect, 80, "rect=(%d,%d)-(%d,%d)", lprect->left, lprect->top, lprect->right, lprect->bottom);
			else strcpy(sRect, "rect=(NULL)");
		}
		else
			sprintf_s(sRect, 80, "lpvoid=%x", lprect);
		OutTrace("Unlock(%d): lpdds=%x%s %s\n", dxversion, lpdds, (IsPrim ? "(PRIM)":""), sRect);
	}

	if(dxw.dwFlags1 & LOCKEDSURFACE){
		(*pGetGDISurface)(lpPrimaryDD, &lpDDSPrim);
		if(lpdds==lpDDSPrim && lpDDSBuffer){
			RECT client;
			POINT upleft={0,0};
			(*pGetClientRect)(dxw.GethWnd(), &client);
			(*pClientToScreen)(dxw.GethWnd(), &upleft);
			if (!lprect) lprect=&client;
			OffsetRect(lprect, upleft.x, upleft.y);
			res=(*pUnlock)((LPDIRECTDRAWSURFACE)lpDDSBuffer, lprect);
			(*pBlt)(lpdds, lprect, (LPDIRECTDRAWSURFACE)lpDDSBuffer, NULL, DDBLT_WAIT, 0);
			(*pReleaseS)((LPDIRECTDRAWSURFACE)lpDDSBuffer);
			lpDDSBuffer = NULL;
		}
		(*pReleaseS)(lpDDSPrim); // to leave a correct refcount 
	}

	res=(*pUnlock)(lpdds, lprect);
	if(res==DDERR_NOTLOCKED) res=DD_OK; // ignore not locked error
	if (res) OutTraceE("Unlock ERROR res=%x(%s) at %d\n",res, ExplainDDError(res), __LINE__);
	if (IsPrim && res==DD_OK) sBlt("Unlock", lpdds, NULL, lpdds, NULL, NULL, 0, FALSE);
	if(dxw.dwFlags1 & SUPPRESSDXERRORS) res=DD_OK;
	return res;
}

HRESULT WINAPI extUnlockDir4(LPDIRECTDRAWSURFACE lpdds, LPRECT lprect)
{
	return extUnlockDir(4, pUnlock4, lpdds, lprect);
}

HRESULT WINAPI extUnlockDir1(LPDIRECTDRAWSURFACE lpdds, LPVOID lpvoid)
{
	return extUnlockDir(1, (Unlock4_Type)pUnlock1, lpdds, (LPRECT)lpvoid);
}

/* to do: instead of calling GDI GetDC, try to map GetDC with Lock and
ReleaseDC with Unlock, returning the surface memory ptr (???) as HDC
and avoiding the consistency check performed by surface::GetDC (why
should it bother if the screen is 32BPP and the surface is not??? */

HRESULT WINAPI extGetDC(LPDIRECTDRAWSURFACE lpdds, HDC FAR *pHDC)
{
	HRESULT res;
	BOOL IsPrim;

	IsPrim=dxw.IsAPrimarySurface(lpdds);
	OutTraceDDRAW("GetDC: lpdss=%x%s\n",lpdds, IsPrim?"(PRIM)":"");
	res=(*pGetDC)(lpdds, pHDC);

	if (res==DDERR_CANTCREATEDC && 
		(dxw.dwFlags1 & EMULATESURFACE) && 
		dxw.VirtualPixelFormat.dwRGBBitCount==8) {
		// for 8BPP palettized surfaces, connect them to either the ddraw emulated palette or the GDI emulated palette
		OutTraceDW("GetDC: adding 8BPP palette to surface lpdds=%x\n", lpdds);
		if(lpDDP==NULL){
			// should link here to the GDI palette? See Hyperblade....
			dxw.palNumEntries=256;
			res=(*pCreatePalette)(lpPrimaryDD, DDPCAPS_ALLOW256|DDPCAPS_8BIT|DDPCAPS_INITIALIZE, dxw.palPalEntry, &lpDDP, NULL);
			if (res) {
				OutTraceE("GetDC: CreatePalette ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
				return res;
			}
		}
		res=(*pSetPalette)(lpdds, lpDDP);
		if (res) {
			OutTraceE("GetDC: SetPalette ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			return res;
		}
		// retry ....
		res=(*pGetDC)(lpdds, pHDC);
	}

	OutTraceDDRAW("GetDC: res=%x hdc=%x\n",res, *pHDC);
	return res;
}

HRESULT WINAPI extReleaseDC(LPDIRECTDRAWSURFACE lpdds, HDC FAR hdc)
{
	HRESULT res;
	BOOL IsPrim;

	IsPrim=dxw.IsAPrimarySurface(lpdds);
	OutTraceDDRAW("ReleaseDC: lpdss=%x%s hdc=%x\n",lpdds, IsPrim?"(PRIM)":"", hdc);	
	res=(*pReleaseDC)(lpdds, hdc);
	if((IsPrim) && (dxw.dwFlags1 & EMULATESURFACE)) sBlt("ReleaseDC", lpdds, NULL, lpdds, NULL, 0, NULL, FALSE);
	if (res) OutTraceE("ReleaseDC: ERROR res=%x(%s)\n", res, ExplainDDError(res));
	if(dxw.dwFlags1 & SUPPRESSDXERRORS) res=DD_OK;
	return res;
}


HRESULT WINAPI extFlipToGDISurface(LPDIRECTDRAW lpdd)
{
	//HRESULT res;

	OutTraceDDRAW("FlipToGDISurface: lpdd=%x\n", lpdd);
	// to revise: so far, it seems the best thing to do is NOTHING, just return 0.
	//res=(*pFlipToGDISurface)(lpdd);
	//if (res) OutTraceE("FlipToGDISurface: ERROR res=%x(%s), skipping\n", res, ExplainDDError(res));
	// pretend you flipped anyway....
	return 0;
}

HRESULT WINAPI extGetGDISurface(LPDIRECTDRAW lpdd, LPDIRECTDRAWSURFACE *w)
{
	int res;

	// v2.02.31:
	// in EMULATED mode, should not return the actual ddraw primary surface, but the virtual one.
	if(dxw.dwFlags1 & EMULATESURFACE){
		*w=dxw.GetPrimarySurface();
		return DD_OK;
	}

	res=(*pGetGDISurface)(lpdd, w);
	if (res) {
		OutTraceE("GetGDISurface: ERROR lpdd=%x res=%x(%s)\n", lpdd, res, ExplainDDError(res));
	}
	else {
		OutTraceDDRAW("GetGDISurface: lpdd=%x w=%x\n", lpdd, *w);
	}

	return res;
}

// debug function to dump all video modes queried by the DirectDrav::EnumDisplayModes method

HRESULT WINAPI EnumModesCallbackDumper(LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext)
{
	OutTrace("EnumModesCallback:\n");
	OutTrace("\tdwSize=%d\n", lpDDSurfaceDesc->dwSize);
	OutTrace("\tdwFlags=%x(%s)\n", lpDDSurfaceDesc->dwFlags, ExplainFlags(lpDDSurfaceDesc->dwFlags));
 	OutTrace("\tdwHeight x dwWidth=(%d,%d)\n", lpDDSurfaceDesc->dwHeight, lpDDSurfaceDesc->dwWidth);
	OutTrace("\tlPitch=%d\n", lpDDSurfaceDesc->lPitch);
	OutTrace("\tdwBackBufferCount=%d\n", lpDDSurfaceDesc->dwBackBufferCount);
	OutTrace("\tdwRefreshRate=%d\n", lpDDSurfaceDesc->dwRefreshRate);
	OutTrace("\tlpSurface=%x\n", lpDDSurfaceDesc->lpSurface);
	OutTrace("\tddpfPixelFormat %s\n", DumpPixelFormat((LPDDSURFACEDESC2)lpDDSurfaceDesc));

#ifdef FULLHEXDUMP
	OutTrace("DDSurfaceDesc=");
	OutTraceHex((BYTE *)lpDDSurfaceDesc, sizeof(DDSURFACEDESC));
#endif

	return DDENUMRET_OK;
}

typedef HRESULT (WINAPI *EnumModesCallback_Type)(LPDDSURFACEDESC, LPVOID);
typedef struct {
	LPVOID lpContext; 
	EnumModesCallback_Type lpCallback; 
	DWORD dwWidth;
	DWORD dwHeight;
} NewContext_Type;

typedef struct {
	int w; 
	int h;
} SupportedRes_Type;

static SupportedRes_Type SupportedSVGARes[10]= {
	{320,200},
	{320,240},
	{512,384},		// needed by "Outcast" loading screen
	{640,400},
	{640,480},
	{720,480},
	{800,600},
	{1024,768},		// XGA
	{1280,800},		// WXGA
	{0,0}
};

static SupportedRes_Type SupportedHDTVRes[10]= {
	{640,360},		// nHD
	{720,480},		// DVD
	{720,576},		// DV-PAL
	{960,540},		// qHD
	{1176,1000},
	{1280,720},		// HD
	{1440,960},
	{1600,900},		// HD+
	{1920,1080},	// FHD
	{0,0}
};

static BOOL CheckResolutionLimit(LPDDSURFACEDESC lpDDSurfaceDesc)
{
	#define HUGE 100000
	DWORD maxw, maxh;
	maxw=HUGE; maxh=HUGE;
	switch(dxw.MaxScreenRes){
		case DXW_LIMIT_320x200: maxw=320; maxh=200; break;
		case DXW_LIMIT_640x480: maxw=640; maxh=480; break;
		case DXW_LIMIT_800x600: maxw=800; maxh=600; break;
		case DXW_LIMIT_1024x768: maxw=1024; maxh=768; break;
		case DXW_LIMIT_1280x960: maxw=1280; maxh=960; break;
	}
	if((lpDDSurfaceDesc->dwWidth > maxw) || (lpDDSurfaceDesc->dwHeight > maxh)){
		OutTraceDW("EnumDisplaySettings: hide device mode=(%d,%d)\n", maxw, maxh);
		return TRUE;
	}
	return FALSE;
}

HRESULT WINAPI myEnumModesFilterDirect(LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext)
{
	HRESULT res;
	SupportedRes_Type *SupportedRes;

	if ((((NewContext_Type *)lpContext)->dwHeight != lpDDSurfaceDesc->dwHeight) ||
		(((NewContext_Type *)lpContext)->dwWidth != lpDDSurfaceDesc->dwWidth)) return DDENUMRET_OK;

	if (IsDebug) EnumModesCallbackDumper(lpDDSurfaceDesc, NULL);

	if (dxw.dwFlags1 & PREVENTMAXIMIZE){
		// if PREVENTMAXIMIZE is set, don't let the caller know about forbidden screen settings.
		if((lpDDSurfaceDesc->dwHeight > dxw.GetScreenHeight()) ||
			(lpDDSurfaceDesc->dwWidth > dxw.GetScreenWidth())){
			OutTraceDW("EnumDisplayModes: skipping screen size=(%d,%d)\n", lpDDSurfaceDesc->dwHeight, lpDDSurfaceDesc->dwWidth);
			return DDENUMRET_OK;
		}
	}

	// tricky part: for each color depth related to current video resolution, fake each of the 
	// supported resolutions, unless is greater than maximum allowed

	SupportedRes = (dxw.dwFlags4 & SUPPORTHDTV) ? &SupportedHDTVRes[0] : &SupportedSVGARes[0];

	for (int ResIdx=0; SupportedRes[ResIdx].h; ResIdx++){
		lpDDSurfaceDesc->dwHeight=SupportedRes[ResIdx].h;
		lpDDSurfaceDesc->dwWidth=SupportedRes[ResIdx].w;
		if((dxw.dwFlags4 & LIMITSCREENRES) && CheckResolutionLimit(lpDDSurfaceDesc)) return DDENUMRET_OK;
		res=(*((NewContext_Type *)lpContext)->lpCallback)(lpDDSurfaceDesc, ((NewContext_Type *)lpContext)->lpContext);
		OutTraceDW("EnumDisplayModes(D): proposed size[%d]=(%d,%d) res=%x\n", ResIdx, SupportedRes[ResIdx].w, SupportedRes[ResIdx].h, res);
		if(res==DDENUMRET_CANCEL) break;
	}
	return res;
}

HRESULT WINAPI myEnumModesFilterNative(LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext)
{
	HRESULT res;

	if (IsDebug) EnumModesCallbackDumper(lpDDSurfaceDesc, NULL);

	if (dxw.dwFlags1 & PREVENTMAXIMIZE){
		// if PREVENTMAXIMIZE is set, don't let the caller know about forbidden screen settings.
		if((lpDDSurfaceDesc->dwHeight > dxw.GetScreenHeight()) ||
			(lpDDSurfaceDesc->dwWidth > dxw.GetScreenWidth())){
			OutTraceDW("EnumDisplayModes: skipping screen size=(%d,%d)\n", lpDDSurfaceDesc->dwHeight, lpDDSurfaceDesc->dwWidth);
			return DDENUMRET_OK;
		}
	}

	if((dxw.dwFlags4 & LIMITSCREENRES) && CheckResolutionLimit(lpDDSurfaceDesc)) return DDENUMRET_OK;
	res=(*((NewContext_Type *)lpContext)->lpCallback)(lpDDSurfaceDesc, ((NewContext_Type *)lpContext)->lpContext);
	OutTraceDW("EnumDisplayModes(D): native size=(%d,%d) res=%x\n", lpDDSurfaceDesc->dwWidth, lpDDSurfaceDesc->dwHeight, res);
	return res;
}

HRESULT WINAPI extEnumDisplayModes(EnumDisplayModes1_Type pEnumDisplayModes, LPDIRECTDRAW lpdd, DWORD dwflags, LPDDSURFACEDESC lpddsd, LPVOID lpContext, LPDDENUMMODESCALLBACK cb)
{
	HRESULT res;
	SupportedRes_Type *SupportedRes;
	NewContext_Type NewContext;

	OutTraceDDRAW("EnumDisplayModes(D): lpdd=%x flags=%x lpddsd=%x callback=%x\n", lpdd, dwflags, lpddsd, cb);

	if(dxw.dwFlags4 & NATIVERES){
		NewContext.dwWidth = 0;
		NewContext.dwHeight = 0;
		NewContext.lpContext=lpContext;
		NewContext.lpCallback=cb;
		res=(*pEnumDisplayModes)(lpdd, dwflags, lpddsd, &NewContext, myEnumModesFilterNative);
		if(res) OutTraceE("EnumDisplayModes(D): ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	// note: extEnumDisplayModes serves both the EnumDisplayModes and EnumDisplayModes2 interfaces:
	// they differ for the lpddsd argument that should point to either DDSURFACEDESC or DDSURFACEDESC2
	// structures, but unification is possible if the lpddsd->dwSize is properly set and is left untouched.

	if((dxw.dwFlags1 & EMULATESURFACE)){
		int SupportedDepths[5]={8,16,24,32,0};
		int ResIdx, DepthIdx;
		DDSURFACEDESC2 EmuDesc;
		DWORD dwSize;

		EmuDesc.dwRefreshRate = 0; 
		EmuDesc.ddpfPixelFormat.dwFlags = DDPF_RGB;
		if (lpddsd) dwSize=lpddsd->dwSize; // sizeof either DDSURFACEDESC or DDSURFACEDESC2 !!!
		else dwSize = sizeof(DDSURFACEDESC2);
		if(dwSize > sizeof(DDSURFACEDESC2)) dwSize=sizeof(DDSURFACEDESC2);
		memset(&EmuDesc, 0, dwSize);
		EmuDesc.dwSize=dwSize;
		EmuDesc.dwFlags=DDSD_PIXELFORMAT|DDSD_REFRESHRATE|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PITCH; 
		SupportedRes = (dxw.dwFlags4 & SUPPORTHDTV) ? &SupportedHDTVRes[0] : &SupportedSVGARes[0];
		for (ResIdx=0; SupportedRes[ResIdx].h; ResIdx++){
			EmuDesc.dwHeight=SupportedRes[ResIdx].h;
			EmuDesc.dwWidth=SupportedRes[ResIdx].w;
			EmuDesc.ddpfPixelFormat.dwSize=sizeof(DDPIXELFORMAT);
			EmuDesc.ddpfPixelFormat.dwFlags=DDPF_RGB;
			for (DepthIdx=0; SupportedDepths[DepthIdx]; DepthIdx++) {
				EmuDesc.ddpfPixelFormat.dwRGBBitCount=SupportedDepths[DepthIdx];
				EmuDesc.lPitch=SupportedRes[ResIdx].w * SupportedDepths[DepthIdx] / 8;
				FixPixelFormat(EmuDesc.ddpfPixelFormat.dwRGBBitCount, &(EmuDesc.ddpfPixelFormat));
				EnumModesCallbackDumper((LPDDSURFACEDESC)&EmuDesc, lpContext);
				res=(*cb)((LPDDSURFACEDESC)&EmuDesc, lpContext);
				if(res==DDENUMRET_CANCEL) break;
			}
			if(res==DDENUMRET_CANCEL) break;
		}
		res=DD_OK;
	}
	else{
		DDSURFACEDESC2 EmuDesc;
		memset(&EmuDesc, 0, sizeof(EmuDesc));
		EmuDesc.dwSize = sizeof(DDSURFACEDESC); // using release 1 type ....
	 	res=(*pGetDisplayMode)(lpdd, (LPDDSURFACEDESC)&EmuDesc);
		if(res==DDERR_GENERIC){ // Win8 missing support for old ddraw interface
			EmuDesc.dwSize = sizeof(DDSURFACEDESC2); // using release 2 type ....
	 		res=(*pGetDisplayMode)(lpdd, (LPDDSURFACEDESC)&EmuDesc);
		}
		if(res){
			OutTraceE("GetDisplayMode(D): ERROR res=%x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
			return res;
		}
		NewContext_Type NewContext;
		NewContext.dwWidth = EmuDesc.dwWidth;
		NewContext.dwHeight = EmuDesc.dwHeight;
		NewContext.lpContext=lpContext;
		NewContext.lpCallback=cb;
		res=(*pEnumDisplayModes)(lpdd, dwflags, lpddsd, &NewContext, myEnumModesFilterDirect);
	}
	if(res) OutTraceE("EnumDisplayModes(D): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extEnumDisplayModes1(LPDIRECTDRAW lpdd, DWORD dwflags, LPDDSURFACEDESC lpddsd, LPVOID lpContext, LPDDENUMMODESCALLBACK cb)
{
	return extEnumDisplayModes(pEnumDisplayModes1, lpdd, dwflags, lpddsd, lpContext, cb);
}

HRESULT WINAPI extEnumDisplayModes4(LPDIRECTDRAW lpdd, DWORD dwflags, LPDDSURFACEDESC2 lpddsd, LPVOID lpContext, LPDDENUMMODESCALLBACK2 cb)
{
	return extEnumDisplayModes((EnumDisplayModes1_Type)pEnumDisplayModes4, lpdd, dwflags, (LPDDSURFACEDESC)lpddsd, lpContext, (LPDDENUMMODESCALLBACK)cb);
}

HRESULT WINAPI extDuplicateSurface(LPDIRECTDRAW lpdd, LPDIRECTDRAWSURFACE s, LPDIRECTDRAWSURFACE *sp)
{
	int res;
	res=(*pDuplicateSurface)(lpdd, s, sp);
	if (res)
		OutTraceE("DuplicateSurface: ERROR dds=%x res=%x(%s)\n", s, res, ExplainDDError(res));
	else
		OutTraceDDRAW("DuplicateSurface: dds=%x pdds=%x\n", s, *sp);
	return res;
}

HRESULT WINAPI extGetPixelFormat(LPDIRECTDRAWSURFACE lpdds, LPDDPIXELFORMAT p)
{
	DWORD res;
	BOOL IsPrim;

	IsPrim=dxw.IsAPrimarySurface(lpdds);
	OutTraceDDRAW("GetPixelFormat: lpdds=%x%s\n", lpdds, IsPrim?"(PRIM)":"");
	res=(*pGetPixelFormat)(lpdds, p);
	if(res){
		OutTraceE("GetPixelFormat: ERROR res=%x(%s)\n", res, ExplainDDError(res));
	}
	else{
		OutTraceDDRAW("GetPixelFormat: Flags=%x(%s) FourCC=%x BitCount=%d RGBA=(%x,%x,%x,%x)\n", 
			p->dwFlags, ExplainPixelFormatFlags(p->dwFlags), p->dwFourCC, p->dwRGBBitCount, 
			p->dwRBitMask, p->dwGBitMask, p->dwBBitMask, p->dwRGBAlphaBitMask );
	}

	if ((dxw.dwFlags1 & EMULATESURFACE) && IsPrim){
		p->dwFlags = dxw.VirtualPixelFormat.dwFlags;
		p->dwRGBBitCount= dxw.VirtualPixelFormat.dwRGBBitCount;
		p->dwRBitMask = dxw.VirtualPixelFormat.dwRBitMask; 
		p->dwGBitMask = dxw.VirtualPixelFormat.dwGBitMask;
		p->dwBBitMask = dxw.VirtualPixelFormat.dwBBitMask;
		p->dwRGBAlphaBitMask = dxw.VirtualPixelFormat.dwRGBAlphaBitMask;
		OutTraceDW("GetPixelFormat: EMULATED BitCount=%d RGBA=(%x,%x,%x,%x)\n", 
			p->dwRGBBitCount, p->dwRBitMask, p->dwGBitMask, p->dwBBitMask, p->dwRGBAlphaBitMask );
	}

	return res;
}

HRESULT WINAPI extTestCooperativeLevel(LPDIRECTDRAW lpdd)
{
	HRESULT res;
	res=(*pTestCooperativeLevel)(lpdd);
	OutTraceB("TestCooperativeLevel: lpdd=%x res=%x(%s)\n", lpdd, res, ExplainDDError(res));
	if(res==DDERR_WRONGMODE) {
		res=((LPDIRECTDRAW7)lpdd)->RestoreAllSurfaces();
		if(res) OutTraceE("TestCooperativeLevel: RestoreAllSurfaces ERROR res=%x(%s)\n", res, ExplainDDError(res));
	}
	if(dxw.dwFlags1 & SUPPRESSDXERRORS) res=DD_OK;
	return res;
}

HRESULT WINAPI extReleaseS(LPDIRECTDRAWSURFACE lpdds)
{
	HRESULT res;
	BOOL IsPrim;
	BOOL IsBack;

	IsPrim=dxw.IsAPrimarySurface(lpdds);
	IsBack=dxw.IsABackBufferSurface(lpdds);

	if(IsBack && bDontReleaseBackBuffer){
		OutTraceDDRAW("Release(S): SKIP RELEASE on lpdds=%x\n", lpdds);
		res = 0;
	}
	else
		res = (*pReleaseS)(lpdds);

	OutTraceDDRAW("Release(S): lpdds=%x%s refcount=%d\n", lpdds, IsPrim?"(PRIM)":(IsBack?"(BACK)":""), res);
	if (res==0) { // common precondition
		// erase surface from primary or backbuffer list
		if(IsPrim || IsBack) dxw.MarkRegularSurface(lpdds);

		// when releasing primary surface, erase clipping region
		if(IsPrim && (dxw.dwFlags1 & CLIPCURSOR)) dxw.EraseClipCursor();
		
		// clear service surface pointers
		if (dxw.dwFlags1 & EMULATESURFACE) {
			if(lpdds==lpDDSEmu_Prim) {
				OutTraceDW("Release(S): Clearing lpDDSEmu_Prim pointer\n");
				lpDDSEmu_Prim=NULL;
			}
			if(lpdds==lpDDSEmu_Back) {
				OutTraceDW("Release(S): Clearing lpDDSEmu_Back pointer\n");
				lpDDSEmu_Back=NULL;
			}
			if(lpdds==dxw.lpDDSPrimHDC) {
				OutTraceDW("Release(S): Clearing lpDDSPrimHDC pointer\n");
				dxw.ResetPrimarySurface();
			}
		}
		if(lpdds==lpDDSBack) { // v2.02.38
			OutTraceDW("Release(S): Clearing lpDDSBack pointer\n");
			lpDDSBack = NULL;
		}
	}

	if(dxw.dwFlags4 & RETURNNULLREF) return 0;
	return res;
}

HRESULT WINAPI extSetColorKey(LPDIRECTDRAWSURFACE lpdds, DWORD flags, LPDDCOLORKEY lpDDColorKey)
{
	HRESULT res;
	BOOL IsPrim;
	IsPrim=dxw.IsAPrimarySurface(lpdds);
	if(IsTraceDDRAW){
		char sInfo[81];
		if (lpDDColorKey) sprintf(sInfo, "(L:%x,H:%x)",lpDDColorKey->dwColorSpaceLowValue, lpDDColorKey->dwColorSpaceHighValue);
		else strcpy(sInfo, "(NULL)");
		OutTrace("SetColorKey: lpdds=%x%s flags=%x(%s) colors=%s\n", 
			lpdds, (IsPrim ? "(PRIM)" : ""), flags, ExplainColorKeyFlag(flags), sInfo);
	}
		
	res=(*pSetColorKey)(lpdds, flags, lpDDColorKey);
	if(res) OutTraceE("SetColorKey: ERROR flags=%x lpdds=%x res=%x(%s)\n", 
		flags, lpdds, res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extGetColorKey(LPDIRECTDRAWSURFACE lpdds, DWORD flags, LPDDCOLORKEY lpDDColorKey)
{
	HRESULT res;
	BOOL IsPrim;
	IsPrim=dxw.IsAPrimarySurface(lpdds);
	OutTraceDDRAW("GetColorKey(S): lpdds=%x%s flags=%x(%s)\n", 
		lpdds, (IsPrim ? "(PRIM)" : ""), flags, ExplainColorKeyFlag(flags));
	res=(*pGetColorKey)(lpdds, flags, lpDDColorKey);
	if(res) 
		OutTraceE("GetColorKey: ERROR lpdds=%x flags=%x res=%x(%s)\n", lpdds, flags, res, ExplainDDError(res));
	else
		OutTraceDDRAW("GetColorKey: colors=(L:%x,H:%x)\n", 
			lpDDColorKey->dwColorSpaceLowValue, lpDDColorKey->dwColorSpaceHighValue);
	return res;
}

HRESULT WINAPI extEnumAttachedSurfaces(LPDIRECTDRAWSURFACE lpdds, LPVOID lpContext, LPDDENUMSURFACESCALLBACK lpEnumSurfacesCallback)
{
	HRESULT res;
	BOOL IsPrim;

	IsPrim=dxw.IsAPrimarySurface(lpdds);

	OutTraceDDRAW("EnumAttachedSurfaces: lpdds=%x%s Context=%x Callback=%x\n", 
		lpdds, (IsPrim ? "(PRIM)":""), lpContext, lpEnumSurfacesCallback);

	if (IsPrim){
		// A Primary surface has not backbuffer attached surfaces actually, 
		// so don't rely on ddraw and call the callback function directly.
		// Needed to make Nox working.
		DDSURFACEDESC2 ddsd;
		// first, call hooked function
		res=(*pEnumAttachedSurfaces)(lpdds, lpContext, lpEnumSurfacesCallback);
		if (res) 
			OutTraceE("EnumAttachedSurfaces: ERROR %x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		if(lpDDSBack){
			ddsd.dwSize=Set_dwSize_From_Surface(lpDDSBack);
			res=lpDDSBack->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd);
			if(res){
				OutTraceE("EnumAttachedSurfaces: GetSurfaceDesc ERROR %x(%s)\n",
				res, ExplainDDError(res));
				return res;
			}
			res=(lpEnumSurfacesCallback)(lpDDSBack, (LPDDSURFACEDESC)&ddsd, lpContext);
			OutTraceDW("EnumSurfacesCallback: on DDSBack res=%x(%s)\n", res, ExplainDDError(res));
		}
		res=0; // for Black Dahlia
	}
	else {
		res=(*pEnumAttachedSurfaces)(lpdds, lpContext, lpEnumSurfacesCallback);
		if (res) 
			OutTraceE("EnumAttachedSurfaces: ERROR %x(%s) at %d\n", res, ExplainDDError(res), __LINE__);
	}
	return res;
}

HRESULT WINAPI extAddAttachedSurface(LPDIRECTDRAWSURFACE lpdds, LPDIRECTDRAWSURFACE lpddsadd)
{
	HRESULT res;
	BOOL IsPrim, IsBack;

	// You can add backbuffers to primary surfaces to join the flipping chain, but you can't do that
	// to an emulated primary surface, and you receive a DDERR_CANNOTATTACHSURFACE error code.
	// In that case, it's worth to try to emulate the attach, and since the Flip method is emulated, 
	// just remember this for further handling in the Flip operation.
	// But beware: this holds to BACKBUFFER surfaces only, and NOT for attached ZBUFFERS or similar!

	IsPrim=dxw.IsAPrimarySurface(lpdds);
	IsBack=dxw.IsABackBufferSurface(lpdds);
	OutTraceDDRAW("AddAttachedSurface: lpdds=%x%s lpddsadd=%x%s\n", lpdds, IsPrim?"(PRIM)":(IsBack?"(BACK)":""), lpddsadd, (lpddsadd==lpDDZBuffer)?"(ZBUF)":"");

	//if(!lpddsadd) return DDERR_CANNOTATTACHSURFACE; // to avoid a crash...
	res=(*pAddAttachedSurface)(lpdds, lpddsadd);
	if (res) {
		HRESULT sdres;
		DDSURFACEDESC2 sd;
		sd.dwSize=Set_dwSize_From_Surface(lpddsadd);
		sdres=lpddsadd->GetSurfaceDesc((DDSURFACEDESC *)&sd);
		if (sdres) 
			OutTraceE("AddAttachedSurface: GetSurfaceDesc ERROR res=%x at %d\n", sdres, __LINE__);
		else
			OutTraceDW("AddAttachedSurface: GetSurfaceDesc dwCaps=%x(%s)\n", 
				sd.ddsCaps.dwCaps, ExplainDDSCaps(sd.ddsCaps.dwCaps));
		if (IsPrim){
			if (sd.ddsCaps.dwCaps & DDSCAPS_BACKBUFFER)
			if ((dxw.dwFlags1 & EMULATESURFACE) && (res==DDERR_CANNOTATTACHSURFACE) ||
				(res==DDERR_NOEXCLUSIVEMODE))
			OutTraceDW("AddAttachedSurface: emulating BACKBUFFER attach on PRIMARY\n");
			lpDDSBack=lpddsadd;
			if (pAddRefS) (*pAddRefS)(lpdds);
			res=DD_OK;
		}
		else if (IsBack) {
			// v2.02.13: emulate ZBUFFER attach to backbuffer: do nothing and return OK
			// this trick makes at least "Nocturne" work also in emulated mode when hardware acceleration
			// is set in the game "Options" menu. 
			if (sd.ddsCaps.dwCaps & DDSCAPS_ZBUFFER) // DDSCAPS_BACKBUFFER for double buffering ???
			if ((dxw.dwFlags1 & EMULATESURFACE) && (res==DDERR_CANNOTATTACHSURFACE)){
				OutTraceDW("AddAttachedSurface: emulating ZBUFFER attach on BACKBUFFER\n");
				if (pAddRefS) (*pAddRefS)(lpdds);
				res=DD_OK;
			}
		}
	}
	if (res) OutTraceE("AddAttachedSurface: ERROR %x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extDeleteAttachedSurface(LPDIRECTDRAWSURFACE lpdds,  DWORD dwflags, LPDIRECTDRAWSURFACE lpddsdel)
{
	HRESULT res;
	OutTraceDDRAW("DeleteAttachedSurface: lpdds=%x flags=%x lpddsdel=%x\n", lpdds, dwflags, lpddsdel);
	res=(*pDeleteAttachedSurface)(lpdds, dwflags, lpddsdel);
	if(res) OutTraceE("DeleteAttachedSurface: ERROR %x(%s)\n", res, ExplainDDError(res));
	if (res && (lpddsdel==lpDDSBack)){
		OutTraceDW("DeleteAttachedSurface: emulating surface detach lpdds=%x\n", lpddsdel);
		lpDDSBack->Release(); // GHO TRY
		lpDDSBack=NULL;
		res=0;
	}
	return res;
}

HRESULT WINAPI cbDump(LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext)
{
	OutTraceDW("EnumDisplayModes: CALLBACK lpdds=%x Context=%x Caps=%x(%s)\n", 
		lpDDSurfaceDesc, lpContext,
		lpDDSurfaceDesc->ddsCaps.dwCaps, ExplainDDSCaps(lpDDSurfaceDesc->ddsCaps.dwCaps));
	return 1;
}

HRESULT WINAPI extGetCapsS(int dxInterface, GetCapsS_Type pGetCapsS, LPDIRECTDRAWSURFACE lpdds, LPDDSCAPS caps)
{
	HRESULT res;
	BOOL IsPrim, IsBack, IsFixed;
	IsPrim=dxw.IsAPrimarySurface(lpdds);
	IsBack=dxw.IsABackBufferSurface(lpdds);
	IsFixed=FALSE;
	OutTraceDDRAW("GetCaps(S%d): lpdds=%x%s, lpcaps=%x\n", dxInterface, lpdds, IsPrim?"(PRIM)":"", caps);
	res=(*pGetCapsS)(lpdds, caps);
	if(res) 
		OutTraceE("GetCaps(S%d): ERROR %x(%s)\n", dxInterface, res, ExplainDDError(res));
	else
		OutTraceDDRAW("GetCaps(S%d): lpdds=%x caps=%x(%s)\n", dxInterface, lpdds, caps->dwCaps, ExplainDDSCaps(caps->dwCaps));

	if (IsPrim) {
		OutTraceDW("GetCaps(S%d): fixing PRIMARY surface\n", dxInterface);
		IsFixed=TRUE;
		caps->dwCaps |= DDSD_Prim.ddsCaps.dwCaps;
		caps->dwCaps |= DDSCAPS_PRIMARYSURFACE|DDSCAPS_FLIP|DDSCAPS_FRONTBUFFER|DDSCAPS_VIDEOMEMORY|DDSCAPS_VISIBLE; // primary surfaces must be this way
		caps->dwCaps &= ~(DDSCAPS_SYSTEMMEMORY|DDSCAPS_OFFSCREENPLAIN); // primary surfaces can't be this way
		}

	if (IsBack) {
		OutTraceDW("GetCaps(S%d): fixing BACKBUFFER surface\n", dxInterface);
		IsFixed=TRUE;
		caps->dwCaps |= (DDSCAPS_BACKBUFFER|DDSCAPS_VIDEOMEMORY|DDSCAPS_LOCALVIDMEM); // you never know....
		caps->dwCaps &= ~(DDSCAPS_SYSTEMMEMORY|DDSCAPS_OFFSCREENPLAIN); // backbuffer surfaces can't be this way
	}

	if ((caps->dwCaps & DDSCAPS_ZBUFFER) || (lpdds == lpDDZBuffer)){
		OutTraceDW("GetCaps(S%d): fixing ZBUFFER surface\n", dxInterface);
		IsFixed=TRUE;
		if (DDZBufferCaps & DDSCAPS_SYSTEMMEMORY){
			caps->dwCaps |= (DDSCAPS_ZBUFFER|DDSCAPS_SYSTEMMEMORY); 
			caps->dwCaps &= ~(DDSCAPS_VIDEOMEMORY|DDSCAPS_LOCALVIDMEM); 
		}
		else {
			caps->dwCaps |= (DDSCAPS_ZBUFFER|DDSCAPS_VIDEOMEMORY|DDSCAPS_LOCALVIDMEM); 
			caps->dwCaps &= ~DDSCAPS_SYSTEMMEMORY; 
		}
	}

	if(IsFixed) OutTraceDW("GetCaps(S%d): lpdds=%x FIXED caps=%x(%s)\n", dxInterface, lpdds, caps->dwCaps, ExplainDDSCaps(caps->dwCaps));
	return res;
}

HRESULT WINAPI extGetCaps1S(LPDIRECTDRAWSURFACE lpdds, LPDDSCAPS caps)
{
	return extGetCapsS(1, pGetCaps1S, lpdds, caps);
}
HRESULT WINAPI extGetCaps2S(LPDIRECTDRAWSURFACE lpdds, LPDDSCAPS caps)
{
	return extGetCapsS(2, pGetCaps2S, lpdds, caps);
}
HRESULT WINAPI extGetCaps3S(LPDIRECTDRAWSURFACE lpdds, LPDDSCAPS caps)
{
	return extGetCapsS(3, pGetCaps3S, lpdds, caps);
}
HRESULT WINAPI extGetCaps4S(LPDIRECTDRAWSURFACE lpdds, LPDDSCAPS2 caps)
{
	return extGetCapsS(4, (GetCapsS_Type)pGetCaps4S, lpdds, (LPDDSCAPS)caps);
}
HRESULT WINAPI extGetCaps7S(LPDIRECTDRAWSURFACE lpdds, LPDDSCAPS2 caps)
{
	return extGetCapsS(7, (GetCapsS_Type)pGetCaps7S, lpdds, (LPDDSCAPS)caps);
}

ULONG WINAPI extReleaseD(LPDIRECTDRAW lpdd)
{
	ULONG ActualRef;
	LONG VirtualRef;
	int dxversion;

	dxversion=lpddHookedVersion(lpdd); // must be called BEFORE releasing the session!!
	OutTraceDDRAW("Release(D): lpdd=%x dxversion=%d\n", lpdd, dxversion);
	
	ActualRef=(*pReleaseD)(lpdd);
	VirtualRef=(LONG)ActualRef;
	OutTraceDW("Release(D): lpdd=%x service_lpdd=%x ref=%d\n", lpdd, lpPrimaryDD, ActualRef);

	if (lpdd == lpPrimaryDD) { // v2.1.87: fix for Dungeon Keeper II
		if(dxw.dwFlags4 & FIXREFCOUNTER){
			// v2.02.41: fix the ref counter to sumulate the unwindowed original situation
			--VirtualRef; // why ????
			if(lpDDSBack) --VirtualRef;
			if(dxw.dwFlags1 & EMULATESURFACE){
				if(lpDDSEmu_Prim) --VirtualRef;
				if(lpDDSEmu_Back) --VirtualRef;
			}
			if(VirtualRef<0) VirtualRef=0;
			OutTraceDW("Release(D): fixed ref counter %d->%d\n", ActualRef, VirtualRef);
		}
		if((dxversion<4) && (ActualRef==0)){
			// directdraw old versions automatically free all linked objects when the parent session is closed.
			OutTraceDW("Release(D): RefCount=0 - service object RESET condition\n");
			lpDDSEmu_Prim=NULL;
			lpDDSEmu_Back=NULL;
			lpDDP=NULL;
			lpPrimaryDD=NULL; // v2.02.31
			if(lpBackBufferDD==lpdd){ 
				lpBackBufferDD=NULL;
				lpDDSBack=NULL; // beware: Silent Hunter II seems to require the backbuffer .... 
			}
		}
	}

	// when lpdd session is closed (ref==0) the system restores the default color depth
	// so if FORCE16BPP is set, dxwnd must restore the 16BPP value
	//extern void SwitchTo16BPP();
	//if((ActualRef==0) && (dxw.dwFlags3 & FORCE16BPP)) SwitchTo16BPP();

	OutTraceDDRAW("Release(D): lpdd=%x ref=%x\n", lpdd, VirtualRef);

	if(dxw.dwFlags4 & RETURNNULLREF) return 0;
	return (ULONG)VirtualRef;
}

HRESULT WINAPI extCreateClipper(LPDIRECTDRAW lpdd, DWORD dwflags, 
		LPDIRECTDRAWCLIPPER FAR* lplpDDClipper, IUnknown FAR* pUnkOuter)
{
	HRESULT res;
	OutTraceDDRAW("CreateClipper: lpdd=%x flags=%x\n", lpdd, dwflags);
	res=(*pCreateClipper)(lpdd, dwflags, lplpDDClipper, pUnkOuter);
	if(res) {
		OutTraceE("CreateClipper: ERROR res=%x(%s)\n", lpdd, res, ExplainDDError(res));
		return res;
	}
	HookDDClipper(lplpDDClipper);
	return res;
}

HRESULT WINAPI extReleaseC(LPDIRECTDRAWCLIPPER lpddClip)
{
	ULONG ref;

	ref = (*pReleaseC)(lpddClip);

	OutTraceDDRAW("Release(C): PROXED lpddClip=%x ref=%x\n", lpddClip, ref);
	return ref;
}

HRESULT WINAPI extGetSurfaceDesc(GetSurfaceDesc_Type pGetSurfaceDesc, LPDIRECTDRAWSURFACE lpdds, LPDDSURFACEDESC lpddsd)
{
	HRESULT res;
	BOOL IsPrim, IsBack, IsFixed;
	IsPrim=dxw.IsAPrimarySurface(lpdds);
	IsBack=dxw.IsABackBufferSurface(lpdds);
	IsFixed=FALSE;

	if (!pGetSurfaceDesc) {
		OutTraceE("GetSurfaceDesc: ERROR no hooked function\n");
		return DDERR_INVALIDPARAMS;
	}

	res=(*pGetSurfaceDesc)(lpdds, lpddsd);
	OutTraceDDRAW("GetSurfaceDesc: %slpdds=%x%s res=%x(%s)\n", 
		res?"ERROR ":"", lpdds, IsPrim?"(PRIM)":(IsBack?"(BACK)":""), res, ExplainDDError(res));
	if(res) {
		OutTraceE("GetSurfaceDesc: ERROR err=%d(%s) at %d\n", res, ExplainDDError(res), __LINE__);
		return res;
	}

	OutTraceDDRAW("GetSurfaceDesc: lpdds=%x %s\n", lpdds, LogSurfaceAttributes(lpddsd, "GetSurfaceDesc", __LINE__));

	if (IsPrim) {
		OutTraceDW("GetSurfaceDesc: fixing PRIMARY surface\n");
		IsFixed=TRUE;
		if (dxw.dwFlags1 & EMULATESURFACE) lpddsd->ddpfPixelFormat = dxw.VirtualPixelFormat;
		lpddsd->ddsCaps.dwCaps |= DDSD_Prim.ddsCaps.dwCaps;
		lpddsd->ddsCaps.dwCaps |= (DDSCAPS_PRIMARYSURFACE|DDSCAPS_FLIP|DDSCAPS_FRONTBUFFER|DDSCAPS_VIDEOMEMORY|DDSCAPS_VISIBLE); // primary surfaces must be this way
		lpddsd->ddsCaps.dwCaps &= ~(DDSCAPS_SYSTEMMEMORY|DDSCAPS_OFFSCREENPLAIN); // primary surfaces can't be this way
		lpddsd->dwBackBufferCount=DDSD_Prim.dwBackBufferCount;
		lpddsd->dwHeight=dxw.GetScreenHeight();
		lpddsd->dwWidth=dxw.GetScreenWidth();
	}

	if (IsBack) {
		OutTraceDW("GetSurfaceDesc: fixing BACKBUFFER surface\n");
		IsFixed=TRUE;
		lpddsd->ddsCaps.dwCaps |= (DDSCAPS_BACKBUFFER|DDSCAPS_VIDEOMEMORY|DDSCAPS_LOCALVIDMEM); // you never know....
		lpddsd->ddsCaps.dwCaps &= ~(DDSCAPS_SYSTEMMEMORY|DDSCAPS_OFFSCREENPLAIN); // primary surfaces can't be this way
	}

	if (lpddsd->ddsCaps.dwCaps & DDSCAPS_ZBUFFER) {
		OutTraceDW("GetSurfaceDesc: fixing ZBUFFER surface\n");
		IsFixed=TRUE;
		if (DDZBufferCaps & DDSCAPS_SYSTEMMEMORY){
			lpddsd->ddsCaps.dwCaps |= (DDSCAPS_ZBUFFER|DDSCAPS_SYSTEMMEMORY); 
			lpddsd->ddsCaps.dwCaps &= ~(DDSCAPS_VIDEOMEMORY|DDSCAPS_LOCALVIDMEM); 
		}
		else {
			lpddsd->ddsCaps.dwCaps |= (DDSCAPS_ZBUFFER|DDSCAPS_VIDEOMEMORY|DDSCAPS_LOCALVIDMEM); 
			lpddsd->ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY; 
		}
	}

	if(IsFixed){
		OutTraceDW("GetSurfaceDesc: lpdds=%x %s\n", lpdds, LogSurfaceAttributes(lpddsd, "GetSurfaceDesc [FIXED]", __LINE__));
	}

	return DD_OK;
}

// Beware: despite the surface version, some game (The Sims!!!) intentionally uses a different dwSize, so that
// you shouldn't reset the value

HRESULT WINAPI extGetSurfaceDesc1(LPDIRECTDRAWSURFACE lpdds, LPDDSURFACEDESC lpddsd)
{
	if (!lpddsd->dwSize) lpddsd->dwSize = sizeof(DDSURFACEDESC); // enforce correct dwSize value
	switch(lpddsd->dwSize){
		case sizeof(DDSURFACEDESC):
		if (pGetSurfaceDesc1) return extGetSurfaceDesc(pGetSurfaceDesc1, lpdds, lpddsd);
		//else return lpdds->GetSurfaceDesc((LPDDSURFACEDESC)lpddsd);
		if (pGetSurfaceDesc4) return extGetSurfaceDesc((GetSurfaceDesc_Type)pGetSurfaceDesc4, (LPDIRECTDRAWSURFACE)lpdds, (LPDDSURFACEDESC)lpddsd);
		break;
		case sizeof(DDSURFACEDESC2):
		if (pGetSurfaceDesc4) return extGetSurfaceDesc((GetSurfaceDesc_Type)pGetSurfaceDesc4, (LPDIRECTDRAWSURFACE)lpdds, (LPDDSURFACEDESC)lpddsd);
		//else return lpdds->GetSurfaceDesc((LPDDSURFACEDESC)lpddsd);
		break;
		default:
		OutTraceDW("GetSurfaceDesc: ASSERT - bad dwSize=%d lpdds=%x at %d\n", lpddsd->dwSize, lpdds, __LINE__);
		return DDERR_INVALIDOBJECT;
		break;
	}
	OutTraceDW("GetSurfaceDesc: ASSERT - missing hook lpdds=%x dwSize=%d(%s) at %d\n", 
		lpdds, lpddsd->dwSize, lpddsd->dwSize==sizeof(DDSURFACEDESC)?"DDSURFACEDESC":"DDSURFACEDESC2", __LINE__);
	return DDERR_INVALIDOBJECT;
}

HRESULT WINAPI extGetSurfaceDesc2(LPDIRECTDRAWSURFACE2 lpdds, LPDDSURFACEDESC2 lpddsd)
{
	if (!lpddsd->dwSize) lpddsd->dwSize = sizeof(DDSURFACEDESC2); // enforce correct dwSize value
	switch(lpddsd->dwSize){
		case sizeof(DDSURFACEDESC):
		if (pGetSurfaceDesc1) return extGetSurfaceDesc(pGetSurfaceDesc1, (LPDIRECTDRAWSURFACE)lpdds, (LPDDSURFACEDESC)lpddsd);
		//else return lpdds->GetSurfaceDesc((LPDDSURFACEDESC)lpddsd);
		break;
		case sizeof(DDSURFACEDESC2):
		if (pGetSurfaceDesc4) return extGetSurfaceDesc((GetSurfaceDesc_Type)pGetSurfaceDesc4, (LPDIRECTDRAWSURFACE)lpdds, (LPDDSURFACEDESC)lpddsd);
		//else return lpdds->GetSurfaceDesc((LPDDSURFACEDESC)lpddsd);
		break;
		default:
		OutTraceDW("GetSurfaceDesc: ASSERT - bad dwSize=%d lpdds=%x at %d\n", lpddsd->dwSize, lpdds, __LINE__);
		return DDERR_INVALIDOBJECT;
	}
	OutTraceDW("GetSurfaceDesc: ASSERT - missing hook lpdds=%x dwSize=%d(%s) at %d\n", 
		lpdds, lpddsd->dwSize, lpddsd->dwSize==sizeof(DDSURFACEDESC)?"DDSURFACEDESC":"DDSURFACEDESC2", __LINE__);
	return DDERR_INVALIDOBJECT;
}

HRESULT WINAPI extGetSurfaceDesc7(LPDIRECTDRAWSURFACE2 lpdds, LPDDSURFACEDESC2 lpddsd)
{
	if (!lpddsd->dwSize) lpddsd->dwSize = sizeof(DDSURFACEDESC2); // enforce correct dwSize value
	switch(lpddsd->dwSize){
		case sizeof(DDSURFACEDESC):
		if (pGetSurfaceDesc1) return extGetSurfaceDesc(pGetSurfaceDesc1, (LPDIRECTDRAWSURFACE)lpdds, (LPDDSURFACEDESC)lpddsd);
		break;
		case sizeof(DDSURFACEDESC2):
		if (pGetSurfaceDesc7) return extGetSurfaceDesc((GetSurfaceDesc_Type)pGetSurfaceDesc7, (LPDIRECTDRAWSURFACE)lpdds, (LPDDSURFACEDESC)lpddsd);
		break;
		default:
		OutTraceDW("GetSurfaceDesc: ASSERT - bad dwSize=%d lpdds=%x at %d\n", lpddsd->dwSize, lpdds, __LINE__);
		return DDERR_INVALIDOBJECT;
	}
	OutTraceDW("GetSurfaceDesc: ASSERT - missing hook lpdds=%x dwSize=%d(%s) at %d\n", 
		lpdds, lpddsd->dwSize, lpddsd->dwSize==sizeof(DDSURFACEDESC)?"DDSURFACEDESC":"DDSURFACEDESC2", __LINE__);
	return DDERR_INVALIDOBJECT;
}

HRESULT WINAPI extReleaseP(LPDIRECTDRAWPALETTE lpddPalette)
{
	ULONG ref;
	
	ref = (*pReleaseP)(lpddPalette);

	OutTraceDDRAW("Release(P): lpddPalette=%x ref=%x\n", lpddPalette, ref);
	if(lpddPalette==lpDDP && ref==0){
		OutTraceDW("Release(P): clearing lpDDP=%x->NULL\n", lpDDP);
		lpDDP=NULL;
	}
	return ref;
}

BOOL WINAPI DDEnumerateCallbackFilter(GUID *lpGuid, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext)
{
	BOOL res;
	typedef struct {LPDDENUMCALLBACK lpCallback; LPVOID lpContext;} Context_Type;
	Context_Type *p=(Context_Type *)lpContext;
	OutTraceDW("DDEnumerateCallback: guid=%x DriverDescription=\"%s\" DriverName=\"%s\" Context=%x\n", 
		lpGuid, lpDriverDescription, lpDriverName, p->lpContext);
	if((lpGuid==NULL) || !(dxw.dwFlags2 & HIDEMULTIMONITOR)) res=(*p->lpCallback)(lpGuid, lpDriverDescription, lpDriverName, p->lpContext);
	if((lpGuid==NULL) && (dxw.dwFlags2 & HIDEMULTIMONITOR)) res=FALSE;
	OutTraceDW("DDEnumerateCallback: res=%x(%s)\n", res, res?"continue":"break");
	return res;
}

BOOL WINAPI DDEnumerateCallbackExFilter(GUID *lpGuid, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext, HMONITOR hm)
{
	BOOL res;
	typedef struct {LPDDENUMCALLBACKEX lpCallback; LPVOID lpContext;} Context_Type;
	Context_Type *p=(Context_Type *)lpContext;
	OutTraceDW("DDEnumerateCallbackEx: guid=%x DriverDescription=\"%s\" DriverName=\"%s\" Context=%x hm=%x\n", 
		lpGuid, lpDriverDescription, lpDriverName, lpContext, hm);
	res=TRUE;
	if((lpGuid==NULL) || !(dxw.dwFlags2 & HIDEMULTIMONITOR)) res=(*p->lpCallback)(lpGuid, lpDriverDescription, lpDriverName, p->lpContext, hm);
	if((lpGuid==NULL) && (dxw.dwFlags2 & HIDEMULTIMONITOR)) res=FALSE;
	OutTraceDW("DDEnumerateCallbackEx: res=%x(%s)\n", res, res?"continue":"break");
	return res;
}

HRESULT WINAPI extDirectDrawEnumerate(LPDDENUMCALLBACK lpCallback, LPVOID lpContext)
{
	HRESULT ret;
	OutTraceDDRAW("DirectDrawEnumerate: lpCallback=%x lpContext=%x\n", lpCallback, lpContext);
	if((dxw.dwFlags2 & HIDEMULTIMONITOR) || (dxw.dwTFlags & OUTDEBUG)){
		struct {LPDDENUMCALLBACK lpCallback; LPVOID lpContext;} myContext;
		myContext.lpCallback=lpCallback;
		myContext.lpContext=lpContext;
		ret=(*pDirectDrawEnumerate)(DDEnumerateCallbackFilter, (LPVOID)&myContext);
	}
	else
		ret=(*pDirectDrawEnumerate)(lpCallback, lpContext);
	if(ret) OutTraceE("DirectDrawEnumerate: ERROR res=%x(%s)\n", ret, ExplainDDError(ret));
	return ret;
}

HRESULT WINAPI extDirectDrawEnumerateEx(LPDDENUMCALLBACKEX lpCallback, LPVOID lpContext, DWORD dwFlags)
{
	HRESULT ret;
	OutTraceDDRAW("DirectDrawEnumerateEx: lpCallback=%x lpContext=%x Flags=%x(%s)\n", 
		lpCallback, lpContext, dxw.dwFlags1, ExplainDDEnumerateFlags(dwFlags));
	if((dxw.dwFlags2 & HIDEMULTIMONITOR) || (dxw.dwTFlags & OUTDEBUG)){
		struct {LPDDENUMCALLBACKEX lpCallback; LPVOID lpContext;} myContext;
		myContext.lpCallback=lpCallback;
		myContext.lpContext=lpContext;
		ret=(*pDirectDrawEnumerateEx)(DDEnumerateCallbackExFilter, (LPVOID)&myContext, dwFlags);
	}
	else
		ret=(*pDirectDrawEnumerateEx)(lpCallback, lpContext, dwFlags);
	if(ret) OutTraceE("DirectDrawEnumerateEx: ERROR res=%x(%s)\n", ret, ExplainDDError(ret));
	if(dxw.dwFlags1 & SUPPRESSDXERRORS) ret=0;
	return ret;
}

HRESULT WINAPI extDDGetGammaRamp(LPDIRECTDRAWSURFACE lpdds, DWORD dwFlags, LPDDGAMMARAMP lpgr)
{
	HRESULT ret;
	OutTraceDDRAW("GetGammaRamp: dds=%x dwFlags=%x\n", lpdds, dwFlags);

	if(dxw.IsAPrimarySurface(lpdds)) lpdds=lpDDSEmu_Prim;
	
	ret=(*pDDGetGammaRamp)(lpdds, dwFlags, lpgr);
	if(ret) {
		OutTraceE("GetGammaRamp: ERROR res=%x(%s)\n", ret, ExplainDDError(ret));
		if(dxw.dwFlags1 & SUPPRESSDXERRORS) {
			// clear the error code, and provide a reasonable gamma ramp array
			for(int i=0; i<256; i++) lpgr->red[i]=lpgr->green[i]=lpgr->blue[i]=(i * 0x100);
			ret=0;
		}
	}
	else{
		if(IsDebug){
			OutTrace("GetGammaRamp: RGB="); 
			for(int i=0; i<256; i++) OutTrace("(%x,%x,%x)", lpgr->red[i], lpgr->green[i], lpgr->blue[i]);
			OutTrace("\n");
		}
	}
	return ret;
}

HRESULT WINAPI extDDSetGammaRamp(LPDIRECTDRAWSURFACE lpdds, DWORD dwFlags, LPDDGAMMARAMP lpgr)
{
	HRESULT ret;
	OutTraceDDRAW("SetGammaRamp: dds=%x dwFlags=%x\n", lpdds, dwFlags);
	if(IsDebug){
		OutTrace("GetGammaRamp: RGB="); 
		for(int i=0; i<256; i++) OutTrace("(%x,%x,%x)", lpgr->red[i], lpgr->green[i], lpgr->blue[i]);
		OutTrace("\n");
	}
	if (dxw.dwFlags2 & DISABLEGAMMARAMP) return DD_OK;
	ret=(*pDDSetGammaRamp)(lpdds, dwFlags, lpgr);
	if(ret) OutTraceE("SetGammaRamp: ERROR res=%x(%s)\n", ret, ExplainDDError(ret));
	if(dxw.dwFlags1 & SUPPRESSDXERRORS) ret = DD_OK;
	return ret;
}

HRESULT WINAPI extGetAvailableVidMem(LPDIRECTDRAW lpdd, LPDDSCAPS lpDDSCaps, LPDWORD lpdwTotal, LPDWORD lpdwFree, GetAvailableVidMem_Type pGetAvailableVidMem)
{
	HRESULT res; 
	//const DWORD dwMaxMem = 0x7FFFF000;
        // v03.01.01: limit to smaller value to allow "Breath of Fire IV" card detection
	const DWORD dwMaxMem = 0x70000000; 
	const DWORD dwHugeMem = 0xF0000000; 
	OutTraceDDRAW("GetAvailableVidMem(D): lpdd=%x\n", lpdd);
	res=(*pGetAvailableVidMem)(lpdd, lpDDSCaps, lpdwTotal, lpdwFree);
	if(res){
		if((dxw.dwFlags3 & FORCESHEL) && (res==DDERR_NODIRECTDRAWHW)){
			// fake some video memory....
			OutTraceDW("GetAvailableVidMem(D): FORCESHEL mode Total=Free=%x\n", dwMaxMem);
			if(lpdwTotal) *lpdwTotal = dwMaxMem; 
			if(lpdwFree) *lpdwFree = dwMaxMem;
			return DD_OK;
		}
		OutTraceE("GetAvailableVidMem(D): ERROR res=%x(%s)\n", res, ExplainDDError(res));
		return res; 
	}

		OutTraceDW("GetAvailableVidMem(D): DDSCaps=%x(%s) Total=%x Free=%x\n", 
			*lpDDSCaps, ExplainDDSCaps(lpDDSCaps->dwCaps), lpdwTotal?*lpdwTotal:0, lpdwFree?*lpdwFree:0);

	if(!(dxw.dwFlags2 & LIMITRESOURCES)) return res;
	
	// simulate a value overflow condition
	if(dxw.dwFlags5 & STRESSRESOURCES){
		if(lpdwTotal) *lpdwTotal = dwHugeMem;
		if(lpdwFree) *lpdwFree = dwHugeMem;
		return DD_OK;
	}
	
	// check for memory value overflow - see "Mageslayer" and "Take no Prisoners"
			DWORD dwLocalTotal;
			if(lpdwTotal == NULL) {
				lpdwTotal = &dwLocalTotal; // point to usable memory....
				res=(*pGetAvailableVidMem)(lpdd, lpDDSCaps, lpdwTotal, lpdwFree); // do it again to get total memory
			}
			if(*lpdwTotal > dwMaxMem){
				if(lpdwFree != NULL){
					DWORD dwDiff = *lpdwTotal - *lpdwFree;
					if(dwDiff > dwMaxMem){
						*lpdwFree = dwMaxMem;
					}
					else{
						*lpdwFree = dwMaxMem - dwDiff;
					}
				}
				*lpdwTotal = dwMaxMem;
				OutTraceDW("GetAvailableVidMem(D): FIXED Total=%x Free=%x\n", *lpdwTotal, *lpdwFree);
			}

	return res; 
}

HRESULT WINAPI extGetAvailableVidMem2(LPDIRECTDRAW lpdd, LPDDSCAPS lpDDSCaps, LPDWORD lpdwTotal, LPDWORD lpdwFree)
{
	return extGetAvailableVidMem(lpdd, lpDDSCaps, lpdwTotal, lpdwFree, pGetAvailableVidMem2);
}
 
HRESULT WINAPI extGetAvailableVidMem4(LPDIRECTDRAW lpdd, LPDDSCAPS lpDDSCaps, LPDWORD lpdwTotal, LPDWORD lpdwFree)
{
	return extGetAvailableVidMem(lpdd, lpDDSCaps, lpdwTotal, lpdwFree, pGetAvailableVidMem4);
}

HRESULT WINAPI extSetSurfaceDesc(LPDIRECTDRAWSURFACE lpdds, LPDDSURFACEDESC lpDDsd2, DWORD dwFlags)
{
	HRESULT res;
	OutTrace("SetSurfaceDesc: REACHED\n");
	res = (*pSetSurfaceDesc)(lpdds, lpDDsd2, dwFlags);
	OutTraceE("SetSurfaceDesc: ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}
