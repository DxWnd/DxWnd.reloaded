#define _WIN32_WINNT 0x0600
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_DEPRECATE 1

// comment defines to get rid of some proxy logs....
//#define TRACECOMMETHODS YES
#define HOOKDDRAW   YES
#define HOOKCLIPPER YES
#define HOOKSURFACE YES
#define HOOKPALETTE YES

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "dxhook.h"
#include "syslibs.h"
#include "dxhelper.h"
#include "hddraw.h"
#include "ddproxy.h"

static void HookDDSessionProxy(LPDIRECTDRAW *, int);
static void HookDDSurfaceProxy(LPDIRECTDRAWSURFACE *, int);
static void HookDDClipperProxy(LPDIRECTDRAWCLIPPER FAR*);
static void HookDDPaletteProxy(LPDIRECTDRAWPALETTE FAR*);

extern FARPROC WINAPI extGetProcAddress(HMODULE, LPCSTR);

// DirectDraw APIs
extern DirectDrawCreate_Type pDirectDrawCreate;
extern DirectDrawCreateEx_Type pDirectDrawCreateEx;
extern DirectDrawEnumerate_Type pDirectDrawEnumerate;
extern DirectDrawEnumerateEx_Type pDirectDrawEnumerateEx;

// DIrectDraw object
// COM object
extern QueryInterface_Type pQueryInterfaceD;
extern AddRefD_Type pAddRefD;
extern ReleaseD_Type pReleaseD;
// v1 interface
extern Compact_Type pCompact;
extern CreateClipper_Type pCreateClipper;
extern CreatePalette_Type pCreatePalette;
extern CreateSurface1_Type pCreateSurface1;
extern CreateSurface1_Type pCreateSurface2;
extern CreateSurface1_Type pCreateSurface3;
extern CreateSurface2_Type pCreateSurface4;
extern CreateSurface2_Type pCreateSurface7;
extern DuplicateSurface_Type pDuplicateSurface;
extern EnumDisplayModes1_Type pEnumDisplayModes1;
extern EnumDisplayModes4_Type pEnumDisplayModes4;
extern EnumSurfaces1_Type pEnumSurfaces1;
extern EnumSurfaces4_Type pEnumSurfaces4;
extern FlipToGDISurface_Type pFlipToGDISurface;
extern GetCapsD_Type pGetCapsD;
extern GetDisplayMode_Type pGetDisplayMode;
extern GetFourCCCodes_Type pGetFourCCCodes;
extern GetGDISurface_Type pGetGDISurface;
extern GetMonitorFrequency_Type pGetMonitorFrequency;
extern GetScanLine_Type pGetScanLine;
extern GetVerticalBlankStatus_Type pGetVerticalBlankStatus;
extern Initialize_Type pInitialize;
extern RestoreDisplayMode_Type pRestoreDisplayMode;
extern SetCooperativeLevel_Type pSetCooperativeLevel;
extern SetDisplayMode1_Type pSetDisplayMode1;
extern SetDisplayMode2_Type pSetDisplayMode2;
// missing WaitForVerticalBlank ...
// v2 interface
extern GetAvailableVidMem_Type pGetAvailableVidMem;
// v4 interface
extern GetSurfaceFromDC_Type pGetSurfaceFromDC;
extern RestoreAllSurfaces_Type pRestoreAllSurfaces;
// v7 interface
extern TestCooperativeLevel_Type pTestCooperativeLevel;
extern GetDeviceIdentifier_Type pGetDeviceIdentifier;

// DIrectDrawSurface object
// COM object
extern QueryInterface_Type pQueryInterfaceS;
extern AddRefS_Type pAddRefS;
extern ReleaseS_Type pReleaseS;
// v1 interface
extern AddAttachedSurface_Type pAddAttachedSurface;
extern AddOverlayDirtyRect_Type pAddOverlayDirtyRect;
extern Blt_Type pBlt;
extern BltBatch_Type pBltBatch;
extern BltFast_Type pBltFast;
extern DeleteAttachedSurface_Type pDeleteAttachedSurface;
extern EnumAttachedSurfaces_Type pEnumAttachedSurfaces;
extern EnumOverlayZOrders_Type pEnumOverlayZOrders;
extern Flip_Type pFlip;
extern GetAttachedSurface_Type pGetAttachedSurface1;
extern GetAttachedSurface_Type pGetAttachedSurface3;
extern GetAttachedSurface_Type pGetAttachedSurface4;
extern GetAttachedSurface_Type pGetAttachedSurface7;
extern GetBltStatus_Type pGetBltStatus;
extern GetCapsS_Type pGetCaps1S;
extern GetCapsS_Type pGetCaps2S;
extern GetCapsS_Type pGetCaps3S;
extern GetCaps2S_Type pGetCaps4S;
extern GetCaps2S_Type pGetCaps7S;
extern GetClipper_Type pGetClipper;
extern GetColorKey_Type pGetColorKey;
extern GetDC_Type pGetDC;
extern GetFlipStatus_Type pGetFlipStatus;
extern GetOverlayPosition_Type pGetOverlayPosition;
extern GetPalette_Type pGetPalette;
extern GetPixelFormat_Type pGetPixelFormat; 
extern GetSurfaceDesc_Type pGetSurfaceDesc1;
extern GetSurfaceDesc2_Type pGetSurfaceDesc4;
// missing Initialize ...
extern IsLost_Type pIsLost;
extern Lock_Type pLock;
extern ReleaseDC_Type pReleaseDC;
extern Restore_Type pRestore;
extern SetClipper_Type pSetClipper;
extern SetColorKey_Type pSetColorKey;
extern SetOverlayPosition_Type pSetOverlayPosition;
extern SetPalette_Type pSetPalette;
extern Unlock1_Type pUnlock1;
extern Unlock4_Type pUnlock4;
extern UpdateOverlay_Type pUpdateOverlay;
extern UpdateOverlayDisplay_Type pUpdateOverlayDisplay;
extern UpdateOverlayZOrder_Type pUpdateOverlayZOrder;
// v2 interface
// missing GetDDInterface
// missing PageLock
// missing PageUnlock
// v3 interface
// missing SetSurfaceDesc
// v4 interface
// missing SetPrivateData
// missing GetPrivateData
// missing FreePrivateData
// missing GetUniquenessValue
// missing ChangeUniquenessValue
// Texture7 methods 
// missing SetPriority
// missing GetPriority
// missing SetLOD
// missing GetLOD

// DIrectDrawClipper object
// COM object
extern QueryInterface_Type pQueryInterfaceC;
extern AddRefC_Type pAddRefC;
extern ReleaseC_Type pReleaseC;
// v1 interface
extern GetClipList_Type pGetClipList;
extern GetHWnd_Type pGetHWnd;
extern InitializeC_Type pInitializeC;
extern IsClipListChanged_Type pIsClipListChanged;
extern SetClipList_Type pSetClipList;
extern SetHWnd_Type pSetHWnd;

// DIrectDrawPalette object
// COM object
extern QueryInterface_Type pQueryInterfaceP;
extern AddRefP_Type pAddRefP;
extern ReleaseP_Type pReleaseP;
// v1 interface
extern GetCapsP_Type pGetCapsP;
extern GetEntries_Type pGetEntries;
// missing Initialize ...
extern SetEntries_Type pSetEntries;

extern GDIGetDC_Type pGDIGetDC;
extern GDIGetDC_Type pGDIGetWindowDC;
extern GDIReleaseDC_Type pGDIReleaseDC;
extern CreateDC_Type pGDICreateDC;
extern CreateCompatibleDC_Type pGDICreateCompatibleDC;
extern BitBlt_Type pGDIBitBlt;
extern PatBlt_Type pGDIPatBlt;
extern StretchBlt_Type pGDIStretchBlt;
extern DeleteDC_Type pGDIDeleteDC;
SaveDC_Type pSaveDC;
RestoreDC_Type pRestoreDC;
extern BeginPaint_Type pBeginPaint;

HDC WINAPI extGDIGetDCProxy(HWND);
HDC WINAPI extGetWindowDCProxy(HWND);
int WINAPI extGDIReleaseDCProxy(HWND, HDC);
HDC WINAPI extCreateDCProxy(LPSTR, LPSTR, LPSTR, CONST DEVMODE *);
HDC WINAPI extCreateCompatibleDCProxy(HDC);
BOOL WINAPI extBitBltProxy(HDC, int, int, int, int, HDC, int, int, DWORD);
BOOL WINAPI extPatBltProxy(HDC, int, int, int, int, DWORD);
BOOL WINAPI extStretchBltProxy(HDC, int, int, int, int, HDC, int, int, int, int, DWORD);
BOOL WINAPI extDeleteDCProxy(HDC);
int WINAPI extSaveDCProxy(HDC);
BOOL WINAPI extRestoreDCProxy(HDC, int);
HPALETTE WINAPI extSelectPaletteProxy(HDC, HPALETTE, BOOL);
UINT WINAPI extRealizePaletteProxy(HDC);
HDC WINAPI extBeginPaintProxy(HWND, LPPAINTSTRUCT);
UINT WINAPI extGetSystemPaletteEntriesProxy(HDC, UINT, UINT, LPPALETTEENTRY);
int WINAPI extGetDeviceCapsProxy(HDC, int);

// ------------------------------------------------------------------------------------------ //
//
// auxiliary calls
//
// ------------------------------------------------------------------------------------------ //

static void DumpSurfaceAttributesProxy(LPDDSURFACEDESC lpddsd, char *label)
{
	OutTraceP("%s: Flags=%x(%s)",
		label, 
		lpddsd->dwFlags, ExplainFlags(lpddsd->dwFlags));
	if (lpddsd->dwFlags & DDSD_BACKBUFFERCOUNT) OutTraceP(" BackBufferCount=%d", lpddsd->dwBackBufferCount);
	if (lpddsd->dwFlags & DDSD_WIDTH) OutTraceP(" Width=%d", lpddsd->dwWidth);
	if (lpddsd->dwFlags & DDSD_HEIGHT) OutTraceP(" Height=%d", lpddsd->dwHeight);
	if (lpddsd->dwFlags & DDSD_CAPS) OutTraceP(" Caps=%x(%s)", lpddsd->ddsCaps.dwCaps, ExplainDDSCaps(lpddsd->ddsCaps.dwCaps));
	if (lpddsd->dwFlags & DDSD_CKDESTBLT ) OutTraceP(" CKDestBlt=(%x,%x)", lpddsd->ddckCKDestBlt.dwColorSpaceLowValue, lpddsd->ddckCKDestBlt.dwColorSpaceHighValue);
	if (lpddsd->dwFlags & DDSD_CKDESTOVERLAY ) OutTraceP(" CKDestOverlay=(%x,%x)", lpddsd->ddckCKDestOverlay.dwColorSpaceLowValue, lpddsd->ddckCKDestOverlay.dwColorSpaceHighValue);
	if (lpddsd->dwFlags & DDSD_CKSRCBLT ) OutTraceP(" CKSrcBlt=(%x,%x)", lpddsd->ddckCKSrcBlt.dwColorSpaceLowValue, lpddsd->ddckCKSrcBlt.dwColorSpaceHighValue);
	if (lpddsd->dwFlags & DDSD_CKSRCOVERLAY ) OutTraceP(" CKSrcOverlay=(%x,%x)", lpddsd->ddckCKSrcOverlay.dwColorSpaceLowValue, lpddsd->ddckCKSrcOverlay.dwColorSpaceHighValue);
	if (lpddsd->dwFlags & DDSD_PIXELFORMAT ) OutTraceP(" PixelFormat BPP=%d RGBA=(%x,%x,%x,%x)", 
		lpddsd->ddpfPixelFormat.dwRGBBitCount, 
		lpddsd->ddpfPixelFormat.dwRBitMask,
		lpddsd->ddpfPixelFormat.dwGBitMask,
		lpddsd->ddpfPixelFormat.dwBBitMask,
		lpddsd->ddpfPixelFormat.dwRGBAlphaBitMask);
	OutTraceP("\n");
}

// ------------------------------------------------------------------------------------------ //
//
// DirectDraw API hooking functions
//
// ------------------------------------------------------------------------------------------ //

FARPROC WINAPI extGetProcAddressProxy(HMODULE hModule, LPCSTR proc)
{
	FARPROC ret;
	// WARNING: seems to be called with bad LPCSTR value....
	__try {
		OutTraceP("GetProcAddress: PROXED hModule=%x proc=%s\n", hModule, proc);
		// should check for both hModule from ddraw.dll & apiname, but....
		if(proc){
			if (!strcmp(proc,"DirectDrawCreate")){
				pDirectDrawCreate=(DirectDrawCreate_Type)(*pGetProcAddress)(hModule, proc);
				OutTraceP("GetProcAddress: hooking proc=%s at addr=%x\n", proc, pDirectDrawCreate);
				return (FARPROC)extDirectDrawCreateProxy;
			}
			if (!strcmp(proc,"DirectDrawCreateEx")){
				pDirectDrawCreateEx=(DirectDrawCreateEx_Type)(*pGetProcAddress)(hModule, proc);
				OutTraceP("GetProcAddress: hooking proc=%s at addr=%x\n", proc, pDirectDrawCreateEx);
				return (FARPROC)extDirectDrawCreateExProxy;
			}
			if (!strcmp(proc,"DirectDrawEnumerateA")){
				pDirectDrawEnumerate=(DirectDrawEnumerate_Type)(*pGetProcAddress)(hModule, proc);
				OutTraceP("GetProcAddress: hooking proc=%s at addr=%x\n", proc, pDirectDrawEnumerate);
				return (FARPROC)extDirectDrawEnumerateProxy;
			}
			if (!strcmp(proc,"DirectDrawEnumerateExA")){
				pDirectDrawEnumerateEx=(DirectDrawEnumerateEx_Type)(*pGetProcAddress)(hModule, proc);
				OutTraceP("GetProcAddress: hooking proc=%s at addr=%x\n", proc, pDirectDrawEnumerateEx);
				return (FARPROC)extDirectDrawEnumerateExProxy;
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{       
		OutTraceP("GetProcAddress: EXCEPTION\n");
	}
	ret=(*pGetProcAddress)(hModule, proc);
	OutTraceP("GetProcAddress: ret=%x\n", ret);
	return ret;
}

// ------------------------------------------------------------------------------------------ //
//
// COM methods hooking functions
//
// ------------------------------------------------------------------------------------------ //

int HookDDProxy(HMODULE module, int dxVersion)
{
	HINSTANCE hinst;
	void *tmp;
	const GUID dd7 = {0x15e65ec0,0x3b9c,0x11d2,0xb9,0x2f,0x00,0x60,0x97,0x97,0xea,0x5b};
//	LPDIRECTDRAW lpdd;
//	BOOL res;

	dxw.dwFlags1 |= OUTDDRAWTRACE;
	
	pGDICreateCompatibleDC=CreateCompatibleDC;
	pGDIGetDC=GetDC;
	pGDIGetWindowDC=GetWindowDC;
	pGDIReleaseDC=ReleaseDC;
	pGDICreateDC=CreateDC;
	pGDIBitBlt=BitBlt;
	pGDIPatBlt=PatBlt;
	pGDIStretchBlt=StretchBlt;
	pGDIDeleteDC=DeleteDC;
	pSaveDC=SaveDC;
	pRestoreDC=RestoreDC;
	pGDISelectPalette=SelectPalette;
	tmp = HookAPI(module, "GDI32.dll", CreateCompatibleDC, "CreateCompatibleDC", extCreateCompatibleDCProxy);
	if(tmp) pGDICreateCompatibleDC = (CreateCompatibleDC_Type)tmp;
	tmp = HookAPI(module, "user32.dll", GetDC, "GetDC", extGDIGetDCProxy);
	if(tmp) pGDIGetDC = (GDIGetDC_Type)tmp;
	tmp = HookAPI(module, "user32.dll", GetWindowDC, "GetWindowDC", extGetWindowDCProxy);
	if(tmp) pGDIGetWindowDC = (GDIGetDC_Type)tmp;
	tmp = HookAPI(module, "user32.dll", ReleaseDC, "ReleaseDC", extGDIReleaseDCProxy);
	if(tmp) pGDIReleaseDC = (GDIReleaseDC_Type)tmp;
	tmp = HookAPI(module, "GDI32.dll", CreateDCA, "CreateDCA", extCreateDCProxy);
	if(tmp) pGDICreateDC = (CreateDC_Type)tmp;
	tmp = HookAPI(module, "GDI32.dll", BitBlt, "BitBlt", extBitBltProxy);
	if(tmp) pGDIBitBlt = (BitBlt_Type)tmp;
	tmp = HookAPI(module, "GDI32.dll", StretchBlt, "StretchBlt", extStretchBltProxy);
	if(tmp) pGDIStretchBlt = (StretchBlt_Type)tmp;
	tmp = HookAPI(module, "GDI32.dll", PatBlt, "PatBlt", extPatBltProxy);
	if(tmp) pGDIPatBlt = (PatBlt_Type)tmp;
	tmp = HookAPI(module, "GDI32.dll", DeleteDC, "DeleteDC", extDeleteDCProxy);
	if(tmp) pGDIDeleteDC = (DeleteDC_Type)tmp;
	tmp = HookAPI(module, "GDI32.dll", SaveDC, "SaveDC", extSaveDCProxy);
	if(tmp) pGDISaveDC = (SaveDC_Type)tmp;
	tmp = HookAPI(module, "GDI32.dll", RestoreDC, "RestoreDC", extRestoreDCProxy);
	if(tmp) pGDIRestoreDC = (RestoreDC_Type)tmp;
	tmp = HookAPI(module, "GDI32.dll", SelectPalette, "SelectPalette", extSelectPaletteProxy);
	if(tmp) pGDISelectPalette = (SelectPalette_Type)tmp;
	tmp = HookAPI(module, "GDI32.dll", RealizePalette, "RealizePalette", extRealizePaletteProxy);
	if(tmp) pGDIRealizePalette = (RealizePalette_Type)tmp;
	tmp = HookAPI(module, "GDI32.dll", GetSystemPaletteEntries, "GetSystemPaletteEntries", extGetSystemPaletteEntriesProxy);
	if(tmp) pGDIGetSystemPaletteEntries = (GetSystemPaletteEntries_Type)tmp;
	tmp = HookAPI(module, "GDI32.dll", GetDeviceCaps, "GetDeviceCaps", extGetDeviceCapsProxy);
	if(tmp) pGDIGetDeviceCaps = (GetDeviceCaps_Type)tmp;
	tmp = HookAPI(module, "user32.dll", BeginPaint, "BeginPaint", extBeginPaintProxy);
	if(tmp) pBeginPaint = (BeginPaint_Type)tmp;
	pGetProcAddress = (GetProcAddress_Type)GetProcAddress;
	tmp = HookAPI(module, "KERNEL32.dll", GetProcAddress, "GetProcAddress", extGetProcAddressProxy);
	if(tmp) pGetProcAddress = (GetProcAddress_Type)tmp;
	switch(dxVersion){
	case 0: // automatic
		tmp = HookAPI(module, "ddraw.dll", NULL, "DirectDrawCreate", extDirectDrawCreateProxy);
		if(tmp) pDirectDrawCreate = (DirectDrawCreate_Type)tmp;
		tmp = HookAPI(module, "ddraw.dll", NULL, "DirectDrawCreateEx", extDirectDrawCreateExProxy);
		if(tmp) pDirectDrawCreateEx = (DirectDrawCreateEx_Type)tmp;
		tmp = HookAPI(module, "ddraw.dll", NULL, "DirectDrawEnumerateA", extDirectDrawEnumerateProxy);
		if(tmp) pDirectDrawEnumerate = (DirectDrawEnumerate_Type)tmp;
		tmp = HookAPI(module, "ddraw.dll", NULL, "DirectDrawEnumerateExA", extDirectDrawEnumerateExProxy);
		if(tmp) pDirectDrawEnumerateEx = (DirectDrawEnumerateEx_Type)tmp;
		break;
	case 1:
	case 2:
	case 3:
	case 5:
	case 6:
		hinst = LoadLibrary("ddraw.dll");
		pDirectDrawEnumerate = 
			(DirectDrawEnumerate_Type)GetProcAddress(hinst, "DirectDrawEnumerateA");
		pDirectDrawCreate =
			(DirectDrawCreate_Type)GetProcAddress(hinst, "DirectDrawCreate");
		if(pDirectDrawCreate){
			LPDIRECTDRAW lpdd;
			BOOL res;
			HookAPI(module, "ddraw.dll", pDirectDrawCreate, "DirectDrawCreate", extDirectDrawCreateProxy);
			HookAPI(module, "ddraw.dll", pDirectDrawEnumerate, "DirectDrawEnumerateA", extDirectDrawEnumerateProxy);
			res=extDirectDrawCreateProxy(0, &lpdd, 0);
			if (res){
				OutTraceP("DirectDrawCreate: ERROR res=%x(%s)\n", res, ExplainDDError(res));
			}
			lpdd->Release();
		}
		break;
	case 7:
		hinst = LoadLibrary("ddraw.dll");
		pDirectDrawEnumerate = 
			(DirectDrawEnumerate_Type)GetProcAddress(hinst, "DirectDrawEnumerateA");
		pDirectDrawEnumerateEx = 
			(DirectDrawEnumerateEx_Type)GetProcAddress(hinst, "DirectDrawEnumerateExA");
		pDirectDrawCreate =
			(DirectDrawCreate_Type)GetProcAddress(hinst, "DirectDrawCreate");
		if(pDirectDrawCreate){
			LPDIRECTDRAW lpdd;
			BOOL res;
			HookAPI(module, "ddraw.dll", pDirectDrawCreate, "DirectDrawCreate", extDirectDrawCreateProxy);
			HookAPI(module, "ddraw.dll", pDirectDrawEnumerate, "DirectDrawEnumerateA", extDirectDrawEnumerateProxy);
			HookAPI(module, "ddraw.dll", pDirectDrawEnumerateEx, "DirectDrawEnumerateExA", extDirectDrawEnumerateExProxy);
			res=extDirectDrawCreateProxy(0, &lpdd, 0);
			if (res){
				OutTraceP("DirectDrawCreate: ERROR res=%x(%s)\n", res, ExplainDDError(res));
			}
			lpdd->Release();
		}
		pDirectDrawCreateEx =
			(DirectDrawCreateEx_Type)GetProcAddress(hinst, "DirectDrawCreateEx");
		if(pDirectDrawCreateEx){
			LPDIRECTDRAW lpdd;
			BOOL res;
			HookAPI(module, "ddraw.dll", pDirectDrawCreateEx, "DirectDrawCreateEx", extDirectDrawCreateExProxy);
			res=extDirectDrawCreateExProxy(0, &lpdd, dd7, 0);
			if (res) OutTraceP("DirectDrawCreateEx: ERROR res=%x(%s)\n", res, ExplainDDError(res));
			lpdd->Release();
		}
		break;
	}

	if(pDirectDrawCreate || pDirectDrawCreateEx) return 1;
	return 0;
}

static void HookDDClipperProxy(LPDIRECTDRAWCLIPPER FAR* lplpDDClipper)
{
	// IDirectDrawClipper::QueryInterface
	SetHook((void *)(**(DWORD **)lplpDDClipper + 0), extQueryInterfaceCProxy, (void **)&pQueryInterfaceC, "QueryInterface(C)");
#ifdef TRACECOMMETHODS
	// IDirectDrawClipper::AddRef
	SetHook((void *)(**(DWORD **)lplpDDClipper + 4), extAddRefCProxy, (void **)&pAddRefC, "AddRef(C)");
	// IDirectDrawClipper::Release
	SetHook((void *)(**(DWORD **)lplpDDClipper + 8), extReleaseCProxy, (void **)&pReleaseC, "Release(C)");
#endif
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

static void HookDDPaletteProxy(LPDIRECTDRAWPALETTE FAR* lplpDDPalette)
{
	// IDirectDrawPalette::QueryInterface
	SetHook((void *)(**(DWORD **)lplpDDPalette + 0), extQueryInterfacePProxy, (void **)&pQueryInterfaceP, "QueryInterface(P)");
#ifdef TRACECOMMETHODS
	// IDirectDrawPalette::AddRef
	SetHook((void *)(**(DWORD **)lplpDDPalette + 4), extAddRefPProxy, (void **)&pAddRefP, "AddRef(P)");
	// IDirectDrawPalette::Release
	SetHook((void *)(**(DWORD **)lplpDDPalette + 8), extReleasePProxy, (void **)&pReleaseP, "Release(P)");
#endif
	// IDirectDrawPalette::GetCaps
	SetHook((void *)(**(DWORD **)lplpDDPalette + 12), extGetCapsPProxy, (void **)&pGetCapsP, "GetCaps(P)");
	// IDirectDrawPalette::GetEntries
	SetHook((void *)(**(DWORD **)lplpDDPalette + 16), extGetEntriesProxy, (void **)&pGetEntries, "GetEntries(P)");
	// IDirectDrawPalette::SetEntries
	SetHook((void *)(**(DWORD **)lplpDDPalette + 24), extSetEntriesProxy, (void **)&pSetEntries, "SetEntries(P)");
	return;
}

static void HookDDSurfaceProxy(LPDIRECTDRAWSURFACE *lplpdds, int dxVersion)
{
	OutTraceP("Hooking surface dds=%x version=%d thread_id=%x\n", *lplpdds, dxVersion, GetCurrentThreadId());

	// IDirectDrawSurface::Query
	SetHook((void *)(**(DWORD **)lplpdds + 0), extQueryInterfaceSProxy, (void **)&pQueryInterfaceS, "QueryInterface(S)");
#ifdef TRACECOMMETHODS
	// IDirectDrawSurface::AddRef
	SetHook((void *)(**(DWORD **)lplpdds + 4), extAddRefSProxy, (void **)&pAddRefS, "AddRef(S)");
	// IDirectDrawSurface::Release
	SetHook((void *)(**(DWORD **)lplpdds + 8), extReleaseSProxy, (void **)&pReleaseS, "Release(S)");
#endif
	// IDirectDrawSurface::AddAttachedSurface
	SetHook((void *)(**(DWORD **)lplpdds + 12), extAddAttachedSurfaceProxy, (void **)&pAddAttachedSurface, "AddAttachedSurface(S)");
	// IDirectDrawSurface::AddOverlayDirtyRect
	SetHook((void *)(**(DWORD **)lplpdds + 16), extAddOverlayDirtyRectProxy, (void **)&pAddOverlayDirtyRect, "AddOverlayDirtyRect(S)");
	// IDirectDrawSurface::Blt
	SetHook((void *)(**(DWORD **)lplpdds + 20), extBltProxy, (void **)&pBlt, "Blt(S)");
	// IDirectDrawSurface::BltBatch
	SetHook((void *)(**(DWORD **)lplpdds + 24), extBltBatchProxy, (void **)&pBltBatch, "BltBatch(S)");
	// IDirectDrawSurface::BltFast
	SetHook((void *)(**(DWORD **)lplpdds + 28), extBltFastProxy, (void **)&pBltFast, "BltFast(S)");
	// IDirectDrawSurface::DeleteAttachedSurface
	SetHook((void *)(**(DWORD **)lplpdds + 32), extDeleteAttachedSurfaceProxy, (void **)&pDeleteAttachedSurface, "DeleteAttachedSurface(S)");
	// IDirectDrawSurface::EnumAttachedSurfaces
	SetHook((void *)(**(DWORD **)lplpdds + 36), extEnumAttachedSurfacesProxy, (void **)&pEnumAttachedSurfaces, "EnumAttachedSurfaces(S)");
	// IDirectDrawSurface::EnumOverlayZOrders
	SetHook((void *)(**(DWORD **)lplpdds + 40), extEnumOverlayZOrdersProxy, (void **)&pEnumOverlayZOrders, "EnumOverlayZOrders(S)");
	// IDirectDrawSurface::Flip
	SetHook((void *)(**(DWORD **)lplpdds + 44), extFlipProxy, (void **)&pFlip, "Flip(S)");
	// IDirectDrawSurface::GetAttachedSurface
	switch(dxVersion) {
	case 1:
	case 2:
		SetHook((void *)(**(DWORD **)lplpdds + 48), extGetAttachedSurface1Proxy, (void **)&pGetAttachedSurface1, "GetAttachedSurface(S1)");
		break;
	case 3:
		SetHook((void *)(**(DWORD **)lplpdds + 48), extGetAttachedSurface3Proxy, (void **)&pGetAttachedSurface3, "GetAttachedSurface(S3)");
		break;
	case 4:
		SetHook((void *)(**(DWORD **)lplpdds + 48), extGetAttachedSurface4Proxy, (void **)&pGetAttachedSurface4, "GetAttachedSurface(S4)");
		break;
	case 7:
		SetHook((void *)(**(DWORD **)lplpdds + 48), extGetAttachedSurface7Proxy, (void **)&pGetAttachedSurface7, "GetAttachedSurface(S7)");
		break;
	}
	// IDirectDrawSurface::GetBltStatus
	SetHook((void *)(**(DWORD **)lplpdds + 52), extGetBltStatusProxy, (void **)&pGetBltStatus, "GetBltStatus(S)");
	// IDirectDrawSurface::GetCaps
	switch(dxVersion) {
	case 1:
		SetHook((void *)(**(DWORD **)lplpdds + 56), extGetCaps1SProxy, (void **)&pGetCaps1S, "GetCaps(S1)");
		break;
	case 2:
		SetHook((void *)(**(DWORD **)lplpdds + 56), extGetCaps2SProxy, (void **)&pGetCaps2S, "GetCaps(S2)");
		break;
	case 3:
		SetHook((void *)(**(DWORD **)lplpdds + 56), extGetCaps3SProxy, (void **)&pGetCaps3S, "GetCaps(S3)");
		break;
	case 4:
		SetHook((void *)(**(DWORD **)lplpdds + 56), extGetCaps4SProxy, (void **)&pGetCaps4S, "GetCaps(S4)");
		break;
	case 7:
		SetHook((void *)(**(DWORD **)lplpdds + 56), extGetCaps7SProxy, (void **)&pGetCaps7S, "GetCaps(S7)");
		break;
	}
	// IDirectDrawSurface::GetClipper
	SetHook((void *)(**(DWORD **)lplpdds + 60), extGetClipperProxy, (void **)&pGetClipper, "GetClipper(S)");
	// IDirectDrawSurface::GetColorKey
	SetHook((void *)(**(DWORD **)lplpdds + 64), extGetColorKeyProxy, (void **)&pGetColorKey, "GetColorKey(S)");
	// IDirectDrawSurface::GetDC
	SetHook((void *)(**(DWORD **)lplpdds + 68), extGetDCProxy, (void **)&pGetDC, "GetDC(S)");
	// IDirectDrawSurface::GetFlipStatus
	SetHook((void *)(**(DWORD **)lplpdds + 72), extGetFlipStatusProxy, (void **)&pGetFlipStatus, "GetFlipStatus(S)");
	// IDirectDrawSurface::GetOverlayPosition
	SetHook((void *)(**(DWORD **)lplpdds + 76), extGetOverlayPositionProxy, (void **)&pGetOverlayPosition, "GetOverlayPosition(S)");
	// IDirectDrawSurface::GetPalette
	SetHook((void *)(**(DWORD **)lplpdds + 80), extGetPaletteProxy, (void **)&pGetPalette, "GetPalette(S)");
	// IDirectDrawSurface::GetPixelFormat
	SetHook((void *)(**(DWORD **)lplpdds + 84), extGetPixelFormatProxy, (void **)&pGetPixelFormat, "GetPixelFormat(S)");
	// IDirectDrawSurface::GetSurfaceDesc
	switch(dxVersion){
	case 1:
	case 2:
	case 3:
		SetHook((void *)(**(DWORD **)lplpdds + 88), extGetSurfaceDesc1Proxy, (void **)&pGetSurfaceDesc1, "pGetSurfaceDesc(S1)");
		break;
	case 4:
	case 7:
		SetHook((void *)(**(DWORD **)lplpdds + 88), extGetSurfaceDesc4Proxy, (void **)&pGetSurfaceDesc4, "pGetSurfaceDesc(S4)");
		break;
	}
	// IDirectDrawSurface::IsLost
	SetHook((void *)(**(DWORD **)lplpdds + 96), extIsLostProxy, (void **)&pIsLost, "IsLost(S)");
	// IDirectDrawSurface::Lock
	SetHook((void *)(**(DWORD **)lplpdds + 100), extLockProxy, (void **)&pLock, "Lock(S)");
	// IDirectDrawSurface::ReleaseDC
	SetHook((void *)(**(DWORD **)lplpdds + 104), extReleaseDCProxy, (void **)&pReleaseDC, "ReleaseDC(S)");
	// IDirectDrawSurface::Restore
	SetHook((void *)(**(DWORD **)lplpdds + 108), extRestoreProxy, (void **)&pRestore, "Restore(S)");
	// IDirectDrawSurface::SetClipper
	SetHook((void *)(**(DWORD **)lplpdds + 112), extSetClipperProxy, (void **)&pSetClipper, "SetClipper(S)");
	// IDirectDrawSurface::SetColorKey
	SetHook((void *)(**(DWORD **)lplpdds + 116), extSetColorKeyProxy, (void **)&pSetColorKey, "SetColorKey(S)");
	// IDirectDrawSurface::SetOverlayPosition
	SetHook((void *)(**(DWORD **)lplpdds + 120), extSetOverlayPositionProxy, (void **)&pSetOverlayPosition, "SetOverlayPosition(S)");
	// IDirectDrawSurface::SetPalette
	SetHook((void *)(**(DWORD **)lplpdds + 124), extSetPaletteProxy, (void **)&pSetPalette, "SetPalette(S)");
	// IDirectDrawSurface::Unlock
	if (dxVersion >= 4)
		SetHook((void *)(**(DWORD **)lplpdds + 128), extUnlock4Proxy, (void **)&pUnlock4, "Unlock(S4)");
	else
		SetHook((void *)(**(DWORD **)lplpdds + 128), extUnlock1Proxy, (void **)&pUnlock1, "Unlock(S1)");
	// IDirectDrawSurface::UpdateOverlay
	SetHook((void *)(**(DWORD **)lplpdds + 132), extUpdateOverlayProxy, (void **)&pUpdateOverlay, "UpdateOverlay(S)");
	// IDirectDrawSurface::UpdateOverlayDisplay
	SetHook((void *)(**(DWORD **)lplpdds + 136), extUpdateOverlayDisplayProxy, (void **)&pUpdateOverlayDisplay, "UpdateOverlayDisplay(S)");
	// IDirectDrawSurface::UpdateOverlayZOrder
	SetHook((void *)(**(DWORD **)lplpdds + 140), extUpdateOverlayZOrderProxy, (void **)&pUpdateOverlayZOrder, "UpdateOverlayZOrder(S)");


#if 0
    /*** Added in the v2 interface ***/
	if (dxVersion >= 2) {
		// missing GetDDInterface)(THIS_ LPVOID FAR *) PURE;
		// missing PageLock)(THIS_ DWORD) PURE;
		// missing PageUnlock)(THIS_ DWORD) PURE;
	}
    /*** Added in the v3 interface ***/
	if (dxVersion >= 3) {
		// missing SetSurfaceDesc)(THIS_ LPDDSURFACEDESC2, DWORD) PURE;
	}
    /*** Added in the v4 interface ***/
	if (dxVersion >= 4) {
		// missing SetPrivateData)(THIS_ REFGUID, LPVOID, DWORD, DWORD) PURE;
		// missing GetPrivateData)(THIS_ REFGUID, LPVOID, LPDWORD) PURE;
		// missing FreePrivateData)(THIS_ REFGUID) PURE;
		// missing GetUniquenessValue)(THIS_ LPDWORD) PURE;
		// missing ChangeUniquenessValue)(THIS) PURE;
	}
    /*** Moved Texture7 methods here ***/
	if (dxVersion >= 7) {
		// missing SetPriority)(THIS_ DWORD) PURE;
		// missing GetPriority)(THIS_ LPDWORD) PURE;
		// missing SetLOD)(THIS_ DWORD) PURE;
		// missing GetLOD)(THIS_ LPDWORD) PURE;
	}
#endif
}

// ------------------------------------------------------------------------------------------ //
//
// DirectDraw proxed methods
//
// ------------------------------------------------------------------------------------------ //

HRESULT WINAPI extQueryInterfaceDProxy(void *lpdd, REFIID riid, LPVOID *obp)
{
	HRESULT res;
	unsigned int dxVersion;
	OutTraceP("QueryInterface(D): PROXED lpdd=%x REFIID=%x(%s) obp=%x\n", lpdd, riid.Data1, ExplainGUID((GUID *)&riid.Data1), *obp);
	res = (*pQueryInterfaceD)(lpdd, riid, obp);
	if(res) {
		OutTraceP("QueryInterface(D): ret=%x(%s)\n", res, ExplainDDError(res));
		return res;
	}
	dxVersion=0;
	switch(riid.Data1){
	case 0x6C14DB80:		//DirectDraw1
		dxVersion = 1;	
		break;
	case 0xB3A6F3E0:		//DirectDraw2
		dxVersion = 2;
		break;
	case 0x9c59509a:		//DirectDraw4
		dxVersion = 4;
		break;
	case 0x15e65ec0:		//DirectDraw7
		dxVersion = 7;
		break;
	}
	if (! *obp) {
		OutTraceP("QueryInterface(D): Interface for DX version %d not found\n", dxVersion);
		return(0);
	}
	OutTraceP("QueryInterface(D): obp=%x interface=%d\n", *obp, dxVersion);
	switch (dxVersion){
	case 1: 
	case 2:
	case 4:
	case 7:
#ifdef HOOKDDRAW
		HookDDSessionProxy((LPDIRECTDRAW *)obp, dxVersion);
#endif
		break;
	}
	return res;
}


ULONG WINAPI extAddRefDProxy(LPDIRECTDRAW lpdd)
{
	ULONG ref;
	ref=(*pAddRefD)(lpdd);
	OutTraceP("AddRef(D): PROXED lpdd=%x ref=%x\n", lpdd, ref);
	return ref;
}

ULONG WINAPI extReleaseDProxy(LPDIRECTDRAW lpdd)
{
	ULONG ref;
	ref=(*pReleaseD)(lpdd);
	OutTraceP("Release(D): PROXED lpdd=%x ref=%x\n", lpdd, ref);
	return ref;
}

HRESULT WINAPI extInitializeProxy(LPDIRECTDRAW lpdd, GUID FAR *lpguid)
{
	HRESULT res;
	GUID FAR *lpPrivGuid = lpguid;

	OutTraceDW("Initialize: lpdd=%x guid=%x(%s)\n", lpdd, lpguid, ExplainGUID(lpguid));

	res=(*pInitialize)(lpdd, lpPrivGuid);

	if(res) OutTraceE("Initialize ERROR: res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extCreateClipperProxy(LPDIRECTDRAW lpdd, DWORD dwflags, LPDIRECTDRAWCLIPPER FAR* lplpDDClipper, IUnknown FAR* pUnkOuter)
{
	HRESULT res;
	OutTraceP("CreateClipper(D): PROXED lpdd=%x flags=%x\n", lpdd, dwflags);
	res=(*pCreateClipper)(lpdd, dwflags, lplpDDClipper, pUnkOuter);
	if(res) {
		OutTraceP("CreateClipper(D): ERROR res=%x(%s)\n", lpdd, res, ExplainDDError(res));
		return res;
	}
	else OutTraceP("CreateClipper(D): lpddc=%x\n", *lplpDDClipper);
#ifdef HOOKCLIPPER
	HookDDClipperProxy(lplpDDClipper);
#endif
	return res;
}

HRESULT WINAPI extCreatePaletteProxy(LPDIRECTDRAW lpdd, DWORD dwflags, LPPALETTEENTRY lpddpa,
	LPDIRECTDRAWPALETTE *lplpddp, IUnknown *pu)
{
	HRESULT res;

	OutTraceP("CreatePalette(D): PROXED lpdd=%x dwFlags=%x(%s)\n", lpdd, dwflags, ExplainCreatePaletteFlags(dwflags));
	if(IsDebug && (dwflags & DDPCAPS_8BIT)) dxw.DumpPalette(256, lpddpa);

	res = (*pCreatePalette)(lpdd, dwflags, lpddpa, lplpddp, pu);
	if (res) {
		OutTraceP("CreatePalette(D): ERROR res=%x(%s)\n", res, ExplainDDError(res));
		return res;
	}
	else OutTraceP("CreatePalette(D): lpddp=%x\n", *lplpddp);
#ifdef HOOKPALETTE
	HookDDPaletteProxy(lplpddp);
#endif
	return res;
}



//HRESULT WINAPI extGetCapsDProxy(LPDIRECTDRAW lpdd, LPDDCAPS c1, LPDDCAPS c2)
//{
//	HRESULT res;
//	OutTraceP("GetCaps(D): PROXED lpdd=%x c1=%x c2=%x\n", lpdd, c1, c2);
//	res=(*pGetCapsD)(lpdd, c1, c2);
//	if(res) 
//		OutTraceP("GetCaps(D): ERROR res=%x(%s)\n", res, ExplainDDError(res));
//	else {
//		// to do: a full Caps dump!
//		OutTraceP("GetCaps(D): ");
//		if (c1) OutTraceP("hwcaps size=%x ", c1->dwSize);
//		if (c2) OutTraceP("swcaps size=%x ", c2->dwSize);
//		OutTraceP("\n");
//	}
//	return res;
//}

HRESULT WINAPI extGetSurfaceFromDCProxy(LPDIRECTDRAW lpdd, HDC hdc, LPDIRECTDRAWSURFACE* lpDDS) 
{
	HRESULT res;
	OutTraceP("GetSurfaceFromDC(D): PROXED lpdd=%x hdc=%x\n", lpdd, hdc);
	res=(*pGetSurfaceFromDC)(lpdd, hdc, lpDDS);
	if(res) 
		OutTraceP("GetSurfaceFromDC(D): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	else
		OutTraceP("GetSurfaceFromDC(D): lpDDS=%x(%s)\n", res, *lpDDS);
	return res;
}

HRESULT WINAPI extGetMonitorFrequencyProxy(LPDIRECTDRAW lpdd, LPDWORD lpdwFrequency) 
{
	HRESULT res;
	OutTraceP("GetMonitorFrequency(D): PROXED lpdd=%x freq=%d\n", lpdd, *lpdwFrequency);
	res=(*pGetMonitorFrequency)(lpdd, lpdwFrequency);
	if(res) OutTraceP("GetMonitorFrequency(D): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extGetScanLineProxy(LPDIRECTDRAW lpdd, LPDWORD lpdwScanLine) 
{
	HRESULT res;
	res=(*pGetScanLine)(lpdd, lpdwScanLine);
	OutTraceP("GetScanLine(D): PROXED lpdd=%x line=%x\n", lpdd, *lpdwScanLine);
	return res;
}

HRESULT WINAPI extGetVerticalBlankStatusProxy(LPDIRECTDRAW lpdd, LPBOOL lpbIsInVB) 
{
	HRESULT res;
	OutTraceP("GetVerticalBlankStatus(D): PROXED lpdd=%x\n", lpdd);
	res=(*pGetVerticalBlankStatus)(lpdd, lpbIsInVB);
	if (res) OutTraceP("GetVerticalBlankStatus(D): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("GetVerticalBlankStatus(D): status=%x\n", *lpbIsInVB);
	return res;
}

HRESULT WINAPI extRestoreDisplayModeProxy(LPDIRECTDRAW lpdd) 
{
	HRESULT res;
	OutTraceP("RestoreDisplayMode(D): PROXED lpdd=%x\n", lpdd);
	res=(*pRestoreDisplayMode)(lpdd);
	if(res) OutTraceP("RestoreDisplayMode(D): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extGetAvailableVidMemProxy(LPDIRECTDRAW lpdd, LPDDSCAPS lpDDSCaps, LPDWORD lpdwTotal, LPDWORD lpdwFree)
{
	HRESULT res;
	OutTraceP("GetAvailableVidMem(D): PROXED lpdd=%x\n", lpdd);
	res=(*pGetAvailableVidMem)(lpdd, lpDDSCaps, lpdwTotal, lpdwFree);
	if(res) OutTraceP("GetAvailableVidMem(D): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("GetAvailableVidMem(D): DDSCaps=%x(%s) Total=%x Free=%x res=%x\n", 
		*lpDDSCaps, ExplainDDSCaps(lpDDSCaps->dwCaps), *lpdwTotal, *lpdwFree, res);
	return res; 
}

HRESULT WINAPI extRestoreAllSurfacesProxy(LPDIRECTDRAW lpdd) 
{
	HRESULT res;
	OutTraceP("RestoreAllSurfaces(D): PROXED lpdd=%x\n", lpdd);
	res=(*pRestoreAllSurfaces)(lpdd);
	if(res) OutTraceP("RestoreAllSurfaces(D): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extGetDeviceIdentifierProxy(LPDIRECTDRAW lpdd, LPDDDEVICEIDENTIFIER lpDevId, DWORD dw) 
{
	HRESULT res;
	OutTraceP("GetDeviceIdentifier(D): PROXED lpdd=%x DevId=%x dw=%x\n", lpdd, lpDevId, dw);
	res=(*pGetDeviceIdentifier)(lpdd, lpDevId, dw);
	if(res) OutTraceP("GetDeviceIdentifier(D): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("GetDeviceIdentifier(D): Id=%x Driver=\"%s\" Description=\"%s\" Version=%d rev.=%x\n", 
		lpDevId->dwDeviceId, lpDevId->szDriver, lpDevId->szDescription, lpDevId->liDriverVersion, lpDevId->dwRevision);
	return res;
}

ULONG WINAPI extCompactProxy(LPDIRECTDRAW lpdd)
{
	ULONG res;
	res=(*pCompact)(lpdd);
	OutTraceP("Compact(D): PROXED lpdd=%x res=%x\n", lpdd, res);
	return res;
}

HRESULT WINAPI extEnumDisplayModesProxy(EnumDisplayModes1_Type pEnumDisplayModes, LPDIRECTDRAW lpdd, DWORD dwflags, LPDDSURFACEDESC lpddsd, LPVOID lpContext, LPDDENUMMODESCALLBACK cb)
{
	HRESULT res;
	OutTraceP("EnumDisplayModes(D): PROXED lpdd=%x flags=%x lpddsd=%x callback=%x\n", lpdd, dwflags, lpddsd, cb);
	res=(*pEnumDisplayModes)(lpdd, dwflags, lpddsd, lpContext, cb);
	if(res) OutTraceP("EnumDisplayModes(D): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extEnumDisplayModes1Proxy(LPDIRECTDRAW lpdd, DWORD dwflags, LPDDSURFACEDESC lpddsd, LPVOID lpContext, LPDDENUMMODESCALLBACK cb)
{
	return extEnumDisplayModesProxy(pEnumDisplayModes1, lpdd, dwflags, lpddsd, lpContext, cb);
}

HRESULT WINAPI extEnumDisplayModes4Proxy(LPDIRECTDRAW lpdd, DWORD dwflags, LPDDSURFACEDESC2 lpddsd, LPVOID lpContext, LPDDENUMMODESCALLBACK2 cb)
{
	return extEnumDisplayModesProxy((EnumDisplayModes1_Type)pEnumDisplayModes4, lpdd, dwflags, (LPDDSURFACEDESC)lpddsd, lpContext, (LPDDENUMMODESCALLBACK)cb);
}

HRESULT WINAPI extGetFourCCCodesProxy(LPDIRECTDRAW lpdd, LPDWORD nc, LPDWORD cc)
{
	int res;
	unsigned int i;
	OutTraceP("GetFourCCCodes(D): PROXED lpdd=%x\n", lpdd);
	res=(*pGetFourCCCodes)(lpdd, nc, cc);
	if (res)
		OutTraceP("GetFourCCCodes(D): res=%x(%s)\n", res, ExplainDDError(res));
	else {
		OutTraceP("GetFourCCCodes(D): CCCodes(%d)=",*nc);
		if (cc)	
			for(i=0;i<*nc;i++) OutTraceP("%x,",cc[i]);
		else 
			OutTraceP("(NULL)");
		OutTraceP("\n");
	}
	return res;
}

HRESULT WINAPI extCreateSurfaceProxy(int dxVersion, CreateSurface_Type pCreateSurface, LPDIRECTDRAW lpdd, DDSURFACEDESC2 *lpddsd, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{	
	HRESULT res;
	OutTraceP("CreateSurface(D): PROXED version=%d lpdd=%x Flags=%x(%s)",
		dxVersion, lpdd, lpddsd->dwFlags, ExplainFlags(lpddsd->dwFlags));
	if (lpddsd->dwFlags & DDSD_BACKBUFFERCOUNT) OutTraceP(" BackBufferCount=%d", lpddsd->dwBackBufferCount);
	if (lpddsd->dwFlags & DDSD_WIDTH) OutTraceP(" Width=%d", lpddsd->dwWidth);
	if (lpddsd->dwFlags & DDSD_HEIGHT) OutTraceP(" Height=%d", lpddsd->dwHeight);
	if (lpddsd->dwFlags & DDSD_CAPS) OutTraceP(" Caps=%x(%s)", lpddsd->ddsCaps.dwCaps, ExplainDDSCaps(lpddsd->ddsCaps.dwCaps));
	if (lpddsd->dwFlags & DDSD_CKDESTBLT ) OutTraceP(" CKDestBlt=(%x,%x)", lpddsd->ddckCKDestBlt.dwColorSpaceLowValue, lpddsd->ddckCKDestBlt.dwColorSpaceHighValue);
	if (lpddsd->dwFlags & DDSD_CKDESTOVERLAY ) OutTraceP(" CKDestOverlay=(%x,%x)", lpddsd->ddckCKDestOverlay.dwColorSpaceLowValue, lpddsd->ddckCKDestOverlay.dwColorSpaceHighValue);
	if (lpddsd->dwFlags & DDSD_CKSRCBLT ) OutTraceP(" CKSrcBlt=(%x,%x)", lpddsd->ddckCKSrcBlt.dwColorSpaceLowValue, lpddsd->ddckCKSrcBlt.dwColorSpaceHighValue);
	if (lpddsd->dwFlags & DDSD_CKSRCOVERLAY ) OutTraceP(" CKSrcOverlay=(%x,%x)", lpddsd->ddckCKSrcOverlay.dwColorSpaceLowValue, lpddsd->ddckCKSrcOverlay.dwColorSpaceHighValue);
	OutTraceP("\n");
	res= pCreateSurface(lpdd, lpddsd, lplpdds, pu);
	if (res) {
		OutTraceP("CreateSurface(D): ERROR res=%x(%s)\n", res, ExplainDDError(res));
		return res;
	}
	if(IsDebug){
		OutTrace("CreateSurface(D): built Caps=%x(%s)\n", lpddsd->ddsCaps.dwCaps, ExplainDDSCaps(lpddsd->ddsCaps.dwCaps));
	}
	OutTraceP("CreateSurface(D): lpdds=%x\n", *lplpdds);
#ifdef HOOKSURFACE
	HookDDSurfaceProxy(lplpdds, dxVersion);
#endif	
	return res;
}

HRESULT WINAPI extCreateSurface1Proxy(LPDIRECTDRAW lpdd, DDSURFACEDESC *lpddsd, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	return extCreateSurfaceProxy(1, (CreateSurface_Type)pCreateSurface1, lpdd, (DDSURFACEDESC2 *)lpddsd, lplpdds, pu);
}

HRESULT WINAPI extCreateSurface2Proxy(LPDIRECTDRAW lpdd, DDSURFACEDESC *lpddsd, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	return extCreateSurfaceProxy(2, (CreateSurface_Type)pCreateSurface2, lpdd, (DDSURFACEDESC2 *)lpddsd, lplpdds, pu);
}

HRESULT WINAPI extCreateSurface4Proxy(LPDIRECTDRAW lpdd, DDSURFACEDESC2 *lpddsd, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	return extCreateSurfaceProxy(4, (CreateSurface_Type)pCreateSurface4, lpdd, (DDSURFACEDESC2 *)lpddsd, lplpdds, pu);
}

HRESULT WINAPI extCreateSurface7Proxy(LPDIRECTDRAW lpdd, DDSURFACEDESC2 *lpddsd, LPDIRECTDRAWSURFACE *lplpdds, void *pu)
{
	return extCreateSurfaceProxy(7, (CreateSurface_Type)pCreateSurface7, lpdd, (DDSURFACEDESC2 *)lpddsd, lplpdds, pu);
}

HRESULT WINAPI extEnumSurfacesProxy1(LPDIRECTDRAW lpdd, DWORD dwflags, LPDDSURFACEDESC lpddsd, LPVOID lpContext, LPDDENUMSURFACESCALLBACK cb)
{
	HRESULT res;
	OutTraceP("EnumSurfaces(D1): PROXED lpdd=%x flags=%x lpddsd=%x callback=%x\n", lpdd, dwflags, lpddsd, cb);
	res=(*pEnumSurfaces1)(lpdd, dwflags, lpddsd, lpContext, cb);
	if(res) OutTraceP("EnumSurfaces(D1): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extEnumSurfacesProxy4(LPDIRECTDRAW lpdd, DWORD dwflags, LPDDSURFACEDESC2 lpddsd, LPVOID lpContext, LPDDENUMSURFACESCALLBACK2 cb)
{
	HRESULT res;
	OutTraceP("EnumSurfaces(D4): PROXED lpdd=%x flags=%x lpddsd=%x callback=%x\n", lpdd, dwflags, lpddsd, cb);
	res=(*pEnumSurfaces4)(lpdd, dwflags, lpddsd, lpContext, cb);
	if(res) OutTraceP("EnumSurfaces(D4): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extDuplicateSurfaceProxy(LPDIRECTDRAW lpdd, LPDIRECTDRAWSURFACE s, LPDIRECTDRAWSURFACE *sp)
{
	int res;
	OutTraceP("DuplicateSurface(D): PROXED lpdd=%x dds=%x\n", lpdd, s);
	res=(*pDuplicateSurface)(lpdd, s, sp);
	if (res) OutTraceP("DuplicateSurface(D): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("DuplicateSurface: pdds=%x\n", *sp);
	return res;
}

HRESULT WINAPI extFlipToGDISurfaceProxy(LPDIRECTDRAW lpdd)
{
	HRESULT res;
	OutTraceP("FlipToGDISurface(D): lpdd=%x\n", lpdd);
	res=(*pFlipToGDISurface)(lpdd);
	if (res) OutTraceP("FlipToGDISurface(D): ERROR res=%x(%s), skipping\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extGetDisplayModeProxy(LPDIRECTDRAW lpdd, LPDDSURFACEDESC lpddsd)
{
	HRESULT res;
	OutTraceP("GetDisplayMode(D): PROXED lpdd=%x\n");
	res=(*pGetDisplayMode)(lpdd, lpddsd);
	if(res) OutTraceP("GetDisplayMode(D): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	else{
		OutTraceP("GetDisplayMode(D): WxH=(%dx%d) PixelFormat Flags=%x(%s) RGBBitCount=%d RGBAmask=(%x,%x,%x,%x) Caps=%x(%s)\n",
			lpddsd->dwWidth, lpddsd->dwHeight, 
			lpddsd->ddpfPixelFormat.dwFlags, ExplainPixelFormatFlags(lpddsd->ddpfPixelFormat.dwFlags),
			lpddsd->ddpfPixelFormat.dwRGBBitCount,
			lpddsd->ddpfPixelFormat.dwRBitMask, lpddsd->ddpfPixelFormat.dwGBitMask, lpddsd->ddpfPixelFormat.dwBBitMask,
			lpddsd->ddpfPixelFormat.dwRGBAlphaBitMask,
			lpddsd->ddsCaps.dwCaps, ExplainDDSCaps(lpddsd->ddsCaps.dwCaps));
	}
	return res;
}

HRESULT WINAPI extGetGDISurfaceProxy(LPDIRECTDRAW lpdd, LPDIRECTDRAWSURFACE *w)
{
	int res;
	OutTraceP("GetGDISurface(D): PROXED lpdd=%x\n", lpdd);
	res=(*pGetGDISurface)(lpdd, w);
	if (res) OutTraceP("GetGDISurface(D): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("GetGDISurface(D): lpdds=%x\n", *w);
	return res;
}

HRESULT WINAPI extSetCooperativeLevelProxy(void *lpdd, HWND hwnd, DWORD dwflags)
{
	HRESULT res;
	OutTraceP("SetCooperativeLevel(D): PROXED lpdd=%x hwnd=%x dwFlags=%x(%s)\n",
		lpdd, hwnd, dwflags,ExplainCoopFlags(dwflags));
	res=(*pSetCooperativeLevel)(lpdd, hwnd, dwflags);
	if (res) OutTraceP("SetCooperativeLevel(D): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extSetDisplayMode2Proxy(LPDIRECTDRAW lpdd,
	DWORD dwwidth, DWORD dwheight, DWORD dwbpp, DWORD dwrefreshrate, DWORD dwflags)
{
	HRESULT res;
	OutTraceP("SetDisplayMode(2): PROXED lpdd=%x WxH=(%dx%d) bpp=%d refresh=%x dwFlags=%x\n",
		lpdd, dwwidth, dwheight, dwbpp, dwrefreshrate, dwflags);
	res=(pSetDisplayMode2)(lpdd, dwwidth, dwheight, dwbpp, dwrefreshrate, dwflags);
	if (res) OutTraceP("SetDisplayMode(2): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extSetDisplayMode1Proxy(LPDIRECTDRAW lpdd,
	DWORD dwwidth, DWORD dwheight, DWORD dwbpp)
{
	HRESULT res;
	OutTraceP("SetDisplayMode(1): PROXED lpdd=%x WxH=(%dx%d) bpp=%d\n", lpdd, dwwidth, dwheight, dwbpp);
	res=(pSetDisplayMode1)(lpdd, dwwidth, dwheight, dwbpp);
	if (res) OutTraceP("SetDisplayMode(1): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extTestCooperativeLevelProxy(LPDIRECTDRAW lpdd)
{
	HRESULT res;
	OutTraceP("TestCooperativeLevel: PROXED lpdd=%x\n", lpdd);
	res=(*pTestCooperativeLevel)(lpdd);
	if (res) OutTraceP("TestCooperativeLevel: ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

// ------------------------------------------------------------------------------------------ //
//
// DirectDrawClipper proxed methods
//
// ------------------------------------------------------------------------------------------ //

HRESULT WINAPI extQueryInterfaceCProxy(void *lpddclip, REFIID riid, LPVOID *obp)
{
	HRESULT res;
	OutTraceP("QueryInterface(C): PROXED lpddClip=%x REFIID=%x(%s)\n", lpddclip, riid.Data1, ExplainGUID((GUID *)&riid.Data1));
	res = (*pQueryInterfaceC)(lpddclip, riid, obp);
	if(res) OutTraceP("QueryInterface(C): ERROR err=%x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("QueryInterface(C): obp=%x\n", *obp);
	return res;
}

HRESULT WINAPI extAddRefCProxy(LPDIRECTDRAWCLIPPER lpddClip)
{
	ULONG ref;
	ref=(*pAddRefC)(lpddClip);
	OutTraceP("AddRef(C): PROXED lpddClip=%x ref=%x\n", lpddClip, ref);
	return ref;
}

HRESULT WINAPI extGetClipListProxy(LPDIRECTDRAWCLIPPER lpddClip, LPRECT lpRect, LPRGNDATA lpRgnData, LPDWORD lpw)
{
	HRESULT res;
	if(IsTraceP){
		char sInfo[81];
		if (lpRect) sprintf(sInfo, "rect=(%d,%d)-(%d,%d) ", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
		else sprintf(sInfo, "rect=(NULL) ");
		OutTrace("GetClipList(C): PROXED lpddClip=%x %s\n", lpddClip, sInfo);
	}
	res=(*pGetClipList)(lpddClip, lpRect, lpRgnData, lpw);
	if(IsTraceP){
		//char *sInfo[1024];
		if(res) OutTrace("GetClipList(C): ERROR err=%x(%s)\n", res, ExplainDDError(res));
		else{
			if(lpRgnData){
				OutTrace("GetClipList(C): w=%x rgndataheader{size=%d type=%x count=%d RgnSize=%d bound=(%d,%d)-(%d,%d)}\n", 
				*lpw, lpRgnData->rdh.dwSize, lpRgnData->rdh.iType, lpRgnData->rdh.nCount, lpRgnData->rdh.nRgnSize, 
				lpRgnData->rdh.rcBound.left, lpRgnData->rdh.rcBound.top, lpRgnData->rdh.rcBound.right, lpRgnData->rdh.rcBound.bottom);
				if(IsDebug){
					RECT *rgns;
					rgns = (RECT *)lpRgnData->Buffer;
					for(DWORD i=0; i<lpRgnData->rdh.nCount; i++){
						OutTrace("GetClipList(C): rect[%d]=(%d,%d)-(%d,%d)\n", 
							i, rgns[i].left, rgns[i].top, rgns[i].right, rgns[i].bottom);
					}
				}
			}
			else{
				OutTrace("GetClipList(C): w=%x\n", *lpw);
			}
		}
	}
	return res;
}

HRESULT WINAPI extGetHWndProxy(LPDIRECTDRAWCLIPPER lpddClip, HWND FAR *lphwnd)
{
	HRESULT res;
	OutTraceP("GetHWnd(C): PROXED lpddClip=%x\n", lpddClip);
	res=(*pGetHWnd)(lpddClip, lphwnd);
	if(res) OutTraceP("GetHWnd(C): ERROR err=%x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("GetHWnd(C): hwnd=%x\n", *lphwnd);
	return res;
}

HRESULT WINAPI extInitializeCProxy(LPDIRECTDRAWCLIPPER lpddClip, LPDIRECTDRAW lpdd, DWORD w)
{
	HRESULT res;
	OutTraceP("Initialize(C): PROXED lpddClip=%x w=%x\n", lpddClip, w);
	res=(*pInitializeC)(lpddClip, lpdd, w);
	if(res) OutTraceP("Initialize(C): ERROR err=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extIsClipListChangedProxy(LPDIRECTDRAWCLIPPER lpddClip, BOOL FAR *w)
{
	HRESULT res;
	OutTraceP("IsClipListChanged(C): PROXED lpddClip=%x\n", lpddClip);
	res=(*pIsClipListChanged)(lpddClip, w);
	if(res) OutTraceP("IsClipListChanged(C): ERROR err=%x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("IsClipListChanged(C): w=%x\n", *w);
	return res;
}

HRESULT WINAPI extSetClipListProxy(LPDIRECTDRAWCLIPPER lpddClip, LPRGNDATA lpRegnData, DWORD flags)
{
	// to complete ....
	HRESULT res;
	OutTraceP("SetClipList(C): PROXED lpddClip=%x RgnData=??? flags=%x\n", lpddClip, flags);
	res=(*pSetClipList)(lpddClip, lpRegnData, flags);
	if(res) OutTraceP("SetClipList(C): ERROR err=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extSetHWndProxy(LPDIRECTDRAWCLIPPER lpddClip, DWORD w, HWND hwnd)
{
	HRESULT res;
	OutTraceP("SetHWnd(C): PROXED lpddClip=%x w=%x hwnd=%x\n", lpddClip, w, hwnd);
	res=(*pSetHWnd)(lpddClip, w, hwnd);
	if(res) OutTraceP("SetHWnd(C): ERROR err=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extReleaseCProxy(LPDIRECTDRAWCLIPPER lpddClip)
{
	ULONG ref;
	ref=(*pReleaseC)(lpddClip);
	OutTraceP("Release(C): PROXED lpddClip=%x ref=%x\n", lpddClip, ref);
	return ref;
}


// ------------------------------------------------------------------------------------------ //
//
// DirectDrawPalette proxed methods
//
// ------------------------------------------------------------------------------------------ //

HRESULT WINAPI extQueryInterfacePProxy(void *lpddp, REFIID riid, LPVOID *obp)
{
	HRESULT res;
	OutTraceP("QueryInterface(P): PROXED lpddp=%x REFIID=%x(%s)\n", lpddp, riid.Data1, ExplainGUID((GUID *)&riid.Data1));
	res = (*pQueryInterfaceP)(lpddp, riid, obp);
	if(res)	OutTraceP("QueryInterface(P): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("QueryInterface(P): obp=%x\n", *obp);
	return res;
}

HRESULT WINAPI extAddRefPProxy(LPDIRECTDRAWPALETTE lpddPalette)
{
	ULONG ref;
	ref=(*pAddRefP)(lpddPalette);
	OutTraceP("AddRef(P): PROXED lpddPalette=%x ref=%x\n", lpddPalette, ref);
	return ref;
}


HRESULT WINAPI extReleasePProxy(LPDIRECTDRAWPALETTE lpddPalette)
{
	ULONG ref;
	//OutTraceP("Release(P): lpddPalette=%x\n", lpddPalette);
	ref=(*pReleaseP)(lpddPalette);
	OutTraceP("Release(P): PROXED lpddPalette=%x ref=%x\n", lpddPalette, ref);
	return ref;
}

HRESULT WINAPI extGetCapsPProxy(LPDIRECTDRAWPALETTE lpddp, LPDWORD w)
{
	HRESULT res;
	OutTraceP("GetCaps(P): PROXED lpddp=%x\n", lpddp);
	res=(*pGetCapsP)(lpddp, w);
	if(res) OutTraceP("GetCaps(P): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("GetCaps(P): caps=%x\n", *w);
	return res;
}

HRESULT WINAPI extGetEntriesProxy(LPDIRECTDRAWPALETTE lpddp, DWORD dwflags, DWORD dwstart, DWORD dwcount, LPPALETTEENTRY lpentries)
{
	HRESULT res;
	OutTraceP("GetEntries(P): PROXED lpddp=%x flags=%x start=%d, count=%d\n", lpddp, dwflags, dwstart, dwcount);
	res=(*pGetEntries)(lpddp, dwflags, dwstart, dwcount, lpentries);
	if(IsDebug && !res) dxw.DumpPalette(dwcount, &lpentries[dwstart]);
	if (res) OutTraceP("GetEntries(P): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extSetEntriesProxy(LPDIRECTDRAWPALETTE lpddp, DWORD dwflags, DWORD dwstart, DWORD dwcount, LPPALETTEENTRY lpentries)
{
	HRESULT res;
	OutTraceP("SetEntries(P): PROXED lpddp=%x flags=%x start=%d, count=%d\n", lpddp, dwflags, dwstart, dwcount);
	if(IsDebug) dxw.DumpPalette(dwcount, &lpentries[dwstart]);
	res=(*pSetEntries)(lpddp, dwflags, dwstart, dwcount, lpentries);
	if (res) OutTraceP("SetEntries(P): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

// ------------------------------------------------------------------------------------------ //
//
// DirectDrawSurface proxed methods
//
// ------------------------------------------------------------------------------------------ //

HRESULT WINAPI extSetOverlayPositionProxy(LPDIRECTDRAWSURFACE lpdds, LONG l1, LONG l2)
{
	HRESULT res;
	OutTraceP("SetOverlayPosition(S): PROXED lpdds=%x l1=%x l2=%x\n", lpdds, l1, l2);
	res=(*pSetOverlayPosition)(lpdds, l1, l2);
	if (res) OutTraceP("SetOverlayPosition(S): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extEnumOverlayZOrdersProxy(LPDIRECTDRAWSURFACE lpdds, DWORD w, LPVOID v, LPDDENUMSURFACESCALLBACK eozCBack)
{
	HRESULT res;
	OutTraceP("EnumOverlayZOrders(S): PROXED lpdds=%x w=%x v=%x\n", lpdds, w, v);
	res=(*pEnumOverlayZOrders)(lpdds, w, v, eozCBack);
	if(res) OutTraceP("EnumOverlayZOrders(S): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extGetBltStatusProxy(LPDIRECTDRAWSURFACE lpdds, DWORD flags)
{
	HRESULT res;
	res=(*pGetBltStatus)(lpdds, flags);
	OutTraceP("GetBltStatus(S): PROXED lpdds=%x flags=%x(%s) res=%x(%s)\n", lpdds, flags, ExplainBltStatus(flags), res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extGetClipperProxy(LPDIRECTDRAWSURFACE lpdds, LPDIRECTDRAWCLIPPER FAR*lpddc)
{
	HRESULT res;
	OutTraceP("GetClipper(P): PROXED lpdds=%x\n", lpdds);
	res=(*pGetClipper)(lpdds, lpddc);
	if(res) OutTraceP("GetClipper(P): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("GetClipper(P): ddc=%x\n", *lpddc);
	return res;
}

HRESULT WINAPI extGetFlipStatusProxy(LPDIRECTDRAWSURFACE lpdds, DWORD flags)
{
	HRESULT res;
	OutTraceP("GetFlipStatus(S): PROXED lpdds=%x flags=%x(%s)\n", lpdds, flags, ExplainFlipStatus(flags));
	res=(*pGetFlipStatus)(lpdds, flags);
	if(res==DDERR_WASSTILLDRAWING) OutTraceP("GetFlipStatus(S): res=%x(%s)\n", res, ExplainDDError(res));
	else
	if(res) OutTraceP("GetFlipStatus(S): ERROR err=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extGetOverlayPositionProxy(LPDIRECTDRAWSURFACE lpdds, LPLONG lpl1, LPLONG lpl2)
{
	HRESULT res;
	OutTraceP("GetOverlayPosition(S): PROXED lpdds=%x\n", lpdds);
	res=(*pGetOverlayPosition)(lpdds, lpl1, lpl2);
	if(res) OutTraceP("GetOverlayPosition(S): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("GetOverlayPosition(S): p1=%x p2=%x\n", *lpl1, *lpl2);
	return res;
}

// ----------------- DirectDraw Surface proxy methods ------------------------//

HRESULT WINAPI extBltBatchProxy(LPDIRECTDRAWSURFACE lpdds, LPDDBLTBATCH lpDDBltBatch , DWORD dwCount, DWORD dwflags)
{
	HRESULT res;
	OutTraceP("BltBatch(S): PROXED lpdds=%x lpddsadd=%x DDBltBatch=%x count=%d flags=%x\n", lpdds, lpDDBltBatch, dwCount, dwflags);
	res=(*pBltBatch)(lpdds, lpDDBltBatch, dwCount, dwflags);
	if(res) OutTraceP("BltBatch(S): ERROR %x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extQueryInterfaceSProxy(void *lpdds, REFIID riid, LPVOID *obp)
{
	HRESULT res;
	unsigned int dxVersion;
	OutTraceP("QueryInterface(S): PROXED lpdds=%x REFIID=%x(%s)\n", lpdds, riid.Data1, ExplainGUID((GUID *)&riid.Data1));
	res = (*pQueryInterfaceS)(lpdds, riid, obp);
	if(res){
		OutTraceP("QueryInterface(S): ERROR err=%x(%s)\n", res, ExplainDDError(res));
		return res;
	}

	dxVersion=0;
	switch(riid.Data1){
	case 0x6C14DB81:
		dxVersion = 1;
		break;
	case 0x57805885:
		dxVersion = 2;
		break;
	case 0xDA044E00:
		dxVersion = 3;
		break;
	case 0x0B2B8630:
		dxVersion = 4;
		break;
	case 0x06675a80:
		dxVersion = 7;
		break;
	}
	OutTraceP("QueryInterface(S): obp=%x interface=%d\n", *obp, dxVersion);
	switch (dxVersion){
		case 1: 
		case 2:
		case 3:
		case 4: 
		case 7:
#ifdef HOOKSURFACE
			HookDDSurfaceProxy((LPDIRECTDRAWSURFACE *)obp, dxVersion);
#endif
		break;
	}
	return res;
}

HRESULT WINAPI extReleaseSProxy(LPDIRECTDRAWSURFACE lpdds)
{
	HRESULT res;
	res=(*pReleaseS)(lpdds);
	OutTraceP("Release(S): PROXED lpdds=%x res=%x\n", lpdds, res);
	return res;
}

HRESULT WINAPI extAddAttachedSurfaceProxy(LPDIRECTDRAWSURFACE lpdds, LPDIRECTDRAWSURFACE lpddsadd)
{
	HRESULT res;
	OutTraceP("AddAttachedSurface(S): PROXED lpdds=%x lpddsadd=%x\n", lpdds, lpddsadd);
	res=(*pAddAttachedSurface)(lpdds, lpddsadd);
	if(res) OutTraceP("AddAttachedSurface(S): ERROR ret=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extBltProxy(LPDIRECTDRAWSURFACE lpdds, LPRECT lpdestrect,
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwflags, LPDDBLTFX lpddbltfx)
{
	HRESULT res;
	OutTraceP("Blt(S): PROXED dest=%x src=%x dwFlags=%x(%s)",
		lpdds, lpddssrc, dwflags, ExplainBltFlags(dwflags));
	if (lpdestrect) 
		OutTraceP(" destrect=(%d,%d)-(%d,%d)", lpdestrect->left, lpdestrect->top, lpdestrect->right, lpdestrect->bottom);
	else
		OutTraceP(" destrect=(NULL)");
	if (lpsrcrect) 
		OutTraceP(" srcrect=(%d,%d)-(%d,%d)\n", lpsrcrect->left, lpsrcrect->top, lpsrcrect->right, lpsrcrect->bottom);
	else
		OutTraceP(" srcrect=(NULL)\n");
	res=(*pBlt)(lpdds, lpdestrect, lpddssrc, lpsrcrect, dwflags, lpddbltfx);
	if(res) OutTraceP("Blt(S): ERROR ret=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extBltFastProxy(LPDIRECTDRAWSURFACE lpdds, DWORD dwx, DWORD dwy, 
	LPDIRECTDRAWSURFACE lpddssrc, LPRECT lpsrcrect, DWORD dwtrans)
{
	HRESULT res;
	OutTraceP("BltFast(S): PROXED lpdds=%x xy=(%d,%d) src=%x dwtrans=%x(%s)",
		lpdds, dwx, dwy, lpddssrc, dwtrans, ExplainBltFastFlags(dwtrans));
	if (lpsrcrect) 
		OutTraceP(" srcrect=(%d,%d)-(%d,%d)\n", lpsrcrect->left, lpsrcrect->top, lpsrcrect->right, lpsrcrect->bottom);
	else
		OutTraceP(" srcrect=(NULL)\n");
	res=(*pBltFast)(lpdds, dwx, dwy, lpddssrc, lpsrcrect, dwtrans);
	if(res) OutTraceP("BltFast(S): ERROR ret=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extEnumAttachedSurfacesProxy(LPDIRECTDRAWSURFACE lpdds, LPVOID lpContext, LPDDENUMSURFACESCALLBACK lpEnumSurfacesCallback)
{
	HRESULT res;
	OutTraceP("EnumAttachedSurfaces(S): PROXED lpdds=%x Context=%x Callback=%x\n", lpdds, lpContext, lpEnumSurfacesCallback);
	res=(*pEnumAttachedSurfaces)(lpdds, lpContext, lpEnumSurfacesCallback);
	if(res) OutTraceP("EnumAttachedSurfaces(S): ERROR ret=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extGetAttachedSurface1Proxy(LPDIRECTDRAWSURFACE lpdds, LPDDSCAPS lpddsc, LPDIRECTDRAWSURFACE *lplpddas)
{
	HRESULT res;
	OutTraceP("GetAttachedSurface(S1): PROXED lpdds=%x caps=%x(%s)\n",
		lpdds, lpddsc->dwCaps, ExplainDDSCaps(lpddsc->dwCaps));
	res=(*pGetAttachedSurface1)(lpdds, lpddsc, lplpddas);
	if(res) OutTraceP("GetAttachedSurface(S1): ERROR ret=%x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("GetAttachedSurface(S1): lpddas=%x\n", *lplpddas);
	return res;
}

HRESULT WINAPI extGetAttachedSurface3Proxy(LPDIRECTDRAWSURFACE lpdds, LPDDSCAPS lpddsc, LPDIRECTDRAWSURFACE *lplpddas)
{
	HRESULT res;
	OutTraceP("GetAttachedSurface(S3): PROXED lpdds=%x caps=%x(%s)\n",
		lpdds, lpddsc->dwCaps, ExplainDDSCaps(lpddsc->dwCaps));
	res=(*pGetAttachedSurface3)(lpdds, lpddsc, lplpddas);
	if(res) OutTraceP("GetAttachedSurface(S3): ERROR ret=%x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("GetAttachedSurface(S3): lpddas=%x\n", *lplpddas);
	return res;
}

HRESULT WINAPI extGetAttachedSurface4Proxy(LPDIRECTDRAWSURFACE lpdds, LPDDSCAPS lpddsc, LPDIRECTDRAWSURFACE *lplpddas)
{
	HRESULT res;
	OutTraceP("GetAttachedSurface(S4): PROXED lpdds=%x caps=%x(%s)\n",
		lpdds, lpddsc->dwCaps, ExplainDDSCaps(lpddsc->dwCaps));
	res=(*pGetAttachedSurface4)(lpdds, lpddsc, lplpddas);
	if(res) OutTraceP("GetAttachedSurface(S4): ERROR ret=%x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("GetAttachedSurface(S4): lpddas=%x\n", *lplpddas);
	return res;
}

HRESULT WINAPI extGetAttachedSurface7Proxy(LPDIRECTDRAWSURFACE lpdds, LPDDSCAPS lpddsc, LPDIRECTDRAWSURFACE *lplpddas)
{
	HRESULT res;
	OutTraceP("GetAttachedSurface(S7): PROXED lpdds=%x caps=%x(%s)\n",
		lpdds, lpddsc->dwCaps, ExplainDDSCaps(lpddsc->dwCaps));
	res=(*pGetAttachedSurface7)(lpdds, lpddsc, lplpddas);
	if(res) OutTraceP("GetAttachedSurface(S7): ERROR ret=%x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("GetAttachedSurface(S7): lpddas=%x\n", *lplpddas);
	return res;
}

HRESULT WINAPI extGetCaps1SProxy(LPDIRECTDRAWSURFACE lpdds, LPDDSCAPS caps)
{
	HRESULT res;
	OutTraceP("GetCaps(S1): PROXED lpdds=%x\n", lpdds);
	res=(*pGetCaps1S)(lpdds, caps);
	if(res) OutTraceP("GetCaps(S1): ERROR %x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("GetCaps(S1): caps=%x(%s)\n", caps->dwCaps, ExplainDDSCaps(caps->dwCaps));
	return res;
}

HRESULT WINAPI extGetCaps2SProxy(LPDIRECTDRAWSURFACE lpdds, LPDDSCAPS caps)
{
	HRESULT res;
	OutTraceP("GetCaps(S2): PROXED lpdds=%x\n", lpdds);
	res=(*pGetCaps2S)(lpdds, caps);
	if(res) OutTraceP("GetCaps(S2): ERROR %x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("GetCaps(S2): caps=%x(%s)\n", caps->dwCaps, ExplainDDSCaps(caps->dwCaps));
	return res;
}

HRESULT WINAPI extGetCaps3SProxy(LPDIRECTDRAWSURFACE lpdds, LPDDSCAPS caps)
{
	HRESULT res;
	OutTraceP("GetCaps(S3): PROXED lpdds=%x\n", lpdds);
	res=(*pGetCaps3S)(lpdds, caps);
	if(res) OutTraceP("GetCaps(S3): ERROR %x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("GetCaps(S3): caps=%x(%s)\n", caps->dwCaps, ExplainDDSCaps(caps->dwCaps));
	return res;
}

HRESULT WINAPI extGetCaps4SProxy(LPDIRECTDRAWSURFACE lpdds, LPDDSCAPS2 caps)
{
	HRESULT res;
	OutTraceP("GetCaps(S4): PROXED lpdds=%x\n", lpdds);
	res=(*pGetCaps4S)(lpdds, caps);
	if(res) OutTraceP("GetCaps(S4): ERROR %x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("GetCaps(S4): caps=%x(%s)\n", caps->dwCaps, ExplainDDSCaps(caps->dwCaps));
	return res;
}

HRESULT WINAPI extGetCaps7SProxy(LPDIRECTDRAWSURFACE lpdds, LPDDSCAPS2 caps)
{
	HRESULT res;
	OutTraceP("GetCaps(S7): PROXED lpdds=%x\n", lpdds);
	res=(*pGetCaps7S)(lpdds, caps);
	if(res) OutTraceP("GetCaps(S7): ERROR %x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("GetCaps(S7): caps=%x(%s)\n", caps->dwCaps, ExplainDDSCaps(caps->dwCaps));
	return res;
}

HRESULT WINAPI extGetColorKeyProxy(LPDIRECTDRAWSURFACE lpdds, DWORD flags, LPDDCOLORKEY lpDDColorKey)
{
	HRESULT res;
	OutTraceP("GetColorKey(S): PROXED lpdds=%x flags=%x(%s)\n", lpdds, flags, ExplainColorKeyFlag(flags));
	res=(*pGetColorKey)(lpdds, flags, lpDDColorKey);
	if(res) OutTraceP("GetColorKey(S): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	else OutTraceP("GetColorKey(S): colors=(L:%x,H:%x)\n", 
		lpDDColorKey->dwColorSpaceLowValue, lpDDColorKey->dwColorSpaceHighValue);
	return res;
}

HRESULT WINAPI extGetPaletteProxy(LPDIRECTDRAWSURFACE lpdds, LPDIRECTDRAWPALETTE *lplpddp)
{
	HRESULT res;
	OutTraceP("GetPalette(S): PROXED lpdds=%x\n", lpdds);
	res=(*pGetPalette)(lpdds, lplpddp);
	if(res)OutTraceP("GetPalette(S): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extGetPixelFormatProxy(LPDIRECTDRAWSURFACE lpdds, LPDDPIXELFORMAT p)
{
	DWORD res;
	OutTraceP("GetPixelFormat(S): PROXED lpdds=%x\n", lpdds);
	res=(*pGetPixelFormat)(lpdds, p);
	if(res)
		OutTraceP("GetPixelFormat(S): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	else
		OutTraceP("GetPixelFormat(S): Flags=%x(%s) FourCC=%x BitCount=%d RGBA=(%x,%x,%x,%x)\n", 
			p->dwFlags, ExplainPixelFormatFlags(p->dwFlags), p->dwFourCC, p->dwRGBBitCount, 
			p->dwRBitMask, p->dwGBitMask, p->dwBBitMask, p->dwRGBAlphaBitMask );
	return res;
}

static char *sdtype(int dwSize)
{
	switch(dwSize){
		case sizeof(DDSURFACEDESC):  return "type1";	break;
		case sizeof(DDSURFACEDESC2): return "type2";	break;
		default: return "unknown";	break;
	}
	return "";
}

HRESULT WINAPI extSetClipperProxy(LPDIRECTDRAWSURFACE lpdds, LPDIRECTDRAWCLIPPER lpddc)
{
	HRESULT res;
	OutTraceP("SetClipper(S): PROXED lpdds=%x lpddc=%x\n", lpdds, lpddc);
	res=(*pSetClipper)(lpdds, lpddc);
	if (res)OutTraceP("SetClipper(S): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extSetColorKeyProxy(LPDIRECTDRAWSURFACE lpdds, DWORD flags, LPDDCOLORKEY lpDDColorKey)
{
	HRESULT res;
	OutTraceP("SetColorKey(S): PROXED lpdds=%x flags=%x ", lpdds, flags);
	if (lpDDColorKey)
		OutTraceP("colors=(L:%x,H:%x)\n",lpDDColorKey->dwColorSpaceLowValue, lpDDColorKey->dwColorSpaceHighValue);
	else
		OutTraceP("colors=(NULL)\n");
	res=(*pSetColorKey)(lpdds, flags, lpDDColorKey);
	if(res) OutTraceP("SetColorKey(S): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extSetPaletteProxy(LPDIRECTDRAWSURFACE lpdds, LPDIRECTDRAWPALETTE lpddp)
{
	HRESULT res;
	OutTraceP("SetPalette(S): PROXED lpdds=%x lpddp=%x\n", lpdds, lpddp);
	res=(*pSetPalette)(lpdds, lpddp);
	if(res)OutTraceP("SetPalette(S): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extLockProxy(LPDIRECTDRAWSURFACE lpdds, LPRECT lprect, LPDDSURFACEDESC lpdds2, DWORD flags, HANDLE hEvent)
{
	HRESULT res;
	OutTraceP("Lock(S): PROXED lpdds=%x flags=%x(%s) lpdds2=%x", lpdds, flags, ExplainLockFlags(flags), lpdds2);
	if (lprect) 
		OutTraceP(" rect=(%d,%d)-(%d,%d)\n", lprect->left, lprect->top, lprect->right, lprect->bottom);
	else
		OutTraceP(" rect=(NULL)\n");
	res=(*pLock)(lpdds, lprect, lpdds2, flags, hEvent);
	if (res) OutTraceP("Lock(S): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extUnlock1Proxy(LPDIRECTDRAWSURFACE lpdds, LPVOID lpvoid)
{
	HRESULT res;
	OutTraceP("Unlock(S): PROXED lpdds=%x ", lpdds);
	if (lpvoid)
		OutTraceP("lpvoid=%x->%x\n", lpvoid, *(DWORD *)lpvoid);
	else
		OutTraceP("lpvoid=(NULL)\n");
	res=(*pUnlock1)(lpdds, lpvoid);
	if(res)OutTraceP("Unlock(S): ERROR res=%x(%s)\n",res,ExplainDDError(res));
	return res;
}

HRESULT WINAPI extUnlock4Proxy(LPDIRECTDRAWSURFACE lpdds, LPRECT lprect)
{
	HRESULT res;
	OutTraceP("Unlock(S): PROXED lpdds=%x ", lpdds);
	if (lprect)
		OutTraceP("rect=(%d,%d)-(%d,%d)\n", lprect->left, lprect->top, lprect->right, lprect->bottom);
	else
		OutTraceP("rect=(NULL)\n");
	res=(*pUnlock4)(lpdds, lprect);
	if(res)OutTraceP("Unlock(S): ERROR res=%x(%s)\n",res,ExplainDDError(res));
	return res;
}

HRESULT WINAPI extGetDCProxy(LPDIRECTDRAWSURFACE lpdds, HDC FAR *pHDC)
{
	HRESULT res;
	OutTraceP("GetDC(S): PROXED lpdss=%x\n",lpdds);
	res=(*pGetDC)(lpdds,pHDC);
	if(res) OutTraceP("GetDC(S): ERROR res=%x(%s)\n",res,ExplainDDError(res));
	else OutTraceP("GetDC(S): hdc=%x\n",*pHDC);
	return res;
}

HRESULT WINAPI extReleaseDCProxy(LPDIRECTDRAWSURFACE lpdds, HDC FAR hdc)
{
	HRESULT res;
	OutTraceP("ReleaseDC(S): lpdds=%x hdc=%x\n", lpdds, hdc);
	res=(*pReleaseDC)(lpdds,hdc);
	if(res) OutTraceP("ReleaseDC(S): ERROR res=%x(%s)\n",res,ExplainDDError(res));
	return res;
}

HRESULT WINAPI extDeleteAttachedSurfaceProxy(LPDIRECTDRAWSURFACE lpdds,  DWORD dwflags, LPDIRECTDRAWSURFACE lpddsdel)
{
	HRESULT res;
	OutTraceP("DeleteAttachedSurface(S): lpdds=%x flags=%x lpddsdel=%x\n", lpdds, dwflags, lpddsdel);
	res=(*pDeleteAttachedSurface)(lpdds, dwflags, lpddsdel);
	if(res) OutTraceP("DeleteAttachedSurface(S): ERROR %x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extIsLostProxy(LPDIRECTDRAWSURFACE lpdds)
{
	HRESULT res;
	//OutTraceP("IsLost: PROXED lpdds=%x\n", lpdds);
	res=(*pIsLost)(lpdds);
	OutTraceP("IsLost(S): PROXED lpdds=%x res=%x\n", lpdds, res);
	return res;
}

HRESULT WINAPI extRestoreProxy(LPDIRECTDRAWSURFACE lpdds)
{
	HRESULT res;
	OutTraceP("Restore(S): PROXED lpdds=%x\n", lpdds);
	res=(*pRestore)(lpdds);
	if(res)	OutTraceP("Restore(S): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extUpdateOverlayDisplayProxy(LPDIRECTDRAWSURFACE lpdds, DWORD dwflags)
{ 
	HRESULT res;
	OutTraceP("UpdateOverlayDisplay(S): PROXED lpdds=%x flags=%x\n", lpdds, dwflags); 
	res=(*pUpdateOverlayDisplay)(lpdds, dwflags);
	if(res) OutTraceP("UpdateOverlayDisplay(S): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extFlipProxy(LPDIRECTDRAWSURFACE lpdds, LPDIRECTDRAWSURFACE lpddst, DWORD dwflags)
{
	HRESULT res;
	OutTraceP("Flip(S): PROXED target=%x, src=%x, flags=%x(%s)\n", lpdds, lpddst, dwflags, ExplainFlipFlags(dwflags));
	res=(*pFlip)(lpdds, lpddst, dwflags);
	if(res) OutTraceP("Flip(S): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extAddOverlayDirtyRectProxy(LPDIRECTDRAWSURFACE lpdds, LPRECT drect)
{
	HRESULT res;
	OutTraceP("AddOverlayDirtyRect(S): PROXED lpdds=%x ", lpdds);
	if (drect)
		OutTraceP(" drect=(%d,%d)-(%d,%d)\n",drect->left, drect->top, drect->right, drect->bottom);
	else
		OutTraceP(" drect=(NULL)\n");

	res=(*pAddOverlayDirtyRect)(lpdds, drect);
	if(res) OutTraceP("AddOverlayDirtyRect(S): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extUpdateOverlayProxy(LPDIRECTDRAWSURFACE lpdds, LPRECT lpSrcRect, LPDIRECTDRAWSURFACE lpddsdest, LPRECT lpDestRect, DWORD dwflags, LPDDOVERLAYFX f)
{ 
	HRESULT res;
	OutTraceP("UpdateOverlay(S): PROXED lpdds=%x lpddsdest=%x dwflags=%x", lpdds, lpddsdest, dwflags); 
	if (lpSrcRect)
		OutTraceP(" src=(%d,%d)-(%d,%d)",lpSrcRect->left, lpSrcRect->top, lpSrcRect->right, lpSrcRect->bottom);
	else
		OutTraceP(" src=(NULL)");
	if (lpDestRect)
		OutTraceP(" dest=(%d,%d)-(%d,%d)\n",lpDestRect->left, lpDestRect->top, lpDestRect->right, lpDestRect->bottom);
	else
		OutTraceP(" dest=(NULL)\n");
	res=(*pUpdateOverlay)(lpdds, lpSrcRect, lpddsdest, lpDestRect, dwflags, f);
	if(res) OutTraceP("UpdateOverlay(S): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

HRESULT WINAPI extUpdateOverlayZOrderProxy(LPDIRECTDRAWSURFACE lpdds, DWORD flags, LPDIRECTDRAWSURFACE lpDDSReference)
{
	HRESULT res;
	OutTraceP("UpdateOverlayZOrder(S): PROXED lpdds=%x lpddsref=%x flags=%x\n",lpdds, lpDDSReference, flags);
	res=(*pUpdateOverlayZOrder)(lpdds, flags, lpDDSReference);
	if(res) OutTraceP("UpdateOverlayZOrder(S): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	return res;
}

ULONG WINAPI extAddRefSProxy(LPDIRECTDRAWSURFACE lpdds)
{
	ULONG ref;
	ref=(*pAddRefS)(lpdds);
	OutTraceP("AddRef(S): PROXED lpdds=%x ref=%x\n", lpdds, ref);
	return ref;
}





static void HookDDSessionProxy(LPDIRECTDRAW *lplpdd, int dxVersion)
{
	OutTraceP("Hooking directdraw session dd=%x dxVersion=%d thread_id=%x\n", *lplpdd, dxVersion, GetCurrentThreadId());

	// IDIrectDraw::QueryInterface
	SetHook((void *)(**(DWORD **)lplpdd + 0), extQueryInterfaceDProxy, (void **)&pQueryInterfaceD, "QueryInterface(D)");
#ifdef TRACECOMMETHODS
	// IDIrectDraw::AddRef
	SetHook((void *)(**(DWORD **)lplpdd + 4), extAddRefDProxy, (void **)&pAddRefD, "AddRef(D)");
	// IDIrectDraw::Release
	SetHook((void *)(**(DWORD **)lplpdd + 8), extReleaseDProxy, (void **)&pReleaseD, "Release(D)");
#endif
	// IDIrectDraw::Compact
	SetHook((void *)(**(DWORD **)lplpdd + 12), extCompactProxy, (void **)&pCompact, "Compact(D)");
	// IDIrectDraw::CreateClipper
	SetHook((void *)(**(DWORD **)lplpdd + 16), extCreateClipperProxy, (void **)&pCreateClipper, "CreateClipper(D)");
	// IDIrectDraw::CreatePalette
	SetHook((void *)(**(DWORD **)lplpdd + 20), extCreatePaletteProxy, (void **)&pCreatePalette, "CreatePalette(D)");
	switch(dxVersion) {
	case 1:
		SetHook((void *)(**(DWORD **)lplpdd + 24), extCreateSurface1Proxy, (void **)&pCreateSurface1, "CreateSurface(D1)");
		break;
	case 2:
		SetHook((void *)(**(DWORD **)lplpdd + 24), extCreateSurface2Proxy, (void **)&pCreateSurface2, "CreateSurface(D2)");
		break;
	case 4:
		SetHook((void *)(**(DWORD **)lplpdd + 24), extCreateSurface4Proxy, (void **)&pCreateSurface4, "CreateSurface(D4)");
		break;
	case 7:
		SetHook((void *)(**(DWORD **)lplpdd + 24), extCreateSurface7Proxy, (void **)&pCreateSurface7, "CreateSurface(D7)");
		break;	
	}
	// IDIrectDraw::DuplicateSurface
	SetHook((void *)(**(DWORD **)lplpdd + 28), extDuplicateSurfaceProxy, (void **)&pDuplicateSurface, "DuplicateSurface(D)");
	// IDIrectDraw::EnumDisplayModes
	switch(dxVersion) {
	case 1:
	case 2:
		SetHook((void *)(**(DWORD **)lplpdd + 32), extEnumDisplayModes1Proxy, (void **)&pEnumDisplayModes1, "EnumDisplayModes(D1)");
		break;
	case 4:
	case 7:
		SetHook((void *)(**(DWORD **)lplpdd + 32), extEnumDisplayModes4Proxy, (void **)&pEnumDisplayModes4, "EnumDisplayModes(D4)");
		break;
	}
	// IDIrectDraw::EnumSurfaces
	if (dxVersion < 4)
		SetHook((void *)(**(DWORD **)lplpdd + 36), extEnumSurfacesProxy1, (void **)&pEnumSurfaces1, "EnumSurfaces(D1)");
	else 
		SetHook((void *)(**(DWORD **)lplpdd + 36), extEnumSurfacesProxy4, (void **)&pEnumSurfaces4, "EnumSurfaces(D4)");
	// IDIrectDraw::FlipToGDISurface
	SetHook((void *)(**(DWORD **)lplpdd + 40), extFlipToGDISurfaceProxy, (void **)&pFlipToGDISurface, "FlipToGDISurface(D)");
	// IDIrectDraw::GetCaps
	SetHook((void *)(**(DWORD **)lplpdd + 44), extGetCapsD, (void **)&pGetCapsD, "GetCaps(D)");
	// IDIrectDraw::GetDisplayMode
	SetHook((void *)(**(DWORD **)lplpdd + 48), extGetDisplayModeProxy, (void **)&pGetDisplayMode, "GetDisplayMode(D)");
	// IDIrectDraw::GetFourCCCodes
	SetHook((void *)(**(DWORD **)lplpdd + 52), extGetFourCCCodesProxy, (void **)&pGetFourCCCodes, "GetFourCCCodes(D)");
	// IDIrectDraw::GetGDISurface
	SetHook((void *)(**(DWORD **)lplpdd + 56), extGetGDISurfaceProxy, (void **)&pGetGDISurface, "GetGDISurface(D)");
	// IDIrectDraw::GetMonitorFrequency
	SetHook((void *)(**(DWORD **)lplpdd + 60), extGetMonitorFrequencyProxy, (void **)&pGetMonitorFrequency, "GetMonitorFrequency(D)");
	// IDIrectDraw::GetScanLine
	SetHook((void *)(**(DWORD **)lplpdd + 64), extGetScanLineProxy, (void **)&pGetScanLine, "GetScanLine(D)");
	// IDIrectDraw::GetVerticalBlankStatus
	SetHook((void *)(**(DWORD **)lplpdd + 68), extGetVerticalBlankStatusProxy, (void **)&pGetVerticalBlankStatus, "GetVerticalBlankStatus(D)");
	// IDIrectDraw::Initialize
	SetHook((void *)(**(DWORD **)lplpdd + 72), extInitializeProxy, (void **)&pInitialize, "Initialize(D)");
	// IDIrectDraw::RestoreDisplayMode
	SetHook((void *)(**(DWORD **)lplpdd + 76), extRestoreDisplayModeProxy, (void **)&pRestoreDisplayMode, "RestoreDisplayMode(D)");
	if (dxVersion >= 2){
		// IDIrectDraw::GetAvailableVidMem
		SetHook((void *)(**(DWORD **)lplpdd + 92), extGetAvailableVidMemProxy, (void **)&pGetAvailableVidMem, "GetAvailableVidMem(D)");
	}
	if (dxVersion >= 4){
		// IDIrectDraw::GetSurfaceFromDC
		SetHook((void *)(**(DWORD **)lplpdd + 96), extGetSurfaceFromDCProxy, (void **)&pGetSurfaceFromDC, "GetSurfaceFromDC(D)");
		// IDIrectDraw::RestoreAllSurfaces
		SetHook((void *)(**(DWORD **)lplpdd + 100), extRestoreAllSurfacesProxy, (void **)&pRestoreAllSurfaces, "RestoreAllSurfaces(D)");
		// IDIrectDraw::GetDeviceIdentifier
		SetHook((void *)(**(DWORD **)lplpdd + 108), extGetDeviceIdentifierProxy, (void **)&pGetDeviceIdentifier, "GetDeviceIdentifier(D)");
	}
	// IDIrectDraw::SetCooperativeLevel
	SetHook((void *)(**(DWORD **)lplpdd + 80), extSetCooperativeLevelProxy, (void **)&pSetCooperativeLevel, "SetCooperativeLevel(D)");
	// IDIrectDraw::SetDisplayMode
	if (dxVersion > 1){
		SetHook((void *)(**(DWORD **)lplpdd + 84), extSetDisplayMode2Proxy, (void **)&pSetDisplayMode2, "SetDisplayMode(D2)");
	}
	else {
		SetHook((void *)(**(DWORD **)lplpdd + 84), extSetDisplayMode1Proxy, (void **)&pSetDisplayMode1, "SetDisplayMode(D1)");
	}
#if 0
	// IDIrectDraw::WaitForVerticalBlank
	SetHook((void *)(**(DWORD **)lplpdd + 88), extWaitForVerticalBlankProxy, (void **)&pWaitForVerticalBlank, "WaitForVerticalBlank(D)");
#endif
	if (dxVersion >= 4){
		// IDIrectDraw::TestCooperativeLevel
		SetHook((void *)(**(DWORD **)lplpdd + 104), extTestCooperativeLevelProxy, (void **)&pTestCooperativeLevel, "TestCooperativeLevel(D)");
	}
}

HRESULT WINAPI extDirectDrawCreateProxy(GUID FAR *lpguid, LPDIRECTDRAW FAR *lplpdd, IUnknown FAR *pu)
{
	HRESULT res;

	OutTraceP("DirectDrawCreate: PROXED guid=%x(%s)\n", 
		lpguid, ExplainGUID(lpguid));

	res = (*pDirectDrawCreate)(lpguid, lplpdd, pu);
	if(res) {
		OutTraceP("DirectDrawCreate: ERROR res=%x(%s)\n", res, ExplainDDError(res));
		return res;
	}

	dxw.dwDDVersion=1;
	char *mode;
	switch ((DWORD)lpguid){
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
	OutTraceP("DirectDrawCreateEx: lpdd=%x guid=%s DDVersion=%d\n", *lplpdd, mode, dxw.dwDDVersion);

#ifdef HOOKDDRAW
	HookDDSessionProxy(lplpdd, dxw.dwDDVersion);
#endif
	return 0;
}

HRESULT WINAPI extDirectDrawCreateExProxy(GUID FAR *lpguid, LPDIRECTDRAW FAR *lplpdd, REFIID RefIid, IUnknown FAR *pu)
{
	HRESULT res;

	OutTraceP("DirectDrawCreateEx: PROXED guid=%x(%s) refiid=%x\n", 
		lpguid, ExplainGUID(lpguid), RefIid);

	res = (*pDirectDrawCreateEx)(lpguid, lplpdd, RefIid, pu);
	if(res) {
		OutTraceP("DirectDrawCreateEx: ERROR res=%x(%s)\n", res, ExplainDDError(res));
		return res;
	}

	dxw.dwDDVersion=7;
	char *mode;
	switch ((DWORD)lpguid){
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
	OutTraceP("DirectDrawCreateEx: lpdd=%x guid=%s DDVersion=%d\n", *lplpdd, mode, dxw.dwDDVersion);

#ifdef HOOKDDRAW
	HookDDSessionProxy(lplpdd, dxw.dwDDVersion);
#endif

	return 0;
}

HRESULT WINAPI extGetSurfaceDesc1Proxy(LPDIRECTDRAWSURFACE lpdds, LPDDSURFACEDESC lpddsd)
{
	HRESULT res;
	OutTraceP("GetSurfaceDesc(1): PROXED lpdds=%x lpddsd=%x size=%x\n", lpdds, lpddsd, lpddsd->dwSize);
	res=(*pGetSurfaceDesc1)(lpdds, lpddsd);
	if (res) OutTraceP("GetSurfaceDesc(1): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	else DumpSurfaceAttributesProxy(lpddsd, "GetSurfaceDesc(1)");
	return res;
}

HRESULT WINAPI extGetSurfaceDesc4Proxy(LPDIRECTDRAWSURFACE2 lpdds, LPDDSURFACEDESC2 lpddsd)
{
	HRESULT res;
	OutTraceP("GetSurfaceDesc(4): PROXED lpdds=%x lpddsd=%x size=%x\n", lpdds, lpddsd, lpddsd->dwSize);
	res=(*pGetSurfaceDesc4)(lpdds, lpddsd);
	if (res) OutTraceP("GetSurfaceDesc(4): ERROR res=%x(%s)\n", res, ExplainDDError(res));
	else DumpSurfaceAttributesProxy((LPDDSURFACEDESC)lpddsd, "GetSurfaceDesc(4)");
	return res;
}

BOOL WINAPI DDEnumerateCallback(GUID *lpGuid, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext)
{
	OutTrace("DDEnumerateCallback: guid=%x DriverDescription=\"%s\" DriverName=\"%s\" Context=%x\n", 
		lpGuid, lpDriverDescription, lpDriverName, lpContext);
	return TRUE;
}

BOOL WINAPI DDEnumerateCallbackEx(GUID *lpGuid, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext, HMONITOR hm)
{
	OutTrace("DDEnumerateCallbackEx: guid=%x DriverDescription=\"%s\" DriverName=\"%s\" Context=%x hm=%x\n", 
		lpGuid, lpDriverDescription, lpDriverName, lpContext, hm);
	return TRUE;
}

HRESULT WINAPI extDirectDrawEnumerateProxy(LPDDENUMCALLBACK lpCallback, LPVOID lpContext)
{
	HRESULT ret;
	OutTraceP("DirectDrawEnumerate: PROXED lpCallback=%x lpContext=%x\n", lpCallback, lpContext);
	if(IsDebug) (*pDirectDrawEnumerate)(DDEnumerateCallback, lpContext);
	ret=(*pDirectDrawEnumerate)(lpCallback, lpContext);
	if(ret) OutTraceP("DirectDrawEnumerate: ERROR res=%x(%s)\n", ret, ExplainDDError(ret));
	return ret;
}

HRESULT WINAPI extDirectDrawEnumerateExProxy(LPDDENUMCALLBACKEX lpCallback, LPVOID lpContext, DWORD dwFlags)
{
	HRESULT ret;
	OutTraceP("DirectDrawEnumerateEx: PROXED lpCallback=%x lpContext=%x Flags=%x(%s)\n", 
		lpCallback, lpContext, dxw.dwFlags1, ExplainDDEnumerateFlags(dwFlags));
	(*pDirectDrawEnumerateEx)(DDEnumerateCallbackEx, lpContext, dwFlags);
	ret=(*pDirectDrawEnumerateEx)(lpCallback, lpContext, dwFlags);
	if(ret) OutTraceP("DirectDrawEnumerateEx: ERROR res=%x(%s)\n", ret, ExplainDDError(ret));
	return ret;
}

// GDI calls proxy

extern GDIGetDC_Type pGDIGetDC;
//GDIGetDC_Type pGDIGetWinDC;
extern GDIReleaseDC_Type pGDIReleaseDC;
extern CreateDC_Type pGDICreateDC;
extern CreateCompatibleDC_Type pGDICreateCompatibleDC;
extern BitBlt_Type pGDIBitBlt;
extern PatBlt_Type pGDIPatBlt;
extern StretchBlt_Type pGDIStretchBlt;
extern DeleteDC_Type pGDIDeleteDC;
extern SaveDC_Type pSaveDC;
extern RestoreDC_Type pRestoreDC;

HDC WINAPI extGDIGetDCProxy(HWND hwnd)
{
	HDC ret;
	ret=(*pGDIGetDC)(hwnd);
	OutTraceP("GDI.GetDC: PROXED hwnd=%x ret=%x\n", hwnd, ret);
	return ret;
}

HDC WINAPI extGetWindowDCProxy(HWND hwnd)
{
	HDC ret;
	ret=(*pGDIGetWindowDC)(hwnd);
	OutTraceP("GDI.GetWindowDC: PROXED hwnd=%x ret=%x\n", hwnd, ret);
	return ret;
}

int WINAPI extGDIReleaseDCProxy(HWND hwnd, HDC hDC)
{
	int res;
	OutTraceP("GDI.ReleaseDC: PROXED hwnd=%x hdc=%x\n", hwnd, hDC);
	res=(*pGDIReleaseDC)(hwnd, hDC);
	if (!res) OutTraceP("GDI.ReleaseDC ERROR: err=%d\n", GetLastError());
	return(res);
}

HDC WINAPI extCreateDCProxy(LPSTR Driver, LPSTR Device, LPSTR Output, CONST DEVMODE *InitData)
{
	HDC RetHDC;
	OutTraceP("GDI.CreateDC: PROXED Driver=%s Device=%s Output=%s InitData=%x\n", 
		Driver?Driver:"(NULL)", Device?Device:"(NULL)", Output?Output:"(NULL)", InitData);

	RetHDC=(*pGDICreateDC)(Driver, Device, Output, InitData);
	if(RetHDC)
		OutTraceP("GDI.CreateDC: returning HDC=%x\n", RetHDC);
	else
		OutTraceP("GDI.CreateDC ERROR: err=%d\n", GetLastError());
	return RetHDC;
}

HDC WINAPI extCreateCompatibleDCProxy(HDC hdc)
{
	HDC RetHdc;
	OutTraceP("GDI.CreateCompatibleDC: PROXED hdc=%x\n", hdc);
	RetHdc=(*pGDICreateCompatibleDC)(hdc);
	if(RetHdc)
		OutTraceP("GDI.CreateCompatibleDC: returning HDC=%x\n", RetHdc);
	else
		OutTraceP("GDI.CreateCompatibleDC ERROR: err=%d\n", GetLastError());
	return RetHdc;
}

BOOL WINAPI extBitBltProxy(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, HDC hdcSrc, int nXSrc, int nYSrc, DWORD dwRop)
{
	BOOL res;

	OutTraceP("GDI.BitBlt: PROXED HDC=%x nXDest=%d nYDest=%d nWidth=%d nHeight=%d hdcSrc=%x nXSrc=%d nYSrc=%d dwRop=%x(%s)\n", 
		hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop, ExplainROP(dwRop));

	res=(*pGDIBitBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
	if(!res) OutTraceP("GDI.BitBlt: ERROR err=%d\n", GetLastError());
	return res;
}

BOOL WINAPI extPatBltProxy(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, DWORD dwRop)
{
	BOOL res;

	OutTraceP("GDI.PatBlt: PROXED HDC=%x nXDest=%d nYDest=%d nWidth=%d nHeight=%d dwRop=%x(%s)\n", 
		hdcDest, nXDest, nYDest, nWidth, nHeight, dwRop, ExplainROP(dwRop));

	res=(*pGDIPatBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, dwRop);
	if(!res) OutTraceP("GDI.PatBlt: ERROR err=%d\n", GetLastError());
	return res;
}

BOOL WINAPI extStretchBltProxy(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, 
							 HDC hdcSrc, int nXSrc, int nYSrc, int nWSrc, int nHSrc, DWORD dwRop)
{
	BOOL res;

	OutTraceP("GDI.StretchBlt: PROXED HDC=%x nXDest=%d nYDest=%d nWidth=%d nHeight=%d hdcSrc=%x nXSrc=%d nYSrc=%d nWSrc=%d nHSrc=%d dwRop=%x(%s)\n", 
		hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, nWSrc, nHSrc, dwRop, ExplainROP(dwRop));

	res=(*pGDIStretchBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, nWSrc, nHSrc, dwRop);
	if(!res) OutTraceP("GDI.StretchBlt: ERROR err=%d\n", GetLastError());
	return res;
}

BOOL WINAPI extDeleteDCProxy(HDC hdc)
{
	BOOL res;

	OutTraceP("GDI.DeleteDC: PROXED hdc=%x\n", hdc);
	res=(*pGDIDeleteDC)(hdc);
	if(!res) OutTraceP("GDI.DeleteDC: ERROR err=%d\n", GetLastError());
	return res;
}

int WINAPI extSaveDCProxy(HDC hdc)
{
	int ret;

	ret=(*pGDISaveDC)(hdc);
	OutTraceP("GDI.SaveDC: PROXED hdc=%x ret=%x\n", hdc, ret);
	return ret;
}

BOOL WINAPI extRestoreDCProxy(HDC hdc, int nSavedDC)
{
	BOOL ret;

	ret=(*pGDIRestoreDC)(hdc, nSavedDC);
	OutTraceP("GDI.RestoreDC: PROXED hdc=%x nSavedDC=%x ret=%x\n", hdc, nSavedDC, ret);
	return ret;
}

HPALETTE WINAPI extSelectPaletteProxy(HDC hdc, HPALETTE hpal, BOOL bForceBackground)
{
	HPALETTE ret;

	ret=(*pGDISelectPalette)(hdc, hpal, bForceBackground);
	OutTraceP("GDI.SelectPalette: PROXED hdc=%x hpal=%x ForceBackground=%x ret=%x\n", hdc, hpal, bForceBackground, ret);
	return ret;
}

UINT WINAPI extRealizePaletteProxy(HDC hdc)
{
	UINT ret;

	ret=(*pGDIRealizePalette)(hdc);
	OutTraceP("GDI.RealizePalette: PROXED hdc=%x ret=%x\n", hdc, ret);
	return ret;
}

UINT WINAPI extGetSystemPaletteEntriesProxy(HDC hdc, UINT iStartIndex, UINT nEntries, LPPALETTEENTRY lppe)
{
	UINT ret;

	ret=(*pGDIGetSystemPaletteEntries)(hdc, iStartIndex, nEntries, lppe);
	OutTrace("GDI.GetSystemPaletteEntries: PROXED hdc=%x start=%d num=%d ret=%d\n", hdc, iStartIndex, nEntries, ret);
	if(IsDebug && ret) dxw.DumpPalette(nEntries, &lppe[iStartIndex]);
	if(!ret) OutTrace("GDI.GetSystemPaletteEntries: ERROR err=%d\n", GetLastError());
	return ret;
}

HDC WINAPI extBeginPaintProxy(HWND hwnd, LPPAINTSTRUCT lpPaint)
{
	HDC ret;

	OutTraceP("GDI.BeginPaint: PROXED hwnd=%x lpPaint=%x\n", hwnd, lpPaint);
	ret=(*pBeginPaint)(hwnd, lpPaint);
	OutTraceP("GDI.BeginPaint: ret=%x\n", ret);
	return ret;
}

int WINAPI extGetDeviceCapsProxy(HDC hdc, int nindex)
{
	DWORD res;
	
	res = (*pGDIGetDeviceCaps)(hdc, nindex);
	OutTraceDW("GetDeviceCaps: hdc=%x index=%x(%s) res=%x\n",
		hdc, nindex, ExplainDeviceCaps(nindex), res);

	switch(nindex){
	case VERTRES:
		OutTraceDW("GetDeviceCaps: VERTRES=%d\n", res);
		break;
	case HORZRES:
		OutTraceDW("GetDeviceCaps: HORZRES=%d\n", res);
		break;
	case RASTERCAPS:
		OutTraceDW("GetDeviceCaps: RASTERCAPS=%x(%s)\n",res, ExplainRasterCaps(res));
		break;
	}
	return res;
}


